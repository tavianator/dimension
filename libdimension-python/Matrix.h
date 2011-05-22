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

typedef struct dmnsn_py_Matrix {
  PyObject_HEAD
  dmnsn_matrix m;
} dmnsn_py_Matrix;

extern PyTypeObject dmnsn_py_MatrixType;

bool dmnsn_py_init_MatrixType(void);

/* Global methods */
PyObject *dmnsn_py_Matrix_scale(PyObject *self, PyObject *args);
PyObject *dmnsn_py_Matrix_translate(PyObject *self, PyObject *args);
PyObject *dmnsn_py_Matrix_rotate(PyObject *self, PyObject *args);
