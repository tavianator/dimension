/*************************************************************************
 * Copyright (C) 2011 Tavian Barnes <tavianator@tavianator.com>          *
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
 * Generic reference count implementation.
 */

/**
 * Increment a reference count.
 * @param[in,out] object  The reference-counted object to acquire.
 */
#define DMNSN_INCREF(obj) ((void)++(*(obj)->refcount))

/**
 * @internal
 * Decrement a reference count.
 * @param[in,out] object  The reference-counted object to release.
 * @return Whether the object is now garbage.
 */
#define DMNSN_DECREF(obj) (*(obj)->refcount == 0 || --(*(obj)->refcount) == 0)

/**
 * Reference counter.
 */
typedef unsigned int dmnsn_refcount;

/**
 * @internal
 * Create a reference count.
 * @return A new reference counter, initialized to zero (a "borrowed" reference,
 *         which will be garbage-collected the first time it is deleted).
 */
dmnsn_refcount *dmnsn_new_refcount(void);

/**
 * @internal
 * Delete a reference count.  Raises an error if the reference count wasn't
 * zero.
 */
void dmnsn_delete_refcount(dmnsn_refcount *refcount);
