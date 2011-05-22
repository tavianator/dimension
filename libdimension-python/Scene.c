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
dmnsn_py_Scene_init(dmnsn_py_Scene *self, PyObject *args, PyObject *kwds)
{
  dmnsn_delete_scene(self->scene);
  self->scene = dmnsn_new_scene();

  static char *kwlist[] = { "canvas", "camera", "objects", NULL };

  dmnsn_py_Canvas *canvas;
  dmnsn_py_Camera *camera;
  PyObject *objects;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!O!O!", kwlist,
                                   &dmnsn_py_CanvasType, &canvas,
                                   &dmnsn_py_CameraType, &camera,
                                   &PyList_Type, &objects))
    return -1;

  dmnsn_scene_set_canvas(self->scene, canvas->canvas);

  if (!PyObject_CallMethod((PyObject *)camera, "initialize", ""))
    return -1;

  /* Account for image proportions in camera */
  dmnsn_matrix stretch = dmnsn_scale_matrix(
    dmnsn_new_vector(
      (double)canvas->canvas->width/canvas->canvas->height,
      1.0,
      1.0
    )
  );
  camera->camera->trans = dmnsn_matrix_mul(camera->camera->trans, stretch);
  dmnsn_scene_set_camera(self->scene, camera->camera);
  self->scene->background = dmnsn_white;

  Py_ssize_t size = PyList_Size(objects);
  for (Py_ssize_t i = 0; i < size; ++i) {
    PyObject *obj = PyList_GetItem(objects, i);
    if (!PyObject_TypeCheck(obj, &dmnsn_py_ObjectType)) {
      PyErr_SetString(PyExc_TypeError, "Expected a list of objects");
      return -1;
    }

    if (!PyObject_CallMethod(obj, "initialize", ""))
      return -1;

    dmnsn_py_Object *object = (dmnsn_py_Object *)obj;
    dmnsn_scene_add_object(self->scene, object->object);
  }

  return 0;
}

static void
dmnsn_py_Scene_dealloc(dmnsn_py_Scene *self)
{
  dmnsn_delete_scene(self->scene);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
dmnsn_py_Scene_raytrace(dmnsn_py_Scene *self)
{
  dmnsn_raytrace_scene(self->scene);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef dmnsn_py_Scene_methods[] = {
  { "raytrace", (PyCFunction)dmnsn_py_Scene_raytrace, METH_NOARGS,
    "Raytrace a scene" },
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
