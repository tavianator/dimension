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
dmnsn_solve_linear(double poly[2], double x[1])
{
  x[0] = -poly[0]/poly[1];
  return (x[0] >= 0.0) ? 1 : 0;
}

static inline size_t
dmnsn_solve_quadratic(double poly[3], double x[2])
{
  double disc = poly[1]*poly[1] - 4.0*poly[0]*poly[2];
  if (disc >= 0.0) {
    double s = sqrt(disc);
    x[0] = (-poly[1] + s)/(2.0*poly[2]);
    x[1] = (-poly[1] - s)/(2.0*poly[2]);
    return (x[0] >= 0.0) ? ((x[1] >= 0.0) ? 2 : 1) : 0;
  } else {
    return 0;
  }
}

static inline size_t
dmnsn_zero_roots(double poly[], size_t *degree, double x[])
{
  size_t i = 0;
  while (i <= *degree && poly[i] == 0.0) {
    x[i] = 0.0;
    ++i;
  }

  if (i > 0) {
    for (size_t j = 0; j + i <= *degree; ++j) {
      poly[j] = poly[j + i];
    }
  }

  *degree -= i;
  return i;
}

/* Get the real degree of a polynomial, ignoring leading zeros */
static inline size_t
dmnsn_real_degree(double poly[], size_t degree)
{
  for (ssize_t i = degree; i >= 0; ++i) {
    if (poly[i] != 0.0) {
      return i;
    }
  }

  return 0;
}

/* Improve a root with Newton's method */
static inline double
dmnsn_improve_root(double poly[], size_t degree, double x)
{
  double dx;
  do {
    /* Calculate the value of the polynomial and its derivative at once */
    double p = poly[degree], dp = 0.0;
    for (ssize_t i = degree - 1; i >= 0; --i) {
      dp = dp*x + p;
      p  = p*x  + poly[i];
    }

    dx = p/dp;
    x -= dx;
  } while (fabs(dx/x) > dmnsn_epsilon);

  return x;
}

/* Use the method of bisection to find a root in a range that contains exactly
   one root, counting multiplicity */
static inline double
dmnsn_bisect_root(double poly[], size_t degree, double min, double max)
{
  double evmin = dmnsn_evaluate_polynomial(poly, degree, min);
  double evmax = dmnsn_evaluate_polynomial(poly, degree, max);

  while (max - min > dmnsn_epsilon) {
    double mid = (min + max)/2.0;
    double evmid = dmnsn_evaluate_polynomial(poly, degree, mid);
    if (dmnsn_signbit(evmid) == dmnsn_signbit(evmax)) {
      max = mid;
      evmax = evmid;
    } else {
      min = mid;
      evmin = evmid;
    }
  }

  return (min + max)/2.0;
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
dmnsn_descartes_rule(double poly[], size_t degree)
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

/* Get the (n k) binomial coefficient */
static double
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

/* Find all ranges that contain a single root, with Uspensky's algorithm */
static size_t
dmnsn_uspensky_bounds(double poly[], size_t degree, double bounds[][2],
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
dmnsn_solve_polynomial(double poly[], size_t degree, double x[])
{
  /* Copy the polynomial so we can be destructive */
  double p[degree + 1];
  for (ssize_t i = degree; i >= 0; --i) {
    p[i] = poly[i];
  }

  size_t i = 0; /* Index into x[] */

  /* Eliminate simple zero roots */
  i += dmnsn_zero_roots(p, &degree, x + i);
  /* Account for leading zero coefficients */
  degree = dmnsn_real_degree(p, degree);

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
      r = dmnsn_improve_root(p, degree, r);

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
dmnsn_print_polynomial(FILE *file, double poly[], size_t degree)
{
  fprintf(file, "%g*x^%zu", poly[degree], degree);
  for (size_t i = degree - 1; i > 1; --i) {
    if (poly[i] > 0.0) {
      fprintf(file, " + %g*x^%zu", poly[i], i);
    } else if (poly[i] < 0.0) {
      fprintf(file, " - %g*x^%zu", -poly[i], i);
    }
  }

  if (poly[1] > 0.0) {
    fprintf(file, " + %g*x", poly[1]);
  } else if (poly[1] < 0.0) {
    fprintf(file, " - %g*x", -poly[1]);
  }

  if (poly[0] > 0.0) {
    fprintf(file, " + %g", poly[0]);
  } else if (poly[0] < 0.0) {
    fprintf(file, " - %g", -poly[0]);
  }

  fprintf(file, "\n");
}
