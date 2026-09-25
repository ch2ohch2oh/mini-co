#include "mini_co.h"
#include <cassert>

struct State {
    int value;
};

struct NestedState {
    mini_co_t* parent;
    mini_co_t* child;
    int steps;
};

static void child_worker(void* arg) {
    NestedState* state = (NestedState*)arg;
    mini_co_resume(state->parent);
    ++state->steps;
}

static void parent_worker(void* arg) {
    NestedState* state = (NestedState*)arg;
    mini_co_resume(state->parent);
    ++state->steps;
    mini_co_resume(state->child);
    ++state->steps;
}

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

    NestedState nested = {nullptr, nullptr, 0};
    assert(mini_co_create(&nested.parent, parent_worker, &nested) == 0);
    assert(mini_co_create(&nested.child, child_worker, &nested) == 0);
    mini_co_resume(nested.parent);
    assert(nested.steps == 3);
    assert(mini_co_finished(nested.parent));
    assert(mini_co_finished(nested.child));
    mini_co_release(nested.child);
    mini_co_release(nested.parent);
    return 0;
}
