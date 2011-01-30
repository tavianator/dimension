/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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

#include "dimension-impl.h"
#include <pthread.h>
#include <stdio.h>
#include <inttypes.h>

/** Info for one predicted branch. */
typedef struct {
  char *location;
  uint64_t predicted, branches;
} dmnsn_profile_info;

/** Count of mispredicted branches. */
static dmnsn_dictionary *dmnsn_profile_data = NULL;
/** Mutex which protects \c dmnsn_profile_data. */
static pthread_mutex_t dmnsn_profile_mutex = PTHREAD_MUTEX_INITIALIZER;

bool
dmnsn_expect(bool result, bool expected, const char *func, const char *file,
             unsigned int line)
{
  int size = snprintf(NULL, 0, "%s:%s:%u", file, func, line) + 1;
  if (size < 1) {
    dmnsn_error(DMNSN_SEVERITY_MEDIUM, "sprintf() failed.");
  } else {
    char key[size];
    if (snprintf(key, size, "%s:%s:%u", file, func, line) > 0) {
      if (pthread_mutex_lock(&dmnsn_profile_mutex) != 0) {
        dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't lock mutex.");
      }

      if (!dmnsn_profile_data) {
        dmnsn_profile_data = dmnsn_new_dictionary(sizeof(dmnsn_profile_info));
      }
      dmnsn_profile_info *info = dmnsn_dictionary_at(dmnsn_profile_data, key);
      if (info) {
        ++info->branches;
        if (result == expected) {
          ++info->predicted;
        }
      } else {
        dmnsn_profile_info info = {
          .location  = dmnsn_strdup(key),
          .predicted = (result == expected) ? 1 : 0,
          .branches  = 1
        };
        dmnsn_dictionary_insert(dmnsn_profile_data, key, &info);
      }

      if (pthread_mutex_unlock(&dmnsn_profile_mutex) != 0) {
        dmnsn_error(DMNSN_SEVERITY_MEDIUM, "Couldn't lock mutex.");
      }
    } else {
      dmnsn_error(DMNSN_SEVERITY_MEDIUM, "sprintf() failed.");
    }
  }

  return result;
}

static void
dmnsn_print_bad_prediction(void *ptr)
{
  dmnsn_profile_info *info = ptr;
  double rate = ((double)info->predicted)/info->branches;
  if (rate < 0.75 || info->branches < 100000) {
    fprintf(stderr,
            "Bad branch prediction: %s: %" PRIu64 "/%" PRIu64 " (%g%%)\n",
            info->location, info->predicted, info->branches, 100.0*rate);
  }

  dmnsn_free(info->location);
}

static void __attribute__((destructor))
dmnsn_print_bad_predictions(void)
{
  if (dmnsn_profile_data) {
    dmnsn_dictionary_apply(dmnsn_profile_data, &dmnsn_print_bad_prediction);
    dmnsn_delete_dictionary(dmnsn_profile_data);
  }
}
