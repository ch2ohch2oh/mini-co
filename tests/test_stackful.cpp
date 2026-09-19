#include "mini_co.h"
#include <cassert>

struct State {
    int value;
};

static void worker(void* arg) {
    State* state = (State*)arg;
    state->value = 1;
    mini_co_yield();
    state->value = 2;
    mini_co_yield();
    state->value = 3;
}

int main() {
    State state = {0};
    mini_co_t* co;
    assert(mini_co_create(&co, worker, &state) == 0);
    assert(state.value == 0);
    assert(!mini_co_finished(co));

    mini_co_resume(co);
    assert(state.value == 1);
    mini_co_resume(co);
    assert(state.value == 2);
    mini_co_resume(co);
    assert(state.value == 3);
    assert(mini_co_finished(co));

    mini_co_release(co);
    return 0;
}
