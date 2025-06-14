#ifndef __RENDER_BUFFER_HPP__
#define __RENDER_BUFFER_HPP__

#include <concepts>
#include <iostream>
#include <ly/int.hpp>

#include <memory>
#include <sstream>
#include <vector>

namespace ly::render {
class Buffer;
}

std::ostream& operator<<(
    std::ostream& other, const ly::render::Buffer& buf);

namespace ly::render {

template <typename T>
struct Color {
    T r = {}, g = {}, b = {};
};

struct ConsoleColor {
private:
    enum { Bit, TrueColor } _ty;
    union _ColorUnion {
        enum class _BitCol : int32_t {
            BLACK  = 0b000,
            RED    = 0b001,
            GREEN  = 0b010,
            YELLOW = 0b011,
            BLUE   = 0b100,
            PURPLE = 0b101,
            CYAN   = 0b110,
            WHITE  = 0b111,
        } bit_col;
        Color<u8> true_col;

        _ColorUnion(_BitCol col) { this->bit_col = col; }
        _ColorUnion(Color<u8> col) { this->true_col = col; }
    } dt;

    ConsoleColor(ConsoleColor::_ColorUnion::_BitCol col)
        : _ty(Bit), dt(col) {};

public:
    ConsoleColor(Color<u8> col)
        : _ty(TrueColor), dt(col) {};

    ConsoleColor(int col)
        : _ty(Bit), dt((_ColorUnion::_BitCol)col) {};

    bool operator==(const ConsoleColor& other) const;
    void display();

    static const ConsoleColor WHITE;
    static const ConsoleColor RED;
    static const ConsoleColor YELLOW;
    static const ConsoleColor GREEN;
    static const ConsoleColor PURPLE;
    static const ConsoleColor CYAN;
    static const ConsoleColor BLUE;
};

struct Unit {
    char32_t chr = ' ';
    ConsoleColor col;
    Unit();
};

class Buffer;
void render(Buffer& buf);

template <typename T>
concept OstreamFormattable =
    requires(std::ostream& os, const T& widget) {
        { os << widget } -> std::same_as<std::ostream&>;
    };

template <typename T>
concept Renderable =
    requires(ly::render::Buffer& buf, const T& widget) {
        { render(buf, widget) } -> std::same_as<void>;
    };

class Buffer {
private:
    using _Buffer =
        std::vector<std::vector<std::shared_ptr<Unit>>>;
    _Buffer _data;
    size_t _w, _h;

    Buffer(size_t w, size_t h, _Buffer buf);

public:
    Buffer(size_t w, size_t h);
    Buffer(const Buffer& other);
    Buffer(Buffer&& other);

    ~Buffer();

    Buffer get_sub_buffer(
        size_t x, size_t y, size_t w, size_t h);

    Unit& get(size_t x, size_t y);
    friend std::ostream& ::operator<<(
        std::ostream& other, const Buffer& buf);

    const size_t width() const;
    const size_t height() const;

    template <typename T>
        requires Renderable<T>
    void render_widget(const T& widget) {
        render(*this, widget);
    }

    template <typename T>
        requires(!Renderable<T> && OstreamFormattable<T>)
    void render_widget(const T& widget) {
        std::ostringstream ss;
        ss << widget;
        auto s     = ss.str();
        size_t idx = 0;
        for (const auto& c : s) {
            size_t i = idx % this->_w;
            size_t j = idx / this->_w;
            if (i >= this->_w) {
                continue;
            }
            if (j >= this->_h) {
                continue;
            }

            this->get(i, j).chr = c;
            this->get(i, j).col = ConsoleColor::WHITE;
            idx++;
        }
    }
};

} // namespace ly::render

template <typename T>
inline bool operator==(const ly::render::Color<T>& a,
    const ly::render::Color<T>& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

template <typename T>
inline bool operator!=(const ly::render::Color<T>& a,
    const ly::render::Color<T>& b) {
    return !(a == b);
}

#endif
