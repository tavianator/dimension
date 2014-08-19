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
 * Utility functions for working with and numerically solving polynomials.
 * Polynomials are represented as simple arrays where the ith element is the
 * coefficient on x^i.  In general, we are only interested in positive roots.
 */

#include "internal.h"
#include <stddef.h>
#include <stdio.h>

/**
 * Evaluate a polynomial at \p x.
 * @param[in] poly    The coefficients of the polynomial to evaluate, in order
 *                    from lowest degree to highest degree.  The array should
 *                    have dimension <tt>degree + 1</tt>.
 * @param[in] degree  The degree of the polynomial.
 * @param[in] x       The value of the variable at which to evaluate.
 */
DMNSN_INTERNAL DMNSN_INLINE double
dmnsn_polynomial_evaluate(const double poly[], size_t degree, double x)
{
  double ret = poly[degree];
  size_t i;
  for (i = degree; i-- > 0;) {
    ret = ret*x + poly[i];
  }
  return ret;
}

/**
 * Evaluate the derivative of a polynomial at \p x.
 * @param[in] poly    The coefficients of the polynomial to evaluate.
 * @param[in] degree  The degree of the polynomial.
 * @param[in] x       The value of the variable at which to evaluate.
 */
DMNSN_INTERNAL DMNSN_INLINE double
dmnsn_polynomial_evaluate_derivative(const double poly[], size_t degree, double x)
{
  double ret = poly[degree]*degree;
  size_t i;
  for (i = degree - 1; i >= 1; --i) {
    ret = ret*x + poly[i]*i;
  }
  return ret;
}

/**
 * Find the positive roots of a polynomial.
 * @param[in]  poly    The coefficients of the polynomial to solve.
 * @param[in]  degree  The degree of the polynomial.
 * @param[out] x       An array in which to store the roots.  It should have
 *                     dimension \p degree.
 * @return The number of positive roots stored in \c x[].
 */
DMNSN_INTERNAL size_t dmnsn_polynomial_solve(const double poly[], size_t degree, double x[]);

/**
 * Output a polynomial.  The polynomial is printed as a function of x suitable
 * for input into a CAS, and without a trailing newline.
 * @param[in,out] file    The file to write to.
 * @param[in]     poly    The coefficients of the polynomial to print.
 * @param[in]     degree  The degree of the polynomial.
 */
DMNSN_INTERNAL void dmnsn_polynomial_print(FILE *file, const double poly[], size_t degree);
