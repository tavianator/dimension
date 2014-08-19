/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
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

#include "dimension/model.h"

// Allocate a dummy pigment
dmnsn_pigment *
dmnsn_new_pigment(dmnsn_pool *pool)
{
  dmnsn_pigment *pigment = DMNSN_PALLOC(pool, dmnsn_pigment);
  dmnsn_init_pigment(pigment);
  return pigment;
}

// Initialize a pigment
void
dmnsn_init_pigment(dmnsn_pigment *pigment)
{
  pigment->pigment_fn = NULL;
  pigment->initialize_fn = NULL;
  pigment->trans = dmnsn_identity_matrix();
  pigment->quick_color = DMNSN_TCOLOR(dmnsn_black);
  pigment->initialized = false;
}

// Precompute pigment properties
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

// Evaluate a pigment
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
