#pragma once
#include "tape.h"
#include "turing_machine.h"
#include "x_integer.h"

struct Simulator {
    BacksymbolMacroMachine machine; // todo: support other machines
    int state;
    Dir dir;

    XInteger old_step_num{0},step_num{0};

    ChainTape tape;

    // todo: implement prover

    // Operation state (e.g. running, halted, proven-infinite, ...)
    RunCondition op_state=RUNNING;
    std::vector<int> op_details;

    // Stats
    long long start_time;
    long long num_loops=0,num_macro_moves=0,num_chain_moves=0,num_rule_moves=0;
    std::string inf_reason; // doesn't need to be enum yet

    Simulator(BacksymbolMacroMachine machine); // todo: support other machines

    // Perform an atomic transition or chain step.
    void step();

    void print_self(bool full=false) const;
};
