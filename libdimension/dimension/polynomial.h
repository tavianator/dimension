/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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

/*
 * Utility functions for working with and numerically solving polynomials.
 * Polynomials are represented as simple arrays where the ith element is the
 * coefficient on x^i.  In general, we are only interested in positive roots.
 */

#ifndef DIMENSION_POLYNOMIAL_H
#define DIMENSION_POLYNOMIAL_H

#include <stddef.h>
#include <stdio.h>

DMNSN_INLINE double
dmnsn_evaluate_polynomial(double poly[], size_t degree, double x)
{
  double ret = poly[degree];
  ssize_t i;
  for (i = degree - 1; i >= 0; --i) {
    ret = ret*x + poly[i];
  }
  return ret;
}

/* Stores the non-negative roots of poly[] in x[], and returns the number of
   such roots that were stored */
size_t dmnsn_solve_polynomial(double poly[], size_t degree, double x[]);

/* Helper function to print a polynomial */
void dmnsn_print_polynomial(FILE *file, double poly[], size_t degree);

#endif /* DIMENSION_POLYNOMIAL_H */