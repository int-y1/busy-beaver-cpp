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

typedef std::pair<int,bool> StrippedSymbol; // stripped version of RepeatedSymbol

// state, dir, left tape, right tape
typedef std::tuple<int,Dir,std::vector<StrippedSymbol>,std::vector<StrippedSymbol>> StrippedConfig;

// state, tape, loop_num
typedef std::tuple<int,const ChainTape&,long long> FullConfig;

// A record of info from past instances of a stripped_config.
// note: this is not really a config
struct PastConfig {
    long long times_seen=0, last_loop_num=0, last_delta=0;

    // Decide whether we should try to prove a rule.
    bool log_config(long long loop_num);
};

// Stores past information, looks for patterns and tries to prove general
// rules when it finds patterns.
struct ProofSystem {
    // only options.compute_steps is true
    BacksymbolMacroMachine *machine; // todo: support other machines
    std::map<StrippedConfig,PastConfig> past_configs;
    std::map<std::string,bool> rules; // todo: find correct type
    // a lot of num_* variables, are they actually needed?
    long long num_failed_proofs=0;

    ProofSystem(BacksymbolMacroMachine *machine); // todo: support other machines

    ProverResult log_and_apply(
        const ChainTape &tape,int state,const XInteger &step_num,long long loop_num);

    std::optional<ProverResult> try_apply_a_rule(
        const StrippedConfig &stripped_config,const FullConfig &full_config);

    // Try to prove a general rule based upon specific example.
    // Returns rule if successful or nullopt.
    std::optional<bool> prove_rule(
        const StrippedConfig &stripped_config,const FullConfig &full_config,long long delta_loop);
};
