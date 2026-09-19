# mini-co

Educational cooperative coroutines for macOS (Apple Silicon and Intel),
implemented in C++20. The project includes stackful and stackless models.

## Stackful

Each coroutine owns a 128 KB stack. `resume()` runs it until `yield()` or
return, preserving its call stack across suspension. Context switching uses
hand-written assembly instead of macOS-deprecated `ucontext`.

```cpp
mini_co_t* co;
if (mini_co_create(&co, worker, arg) == 0) {
    while (!mini_co_finished(co))
        mini_co_resume(co);
    mini_co_release(co);
}
```

## Stackless

Each step returns instead of suspending a private stack. The callback stores
its state in `arg` and returns `MINI_CO_STACKLESS_YIELD` or
`MINI_CO_STACKLESS_DONE`.

```cpp
struct State { int step = 0; } state;

static mini_co_stackless_status step(void* arg) {
    State* state = (State*)arg;
    return state->step++ == 0 ? MINI_CO_STACKLESS_YIELD
                              : MINI_CO_STACKLESS_DONE;
}

mini_co_stackless_t* co;
mini_co_stackless_create(&co, step, &state);
while (!mini_co_stackless_finished(co))
    mini_co_stackless_resume(co);
mini_co_stackless_release(co);
```

Both models are cooperative and same-thread only. The I/O examples use
simulated tick-based readiness; see `examples/` for dependency, async/await,
and scheduler examples.

## Build

```sh
make              # build library and demos
make run          # run all demos
make test         # run focused tests
make format-check
```
