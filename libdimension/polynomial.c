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

/**
 * @file
 * Polynomials.
 */

#include "dimension.h"
#include <math.h>

/** Get the real degree of a polynomial, ignoring leading zeros. */
static inline size_t
dmnsn_real_degree(const double poly[], size_t degree)
{
  for (size_t i = degree + 1; i-- > 0;) {
    if (fabs(poly[i]) >= dmnsn_epsilon) {
      return i;
    }
  }

  return 0;
}

/** Divide each coefficient by the leading coefficient. */
static inline void
dmnsn_normalize_polynomial(double poly[], size_t degree)
{
  for (size_t i = 0; i < degree; ++i) {
    poly[i] /= poly[degree];
  }
  poly[degree] = 1.0;
}

/** Eliminate trivial zero roots from \p poly[]. */
static inline void
dmnsn_eliminate_zero_roots(double poly[], size_t *degree)
{
  size_t i, deg = *degree;
  for (i = 0; i <= deg && fabs(poly[i]) < dmnsn_epsilon; ++i);

  if (i > 0) {
    for (size_t j = 0; j + i <= deg; ++j) {
      poly[j] = poly[j + i];
    }
  }

  *degree -= i;
}

/** Returns the number of sign changes between coefficients of \p poly[]. */
static inline size_t
dmnsn_descartes_rule(const double poly[], size_t degree)
{
  int lastsign = 0;
  size_t i;
  for (i = 0; i <= degree; ++i) {
    if (fabs(poly[i]) >= dmnsn_epsilon) {
      lastsign = dmnsn_signbit(poly[i]);
      break;
    }
  }

  size_t changes = 0;
  for (++i; i <= degree; ++i) {
    int sign = dmnsn_signbit(poly[i]);
    if (fabs(poly[i]) >= dmnsn_epsilon && sign != lastsign) {
      lastsign = sign;
      ++changes;
    }
  }

  return changes;
}

/** How many levels of Pascal's triangle to precompute. */
#define DMNSN_NBINOM 11

/** Pre-computed values of Pascal's triangle. */
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

/** Compute the (n k) binomial coefficient. */
static inline double
dmnsn_binom(size_t n, size_t k)
{
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

/** Find ranges that contain a single root, with Uspensky's algorithm. */
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
    /* Number of roots found so far */
    size_t n = 0;

    /* First divide poly[] by (x - 1) to test for a root at x = 1 */
    double pdiv1[degree], rem = poly[degree];
    for (size_t i = degree; i-- > 0;) {
      pdiv1[i] = rem;
      rem += poly[i];
    }
    if (fabs(rem) < dmnsn_epsilon) {
      bounds[n][0] = 1.0;
      bounds[n][1] = 1.0;
      ++n;
      if (n == max_roots)
        return n;

      --degree;
      poly = pdiv1;
    }

    /* a[] is the expanded form of poly(x + 1), b[] is the expanded form of
       (x + 1)^degree * poly(1/(x + 1)) */
    double a[degree + 1], b[degree + 1];
    if (degree < DMNSN_NBINOM) {
      /* Use precomputed binomial coefficients if possible */
      for (size_t i = 0; i <= degree; ++i) {
        a[i] = poly[i];
        b[i] = poly[degree - i];
        for (size_t j = i + 1; j <= degree; ++j) {
          double binom = dmnsn_pascals_triangle[j][i];
          a[i] += binom*poly[j];
          b[i] += binom*poly[degree - j];
        }
      }
    } else {
      for (size_t i = 0; i <= degree; ++i) {
        a[i] = poly[i];
        b[i] = poly[degree - i];
        for (size_t j = i + 1; j <= degree; ++j) {
          double binom = dmnsn_binom(j, i);
          a[i] += binom*poly[j];
          b[i] += binom*poly[degree - j];
        }
      }
    }

    /* Recursively test for roots in b[] */
    size_t nb = dmnsn_uspensky_bounds(b, degree, bounds + n, max_roots - n);
    for (size_t i = n; i < n + nb; ++i) {
      /* Transform the found roots of b[] into roots of poly[] */
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
      /* Transform the found roots of a[] into roots of poly[] */
      bounds[i][0] += 1.0;
      bounds[i][1] += 1.0;
    }
    n += na;

    return n;
  }
}

/** Calculate a finite upper bound for the roots of \p poly[]. */
static inline double
dmnsn_root_bound(double poly[], size_t degree)
{
  double bound = 0.0;
  for (size_t i = 0; i < degree; ++i) {
    bound = dmnsn_max(bound, fabs(poly[i]));
  }
  bound /= fabs(poly[degree]);
  bound += 1.0;
  return bound;
}

/** Improve a root with Newton's method. */
static inline double
dmnsn_improve_root(const double poly[], size_t degree, double x)
{
  double p;
  do {
    /* Calculate the value of the polynomial and its derivative at once */
    p = poly[degree];
    double dp = 0.0;
    for (size_t i = degree; i-- > 0;) {
      dp = dp*x + p;
      p  = p*x  + poly[i];
    }

    x -= p/dp;
  } while (fabs(p) >= fabs(x)*dmnsn_epsilon);

  return x;
}

/** Use the false position method to find a root in a range that contains
    exactly one root. */
static inline double
dmnsn_bisect_root(const double poly[], size_t degree, double min, double max)
{
  if (max - min < dmnsn_epsilon) {
    /* Bounds are equal, use Newton's method */
    return dmnsn_improve_root(poly, degree, min);
  }

  double evmin = dmnsn_evaluate_polynomial(poly, degree, min);
  double evmax = dmnsn_evaluate_polynomial(poly, degree, max);
  double initial_evmin = fabs(evmin), initial_evmax = fabs(evmax);
  double mid = 0.0, evmid;
  int lastsign = -1;

  do {
    mid = (min*evmax - max*evmin)/(evmax - evmin);
    evmid = dmnsn_evaluate_polynomial(poly, degree, mid);
    int sign = dmnsn_signbit(evmid);

    if (mid < min) {
      /* This can happen due to numerical instability in the root bounding
         algorithm, so behave like the normal secant method */
      max = min;
      evmax = evmin;
      min = mid;
      evmin = evmid;
    } else if (mid > max) {
      min = max;
      evmin = evmax;
      max = mid;
      evmax = evmid;
    } else if (sign == dmnsn_signbit(evmax)) {
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
  } while (fabs(evmid) >= fabs(mid)*dmnsn_epsilon
           /* These conditions improve stability when one of the bounds is
              close to a different root than we are trying to find */
           || fabs(evmid) > initial_evmin
           || fabs(evmid) > initial_evmax);

  return mid;
}

/** Use synthetic division to eliminate the root \p r from \p poly[]. */
static inline void
dmnsn_eliminate_root(double poly[], size_t *degree, double r)
{
  size_t deg = *degree;
  double rem = poly[deg];
  for (size_t i = deg; i-- > 0;) {
    double temp = poly[i];
    poly[i] = rem;
    rem = temp + r*rem;
  }
  --*degree;
}

/** Solve a normalized linear polynomial algebraically. */
static inline size_t
dmnsn_solve_linear(const double poly[2], double x[1])
{
  x[0] = -poly[0];
  return (x[0] >= dmnsn_epsilon) ? 1 : 0;
}

/** Solve a normalized quadratic polynomial algebraically. */
static inline size_t
dmnsn_solve_quadratic(const double poly[3], double x[2])
{
  double disc = poly[1]*poly[1] - 4.0*poly[0];
  if (disc >= 0.0) {
    double s = sqrt(disc);
    x[0] = (-poly[1] + s)/2.0;
    x[1] = (-poly[1] - s)/2.0;
    return (x[0] >= dmnsn_epsilon) ? ((x[1] >= dmnsn_epsilon) ? 2 : 1) : 0;
  } else {
    return 0;
  }
}

/* Uspensky's algorithm */
size_t
dmnsn_solve_polynomial(const double poly[], size_t degree, double x[])
{
  /* Copy the polynomial so we can be destructive */
  double p[degree + 1];
  for (size_t i = 0; i <= degree; ++i) {
    p[i] = poly[i];
  }

  /* Index into x[] */
  size_t i = 0;

  /* Account for leading zero coefficients */
  degree = dmnsn_real_degree(p, degree);
  /* Normalize the leading coefficient to 1.0 */
  dmnsn_normalize_polynomial(p, degree);
  /* Eliminate simple zero roots */
  dmnsn_eliminate_zero_roots(p, &degree);

  if (degree >= 3) {
    /* Find isolating intervals for (degree - 2) roots of p[] */
    double ranges[degree - 2][2];
    size_t n = dmnsn_uspensky_bounds(p, degree, ranges, degree - 2);

    /* Calculate a finite upper bound for the roots of p[] */
    double absmax = dmnsn_root_bound(p, degree);

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

  switch (degree) {
  case 1:
    i += dmnsn_solve_linear(p, x + i);
    break;
  case 2:
    i += dmnsn_solve_quadratic(p, x + i);
    break;
  }

  return i;
}

/* Print a polynomial */
void
dmnsn_print_polynomial(FILE *file, const double poly[], size_t degree)
{
  for (size_t i = degree + 1; i-- > 0;) {
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
