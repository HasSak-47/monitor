#ifndef __RENDER_SYSTEM_HPP__
#define __RENDER_SYSTEM_HPP__
#include <algorithm>
#include <functional>
#include <sys/types.h>
#include <utility>
#include <vector>

#include "../system.hpp"
#include "./widgets.hpp"
#include "render/buffer.hpp"

namespace ly::render::sys {

class Memory : public widgets::Widget {
private:
    std::vector<float> _pers;
    // used buffer cached
    const std::vector<Color<u8>> _cols = {
        {
         GREEN_U8, BLUE_U8,
         YELLOW_U8, },
    };

public:
    Memory() : _pers(3) {}
    ~Memory() override {}

    void update() override {
        float max = Sys::sys._max_mem;

        this->_pers[0] =
            (Sys::sys._max_mem - Sys::sys._av_mem) /
            max; // used
        this->_pers[1] =
            Sys::sys._buffer_mem / max; // buffered
        this->_pers[2] =
            Sys::sys._cached_mem / max; // cached
    }

    void render(Buffer& buf) const override {
        buf.render_widget(
            widgets::MultiBar(this->_pers, this->_cols));
    }
};

class Process : public widgets::Widget {
private:
    const Sys::Process& _proc;

public:
    Process(const Sys::Process& proc) : _proc(proc) {}

    void render(Buffer& buffer) const override {
        buffer.get_sub_buffer(0, 0, 6, 1)
            .render_widget(this->_proc._stat.pid);
        buffer.get_sub_buffer(6, 0, 15, 1)
            .render_widget(widgets::Text(
                this->_proc._stat.name, GREEN_U8));
        // TODO: add here proc cmd ig
        // size_t w = buffer.width() - 21;
    }
};

class Processes : public widgets::Widget {
private:
    size_t _offset;
    bool _kernel;

    using _Sorter = std::function<bool(
        const Sys::Process*&, const Sys::Process*&)>;

    static inline bool _sort_name(
        const Sys::Process*& a, const Sys::Process*& b) {
        return a->_stat.name < b->_stat.name;
    }

    static inline bool _sort_pid(
        const Sys::Process*& a, const Sys::Process*& b) {
        return a->_stat.pid < b->_stat.pid;
    }

    static inline bool _sort_mem(
        const Sys::Process*& a, const Sys::Process*& b) {
        return a->total() > b->total();
    }

    const _Sorter _get_sorter() const {
        switch (this->sort) {
        case Processes::Sort::Name:
            return _sort_name;
        case Processes::Sort::Mem:
            return _sort_mem;
        case Processes::Sort::Pid:
            return _sort_pid;
        }
        std::unreachable();
    }

public:
    enum class Sort {
        Name,
        Mem,
        Pid,
    } sort = Sort::Mem;

    Processes(size_t offset, bool kernel = false)
        : _offset(offset), _kernel(kernel) {}
    ~Processes() {}

    void inc_offset() {
        // should check for only non kernel processes if
        // this->_kernel = false;
        auto& procs = Sys::sys.get_processes();
        if (_offset + 1 < procs.size()) {
            _offset += 1;
        }
    }
    void dec_offset() {
        if (_offset > 0)
            _offset -= 1;
    }

    const size_t& get_offset() const {
        return this->_offset;
    }

    void goto_end() {
        _offset = Sys::sys.get_processes().size();
    }
    void goto_beg() { _offset = 0; }

    void render(Buffer& buf) const override {
        std::vector<const Sys::Process*> procs = {};
        for (const auto& [_, proc] :
            Sys::sys.get_processes()) {
            procs.push_back(&proc);
        }

        auto val = std::find_if(procs.begin(), procs.end(),
            [](const auto& a) { return a->is_kernel(); });

        if (!this->_kernel) {
            while (val != procs.end()) {
                procs.erase(val);
                val = std::find_if(
                    val, procs.end(), [](const auto& a) {
                        return a->is_kernel();
                    });
            }
        }

        std::sort(procs.begin(), procs.end(),
            this->_get_sorter());

        size_t end = std::min(
            buf.height(), procs.size() - this->_offset);

        if (this->_offset >= procs.size()) {
            return;
        }
        for (size_t i = 0; i < end; ++i) {
            buf.get_sub_buffer(0, i, buf.width(), 1)
                .render_widget(
                    Process(*procs[i + this->_offset]));
        }
    }
};

} // namespace ly::render::sys

#endif
