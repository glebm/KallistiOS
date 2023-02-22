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

_Thread_local uint32_t bss_test = 0;
_Thread_local uint32_t tdata_test = 5;

/* Thread Function */
void *thd(void *v) {
    int id = (int) v;

    dbglog(DBG_INFO, "Started Thread %d\n", id);

    for (int i = 0; i < 5; i++){        
        dbglog(DBG_INFO, "Thread[%d]\tbss_test = 0x%lX\n", id, bss_test);
        bss_test++;
        thd_sleep(50);
    }

    for (int i = 0; i < 5; i++){
        dbglog(DBG_INFO, "Thread[%d]\ttdata_test = 0x%lX\n", id, tdata_test);
        tdata_test++;
        thd_sleep(50);
    }

    dbglog(DBG_INFO, "Finished Thread %d\n", id);
    return NULL;
}

/* The main program */
int main(int argc, char **argv) {
    kthread_t * t0, * t1;

    dbglog(DBG_INFO, "Starting Threads\n");

    kthread_t * threads[] = {
        thd_create(0, thd, 1),
        thd_create(0, thd, 2)
    };

    int ret[] = { 
        thd_join(threads[0], NULL),
        thd_join(threads[1], NULL)
    };

    dbglog(DBG_INFO, "Threads Finished!\n");
    return 0;
}
