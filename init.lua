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

local letters = ''

state.on_event('keypress', function(key)
    if key == 'q' then
        state.exit = true;
        return
    end
    letters = letters .. key
end)

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

    render = function(this, buffer)
        local x, _ = buffer:get_size()

        if this.show_number and x > 10 then
            local s = string.format("%.0f%%", this.percentage * 100)
            buffer:get_sub(x - 7, 1, 7, 1):render(s)
            x = x - 8
        end

        buffer:set(1, 1, '[')
        buffer:set(x, 1, ']')

        local bar_width = x - 2
        for i = 1, math.floor(bar_width * this.percentage) do
            buffer:set(i + 1, 1, '|', { type = "bit", r = 1, g = 0, b = 0 })
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
    render = function(this, buffer)
        local x, _ = buffer:get_size()

        if this.show_number and x > 10 then
            local sum_pct = 0
            for _, v in ipairs(this.values) do
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

        for i, pct in ipairs(this.values) do
            if pct > 1 then pct = 1 end
            local seg_width = math.floor(bar_width * pct)
            local color = this.colors[i] or fallback_color(i)

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
        return t
    end,

    render = function(this, buffer)
        local x, _ = buffer:get_size()
        -- render memory
        local sub = buffer:get_sub(1, 1, x, 1)
        this.memory:render(sub)
    end,

    update = function(this)
        local used = (state.total_mem - state.av_mem) / state.total_mem
        this.memory.values[1] = used
        this.memory.values[2] = state.cached_mem / state.total_mem
    end
}
M_type.__index = M_type

return M_type:new();
