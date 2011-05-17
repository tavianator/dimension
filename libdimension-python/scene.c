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

typedef struct {
  PyObject_HEAD
  dmnsn_scene *scene;
} dmnsn_py_SceneObject;

static PyObject *
dmnsn_py_SceneNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  dmnsn_py_SceneObject *self;
  self = (dmnsn_py_SceneObject *)type->tp_alloc(type, 0);
  self->scene = dmnsn_new_scene();
  return (PyObject *)self;
}

static int
dmnsn_py_SceneInit(dmnsn_py_SceneObject *self, PyObject *args, PyObject *kwds)
{
  return 0;
}

static void
dmnsn_py_SceneDealloc(dmnsn_py_SceneObject *self)
{
  dmnsn_delete_scene(self->scene);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyMemberDef dmnsn_py_SceneMembers[] = {
  { NULL }
};

static PyMethodDef dmnsn_py_SceneMethods[] = {
  { NULL }
};

static PyGetSetDef dmnsn_py_SceneGetSetters[] = {
  { NULL }
};

static PyTypeObject dmnsn_py_SceneType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "dimension.Scene",                 /* tp_name */
  sizeof(dmnsn_py_SceneObject),      /* tp_basicsize */
  0,                                 /* tp_itemsize */
  (destructor)dmnsn_py_SceneDealloc, /* tp_dealloc */
  0,                                 /* tp_print */
  0,                                 /* tp_getattr */
  0,                                 /* tp_setattr */
  0,                                 /* tp_reserved */
  0,                                 /* tp_repr */
  0,                                 /* tp_as_number */
  0,                                 /* tp_as_sequence */
  0,                                 /* tp_as_mapping */
  0,                                 /* tp_hash  */
  0,                                 /* tp_call */
  0,                                 /* tp_str */
  0,                                 /* tp_getattro */
  0,                                 /* tp_setattro */
  0,                                 /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,                /* tp_flags */
  "Dimension scene",                 /* tp_doc */
  0,                                 /* tp_traverse */
  0,                                 /* tp_clear */
  0,                                 /* tp_richcompare */
  0,                                 /* tp_weaklistoffset */
  0,                                 /* tp_iter */
  0,                                 /* tp_iternext */
  dmnsn_py_SceneMethods,             /* tp_methods */
  dmnsn_py_SceneMembers,             /* tp_members */
  dmnsn_py_SceneGetSetters,          /* tp_getset */
  0,                                 /* tp_base */
  0,                                 /* tp_dict */
  0,                                 /* tp_descr_get */
  0,                                 /* tp_descr_set */
  0,                                 /* tp_dictoffset */
  (initproc)dmnsn_py_SceneInit,      /* tp_init */
  0,                                 /* tp_alloc */
  dmnsn_py_SceneNew,                 /* tp_new */
};

static bool
dmnsn_py_init_SceneType(void)
{
  Py_INCREF(&dmnsn_py_SceneType);
  return PyType_Ready(&dmnsn_py_SceneType) >= 0;
}
