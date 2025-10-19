#include "prover.h"
#include "simulator.h"

StrippedSymbol stripped_info(const RepeatedSymbol &block) {
    return {block.symbol,block.num.num==mpz1};
}

// Return a generalized configuration removing the non-1 repetition counts from the tape.
StrippedConfig strip_config(int state,const ChainTape &tape) {
    std::vector<StrippedSymbol> s0,s1;
    std::transform(tape.tape[0].begin(),tape.tape[0].end(),std::back_inserter(s0),stripped_info);
    std::transform(tape.tape[1].begin(),tape.tape[1].end(),std::back_inserter(s1),stripped_info);
    return {state,tape.dir,s0,s1};
}

// Currently, we try to prove a rule we've seen happen twice (not necessarily
// consecutive) with the same num of loops (last_delta or delta_loops).
bool PastConfig::log_config(long long loop_num) {
    // First time we see stripped_config, store loop_num.
    if (this->last_loop_num==0) {
        this->last_loop_num=loop_num;
        this->times_seen++;
        return 0;
    }
    // Next store last_delta. If we have a last_delta but it hasn't repeated,
    // then update the new delta. (Note: We can only prove rules that take
    // the same number of loops each time.)
    long long delta=loop_num-this->last_loop_num;
    if (!this->last_delta || this->last_delta!=delta) {
        this->last_delta=delta;
        this->last_loop_num=loop_num;
        this->times_seen++;
        return 0;
    }
    // Now we can finally try a proof.
    return 1;
}

ProofSystem::ProofSystem(BacksymbolMacroMachine *machine) :
    machine{machine} {}

// Log this configuration into the memory and check if it is similar to a
// past one. Apply rule if possible.
ProverResult ProofSystem::log_and_apply(
    const ChainTape &tape, int state, const XInteger &step_num, long long loop_num
) {
    return ProverResultNothingToDo{}; // todo: remove when ready
    if (tape.tape[0].size()+tape.tape[1].size()>50) {
        return ProverResultNothingToDo{}; // todo: prove rules about big tapes
    }
    StrippedConfig stripped_config=strip_config(state,tape);
    FullConfig full_config{state,tape,loop_num};

    // Try to apply an already proven rule.
    if (auto result=this->try_apply_a_rule(stripped_config,full_config); result.has_value()) {
        return result.value();
    }

    // Otherwise log it into past_configs and see if we should try and prove a new rule.
    PastConfig past_config=this->past_configs[stripped_config];
    if (past_config.log_config(loop_num)) {
        // We see enough of a pattern to try and prove a rule.
        auto rule=this->prove_rule(stripped_config,full_config,loop_num-past_config.last_loop_num);
        if (!rule.has_value()) this->num_failed_proofs++;
        else {
            // add_rule
        }
    }

    return ProverResultNothingToDo{};
}

std::optional<ProverResult> ProofSystem::try_apply_a_rule(
    const StrippedConfig &stripped_config,const FullConfig &full_config
) {
    return std::nullopt;
}

std::optional<bool> ProofSystem::prove_rule(
    const StrippedConfig &stripped_config,const FullConfig &full_config,long long delta_loop
) {
    // Unpack configurations
    auto &[new_state,new_tape,new_loop_num]=full_config;
    std::map<int,XInteger> min_val; // Notes the minimum value exponents with each unknown take.
    // Create the limited simulator with limited or no prover.
    GeneralSimulator gen_sim(this->machine,new_state,GeneralChainTape(new_tape,min_val));

    int max_offset_touched[2]={0,0};
    // Run the simulator
    while (gen_sim.num_loops<delta_loop) {
        const GeneralRepeatedSymbol& block=gen_sim.tape.get_top_block();
        if (block.num.num.num==mpz0) {
            // This corresponds to a block which looks like 2^n+0 .
            // In this situation, we can no longer generalize over all n >= 0.
            // Instead the simulator will act differently if n == 0 or n > 0.
            return std::nullopt;
        }
        // Before step: Record the block we are looking at (about to read).
        Dir cur_dir=gen_sim.tape.dir;
        int facing_offset=block.id;
        if (facing_offset) {
            max_offset_touched[cur_dir]=std::max(max_offset_touched[cur_dir],facing_offset);
        }
        gen_sim.step();
        // After step: Record the block behind us (which we just wrote to).
    }
    return std::nullopt;
}
