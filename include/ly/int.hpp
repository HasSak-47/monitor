#ifndef __LY_INT_HPP__
#define __LY_INT_HPP__

#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace ly {
#define MAKE_INT(p)                                        \
    typedef int##p##_t i##p;                               \
    typedef uint##p##_t u##p;

MAKE_INT(8);
MAKE_INT(16);
MAKE_INT(32);
MAKE_INT(64);

#undef MAKE_INT

typedef size_t usize;
typedef float f32;
typedef double f64;

} // namespace ly

#endif
