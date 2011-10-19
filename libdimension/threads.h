/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Library.                           *
 *                                                                       *
 * The Dimension Library is free software; you can redistribute it and/  *
 * or modify it under the terms of the GNU Lesser General Public License *
 * as published by the Free Software Foundation; either version 3 of the *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Library is distributed in the hope that it will be      *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

/**
 * @file
 * Background threading interface.
 */

#include <pthread.h>

/**
 * Thread callback type.
 * @param[in,out] ptr  An arbitrary pointer.
 * @return 0 on success, non-zero on failure.
 */
typedef int dmnsn_thread_fn(void *ptr);

/**
 * Create a thread that cleans up after itself on errors.
 * @param[in,out] future     The future object to associate with the thread.
 * @param[in]     thread_fn  The thread callback.
 * @param[in,out] arg        The pointer to pass to the thread callback.
 */
DMNSN_INTERNAL void dmnsn_new_thread(dmnsn_future *future,
                                     dmnsn_thread_fn *thread_fn, void *arg);

/**
 * Thread callback type for parallel tasks.
 * @param[in,out] ptr       An arbitrary pointer.
 * @param[in]     thread    An ID for this thread, in [0, \p nthreads).
 * @param[in]     nthreads  The number of concurrent threads.
 * @return 0 on success, non-zero on failure.
 */
typedef int dmnsn_ccthread_fn(void *ptr, unsigned int thread,
                              unsigned int nthreads);

/**
 * Run \p nthreads threads in parallel.
 * @param[in]     ccthread_fn  The routine to run in each concurrent thread.
 * @param[in,out] arg          The pointer to pass to the thread callbacks.
 * @param[in]     nthreads     The number of concurrent threads to run.
 * @return 0 if all threads were successful, and an error code otherwise.
 */
DMNSN_INTERNAL int dmnsn_execute_concurrently(dmnsn_ccthread_fn *ccthread_fn,
                                              void *arg, unsigned int nthreads);

/**
 * Initialize a mutex, bailing out on failure.
 * @param[out] mutex  The mutex to initialize.
 */
DMNSN_INTERNAL void dmnsn_initialize_mutex(pthread_mutex_t *mutex);

/** dmnsn_lock_mutex() implementation. */
DMNSN_INTERNAL void dmnsn_lock_mutex_impl(pthread_mutex_t *mutex);
/** dmnsn_unlock_mutex() implementation. */
DMNSN_INTERNAL void dmnsn_unlock_mutex_impl(pthread_mutex_t *mutex);

/**
 * Lock a mutex, bailing out on failure.
 * Contains a {, so must be used in the same block as dmnsn_unlock_mutex().
 * @param[in,out] mutex  The mutex to lock.
 */
#define dmnsn_lock_mutex(mutex) dmnsn_lock_mutex_impl((mutex)); {

/**
 * Lock a mutex, bailing out on failure.
 * Contains a }, so must be used in the same block as dmnsn_lock_mutex().
 * @param[in,out] mutex  The mutex to unlock.
 */
#define dmnsn_unlock_mutex(mutex) dmnsn_unlock_mutex_impl((mutex)); }

/**
 * Destroy a mutex, warning on failure.
 * @param[in,out] mutex  The mutex to destroy.
 */
DMNSN_INTERNAL void dmnsn_destroy_mutex(pthread_mutex_t *mutex);

/**
 * Initialize a read-write lock, bailing out on failure.
 * @param[out] rwlock  The read-write lock to initialize.
 */
DMNSN_INTERNAL void dmnsn_initialize_rwlock(pthread_rwlock_t *rwlock);

/** dmnsn_read_lock() implementation. */
DMNSN_INTERNAL void dmnsn_read_lock_impl(pthread_rwlock_t *rwlock);
/** dmnsn_write_lock() implementation. */
DMNSN_INTERNAL void dmnsn_write_lock_impl(pthread_rwlock_t *rwlock);
/** dmnsn_unlock_rwlock() implementation. */
DMNSN_INTERNAL void dmnsn_unlock_rwlock_impl(pthread_rwlock_t *rwlock);

/**
 * Lock a read-write lock, bailing out on failure.
 * Contains a {, so must be used in the same block as dmnsn_unlock_rwlock().
 * @param[in,out] rwlock  The read-write lock to lock.
 */
#define dmnsn_read_lock(rwlock) dmnsn_read_lock_impl((rwlock)); {

/**
 * Lock a read-write lock, bailing out on failure.
 * Contains a {, so must be used in the same block as dmnsn_unlock_rwlock().
 * @param[in,out] rwlock  The read-write lock to lock.
 */
#define dmnsn_write_lock(rwlock) dmnsn_write_lock_impl((rwlock)); {

/**
 * Lock a read-write lock, bailing out on failure.
 * Contains a }, so must be used in the same block as dmnsn_read_lock() or
 * dmnsn_write_lock().
 * @param[in,out] rwlock  The read-write lock to lock.
 */
#define dmnsn_unlock_rwlock(rwlock) dmnsn_unlock_rwlock_impl((rwlock)); }

/**
 * Destroy a read-write lock, warning on failure.
 * @param[in,out] rwlock  The read-write lock to destroy.
 */
DMNSN_INTERNAL void dmnsn_destroy_rwlock(pthread_rwlock_t *rwlock);

/**
 * Initialize a condition variable, bailing out on failure.
 * @param[out] cond  The condition variable to initialize.
 */
DMNSN_INTERNAL void dmnsn_initialize_cond(pthread_cond_t *cond);

/**
 * Wait on a condition variable, bailing out on error.
 * @param[in] cond   The condition variable to wait on.
 * @param[in] mutex  The associated mutex.
 */
DMNSN_INTERNAL void dmnsn_cond_wait(pthread_cond_t *cond,
                                    pthread_mutex_t *mutex);

/**
 * Signal a condition variable, bailing out on error.
 * @param[in] cond   The condition variable to signal.
 */
DMNSN_INTERNAL void dmnsn_cond_broadcast(pthread_cond_t *cond);

/**
 * Destroy a condition variable, warning on failure.
 * @param[in,out] cond  The condition variable to destroy.
 */
DMNSN_INTERNAL void dmnsn_destroy_cond(pthread_cond_t *cond);

/**
 * Once-called callback type.
 */
typedef void dmnsn_once_fn(void);

/**
 * Call a function exactly once, bailing out on failure.
 * @param[in,out] once     The once control.
 * @param[in]     once_fn  The function to call.
 */
DMNSN_INTERNAL void dmnsn_once(pthread_once_t *once, dmnsn_once_fn *once_fn);

/**
 * Initialize a thread-local storage key, bailing out on failure.
 * @param[out] key         The key to initialize.
 * @param[in]  destructor  An optional destructor callback.
 */
DMNSN_INTERNAL void dmnsn_key_create(pthread_key_t *key,
                                     dmnsn_callback_fn *destructor);

/**
 * Set a thread-specific pointer, bailing out on failure.
 * @param[in] key    The thread-local storage key.
 * @param[in] value  The value to set.
 */
DMNSN_INTERNAL void dmnsn_setspecific(pthread_key_t key, const void *value);

/**
 * Destroy a thread-local storage key, warning on failure.
 * @param[out] key         The key to destroy.
 */
DMNSN_INTERNAL void dmnsn_key_delete(pthread_key_t key);

/**
 * Join a thread, bailing out on failure.
 * @param[in,out] thread  The thread to join.
 */
DMNSN_INTERNAL void dmnsn_join_thread(pthread_t thread, void **retval);
