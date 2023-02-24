/* KallistiOS ##version##

   compiler_tls.c

   (c)2023 Colton Pawielski

   A simple example showing off thread local variables

   This example launches two threads that access variables
   placed in the TLS segment by the compiler. The compiler
   is then able to generate trivial lookups based on the GBR
   register which holds the address to the current threads's
   control block.

 */

#include <kos.h>

#if (__GNUC__ <= 4)
/* GCC4 only supports using TLS with the __thread identifier,
   even when passed the -std=c99 flag */
#define thread_local __thread
#else
/* Newer versions of GCC use C11's _Thread_local to specify TLS */
#define thread_local _Thread_local
#endif

extern thread_local uint32_t tbss_test = 0;
extern thread_local uint32_t tdata_test = 5;


/* Thread Function */
void *thd(void *v) {
    int i;
    int id = (int) v;

    printf("Started Thread %d\n", id);

    for (i = 0; i < 5; i++){        
        printf("Thread[%d]\tbss_test = 0x%lX\n", id, tbss_test);
        tbss_test++;
        thd_sleep(50);
    }

    for (i = 0; i < 5; i++){
        printf("Thread[%d]\ttdata_test = 0x%lX\n", id, tdata_test);
        tdata_test++;
        thd_sleep(50);
    }

    printf("Finished Thread %d\n", id);
    return NULL;
}

/* The main program */
int main(int argc, char **argv) {
    const int thread_count = 2;

    int i;
    kthread_t * threads[thread_count];   

    printf("Starting Threads\n");

    for(i = 0; i < thread_count; i++) {
        threads[i] = thd_create(0, thd, (void *) i);
    };

    for(i = 0; i < thread_count; i++) {
        int ret = (int) thd_join(threads[i], NULL);
        printf("Thread[%d] Returned: %d\n", i, ret);
    }
    
    printf("Threads Finished!\n");
    return 0;
}
