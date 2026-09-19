// example_deps.cpp — one coroutine depending on another.
// The consumer pulls values from the producer by resuming it (nested
// resume); a shared slot hands the value over on each yield.
#include "mini_co.h"
#include <stdio.h>

static int produced = -1; // handoff slot: producer writes, consumer reads

static void producer(void* arg) {
    (void)arg;
    for (int i = 1; i <= 3; i++) {
        produced = i * 10;
        printf("producer: made %d\n", produced);
        mini_co_yield(); // back to whoever resumed us (consumer)
    }
    printf("producer: done\n");
}

static void consumer(void* arg) {
    mini_co_t* prod = (mini_co_t*)arg; // our dependency
    while (!mini_co_finished(prod)) {
        mini_co_resume(prod); // run dependency until it yields or returns
        if (!mini_co_finished(prod))
            printf("consumer: got %d\n", produced);
    }
    printf("consumer: done\n");
}

int main() {
    mini_co_t* prod;
    mini_co_t* cons;
    mini_co_create(&prod, producer, nullptr);
    mini_co_create(&cons, consumer, prod);
    while (!mini_co_finished(cons))
        mini_co_resume(cons);
    mini_co_release(cons);
    mini_co_release(prod);
    printf("all done\n");
    return 0;
}
