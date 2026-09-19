// example_stackless_scheduler.cpp — one scheduler for three task types.
#include "mini_co_stackless.h"
#include <cstdlib>
#include <stdio.h>
#include <vector>

static int now = 0;

class Scheduler {
    typedef bool (*ready_fn_t)(void* arg);

    struct Entry {
        mini_co_stackless_t* co;
        ready_fn_t ready;
        void* arg;
    };

    std::vector<Entry> entries;

  public:
    ~Scheduler() {
        for (const Entry& entry : entries)
            mini_co_stackless_release(entry.co);
    }

    // The scheduler owns spawned handles and releases them in its destructor.
    mini_co_stackless_t* spawn(mini_co_stackless_fn_t fn, void* arg, ready_fn_t ready) {
        mini_co_stackless_t* co = nullptr;
        if (mini_co_stackless_create(&co, fn, arg) != 0)
            std::abort();
        entries.push_back({co, ready, arg});
        return co;
    }

    void run() {
        bool pending = true;
        while (pending) {
            pending = false;
            for (size_t i = 0; i < entries.size(); i++) {
                Entry& entry = entries[i];
                if (mini_co_stackless_finished(entry.co))
                    continue;
                pending = true;
                if (entry.ready == nullptr || entry.ready(entry.arg))
                    mini_co_stackless_resume(entry.co);
            }
            now++;
        }
    }
};

struct Fetch {
    const char* name;
    int latency;
    int result;
    int wake_at;
    int step;
};

static bool fetch_ready(void* arg) {
    Fetch* fetch = (Fetch*)arg;
    return fetch->step == 0 || now >= fetch->wake_at;
}

static mini_co_stackless_status fetch_step(void* arg) {
    Fetch* fetch = (Fetch*)arg;
    if (fetch->step == 0) {
        printf("t=%d %s: submit IO\n", now, fetch->name);
        fetch->wake_at = now + fetch->latency;
        fetch->step = 1;
        return MINI_CO_STACKLESS_YIELD;
    }

    printf("t=%d %s: complete\n", now, fetch->name);
    return MINI_CO_STACKLESS_DONE;
}

struct App {
    Scheduler* scheduler;
    mini_co_stackless_t* user;
    mini_co_stackless_t* posts;
    Fetch user_state;
    Fetch posts_state;
    int step;
};

static mini_co_stackless_status app_step(void* arg) {
    App* app = (App*)arg;
    if (app->step == 0) {
        printf("t=%d app: spawn and await both fetches\n", now);
        app->user = app->scheduler->spawn(fetch_step, &app->user_state, fetch_ready);
        app->posts = app->scheduler->spawn(fetch_step, &app->posts_state, fetch_ready);
        app->step = 1;
    }

    if (!mini_co_stackless_finished(app->user) || !mini_co_stackless_finished(app->posts)) {
        printf("t=%d app: suspended\n", now);
        return MINI_CO_STACKLESS_YIELD;
    }

    printf("t=%d app: got user=%d posts=%d\n", now, app->user_state.result,
           app->posts_state.result);
    return MINI_CO_STACKLESS_DONE;
}

static bool always_ready(void*) { return true; }

int main() {
    Scheduler scheduler;
    App app = {
        .scheduler = &scheduler,
        .user = nullptr,
        .posts = nullptr,
        .user_state = {.name = "user", .latency = 5, .result = 42, .wake_at = 0, .step = 0},
        .posts_state = {.name = "posts", .latency = 10, .result = 7, .wake_at = 0, .step = 0},
        .step = 0,
    };
    scheduler.spawn(app_step, &app, always_ready);
    scheduler.run();
    return 0;
}
