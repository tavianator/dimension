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
 * Branch profile accounting.
 */

#include "internal.h"
#include "internal/concurrency.h"
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>

/// Information on one predicted branch.
typedef struct {
  char *location;
  uint64_t predicted, branches;
} dmnsn_branch;

/// Count of mispredicted branches.
static dmnsn_dictionary *dmnsn_profile = NULL;
/// Mutex which protects \c dmnsn_profile.
static pthread_mutex_t dmnsn_profile_mutex = PTHREAD_MUTEX_INITIALIZER;

/// Thread-local count of mispredicted branches.
static pthread_key_t dmnsn_thread_profile;
/// Initialize the thread-specific pointer exactly once.
static pthread_once_t dmnsn_thread_profile_once = PTHREAD_ONCE_INIT;

/// Add thread-specific profile data to the global counts.
static void
dmnsn_profile_globalize(void *ptr)
{
  dmnsn_branch *branch = ptr;
  dmnsn_branch *global = dmnsn_dictionary_at(dmnsn_profile, branch->location);
  if (global) {
    global->predicted += branch->predicted;
    global->branches  += branch->branches;
    dmnsn_free(branch->location);
  } else {
    dmnsn_dictionary_insert(dmnsn_profile, branch->location, branch);
  }
}

/// Destructor function for thread-specific profile data.
static void
dmnsn_delete_thread_profile(void *ptr)
{
  dmnsn_dictionary *thread_profile = ptr;

  dmnsn_lock_mutex(&dmnsn_profile_mutex);
    dmnsn_dictionary_apply(thread_profile, dmnsn_profile_globalize);
  dmnsn_unlock_mutex(&dmnsn_profile_mutex);

  dmnsn_delete_dictionary(thread_profile);
}

/// Initialize the thread-specific pointer.
static void
dmnsn_initialize_thread_profile(void)
{
  dmnsn_key_create(&dmnsn_thread_profile, dmnsn_delete_thread_profile);

  dmnsn_lock_mutex(&dmnsn_profile_mutex);
    dmnsn_profile = dmnsn_new_dictionary(sizeof(dmnsn_branch));
  dmnsn_unlock_mutex(&dmnsn_profile_mutex);
}

/// Get the thread-specific profile data.
static dmnsn_dictionary *
dmnsn_get_thread_profile(void)
{
  dmnsn_once(&dmnsn_thread_profile_once, dmnsn_initialize_thread_profile);
  return pthread_getspecific(dmnsn_thread_profile);
}

/// Set the thread-specific profile data.
static void
dmnsn_set_thread_profile(dmnsn_dictionary *thread_profile)
{
  dmnsn_setspecific(dmnsn_thread_profile, thread_profile);
}

bool
dmnsn_expect(bool result, bool expected, const char *func, const char *file,
             unsigned int line)
{
  int size = snprintf(NULL, 0, "%s:%s:%u", file, func, line) + 1;
  if (size < 1) {
    dmnsn_error("sprintf() failed.");
  }

  char key[size];
  if (snprintf(key, size, "%s:%s:%u", file, func, line) < 0) {
    dmnsn_error("sprintf() failed.");
  }

  dmnsn_dictionary *thread_profile = dmnsn_get_thread_profile();
  if (!thread_profile) {
    thread_profile = dmnsn_new_dictionary(sizeof(dmnsn_branch));
    dmnsn_set_thread_profile(thread_profile);
  }

  dmnsn_branch *branch = dmnsn_dictionary_at(thread_profile, key);
  if (branch) {
    ++branch->branches;
    if (result == expected) {
      ++branch->predicted;
    }
  } else {
    dmnsn_branch new_branch = {
      .location  = dmnsn_strdup(key),
      .predicted = (result == expected) ? 1 : 0,
      .branches  = 1
    };
    dmnsn_dictionary_insert(thread_profile, key, &new_branch);
  }

  return result;
}

static void
dmnsn_print_bad_prediction(void *ptr)
{
  dmnsn_branch *branch = ptr;
  double rate = ((double)branch->predicted)/branch->branches;
  if (rate < 0.75 || branch->branches < 100000) {
    fprintf(stderr,
            "Bad branch prediction: %s: %" PRIu64 "/%" PRIu64 " (%g%%)\n",
            branch->location, branch->predicted, branch->branches, 100.0*rate);
  }

  dmnsn_free(branch->location);
}

DMNSN_DESTRUCTOR static void
dmnsn_print_bad_predictions(void)
{
  dmnsn_dictionary *thread_profile = dmnsn_get_thread_profile();
  if (thread_profile) {
    dmnsn_delete_thread_profile(thread_profile);
    dmnsn_set_thread_profile(NULL);
  }

  dmnsn_lock_mutex(&dmnsn_profile_mutex);
    dmnsn_dictionary_apply(dmnsn_profile, dmnsn_print_bad_prediction);
    dmnsn_delete_dictionary(dmnsn_profile);
    dmnsn_profile = NULL;
  dmnsn_unlock_mutex(&dmnsn_profile_mutex);
}
