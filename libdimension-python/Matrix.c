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
dmnsn_py_Matrix_init(dmnsn_py_Matrix *self, PyObject *args, PyObject *kwds)
{
  if (kwds) {
    PyErr_SetString(PyExc_TypeError, "Keyword arguments not supported");
    return -1;
  }

  if (!PyArg_ParseTuple(args, "dddddddddddd",
                        &self->m.n[0][0], &self->m.n[0][1], &self->m.n[0][2],
                        &self->m.n[0][3],
                        &self->m.n[1][0], &self->m.n[1][1], &self->m.n[1][2],
                        &self->m.n[1][3],
                        &self->m.n[2][0], &self->m.n[2][1], &self->m.n[2][2],
                        &self->m.n[2][3]))
    return -1;

  return 0;
}

PyObject *
dmnsn_py_Matrix_scale(PyObject *self, PyObject *args)
{
  dmnsn_vector scale;
  if (!PyArg_ParseTuple(args, "O&", dmnsn_py_VectorParse, &scale))
    return NULL;

  dmnsn_py_Matrix *ret = PyObject_New(dmnsn_py_Matrix, &dmnsn_py_MatrixType);
  if (ret) {
    ret->m = dmnsn_scale_matrix(scale);
  }
  return (PyObject *)ret;
}

PyObject *
dmnsn_py_Matrix_translate(PyObject *self, PyObject *args)
{
  dmnsn_vector translate;
  if (!PyArg_ParseTuple(args, "O&", dmnsn_py_VectorParse, &translate))
    return NULL;

  dmnsn_py_Matrix *ret = PyObject_New(dmnsn_py_Matrix, &dmnsn_py_MatrixType);
  if (ret) {
    ret->m = dmnsn_translation_matrix(translate);
  }
  return (PyObject *)ret;
}

PyObject *
dmnsn_py_Matrix_rotate(PyObject *self, PyObject *args)
{
  dmnsn_vector rotate;
  if (!PyArg_ParseTuple(args, "O&", dmnsn_py_VectorParse, &rotate))
    return NULL;

  dmnsn_py_Matrix *ret = PyObject_New(dmnsn_py_Matrix, &dmnsn_py_MatrixType);
  if (ret) {
    ret->m = dmnsn_rotation_matrix(dmnsn_vector_mul(dmnsn_radians(1.0),
                                                    rotate));
  }
  return (PyObject *)ret;
}

static PyObject *
dmnsn_py_Matrix_repr(dmnsn_py_Matrix *self)
{
  PyObject *floats[3][4] = { { NULL } };
  bool initialized = true;
  for (size_t i = 0; i < 3; ++i) {
    for (size_t j = 0; j < 4; ++j) {
      floats[i][j] = PyFloat_FromDouble(self->m.n[i][j]);
      if (!floats[i][j]) {
        initialized = false;
        break;
      }
    }
    if (!initialized)
      break;
  }

  if (!initialized) {
    for (size_t i = 0; i < 3; ++i) {
      for (size_t j = 0; j < 4; ++j) {
        Py_XDECREF(floats[i][j]);
      }
    }
    return NULL;
  }

  PyObject *repr = PyUnicode_FromFormat("dimension.Matrix(%R, %R, %R, %R, "
                                                         "%R, %R, %R, %R, "
                                                         "%R, %R, %R, %R)",
                                        floats[0][0], floats[0][1],
                                        floats[0][2], floats[0][3],
                                        floats[1][0], floats[1][1],
                                        floats[1][2], floats[1][3],
                                        floats[2][0], floats[2][1],
                                        floats[2][2], floats[2][3]);
  for (size_t i = 0; i < 3; ++i) {
    for (size_t j = 0; j < 4; ++j) {
      Py_DECREF(floats[i][j]);
    }
  }
  return repr;
}

static PyObject *
dmnsn_py_Matrix_str(dmnsn_py_Matrix *self)
{
  PyObject *floats[3][4] = { { NULL } };
  bool initialized = true;
  for (size_t i = 0; i < 3; ++i) {
    for (size_t j = 0; j < 4; ++j) {
      floats[i][j] = PyFloat_FromDouble(self->m.n[i][j]);
      if (!floats[i][j]) {
        initialized = false;
        break;
      }
    }
    if (!initialized)
      break;
  }

  if (!initialized) {
    for (size_t i = 0; i < 3; ++i) {
      for (size_t j = 0; j < 4; ++j) {
        Py_XDECREF(floats[i][j]);
      }
    }
    return NULL;
  }

  PyObject *str = PyUnicode_FromFormat("\n[%S\t%S\t%S\t%S]\n"
                                       "[%S\t%S\t%S\t%S]\n"
                                       "[%S\t%S\t%S\t%S]\n"
                                       "[0.0\t0.0\t0.0\t1.0]",
                                        floats[0][0], floats[0][1],
                                        floats[0][2], floats[0][3],
                                        floats[1][0], floats[1][1],
                                        floats[1][2], floats[1][3],
                                        floats[2][0], floats[2][1],
                                        floats[2][2], floats[2][3]);
  for (size_t i = 0; i < 3; ++i) {
    for (size_t j = 0; j < 4; ++j) {
      Py_DECREF(floats[i][j]);
    }
  }
  return str;
}

static PyObject *
dmnsn_py_Matrix_richcompare(PyObject *lhs, PyObject *rhs, int op)
{
  if (!PyObject_TypeCheck(lhs, &dmnsn_py_MatrixType)
      || !PyObject_TypeCheck(rhs, &dmnsn_py_MatrixType))
  {
    PyErr_SetString(PyExc_TypeError,
                    "Matricies can only be compared with Matricies");
    return NULL;
  }

  dmnsn_py_Matrix *mlhs = (dmnsn_py_Matrix *)lhs;
  dmnsn_py_Matrix *mrhs = (dmnsn_py_Matrix *)rhs;

  double sqdiff = 0.0;
  for (size_t i = 0; i < 3; ++i) {
    for (size_t j = 0; j < 4; ++j) {
      double diff = mlhs->m.n[i][j] - mrhs->m.n[i][j];
      sqdiff += diff*diff;
    }
  }

  bool equal = sqrt(sqdiff) < dmnsn_epsilon;

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
dmnsn_py_Matrix_mul(PyObject *lhs, PyObject *rhs)
{
  if (!PyObject_TypeCheck(lhs, &dmnsn_py_MatrixType)) {
    PyErr_SetString(PyExc_TypeError,
                    "Left-hand side of matrix multiplication must be a matrix");
    return NULL;
  }

  dmnsn_py_Matrix *mlhs = (dmnsn_py_Matrix *)lhs;

  if (PyObject_TypeCheck(rhs, &dmnsn_py_MatrixType)) {
    dmnsn_py_Matrix *mrhs = (dmnsn_py_Matrix *)rhs;
    dmnsn_py_Matrix *ret = PyObject_New(dmnsn_py_Matrix, &dmnsn_py_MatrixType);
    if (ret) {
      ret->m = dmnsn_matrix_mul(mlhs->m, mrhs->m);
    }
    return (PyObject *)ret;
  } else if (PyObject_TypeCheck(rhs, &dmnsn_py_VectorType)) {
    dmnsn_py_Vector *vrhs = (dmnsn_py_Vector *)rhs;
    dmnsn_py_Vector *ret = PyObject_New(dmnsn_py_Vector, &dmnsn_py_VectorType);
    if (ret) {
      ret->v = dmnsn_transform_vector(mlhs->m, vrhs->v);
    }
    return (PyObject *)ret;
  } else {
    PyErr_SetString(PyExc_TypeError,
                    "Right-hand side of matrix multiplication must be a matrix"
                    " or vector");
    return NULL;
  }
}

static PyNumberMethods dmnsn_py_Matrix_as_number = {
  .nb_multiply    = dmnsn_py_Matrix_mul,
};

static PyObject *
dmnsn_py_Matrix_inverse(dmnsn_py_Matrix *self)
{
  dmnsn_py_Matrix *ret = PyObject_New(dmnsn_py_Matrix, &dmnsn_py_MatrixType);
  if (ret) {
    ret->m = dmnsn_matrix_inverse(self->m);
  }
  return (PyObject *)ret;
}

static PyMethodDef dmnsn_py_Matrix_methods[] = {
  { "inverse", (PyCFunction)dmnsn_py_Matrix_inverse, METH_NOARGS,
    "Return the inverse of the matrix" },
  { NULL }
};

static PyGetSetDef dmnsn_py_Matrix_getsetters[] = {
  { NULL }
};

PyTypeObject dmnsn_py_MatrixType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "dimension.Matrix",
  .tp_basicsize = sizeof(dmnsn_py_Matrix),
  .tp_repr = (reprfunc)dmnsn_py_Matrix_repr,
  .tp_str = (reprfunc)dmnsn_py_Matrix_str,
  .tp_as_number = &dmnsn_py_Matrix_as_number,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_doc = "Dimension matrix",
  .tp_richcompare = dmnsn_py_Matrix_richcompare,
  .tp_methods = dmnsn_py_Matrix_methods,
  .tp_getset = dmnsn_py_Matrix_getsetters,
  .tp_init = (initproc)dmnsn_py_Matrix_init,
};

bool
dmnsn_py_init_MatrixType(void)
{
  dmnsn_py_MatrixType.tp_new = PyType_GenericNew;
  Py_INCREF(&dmnsn_py_MatrixType);
  return PyType_Ready(&dmnsn_py_MatrixType) >= 0;
}
