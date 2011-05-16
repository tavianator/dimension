/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Python Extension.                  *
 *                                                                       *
 * The Dimension Python Extension is free software; you can redistribute *
 * it and/ or modify it under the terms of the GNU Lesser General Public *
 * License as published by the Free Software Foundation; either version  *
 * 3 of the License, or (at your option) any later version.              *
 *                                                                       *
 * The Dimension Python Extension is distributed in the hope that it     *
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied    *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See *
 * the GNU Lesser General Public License for more details.               *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#include <Python.h>
#include "dimension.h"

typedef struct {
  PyObject_HEAD
  dmnsn_scene *scene;
} dmnsn_SceneObject;

static PyTypeObject dmnsn_SceneType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "dimension.Scene",         /* tp_name */
  sizeof(dmnsn_SceneObject), /* tp_basicsize */
  0,                         /* tp_itemsize */
  0,                         /* tp_dealloc */
  0,                         /* tp_print */
  0,                         /* tp_getattr */
  0,                         /* tp_setattr */
  0,                         /* tp_reserved */
  0,                         /* tp_repr */
  0,                         /* tp_as_number */
  0,                         /* tp_as_sequence */
  0,                         /* tp_as_mapping */
  0,                         /* tp_hash  */
  0,                         /* tp_call */
  0,                         /* tp_str */
  0,                         /* tp_getattro */
  0,                         /* tp_setattro */
  0,                         /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,        /* tp_flags */
  "Dimension scene",         /* tp_doc */
};

static PyMethodDef DimensionMethods[] = {
  { NULL, NULL, 0, NULL } /* Sentinel */
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
  dmnsn_SceneType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&dmnsn_SceneType) < 0) {
    return NULL;
  }

  PyObject *m = PyModule_Create(&dimensionmodule);
  if (!m) {
    return NULL;
  }

  Py_INCREF(&dmnsn_SceneType);
  PyModule_AddObject(m, "Scene", (PyObject *)&dmnsn_SceneType);
  return m;
}
