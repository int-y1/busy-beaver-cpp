#include "tape.h"
#include <iostream>

std::string RepeatedSymbol::to_string(const std::vector<std::string> &symbol_to_string) const {
    std::string s=symbol_to_string[this->symbol];
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
        if (top.num==XInteger{0}) half_tape.pop_back();
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

const int CUTOFF=3;
void ChainTape::print_with_state(int state,const std::vector<std::string> &symbol_to_string,bool full) const {
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
    if (this->dir==LEFT) std::cout<<"<";
    if (state<0) std::cout<<"Z";
    else std::cout<<(char)('A'+state);
    if (this->dir==RIGHT) std::cout<<">";
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
