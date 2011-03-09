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
 * Built-in branch profiler.
 */

#ifndef DIMENSION_IMPL_PROFILE_H
#define DIMENSION_IMPL_PROFILE_H

#include <stdbool.h>

/**
 * Record an test and its expected result.  Called by dmnsn_[un]likely();
 * don't call directly.
 * @param[in] result    The result of the test.
 * @param[in] expected  The expected result of the test.
 * @param[in] func      The name of the function the test occurs in.
 * @param[in] file      The name of the file the test occurs in.
 * @param[in] line      The line number on which the test occurs.
 * @return \p result.
 */
bool dmnsn_expect(bool result, bool expected,
                  const char *func, const char *file, unsigned int line);

#endif /* DIMENSION_IMPL_PROFILE_H */
