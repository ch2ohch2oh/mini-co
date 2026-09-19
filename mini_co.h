// mini_co.h — simplified libco for Apple Silicon / Linux.
// Stackful cooperative coroutines on top of ucontext (portable, no x86 asm).
// API mirrors libco: create / resume / yield / self / release.
#pragma once

typedef void (*mini_co_fn_t)(void* arg);

struct mini_co_t;

int mini_co_create(mini_co_t** co, mini_co_fn_t fn, void* arg);
void mini_co_resume(mini_co_t* co);
void mini_co_yield();
mini_co_t* mini_co_self();
bool mini_co_finished(mini_co_t* co);
void mini_co_release(mini_co_t* co);
