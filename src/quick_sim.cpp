// ./quick_sim 1RB0LE_1RC1RB_1RD0RA_0RE---_1LF1LA_1LA1LF 12
// expected speed: 32500000 loop/s

#include "simulator.h"
#include "turing_machine.h"
#include <cassert>
#include <iostream>

void run(
    SimpleMachine machine,
    int block_size
) {
    BlockMacroMachine machine2(machine,block_size);
    Simulator sim(machine2);
    sim.print_self();
    long long next_print=1000000;
    for(long long total_loops=0; sim.op_state==RUNNING; total_loops++) {
        sim.step();
        if (sim.num_loops>=next_print) {
            sim.print_self();
            next_print=next_print*6/5;
            if (sim.num_loops>=1000000000) break; // todo: remove
        }
    }

    sim.print_self(true);
    std::cout<<"end of run"<<std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: quick_sim tm block_size" << std::endl;
        return 1;
    }
    SimpleMachine machine = parseTM(std::string(argv[1],strlen(argv[1])));
    int block_size=std::stoi(argv[2]);
    run(std::move(machine),block_size);
}