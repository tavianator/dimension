/*************************************************************************
 * Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
 * Real root isolation algorithm based on work by Vincent, Uspensky, Collins and
 * Akritas, Johnson, Krandick, and Rouillier and Zimmerman.
 */

#include "dimension-internal.h"
#include <math.h>

/** Get the real degree of a polynomial, ignoring leading zeros. */
static inline size_t
dmnsn_real_degree(const double poly[], size_t degree)
{
  for (size_t i = degree + 1; i-- > 0;) {
    if (dmnsn_likely(fabs(poly[i]) >= dmnsn_epsilon)) {
      return i;
    }
  }

  return 0;
}

/** Divide each coefficient by the leading coefficient. */
static inline void
dmnsn_polynomial_normalize(double poly[], size_t degree)
{
  for (size_t i = 0; i < degree; ++i) {
    poly[i] /= poly[degree];
  }
  poly[degree] = 1.0;
}

/** Eliminate trivial zero roots from \p poly[]. */
static inline void
dmnsn_eliminate_zero_roots(double **poly, size_t *degree)
{
  size_t i;
  for (i = 0; i <= *degree; ++i) {
    if (fabs((*poly)[i]) >= dmnsn_epsilon) {
      break;
    }
  }

  *poly += i;
  *degree -= i;
}

/** Calculate a finite upper bound on the roots of a normalized polynomial. */
static inline double
dmnsn_root_bound(const double poly[], size_t degree)
{
  double bound = fabs(poly[0]);
  for (size_t i = 1; i < degree; ++i) {
    bound = dmnsn_max(bound, fabs(poly[i]));
  }
  bound += 1.0;
  return bound;
}

/** Copy a polynomial. */
static inline void
dmnsn_polynomial_copy(double dest[], const double src[], size_t degree)
{
  for (size_t i = 0; i <= degree; ++i) {
    dest[i] = src[i];
  }
}

/** Transform a polynomial by P'(x) = P(x + 1). */
static inline void
dmnsn_polynomial_translate(double poly[], size_t degree)
{
  for (size_t i = 0; i <= degree; ++i) {
    for (size_t j = degree - i; j <= degree - 1; ++j) {
      poly[j] += poly[j + 1];
    }
  }
}

/** Transform a polynomial by P'(x) = P(c*x). */
static inline void
dmnsn_polynomial_scale(double poly[], size_t degree, double c)
{
  double factor = c;
  for (size_t i = 1; i <= degree; ++i) {
    poly[i] *= factor;
    factor *= c;
  }
}

/** Returns the result of Descartes' rule on x^degree * poly(1/(x + 1)). */
static size_t
dmnsn_descartes_bound(const double poly[], size_t degree)
{
  /* Copy the polynomial so we can be destructive */
  double p[degree + 1];
  dmnsn_polynomial_copy(p, poly, degree);

  /* Calculate poly(1/(1/x + 1)) which avoids reversal */
  for (size_t i = 1; i <= degree; ++i) {
    for (size_t j = i; j >= 1; --j) {
      p[j] += p[j - 1];
    }
  }

  /* Find the number of sign changes in p[] */
  size_t changes = 0;
  int lastsign = dmnsn_sign(p[0]);
  for (size_t i = 1; changes <= 1 && i <= degree; ++i) {
    int sign = dmnsn_sign(p[i]);
    if (sign != 0 && sign != lastsign) {
      ++changes;
      lastsign = sign;
    }
  }

  return changes;
}

/** Depth-first search of possible isolating intervals. */
static size_t
dmnsn_root_bounds_recursive(double poly[], size_t degree, double *c, double *k,
                            double bounds[][2], size_t nbounds)
{
  size_t s = dmnsn_descartes_bound(poly, degree);
  if (s >= 2) {
    *c *= 2.0;
    *k /= 2.0;
    double currc = *c, currk = *k;

    /* Get the left child */
    dmnsn_polynomial_scale(poly, degree, 1.0/2.0);

    size_t n = dmnsn_root_bounds_recursive(poly, degree, c, k, bounds, nbounds);
    if (n == nbounds) {
      return n;
    }

    /* Get the right child from the last tested polynomial */
    dmnsn_polynomial_translate(poly, degree);
    dmnsn_polynomial_scale(poly, degree, currk/(*k));

    *c = currc + 1.0;
    *k = currk;

    bounds += n;
    nbounds -= n;
    n += dmnsn_root_bounds_recursive(poly, degree, c, k, bounds, nbounds);
    return n;
  } else if (s == 1) {
    bounds[0][0] = (*c)*(*k);
    bounds[0][1] = (*c + 1.0)*(*k);
    return 1;
  } else {
    return 0;
  }
}

/** Find ranges that contain a single root. */
static size_t
dmnsn_root_bounds(const double poly[], size_t degree, double bounds[][2],
                  size_t nbounds)
{
  /* Copy the polynomial so we can be destructive */
  double p[degree + 1];
  dmnsn_polynomial_copy(p, poly, degree);

  /* Scale the roots to within (0, 1] */
  double bound = dmnsn_root_bound(p, degree);
  dmnsn_polynomial_scale(p, degree, bound);

  /* Bounding intervals are of the form (c*k, (c + 1)*k) */
  double c = 0.0, k = 1.0;

  /* Isolate the roots */
  size_t n = dmnsn_root_bounds_recursive(p, degree, &c, &k, bounds, nbounds);

  /* Scale the roots back to within (0, bound] */
  for (size_t i = 0; i < n; ++i) {
    bounds[i][0] *= bound;
    bounds[i][1] *= bound;
  }

  return n;
}

/** Use the false position method to find a root in a range that contains
    exactly one root. */
static inline double
dmnsn_bisect_root(const double poly[], size_t degree, double min, double max)
{
  double evmin = dmnsn_polynomial_evaluate(poly, degree, min);
  double evmax = dmnsn_polynomial_evaluate(poly, degree, max);
  double evinitial = dmnsn_min(fabs(evmin), fabs(evmax));
  double mid, evmid;
  int lastsign = 0;

  do {
    mid = (min*evmax - max*evmin)/(evmax - evmin);
    evmid = dmnsn_polynomial_evaluate(poly, degree, mid);
    int sign = dmnsn_sign(evmid);

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
    } else if (sign == dmnsn_sign(evmax)) {
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
  } while ((fabs(evmid) >= fabs(mid)*dmnsn_epsilon
            /* This condition improves stability when one of the bounds is
               close to a different root than we are trying to find */
            || fabs(evmid) > evinitial)
           && max - min >= fabs(mid)*dmnsn_epsilon);

  return mid;
}

/** Use synthetic division to eliminate the root \p r from \p poly[]. */
static inline size_t
dmnsn_eliminate_root(double poly[], size_t degree, double r)
{
  double rem = poly[degree];
  for (size_t i = degree; i-- > 0;) {
    double temp = poly[i];
    poly[i] = rem;
    rem = temp + r*rem;
  }
  return degree - 1;
}

/** Solve a normalized linear polynomial algebraically. */
static inline size_t
dmnsn_solve_linear(const double poly[2], double x[1])
{
  x[0] = -poly[0];
  if (x[0] >= dmnsn_epsilon)
    return 1;
  else
    return 0;
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

    if (x[1] >= dmnsn_epsilon)
      return 2;
    else if (x[0] >= dmnsn_epsilon)
      return 1;
    else
      return 0;
  } else {
    return 0;
  }
}

/** Solve a normalized cubic polynomial algebraically. */
static inline size_t
dmnsn_solve_cubic(double poly[4], double x[3])
{
  /* Reduce to a monic trinomial (t^3 + p*t + q, t = x + b/3) */
  double b2 = poly[2]*poly[2];
  double p = poly[1] - b2/3.0;
  double q = poly[0] - poly[2]*(9.0*poly[1] - 2.0*b2)/27.0;

  double disc = 4.0*p*p*p + 27.0*q*q;
  double bdiv3 = poly[2]/3.0;

  if (disc < 0.0) {
    /* Three real roots -- this implies p < 0 */
    double msqrtp3 = -sqrt(-p/3.0);
    double theta = acos(3*q/(2*p*msqrtp3))/3.0;

    /* Store the roots in order from largest to smallest */
    x[2] = 2.0*msqrtp3*cos(theta) - bdiv3;
    x[0] = -2.0*msqrtp3*cos(4.0*atan(1.0)/3.0 - theta) - bdiv3;
    x[1] = -(x[0] + x[2] + poly[2]);

    if (x[2] >= dmnsn_epsilon)
      return 3;
    else if (x[1] >= dmnsn_epsilon)
      return 2;
  } else if (disc > 0.0) {
    /* One real root */
    double cbrtdiscq = cbrt(sqrt(disc/108.0) + fabs(q)/2.0);
    double abst = cbrtdiscq - p/(3.0*cbrtdiscq);

    if (q >= 0) {
      x[0] = -abst - bdiv3;
    } else {
      x[0] = abst - bdiv3;
    }
  } else if (fabs(p) < dmnsn_epsilon) {
    /* Equation is a perfect cube */
    x[0] = -bdiv3;
  } else {
    /* Two real roots; one duplicate */
    double t1 = 3.0*q/p, t2 = -t1/2.0;
    x[0] = dmnsn_max(t1, t2);
    x[1] = dmnsn_min(t1, t2);
    if (x[1] >= dmnsn_epsilon)
      return 2;
  }

  if (x[0] >= dmnsn_epsilon)
    return 1;
  else
    return 0;
}

/* Solve a polynomial */
DMNSN_HOT size_t
dmnsn_polynomial_solve(const double poly[], size_t degree, double x[])
{
  /* Copy the polynomial so we can be destructive */
  double copy[degree + 1], *p = copy;
  dmnsn_polynomial_copy(p, poly, degree);

  /* Index into x[] */
  size_t i = 0;

  /* Account for leading zero coefficients */
  degree = dmnsn_real_degree(p, degree);
  /* Normalize the leading coefficient to 1.0 */
  dmnsn_polynomial_normalize(p, degree);
  /* Eliminate simple zero roots */
  dmnsn_eliminate_zero_roots(&p, &degree);

  static const size_t max_algebraic = 3;
  if (degree > max_algebraic) {
    /* Find isolating intervals for (degree - max_algebraic) roots of p[] */
    double ranges[degree - max_algebraic][2];
    size_t n = dmnsn_root_bounds(p, degree, ranges, degree - max_algebraic);

    for (size_t j = 0; j < n; ++j) {
      /* Bisect within the found range */
      double r = dmnsn_bisect_root(p, degree, ranges[j][0], ranges[j][1]);

      /* Use synthetic division to eliminate the root `r' */
      degree = dmnsn_eliminate_root(p, degree, r);

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
  case 3:
    i += dmnsn_solve_cubic(p, x + i);
    break;
  }

  return i;
}

/* Print a polynomial */
void
dmnsn_polynomial_print(FILE *file, const double poly[], size_t degree)
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
