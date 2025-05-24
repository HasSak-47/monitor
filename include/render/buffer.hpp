#ifndef __RENDER_UTILS_HPP__
#define __RENDER_UTILS_HPP__

#include <ly/int.hpp>

#include <memory>
#include <vector>

namespace ly::render {

template <typename T>
class Color {
    T r = {}, g = {}, b = {};
};

struct Unit {
    int chr = 0;
    Color<u8> col;
};

class Buffer {
private:
    std::vector<std::vector<std::shared_ptr<Unit>>> _data;
    size_t _w, _h;

public:
    Buffer(size_t w, size_t h);
    Buffer(const Buffer& other);

    Buffer get_sub_buffer(
        size_t x, size_t y, size_t w, size_t h);

    Unit& get(size_t x, size_t y);

    ~Buffer();
};

} // namespace ly::render

#endif
