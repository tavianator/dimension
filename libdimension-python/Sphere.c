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
dmnsn_py_Sphere_init(dmnsn_py_Sphere *self, PyObject *args, PyObject *kwds)
{
  if (dmnsn_py_ObjectType.tp_init((PyObject *)self, NULL, NULL) < 0)
    return -1;

  static char *kwlist[] = { "radius", "center", NULL };

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "dO&", kwlist,
                                   &self->radius,
                                   &dmnsn_py_VectorParse, &self->center))
    return -1;

  dmnsn_delete_object(self->base.object);
  self->base.object = dmnsn_new_sphere();
  DMNSN_INCREF(self->base.object);
  return 0;
}

static PyObject *
dmnsn_py_Sphere_initialize(dmnsn_py_Sphere *self)
{
  dmnsn_matrix trans = dmnsn_scale_matrix(
    dmnsn_new_vector(self->radius, self->radius, self->radius)
  );
  trans = dmnsn_matrix_mul(dmnsn_translation_matrix(self->center), trans);

  dmnsn_object *object = self->base.object;
  object->trans = dmnsn_matrix_mul(object->trans, trans);

  dmnsn_texture *texture = object->texture;
  if (texture) {
    dmnsn_matrix trans_inv = dmnsn_matrix_inverse(trans);
    texture->trans = dmnsn_matrix_mul(trans_inv, texture->trans);
  }

  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef dmnsn_py_Sphere_methods[] = {
  { "initialize", (PyCFunction)dmnsn_py_Sphere_initialize, METH_NOARGS,
    "Initialize an sphere" },
  { NULL }
};

static PyGetSetDef dmnsn_py_Sphere_getsetters[] = {
  { NULL }
};

PyTypeObject dmnsn_py_SphereType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name      = "dimension.Sphere",
  .tp_basicsize = sizeof(dmnsn_py_Sphere),
  .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_doc       = "Dimension sphere",
  .tp_methods   = dmnsn_py_Sphere_methods,
  .tp_getset    = dmnsn_py_Sphere_getsetters,
  .tp_init      = (initproc)dmnsn_py_Sphere_init,
  .tp_base      = &dmnsn_py_ObjectType,
};

bool
dmnsn_py_init_SphereType(void)
{
  dmnsn_py_SphereType.tp_new = PyType_GenericNew;
  return PyType_Ready(&dmnsn_py_SphereType) >= 0;
}
