#ifndef __RENDER_WINDOW_HPP__
#define __RENDER_WINDOW_HPP__

#include <ly/int.hpp>
#include <render/buffer.hpp>

namespace ly::render {
class Window {
    Buffer _back;
    size_t _width, _height;

public:
    Window();
    ~Window();

    Buffer get_subbuf(
        size_t x, size_t y, size_t w, size_t h);
    Buffer& get_buf() { return this->_back; }

    void render();
    const size_t width() const { return this->_width; }
    const size_t height() const { return this->_height; }
};
} // namespace ly::render

#endif
