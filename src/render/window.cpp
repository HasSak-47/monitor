#include <bits/types/wint_t.h>
#include <cstdio>
#include <sys/ioctl.h>
#include <unistd.h>

#include <render/window.hpp>
#include "render/buffer.hpp"

using namespace ly::render;

static Buffer _back_gen() {
    struct winsize w;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &w);

    Buffer buf(w.ws_col, w.ws_row);
    return buf;
}

Window::Window() : _back(_back_gen()) {
    struct winsize w;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &w);

    this->_width  = w.ws_col;
    this->_height = w.ws_row;
}

Window::~Window() {}

void Window::render() {
    ConsoleColor last = ConsoleColor::WHITE;
    last.display();
    char buf[5] = {};
    for (size_t j = 0; j < this->_back.height(); ++j) {
        for (size_t i = 0; i < this->_back.width(); ++i) {
            auto& cur = this->_back.get(i, j);
            if (cur.col != last) {
                last = cur.col;
                last.display();
            }

            printf("%lc", cur.chr);

            // reset buffer
            cur.chr = ' ';
            cur.col = ConsoleColor::WHITE;
        }
    }
    printf("\e[38;2;%d;%d;%dm", 255, 255, 255);
}

Buffer Window::get_subbuf(
    size_t x, size_t y, size_t w, size_t h) {
    return this->_back.get_sub_buffer(x, y, w, h);
}
