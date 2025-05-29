#include <cstdio>
#include <sys/ioctl.h>
#include <unistd.h>

#include <render/window.hpp>
#include "render/buffer.hpp"

using namespace ly::render;

static Buffer _back_gen() {
    struct winsize w;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &w);

    Buffer buf(w.ws_row, w.ws_col);
    return buf;
}

Window::Window() : _back(_back_gen()) {}
Window::~Window() {}

void Window::render() {
    Color<u8> last = {0xff, 0xff, 0xff};
    for (size_t j = 0; j < this->_back.height(); ++j) {
        for (size_t i = 0; i < this->_back.width(); ++i) {
            auto& cur = this->_back.get(i, j);
            if (cur.col != last) {
                last = cur.col;
                printf("\e[38;2;%d;%d;%dm", last.r, last.g,
                    last.b);
            }

            putchar(cur.chr);
        }
    }
}

Buffer Window::get_subbuf(
    size_t x, size_t y, size_t w, size_t h) {
    return this->_back.get_sub_buffer(x, y, w, h);
}
