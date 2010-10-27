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
  } while (fabs(dx) > dmnsn_epsilon);

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

/* Find a range that contains a single root, with Uspensky's algorithm */
static bool
dmnsn_uspensky_bound(double poly[], size_t degree, double *min, double *max)
{
  size_t n = dmnsn_descartes_rule(poly, degree);
  if (n == 0) {
    return false;
  } else if (n == 1) {
    return true;
  } else {
    double a[degree];
    /* a is the expanded form of poly(x + 1) */
    for (size_t i = 0; i <= degree; ++i) {
      a[i] = poly[i];
      for (size_t j = i + 1; j <= degree; ++j) {
        a[i] += dmnsn_binom(j, i)*poly[j];
      }
    }

    if (a[0] == 0.0) {
      *max = *min;
      return true;
    } else if (dmnsn_uspensky_bound(a, degree, min, max)) {
      *min += 1.0;
      *max += 1.0;
      return true;
    }

    double b[degree];
    /* b is the expanded form of (x + 1)^degree * poly(1/(x + 1)) */
    for (size_t i = 0; i <= degree; ++i) {
      b[i] = poly[degree - i];
      for (size_t j = i + 1; j <= degree; ++j) {
        b[i] += dmnsn_binom(j, i)*poly[degree - j];
      }
    }

    if (dmnsn_uspensky_bound(b, degree, min, max)) {
      double temp = *min;
      *min = 1.0/(*max + 1.0);
      *max = 1.0/(temp + 1.0);
      return true;
    }

    return false;
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

  while (degree > 2) {
    /* Get a bound on the range of positive roots */
    double min = +0.0, max = INFINITY;
    if (dmnsn_uspensky_bound(p, degree, &min, &max)) {
      if (isinf(max)) {
        /* Replace an infinite upper bound with a finite one due to Cauchy */
        max = 0.0;
        for (size_t j = 0; j < degree; ++j) {
          max = dmnsn_max(max, fabs(p[j]));
        }
        max /= fabs(p[degree]);
        max += 1.0;
      }

      /* Bisect within the found range */
      double r = dmnsn_bisect_root(p, degree, min, max);
      r = dmnsn_improve_root(p, degree, r);

      /* Use synthetic division to eliminate the root `r' */
      dmnsn_eliminate_root(p, &degree, r);

      /* Store the found root */
      x[i] = r;
      ++i;
    } else {
      break;
    }

    i += dmnsn_zero_roots(p, &degree, x + i);
    degree = dmnsn_real_degree(p, degree);
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
