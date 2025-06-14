#ifndef __LY_ARGUMENT_HPP__
#define __LY_ARGUMENT_HPP__

#include <string>
#include <vector>

namespace ly {
namespace arg {

enum class Type {
    Float,
    Int,
    UInt,
    String,
    Void,
};

class Argumnent {
public:
};

class Builder {
private:
    Type _ty;
    struct Name {
        enum class Type { Long, Short } ty;

        std::string name;
    };
    std::vector<Name> _names;

    bool _parameter;

public:
    Builder() : _ty(Type::Void) {}

    Builder& type(Type ty) {
        this->_ty = ty;
        return *this;
    }

    Builder& short_name(std::string name) {
        this->_names.push_back(
            Name(Name::Type::Short, name));

        return *this;
    }

    Builder& long_name(std::string name) {
        this->_names.push_back(
            Name(Name::Type::Long, name));

        return *this;
    }

    Builder& parameter() {
        this->_parameter = true;

        return *this;
    }

    Builder& option() {
        this->_parameter = false;

        return *this;
    }

    void build();
};

} // namespace arg
} // namespace ly

#endif // !__LY_ARGUMENT_HPP__
