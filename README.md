# mini-co

Educational cooperative coroutines for macOS (Apple Silicon and Intel).
Includes stackful and stackless implementations.

## How it works

Each coroutine owns a 128 KB stack. A context switch saves the
callee-saved registers and stack pointer, then loads the other
coroutine's context.

Scheduling is a stack of who resumed whom. Resume pushes, yield pops
back to the resumer.

Coroutines are cooperative and same-thread only. They must not be
resumed after finishing or released while running.

## Use

```cpp
mini_co_t* co;
mini_co_create(&co, worker, &arg);   // paused; fn not started yet
while (!mini_co_finished(co))
    mini_co_resume(co);              // runs until yield or return
mini_co_release(co);
```

`mini_co_create` returns 0 on success and -1 on allocation failure.
The implementation uses hand-rolled assembly instead of `ucontext`,
which Apple deprecated in macOS 10.6.

Stackful coroutines preserve a call stack across `yield`. Stackless
coroutines return after each step; suspended state lives in the
caller-owned argument instead of a coroutine stack.

## Build

`make` builds the library and all demos. `make run` runs the demos;
`make test` runs the focused tests. `make format-check` verifies
formatting. The project uses C++20.

The `examples/` directory contains round-robin, dependency, await-style,
event-loop, stackless, stackless IO, async/await, and scheduler demos.
