#pragma once
#include <cassert>
#include <optional>

// supports integers of the form: {0,1,2,3,...} U {+inf}
// todo: support integers larger than LLONG_MAX
struct XInteger {
    std::optional<long long> num;

    bool is_inf() const {return !num.has_value();}

    bool operator==(const XInteger &other) const {
        return this->num==other.num;
    }
    XInteger operator+(int other) const {
        if (this->is_inf()) return {};
        return XInteger{this->num.value()+other};
    }
    XInteger operator+(const XInteger &other) const {
        if (this->is_inf()) return {};
        if (other.is_inf()) return {};
        return XInteger{this->num.value()+other.num.value()};
    }
    XInteger operator-(int other) const {
        if (this->is_inf()) return {};
        if (this->num.value()<other) assert(0);
        return XInteger{this->num.value()-other};
    }
    XInteger operator*(int other) const {
        if (other==0) return {0};
        if (this->is_inf()) return {};
        return XInteger{this->num.value()*other};
    }
    XInteger operator*(const XInteger &other) const {
        if (this->num==std::optional<long long>{0} || other.num==std::optional<long long>{0}) return {0};
        if (this->is_inf()) return {};
        if (other.is_inf()) return {};
        return XInteger{this->num.value()*other.num.value()};
    }

    std::string to_string() const {
        if (this->is_inf()) return "inf";
        return std::to_string(this->num.value());
    }
};
