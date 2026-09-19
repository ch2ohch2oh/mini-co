// example_await.cpp — await-style helper on top of mini_co.
// await() runs a dependency coroutine to completion and returns its
// result, so callers read top-down instead of managing resume loops.
#include "mini_co.h"
#include <stdio.h>

struct Task {
    mini_co_t* co;
    int result; // written by the task before it returns
};

// Run t to completion and return its result.
static int await(Task* t) {
    while (!mini_co_finished(t->co))
        mini_co_resume(t->co);
    return t->result;
}

static void fetch(void* arg) {
    Task* t = (Task*)arg;
    printf("fetch: working...\n");
    mini_co_yield(); // pretend IO wait
    mini_co_yield();
    t->result = 42;
    printf("fetch: done\n");
}

static void main_task(void* arg) {
    Task* t = (Task*)arg;
    printf("main: awaiting...\n");
    int v = await(t);
    printf("main: got %d\n", v);
}

int main() {
    Task t;
    mini_co_t* fetcher;
    mini_co_t* main_co;
    mini_co_create(&fetcher, fetch, &t);
    t.co = fetcher;
    t.result = 0;
    mini_co_create(&main_co, main_task, &t);
    while (!mini_co_finished(main_co))
        mini_co_resume(main_co);
    mini_co_release(main_co);
    mini_co_release(fetcher);
    printf("all done\n");
    return 0;
}
