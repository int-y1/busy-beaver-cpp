#include "prover.h"

typedef std::pair<int,bool> StrippedSymbol; // stripped version of RepeatedSymbol

StrippedSymbol stripped_info(const RepeatedSymbol &block) {
    return {block.symbol,block.num.num==mpz1};
}

typedef std::tuple<int,Dir,std::vector<StrippedSymbol>,std::vector<StrippedSymbol>> StrippedConfig;

// Return a generalized configuration removing the non-1 repetition counts from the tape.
StrippedConfig strip_config(int state,const ChainTape &tape) {
    std::vector<StrippedSymbol> s0,s1;
    std::transform(tape.tape[0].begin(),tape.tape[0].end(),std::back_inserter(s0),stripped_info);
    std::transform(tape.tape[1].begin(),tape.tape[1].end(),std::back_inserter(s1),stripped_info);
    return {state,tape.dir,s0,s1};
}

ProofSystem::ProofSystem(BacksymbolMacroMachine *machine) :
    machine{machine},
    past_configs{0} {}

// Log this configuration into the memory and check if it is similar to a
// past one. Apply rule if possible.
ProverResult ProofSystem::log_and_apply(
    const ChainTape &tape, int state, const XInteger &step_num, long long loop_num
) {
    //StrippedConfig stripped_config=strip_config(state,tape);
    return ProverResultNothingToDo{};
}
