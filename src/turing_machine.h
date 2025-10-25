#pragma once
#include "transition.h"
#include <functional>
#include <map>
#include <string>
#include <vector>

struct TuringMachine {
    int num_states;
    int num_symbols;
    TuringMachine(int num_states,int num_symbols) :
        num_states{num_states}, num_symbols{num_symbols} {}
    virtual const Transition& get_trans_object(int symbol_in,int state_in,Dir dir)=0;
};

// The most general Turing Machine based off of a transition table
struct SimpleMachine : public TuringMachine {
    std::vector<std::vector<Transition>> ttable;

    int init_state=0;
    int init_symbol=0;
    Dir init_dir=RIGHT;

    std::vector<std::string> symbol_to_string;

    SimpleMachine(std::vector<std::vector<Transition>> ttable, int num_states, int num_symbols) :
        TuringMachine(num_states, num_symbols), ttable{ttable} {
            for (int i=0; i<num_states; i++) symbol_to_string.push_back({(char)('0'+i)});
        }

    const Transition& get_trans_object(int symbol_in,int state_in,Dir dir) {
        return this->ttable.at(state_in).at(symbol_in);
    }
};

// Parse TMs in standard text format.
SimpleMachine parseTM(const std::string& line);

// A derivative Turing Machine which simulates another machine clumping k-symbols together into a block-symbol
struct BlockMacroMachine : public TuringMachine {
    SimpleMachine base_machine; // todo: support other machines
    int block_size;

    // A lazy evaluation hashed macro transition table
    std::map<int,Transition> trans_table;

    int init_state;
    int init_symbol;
    Dir init_dir;

    BlockMacroMachine(SimpleMachine base_machine, int block_size);

    std::function<std::string(int)> symbol_to_string() const;

    const Transition& get_trans_object(int symbol_in,int state_in,Dir dir);
};

struct BacksymbolMacroMachine : public TuringMachine {
    BlockMacroMachine base_machine; // todo: support other machines

    // A lazy evaluation hashed macro transition table
    std::map<std::pair<int,int>,Transition> trans_table;

    int init_state;
    int init_symbol;
    Dir init_dir;

    BacksymbolMacroMachine(BlockMacroMachine base_machine);

    std::string head_to_string(int state,Dir dir) const;
    std::function<std::string(int)> symbol_to_string() const {
        return this->base_machine.symbol_to_string();
    }

    const Transition& get_trans_object(int symbol_in,int state_in,Dir dir);
};
