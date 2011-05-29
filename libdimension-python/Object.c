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
dmnsn_py_Object_init(dmnsn_py_Object *self, PyObject *args, PyObject *kwds)
{
  if (kwds || (args && !PyArg_ParseTuple(args, "")))
    return -1;

  return 0;
}

static void
dmnsn_py_Object_dealloc(dmnsn_py_Object *self)
{
  dmnsn_delete_object(self->object);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
dmnsn_py_Object_initialize(dmnsn_py_Object *self)
{
  PyErr_SetString(PyExc_TypeError, "Attempt to initialize base Object");
  return NULL;
}

static PyObject *
dmnsn_py_Object_transform(dmnsn_py_Object *self, PyObject *args)
{
  dmnsn_py_Matrix *trans;
  if (!PyArg_ParseTuple(args, "O!", &dmnsn_py_MatrixType, &trans))
    return NULL;

  if (!self->object) {
    PyErr_SetString(PyExc_TypeError, "Attempt to transform base Object");
    return NULL;
  }

  self->object->trans = dmnsn_matrix_mul(trans->m, self->object->trans);

  Py_INCREF(self);
  return (PyObject *)self;
}

static PyMethodDef dmnsn_py_Object_methods[] = {
  { "initialize", (PyCFunction)dmnsn_py_Object_initialize, METH_NOARGS,
    "Initialize an object" },
  { "transform", (PyCFunction)dmnsn_py_Object_transform, METH_VARARGS,
    "Transform an object" },
  { NULL }
};

static PyGetSetDef dmnsn_py_Object_getsetters[] = {
  { NULL }
};

PyTypeObject dmnsn_py_ObjectType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name      = "dimension.Object",
  .tp_basicsize = sizeof(dmnsn_py_Object),
  .tp_dealloc   = (destructor)dmnsn_py_Object_dealloc,
  .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_doc       = "Dimension object",
  .tp_methods   = dmnsn_py_Object_methods,
  .tp_getset    = dmnsn_py_Object_getsetters,
  .tp_init      = (initproc)dmnsn_py_Object_init,
};

bool
dmnsn_py_init_ObjectType(void)
{
  dmnsn_py_ObjectType.tp_new = PyType_GenericNew;
  return PyType_Ready(&dmnsn_py_ObjectType) >= 0;
}
