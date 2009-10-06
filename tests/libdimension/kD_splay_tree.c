/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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
 * Basic tests of our fancy k-D splay tree framework
 */

#include "../../libdimension/dimension_impl.h"
#include <stdlib.h>

int
main()
{
  dmnsn_kD_splay_node *tree;
  dmnsn_object *obj1, *obj2, *obj3;

  obj1 = dmnsn_new_object();
  obj2 = dmnsn_new_object();
  obj3 = dmnsn_new_object();

  obj1->min = dmnsn_vector_construct(0.0, 0.0, 0.0);
  obj1->max = dmnsn_vector_construct(1.0, 1.0, 1.0);

  obj2->min = dmnsn_vector_construct(-2.0, -2.0, -2.0);
  obj2->max = dmnsn_vector_construct(1.0, 1.0, 1.0);

  obj3->min = dmnsn_vector_construct(-1.0, -1.0, -1.0);
  obj3->max = dmnsn_vector_construct(0.0, 0.0, 0.0);

  tree = dmnsn_new_kD_splay_tree();

  tree = dmnsn_kD_splay_insert(tree, obj1);
  if (tree->object != obj1) {
    fprintf(stderr, "Wrong kD splay tree built.\n");
    return EXIT_FAILURE;
  }

  tree = dmnsn_kD_splay_insert(tree, obj2);
  if (tree->object != obj2 || tree->left->object != obj1) {
    fprintf(stderr, "Wrong kD splay tree built.\n");
    return EXIT_FAILURE;
  }

  tree = dmnsn_kD_splay_insert(tree, obj3);
  if (tree->object != obj3 || tree->left->object != obj1
      || tree->right->object != obj2) {
    fprintf(stderr, "Wrong kD splay tree built.\n");
    return EXIT_FAILURE;
  }

  dmnsn_delete_object(obj3);
  dmnsn_delete_object(obj2);
  dmnsn_delete_object(obj1);
  dmnsn_delete_kD_splay_tree(tree);
  return EXIT_SUCCESS;
}
