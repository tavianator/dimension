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
dmnsn_py_Color_init(dmnsn_py_Color *self, PyObject *args, PyObject *kwds)
{
  self->sRGB.trans  = 0.0;
  self->sRGB.filter = 0.0;

  static char *kwlist[] = { "red", "green", "blue", "trans", "filter", NULL };
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "ddd|dd", kwlist,
                                   &self->sRGB.R, &self->sRGB.G, &self->sRGB.B,
                                   &self->sRGB.trans, &self->sRGB.filter))
    return -1;

  self->c = dmnsn_color_from_sRGB(self->sRGB);
  return 0;
}

static PyObject *
dmnsn_py_Color_repr(dmnsn_py_Color *self)
{
  PyObject *R = PyFloat_FromDouble(self->sRGB.R);
  PyObject *G = PyFloat_FromDouble(self->sRGB.G);
  PyObject *B = PyFloat_FromDouble(self->sRGB.B);
  PyObject *trans  = PyFloat_FromDouble(self->sRGB.trans);
  PyObject *filter = PyFloat_FromDouble(self->sRGB.filter);

  if (!R || !G || !B || !trans || !filter) {
    Py_XDECREF(filter);
    Py_XDECREF(trans);
    Py_XDECREF(B);
    Py_XDECREF(G);
    Py_XDECREF(R);
    return NULL;
  }

  PyObject *repr = PyUnicode_FromFormat("dimension.Color(%R, %R, %R, %R, %R)",
                                        R, G, B, trans, filter);
  Py_XDECREF(filter);
  Py_XDECREF(trans);
  Py_XDECREF(B);
  Py_XDECREF(G);
  Py_XDECREF(R);
  return repr;
}

static PyObject *
dmnsn_py_Color_str(dmnsn_py_Color *self)
{
  PyObject *R = PyFloat_FromDouble(self->sRGB.R);
  PyObject *G = PyFloat_FromDouble(self->sRGB.G);
  PyObject *B = PyFloat_FromDouble(self->sRGB.B);
  PyObject *trans  = PyFloat_FromDouble(self->sRGB.trans);
  PyObject *filter = PyFloat_FromDouble(self->sRGB.filter);

  if (!R || !G || !B || !trans || !filter) {
    Py_XDECREF(filter);
    Py_XDECREF(trans);
    Py_XDECREF(B);
    Py_XDECREF(G);
    Py_XDECREF(R);
    return NULL;
  }

  PyObject *str;
  if (self->sRGB.filter < dmnsn_epsilon && self->sRGB.trans < dmnsn_epsilon) {
    str = PyUnicode_FromFormat("<red = %S, green = %S, blue = %S>",
                               R, G, B);
  } else {
    str = PyUnicode_FromFormat("<red = %S, green = %S, blue = %S,"
                               " trans = %S, filter = %S>",
                               R, G, B, trans, filter);
  }
  Py_XDECREF(filter);
  Py_XDECREF(trans);
  Py_XDECREF(B);
  Py_XDECREF(G);
  Py_XDECREF(R);
  return str;
}

static PyObject *
dmnsn_py_Color_richcompare(PyObject *lhs, PyObject *rhs, int op)
{
  if (!PyObject_TypeCheck(lhs, &dmnsn_py_ColorType)
      || !PyObject_TypeCheck(rhs, &dmnsn_py_ColorType))
  {
    PyErr_SetString(PyExc_TypeError,
                    "Colors can only be compared with Colors");
    return NULL;
  }

  dmnsn_py_Color *clhs = (dmnsn_py_Color *)lhs;
  dmnsn_py_Color *crhs = (dmnsn_py_Color *)rhs;

  double rdiff = (clhs->c.R - crhs->c.R)*(clhs->c.R - crhs->c.R);
  double gdiff = (clhs->c.G - crhs->c.G)*(clhs->c.G - crhs->c.G);
  double bdiff = (clhs->c.B - crhs->c.B)*(clhs->c.B - crhs->c.B);
  double tdiff = (clhs->c.trans - crhs->c.trans)
                 * (clhs->c.trans - crhs->c.trans);
  double fdiff = (clhs->c.filter - crhs->c.filter)
                 * (clhs->c.filter - crhs->c.filter);
  bool equal = sqrt(rdiff + gdiff + bdiff + tdiff + fdiff) < dmnsn_epsilon;

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
dmnsn_py_Color_add(PyObject *lhs, PyObject *rhs)
{
  if (!PyObject_TypeCheck(lhs, &dmnsn_py_ColorType)
      || !PyObject_TypeCheck(rhs, &dmnsn_py_ColorType))
  {
    PyErr_SetString(PyExc_TypeError,
                    "Colors can only be added to Colors");
    return NULL;
  }

  dmnsn_py_Color *ret = PyObject_New(dmnsn_py_Color, &dmnsn_py_ColorType);
  if (ret) {
    dmnsn_py_Color *clhs = (dmnsn_py_Color *)lhs;
    dmnsn_py_Color *crhs = (dmnsn_py_Color *)rhs;
    ret->sRGB = dmnsn_color_add(clhs->sRGB, crhs->sRGB);
    ret->c    = dmnsn_color_from_sRGB(ret->sRGB);
  }
  return (PyObject *)ret;
}

static PyObject *
dmnsn_py_Color_mul(PyObject *lhs, PyObject *rhs)
{
  dmnsn_py_Color *col;
  double dbl;

  if (PyObject_TypeCheck(lhs, &dmnsn_py_ColorType)) {
    col = (dmnsn_py_Color *)lhs;
    dbl = PyFloat_AsDouble(rhs);
    if (PyErr_Occurred())
      return NULL;
  } else {
    col = (dmnsn_py_Color *)rhs;
    dbl = PyFloat_AsDouble(lhs);
    if (PyErr_Occurred())
      return NULL;
  }

  dmnsn_py_Color *ret = PyObject_New(dmnsn_py_Color, &dmnsn_py_ColorType);
  if (ret) {
    ret->sRGB = dmnsn_color_mul(dbl, col->sRGB);
    ret->c    = dmnsn_color_from_sRGB(ret->sRGB);
  }
  return (PyObject *)ret;
}

static int
dmnsn_py_Color_bool(PyObject *obj)
{
  dmnsn_py_Color *col = (dmnsn_py_Color *)obj;
  return !dmnsn_color_is_black(col->c);
}

static PyNumberMethods dmnsn_py_Color_as_number = {
  .nb_add         = dmnsn_py_Color_add,
  .nb_multiply    = dmnsn_py_Color_mul,
  .nb_bool        = dmnsn_py_Color_bool,
};

static PyMethodDef dmnsn_py_Color_methods[] = {
  { NULL }
};

static PyObject *
dmnsn_py_Color_get_red(dmnsn_py_Color *self, void *closure)
{
  return PyFloat_FromDouble(self->sRGB.R);
}

static PyObject *
dmnsn_py_Color_get_green(dmnsn_py_Color *self, void *closure)
{
  return PyFloat_FromDouble(self->sRGB.G);
}

static PyObject *
dmnsn_py_Color_get_blue(dmnsn_py_Color *self, void *closure)
{
  return PyFloat_FromDouble(self->sRGB.B);
}

static PyObject *
dmnsn_py_Color_get_trans(dmnsn_py_Color *self, void *closure)
{
  return PyFloat_FromDouble(self->sRGB.trans);
}

static PyObject *
dmnsn_py_Color_get_filter(dmnsn_py_Color *self, void *closure)
{
  return PyFloat_FromDouble(self->sRGB.filter);
}

static PyGetSetDef dmnsn_py_Color_getsetters[] = {
  { "red",    (getter)dmnsn_py_Color_get_red,    NULL,
    "Red component",           NULL },
  { "green",  (getter)dmnsn_py_Color_get_green,  NULL,
    "Green componant",         NULL },
  { "blue",   (getter)dmnsn_py_Color_get_blue,   NULL,
    "Blue componant",          NULL },
  { "trans",  (getter)dmnsn_py_Color_get_trans,  NULL,
    "Transmittance component", NULL },
  { "filter", (getter)dmnsn_py_Color_get_filter, NULL,
    "Filter component",        NULL },
  { NULL }
};

PyTypeObject dmnsn_py_ColorType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "dimension.Color",
  .tp_basicsize = sizeof(dmnsn_py_Color),
  .tp_repr = (reprfunc)dmnsn_py_Color_repr,
  .tp_str = (reprfunc)dmnsn_py_Color_str,
  .tp_as_number = &dmnsn_py_Color_as_number,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_doc = "Dimension color",
  .tp_richcompare = dmnsn_py_Color_richcompare,
  .tp_methods = dmnsn_py_Color_methods,
  .tp_getset = dmnsn_py_Color_getsetters,
  .tp_init = (initproc)dmnsn_py_Color_init,
};

#define dmnsn_py_Color_global(name)             \
  dmnsn_py_Color name = {                       \
    PyObject_HEAD_INIT(&dmnsn_py_ColorType)     \
  };

dmnsn_py_Color_global(dmnsn_py_Black);
dmnsn_py_Color_global(dmnsn_py_White);
dmnsn_py_Color_global(dmnsn_py_Clear);
dmnsn_py_Color_global(dmnsn_py_Red);
dmnsn_py_Color_global(dmnsn_py_Green);
dmnsn_py_Color_global(dmnsn_py_Blue);
dmnsn_py_Color_global(dmnsn_py_Magenta);
dmnsn_py_Color_global(dmnsn_py_Orange);
dmnsn_py_Color_global(dmnsn_py_Yellow);
dmnsn_py_Color_global(dmnsn_py_Cyan);

bool
dmnsn_py_init_ColorType(void)
{
#define dmnsn_py_define_global(global, color)   \
  do {                                          \
    (global).c    = color;                      \
    (global).sRGB = dmnsn_color_to_sRGB(color); \
  } while (0)

  dmnsn_py_define_global(dmnsn_py_Black,   dmnsn_black);
  dmnsn_py_define_global(dmnsn_py_White,   dmnsn_white);
  dmnsn_py_define_global(dmnsn_py_Clear,   dmnsn_clear);
  dmnsn_py_define_global(dmnsn_py_Red,     dmnsn_red);
  dmnsn_py_define_global(dmnsn_py_Green,   dmnsn_green);
  dmnsn_py_define_global(dmnsn_py_Blue,    dmnsn_blue);
  dmnsn_py_define_global(dmnsn_py_Magenta, dmnsn_magenta);
  dmnsn_py_define_global(dmnsn_py_Orange,  dmnsn_orange);
  dmnsn_py_define_global(dmnsn_py_Yellow,  dmnsn_yellow);
  dmnsn_py_define_global(dmnsn_py_Cyan,    dmnsn_cyan);

  dmnsn_py_ColorType.tp_new = PyType_GenericNew;
  return PyType_Ready(&dmnsn_py_ColorType) >= 0;
}
