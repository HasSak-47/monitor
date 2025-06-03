#include <functional>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>

#include <render/buffer.hpp>

namespace ly::render {
const Color<ly::u8> BLACK_U8 = {
    0x00,
    0x00,
    0x00,
};
const Color<ly::u8> RED_U8 = {
    0xff,
    0x00,
    0x00,
};
const Color<ly::u8> GREEN_U8 = {
    0x00,
    0xff,
    0x00,
};
const Color<ly::u8> YELLOW_U8 = {
    0xff,
    0xff,
    0x00,
};
const Color<ly::u8> BLUE_U8 = {
    0x00,
    0x00,
    0xff,
};
const Color<ly::u8> PURPLE_U8 = {
    0xff,
    0x00,
    0xff,
};
const Color<ly::u8> CYAN_U8 = {
    0xff,
    0x00,
    0xff,
};
const Color<ly::u8> WHITE_U8 = {
    0xff,
    0xff,
    0xff,
};
} // namespace ly::render

using namespace ly::render;

Unit::Unit() : col({0xff, 0xff, 0xff}) {}

using SharedUnit = std::shared_ptr<Unit>;

using DataIniter =
    std::function<SharedUnit(size_t x, size_t y)>;

using EmptyIniter = std::function<SharedUnit()>;

std::vector<std::vector<SharedUnit>> init_buffer(
    size_t w, size_t h, DataIniter initer) {

    std::vector<std::vector<SharedUnit>> buf(w);
    for (size_t i = 0; i < w; ++i) {
        for (size_t j = 0; j < h; ++j)
            buf[i].push_back(initer(i, j));
    }

    return buf;
}

std::vector<std::vector<SharedUnit>> init_buffer(
    size_t w, size_t h, EmptyIniter initer) {
    std::vector<std::vector<SharedUnit>> buf = {};
    buf.resize(w);
    for (size_t i = 0; i < w; ++i) {
        std::vector<SharedUnit> col = {};

        for (size_t j = 0; j < h; ++j)
            col.push_back(initer());

        buf[i] = col;
    }

    return buf;
}

Buffer::Buffer(size_t w, size_t h, _Buffer buf)
    : _w(w), _h(h) {
    this->_data = buf;
}

Buffer::~Buffer() {}

Buffer::Buffer(size_t w, size_t h) : _w(w), _h(h) {
    this->_data = init_buffer(
        w, h, []() { return std::make_shared<Unit>(); });
}

Buffer::Buffer(const Buffer& other)
    : _w(other._w), _h(other._h) {
    this->_data = other._data;
}

Buffer::Buffer(Buffer&& other)
    : _w(other._w), _h(other._h) {
    this->_data = std::move(other._data);
}

Buffer Buffer::get_sub_buffer(
    size_t x, size_t y, size_t w, size_t h) {
    auto buf = init_buffer(w, h, [&](size_t i, size_t j) {
        return this->_data[x + i][y + j];
    });

    return Buffer(w, h, buf);
}

Unit& Buffer::get(size_t x, size_t y) {
    if (this->width() <= x)
        x = this->width() - 1;
    if (this->height() <= y)
        y = this->height() - 1;

    return *this->_data[x][y];
}

std::ostream& operator<<(
    std::ostream& os, const Buffer& buf) {
    for (size_t j = 0; j < buf._h; ++j) {
        for (size_t i = 0; i < buf._w; ++i) {
            os << (char)buf._data[i][j]->chr;
        }
        os << '\n';
    }
    return os;
}

const size_t Buffer::width() const {
    return this->_w;
}
const size_t Buffer::height() const {
    return this->_h;
};
