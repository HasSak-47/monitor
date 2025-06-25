---@class ColorAnsi
---@field type "bit"
---@field R 0 | 1
---@field G 0 | 1
---@field B 0 | 1

---@class Color8bit
---@field type "8bit"
---@field R integer @[0,255]
---@field G integer @[0,255]
---@field B integer @[0,255]

---@alias Color ColorAnsi | Color8bit

---@class Buffer
---@field set function

---@class Widget
---@field new function
---@field render function
---@field update function

---@class Widget
local Bar = widget:extend({
    new = function(self)
        local t = self.super.new(self)
        setmetatable(t, self)

        t._type = "Bar"

        t.percentage = 0.
        t.show_number = false

        t.__index = self
        return t
    end,

    render = function(self, buffer)
        local x, _ = buffer:get_size()

        if self.show_number and x > 10 then
            local s = string.format("%.0f%%", self.percentage * 100)
            buffer:get_sub(x - 7, 1, 7, 1):render(s)
            x = x - 8
        end

        buffer:set(1, 1, '[')
        buffer:set(x, 1, ']')

        local bar_width = x - 2
        for i = 1, math.floor(bar_width * self.percentage) do
            buffer:set(i + 1, 1, '|', { type = "bit", r = 1, g = 0, b = 0 })
        end
    end
})

---@class Widget
local Box = widget:extend({
    new = function(self, inner)
        local t = self.super.new(self)
        setmetatable(t, self)

        t._type = "Box"
        t.__index = self
        t.inner = inner

        return t
    end,

    render = function(self, buffer)
        local x, y = buffer:get_size()

        for i = 2, x - 1 do
            for j = 2, y - 1 do
                buffer:set(i, j, ' ') -- Right
            end
        end
        for i = 2, x - 1 do
            buffer:set(i, 1, '-') -- Top
            buffer:set(i, y, '-') -- Bottom
        end
        for j = 2, y - 1 do
            buffer:set(1, j, '|') -- Left
            buffer:set(x, j, '|') -- Right
        end
        buffer:set(1, 1, '+')
        buffer:set(x, 1, '+')
        buffer:set(1, y, '+')
        buffer:set(x, y, '+')
        self.inner:render(buffer:get_sub(2, 2, x - 2, y - 2))
    end
})

local function fallback_color(i)
    local bit = function(n, b) return (n >> (b - 1)) & 1 end

    local r = bit(i, 1) * 1
    local g = bit(i, 2) * 1
    local b = bit(i, 3) * 1

    return { type = "bit", r = r, g = g, b = b }
end

---@class Widget
local MultiBar = widget:extend({
    new = function(self, values, colors)
        local t = self.super.new(self)
        setmetatable(t, self)

        t._type = "MultiBar"

        t.values = values or {}
        t.colors = colors or {}
        t.show_number = true

        t.__index = self
        return t
    end,
    render = function(self, buffer)
        local x, _ = buffer:get_size()

        if self.show_number and x > 10 then
            local sum_pct = 0
            for _, v in ipairs(self.values) do
                sum_pct = sum_pct + v
            end
            sum_pct = math.min(sum_pct, 1)
            local s = string.format("%.0f%%", sum_pct * 100)
            -- Reserve space for the number at the right
            buffer:get_sub(x - 7, 1, 7, 1):render(s)
            x = x - 8 -- reduce available width for bars
        end

        buffer:set(1, 1, '[')
        buffer:set(x, 1, ']')

        local bar_width = x - 2
        local current_pos = 2

        for i, pct in ipairs(self.values) do
            if pct > 1 then pct = 1 end
            local seg_width = math.floor(bar_width * pct)
            local color = self.colors[i] or fallback_color(i)

            for j = 0, seg_width - 1 do
                if current_pos + j <= x - 1 then
                    buffer:set(current_pos + j, 1, '|', color)
                end
            end
            current_pos = current_pos + seg_width
        end
    end,
})
MultiBar.__index = MultiBar

local function format_mem(mem)
    local units = { "B", "KB", "MB", "GB", "TB", "PB" }
    local i = 1
    while mem >= 1024 and i < #units do
        mem = mem / 1024
        i = i + 1
    end

    return string.format("%.1f %s", mem, units[i])
end

local actions = {}

local last_keys = ''
---@class Widget
local M_type = widget:extend {
    new = function(self)
        local t = self.super.new(self)
        setmetatable(t, self)
        t._type = "M_type"
        t.__index = self
        t.memory = MultiBar:new({ 0, 0, }, {
            { type = "bit", r = 0, g = 1, b = 0, },
            { type = "bit", r = 1, g = 1, b = 0, },
        })
        t.debug = false
        t.help = false
        t.debug_box = Box:new(widget:new {
            render = function(_, buffer)
                local x, y = buffer:get_size()
                local lines = {
                    string.format(" fps:    %3.f", state.fps or 0),
                    string.format(" tick:   %3.f", state.tick or 0),
                    string.format(" tdelta: %3.f", state.tdelta or 0),
                    string.format(" offset: %d", state.offset or 0),
                    string.format(" process_total: %d", state.process_total or 0),
                    string.format(" keys: %s", last_keys),
                    " show kernel: " .. tostring(state.show_kernel),
                }
                for i, line in ipairs(lines) do
                    if i > y then break end
                    buffer:get_sub(1, i, x, 1):render(line)
                end
            end
        })
        t.help_box = Box:new(widget:new {
            render = function(_, buffer)
                local x, y = buffer:get_size()
                local i = 1
                for key, data in pairs(actions) do
                    local line = string.format(" [%s] - %s", key, data.desc)
                    if i >= y then break end
                    buffer:get_sub(1, i, x, 1):render(line)
                    i = i + 1
                end
            end
        })
        return t
    end,

    render = function(self, buffer)
        local x, y = buffer:get_size()
        -- render memory
        local sub = buffer:get_sub(1, 1, x, 1)
        self.memory:render(sub)

        -- processes
        local ps = state.processes
        buffer:get_sub(1, 2, 5, 1):render('pid')
        buffer:get_sub(7, 2, 10, 1):render('mem')
        buffer:get_sub(18, 2, 12, 1):render('name')
        buffer:get_sub(31, 2, x - 30, 1):render('cmd')

        for i = 1, y - 1, 1 do
            if i + 1 >= y then break end

            local idx = state.offset + i
            if idx > state.process_total then break end


            buffer:get_sub(1, i + 2, 5, 1):render(ps[idx].pid)
            buffer:get_sub(7, i + 2, 10, 1):render(string.format("%10s", format_mem(ps[idx].mem)))
            buffer:get_sub(18, i + 2, 12, 1):render(ps[idx].name)
            buffer:get_sub(31, i + 2, x - 30, 1):render(ps[idx].cmd)
        end

        -- debug
        if state.debug then
            local w = 20
            local h = 10
            self.debug_box:render(buffer:get_sub((x - w // 2) // 2, (y - h // 2) // 2, w, h))
        end

        -- help
        if state.help then
            local w = 40
            local h = 20
            self.help_box:render(buffer:get_sub((x - w // 2) // 2, (y - h // 2) // 2, w, h))
        end
    end,

    update = function(self)
        local used = (state.total_mem - state.av_mem) / state.total_mem
        self.memory.values[1] = used
        self.memory.values[2] = state.cached_mem / state.total_mem
    end
}
M_type.__index = M_type

local M = M_type:new();

state.on_event('keypress', function(key)
    local action_entry = actions[key]
    if action_entry then
        action_entry.action()
    end

    last_keys = string.sub(last_keys .. key, -10)
end)

actions = {
    q = {
        action = function()
            state.exit = true
        end,
        desc = 'to exit'
    },

    a = {
        action = function()
            state.show_kernel = not state.show_kernel
        end,
        desc = 'toggle show kernel'
    },

    j = {
        action = function()
            state.offset = state.offset + 1

            if state.process_total - state.offset < state.heigth then
                state.offset = state.process_total - state.heigth
            end
        end,
        desc = 'increase proc offset'
    },

    G = {
        action = function()
            state.offset = 0
        end,
        desc = 'goto start of procs'
    },

    g = {
        action = function()
            state.offset = state.process_total - state.heigth
        end,
        desc = 'goto end of procs'
    },

    k = {
        action = function()
            state.offset = math.max(0, state.offset - 1)
        end,
        desc = 'decrease proc offset'
    },

    r = {
        action = function()
            state.reload = true
        end,
        desc = 'sets reload flag'
    },

    h = {
        action = function()
            state.help = not state.help
        end,
        desc = 'toggle help'
    },

    d = {
        action = function()
            state.debug = not state.debug
        end,
        desc = 'toggle debug'
    },
}

state.set_color("bg", { type = "8bit", r = 0, g = 0, b = 0 })
state.set_color("fg", { type = "bit", r = 1, g = 1, b = 1 })

return M
