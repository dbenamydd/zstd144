/**
 * Copyright (c) 2016 Tino Reichardt
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 *
 * You can contact the author at:
 * - zstdmt source repository: https://github.com/mcmilk/zstdmt
 */

/**
 * This file will hold wrapper for systems, which do not support pthreads
 */

#include "threading.h"

/* create fake symbol to avoid empty translation unit warning */
int g_ZSTD144_threading_useless_symbol;

#if defined(ZSTD144_MULTITHREAD) && defined(_WIN32)

/**
 * Windows minimalist Pthread Wrapper, based on :
 * http://www.cse.wustl.edu/~schmidt/win32-cv-1.html
 */


/* ===  Dependencies  === */
#include <process.h>
#include <errno.h>


/* ===  Implementation  === */

static unsigned __stdcall worker(void *arg)
{
    ZSTD144_pthread_t* const thread = (ZSTD144_pthread_t*) arg;
    thread->arg = thread->start_routine(thread->arg);
    return 0;
}

int ZSTD144_pthread_create(ZSTD144_pthread_t* thread, const void* unused,
            void* (*start_routine) (void*), void* arg)
{
    (void)unused;
    thread->arg = arg;
    thread->start_routine = start_routine;
    thread->handle = (HANDLE) _beginthreadex(NULL, 0, worker, thread, 0, NULL);

    if (!thread->handle)
        return errno;
    else
        return 0;
}

int ZSTD144_pthread_join(ZSTD144_pthread_t thread, void **value_ptr)
{
    DWORD result;

    if (!thread.handle) return 0;

    result = WaitForSingleObject(thread.handle, INFINITE);
    switch (result) {
    case WAIT_OBJECT_0:
        if (value_ptr) *value_ptr = thread.arg;
        return 0;
    case WAIT_ABANDONED:
        return EINVAL;
    default:
        return GetLastError();
    }
}

#endif   /* ZSTD144_MULTITHREAD */

#if defined(ZSTD144_MULTITHREAD) && DEBUGLEVEL >= 1 && !defined(_WIN32)

#include <stdlib.h>

int ZSTD144_pthread_mutex_init(ZSTD144_pthread_mutex_t* mutex, pthread_mutexattr_t const* attr)
{
    *mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if (!*mutex)
        return 1;
    return pthread_mutex_init(*mutex, attr);
}

int ZSTD144_pthread_mutex_destroy(ZSTD144_pthread_mutex_t* mutex)
{
    if (!*mutex)
        return 0;
    {
        int const ret = pthread_mutex_destroy(*mutex);
        free(*mutex);
        return ret;
    }
}

int ZSTD144_pthread_cond_init(ZSTD144_pthread_cond_t* cond, pthread_condattr_t const* attr)
{
    *cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    if (!*cond)
        return 1;
    return pthread_cond_init(*cond, attr);
}

int ZSTD144_pthread_cond_destroy(ZSTD144_pthread_cond_t* cond)
{
    if (!*cond)
        return 0;
    {
        int const ret = pthread_cond_destroy(*cond);
        free(*cond);
        return ret;
    }
}

#endif
