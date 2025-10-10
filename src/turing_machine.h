#pragma once
#include "transition.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// The most general Turing Machine based off of a transition table
struct SimpleMachine {
    std::vector<std::vector<Transition>> ttable;
    int num_states;
    int num_symbols;

    int init_state=0;
    int init_symbol=0;
    Dir init_dir=RIGHT;

    std::vector<std::string> symbol_to_string;

    SimpleMachine(std::vector<std::vector<Transition>> ttable, int num_states, int num_symbols) :
        ttable{ttable}, num_states{num_states}, num_symbols{num_symbols} {
            for (int i=0; i<num_states; i++) symbol_to_string.push_back({(char)('0'+i)});
        }

    const Transition &get_trans_object(int symbol_in,int state_in,Dir dir) const {
        return this->ttable.at(state_in).at(symbol_in);
    }
};

// Parse TMs in standard text format.
SimpleMachine parseTM(const std::string &line);

// A derivative Turing Machine which simulates another machine clumping k-symbols together into a block-symbol
struct BlockMacroMachine {
    SimpleMachine base_machine; // todo: support other machines
    int block_size;

    int num_states;
    int num_symbols;
    // A lazy evaluation hashed macro transition table
    std::unordered_map<int,Transition> trans_table;

    int init_state;
    int init_symbol;
    Dir init_dir;

    BlockMacroMachine(SimpleMachine base_machine, int block_size);

    std::function<std::string(int)> symbol_to_string() const;

    const Transition& get_trans_object(int symbol_in,int state_in,Dir dir);
};
