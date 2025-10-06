#pragma once
#include "transition.h"
#include <map>
#include <optional>
#include <string>
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
    // A macro transition table. Use hashTransTableArgs to get the index.
    std::vector<Transition> trans_table;

    int init_state;
    int init_symbol;
    Dir init_dir;

    std::vector<std::string> symbol_to_string;

    BlockMacroMachine(SimpleMachine base_machine, int block_size);

    int hashTransTableArgs(int state,int symbol,Dir dir) const {
        return state*this->num_symbols*2+symbol*2+dir;
    }

    const Transition &get_trans_object(int symbol_in,int state_in,Dir dir) const {
        return this->trans_table.at(hashTransTableArgs(state_in,symbol_in,dir));
    }
};
