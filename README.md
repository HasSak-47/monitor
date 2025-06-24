# LyTop 

**LyTop** is a terminal-based, `htop`-like system monitor for Linux, designed to be lightweight, extensible, and scriptable. It provides a live overview of memory usage and active processes and supports user-defined widgets through Lua.

## Features

- **Memory Usage Bar**  
  Displays RAM usage with multiple colored segments:
  - **Green**: Used memory
  - **Yellow**: Cached memory

- **Process Table**  
  Dynamically shows active processes sorted by memory use.

- **Overlay Widgets**  
  Toggleable help and debug boxes rendered directly in the UI.

- **Keybindings**
  - `j/k`: Scroll process list
  - `a`: Toggle kernel processes
  - `h`: Toggle help overlay
  - `d`: Toggle debug overlay
  - `q`: Quit

## Build Instructions

### Requirements

- A C++20-compatible compiler (GCC or Clang)
- Lua 5.4
- A POSIX-compatible terminal (Linux/macOS)

### Build & Run

Clone the repository and build:

```bash
git clone -b stable https://github.com/HasSak-47/monitor.git
cd monitor
make build      # Build the application
make run        # Run the default demo
```

## Lua Scripting API

Monitor is designed for Lua-driven customization. The entire interface—including widgets, rendering, and user input—is controlled from a Lua script.

### Widget Architecture

Widgets follow a simple inheritance model using metatables. Each widget can implement:

- `:new()` – Constructor  
- `:render(buffer)` – Draw the widget  
- `:update()` – Optional, for dynamic state

### Example: A Custom Bar Widget

```lua
local Bar = widget:extend({
    new = function(self)
        local t = self.super.new(self)
        setmetatable(t, self)
        t._type = "Bar"
        t.percentage = 0.5
        return t
    end,

    render = function(self, buffer)
        local x, _ = buffer:get_size()
        buffer:set(1, 1, '[')
        buffer:set(x, 1, ']')
        for i = 1, math.floor((x - 2) * self.percentage) do
            buffer:set(i + 1, 1, '|', { type = "bit", R = 1, G = 0, B = 0 })
        end
    end
})
```

### Provided Widgets

- `Bar` – Simple single-value progress bar
- `MultiBar` – Composite memory bar with multiple values/colors
- `Box` – Draws a border around another widget
- `DebugData`, `HelpData` – Optional overlays showing internal state or usage hints
- `M_type` – Default UI layout: renders memory usage and process table, handles input

## Memory Format Helpers

Memory values are automatically humanized using base-1024 formatting (e.g., 512MB, 2.1GB).

## Extending the Monitor

To create a new layout or behavior:

- Edit `init.lua` (or your custom script)
- Add or modify widgets using the provided API
- Use `state` to access system data (`fps`, `tick`, `offset`, `processes`, etc.)
- Use `state.on_event` to bind keys
