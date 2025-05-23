
#include "span.hpp"

namespace Ly {

template <class T, size_t len>
class ArraySpan : public PrimitiveSpan<T> {
private:
    typename std::array<T, len>::iterator _beg, _end;

public:
    ArraySpan(std::array<T, len>& vec) {
        this->_beg = vec.begin();
        this->_end = vec.end();
    }

    size_t size() const {
        return (size_t)(this->_end - this->_beg);
    }

    T& get(size_t i) { return *(this->_beg + i); }
};

template <typename T, size_t len,
    typename Container = std::array<T, len>>
inline Span<T> span(std::array<T, len>& c) {
    return Span<T>(std::make_shared<ArraySpan<T, len>>(c));
}

} // namespace Ly
