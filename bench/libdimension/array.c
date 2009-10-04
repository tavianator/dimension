/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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

#include <dimension.h>
#include <sandglass.h>
#include <stdlib.h>
#include <stdint.h>

int
main()
{
  dmnsn_array *array;
  uint32_t object = 1;
  void *ptr;
  size_t size;
  unsigned int i;
  const unsigned int count = 32;

  sandglass_t sandglass;
  sandglass_attributes_t attr = { SANDGLASS_MONOTONIC, SANDGLASS_REALTICKS };

  if (sandglass_create(&sandglass, &attr, &attr) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  /* Benchmark allocation and deallocation */
  sandglass_bench(&sandglass, {
    array = dmnsn_new_array(sizeof(object));
    dmnsn_delete_array(array);
  });
  printf("dmnsn_new_array() + dmnsn_delete_array(): %ld\n", sandglass.grains);

  /* Create our test array */
  array = dmnsn_new_array(sizeof(object));

  /* dmnsn_array_push() */

  printf("dmnsn_array_push():");

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);
  sandglass.baseline = sandglass.grains;

  for (i = 0; i < count; ++i) {
    sandglass_begin(&sandglass);
      dmnsn_array_push(array, &object);
    sandglass_elapse(&sandglass);
    sandglass.grains -= sandglass.baseline;

    printf(" %ld", sandglass.grains);
  }
  printf("\n");

  /* dmnsn_array_get() */
  sandglass_bench(&sandglass, dmnsn_array_get(array, count/2, &object));
  printf("dmnsn_array_get(): %ld\n", sandglass.grains);

  /* dmnsn_array_set() */
  sandglass_bench(&sandglass, dmnsn_array_set(array, count/2, &object));
  printf("dmnsn_array_set(): %ld\n", sandglass.grains);

  /* dmnsn_array_at() */
  sandglass_bench(&sandglass, ptr = dmnsn_array_at(array, count/2));
  printf("dmnsn_array_at(): %ld\n", sandglass.grains);

  /* dmnsn_array_size() */
  sandglass_bench(&sandglass, size = dmnsn_array_size(array));
  printf("dmnsn_array_size(): %ld\n", sandglass.grains);

  /* dmnsn_array_resize() */

  dmnsn_array_resize(array, count);

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);
  sandglass.baseline = sandglass.grains;

  sandglass_begin(&sandglass);
    dmnsn_array_resize(array, count * 2);
  sandglass_elapse(&sandglass);
  sandglass.grains -= sandglass.baseline;
  printf("dmnsn_array_resize(): %ld", sandglass.grains);

  sandglass_begin(&sandglass);
    dmnsn_array_resize(array, count);
  sandglass_elapse(&sandglass);
  sandglass.grains -= sandglass.baseline;
  printf(" %ld\n", sandglass.grains);

  /* dmnsn_array_insert() */

  printf("dmnsn_array_insert():");

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);
  sandglass.baseline = sandglass.grains;

  for (i = 0; i < count; ++i) {
    sandglass_begin(&sandglass);
      dmnsn_array_insert(array, count/2, &object);
    sandglass_elapse(&sandglass);
    sandglass.grains -= sandglass.baseline;

    printf(" %ld", sandglass.grains);
  }
  printf("\n");

  /* dmnsn_array_remove() */

  printf("dmnsn_array_remove():");

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);
  sandglass.baseline = sandglass.grains;

  for (i = 0; i < count; ++i) {
    sandglass_begin(&sandglass);
      dmnsn_array_remove(array, count/2);
    sandglass_elapse(&sandglass);
    sandglass.grains -= sandglass.baseline;

    printf(" %ld", sandglass.grains);
  }
  printf("\n");

  /* dmnsn_array_pop() */

  printf("dmnsn_array_pop():");

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);

  sandglass_begin(&sandglass);
  sandglass_elapse(&sandglass);
  sandglass.baseline = sandglass.grains;

  for (i = 0; i < count; ++i) {
    sandglass_begin(&sandglass);
      dmnsn_array_pop(array, &object);
    sandglass_elapse(&sandglass);
    sandglass.grains -= sandglass.baseline;

    printf(" %ld", sandglass.grains);
  }
  printf("\n");

  dmnsn_array_delet(array);
  return EXIT_SUCCESS;
}
