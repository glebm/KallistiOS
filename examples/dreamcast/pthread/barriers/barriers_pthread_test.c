#include <arch/wdt.h>

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

#define WATCHDOG_TIMEOUT    (10 * 1000 * 1000)  /* microseconds */
#define THREAD_COUNT        15
#define ITERATION_COUNT     10

static pthread_barrier_t barrier;
static pthread_barrierattr_t attr;

static atomic_uint pre_barrier_counter = 0;
static atomic_uint serial_barrier_counter = 0;
static atomic_uint post_barrier_counter = 0;

static void run_iteration(void) {
    const pthread_id_np_t id = pthread_getthreadid_np();

    ++pre_barrier_counter;

    printf("Thread[%u]: Before barrier!\n", id);

    int ret = pthread_barrier_wait(&barrier);

    if(ret == PTHREAD_BARRIER_SERIAL_THREAD) {
        printf("Thread[%u]: After barrier: SERIAL!\n", id);
        ++serial_barrier_counter;
    }
    else 
        printf("Thread[%u]: After barrier: NONSERIAL!\n", id);

    ++post_barrier_counter;
}

static void* thread_exec(void *user_data) {
    for(uintptr_t i = 0; i < ITERATION_COUNT; ++i) 
        run_iteration();

    return (void*)true;
}

static void watchdog_timeout(void *user_data) {
    (void)user_data;

    fprintf(stderr, "FAILURE: Watchdog timeout reached!");
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) { 
    pthread_t threads[THREAD_COUNT - 1];
    int ret;
    bool success = true;

    wdt_enable_timer(0, WATCHDOG_TIMEOUT, 0xf, 
                     watchdog_timeout, NULL);
    atexit(wdt_disable);

    ret = pthread_barrier_init(&barrier, NULL, THREAD_COUNT);
    if(ret) {
        fprintf(stderr, "Failed to create pthread barrier: %d\n", ret);
        success = false;
    }

    for(size_t t = 0; t < THREAD_COUNT - 1; ++t) { 
        ret = pthread_create(&threads[t], NULL, thread_exec, NULL);
        
        if(ret) {
            fprintf(stderr, "Failed to create pthread %u with code: %d!", t, ret);
            success = false;
        }
    }

    thread_exec(NULL);

    for(size_t t = 0; t < THREAD_COUNT - 1; ++t) {
        bool thread_ret;
        ret = pthread_join(threads[t], (void**)&thread_ret);

        if(ret) {
            fprintf(stderr, "Failed to join pthread %u with code: %d!", t, ret);
            success = false;
        }
        else if(!thread_ret) {
            fprintf(stderr, "pthread %u returned an error!\n", t);
            success = false;
        }
    }

    ret = pthread_barrier_destroy(&barrier);
    if(ret) {
        fprintf(stderr, "Failed to destroy pthread barrier: %d!\n", ret);
        success = false;
    }


    if(success) {
        printf("\n\n***** TEST COMPLETE: SUCCESS *****\n\n");
        return EXIT_SUCCESS;
    }
    else {
        fprintf(stderr, "\n\nXXXXX TEST COMPLETE: FAILURE XXXXX\n\n");
        return EXIT_FAILURE;
    }
}