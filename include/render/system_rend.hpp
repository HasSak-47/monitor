#ifndef __RENDER_SYSTEM_HPP__
#define __RENDER_SYSTEM_HPP__
#include <algorithm>
#include <vector>

#include "../system.hpp"
#include "./widgets.hpp"
#include "render/buffer.hpp"

namespace ly::render::sys {

class Memory : public widgets::Widget {
private:
    std::vector<float>& _pers;
    // used buffer cached
    const std::vector<Color<u8>> _cols = {
        {
         GREEN_U8, BLUE_U8,
         YELLOW_U8, },
    };

public:
    Memory(std::vector<float>& pers) : _pers(pers) {}
    ~Memory() override {}
    void render(Buffer& buf) const override {
        buf.render_widget(
            widgets::MultiBar(this->_pers, this->_cols));
    }
};

class Process : public widgets::Widget {
    const Sys::Processes& _procs;
    size_t _offset;

public:
    Process(const Sys::Processes& procs, size_t offset = 0)
        : _procs(procs), _offset(offset) {}
    ~Process() override {}

    void render(Buffer& buf) const override {

        std::vector<const Sys::Process*> procs = {};
        for (const auto& [_, proc] : this->_procs) {
            procs.push_back(&proc);
        }

        auto val = std::find_if(procs.begin(), procs.end(),
            [](const auto& a) { return a->is_kernel(); });
        while (val != procs.end()) {
            procs.erase(val);
            val = std::find_if(
                val, procs.end(), [](const auto& a) {
                    return a->is_kernel();
                });
        }

        std::sort(procs.begin(), procs.end(),
            [](auto& a, auto& b) -> bool {
                return a->total() > b->total();
            });

        size_t end = std::min(
            buf.height(), procs.size() - this->_offset);
        for (size_t i = 0; i < end; ++i) {
        }
    }
};

} // namespace ly::render::sys

#endif
