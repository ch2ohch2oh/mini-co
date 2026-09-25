// mini_co.cpp — minimal libco-like runtime, macOS only (Apple Silicon and Intel).
// No ucontext: Apple deprecated getcontext/makecontext/swapcontext since
// macOS 10.6, so context switching is a tiny hand-rolled asm (callee-saved
// regs + SP only, which is all a resume/yield call needs to preserve).
// How it maps to libco:
//   libco stCoRoutineEnv_t::pCallStack  -> t_stack (resume chain)
//   libco coctx_swap (x86 asm)          -> mini_co_swap (ARM64/x86_64 asm)
//   libco coctx_make (manual stack)     -> mini_co_init_ctx (manual stack)
//   libco co_poll/co_eventloop          -> omitted; scheduling is explicit resume/yield
#include "mini_co.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

static const int kStackSize = 128 * 1024;

#ifndef __APPLE__
#error "mini_co supports macOS only (Apple Silicon and Intel)"
#endif

#if defined(__aarch64__)
struct mini_ctx_t {
    void* x19;  // 0
    void* x20;  // 8
    void* x21;  // 16
    void* x22;  // 24
    void* x23;  // 32
    void* x24;  // 40
    void* x25;  // 48
    void* x26;  // 56
    void* x27;  // 64
    void* x28;  // 72
    void* fp;   // 80 (x29)
    void* lr;   // 88 (x30)
    void* sp;   // 96
    double d8;  // 104
    double d9;  // 112
    double d10; // 120
    double d11; // 128
    double d12; // 136
    double d13; // 144
    double d14; // 152
    double d15; // 160
};
static_assert(offsetof(mini_ctx_t, sp) == 96, "mini_ctx_t layout changed");
static_assert(sizeof(mini_ctx_t) == 168, "mini_ctx_t layout changed");

extern "C" void mini_co_swap(mini_ctx_t* old_, mini_ctx_t* new_);
extern "C" void mini_co_entry(void);
extern "C" void mini_co_trampoline_c(void* arg);

__asm__(".text\n"
        ".p2align 2\n"
        ".globl _mini_co_swap\n"
        "_mini_co_swap:\n"
        "  stp x19, x20, [x0, #0]\n"
        "  stp x21, x22, [x0, #16]\n"
        "  stp x23, x24, [x0, #32]\n"
        "  stp x25, x26, [x0, #48]\n"
        "  stp x27, x28, [x0, #64]\n"
        "  stp x29, x30, [x0, #80]\n"
        "  mov x2, sp\n"
        "  str x2, [x0, #96]\n"
        "  stp d8, d9, [x0, #104]\n"
        "  stp d10, d11, [x0, #120]\n"
        "  stp d12, d13, [x0, #136]\n"
        "  stp d14, d15, [x0, #152]\n"
        "  ldp x19, x20, [x1, #0]\n"
        "  ldp x21, x22, [x1, #16]\n"
        "  ldp x23, x24, [x1, #32]\n"
        "  ldp x25, x26, [x1, #48]\n"
        "  ldp x27, x28, [x1, #64]\n"
        "  ldp x29, x30, [x1, #80]\n"
        "  ldr x2, [x1, #96]\n"
        "  mov sp, x2\n"
        "  ldp d8, d9, [x1, #104]\n"
        "  ldp d10, d11, [x1, #120]\n"
        "  ldp d12, d13, [x1, #136]\n"
        "  ldp d14, d15, [x1, #152]\n"
        "  ret\n"
        ".globl _mini_co_entry\n"
        "_mini_co_entry:\n"
        "  mov x0, x19\n"
        "  bl _mini_co_trampoline_c\n"
        "  brk #0\n");
#else // __x86_64__
struct mini_ctx_t {
    void* rbx; // 0
    void* rbp; // 8
    void* r12; // 16
    void* r13; // 24
    void* r14; // 32
    void* r15; // 40
    void* rsp; // 48
};

extern "C" void mini_co_swap(mini_ctx_t* old_, mini_ctx_t* new_);
extern "C" void mini_co_entry(void);
extern "C" void mini_co_trampoline_c(void* arg);

__asm__(".text\n"
        ".globl _mini_co_swap\n"
        "_mini_co_swap:\n"
        "  movq %rbx, 0(%rdi)\n"
        "  movq %rbp, 8(%rdi)\n"
        "  movq %r12, 16(%rdi)\n"
        "  movq %r13, 24(%rdi)\n"
        "  movq %r14, 32(%rdi)\n"
        "  movq %r15, 40(%rdi)\n"
        "  movq %rsp, 48(%rdi)\n"
        "  movq 48(%rsi), %rsp\n"
        "  movq 0(%rsi), %rbx\n"
        "  movq 8(%rsi), %rbp\n"
        "  movq 16(%rsi), %r12\n"
        "  movq 24(%rsi), %r13\n"
        "  movq 32(%rsi), %r14\n"
        "  movq 40(%rsi), %r15\n"
        "  retq\n"
        ".globl _mini_co_entry\n"
        "_mini_co_entry:\n"
        "  movq %r12, %rdi\n"
        "  callq _mini_co_trampoline_c\n"
        "  ud2\n");
#endif

struct mini_co_t {
    mini_ctx_t ctx;     // own context (== libco coctx_t)
    mini_ctx_t* caller; // context to swap back to on yield
    char* stack;        // own stack (== libco stack_mem)
    mini_co_fn_t fn;
    void* arg;
    bool finished;
};

// libco: __thread gCoEnvPerThread. Here: main ctx + resume chain per thread.
static thread_local mini_ctx_t t_main;
static thread_local std::vector<mini_co_t*> t_stack;

extern "C" void mini_co_trampoline_c(void* arg) {
    mini_co_t* co = (mini_co_t*)arg;
    co->fn(co->arg);
    co->finished = true;
    mini_co_yield(); // return to caller; never resume a finished co
    abort();         // unreachable
}

static void mini_co_init_ctx(mini_co_t* co) {
#if defined(__aarch64__)
    uintptr_t top = (uintptr_t)(co->stack + kStackSize);
    top &= ~(uintptr_t)15; // SP must be 16-byte aligned
    co->ctx.x19 = co;      // entry passes it as x0
    co->ctx.fp = nullptr;
    co->ctx.lr = (void*)mini_co_entry;
    co->ctx.sp = (void*)top;
#else // x86_64: return address lives on the new stack
    uintptr_t top = (uintptr_t)(co->stack + kStackSize);
    top &= ~(uintptr_t)15;
    void** rsp = (void**)(top - sizeof(void*));
    *rsp = (void*)mini_co_entry;
    co->ctx.r12 = co; // entry passes it as rdi
    co->ctx.rsp = rsp;
#endif
}

int mini_co_create(mini_co_t** out, mini_co_fn_t fn, void* arg) {
    mini_co_t* co = (mini_co_t*)calloc(1, sizeof(mini_co_t));
    if (!co)
        return -1;
    co->stack = (char*)malloc(kStackSize);
    if (!co->stack) {
        free(co);
        return -1;
    }
    co->fn = fn;
    co->arg = arg;
    co->finished = false;
    co->caller = nullptr;
    mini_co_init_ctx(co);
    *out = co;
    return 0;
}

void mini_co_resume(mini_co_t* co) {
    if (co->finished)
        return;
    // A running coroutine's context is not suspended and must not be restored.
    if (std::find(t_stack.begin(), t_stack.end(), co) != t_stack.end())
        return;
    mini_ctx_t* caller = t_stack.empty() ? &t_main : &t_stack.back()->ctx;
    co->caller = caller;
    t_stack.push_back(co);
    mini_co_swap(caller, &co->ctx);
    // resumed here when co yields or finishes
}

void mini_co_yield() {
    assert(!t_stack.empty() && "mini_co_yield() called from main");
    mini_co_t* cur = t_stack.back();
    t_stack.pop_back();
    mini_co_swap(&cur->ctx, cur->caller);
}

mini_co_t* mini_co_self() { return t_stack.empty() ? nullptr : t_stack.back(); }

bool mini_co_finished(mini_co_t* co) { return co->finished; }

void mini_co_release(mini_co_t* co) {
    free(co->stack);
    free(co);
}
