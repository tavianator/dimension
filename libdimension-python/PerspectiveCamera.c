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
dmnsn_py_PerspectiveCamera_init(dmnsn_py_PerspectiveCamera *self,
                                PyObject *args, PyObject *kwds)
{
  if (dmnsn_py_CameraType.tp_init((PyObject *)self, NULL, NULL) < 0)
    return -1;

  static char *kwlist[] = { "location", "look_at", NULL };

  dmnsn_vector location, look_at;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O&O&", kwlist,
                                   &dmnsn_py_VectorParse, &location,
                                   &dmnsn_py_VectorParse, &look_at))
    return -1;

  self->location = location;
  self->look_at = look_at;

  dmnsn_delete_camera(self->base.camera);
  self->base.camera = dmnsn_new_perspective_camera();
  DMNSN_INCREF(self->base.camera);
  return 0;
}

static PyObject *
dmnsn_py_PerspectiveCamera_initialize(dmnsn_py_PerspectiveCamera *self)
{
  dmnsn_vector look_at = dmnsn_vector_sub(self->look_at, self->location);

  /* Align the camera with the look_at point */

  dmnsn_vector right   = dmnsn_x;
  dmnsn_vector up      = dmnsn_y;
  dmnsn_vector forward = dmnsn_z;

  dmnsn_vector theta = dmnsn_vector_mul(
    dmnsn_vector_axis_angle(forward, look_at, up),
    up
  );
  dmnsn_matrix align = dmnsn_rotation_matrix(theta);

  right   = dmnsn_transform_vector(align, right);
  forward = dmnsn_transform_vector(align, forward);

  theta = dmnsn_vector_mul(
    dmnsn_vector_axis_angle(forward, look_at, right),
    right
  );
  align = dmnsn_matrix_mul(dmnsn_rotation_matrix(theta), align);

  /* Translate the camera to the location */
  dmnsn_matrix move = dmnsn_translation_matrix(self->location);

  dmnsn_matrix trans = dmnsn_matrix_mul(move, align);
  self->base.camera->trans = dmnsn_matrix_mul(self->base.camera->trans,
                                              trans);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef dmnsn_py_PerspectiveCamera_methods[] = {
  { "initialize", (PyCFunction)dmnsn_py_PerspectiveCamera_initialize,
    METH_NOARGS, "Initialize a perspective camera" },
  { NULL }
};

static PyGetSetDef dmnsn_py_PerspectiveCamera_getsetters[] = {
  { NULL }
};

PyTypeObject dmnsn_py_PerspectiveCameraType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name      = "dimension.PerspectiveCamera",
  .tp_basicsize = sizeof(dmnsn_py_PerspectiveCamera),
  .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_doc       = "Dimension perspective camera",
  .tp_methods   = dmnsn_py_PerspectiveCamera_methods,
  .tp_getset    = dmnsn_py_PerspectiveCamera_getsetters,
  .tp_init      = (initproc)dmnsn_py_PerspectiveCamera_init,
  .tp_base      = &dmnsn_py_CameraType,
};

bool
dmnsn_py_init_PerspectiveCameraType(void)
{
  dmnsn_py_PerspectiveCameraType.tp_new = PyType_GenericNew;
  return PyType_Ready(&dmnsn_py_PerspectiveCameraType) >= 0;
}
