// example_stackless_await.cpp — two async fetches without private stacks.
#include "mini_co_stackless.h"
#include <stdio.h>

static int now = 0;

struct Fetch {
    const char* name;
    int latency;
    int result;
    int wake_at;
    int step;
};

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
    mini_co_stackless_t* user_fetch;
    mini_co_stackless_t* posts_fetch;
    Fetch user_state;
    Fetch posts_state;
    int step;
};

static bool is_ready(mini_co_stackless_t* co) { return mini_co_stackless_finished(co); }

static mini_co_stackless_status app_step(void* arg) {
    App* app = (App*)arg;
    if (app->step == 0) {
        printf("t=%d app: awaiting user and posts\n", now);
        mini_co_stackless_create(&app->user_fetch, fetch_step, &app->user_state);
        mini_co_stackless_create(&app->posts_fetch, fetch_step, &app->posts_state);
        // Start both dependencies; later steps are scheduler-driven.
        mini_co_stackless_resume(app->user_fetch);
        mini_co_stackless_resume(app->posts_fetch);
        app->step = 1;
    }

    // Await means return pending; the scheduler will call us again later.
    if (!is_ready(app->user_fetch) || !is_ready(app->posts_fetch)) {
        printf("t=%d app: suspended\n", now);
        return MINI_CO_STACKLESS_YIELD;
    }

    printf("t=%d app: got user=%d posts=%d\n", now, app->user_state.result,
           app->posts_state.result);
    return MINI_CO_STACKLESS_DONE;
}

int main() {
    App app = {nullptr, nullptr, {"user", 1, 42, 0, 0}, {"posts", 2, 7, 0, 0}, 0};
    mini_co_stackless_t* app_co;

    mini_co_stackless_create(&app_co, app_step, &app);

    while (!mini_co_stackless_finished(app_co)) {
        if (app.user_fetch != nullptr && !mini_co_stackless_finished(app.user_fetch) &&
            now >= app.user_state.wake_at)
            mini_co_stackless_resume(app.user_fetch);
        if (app.posts_fetch != nullptr && !mini_co_stackless_finished(app.posts_fetch) &&
            now >= app.posts_state.wake_at)
            mini_co_stackless_resume(app.posts_fetch);
        mini_co_stackless_resume(app_co);
        now++;
    }

    mini_co_stackless_release(app_co);
    mini_co_stackless_release(app.user_fetch);
    mini_co_stackless_release(app.posts_fetch);
    return 0;
}
