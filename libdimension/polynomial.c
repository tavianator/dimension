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

#include "dimension.h"
#include <math.h>

/* Basic solving methods */

static inline size_t
dmnsn_solve_linear(const double poly[2], double x[1])
{
  x[0] = -poly[0]/poly[1];
  return (x[0] > 0.0) ? 1 : 0;
}

static inline size_t
dmnsn_solve_quadratic(const double poly[3], double x[2])
{
  double disc = poly[1]*poly[1] - 4.0*poly[0]*poly[2];
  if (disc >= 0.0) {
    double s = sqrt(disc);
    x[0] = (-poly[1] + s)/(2.0*poly[2]);
    x[1] = (-poly[1] - s)/(2.0*poly[2]);
    return (x[0] > 0.0) ? ((x[1] > 0.0) ? 2 : 1) : 0;
  } else {
    return 0;
  }
}

/* Eliminate trivial zero roots from poly[] */
static inline void
dmnsn_eliminate_zero_roots(double poly[], size_t *degree)
{
  size_t i, deg = *degree;
  for (i = 0; i <= deg && poly[i] == 0.0; ++i);

  if (i > 0) {
    for (size_t j = 0; j + i <= deg; ++j) {
      poly[j] = poly[j + i];
    }
  }

  *degree -= i;
}

/* Get the real degree of a polynomial, ignoring leading zeros */
static inline size_t
dmnsn_real_degree(const double poly[], size_t degree)
{
  for (ssize_t i = degree; i >= 0; ++i) {
    if (poly[i] != 0.0) {
      return i;
    }
  }

  return 0;
}

/* Use the false position method to find a root in a range that contains exactly
   one root */
static inline double
dmnsn_bisect_root(const double poly[], size_t degree, double min, double max)
{
  double evmin = dmnsn_evaluate_polynomial(poly, degree, min);
  double evmax = dmnsn_evaluate_polynomial(poly, degree, max);
  double mid = 0.0, evmid;
  int lastsign = -1;

  if (max - min <= dmnsn_epsilon)
    return min;

  do {
    mid = (min*evmax - max*evmin)/(evmax - evmin);
    evmid = dmnsn_evaluate_polynomial(poly, degree, mid);
    int sign = dmnsn_signbit(evmid);
    if (sign == dmnsn_signbit(evmax)) {
      max = mid;
      evmax = evmid;
      if (sign == lastsign) {
        /* Don't allow the algorithm to keep the same endpoint for three
           iterations in a row; this ensures superlinear convergence */
        evmin /= 2.0;
      }
    } else {
      min = mid;
      evmin = evmid;
      if (sign == lastsign) {
        evmax /= 2.0;
      }
    }

    lastsign = sign;
  } while (fabs(evmid) > fabs(mid)*dmnsn_epsilon);

  return mid;
}

/* Use synthetic division to eliminate the root `r' from poly[] */
static inline void
dmnsn_eliminate_root(double poly[], size_t *degree, double r)
{
  double rem = poly[*degree];
  for (ssize_t i = *degree - 1; i >= 0; --i) {
    double temp = poly[i];
    poly[i] = rem;
    rem = temp + r*rem;
  }
  --*degree;
}

/* Returns the number of sign changes between coefficients of `poly' */
static inline size_t
dmnsn_descartes_rule(const double poly[], size_t degree)
{
  int lastsign = 0;
  size_t i;
  for (i = 0; i <= degree; ++i) {
    if (poly[i] != 0.0) {
      lastsign = dmnsn_signbit(poly[i]);
      break;
    }
  }

  size_t changes = 0;
  for (++i; i <= degree; ++i) {
    int sign = dmnsn_signbit(poly[i]);
    if (poly[i] != 0.0 && sign != lastsign) {
      lastsign = sign;
      ++changes;
    }
  }

  return changes;
}

#define DMNSN_NBINOM 11
static const double dmnsn_pascals_triangle[DMNSN_NBINOM][DMNSN_NBINOM] = {
  { 1.0 },
  { 1.0, 1.0 },
  { 1.0, 2.0, 1.0 },
  { 1.0, 3.0, 3.0, 1.0 },
  { 1.0, 4.0, 6.0, 4.0, 1.0 },
  { 1.0, 5.0, 10.0, 10.0, 5.0, 1.0 },
  { 1.0, 6.0, 15.0, 20.0, 15.0, 6.0, 1.0 },
  { 1.0, 7.0, 21.0, 35.0, 35.0, 21.0, 7.0, 1.0 },
  { 1.0, 8.0, 28.0, 56.0, 70.0, 56.0, 28.0, 8.0, 1.0 },
  { 1.0, 9.0, 36.0, 84.0, 126.0, 126.0, 84.0, 36.0, 9.0, 1.0 },
  { 1.0, 10.0, 45.0, 120.0, 210.0, 252.0, 210.0, 120.0, 45.0, 10.0, 1.0 }
};

/* Get the (n k) binomial coefficient */
static inline double
dmnsn_binom(size_t n, size_t k)
{
  if (n < DMNSN_NBINOM && k < DMNSN_NBINOM) {
    return dmnsn_pascals_triangle[n][k];
  } else {
    if (k > n - k) {
      k = n - k;
    }

    double ret = 1.0;
    for (size_t i = 0; i < k; ++i) {
      ret *= n - i;
      ret /= i + 1;
    }

    return ret;
  }
}

/* Find all ranges that contain a single root, with Uspensky's algorithm */
static size_t
dmnsn_uspensky_bounds(const double poly[], size_t degree, double bounds[][2],
                      size_t max_roots)
{
  size_t signchanges = dmnsn_descartes_rule(poly, degree);
  if (signchanges == 0) {
    return 0;
  } else if (signchanges == 1) {
    bounds[0][0] = +0.0;
    bounds[0][1] = INFINITY;
    return 1;
  } else {
    size_t n = 0;

    /* a[] is the expanded form of poly(x + 1), b[] is the expanded form of
       (x + 1)^degree * poly(1/(x + 1)) */
    double a[degree], b[degree];
    for (size_t i = 0; i <= degree; ++i) {
      a[i] = poly[i];
      b[i] = poly[degree - i];
      for (size_t j = i + 1; j <= degree; ++j) {
        double binom = dmnsn_binom(j, i);
        a[i] += binom*poly[j];
        b[i] += binom*poly[degree - j];
      }
    }

    /* Test for a root at 1.0 */
    if (a[0] == 0.0) {
      bounds[n][0] = 1.0;
      bounds[n][1] = 1.0;
      ++n;
      if (n == max_roots)
        return n;
    }

    /* Recursively test for roots in b[] */
    size_t nb = dmnsn_uspensky_bounds(b, degree, bounds + n, max_roots - n);
    for (size_t i = n; i < n + nb; ++i) {
      double temp = bounds[i][0];
      bounds[i][0] = 1.0/(bounds[i][1] + 1.0);
      bounds[i][1] = 1.0/(temp + 1.0);
    }
    n += nb;
    if (n == max_roots)
      return n;

    /* Recursively test for roots in a[] */
    size_t na = dmnsn_uspensky_bounds(a, degree, bounds + n, max_roots - n);
    for (size_t i = n; i < n + na; ++i) {
      bounds[i][0] += 1.0;
      bounds[i][1] += 1.0;
    }
    n += na;

    return n;
  }
}

/* Modified Uspensky's algorithm */
size_t
dmnsn_solve_polynomial(const double poly[], size_t degree, double x[])
{
  /* Copy the polynomial so we can be destructive */
  double p[degree + 1];
  for (ssize_t i = degree; i >= 0; --i) {
    p[i] = poly[i];
  }

  /* Account for leading zero coefficients */
  degree = dmnsn_real_degree(p, degree);
  /* Eliminate simple zero roots */
  dmnsn_eliminate_zero_roots(p, &degree);

  size_t i = 0; /* Index into x[] */

  if (degree >= 3) {
    /* Find isolating intervals for degree - 2 roots of p[] */
    double ranges[degree - 2][2];
    size_t n = dmnsn_uspensky_bounds(p, degree, ranges, degree - 2);

    /* Calculate an finite upper bound for the roots of p[] */
    double absmax = 0.0;
    for (size_t j = 0; j < degree; ++j) {
      absmax = dmnsn_max(absmax, fabs(p[j]));
    }
    absmax /= fabs(p[degree]);
    absmax += 1.0;

    for (size_t j = 0; j < n; ++j) {
      /* Replace large or infinite upper bounds with a finite one */
      ranges[j][1] = dmnsn_min(ranges[j][1], absmax);

      /* Bisect within the found range */
      double r = dmnsn_bisect_root(p, degree, ranges[j][0], ranges[j][1]);

      /* Use synthetic division to eliminate the root `r' */
      dmnsn_eliminate_root(p, &degree, r);

      /* Store the found root */
      x[i] = r;
      ++i;
    }
  }

  if (degree == 1) {
    i += dmnsn_solve_linear(p, x + i);
  } else if (degree == 2) {
    i += dmnsn_solve_quadratic(p, x + i);
  }

  return i;
}

void
dmnsn_print_polynomial(FILE *file, const double poly[], size_t degree)
{
  for (ssize_t i = degree; i >= 0; --i) {
    if (i < degree) {
      fprintf(file, (poly[i] >= 0.0) ? " + " : " - ");
    }
    fprintf(file, "%.15g", fabs(poly[i]));
    if (i >= 2) {
      fprintf(file, "*x^%zu", i);
    } else if (i == 1) {
      fprintf(file, "*x");
    }
  }
}
