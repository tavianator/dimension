/*************************************************************************
 * Copyright (C) 2011 Tavian Barnes <tavianator@tavianator.com>          *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
 *                                                                       *
 * The Dimension Test Suite is free software; you can redistribute it    *
 * and/or modify it under the terms of the GNU General Public License as *
 * published by the Free Software Foundation; either version 3 of the    *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Test Suite is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

// Make sure that custom fatal error handlers work.

#include "dimension.h"
#include <stdlib.h>

void
dmnsn_custom_fatal_error_fn(void)
{
  exit(EXIT_SUCCESS);
}

int
main(void)
{
  dmnsn_set_fatal_error_fn(dmnsn_custom_fatal_error_fn);
  dmnsn_error("This error is expected.");
  return EXIT_FAILURE;
}
