#include <memory>
#include <render/buffer.hpp>
#include <vector>

using namespace ly::render;

using SharedUnit = std::shared_ptr<Unit>;

typedef SharedUnit (*DataIniter)(size_t x, size_t y);
typedef SharedUnit (*EmptyIniter)();

std::vector<std::vector<SharedUnit>> init_buffer(
    size_t w, size_t h, DataIniter initer) {
    std::vector<std::vector<SharedUnit>> buf = {};
    buf.resize(w);
    for (size_t i = 0; i < w; ++i) {
        std::vector<std::shared_ptr<Unit>> col = {};
        for (size_t j = 0; j < h; ++j)
            col.push_back(initer(i, j));

        buf[i] = col;
    }

    return buf;
}

std::vector<std::vector<SharedUnit>> init_buffer(
    size_t w, size_t h, EmptyIniter initer) {
    std::vector<std::vector<SharedUnit>> buf = {};
    buf.resize(w);
    for (size_t i = 0; i < w; ++i) {
        std::vector<std::shared_ptr<Unit>> col = {};
        for (size_t j = 0; j < h; ++j)
            col.push_back(initer());

        buf[i] = col;
    }

    return buf;
}

Buffer::Buffer(size_t w, size_t h) : _w(w), _h(h) {
    this->_data = init_buffer(
        w, h, []() { return std::make_shared<Unit>(); });
}

Buffer Buffer::get_sub_buffer(
    size_t x, size_t y, size_t w, size_t h) {
    return init_buffer(w, h, [&](size_t i, size_t j) {
        return this->_data[x + i][y + j];
    });
}
