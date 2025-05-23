
#include <vector>
#include "span.hpp"

namespace Ly {

template <class T>
class VectorSpan : public PrimitiveSpan<T> {
private:
    typename std::vector<T>::iterator _beg, _end;

public:
    VectorSpan(std::vector<T>& vec) {
        this->_beg = vec.begin();
        this->_end = vec.end();
    }

    size_t size() const {
        return (size_t)(this->_end - this->_beg);
    }

    T& get(size_t i) { return *(this->_beg + i); }
};

template <typename T, typename Container = std::vector<T>>
inline Span<T> span(std::vector<T>& c) {
    return Span<T>(std::make_shared<VectorSpan<T>>(c));
}

} // namespace Ly
