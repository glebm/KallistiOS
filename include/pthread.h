/*  pthread.h
 *
 *  Written by Joel Sherrill <joel@OARcorp.com>.
 *
 *  COPYRIGHT (c) 1989-2000.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  Permission to use, copy, modify, and distribute this software for any
 *  purpose without fee is hereby granted, provided that this entire notice
 *  is included in all copies of any software which is or includes a copy
 *  or modification of this software.
 *
 *  THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 *  WARRANTY.  IN PARTICULAR,  THE AUTHOR MAKES NO REPRESENTATION
 *  OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
 *  SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 */

// Need a local copy of this because the default one is buggy.

/** \file   pthread.h
    \brief  POSIX-compatibile (sorta) threading support.
    \ingroup threading_posix

    This file was imported (with a few changes) from Newlib. If you really want
    to know about the functions in here, you should probably consult the Single
    Unix Specification and the POSIX specification. Here's a link to that:
    http://pubs.opengroup.org/onlinepubs/007904875/basedefs/pthread.h.html

    The rest of this file will remain undocumented, as it isn't really a part of
    KOS proper... Also, doxygen tends to mangle this whole thing anyway...
*/

/** \cond */

#ifndef __PTHREAD_h
#define __PTHREAD_h

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <sys/_pthread.h>

#if defined(_POSIX_THREADS)

#include <sys/types.h>
#include <time.h>
#include <sys/sched.h>

    /* Register Fork Handlers, P1003.1c/Draft 10, P1003.1c/Draft 10, p. 27

        If an OS does not support processes, then it falls under this provision
        and may not provide pthread_atfork():

        "Either the implementation shall support the pthread_atfork() function
         as described above or the pthread_atfork() function shall not be
         provided."

        NOTE: RTEMS does not provide pthread_atfork().  */

#if !defined(__rtems__)
    int pthread_atfork(void (*prepare)(void), void (*parent)(void),
                    void (*child)(void));
#endif

    /* Mutex Initialization Attributes, P1003.1c/Draft 10, p. 81 */

    int pthread_mutexattr_init(pthread_mutexattr_t *attr);
    int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
    int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int  *pshared);
    int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);

    /* Initializing and Destroying a Mutex, P1003.1c/Draft 10, p. 87 */

    int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
    int pthread_mutex_destroy(pthread_mutex_t *mutex);

    /* This is used to statically initialize a pthread_mutex_t. Example:

        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
     */

#define PTHREAD_MUTEX_INITIALIZER  MUTEX_INITIALIZER

    /*  Locking and Unlocking a Mutex, P1003.1c/Draft 10, p. 93
        NOTE: P1003.4b/D8 adds pthread_mutex_timedlock(), p. 29 */

    int pthread_mutex_lock(pthread_mutex_t *mutex);
    int pthread_mutex_trylock(pthread_mutex_t *mutex);
    int pthread_mutex_unlock(pthread_mutex_t *mutex);

#if defined(_POSIX_TIMEOUTS)

    int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *timeout);

#endif /* _POSIX_TIMEOUTS */

    /* Condition Variable Initialization Attributes, P1003.1c/Draft 10, p. 96 */

    int pthread_condattr_init(pthread_condattr_t *attr);
    int pthread_condattr_destroy(pthread_condattr_t *attr);
    int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared);
    int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared);

    /* Initializing and Destroying a Condition Variable, P1003.1c/Draft 10, p. 87 */

    int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
    int pthread_cond_destroy(pthread_cond_t *cond);

    /* This is used to statically initialize a pthread_cond_t. Example:

        pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
     */

#define PTHREAD_COND_INITIALIZER  COND_INITIALIZER

    /* Broadcasting and Signaling a Condition, P1003.1c/Draft 10, p. 101 */

    int pthread_cond_signal(pthread_cond_t *cond);
    int pthread_cond_broadcast(pthread_cond_t *cond);

    /* Waiting on a Condition, P1003.1c/Draft 10, p. 105 */

    int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

    int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                const struct timespec *abstime);

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)

    /* Thread Creation Scheduling Attributes, P1003.1c/Draft 10, p. 120 */

    int pthread_attr_setscope(pthread_attr_t *attr, int contentionscope);
    int pthread_attr_getscope(const pthread_attr_t *attr, int *contentionscope);
    int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
    int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched);
    int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
    int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy);

#endif /* defined(_POSIX_THREAD_PRIORITY_SCHEDULING) */

    int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
    int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param);

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)

    /* Dynamic Thread Scheduling Parameters Access, P1003.1c/Draft 10, p. 124 */

    int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param);
    int pthread_setschedparam(pthread_t thread, int policy, struct sched_param *param);

#endif /* defined(_POSIX_THREAD_PRIORITY_SCHEDULING) */

#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)

    /* Mutex Initialization Scheduling Attributes, P1003.1c/Draft 10, p. 128 */

    int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol);
    int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol);
    int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling);
    int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling);

#endif /* _POSIX_THREAD_PRIO_INHERIT || _POSIX_THREAD_PRIO_PROTECT */

#if defined(_POSIX_THREAD_PRIO_PROTECT)

    /* Change the Priority Ceiling of a Mutex, P1003.1c/Draft 10, p. 131 */

    int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling);
    int pthread_mutex_getprioceiling(pthread_mutex_t *mutex, int *prioceiling);

#endif /* _POSIX_THREAD_PRIO_PROTECT */

    /* Thread Creation Attributes, P1003.1c/Draft 10, p, 140 */

    int pthread_attr_init(pthread_attr_t *attr);
    int pthread_attr_destroy(pthread_attr_t *attr);
    int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);
    int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
    int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr);
    int pthread_attr_setstackaddr(pthread_attr_t  *attr, void *stackaddr);
    int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
    int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);

    /* Thread Creation, P1003.1c/Draft 10, p. 144 */

    int pthread_create(pthread_t *thread, const pthread_attr_t  *attr,
                void * (*start_routine)(void *), void *arg);

    /* Wait for Thread Termination, P1003.1c/Draft 10, p. 147 */

    int pthread_join(pthread_t thread, void **value_ptr);

    /* Detaching a Thread, P1003.1c/Draft 10, p. 149 */

    int pthread_detach(pthread_t thread);

    /* Thread Termination, p1003.1c/Draft 10, p. 150 */

    void pthread_exit(void *value_ptr);

    /* Get Calling Thread's ID, p1003.1c/Draft 10, p. XXX */

    pthread_t pthread_self(void);

    /* Compare Thread IDs, p1003.1c/Draft 10, p. 153 */

    int pthread_equal(pthread_t t1, pthread_t t2);

    /* Dynamic Package Initialization */

    /* This is used to statically initialize a pthread_once_t. Example:

        pthread_once_t once = PTHREAD_ONCE_INIT;

        NOTE:  This is named inconsistently -- it should be INITIALIZER.  */

#define PTHREAD_ONCE_INIT  { 1, 0 }  /* is initialized and not run */

    int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));

    /* Thread-Specific Data Key Create, P1003.1c/Draft 10, p. 163 */

    int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));

    /* Thread-Specific Data Management, P1003.1c/Draft 10, p. 165 */

    int pthread_setspecific(pthread_key_t key, const void *value);
    void *pthread_getspecific(pthread_key_t key);

    /* Thread-Specific Data Key Deletion, P1003.1c/Draft 10, p. 167 */

    int pthread_key_delete(pthread_key_t key);

    /* Execution of a Thread, P1003.1c/Draft 10, p. 181 */

#define PTHREAD_CANCEL_ENABLE  0
#define PTHREAD_CANCEL_DISABLE 1

#define PTHREAD_CANCEL_DEFERRED 0
#define PTHREAD_CANCEL_ASYNCHRONOUS 1

#define PTHREAD_CANCELED ((void *) -1)

    int pthread_cancel(pthread_t thread);

    /* Setting Cancelability State, P1003.1c/Draft 10, p. 183 */

    int pthread_setcancelstate(int state, int *oldstate);
    int pthread_setcanceltype(int type, int *oldtype);
    void pthread_testcancel(void);

    /* Establishing Cancellation Handlers, P1003.1c/Draft 10, p. 184 */

    void pthread_cleanup_push(void (*routine)(void *), void *arg);
    void pthread_cleanup_pop(int execute);

#if defined(_POSIX_THREAD_CPUTIME)

    /* Accessing a Thread CPU-time Clock, P1003.4b/D8, p. 58 */

    int pthread_getcpuclockid(pthread_t thread_id, clockid_t *clock_id);

    /* CPU-time Clock Thread Creation Attribute, P1003.4b/D8, p. 59 */

    int pthread_attr_setcputime(pthread_attr_t *attr, int clock_allowed);

    int pthread_attr_getcputime(pthread_attr_t *attr, int *clock_allowed);

#endif /* defined(_POSIX_THREAD_CPUTIME) */

#endif /* defined(_POSIX_THREADS) */

#ifdef __cplusplus
}
#endif

#endif
/** \endcond */
/* end of include file */
