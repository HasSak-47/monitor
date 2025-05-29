#ifndef __RENDER_WINDOW_HPP__
#define __RENDER_WINDOW_HPP__

#include <ly/int.hpp>
#include <render/buffer.hpp>

namespace ly::render {
class Window {
    Buffer _back;

public:
    Window();
    ~Window();

    Buffer get_subbuf(
        size_t x, size_t y, size_t w, size_t h);
    void render();
};
} // namespace ly::render

#endif
