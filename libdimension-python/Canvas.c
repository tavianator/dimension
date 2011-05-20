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

#include "Color.h"
#include "Canvas.h"

static int
dmnsn_py_Canvas_init(dmnsn_py_Canvas *self, PyObject *args, PyObject *kwds)
{
  static char *kwlist[] = { "width", "height", NULL };

  Py_ssize_t width, height;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "nn", kwlist, &width, &height))
    return -1;

  if (width < 0 || height < 0) {
    PyErr_SetString(PyExc_ValueError, "Canvas dimensions must be positive");
    return -1;
  }

  dmnsn_delete_canvas(self->canvas);
  self->canvas = dmnsn_new_canvas(width, height);
  DMNSN_INCREF(self->canvas);
  dmnsn_clear_canvas(self->canvas, dmnsn_black);
  return 0;
}

static void
dmnsn_py_Canvas_dealloc(dmnsn_py_Canvas *self)
{
  dmnsn_delete_canvas(self->canvas);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
dmnsn_py_Canvas_optimizePNG(dmnsn_py_Canvas *self)
{
  if (dmnsn_png_optimize_canvas(self->canvas) != 0)
    return PyErr_SetFromErrno(PyExc_OSError);

  /* Re-clear the canvas to clear the optimizer too */
  dmnsn_clear_canvas(self->canvas, dmnsn_black);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
dmnsn_py_Canvas_optimizeGL(dmnsn_py_Canvas *self)
{
  if (dmnsn_gl_optimize_canvas(self->canvas) != 0)
    return PyErr_SetFromErrno(PyExc_OSError);

  /* Re-clear the canvas to clear the optimizer too */
  dmnsn_clear_canvas(self->canvas, dmnsn_black);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
dmnsn_py_Canvas_clear(dmnsn_py_Canvas *self, PyObject *args, PyObject *kwds)
{
  dmnsn_color color;
  if (!dmnsn_py_Color_args(&color, args, kwds))
    return NULL;

  dmnsn_clear_canvas(self->canvas, color);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
dmnsn_py_Canvas_writePNG(dmnsn_py_Canvas *self, PyObject *args)
{
  PyObject *filenameobj;
  if (!PyArg_ParseTuple(args, "O&", PyUnicode_FSConverter, &filenameobj))
    return NULL;

  const char *filename = PyBytes_AS_STRING(filenameobj);
  FILE *file = fopen(filename, "wb");
  if (!file)
    return PyErr_SetFromErrno(PyExc_OSError);

  if (dmnsn_png_write_canvas(self->canvas, file) != 0)
    return PyErr_SetFromErrno(PyExc_OSError);

  if (fclose(file) != 0)
    return PyErr_SetFromErrno(PyExc_OSError);

  Py_DECREF(filenameobj);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
dmnsn_py_Canvas_drawGL(dmnsn_py_Canvas *self)
{
  if (dmnsn_gl_write_canvas(self->canvas) != 0)
    return PyErr_SetFromErrno(PyExc_OSError);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef dmnsn_py_Canvas_methods[] = {
  { "optimizePNG", (PyCFunction)dmnsn_py_Canvas_optimizePNG, METH_NOARGS,
    "Optimize a canvas for PNG output" },
  { "optimizeGL", (PyCFunction)dmnsn_py_Canvas_optimizeGL, METH_NOARGS,
    "Optimize a canvas for OpenGL output" },
  { "clear", (PyCFunction)dmnsn_py_Canvas_clear, METH_VARARGS | METH_KEYWORDS,
    "Clear a canvas with a solid color" },
  { "writePNG", (PyCFunction)dmnsn_py_Canvas_writePNG, METH_VARARGS,
    "Write a canvas to a PNG file" },
  { "drawGL", (PyCFunction)dmnsn_py_Canvas_drawGL, METH_NOARGS,
    "Draw a canvas to the screen with OpenGL" },
  { NULL }
};

static PyObject *
dmnsn_py_Canvas_get_width(dmnsn_py_Canvas *self, void *closure)
{
  return PyLong_FromSize_t(self->canvas->width);
}

static PyObject *
dmnsn_py_Canvas_get_height(dmnsn_py_Canvas *self, void *closure)
{
  return PyLong_FromSize_t(self->canvas->height);
}

static PyGetSetDef dmnsn_py_Canvas_getsetters[] = {
  { "width",  (getter)dmnsn_py_Canvas_get_width,  NULL, "Canvas width",  NULL },
  { "height", (getter)dmnsn_py_Canvas_get_height, NULL, "Canvas height", NULL },
  { NULL }
};

PyTypeObject dmnsn_py_CanvasType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name      = "dimension.Canvas",
  .tp_basicsize = sizeof(dmnsn_py_Canvas),
  .tp_dealloc   = (destructor)dmnsn_py_Canvas_dealloc,
  .tp_flags     = Py_TPFLAGS_DEFAULT,
  .tp_doc       = "Dimension canvas",
  .tp_methods   = dmnsn_py_Canvas_methods,
  .tp_getset    = dmnsn_py_Canvas_getsetters,
  .tp_init      = (initproc)dmnsn_py_Canvas_init,
};

bool
dmnsn_py_init_CanvasType(void)
{
  dmnsn_py_CanvasType.tp_new = PyType_GenericNew;
  return PyType_Ready(&dmnsn_py_CanvasType) >= 0;
}
