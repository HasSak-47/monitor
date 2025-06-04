#include <functional>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>

#include <render/buffer.hpp>

namespace ly::render {
const ConsoleColor ConsoleColor::WHITE =
    ConsoleColor(ConsoleColor::_ColorUnion::_BitCol::WHITE);
const ConsoleColor ConsoleColor::RED =
    ConsoleColor(ConsoleColor::_ColorUnion::_BitCol::RED);
const ConsoleColor ConsoleColor::YELLOW = ConsoleColor(
    ConsoleColor::_ColorUnion::_BitCol::YELLOW);
const ConsoleColor ConsoleColor::GREEN =
    ConsoleColor(ConsoleColor::_ColorUnion::_BitCol::GREEN);
const ConsoleColor ConsoleColor::PURPLE = ConsoleColor(
    ConsoleColor::_ColorUnion::_BitCol::PURPLE);
const ConsoleColor ConsoleColor::CYAN =
    ConsoleColor(ConsoleColor::_ColorUnion::_BitCol::CYAN);
const ConsoleColor ConsoleColor::BLUE =
    ConsoleColor(ConsoleColor::_ColorUnion::_BitCol::BLUE);
} // namespace ly::render

using namespace ly::render;

bool ConsoleColor::operator==(
    const ConsoleColor& other) const {
    if (this->_ty != other._ty) {
        return false;
    }

    if (this->_ty == ConsoleColor::Bit) {
        return this->dt.bit_col == other.dt.bit_col;
    }
    return this->dt.true_col == other.dt.true_col;
}

void ConsoleColor::display() {
    switch (this->_ty) {
    case Bit:
        printf("\e[3%dm", (int)this->dt.bit_col);
        break;
    case TrueColor:
        printf("\e[38;2;%d;%d;%dm", this->dt.true_col.r,
            this->dt.true_col.g, this->dt.true_col.b);
        break;
    }
}

Unit::Unit() : col(ConsoleColor::WHITE) {}

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
