/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Dynamic memory.
 */

#include "dimension-internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#ifndef NDEBUG
static atomic_size_t dmnsn_allocs = ATOMIC_VAR_INIT(0);
#endif

void *
dmnsn_malloc(size_t size)
{
  void *ptr = malloc(size);
  if (!ptr) {
    dmnsn_error("Memory allocation failed.");
  }

#ifndef NDEBUG
  atomic_fetch_add(&dmnsn_allocs, 1);
#endif

  return ptr;
}

void *
dmnsn_realloc(void *ptr, size_t size)
{
#ifndef NDEBUG
  if (!ptr) {
    atomic_fetch_add(&dmnsn_allocs, 1);
  }
#endif

  ptr = realloc(ptr, size);
  if (!ptr) {
    dmnsn_error("Memory allocation failed.");
  }
  return ptr;
}

char *
dmnsn_strdup(const char *s)
{
  char *copy = dmnsn_malloc(strlen(s) + 1);
  strcpy(copy, s);
  return copy;
}

void
dmnsn_free(void *ptr)
{
#ifndef NDEBUG
  if (ptr) {
    atomic_fetch_sub(&dmnsn_allocs, 1);
  }
#endif

  free(ptr);
}

#ifndef NDEBUG
DMNSN_LATE_DESTRUCTOR static void
dmnsn_leak_check(void)
{
  if (atomic_load_explicit(&dmnsn_allocs, memory_order_relaxed) > 0) {
    dmnsn_warning("Leaking memory.");
  }
}
#endif
