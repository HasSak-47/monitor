#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <lua.hpp>

#include <thread>

#include <termios.h>
#include <unistd.h>

#include <render/buffer.hpp>
#include <render/system_rend.hpp>
#include <render/widgets.hpp>
#include <render/window.hpp>
#include <system/system.hpp>
#include "render/lua_bindings.hpp"

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

/*
int main(int argc, char* argv[]) {
    ly::render::lua::lua_init();
    ly::render::Buffer test_buf(10, 10);

    lua_pushlightuserdata(ly::render::lua::L, &test_buf);
    luaL_getmetatable(ly::render::lua::L, "Buffer");
    lua_setmetatable(ly::render::lua::L, -2);
    lua_setglobal(ly::render::lua::L, "buffer");

    if (luaL_dofile(ly::render::lua::L, "./test.lua") !=
        LUA_OK) {
        std::cerr << "Lua Error: "
                  << lua_tostring(ly::render::lua::L, -1)
                  << '\n';
        lua_pop(ly::render::lua::L, 1);
    }

    std::cout << test_buf;
}
*/
int main(int argc, char* argv[]) {
    using namespace std::chrono;
    using namespace ly;

    printf("\e[?1049h"); // enter alternate screen
    printf("\e[0;0H");   // move cursor to 0
    printf("\e[?25l");   // hide cursor
    fflush(stdout);
    set_raw_mode();

    constexpr auto frame_duration = 16.6666ms;
    render::Window win;

    render::sys::Memory mem;
    render::sys::Processes procs(0);

    render::widgets::Box debug_box(procs.get_offset());
    int c = 0;
    while (true) {
        auto t_start = high_resolution_clock::now();
        printf("\e[0;0H"); // return cursor to 0,0

        // input processing
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n > 0) {
            if (c == 'q')
                break;
            switch (c) {
            case 'j':
                procs.inc_offset();
                break;
            case 'k':
                procs.dec_offset();
                break;
            case 'm':
                procs.sort =
                    render::sys::Processes::Sort::Mem;
                break;
            case 'p':
                procs.sort =
                    render::sys::Processes::Sort::Pid;
                break;
            case 'n':
                procs.sort =
                    render::sys::Processes::Sort::Name;
                break;
            case 'g':
                procs.goto_beg();
                break;
            case 'G':
                procs.goto_end();
                break;
            }
        }

        mem.update();

        win.get_subbuf(0, 0, win.width(), 1)
            .render_widget(mem);
        win.get_subbuf(0, 1, win.width(), win.height() - 1)
            .render_widget(procs);
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
        // fps = 1000.0 / total_elapsed.count();
        // frame++;
    }

    unset_raw_mode();
    printf("\e[?1049l"); // leave alternate screen
    printf("\e[?25h");   // show cursor
    fflush(stdout);
    return 0;
}
