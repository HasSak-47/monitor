#include <lua.hpp>

#include <stdlib.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

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
    raw.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
    L = luaL_newstate();
    luaL_requiref(L, "base", luaopen_base, true);
    luaL_requiref(L, "math", luaopen_math, true);
    luaL_requiref(L, "table", luaopen_table, true);
    luaL_requiref(L, "package", luaopen_package, true);
    luaL_requiref(L, "string", luaopen_string, true);

    set_raw_mode();
    using namespace std::chrono;

    std::this_thread::sleep_for(1s);

    unset_raw_mode();

    lua_close(L);
    return 0;
}
