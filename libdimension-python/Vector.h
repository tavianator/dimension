/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Python Module.                     *
 *                                                                       *
 * The Dimension Python Module is free software; you can redistribute it *
 * and/ or modify it under the terms of the GNU Lesser General Public    *
 * License as published by the Free Software Foundation; either version  *
 * 3 of the License, or (at your option) any later version.              *
 *                                                                       *
 * The Dimension Python Module is distributed in the hope that it will   *
 * be useful, but WITHOUT ANY WARRANTY; without even the implied         *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See *
 * the GNU Lesser General Public License for more details.               *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#include "dimension-python.h"

typedef struct dmnsn_py_Vector {
  PyObject_HEAD
  dmnsn_vector v;
} dmnsn_py_Vector;

extern PyTypeObject dmnsn_py_VectorType;

bool dmnsn_py_Vector_args(dmnsn_vector *v, PyObject *args, PyObject *kwds);
bool dmnsn_py_init_VectorType(void);

/* Global methods */
PyObject *dmnsn_py_Vector_cross(PyObject *self, PyObject *args);
PyObject *dmnsn_py_Vector_dot(PyObject *self, PyObject *args);
PyObject *dmnsn_py_Vector_proj(PyObject *self, PyObject *args);

/* Vector constants */
extern dmnsn_py_Vector dmnsn_py_Zero;
extern dmnsn_py_Vector dmnsn_py_X;
extern dmnsn_py_Vector dmnsn_py_Y;
extern dmnsn_py_Vector dmnsn_py_Z;
