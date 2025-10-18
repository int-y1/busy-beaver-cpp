#pragma once
#include "transition.h"
#include "x_integer.h"
#include <functional>
#include <map>

struct RepeatedSymbol {
    int symbol;
    XInteger num;

    std::string to_string(std::function<std::string(int)> symbol_to_string) const;
};

struct ChainTape {
    Dir dir;
    std::vector<RepeatedSymbol> tape[2];

    ChainTape(int init_symbol,Dir init_dir) :
        dir{init_dir},
        tape{{{init_symbol,XInteger{}}},{{init_symbol,XInteger{}}}} {}

    int get_top_symbol() const {
        return this->tape[this->dir].back().symbol;
    }

    // Apply a chain step which replaces an entire string of symbols.
    // Returns the number of symbols replaced.
    XInteger apply_chain_move(int new_symbol);

    // Apply a single macro step. del old symbol, push new one.
    void apply_single_move(int new_symbol,Dir new_dir);

    void print_with_state(std::string head,std::function<std::string(int)> symbol_to_string,bool full) const;
};

struct VarPlusXInteger {
    std::optional<int> var; // index to min_val
    XInteger num;
};

struct GeneralRepeatedSymbol {
    int id;
    int symbol;
    VarPlusXInteger num;
};

struct GeneralChainTape {
    Dir dir;
    std::vector<GeneralRepeatedSymbol> tape[2];

    GeneralChainTape(const ChainTape& chain_tape,std::map<int,XInteger>& min_val);

    /*
    int get_top_symbol() const {
        return this->tape[this->dir].back().symbol;
    }

    // Apply a chain step which replaces an entire string of symbols.
    // Returns the number of symbols replaced.
    XInteger apply_chain_move(int new_symbol);

    // Apply a single macro step. del old symbol, push new one.
    void apply_single_move(int new_symbol,Dir new_dir);
    */
};
