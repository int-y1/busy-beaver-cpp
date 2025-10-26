#pragma once
#include <cassert>
#include <gmpxx.h>
#include <map>
#include <optional>

// because mpz0 is faster than 0_mpz. i have no idea why.
inline const mpz_class mpz0=0_mpz, mpz1=1_mpz;

// supports integers of the form: {0,1,2,3,...} U {+inf}
// for numbers >10^10^8, the memory usage might be too big
struct XInteger {
    std::optional<mpz_class> num;

    bool is_inf() const {return !num.has_value();}

    bool operator==(const XInteger& other) const {
        return this->num==other.num;
    }
    bool operator<(const XInteger& other) const {
        if (this->is_inf()) return false;
        if (other.is_inf()) return true;
        return this->num.value()<other.num.value();
    }

    XInteger operator+(int other) const {
        if (this->is_inf()) return {};
        return {this->num.value()+other};
    }
    XInteger operator+(const XInteger& other) const {
        if (this->is_inf()) return {};
        if (other.is_inf()) return {};
        return {this->num.value()+other.num.value()};
    }

    XInteger operator-(int other) const {
        if (this->is_inf()) return {};
        if (this->num.value()<other) assert(0);
        return {this->num.value()-other};
    }
    XInteger operator-(const XInteger& other) const {
        if (other.is_inf()) assert(0);
        if (this->is_inf()) return {};
        if (this->num.value()<other.num.value()) assert(0);
        return {this->num.value()-other.num.value()};
    }

    XInteger operator*(int other) const {
        if (other==0) return {mpz0};
        if (this->is_inf()) return {};
        return {this->num.value()*other};
    }
    XInteger operator*(const XInteger& other) const {
        if (this->num==mpz0 || other.num==mpz0) return {mpz0};
        if (this->is_inf()) return {};
        if (other.is_inf()) return {};
        return {this->num.value()*other.num.value()};
    }

    XInteger operator/(int other) const {
        if (other==0) assert(0);
        if (this->is_inf()) return {};
        return {this->num.value()/other};
    }
    XInteger operator/(const XInteger& other) const {
        if (other.is_inf() || other.num==mpz0) assert(0);
        if (this->is_inf()) return {};
        return {this->num.value()/other.num.value()};
    }

    std::string to_string() const {
        if (this->is_inf()) return "inf";
        std::string out=this->num.value().get_str();
        if (out.size()<=50) return out;
        return "(sz="+std::to_string(out.size())+":"+out.substr(0,25)+"..."+out.substr(out.size()-25)+")";
    }
};

// supports expressions like: (Sum_i coefficient_i*variable_i) + num
struct VarPlusXInteger {
    std::map<int,XInteger> var; // var[index to min_val] = coefficient
    XInteger num;

    XInteger substitute(const std::map<int,XInteger>& assignment) const {
        XInteger out=this->num;
        for (auto& p:this->var) {
            auto it=assignment.find(p.first);
            assert(it!=assignment.end());
            out=out+p.second*it->second;
        }
        return out;
    }

    VarPlusXInteger operator+(int other) const {
        return {this->var,this->num+other};
    }
    VarPlusXInteger operator+(const XInteger& other) const {
        return {this->var,this->num+other};
    }
    VarPlusXInteger operator+(const VarPlusXInteger& other) const {
        std::map<int,XInteger> var2=this->var;
        for (auto& p:other.var) {
            if (auto it=var2.find(p.first); it!=var2.end()) it->second=it->second+p.second;
            else var2[p.first]=p.second;
        }
        return {var2,this->num+other.num};
    }

    VarPlusXInteger operator-(int other) const {
        return {this->var,this->num-other};
    }

    VarPlusXInteger operator*(const XInteger& other) const {
        std::map<int,XInteger> var2=this->var;
        for (auto& p:var2) p.second=p.second*other;
        return {var2,this->num*other};
    }

    std::string to_string() const {
        std::string out;
        if (!this->var.empty()) out+="(";
        for (auto& p:this->var) {
            out+=p.second.to_string();
            out+="*v";
            out+=std::to_string(p.first);
            out+="+";
        }
        out+=this->num.to_string();
        if (!this->var.empty()) out+=")";
        return out;
    }
};
