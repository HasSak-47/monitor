#ifndef __LY_ARGUMENT_HPP__
#define __LY_ARGUMENT_HPP__

#include <cstdint>
#include <string>
#include <vector>

namespace ly {
class Arg {
private:
    enum Type {
        String,
        Int,
        Uint,
        Float,
    } type;

    union Data {
        std::string s;
        int64_t i;
        uint64_t u;
        float f;

        Data() {}
        ~Data() {}
    } data;

public:
    Arg(char* input);

    ~Arg();
};

std::vector<Arg> parse_args();

} // namespace ly

#endif // !__LY_ARGUMENT_HPP__
