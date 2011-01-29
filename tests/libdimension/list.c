/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
 *                                                                       *
 * The Dimension Test Suite is free software; you can redistribute it    *
 * and/or modify it under the terms of the GNU General Public License as *
 * published by the Free Software Foundation; either version 3 of the    *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Test Suite is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

/*
 * Basic tests of linked lists
 */

#include "dimension.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct dmnsn_item {
  int value;
  size_t seq;
} dmnsn_item;

static bool
dmnsn_comparator(const dmnsn_list_iterator *i, const dmnsn_list_iterator *j)
{
  dmnsn_item *a = dmnsn_list_at(i), *b = dmnsn_list_at(j);
  return a->value < b->value;
}

int
main()
{
  /* Set the resilience low for tests */
  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  dmnsn_list *list = dmnsn_new_list(sizeof(dmnsn_item));

  /* Fill up the list with random numbers */
  srand(1);
  const size_t list_size = 10000;
  for (size_t i = 0; i < list_size; ++i) {
    dmnsn_item item;
    item.value = rand()%(list_size/10);
    item.seq = i;
    dmnsn_list_push(list, &item);
  }

  /* Ensure that sorting works */
  dmnsn_list_sort(list, &dmnsn_comparator);
  for (dmnsn_list_iterator *i = dmnsn_list_first(list);
       i != dmnsn_list_last(list);
       i = dmnsn_list_next(i))
  {
    dmnsn_item *a = dmnsn_list_at(i), *b = dmnsn_list_at(dmnsn_list_next(i));
    if (a->value > b->value) {
      fprintf(stderr, "--- Sorting failed (%d > %d)! ---\n",
              a->value, b->value);
      return EXIT_FAILURE;
    } else if (a->value == b->value && a->seq > b->seq) {
      fprintf(stderr, "--- Sorting was unstable (%zu > %zu)! ---\n",
              a->seq, b->seq);
      return EXIT_FAILURE;
    }
  }

  dmnsn_delete_list(list);
  return EXIT_SUCCESS;
}
