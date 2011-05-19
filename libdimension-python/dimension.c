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

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include "dimension.h"

#include "Vector.c"
#include "Scene.c"

static PyObject *
dmnsn_py_dieOnWarnings(PyObject *self, PyObject *args)
{
  int die;
  if (!PyArg_ParseTuple(args, "i", &die))
    return NULL;

  dmnsn_die_on_warnings(die);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef DimensionMethods[] = {
  { "dieOnWarnings", dmnsn_py_dieOnWarnings, METH_VARARGS,
    "Turn Dimension warnings into fatal errors." },

  { "cross", dmnsn_py_Vector_cross, METH_VARARGS, "Cross product."     },
  { "dot",   dmnsn_py_Vector_dot,   METH_VARARGS, "Dot product."       },
  { "proj",  dmnsn_py_Vector_proj,  METH_VARARGS, "Vector projection." },

  { NULL, NULL, 0, NULL }
};

static struct PyModuleDef dimensionmodule = {
  PyModuleDef_HEAD_INIT,
  "dimension",
  NULL,
  -1,
  DimensionMethods
};

PyMODINIT_FUNC
PyInit_dimension(void)
{
  if (!dmnsn_py_init_VectorType()
      || !dmnsn_py_init_SceneType())
    return NULL;

  PyObject *module = PyModule_Create(&dimensionmodule);
  if (!module)
    return NULL;

  PyModule_AddObject(module, "Vector", (PyObject *)&dmnsn_py_VectorType);

  /* Vector constants */
  dmnsn_py_Vector *zero = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
  dmnsn_py_Vector *x    = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
  dmnsn_py_Vector *y    = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
  dmnsn_py_Vector *z    = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
  if (!zero || !x || !y || !z) {
    Py_XDECREF(zero);
    Py_XDECREF(x);
    Py_XDECREF(y);
    Py_XDECREF(z);
    Py_DECREF(module);
    return NULL;
  }
  zero->v = dmnsn_zero;
  x->v    = dmnsn_x;
  y->v    = dmnsn_y;
  z->v    = dmnsn_z;
  PyModule_AddObject(module, "Zero", (PyObject *)zero);
  PyModule_AddObject(module, "X",    (PyObject *)x);
  PyModule_AddObject(module, "Y",    (PyObject *)y);
  PyModule_AddObject(module, "Z",    (PyObject *)z);

  PyModule_AddObject(module, "Scene",  (PyObject *)&dmnsn_py_SceneType);

  return module;
}
