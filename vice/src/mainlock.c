/** \file   mainlock.c
 * \brief   VICE mutex used to synchronise access to the VICE api and data
 *
 * The mutex is held most of the time by the thread spawned to run VICE in the background.
 * It is frequently unlocked and relocked to allow the UI thread an opportunity to safely
 * call vice functions and access vice data structures.
 *
 * \author  David Hogan <david.q.hogan@gmail.com>
 */

/*
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#ifdef USE_VICE_THREAD

/* #define VICE_MAINLOCK_DEBUG */

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "archdep.h"
#include "debug.h"
#include "log.h"
#include "machine.h"
#include "mainlock.h"
#include "tick.h"
#include "vsyncapi.h"

static pthread_mutex_t lock;

static pthread_t vice_thread;

/* Atomic flags */
static int32_t vice_thread_keepalive    = 1;
static int32_t ui_waiting_for_vice_lock = 0;

#if defined(_MSC_VER) && !defined(__clang__)
#error "You'll need to implement these using msvc intrinsics"
#else
#define atomic_flag_set(x) __atomic_store_n((x), 1, __ATOMIC_RELEASE)
#define atomic_flag_clear(x) __atomic_store_n((x), 0, __ATOMIC_RELEASE)
#define atomic_flag_check(x) __atomic_load_n((x), __ATOMIC_ACQUIRE)
#endif

void mainlock_init(void)
{
    pthread_mutexattr_t lock_attributes;
    pthread_mutexattr_init(&lock_attributes);
    pthread_mutexattr_settype(&lock_attributes, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&lock, &lock_attributes);
}


void mainlock_set_vice_thread(void)
{
    /* The vice thread owns this lock except when explicitly releasing it */
    pthread_mutex_lock(&lock);

    vice_thread = pthread_self();
}


static void consider_exit(void)
{
    /* Check if the vice thread has been told to die. */
    if (atomic_flag_check(&vice_thread_keepalive)) {
        // Nope, keep going.
        return;
    }

    /* NASTY - some emulation can continue on the vice thread during shutdown. */
    if (!pthread_equal(pthread_self(), vice_thread)) {
        return;
    }
    
    pthread_mutex_unlock(&lock);
    
    log_message(LOG_DEFAULT, "VICE thread is exiting");

    archdep_thread_shutdown();

    /* Execution ends here - this function does not return. */
    pthread_exit(NULL);
}


void mainlock_initiate_shutdown(void)
{
    if (!atomic_flag_check(&vice_thread_keepalive)) {
        /* Already shuttting down */
        pthread_mutex_unlock(&lock);
        return;
    }

    log_message(LOG_DEFAULT, "VICE thread initiating shutdown");

    atomic_flag_clear(&vice_thread_keepalive);

    /* If called on the vice thread itself, run the exit code immediately */
    if (pthread_equal(pthread_self(), vice_thread)) {
        consider_exit();
        log_error(LOG_ERR, "VICE thread didn't immediately exit when it should have");
    } else {
        /*
         * The UI thread is initiating shutdown, so we need to let the vice
         * thread know that it should release the lock and consider exit.
         */

        atomic_flag_set(&ui_waiting_for_vice_lock);
    }
}


/** \brief Offer the vice mainlock to the ui thread.
 */
void mainlock_yield(void)
{
    if (atomic_flag_check(&ui_waiting_for_vice_lock)) {
        /*
         * The UI thread is waiting for the vice lock, so we release it and
         * perform a minimal sleep to give the UI thread its chance.
         * 
         * Sleeping appears to be necessary on the linux kernel to avoid
         * the vice thread immediately regaining the lock.
         * 
         * Note that our tick_sleep() implementation releases the vice lock.
         */
        tick_sleep(0);
    }
}


/** \brief Enter a period during which the mainlock can freely be obtained.
 */
void mainlock_yield_begin(void)
{
    pthread_mutex_unlock(&lock);
}


/** \brief The vice thread takes back ownership of the mainlock.
 */
void mainlock_yield_end(void)
{
    pthread_mutex_lock(&lock);
    
    /* After the UI *might* have had the lock, check if we should exit. */
    consider_exit();
}

/****/

void mainlock_obtain(void)
{
#ifdef DEBUG
    if (pthread_equal(pthread_self(), vice_thread)) {
        /*
         * Bad - likely the vice thread directly triggered some UI code.
         * That UI code then generated a signal which is then synchronously
         * pushed through to the handler, which tries to obtain the lock.
         * 
         * The solution is ALWAYS to make VICE asynchronously trigger the
         * UI code.
         */
        printf("FIXME! VICE thread is trying to obtain the mainlock!\n"); fflush(stderr);
        return;
    }
#endif

    /*
     * Indicate that we are waiting for the vice mainlock. This will trigger an
     * unlock and minimal sleep next time the vice thread can yield.
     */
     
    atomic_flag_set(&ui_waiting_for_vice_lock);

    pthread_mutex_lock(&lock);

    /* Not waiting anymore */
    atomic_flag_clear(&ui_waiting_for_vice_lock);
}


bool mainlock_is_vice_thread(void)
{
    return pthread_equal(pthread_self(), vice_thread);
}


void mainlock_release(void)
{
#ifdef DEBUG
    if (pthread_equal(pthread_self(), vice_thread)) {
        /* See detailed comment in mainlock_obtain() */
        printf("FIXME! VICE thread is trying to release the mainlock!\n"); fflush(stdout);
        return;
    }
#endif
    
    pthread_mutex_unlock(&lock);
}

#endif /* #ifdef USE_VICE_THREAD */
