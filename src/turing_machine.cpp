#include "turing_machine.h"
#include <cassert>
#include <tuple>

SimpleMachine tmFromQuintuples(
    const std::vector<std::tuple<int,int,int,Dir,int>>& quints,
    int num_states,
    int num_symbols
) {
    std::vector<std::vector<Transition>> ttable;
    // Define "undefined" transitions (with metadata).
    for (int state_in=0; state_in<num_states; state_in++) {
        ttable.emplace_back();
        for (int symbol_in=0; symbol_in<num_symbols; symbol_in++) {
            ttable.at(state_in).push_back({UNDEFINED,{symbol_in,state_in},1,-1,RIGHT,1});
        }
    }
    // Set all defined transitions.
    for (auto& [state_in,symbol_in,symbol_out,dir_out,state_out] : quints) {
        if (state_out==-1) {
            ttable.at(state_in).at(symbol_in)=
                {HALT,{symbol_in,state_in},symbol_out,state_out,dir_out,1};
        }
        else {
            ttable.at(state_in).at(symbol_in)=
                {RUNNING,{},symbol_out,state_out,dir_out,1};
        }
    }
    return SimpleMachine(ttable,num_states,num_symbols);
}

SimpleMachine parseTM(const std::string& line) {
    // Read transition table given a standard text representation.
    std::vector<std::tuple<int,int,int,Dir,int>> quints;
    std::vector<std::string> rows;
    {
        std::string token;
        for (char c:line) {
            if (c=='_') {
                rows.push_back(token);
                token.clear();
            }
            else token.push_back(c);
        }
        rows.push_back(token);
    }
    int num_states=rows.size();
    int max_symbol=0;
    for (int state_in=0; state_in<rows.size(); state_in++) {
        std::string row=rows.at(state_in);
        for (int symbol_in=0; symbol_in*3<row.size(); symbol_in++) {
            max_symbol=std::max(max_symbol,symbol_in);
            std::string trans_str=row.substr(symbol_in*3,3);
            assert(trans_str.size()==3);
            if (trans_str=="---") continue;
            assert('0'<=trans_str.at(0) && trans_str.at(0)<='9');
            int symbol_out=trans_str.at(0)-'0';
            assert(trans_str.at(1)=='L' || trans_str.at(1)=='R');
            Dir dir_out=(trans_str.at(1)=='L' ? LEFT : RIGHT);
            assert('A'<=trans_str.at(2) && trans_str.at(2)<='Z');
            int state_out=trans_str.at(2)-'A';
            if (state_out>=num_states) state_out=-1;
            quints.emplace_back(state_in,symbol_in,symbol_out,dir_out,state_out);
        }
    }

    return tmFromQuintuples(quints,num_states,max_symbol+1);
}

// Simulate TM on a limited tape segment.
// Can detect HALT and INF_REPEAT. Used by Macro Machines.
std::pair<Transition,std::vector<int>> sim_limited(
    TuringMachine& tm,
    int state,
    std::vector<int> tape,
    Dir dir,
    int pos
) {
    XInteger num_base_steps{mpz0}; // num_base_steps in the bottom level Simple_Machine.

    // Once we run long enough use this to detect repeat-in-place.
    // If `old_config` is ever repeated, we know it will repeat forever.
    std::tuple<int,std::vector<int>,Dir,int> old_config;
    int next_config_save=128;

    // Simulate Machine on macro symbol
    for (int num_loops=1; 1; num_loops++) { // num_loops is the # steps simulated in this function.
        int symbol=tape.at(pos);
        Transition trans=tm.get_trans_object(symbol,state,dir);
        num_base_steps=num_base_steps+trans.num_base_steps;
        tape.at(pos)=trans.symbol_out;
        state=trans.state_out;
        dir=trans.dir_out;
        if (dir==RIGHT) pos++;
        else pos--;

        if (std::tuple<int,std::vector<int>,Dir,int>{state,tape,dir,pos}==old_config) {
            // Found a repeated config.
            int symbol=0;
            for (int i=tape.size(); i>0; i--) symbol=symbol*tm.num_symbols+tape.at(i-1);
            return {{INF_REPEAT,{pos},symbol,state,dir,num_base_steps},tape};
        }
        if (num_loops>=next_config_save) {
            old_config=std::tuple<int,std::vector<int>,Dir,int>{state,tape,dir,pos};
            next_config_save*=2;
        }

        if (trans.condition!=RUNNING) {
            // Base machine stopped running (HALT, INF_REPEAT, etc.)
            int symbol=0;
            for (int i=tape.size(); i>0; i--) symbol=symbol*tm.num_symbols+tape.at(i-1);
            std::vector<int> condition_details=trans.condition_details;
            condition_details.push_back(pos);
            return {{trans.condition,condition_details,symbol,state,dir,num_base_steps},tape};
        }
        if (!(0<=pos && pos<tape.size())) {
            // We ran off one end of the macro symbol. We're done.
            int symbol=0;
            for (int i=tape.size(); i>0; i--) symbol=symbol*tm.num_symbols+tape.at(i-1);
            return {{RUNNING,{},symbol,state,dir,num_base_steps},tape};
        }
    }
    assert(0); // unreachable
}

BlockMacroMachine::BlockMacroMachine(SimpleMachine base_machine, int block_size) :
        TuringMachine(base_machine.num_states,1),
        base_machine{base_machine},
        block_size{block_size},
        init_state{base_machine.init_state},
        init_symbol{0}, // assume init_symbol = 0
        init_dir{base_machine.init_dir} {
    for (int i=0; i<block_size; i++) {
        assert(2e9/this->num_symbols/base_machine.num_symbols>=1); // prevent int overflow
        this->num_symbols*=base_machine.num_symbols;
    }
    assert(2e9/this->num_states/this->num_symbols/2>=1); // prevent int overflow in get_trans_object
}

std::function<std::string(int)> BlockMacroMachine::symbol_to_string() const {
    return [block_size=this->block_size,num_symbols=this->base_machine.num_symbols](int symbol) {
        std::string s;
        for (int i=0; i<block_size; i++) {
            s.push_back('0'+symbol%num_symbols);
            symbol/=num_symbols;
        }
        return s;
    };
}

const Transition& BlockMacroMachine::get_trans_object(int symbol_in,int state_in,Dir dir) {
    int hash=symbol_in*this->num_states*2+state_in*2+dir;
    if (auto it=this->trans_table.find(hash); it!=this->trans_table.end()) return it->second;

    std::vector<int> tape;
    for (int h=symbol_in,i=0; i<block_size; h/=this->base_machine.num_symbols,i++) {
        tape.push_back(h%this->base_machine.num_symbols);
    }
    int pos=(dir==RIGHT ? 0 : block_size-1);
    return this->trans_table[hash]=sim_limited(this->base_machine,state_in,tape,dir,pos).first;
}

BacksymbolMacroMachine::BacksymbolMacroMachine(BlockMacroMachine base_machine) :
        TuringMachine(base_machine.num_states+1,base_machine.num_symbols),
        base_machine{base_machine},
        init_state{base_machine.init_state}, // assume backsymbol = 0
        init_symbol{0}, // assume init_symbol = 0
        init_dir{base_machine.init_dir} {
    assert(2e9/this->num_symbols/this->num_states>=1); // prevent int overflow in get_trans_object hash.first
    assert(2e9/this->num_symbols/2>=1); // prevent int overflow in get_trans_object hash.second
}

std::string BacksymbolMacroMachine::head_to_string(int state,Dir dir) const {
    char base_state;
    if ((state+1)%this->num_states==0) { // halt (this is a bit sketchy)
        base_state='Z';
        state++;
    }
    else base_state='A'+state%this->num_states;
    std::string s;
    if (dir==LEFT) {
        s+="<";
        s.push_back(base_state);
        s+=" (";
        s+=this->symbol_to_string()(state/this->num_states);
        s+=")";
    }
    else {
        s+="(";
        s+=this->symbol_to_string()(state/this->num_states);
        s+=") ";
        s.push_back(base_state);
        s+=">";
    }
    return s;
}

const Transition& BacksymbolMacroMachine::get_trans_object(int symbol_in,int state_in,Dir dir) {
    std::pair<int,int> hash{state_in,symbol_in*2+dir};
    if (auto it=this->trans_table.find(hash); it!=this->trans_table.end()) return it->second;

    int base_state=state_in%this->num_states;
    std::vector<int> tape;
    int pos;
    if (dir==RIGHT) {
        tape={state_in/this->num_states,symbol_in};
        pos=1;
    }
    else {
        tape={symbol_in,state_in/this->num_states};
        pos=0;
    }
    auto [trans,tape2]=sim_limited(this->base_machine,base_state,tape,dir,pos);
    // sim_limited just leaves the final tape in `trans.symbol_out`, we
    // need to split out the backsymbol and printed_symbol ourselves.
    int final_tape=trans.symbol_out;
    int symbol_out,backsymbol;
    if (trans.dir_out==RIGHT) {
        // [0, 1], A, RIGHT -> 0 (1)A>
        symbol_out=tape2.at(0);
        backsymbol=tape2.at(1);
    }
    else {
        // [0, 1], A, LEFT -> <A(0) 1
        backsymbol=tape2.at(0);
        symbol_out=tape2.at(1);
    }
    // Update symbol_out and state_out to be backsymbol-style.
    int state_out=backsymbol*this->num_states+trans.state_out;
    trans.symbol_out=symbol_out;
    trans.state_out=state_out;
    return this->trans_table[hash]=trans;
}
