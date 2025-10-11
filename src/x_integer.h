#pragma once
#include <cassert>
#include <gmpxx.h>
#include <optional>

// because mpz0 is faster than 0_mpz. i have no idea why.
inline const mpz_class mpz0=0_mpz, mpz1=1_mpz;

// supports integers of the form: {0,1,2,3,...} U {+inf}
// for numbers >10^10^8, the memory usage might be too big
struct XInteger {
    std::optional<mpz_class> num;

    bool is_inf() const {return !num.has_value();}

    bool operator==(const XInteger &other) const {
        return this->num==other.num;
    }
    XInteger operator+(int other) const {
        if (this->is_inf()) return {};
        return {this->num.value()+other};
    }
    XInteger operator+(const XInteger &other) const {
        if (this->is_inf()) return {};
        if (other.is_inf()) return {};
        return {this->num.value()+other.num.value()};
    }
    XInteger operator-(int other) const {
        if (this->is_inf()) return {};
        if (this->num.value()<other) assert(0);
        return {this->num.value()-other};
    }
    XInteger operator*(int other) const {
        if (other==0) return {mpz0};
        if (this->is_inf()) return {};
        return {this->num.value()*other};
    }
    XInteger operator*(const XInteger &other) const {
        if (this->num==mpz0 || other.num==mpz0) return {mpz0};
        if (this->is_inf()) return {};
        if (other.is_inf()) return {};
        return {this->num.value()*other.num.value()};
    }

    std::string to_string() const {
        if (this->is_inf()) return "inf";
        return this->num.value().get_str();
    }
};
