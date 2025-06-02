#ifndef __RENDER_SYSTEM_HPP__
#define __RENDER_SYSTEM_HPP__
#include <algorithm>
#include <vector>
#include "../system.hpp"
#include "./widgets.hpp"
#include "render/buffer.hpp"

namespace ly::render {

class Process : public widgets::Widget {
    const std::vector<Sys::Process>& _procs;
    size_t _offset;

public:
    Process(const std::vector<Sys::Process>& procs,
        size_t offset = 0)
        : _procs(procs), _offset(offset) {}
    ~Process() override {}

    void render(Buffer& buf) const override {
        size_t end = std::min(buf.height(),
            this->_procs.size() - this->_offset);
        for (size_t i = 0; i < end; ++i) {
            size_t idx = i + this->_offset;
            float val  = this->_procs[idx].total() /
                        (float)Sys::sys._max_mem;

            buf.get_sub_buffer(0, i, 20, 1)
                .render_widget(
                    this->_procs[idx]._stat.name);
            buf.get_sub_buffer(20, i, 20, 1)
                .render_widget(this->_procs[idx].total());
        }
    }
};

} // namespace ly::render

#endif
