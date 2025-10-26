#include "simulator.h"
#include <cassert>
#include <chrono>
#include <iostream>

Simulator::Simulator(BacksymbolMacroMachine* machine) :
    machine{machine},
    state{machine->init_state},
    dir{machine->init_dir},
    tape{ChainTape(machine->init_symbol,machine->init_dir)},
    prover{machine},
    start_time{std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()).time_since_epoch().count()} {
        //
    }

// todo: need to keep Simulator::step and GeneralSimulator::step in sync
void Simulator::step() {
    if (this->op_state != RUNNING) return;
    this->old_step_num=this->step_num;
    // Note: We increment the number of loops early to take care of all the
    // places step() could early-return.
    this->num_loops++;

    if (1) { // todo: switch prover on/off?
        // Log the configuration in the prover and apply rule if possible.
        ProverResult prover_result=this->prover.log_and_apply(
            this->tape,this->state,this->step_num,this->num_loops-1);
        if (std::get_if<ProverResultNothingToDo>(&prover_result)) {}
        else if (auto apply_rule=std::get_if<ProverResultApplyRule>(&prover_result)) {
            // Proof system says that we can apply a rule
            this->tape=apply_rule->new_tape;
            this->num_rule_moves++;
            this->step_num=this->step_num+apply_rule->num_base_steps;
            return;
        }
        else if (std::get_if<ProverResultInfRepeat>(&prover_result)) {
            // Proof system says that machine will repeat forever
            this->op_state=INF_REPEAT;
            this->inf_reason="INF_PROOF_SYSTEM";
            return;
        }
        else assert(0); // unreachable
    }

    // Get current symbol
    int cur_symbol=this->tape.get_top_symbol();
    // Lookup TM transition rule
    Transition trans=this->machine->get_trans_object(cur_symbol,this->state,this->dir);
    this->op_state=trans.condition;
    this->op_details=trans.condition_details;
    // Apply transition
    if (this->op_state==INF_REPEAT) {
        this->inf_reason = "INF_MACRO_STEP";
    }
    // Chain move
    else if (trans.state_out==this->state && trans.dir_out == this->dir && this->op_state==RUNNING) {
        XInteger num_reps=this->tape.apply_chain_move(trans.symbol_out);
        if (num_reps.is_inf()) {
            this->op_state=INF_REPEAT;
            this->inf_reason="INF_CHAIN_STEP";
            return;
        }
        // Don't need to change state or direction
        this->num_chain_moves++;
        this->step_num=this->step_num+num_reps*trans.num_base_steps;
    }
    // Simple move
    else if (this->op_state!=OVER_STEPS_IN_MACRO) {
        this->tape.apply_single_move(trans.symbol_out,trans.dir_out);
        this->state=trans.state_out;
        this->dir=trans.dir_out;
        this->num_macro_moves++;
        this->step_num=this->step_num+trans.num_base_steps;
    }
    else assert(0); // unreachable?
}

void Simulator::print_self(bool full) const {
    //num_loops,num_macro_moves,num_chain_moves,num_rule_moves
    std::cout<<"\n";
    std::cout<<"Elapsed time: "<<(std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()).time_since_epoch().count()-this->start_time)/1e9<<"\n";
    this->tape.print_with_state(this->machine->head_to_string(this->state,this->dir),this->machine->symbol_to_string(),full);
    std::cout<<"Total steps:  "<<this->step_num.to_string()<<"\n";
    std::cout<<"Loops:        "<<this->num_loops<<"\n";
    std::cout<<"Macro moves:  "<<this->num_macro_moves<<"\n";
    std::cout<<"Chain moves:  "<<this->num_chain_moves<<"\n";
    std::cout<<"Rule moves:   "<<this->num_rule_moves<<"\n";
    std::cout<<"Rule proven:  "<<this->prover.rules.size()<<"\n";
    std::cout<<"Elapsed time: "<<(std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()).time_since_epoch().count()-this->start_time)/1e9<<"\n";
}

GeneralSimulator::GeneralSimulator(BacksymbolMacroMachine* machine,int state,const GeneralChainTape& tape) :
    machine{machine},
    state{state},
    dir{tape.dir},
    tape{tape} {
        //
    }

// todo: need to keep Simulator::step and GeneralSimulator::step in sync
void GeneralSimulator::step() {
    if (this->op_state != RUNNING) return;
    this->old_step_num=this->step_num;
    // Note: We increment the number of loops early to take care of all the
    // places step() could early-return.
    this->num_loops++;

    // Get current symbol
    int cur_symbol=this->tape.get_top_symbol();
    // Lookup TM transition rule
    Transition trans=this->machine->get_trans_object(cur_symbol,this->state,this->dir);
    this->op_state=trans.condition;
    this->op_details=trans.condition_details;
    // Apply transition
    if (this->op_state==INF_REPEAT) {
        this->inf_reason = "INF_MACRO_STEP";
    }
    // Chain move
    else if (trans.state_out==this->state && trans.dir_out == this->dir && this->op_state==RUNNING) {
        VarPlusXInteger num_reps=this->tape.apply_chain_move(trans.symbol_out);
        if (num_reps.num.is_inf()) {
            this->op_state=INF_REPEAT;
            this->inf_reason="INF_CHAIN_STEP";
            return;
        }
        // Don't need to change state or direction
        this->step_num=this->step_num+num_reps*trans.num_base_steps;
    }
    // Simple move
    else if (this->op_state!=OVER_STEPS_IN_MACRO) {
        this->tape.apply_single_move(trans.symbol_out,trans.dir_out);
        this->state=trans.state_out;
        this->dir=trans.dir_out;
        this->step_num=this->step_num+trans.num_base_steps;
    }
    else assert(0); // unreachable?
}

void GeneralSimulator::print_self() const {
    std::cout<<"\n";
    this->tape.print_with_state(this->machine->head_to_string(this->state,this->dir),this->machine->symbol_to_string());
    std::cout<<"Total steps: "<<this->step_num.to_string()<<"\n";
    std::cout<<"Loops: "<<this->num_loops<<"\n";
}
