#pragma once
#include "tape.h"
#include "turing_machine.h"
#include "x_integer.h"
#include <map>
#include <variant>

// Possible values for ProverResult
struct ProverResultNothingToDo {}; // No rule applies, nothing to do.
struct ProverResultApplyRule { // Rule applies, but only finitely many times.
    ChainTape new_tape;
    XInteger num_base_steps;
};
struct ProverResultInfRepeat {}; // Rule applies infinitely.
typedef std::variant<
    ProverResultNothingToDo,
    ProverResultApplyRule,
    ProverResultInfRepeat> ProverResult;

// Stores past information, looks for patterns and tries to prove general
// rules when it finds patterns.
struct ProofSystem {
    // only options.compute_steps is true
    BacksymbolMacroMachine *machine; // todo: support other machines
    bool past_configs; // todo: find correct type
    std::map<std::string,bool> rules; // todo: find correct type
    // a lot of num_* variables, are they actually needed?

    ProofSystem(BacksymbolMacroMachine *machine); // todo: support other machines

    ProverResult log_and_apply(
        const ChainTape &tape,int state,const XInteger &step_num,long long loop_num);
};
