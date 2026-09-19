#include "mini_co_stackless.h"
#include <cassert>

struct State {
    int step;
};

static mini_co_stackless_status worker(void* arg) {
    State* state = (State*)arg;
    if (state->step++ < 2)
        return MINI_CO_STACKLESS_YIELD;
    return MINI_CO_STACKLESS_DONE;
}

int main() {
    State state = {0};
    mini_co_stackless_t* co;
    assert(mini_co_stackless_create(&co, worker, &state) == 0);
    assert(!mini_co_stackless_finished(co));

    mini_co_stackless_resume(co);
    assert(state.step == 1);
    mini_co_stackless_resume(co);
    assert(state.step == 2);
    mini_co_stackless_resume(co);
    assert(state.step == 3);
    assert(mini_co_stackless_finished(co));

    mini_co_stackless_release(co);
    return 0;
}
