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

#include "Canvas.h"
#include "Scene.h"

static int
dmnsn_py_Scene_init(dmnsn_py_Scene *self, PyObject *args, PyObject *kwds)
{
  static char *kwlist[] = { "canvas", NULL };

  dmnsn_py_Canvas *canvas;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwlist,
                                   &dmnsn_py_CanvasType, &canvas))
    return -1;

  dmnsn_delete_scene(self->scene);
  self->scene = dmnsn_new_scene();
  dmnsn_scene_set_canvas(self->scene, canvas->canvas);
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

PyTypeObject dmnsn_py_SceneType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name      = "dimension.Scene",
  .tp_basicsize = sizeof(dmnsn_py_Scene),
  .tp_dealloc   = (destructor)dmnsn_py_Scene_dealloc,
  .tp_flags     = Py_TPFLAGS_DEFAULT,
  .tp_doc       = "Dimension scene",
  .tp_methods   = dmnsn_py_Scene_methods,
  .tp_getset    = dmnsn_py_Scene_getsetters,
  .tp_init      = (initproc)dmnsn_py_Scene_init,
};

bool
dmnsn_py_init_SceneType(void)
{
  dmnsn_py_SceneType.tp_new = PyType_GenericNew;
  return PyType_Ready(&dmnsn_py_SceneType) >= 0;
}
