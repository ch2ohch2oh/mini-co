// example_event.cpp — minimal event loop on top of mini_co.
// Time is a tick counter (a stand-in for real IO readiness, which would
// come from epoll/kqueue). Each task sleeps until its tick; the loop
// resumes only ready tasks instead of polling everything blindly.
#include "mini_co.h"
#include <stdio.h>

static int now = 0; // loop time

struct Task {
    const char* name;
    int interval; // pretend IO latency: ticks to sleep after each step
    int wake_at;  // simulates IO readiness: ready once now >= wake_at
    mini_co_t* co;
};

static void worker(void* arg) {
    Task* t = (Task*)arg;
    for (int i = 0; i < 3; i++) {
        printf("t=%d %s step %d\n", now, t->name, i);
        t->wake_at = now + t->interval; // sleep until ready again
        mini_co_yield();
    }
}

int main() {
    Task tasks[2] = {{"fast", 1, 0, nullptr}, {"slow", 2, 0, nullptr}};
    for (int i = 0; i < 2; i++)
        mini_co_create(&tasks[i].co, worker, &tasks[i]);

    // Event loop: advance time, run only what is ready.
    bool pending = true;
    while (pending) {
        pending = false;
        for (int i = 0; i < 2; i++) {
            if (mini_co_finished(tasks[i].co))
                continue;
            pending = true;
            if (now >= tasks[i].wake_at)
                mini_co_resume(tasks[i].co);
        }
        now++;
    }
    for (int i = 0; i < 2; i++)
        mini_co_release(tasks[i].co);
    printf("all done\n");
    return 0;
}
