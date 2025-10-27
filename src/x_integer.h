#pragma once
#include <cassert>
#include <flint/fmpz.h>
#include <map>
#include <optional>
#include <string>

struct fmpz_class {
    fmpz_t num;

    // rule of 3
    fmpz_class() {fmpz_init(num);}
    fmpz_class(slong x) {fmpz_init_set_si(num,x);}
private:
    fmpz_class(fmpz_t x): num{x[0]} {}
public:
    ~fmpz_class() {fmpz_clear(num);}
    fmpz_class(const fmpz_class& other) {fmpz_init_set(num,other.num);}
    fmpz_class& operator=(const fmpz_class& other) {
        fmpz_set(num,other.num);
        return *this;
    }
    // actually, i want rule of 5
    fmpz_class(fmpz_class&& other) noexcept {
        fmpz_init(num);
        fmpz_swap(num,other.num);
    }
    fmpz_class& operator=(fmpz_class&& other) noexcept {
        fmpz_swap(num,other.num);
        return *this;
    }

    bool operator==(const fmpz_class& other) const {
        return fmpz_equal(num,other.num);
    }
    bool operator<(const fmpz_class& other) const {
        return fmpz_cmp(num,other.num)<0;
    }
    fmpz_class operator+(const fmpz_class& other) const {
        fmpz_t res;
        fmpz_init(res);
        fmpz_add(res,num,other.num);
        return {res};
    }
    fmpz_class operator+(slong other) const {
        fmpz_t res;
        fmpz_init(res);
        fmpz_add_si(res,num,other);
        return {res};
    }
    fmpz_class operator-(const fmpz_class& other) const {
        fmpz_t res;
        fmpz_init(res);
        fmpz_sub(res,num,other.num);
        return {res};
    }
    fmpz_class operator-(slong other) const {
        fmpz_t res;
        fmpz_init(res);
        fmpz_sub_si(res,num,other);
        return {res};
    }
    fmpz_class operator*(const fmpz_class& other) const {
        fmpz_t res;
        fmpz_init(res);
        fmpz_mul(res,num,other.num);
        return {res};
    }
    fmpz_class operator*(slong other) const {
        fmpz_t res;
        fmpz_init(res);
        fmpz_mul_si(res,num,other);
        return {res};
    }
    fmpz_class operator/(const fmpz_class& other) const {
        fmpz_t res;
        fmpz_init(res);
        fmpz_fdiv_q(res,num,other.num);
        return {res};
    }
    fmpz_class operator/(slong other) const {
        fmpz_t res;
        fmpz_init(res);
        fmpz_fdiv_q_si(res,num,other);
        return {res};
    }
    std::string get_str() const {
        char* p=fmpz_get_str(nullptr,10,num);
        std::string out(p);
        free(p);
        return out;
    }
};

// speed up access to 0 and 1
inline const fmpz_class mpz0((slong)0), mpz1((slong)1);

// supports integers of the form: {0,1,2,3,...} U {+inf}
// for numbers >10^10^8, the memory usage might be too big
struct XInteger {
    std::optional<fmpz_class> num;

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
