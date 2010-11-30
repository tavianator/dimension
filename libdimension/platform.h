/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
 * Platform abstractions.
 */

#ifndef DIMENSION_IMPL_PLATFORM_H
#define DIMENSION_IMPL_PLATFORM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/**
 * Print a stack trace, if implemented for the current platform.
 * @param[in,out] file  The file to which to write the stack trace.
 */
void dmnsn_backtrace(FILE *file);

/**
 * Is the calling thread the main thread?
 * @return Whether this is the main execution thread, or \c true if we can't
 *         tell.
 */
bool dmnsn_is_main_thread(void);

/**
 * Are we running on a little-endian computer?
 * @return Whether the current architecture is little-endian.
 */
bool dmnsn_is_little_endian(void);

/**
 * How many CPUs are available?
 * @return The number of CPUs available to dimension.
 */
size_t dmnsn_ncpus(void);

#endif /* DIMENSION_IMPL_PLATFORM_H */
