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

#include "Vector.h"

bool
dmnsn_py_Vector_args(dmnsn_vector *v, PyObject *args, PyObject *kwds)
{
  static char *kwlist[] = { "x", "y", "z", NULL };
  if (PyArg_ParseTupleAndKeywords(args, kwds, "ddd", kwlist,
                                  &v->x, &v->y, &v->z)) {
    return true;
  } else {
    if (kwds)
      return false;

    PyErr_Clear();

    dmnsn_py_Vector *vec;
    if (!PyArg_ParseTuple(args, "O!", &dmnsn_py_VectorType, &vec))
      return false;

    *v = vec->v;
    return true;
  }
}

static int
dmnsn_py_Vector_init(dmnsn_py_Vector *self, PyObject *args, PyObject *kwds)
{
  return dmnsn_py_Vector_args(&self->v, args, kwds) ? 0 : -1;
}

static PyObject *
dmnsn_py_Vector_repr(dmnsn_py_Vector *self)
{
  PyObject *x = PyFloat_FromDouble(self->v.x);
  PyObject *y = PyFloat_FromDouble(self->v.y);
  PyObject *z = PyFloat_FromDouble(self->v.z);

  if (!x || !y || !x) {
    Py_XDECREF(z);
    Py_XDECREF(y);
    Py_XDECREF(x);
    return NULL;
  }

  PyObject *repr = PyUnicode_FromFormat("dimension.Vector(%R, %R, %R)",
                                        x, y, z);
  Py_DECREF(z);
  Py_DECREF(y);
  Py_DECREF(x);
  return repr;
}

static PyObject *
dmnsn_py_Vector_str(dmnsn_py_Vector *self)
{
  PyObject *x = PyFloat_FromDouble(self->v.x);
  PyObject *y = PyFloat_FromDouble(self->v.y);
  PyObject *z = PyFloat_FromDouble(self->v.z);

  if (!x || !y || !x) {
    Py_XDECREF(z);
    Py_XDECREF(y);
    Py_XDECREF(x);
    return NULL;
  }

  PyObject *str = PyUnicode_FromFormat("<%S, %S, %S>", x, y, z);
  Py_DECREF(z);
  Py_DECREF(y);
  Py_DECREF(x);
  return str;
}

static PyObject *
dmnsn_py_Vector_richcompare(PyObject *lhs, PyObject *rhs, int op)
{
  if (!PyObject_TypeCheck(lhs, &dmnsn_py_VectorType)
      || !PyObject_TypeCheck(rhs, &dmnsn_py_VectorType))
  {
    PyErr_SetString(PyExc_TypeError,
                    "Vectors can only be compared with Vectors");
    return NULL;
  }

  dmnsn_py_Vector *vlhs = (dmnsn_py_Vector *)lhs;
  dmnsn_py_Vector *vrhs = (dmnsn_py_Vector *)rhs;

  bool equal =
    dmnsn_vector_norm(dmnsn_vector_sub(vlhs->v, vrhs->v)) < dmnsn_epsilon;

  PyObject *result;
  switch (op) {
  case Py_EQ:
    result = equal ? Py_True : Py_False;
    break;
  case Py_NE:
    result = !equal ? Py_True : Py_False;
    break;
  default:
    result = Py_NotImplemented;
    break;
  }

  Py_INCREF(result);
  return result;
}

static PyObject *
dmnsn_py_Vector_add(PyObject *lhs, PyObject *rhs)
{
  if (!PyObject_TypeCheck(lhs, &dmnsn_py_VectorType)
      || !PyObject_TypeCheck(rhs, &dmnsn_py_VectorType))
  {
    PyErr_SetString(PyExc_TypeError,
                    "Vectors can only be added to Vectors");
    return NULL;
  }

  dmnsn_py_Vector *ret = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
  if (ret) {
    dmnsn_py_Vector *vlhs = (dmnsn_py_Vector *)lhs;
    dmnsn_py_Vector *vrhs = (dmnsn_py_Vector *)rhs;
    ret->v = dmnsn_vector_add(vlhs->v, vrhs->v);
  }
  return (PyObject *)ret;
}

static PyObject *
dmnsn_py_Vector_sub(PyObject *lhs, PyObject *rhs)
{
  if (!PyObject_TypeCheck(lhs, &dmnsn_py_VectorType)
      || !PyObject_TypeCheck(rhs, &dmnsn_py_VectorType))
  {
    PyErr_SetString(PyExc_TypeError,
                    "Vectors can only be subtracted from Vectors");
    return NULL;
  }

  dmnsn_py_Vector *ret = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
  if (ret) {
    dmnsn_py_Vector *vlhs = (dmnsn_py_Vector *)lhs;
    dmnsn_py_Vector *vrhs = (dmnsn_py_Vector *)rhs;
    ret->v = dmnsn_vector_sub(vlhs->v, vrhs->v);
  }
  return (PyObject *)ret;
}

static PyObject *
dmnsn_py_Vector_mul(PyObject *lhs, PyObject *rhs)
{
  dmnsn_py_Vector *vec;
  double dbl;

  if (PyObject_TypeCheck(lhs, &dmnsn_py_VectorType)) {
    vec = (dmnsn_py_Vector *)lhs;
    dbl = PyFloat_AsDouble(rhs);
    if (PyErr_Occurred())
      return NULL;
  } else {
    vec = (dmnsn_py_Vector *)rhs;
    dbl = PyFloat_AsDouble(lhs);
    if (PyErr_Occurred())
      return NULL;
  }

  dmnsn_py_Vector *ret = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
  if (ret) {
    ret->v = dmnsn_vector_mul(dbl, vec->v);
  }
  return (PyObject *)ret;
}

static PyObject *
dmnsn_py_Vector_div(PyObject *lhs, PyObject *rhs)
{
  if (!PyObject_TypeCheck(lhs, &dmnsn_py_VectorType)) {
    PyErr_SetString(PyExc_TypeError,
                    "Vectors can only be divided by scalars");
    return NULL;
  }

  double drhs = PyFloat_AsDouble(rhs);
  if (PyErr_Occurred())
    return NULL;

  dmnsn_py_Vector *ret = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
  if (ret) {
    dmnsn_py_Vector *vlhs = (dmnsn_py_Vector *)lhs;
    ret->v = dmnsn_vector_div(vlhs->v, drhs);
  }
  return (PyObject *)ret;
}

static int
dmnsn_py_Vector_bool(PyObject *obj)
{
  dmnsn_py_Vector *vec = (dmnsn_py_Vector *)obj;
  return dmnsn_vector_norm(vec->v) >= dmnsn_epsilon;
}

static PyObject *
dmnsn_py_Vector_positive(PyObject *rhs)
{
  Py_INCREF(rhs);
  return rhs;
}

static PyObject *
dmnsn_py_Vector_negative(PyObject *rhs)
{
  dmnsn_py_Vector *ret = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
  if (ret) {
    dmnsn_py_Vector *vrhs = (dmnsn_py_Vector *)rhs;
    ret->v = dmnsn_vector_negate(vrhs->v);
  }
  return (PyObject *)ret;
}

PyObject *
dmnsn_py_Vector_cross(PyObject *self, PyObject *args)
{
  dmnsn_py_Vector *lhs, *rhs;
  if (!PyArg_ParseTuple(args, "O!O!",
                        &dmnsn_py_VectorType, &lhs,
                        &dmnsn_py_VectorType, &rhs))
    return NULL;

  dmnsn_py_Vector *ret = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
  if (ret) {
    ret->v = dmnsn_vector_cross(lhs->v, rhs->v);
  }
  return (PyObject *)ret;
}

PyObject *
dmnsn_py_Vector_dot(PyObject *self, PyObject *args)
{
  dmnsn_py_Vector *lhs, *rhs;
  if (!PyArg_ParseTuple(args, "O!O!",
                        &dmnsn_py_VectorType, &lhs,
                        &dmnsn_py_VectorType, &rhs))
    return NULL;

  return PyFloat_FromDouble(dmnsn_vector_dot(lhs->v, rhs->v));
}

PyObject *
dmnsn_py_Vector_proj(PyObject *self, PyObject *args)
{
  dmnsn_py_Vector *u, *d;
  if (!PyArg_ParseTuple(args, "O!O!",
                        &dmnsn_py_VectorType, &u,
                        &dmnsn_py_VectorType, &d))
    return NULL;

  dmnsn_py_Vector *ret = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
  if (ret) {
    ret->v = dmnsn_vector_proj(u->v, d->v);
  }
  return (PyObject *)ret;
}

static PyNumberMethods dmnsn_py_Vector_as_number = {
  .nb_add         = dmnsn_py_Vector_add,
  .nb_subtract    = dmnsn_py_Vector_sub,
  .nb_multiply    = dmnsn_py_Vector_mul,
  .nb_true_divide = dmnsn_py_Vector_div,
  .nb_bool        = dmnsn_py_Vector_bool,
  .nb_positive    = dmnsn_py_Vector_positive,
  .nb_negative    = dmnsn_py_Vector_negative,
};

static PyObject *
dmnsn_py_Vector_norm(dmnsn_py_Vector *self)
{
  return PyFloat_FromDouble(dmnsn_vector_norm(self->v));
}

static PyObject *
dmnsn_py_Vector_normalized(dmnsn_py_Vector *self)
{
  dmnsn_py_Vector *ret = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
  if (ret) {
    ret->v = dmnsn_vector_normalized(self->v);
  }
  return (PyObject *)ret;
}

static PyMethodDef dmnsn_py_Vector_methods[] = {
  { "norm", (PyCFunction)dmnsn_py_Vector_norm, METH_NOARGS,
    "Return the magnitude of the vector" },
  { "normalized", (PyCFunction)dmnsn_py_Vector_normalized, METH_NOARGS,
    "Return the magnitude of the vector" },
  { NULL }
};

static PyObject *
dmnsn_py_Vector_get_x(dmnsn_py_Vector *self, void *closure)
{
  return PyFloat_FromDouble(self->v.x);
}

static PyObject *
dmnsn_py_Vector_get_y(dmnsn_py_Vector *self, void *closure)
{
  return PyFloat_FromDouble(self->v.y);
}

static PyObject *
dmnsn_py_Vector_get_z(dmnsn_py_Vector *self, void *closure)
{
  return PyFloat_FromDouble(self->v.z);
}

static PyGetSetDef dmnsn_py_Vector_getsetters[] = {
  { "x", (getter)dmnsn_py_Vector_get_x, NULL, "x coordinate", NULL },
  { "y", (getter)dmnsn_py_Vector_get_y, NULL, "y coordinate", NULL },
  { "z", (getter)dmnsn_py_Vector_get_z, NULL, "z coordinate", NULL },
  { NULL }
};

PyTypeObject dmnsn_py_VectorType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "dimension.Vector",
  .tp_basicsize = sizeof(dmnsn_py_Vector),
  .tp_repr = (reprfunc)dmnsn_py_Vector_repr,
  .tp_str = (reprfunc)dmnsn_py_Vector_str,
  .tp_as_number = &dmnsn_py_Vector_as_number,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_doc = "Dimension vector",
  .tp_richcompare = dmnsn_py_Vector_richcompare,
  .tp_methods = dmnsn_py_Vector_methods,
  .tp_getset = dmnsn_py_Vector_getsetters,
  .tp_init = (initproc)dmnsn_py_Vector_init,
};

#define dmnsn_py_Vector_global(name)            \
  dmnsn_py_Vector name = {                      \
    PyObject_HEAD_INIT(&dmnsn_py_VectorType)    \
  };

dmnsn_py_Vector_global(dmnsn_py_Zero);
dmnsn_py_Vector_global(dmnsn_py_X);
dmnsn_py_Vector_global(dmnsn_py_Y);
dmnsn_py_Vector_global(dmnsn_py_Z);

bool
dmnsn_py_init_VectorType(void)
{
  dmnsn_py_Zero.v = dmnsn_zero;
  dmnsn_py_X.v    = dmnsn_x;
  dmnsn_py_Y.v    = dmnsn_y;
  dmnsn_py_Z.v    = dmnsn_z;

  dmnsn_py_VectorType.tp_new = PyType_GenericNew;
  return PyType_Ready(&dmnsn_py_VectorType) >= 0;
}
