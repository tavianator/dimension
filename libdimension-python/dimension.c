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
#include "Vector.h"
#include "Matrix.h"
#include "Color.h"
#include "Scene.h"

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

  { "scale", (PyCFunction)dmnsn_py_Matrix_scale,
    METH_VARARGS | METH_KEYWORDS, "Scaling." },
  { "translate", (PyCFunction)dmnsn_py_Matrix_translate,
    METH_VARARGS | METH_KEYWORDS, "Translation." },
  { "rotate", (PyCFunction)dmnsn_py_Matrix_rotate,
    METH_VARARGS | METH_KEYWORDS, "Rotation." },

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
      || !dmnsn_py_init_MatrixType()
      || !dmnsn_py_init_ColorType()
      || !dmnsn_py_init_SceneType())
    return NULL;

  PyObject *module = PyModule_Create(&dimensionmodule);
  if (!module)
    return NULL;

  PyModule_AddObject(module, "Vector", (PyObject *)&dmnsn_py_VectorType);

  /* Vector constants */
  PyModule_AddObject(module, "Zero", (PyObject *)&dmnsn_py_Zero);
  PyModule_AddObject(module, "X",    (PyObject *)&dmnsn_py_X);
  PyModule_AddObject(module, "Y",    (PyObject *)&dmnsn_py_Y);
  PyModule_AddObject(module, "Z",    (PyObject *)&dmnsn_py_Z);

  PyModule_AddObject(module, "Matrix", (PyObject *)&dmnsn_py_MatrixType);

  PyModule_AddObject(module, "Color", (PyObject *)&dmnsn_py_ColorType);

  /* Color constants */
  PyModule_AddObject(module, "Black",   (PyObject *)&dmnsn_py_Black);
  PyModule_AddObject(module, "White",   (PyObject *)&dmnsn_py_White);
  PyModule_AddObject(module, "Clear",   (PyObject *)&dmnsn_py_Clear);
  PyModule_AddObject(module, "Red",     (PyObject *)&dmnsn_py_Red);
  PyModule_AddObject(module, "Green",   (PyObject *)&dmnsn_py_Green);
  PyModule_AddObject(module, "Blue",    (PyObject *)&dmnsn_py_Blue);
  PyModule_AddObject(module, "Magenta", (PyObject *)&dmnsn_py_Magenta);
  PyModule_AddObject(module, "Orange",  (PyObject *)&dmnsn_py_Orange);
  PyModule_AddObject(module, "Yellow",  (PyObject *)&dmnsn_py_Yellow);
  PyModule_AddObject(module, "Cyan",    (PyObject *)&dmnsn_py_Cyan);

  PyModule_AddObject(module, "Scene", (PyObject *)&dmnsn_py_SceneType);

  return module;
}
