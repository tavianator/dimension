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
 * Constructive solid geometry
 */

#ifndef DIMENSION_CSG_H
#define DIMENSION_CSG_H

/**
 * CSG union.
 * @param[in] objects  The objects from which to compose the union.
 * @return A union of the objects in \p objects.
 */
dmnsn_object *dmnsn_new_csg_union(const dmnsn_array *objects);

/**
 * CSG intersection.
 * @param[in,out] A  The first object.
 * @param[in,out] B  The second object.
 * @return The intersection of \p A and \p B.
 */
dmnsn_object *dmnsn_new_csg_intersection(dmnsn_object *A, dmnsn_object *B);

/**
 * CSG intersection.
 * @param[in,out] A  The outer object.
 * @param[in,out] B  The inner object.
 * @return The difference between \p A and \p B.
 */
dmnsn_object *dmnsn_new_csg_difference(dmnsn_object *A, dmnsn_object *B);

/**
 * CSG Merge.
 * @param[in,out] A  The first object.
 * @param[in,out] B  The second object.
 * @return The merge of \p A and \p B.
 */
dmnsn_object *dmnsn_new_csg_merge(dmnsn_object *A, dmnsn_object *B);

#endif /* DIMENSION_CSG_H */
