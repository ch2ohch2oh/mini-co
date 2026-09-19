// mini_co_stackless.h — explicit state-machine coroutines.
#pragma once

// A step returns after doing a small unit of work. The caller owns any
// suspended state in arg; this coroutine object owns no execution stack.
typedef enum mini_co_stackless_status {
    MINI_CO_STACKLESS_YIELD,
    MINI_CO_STACKLESS_DONE,
} mini_co_stackless_status;

typedef mini_co_stackless_status (*mini_co_stackless_fn_t)(void* arg);

struct mini_co_stackless_t;

// Create a paused stackless coroutine; returns 0 on success / -1 on OOM.
int mini_co_stackless_create(mini_co_stackless_t** co, mini_co_stackless_fn_t fn, void* arg);

// Run one step. The step callback must preserve its state in arg.
void mini_co_stackless_resume(mini_co_stackless_t* co);

bool mini_co_stackless_finished(mini_co_stackless_t* co);

// Free a finished coroutine.
void mini_co_stackless_release(mini_co_stackless_t* co);
