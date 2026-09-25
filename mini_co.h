// mini_co.h — simplified libco for macOS (Apple Silicon and Intel).
// Cooperative coroutines: resume to run, yield to pause. Same thread
// only; 128 KB stack each. Lifecycle: create -> resume -> release.
#pragma once

// Entry point; returning from fn finishes the coroutine.
typedef void (*mini_co_fn_t)(void* arg);

// Opaque handle; caller owns it (create/release).
struct mini_co_t;

// Create paused coroutine; writes *co, returns 0 ok / -1 on OOM.
int mini_co_create(mini_co_t** co, mini_co_fn_t fn, void* arg);

// Run until yield or return; no-op if finished or already active.
// Nesting with a different coroutine is supported.
void mini_co_resume(mini_co_t* co);

// Pause self, back to resumer. Coroutine-only.
void mini_co_yield();

// Running coroutine, or NULL on main.
mini_co_t* mini_co_self();

// True after entry fn returned; do not resume again.
bool mini_co_finished(mini_co_t* co);

// Free a finished (or never-to-run-again) coroutine; never self.
void mini_co_release(mini_co_t* co);
