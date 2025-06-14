#include <algorithm>
#include <iomanip>
#include <render/widgets.hpp>
#include <sstream>

using namespace ly::render;
using namespace ly::render::widgets;

void Block::render(Buffer& buf, size_t tick) const {
    if (buf.width() < 2 || buf.height() < 2) {
        return;
    }
    size_t w = buf.width() - 1;
    size_t h = buf.height() - 1;

    for (size_t i = 1; i < w; ++i) {
        buf.get(i, 0).chr = '-';
        buf.get(i, h).chr = '-';
    }

    for (size_t j = 1; j < h; ++j) {
        buf.get(0, j).chr = '|';
        buf.get(w, j).chr = '|';
    }

    buf.get(0, 0).chr = '+';
    buf.get(0, h).chr = '+';
    buf.get(w, 0).chr = '+';
    buf.get(w, h).chr = '+';
}

void Text::render(Buffer& buf, size_t tick) const {
    size_t max = std::min(buf.width(), this->_text.size());
    for (size_t i = 0; i < max; ++i) {
        buf.get(i, 0).chr = this->_text[i];
        buf.get(i, 0).col = this->_col;
    }
}

void Bar::render(Buffer& buf, size_t tick) const {
    size_t end = buf.width();
    if (this->_show && buf.width() > 7) {
        constexpr size_t sub_width = 10;
        size_t x = buf.width() - sub_width;

        auto sub = buf.get_sub_buffer(x, 0, sub_width, 1);

        // Format the percentage string
        std::ostringstream ss_raw;
        ss_raw << std::fixed << std::setprecision(1)
               << this->_val * 100 << '%';
        std::string str = ss_raw.str();

        // Truncate from the left if string is too long
        if (str.length() > sub_width)
            str = str.substr(str.length() - sub_width);

        // Right-align the string
        std::ostringstream ss;
        ss << std::setw(sub_width) << str;

        sub.render_widget(ss.str());
        end -= sub_width;
    }

    buf.get(0, 0).chr   = '[';
    buf.get(end, 0).chr = ']';

    size_t len = (end - 1);
    size_t w   = len * std::min(1.0f, this->_val);

    for (size_t i = 0; i < w; ++i) {
        buf.get(1 + i, 0).chr = '|';
    }
}

void MultiBar::render(Buffer& buf, size_t tick) const {
    size_t end = buf.width();

    if (this->_show && buf.width() > 7) {
        constexpr size_t sub_width = 10;
        size_t x = buf.width() - sub_width;

        auto sub  = buf.get_sub_buffer(x, 0, sub_width, 1);
        float val = 0.;
        for (const auto& v : this->_val) {
            val += v;
        }

        // Format the percentage string
        std::ostringstream ss_raw;
        ss_raw << std::fixed << std::setprecision(1)
               << val * 100 << '%';
        std::string str = ss_raw.str();

        // Truncate from the left if string is too long
        if (str.length() > sub_width)
            str = str.substr(str.length() - sub_width);

        // Right-align the string
        std::ostringstream ss;
        ss << std::setw(sub_width) << str;

        sub.render_widget(ss.str());
        end -= sub_width;
    }

    buf.get(0, 0).chr   = '[';
    buf.get(end, 0).chr = ']';

    size_t len = (end - 1);

    std::vector<size_t> w(this->_val.size());
    size_t last = 1;
    for (size_t j = 0; j < this->_val.size(); ++j) {
        size_t w = len * this->_val[j];
        for (size_t i = 0; i < w; ++i) {
            buf.get(i + last, 0).chr = '|';
            buf.get(i + last, 0).col = this->_cols[j];
        }
        last += w;
    }
}
