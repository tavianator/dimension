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

#ifndef DIMENSION_IMPL_UTILITIES_H
#define DIMENSION_IMPL_UTILITIES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* Provide a stack trace if possible */
void dmnsn_backtrace(FILE *file);

/* Return whether this is the main execution thread, if we can tell */
bool dmnsn_is_main_thread(void);

/* Return true if we are little-endian */
bool dmnsn_is_little_endian(void);

/* Return the number of CPUs available to dimension */
size_t dmnsn_ncpus(void);

#endif /* DIMENSION_IMPL_UTILITIES_H */
