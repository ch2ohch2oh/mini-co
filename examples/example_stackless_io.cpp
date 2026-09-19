// example_stackless_io.cpp — stackless coroutines with simulated IO waits.
#include "mini_co_stackless.h"
#include <stdio.h>

static int now = 0;

struct Task {
    const char* name;
    int latency;
    int wake_at;
    int step;
};

static mini_co_stackless_status io_task(void* arg) {
    Task* task = (Task*)arg;
    if (task->step == 0) {
        printf("t=%d %s: submit IO\n", now, task->name);
        task->wake_at = now + task->latency;
        task->step = 1;
        return MINI_CO_STACKLESS_YIELD;
    }

    printf("t=%d %s: IO ready\n", now, task->name);
    task->step = 2;
    return MINI_CO_STACKLESS_DONE;
}

int main() {
    Task tasks[2] = {{"fast", 1, 0, 0}, {"slow", 2, 0, 0}};
    mini_co_stackless_t* cos[2];

    for (int i = 0; i < 2; i++)
        mini_co_stackless_create(&cos[i], io_task, &tasks[i]);

    bool pending = true;
    while (pending) {
        pending = false;
        for (int i = 0; i < 2; i++) {
            if (mini_co_stackless_finished(cos[i]))
                continue;
            pending = true;
            if (tasks[i].step == 0 || now >= tasks[i].wake_at)
                mini_co_stackless_resume(cos[i]);
        }
        now++;
    }

    for (int i = 0; i < 2; i++)
        mini_co_stackless_release(cos[i]);
    return 0;
}
