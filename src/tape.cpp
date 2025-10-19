#include "tape.h"
#include <iostream>

std::string RepeatedSymbol::to_string(std::function<std::string(int)> symbol_to_string) const {
    std::string s=symbol_to_string(this->symbol);
    s+="^";
    s+=this->num.to_string();
    return s;
}

XInteger ChainTape::apply_chain_move(int new_symbol) {
    // Pop off old sequence
    XInteger num=this->tape[this->dir].back().num;
    // Can't pop off infinite symbols, TM will never halt
    if (num.is_inf()) return num;
    this->tape[this->dir].pop_back();
    // Push on new one behind us
    std::vector<RepeatedSymbol> &half_tape=this->tape[!this->dir];
    RepeatedSymbol &top=half_tape.back();
    if (top.symbol==new_symbol) top.num=top.num+num;
    else half_tape.push_back({new_symbol,num});
    return num;
}

void ChainTape::apply_single_move(int new_symbol,Dir new_dir) {
    {
        // Delete old symbol
        std::vector<RepeatedSymbol> &half_tape=this->tape[this->dir];
        RepeatedSymbol &top=half_tape.back();
        // Decrement (delete one symbol)
        top.num=top.num-1; // yes i can decrement infinity. it is ok.
        // If there are none left, remove from the tape
        if (top.num.num==mpz0) half_tape.pop_back();
    }
    {
        // Push new symbol
        std::vector<RepeatedSymbol> &half_tape=this->tape[!new_dir];
        RepeatedSymbol &top=half_tape.back();
        // If it is identical to the top symbol, combine them.
        if (top.symbol==new_symbol) top.num=top.num+1;
        // Otherwise, just add it separately.
        else half_tape.push_back({new_symbol,1});
    }
    // Update direction
    this->dir=new_dir;
}

const int CUTOFF=3; // todo: increase to 30
void ChainTape::print_with_state(std::string head,std::function<std::string(int)> symbol_to_string,bool full) const {
    XInteger blocks{0};
    if (full) {
        for (auto &sym:this->tape[0]) {
            if (!sym.num.is_inf()) blocks=blocks+sym.num;
            std::cout<<sym.to_string(symbol_to_string)<<" ";
        }
    }
    else {
        int cnt=this->tape[0].size();
        int i=0;
        for (auto &sym:this->tape[0]) {
            if (!sym.num.is_inf()) blocks=blocks+sym.num;
            if (i<CUTOFF || cnt<=i+CUTOFF) std::cout<<sym.to_string(symbol_to_string)<<" ";
            if (i==CUTOFF-1 && cnt>2*CUTOFF) std::cout<<"... ";
            i++;
        }
    }
    std::cout<<head;
    if (full) {
        for (auto it=this->tape[1].rbegin(); it!=this->tape[1].rend(); ++it) {
            if (!it->num.is_inf()) blocks=blocks+it->num;
            std::cout<<" "<<it->to_string(symbol_to_string);
        }
    }
    else {
        int cnt=this->tape[1].size();
        int i=0;
        for (auto it=this->tape[1].rbegin(); it!=this->tape[1].rend(); ++it) {
            if (!it->num.is_inf()) blocks=blocks+it->num;
            if (i<CUTOFF || cnt<=i+CUTOFF) std::cout<<" "<<it->to_string(symbol_to_string);
            if (i==CUTOFF-1 && cnt>2*CUTOFF) std::cout<<" ...";
            i++;
        }
    }
    std::cout<<"\n";
    std::cout<<"Total blocks: "<<blocks.to_string()<<"\n";
}

// Generalize, eg. (abc)^5 -> (abc)^(n+5)
// Blocks with one rep are not generalized, eg. (abc)^1 -> (abc)^1
VarPlusXInteger get_general_num(const XInteger& num,std::map<int,XInteger>& min_val) {
    if (num.is_inf() || num.num==mpz1) return {{},num};
    int v=min_val.size();
    min_val[v]=num;
    return {{{v,{mpz1}}},num};
}

GeneralChainTape::GeneralChainTape(const ChainTape& chain_tape,std::map<int,XInteger>& min_val) :
    dir{chain_tape.dir} {
        for (Dir direction:{LEFT,RIGHT}) {
            int offset=chain_tape.tape[direction].size();
            for (auto &block:chain_tape.tape[direction]) {
                // Mark all starting blocks with IDs to indicate their offset from the
                // starting TM head. If we allow Limited_Diff_Rules, then we will use
                // this to detect which blocks were touched.
                GeneralRepeatedSymbol new_block{offset,block.symbol,get_general_num(block.num,min_val)};
                offset--;
                this->tape[direction].push_back(new_block);
            }
        }
    }

VarPlusXInteger GeneralChainTape::apply_chain_move(int new_symbol) {
    // Pop off old sequence
    VarPlusXInteger num=this->tape[this->dir].back().num;
    // Can't pop off infinite symbols, TM will never halt
    if (num.num.is_inf()) return num;
    this->tape[this->dir].pop_back();
    // Push on new one behind us
    std::vector<GeneralRepeatedSymbol> &half_tape=this->tape[!this->dir];
    GeneralRepeatedSymbol &top=half_tape.back();
    if (top.symbol==new_symbol) top.num=top.num+num;
    else half_tape.push_back({0,new_symbol,num});
    return num;
}

void GeneralChainTape::apply_single_move(int new_symbol,Dir new_dir) {
    {
        // Delete old symbol
        std::vector<GeneralRepeatedSymbol> &half_tape=this->tape[this->dir];
        GeneralRepeatedSymbol &top=half_tape.back();
        // Decrement (delete one symbol)
        top.num=top.num-1; // yes i can decrement infinity. it is ok.
        // If there are none left, remove from the tape
        if (top.num.var.empty() && top.num.num.num==mpz0) half_tape.pop_back();
    }
    {
        // Push new symbol
        std::vector<GeneralRepeatedSymbol> &half_tape=this->tape[!new_dir];
        GeneralRepeatedSymbol &top=half_tape.back();
        // If it is identical to the top symbol, combine them.
        if (top.symbol==new_symbol) top.num=top.num+1;
        // Otherwise, just add it separately.
        else half_tape.push_back({new_symbol,1});
    }
    // Update direction
    this->dir=new_dir;
}
