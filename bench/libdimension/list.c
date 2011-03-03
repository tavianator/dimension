/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Benchmark Suite.                   *
 *                                                                       *
 * The Dimension Benchmark Suite is free software; you can redistribute  *
 * it and/or modify it under the terms of the GNU General Public License *
 * as published by the Free Software Foundation; either version 3 of the *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Benchmark Suite is distributed in the hope that it will *
 * be useful, but WITHOUT ANY WARRANTY; without even the implied         *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See *
 * the GNU General Public License for more details.                      *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "dimension.h"
#include <sandglass.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

static bool
dmnsn_comparator(const dmnsn_list_iterator *i, const dmnsn_list_iterator *j)
{
  uint32_t *a, *b;
  a = dmnsn_list_at(i);
  b = dmnsn_list_at(j);
  return *a < *b;
}

int
main(void)
{
  const size_t count = 32;
  uint32_t object = 1;

  sandglass_t sandglass;
  if (sandglass_init_monotonic(&sandglass, SANDGLASS_CPUTIME) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  /* Benchmark allocation and deallocation */
  dmnsn_list *list;
  sandglass_bench_fine(&sandglass, {
    list = dmnsn_new_list(sizeof(object));
    dmnsn_delete_list(list);
  });
  printf("dmnsn_new_list() + dmnsn_delete_list(): %ld\n", sandglass.grains);

  /* Create our test list */
  list = dmnsn_new_list(sizeof(object));

  /* dmnsn_list_push() */

  printf("dmnsn_list_push():");

  for (size_t i = 0; i < count; ++i) {
    sandglass_bench_noprecache(&sandglass, dmnsn_list_push(list, &object));
    ++object;
    printf(" %ld", sandglass.grains);
  }
  printf("\n");

  /* dmnsn_list_first() */
  dmnsn_list_iterator *ii;
  sandglass_bench_fine(&sandglass, ii = dmnsn_list_first(list));
  printf("dmnsn_list_first(): %ld\n", sandglass.grains);

  /* dmnsn_list_last() */
  sandglass_bench_fine(&sandglass, ii = dmnsn_list_last(list));
  printf("dmnsn_list_last(): %ld\n", sandglass.grains);

  /* dmnsn_list_get() */
  sandglass_bench_fine(&sandglass, dmnsn_list_get(ii, &object));
  printf("dmnsn_list_get(): %ld\n", sandglass.grains);

  /* dmnsn_list_set() */
  sandglass_bench_fine(&sandglass, dmnsn_list_set(ii, &object));
  printf("dmnsn_list_set(): %ld\n", sandglass.grains);

  /* dmnsn_list_size() */
  size_t size;
  sandglass_bench_fine(&sandglass, size = dmnsn_list_size(list));
  printf("dmnsn_list_size(): %ld\n", sandglass.grains);

  /* dmnsn_list_insert() */

  printf("dmnsn_list_insert():");

  for (size_t i = 0; i < count; ++i) {
    sandglass_bench_noprecache(&sandglass,
                               dmnsn_list_insert(list, ii, &object));
    printf(" %ld", sandglass.grains);
  }
  printf("\n");

  /* dmnsn_list_split() */
  dmnsn_list *split;
  sandglass_bench_noprecache(&sandglass, split = dmnsn_list_split(list));
  printf("dmnsn_list_split(): %ld\n", sandglass.grains);

  /* dmnsn_list_sort() */
  sandglass_bench(&sandglass, dmnsn_list_sort(list, &dmnsn_comparator));
  printf("dmnsn_list_sort(): %ld\n", sandglass.grains);

  /* dmnsn_list_remove() */

  printf("dmnsn_list_remove():");

  for (size_t i = 0; i < count; ++i) {
    ii = dmnsn_list_last(split);
    sandglass_bench_noprecache(&sandglass,
                               dmnsn_list_remove(split, ii));
    printf(" %ld", sandglass.grains);
  }
  printf("\n");

  /* dmnsn_list_pop() */

  printf("dmnsn_list_pop():");

  for (size_t i = 0; i < count; ++i) {
    sandglass_bench_noprecache(&sandglass, dmnsn_list_pop(list, &object));
    printf(" %ld", sandglass.grains);
  }
  printf("\n");

  dmnsn_delete_list(split);
  dmnsn_delete_list(list);
  return EXIT_SUCCESS;
}
