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
local DebugBox = widget:extend({
    new = function(self)
        local t = self.super.new(self)
        setmetatable(t, self)

        t._type = "DebugBox"
        t.__index = self

        return t
    end,

    render = function(self, buffer)
        local x, y = buffer:get_size()
        local lines = {
            string.format("fps:    %3.f", state.fps or 0),
            string.format("tick:   %3.f", state.tick or 0),
            string.format("tdelta: %3.f", state.tdelta or 0),
            string.format("offset: %d", state.offset or 0),
            string.format("process_total: %d", state.process_total or 0),
        }

        local width = 30
        local height = #lines + 2 -- 1 line padding top and bottom

        -- Clamp to buffer
        width = math.min(width, x)
        height = math.min(height, y)

        -- Draw border
        for i = 2, width - 1 do
            buffer:set(i, 1, '-')      -- Top
            buffer:set(i, height, '-') -- Bottom
        end
        for j = 2, height - 1 do
            buffer:set(1, j, '|')     -- Left
            buffer:set(width, j, '|') -- Right
        end
        buffer:set(1, 1, '+')
        buffer:set(width, 1, '+')
        buffer:set(1, height, '+')
        buffer:set(width, height, '+')

        -- Draw text inside border
        for i, line in ipairs(lines) do
            if i + 1 >= y then
                break
            end
            buffer:get_sub(2, i + 1, width - 2, 1):render(line)
        end
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
    return string.format("%.1f%s", mem, units[i])
end

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
        t.debug_box = DebugBox:new()
        return t
    end,

    render = function(self, buffer)
        local x, y = buffer:get_size()
        -- render memory
        local sub = buffer:get_sub(1, 1, x, 1)
        self.memory:render(sub)

        -- debug
        if self.debug then
            self.debug_box:render(buffer:get_sub(x // 2, y // 2, 20, 20))
        end
        local ps = state.processes
        for i, p in ipairs(ps) do
            if i + 1 >= y then break end
            buffer:get_sub(1, i + 1, 8, 1):render(p.pid)
            buffer:get_sub(8, i + 1, x - 20, 1):render(p.name)
            buffer:get_sub(30, i + 1, x - 30, 1):render(format_mem(p.mem))
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
    if key == 'q' then
        state.exit = true;
        return
    elseif key == 'j' then
        state.offset = state.offset + 1
    elseif key == 'k' then
        state.offset = state.offset - 1
    elseif key == 'd' then
        M.debug = not M.debug
    end
end)

return M
