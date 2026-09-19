// example_stackless.cpp — the same cooperative style without private stacks.
#include "mini_co_stackless.h"
#include <stdio.h>

struct Task {
    const char* name;
    int step; // program counter: the state that survives each return
};

static mini_co_stackless_status worker(void* arg) {
    Task* task = (Task*)arg;
    switch (task->step++) {
    case 0:
        printf("%s: step 0\n", task->name);
        return MINI_CO_STACKLESS_YIELD;
    case 1:
        printf("%s: step 1\n", task->name);
        return MINI_CO_STACKLESS_YIELD;
    default:
        printf("%s: done\n", task->name);
        return MINI_CO_STACKLESS_DONE;
    }
}

int main() {
    Task task = {"stackless", 0};
    mini_co_stackless_t* co;
    mini_co_stackless_create(&co, worker, &task);
    while (!mini_co_stackless_finished(co))
        mini_co_stackless_resume(co);
    mini_co_stackless_release(co);
    return 0;
}
