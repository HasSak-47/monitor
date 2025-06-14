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

---@class Unit
---@field chr string
---@field color Color

---@class Buffer
---@field cell Unit[][]

---@class Widget
---@field type string
---@field style string
---@field render function

local M = Widget:new();
local Bar = Widget:new();

function Bar:render(buffer)
    --- ...
end

function M:render(buffer)
    local b = Bar:new()
    b.render(buffer.sub(0, 0, 10, 1))
    --- ...
end

return M
