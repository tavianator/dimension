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

static int
dmnsn_py_Camera_init(dmnsn_py_Camera *self, PyObject *args, PyObject *kwds)
{
  if (kwds || (args && !PyArg_ParseTuple(args, "")))
    return -1;

  return 0;
}

static void
dmnsn_py_Camera_dealloc(dmnsn_py_Camera *self)
{
  dmnsn_delete_camera(self->camera);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
dmnsn_py_Camera_initialize(dmnsn_py_Camera *self)
{
  PyErr_SetString(PyExc_TypeError, "Attempt to initialize base Camera");
  return NULL;
}

static PyObject *
dmnsn_py_Camera_transform(dmnsn_py_Camera *self, PyObject *args)
{
  dmnsn_py_Matrix *trans;
  if (!PyArg_ParseTuple(args, "O!", &dmnsn_py_MatrixType, &trans))
    return NULL;

  if (!self->camera) {
    PyErr_SetString(PyExc_TypeError, "Attempt to transform base Camera");
    return NULL;
  }

  self->camera->trans = dmnsn_matrix_mul(trans->m, self->camera->trans);
  Py_INCREF(self);
  return (PyObject *)self;
}

static PyMethodDef dmnsn_py_Camera_methods[] = {
  { "initialize", (PyCFunction)dmnsn_py_Camera_initialize, METH_NOARGS,
    "Initialize a camera" },
  { "transform", (PyCFunction)dmnsn_py_Camera_transform, METH_VARARGS,
    "Transform a camera" },
  { NULL }
};

static PyGetSetDef dmnsn_py_Camera_getsetters[] = {
  { NULL }
};

PyTypeObject dmnsn_py_CameraType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name      = "dimension.Camera",
  .tp_basicsize = sizeof(dmnsn_py_Camera),
  .tp_dealloc   = (destructor)dmnsn_py_Camera_dealloc,
  .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_doc       = "Dimension camera",
  .tp_methods   = dmnsn_py_Camera_methods,
  .tp_getset    = dmnsn_py_Camera_getsetters,
  .tp_init      = (initproc)dmnsn_py_Camera_init,
};

bool
dmnsn_py_init_CameraType(void)
{
  dmnsn_py_CameraType.tp_new = PyType_GenericNew;
  return PyType_Ready(&dmnsn_py_CameraType) >= 0;
}
