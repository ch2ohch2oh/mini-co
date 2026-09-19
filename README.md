# mini-co

Stackful cooperative coroutines for macOS (Apple Silicon and Intel).
libco-like API, ~200 lines.

## How it works

Each coroutine owns a malloc'd, aligned stack. A switch saves the
callee-saved registers plus SP and loads the other coroutine's.
Caller-saved registers need no handling; the call into resume/yield
already spilled them.

Scheduling is a stack of who resumed whom. Resume pushes, yield pops
back to the resumer.

Remaining details: 16-byte SP alignment, a preset first frame that
returns into the entry trampoline, never resume a finished coroutine,
never free a running one, one scheduler per thread.

## Use

```cpp
mini_co_t* co;
mini_co_create(&co, worker, &arg);   // paused; fn not started yet
while (!mini_co_finished(co))
    mini_co_resume(co);              // runs until yield or return
mini_co_release(co);
```

Same thread only, 128 KB per stack. Uses hand-rolled asm instead of
ucontext, which Apple deprecated in macOS 10.6.

## Build

make builds libminico.a and the demo. make run runs it. make
format-check verifies formatting.

Files: mini_co.h, mini_co.cpp, example_mini.cpp.
