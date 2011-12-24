/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Pigments.
 */

#include "dimension-internal.h"

/* Allocate a dummy pigment */
dmnsn_pigment *
dmnsn_new_pigment(void)
{
  dmnsn_pigment *pigment = dmnsn_malloc(sizeof(dmnsn_pigment));
  pigment->pigment_fn    = NULL;
  pigment->initialize_fn = NULL;
  pigment->free_fn       = NULL;
  pigment->trans         = dmnsn_identity_matrix();
  pigment->quick_color   = DMNSN_TCOLOR(dmnsn_black);
  pigment->initialized   = false;
  DMNSN_REFCOUNT_INIT(pigment);
  return pigment;
}

/* Free a pigment */
void
dmnsn_delete_pigment(dmnsn_pigment *pigment)
{
  if (DMNSN_DECREF(pigment)) {
    if (pigment->free_fn) {
      pigment->free_fn(pigment->ptr);
    }
    dmnsn_free(pigment);
  }
}

/* Precompute pigment properties */
void
dmnsn_pigment_initialize(dmnsn_pigment *pigment)
{
  dmnsn_assert(!pigment->initialized, "Pigment double-initialized.");
  pigment->initialized = true;

  if (pigment->initialize_fn) {
    pigment->initialize_fn(pigment);
  }

  pigment->trans_inv = dmnsn_matrix_inverse(pigment->trans);
}

/* Evaluate a pigment */
dmnsn_tcolor
dmnsn_pigment_evaluate(const dmnsn_pigment *pigment, dmnsn_vector v)
{
  if (pigment->pigment_fn) {
    dmnsn_vector v_trans = dmnsn_transform_point(pigment->trans_inv, v);
    return pigment->pigment_fn(pigment, v_trans);
  } else {
    return pigment->quick_color;
  }
}
