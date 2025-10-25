#include "prover.h"
#include "simulator.h"

StrippedSymbol stripped_info(const RepeatedSymbol& block) {
    return {block.symbol,block.num.num==mpz1};
}

StrippedSymbol gen_stripped_info(const GeneralRepeatedSymbol& block) {
    return {block.symbol,block.num.var.empty() && block.num.num.num==mpz1};
}

// Return a generalized configuration removing the non-1 repetition counts from the tape.
StrippedConfig strip_config(int state,const ChainTape& tape) {
    std::vector<StrippedSymbol> s0,s1;
    std::transform(tape.tape[0].begin(),tape.tape[0].end(),std::back_inserter(s0),stripped_info);
    std::transform(tape.tape[1].begin(),tape.tape[1].end(),std::back_inserter(s1),stripped_info);
    return {state,tape.dir,s0,s1};
}

StrippedConfig gen_strip_config(int state,const GeneralChainTape& tape) {
    std::vector<StrippedSymbol> s0,s1;
    std::transform(tape.tape[0].begin(),tape.tape[0].end(),std::back_inserter(s0),gen_stripped_info);
    std::transform(tape.tape[1].begin(),tape.tape[1].end(),std::back_inserter(s1),gen_stripped_info);
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

ProofSystem::ProofSystem(BacksymbolMacroMachine* machine) :
    machine{machine} {}

// Log this configuration into the memory and check if it is similar to a
// past one. Apply rule if possible.
ProverResult ProofSystem::log_and_apply(
    const ChainTape& tape, int state, const XInteger& step_num, long long loop_num
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
    PastConfig& past_config=this->past_configs[stripped_config];
    if (past_config.log_config(loop_num)) {
        // We see enough of a pattern to try and prove a rule.
        auto rule=this->prove_rule(stripped_config,full_config,loop_num-past_config.last_loop_num);
        if (!rule.has_value()) this->num_failed_proofs++;
        else {
            this->add_rule(rule.value(),stripped_config);
            // todo: i think "Try to apply transition" is redundant. investigate later.
        }
    }

    return ProverResultNothingToDo{};
}

std::optional<ProverResult> ProofSystem::try_apply_a_rule(
    const StrippedConfig& stripped_config,const FullConfig& full_config
) {
    return std::nullopt;
}

void ProofSystem::add_rule(const DiffRule& diff_rule,const StrippedConfig& stripped_config) {
    //
}

std::optional<DiffRule> ProofSystem::prove_rule(
    const StrippedConfig& stripped_config,const FullConfig& full_config,long long delta_loop
) {
    // Unpack configurations
    auto& [new_state,new_tape,new_loop_num]=full_config;
    std::map<int,XInteger> min_val; // Notes the minimum value exponents with each unknown take.
    // Create the limited simulator with limited or no prover.
    GeneralChainTape initial_tape(new_tape,min_val);
    GeneralSimulator gen_sim(this->machine,new_state,initial_tape);

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
        if (int wrote_offset=gen_sim.tape.tape[!gen_sim.tape.dir].back().id; wrote_offset) {
            max_offset_touched[!gen_sim.tape.dir]=
                std::max(max_offset_touched[!gen_sim.tape.dir],wrote_offset);
        }
        if (gen_sim.op_state!=RUNNING) return std::nullopt;
        // Update min_val for each expression.
        for (Dir dir:{LEFT,RIGHT}) {
            for (auto& block:gen_sim.tape.tape[dir]) {
                if (block.num.var.size()==0) {}
                else if (block.num.var.size()==1 && block.num.var.begin()->second.num==mpz1) {
                    int x=block.num.var.begin()->first;
                    min_val[x]=std::min(min_val[x],block.num.num);
                }
                else return std::nullopt; // shouldn't happen (yet)
            }
        }
    }
    // Make sure finishing tape has the same stripped config as original.
    StrippedConfig gen_stripped_config=gen_strip_config(gen_sim.state,gen_sim.tape);
    if (gen_stripped_config!=stripped_config) return std::nullopt;
    // assume is_diff_rule = true, is_meta_rule = false
    // Tighten up rule to be as general as possible
    // (e.g. by replacing x+5 with x+1 if the rule holds for 1).
    for (Dir dir:{LEFT,RIGHT}) {
        assert(initial_tape.tape[dir].size()==gen_sim.tape.tape[dir].size());
        for (int i=0; i<initial_tape.tape[dir].size(); i++) {
            auto& init_block=initial_tape.tape[dir][i];
            auto& fini_block=gen_sim.tape.tape[dir][i];
            assert(init_block.num.var==fini_block.num.var); // this can fail. todo: find a tm that triggers this
            if (!init_block.num.var.empty()) {
                int x=init_block.num.var.begin()->first;
                init_block.num.num=init_block.num.num-min_val[x]+1;
                fini_block.num.num=fini_block.num.num-min_val[x]+1;
            }
        }
    }
    // Fix num_steps.
    for (auto& p:gen_sim.step_num.var) {
        gen_sim.step_num.num=gen_sim.step_num.num-(min_val[p.first]-1)*p.second;
    }
    return DiffRule{initial_tape,gen_sim.tape,new_state,gen_sim.step_num,gen_sim.num_loops};
}
