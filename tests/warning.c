/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
 *                                                                       *
 * Dimension is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU Lesser General Public License as published *
 * by the Free Software Foundation; either version 3 of the License, or  *
 * (at your option) any later version.                                   *
 *                                                                       *
 * Dimension is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

// Make sure warnings don't kill us - this test should pass

#include "tests.h"
#include <stdlib.h>

int
main()
{
  if (dmnsn_get_resilience() != DMNSN_SEVERITY_MEDIUM) return EXIT_FAILURE;

  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);
  if (dmnsn_get_resilience() != DMNSN_SEVERITY_LOW) return EXIT_FAILURE;

  dmnsn_set_resilience(DMNSN_SEVERITY_MEDIUM);
  if (dmnsn_get_resilience() != DMNSN_SEVERITY_MEDIUM) return EXIT_FAILURE;

  dmnsn_error(DMNSN_SEVERITY_LOW, "This warning is expected.");

  dmnsn_set_resilience(DMNSN_SEVERITY_HIGH);
  if (dmnsn_get_resilience() != DMNSN_SEVERITY_HIGH) return EXIT_FAILURE;

  dmnsn_error(DMNSN_SEVERITY_LOW, "This warning is expected.");
  dmnsn_error(DMNSN_SEVERITY_MEDIUM, "This warning is expected.");

  return EXIT_SUCCESS;
}
