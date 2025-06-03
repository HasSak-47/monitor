#ifndef __RENDER_WIDGETS_HPP__
#define __RENDER_WIDGETS_HPP__

#include "render/buffer.hpp"

namespace ly::render::widgets {

class Widget {
public:
    virtual ~Widget() {};
    virtual void render(Buffer& buffer) const = 0;
};

} // namespace ly::render::widgets

namespace ly::render {
inline void render(
    Buffer& buf, const widgets::Widget& widget) {
    widget.render(buf);
}
} // namespace ly::render

namespace ly::render::widgets {
template <typename T>
concept Encapsulable =
    Renderable<T> || OstreamFormattable<T> ||
    std::derived_from<T, Widget>;

class Block : public Widget {
public:
    Block() {}
    ~Block() {}

    void render(Buffer& buffer) const override;
};

template <typename T>
    requires Encapsulable<T>
class Box : public Widget {
private:
    T& _capture;

public:
    ~Box() {}
    Box(T& capture) : _capture(capture) {}

    void render(Buffer& buf) const override {
        buf.render_widget(Block());
        if (buf.width() < 3 || buf.height() < 3) {
            return;
        }

        auto sub = buf.get_sub_buffer(
            1, 1, buf.width() - 2, buf.height() - 2);
        sub.render_widget(this->_capture);
    }
};

class Bar : public Widget {
private:
    const float& _val;
    const bool _show;

public:
    Bar(float& val, bool show = false)
        : _val(val), _show(show) {}
    ~Bar() {}

    void render(Buffer& buf) const override;
};

class MultiBar : public Widget {
private:
    const std::vector<float>& _val;
    const std::vector<Color<u8>>& _cols;

public:
    MultiBar(const std::vector<float>& val,
        const std::vector<Color<u8>>& cols)
        : _val(val), _cols(cols) {}

    ~MultiBar() {}

    void render(Buffer& buf) const override;
};

} // namespace ly::render::widgets

#endif
