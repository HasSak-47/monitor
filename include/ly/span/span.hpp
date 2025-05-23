#ifndef __LY_SPAN_HPP__
#define __LY_SPAN_HPP__

#include <memory>
#include <stdexcept>

namespace Ly {

template <class T> class PrimitiveSpan {
public:
    virtual ~PrimitiveSpan()    = 0;
    virtual size_t size() const = 0;
    virtual T& get(size_t i)    = 0;
};

template <class T> class Span : PrimitiveSpan<T> {
    std::shared_ptr<PrimitiveSpan<T>> _v;

public:
    Span(std::shared_ptr<PrimitiveSpan<T>> v) : _v(v) {}
    ~Span() {}

    size_t size() const { return this->_v->size(); }
    T& get(size_t i) { return this->_v->get(i); }
};

template <typename T, typename Container>
inline Span<T> span(Container& c) {
    throw std::runtime_error("Unimplemented!!");
}

} // namespace Ly
#endif
