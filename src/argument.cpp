#include <ly/argument.hpp>
 
using namespace ly;

/*

Arg::Arg(char* input) {
    char* end;

    errno          = 0;
    int64_t ivalue = std::strtoll(input, &end, 10);
    if (errno == 0 && *end == '\0') {
        type   = Int;
        data.i = ivalue;
        return;
    }

    errno           = 0;
    uint64_t uvalue = std::strtoull(input, &end, 10);
    if (errno == 0 && *end == '\0') {
        type   = Uint;
        data.u = uvalue;
        return;
    }

    errno        = 0;
    float fvalue = std::strtof(input, &end);
    if (errno == 0 && *end == '\0') {
        type   = Float;
        data.f = fvalue;
        return;
    }

    type = String;
    new (&data.s) std::string(input);
}

Arg::~Arg() {
    if (type == String) {
        data.s.~basic_string();
    }
}

*/
