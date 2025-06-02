#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <lua.hpp>

#include <memory>
#include <thread>

#include <termios.h>
#include <unistd.h>

#include <render/buffer.hpp>
#include "render/system_rend.hpp"
#include "render/widgets.hpp"
#include "render/window.hpp"
#include "system.hpp"

lua_State* L = nullptr;

bool got_original = false;

struct termios orig_termios = {};

void unset_raw_mode() {
    if (got_original)
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void set_raw_mode() {
    if (!got_original) {
        tcgetattr(STDIN_FILENO, &orig_termios);
        atexit(unset_raw_mode);
        got_original = true;
    }

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
    L = luaL_newstate();
    luaL_requiref(L, "base", luaopen_base, true);
    luaL_requiref(L, "math", luaopen_math, true);
    luaL_requiref(L, "table", luaopen_table, true);
    luaL_requiref(L, "package", luaopen_package, true);
    luaL_requiref(L, "string", luaopen_string, true);

    using namespace std::chrono;
    using namespace ly;

    printf("\e[?1049h"); // enter alternate screen
    printf("\e[0;0H");   // move cursor to 0
    printf("\e[?25l");   // hide cursor
    fflush(stdout);
    set_raw_mode();

    constexpr auto frame_duration = 16.6666ms;
    render::Window win;
    auto box_buffer = win.get_subbuf(0, 0, 30, 3);

    float fps    = 0;
    int offset   = 0;
    size_t frame = 0;

    render::widgets::Box debug_box(Sys::sys._max_mem);

    int c = 0;
    while (true) {
        auto t_start = high_resolution_clock::now();
        printf("\e[0;0H");

        auto processes = Sys::sys.get_processes();

        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n > 0) {
            if (c == 'q')
                break;
            switch (c) {
            case 'j':
                offset = std::min(
                    offset + 1, (int)processes.size());
                break;
            case 'k':
                offset = std::max(
                    (int64_t)offset - 1, (int64_t)0);
                break;
            }
        }

        auto p = Sys::sys.get_processes();
        std::sort(p.begin(), p.end(), [](auto a, auto b) {
            return a.total() > b.total();
        });
        win.get_subbuf(0, 3, win.width(), win.height() - 3)
            .render_widget(render::Process(p, offset));
        box_buffer.render_widget(debug_box);
        win.render();

        // frame rate calc
        auto t_now = high_resolution_clock::now();
        auto delta = t_now - t_start;

        // wait for end of frame
        if (delta < frame_duration)
            std::this_thread::sleep_for(
                frame_duration - delta);

        // Optional: print elapsed time for debugging
        auto t_end = high_resolution_clock::now();
        auto total_elapsed =
            duration_cast<milliseconds>(t_end - t_start);
        fps = 1000.0 / total_elapsed.count();
        frame++;
    }

    lua_close(L);
    unset_raw_mode();
    printf("\e[?1049l"); // leave alternate screen
    printf("\e[?25h");   // show cursor
    fflush(stdout);
    return 0;
}
