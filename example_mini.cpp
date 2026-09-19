// example_mini.cpp — round-robin demo of mini_co (libco core idea).
#include "mini_co.h"
#include <stdio.h>

static void worker(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 3; i++) {
        printf("co %d step %d (self=%p)\n", id, i, (void*)mini_co_self());
        mini_co_yield();  // == libco co_yield_ct()
    }
    printf("co %d done\n", id);
}

int main() {
    int ids[3] = {1, 2, 3};
    mini_co_t* cos[3];
    for (int i = 0; i < 3; i++) mini_co_create(&cos[i], worker, &ids[i]);

    // Minimal scheduler: round-robin resume until all finished
    // (libco does this from co_eventloop on epoll/timeout events).
    bool pending = true;
    while (pending) {
        pending = false;
        for (int i = 0; i < 3; i++) {
            if (!mini_co_finished(cos[i])) {
                pending = true;
                mini_co_resume(cos[i]);  // == libco co_resume()
            }
        }
    }
    for (int i = 0; i < 3; i++) mini_co_release(cos[i]);
    printf("all done\n");
    return 0;
}
