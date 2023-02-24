/* KallistiOS ##version##

   compiler_tls.c

   (c)2023 Colton Pawielski

   A simple example showing off _Thread_local variables

   This example launches two threads that access variables
   placed in the TLS segment by the compiler. The compiler
   is then able to generate trivial lookups based on the GBR
   register which holds the address to the current threads's
   control block.

 */

#include <kos.h>

_Thread_local uint32_t tbss_test = 0;
_Thread_local uint32_t tdata_test = 5;

/* Thread Function */
void *thd(void *v) {
    int id = (int) v;

    printf("Started Thread %d\n", id);

    for (int i = 0; i < 5; i++){        
        printf("Thread[%d]\tbss_test = 0x%lX\n", id, tbss_test);
        tbss_test++;
        thd_sleep(50);
    }

    for (int i = 0; i < 5; i++){
        printf("Thread[%d]\ttdata_test = 0x%lX\n", id, tdata_test);
        tdata_test++;
        thd_sleep(50);
    }

    printf("Finished Thread %d\n", id);
    return NULL;
}

/* The main program */
int main(int argc, char **argv) {
    kthread_t * t0, * t1;

    const int thread_count = 2;

    printf("Starting Threads\n");

    kthread_t * threads[thread_count];
    for(int i = 0; i < thread_count; i++) {
        threads[i] = thd_create(0, thd, (void *) i);
    };

    for(int i = 0; i < thread_count; i++) {
        uint32_t ret = (uint32_t) thd_join(threads[i], NULL);
        printf("Thread[%d] Returned: %d\n", i, ret);
    }
    
    printf("Threads Finished!\n");
    return 0;
}
