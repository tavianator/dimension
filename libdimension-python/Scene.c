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

typedef struct dmnsn_py_Scene {
  PyObject_HEAD
  dmnsn_scene *scene;
} dmnsn_py_Scene;

static PyObject *
dmnsn_py_Scene_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  dmnsn_py_Scene *self;
  self = (dmnsn_py_Scene *)type->tp_alloc(type, 0);
  self->scene = dmnsn_new_scene();
  return (PyObject *)self;
}

static int
dmnsn_py_Scene_init(dmnsn_py_Scene *self, PyObject *args, PyObject *kwds)
{
  return 0;
}

static void
dmnsn_py_Scene_dealloc(dmnsn_py_Scene *self)
{
  dmnsn_delete_scene(self->scene);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyMethodDef dmnsn_py_Scene_methods[] = {
  { NULL }
};

static PyGetSetDef dmnsn_py_Scene_getsetters[] = {
  { NULL }
};

static PyTypeObject dmnsn_py_SceneType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name      = "dimension.Scene",
  .tp_basicsize = sizeof(dmnsn_py_Scene),
  .tp_dealloc   = (destructor)dmnsn_py_Scene_dealloc,
  .tp_flags     = Py_TPFLAGS_DEFAULT,
  .tp_doc       = "Dimension scene",
  .tp_methods   = dmnsn_py_Scene_methods,
  .tp_getset    = dmnsn_py_Scene_getsetters,
  .tp_init      = (initproc)dmnsn_py_Scene_init,
  .tp_new       = dmnsn_py_Scene_new,
};

static bool
dmnsn_py_init_SceneType(void)
{
  Py_INCREF(&dmnsn_py_SceneType);
  return PyType_Ready(&dmnsn_py_SceneType) >= 0;
}
