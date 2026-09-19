// mini_co_stackless.cpp — explicit state-machine coroutines.
#include "mini_co_stackless.h"
#include <cstdlib>

struct mini_co_stackless_t {
    mini_co_stackless_fn_t fn;
    void* arg;
    bool finished;
};

int mini_co_stackless_create(mini_co_stackless_t** out, mini_co_stackless_fn_t fn, void* arg) {
    mini_co_stackless_t* co = (mini_co_stackless_t*)calloc(1, sizeof(mini_co_stackless_t));
    if (!co)
        return -1;
    co->fn = fn;
    co->arg = arg;
    co->finished = false;
    *out = co;
    return 0;
}

void mini_co_stackless_resume(mini_co_stackless_t* co) {
    if (co->finished)
        return;
    if (co->fn(co->arg) == MINI_CO_STACKLESS_DONE)
        co->finished = true;
}

bool mini_co_stackless_finished(mini_co_stackless_t* co) { return co->finished; }

void mini_co_stackless_release(mini_co_stackless_t* co) { free(co); }
