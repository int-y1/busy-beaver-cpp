#pragma once
#include "x_integer.h"
#include <vector>

enum Dir {
    LEFT=0,
    RIGHT=1,
    STAY=2, // unused?
};

enum RunCondition {
    RUNNING, // Machine still running normally
    HALT, // Machine halts in or directly after move
    INF_REPEAT, // Machine proven not to halt within move
    UNDEFINED, // Machine encountered undefined transition
    OVER_STEPS_IN_MACRO, // ?
};

// Class representing the result of a transition.
struct Transition {
    RunCondition condition;
    std::vector<int> condition_details;
    int symbol_out;
    int state_out; // not an optional<int> :(
    Dir dir_out;
    XInteger num_base_steps;
};
