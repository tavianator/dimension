/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
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

#ifndef DIMENSION_H
#define DIMENSION_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Debug and error handling stuff */

typedef enum {
  DMNSN_SEVERITY_LOW,    /* Only die on low resilience */
  DMNSN_SEVERITY_MEDIUM, /* Die on low or medium resilience */
  DMNSN_SEVERITY_HIGH    /* Always die */
} dmnsn_severity;

#define dmnsn_error(severity, str)                              \
  dmnsn_report_error(severity, __PRETTY_FUNCTION__, str)

void dmnsn_report_error(dmnsn_severity severity,
                        const char *func, const char *str);
dmnsn_severity dmnsn_get_resilience();
void dmnsn_set_resilience(dmnsn_severity resilience);

/* More includes */
#include <dimension/geometry.h>
#include <dimension/color.h>
#include <dimension/canvas.h>

#ifdef __cplusplus
}
#endif

#endif /* DIMENSION_H */
