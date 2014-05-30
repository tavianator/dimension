#########################################################################
# Copyright (C) 2011-2014 Tavian Barnes <tavianator@tavianator.com>     #
#                                                                       #
# This file is part of The Dimension Python Module.                     #
#                                                                       #
# The Dimension Python Module is free software; you can redistribute it #
# and/or modify it under the terms of the GNU Lesser General Public     #
# License as published by the Free Software Foundation; either version  #
# 3 of the License, or (at your option) any later version.              #
#                                                                       #
# The Dimension Python Module is distributed in the hope that it will   #
# be useful, but WITHOUT ANY WARRANTY; without even the implied         #
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See #
# the GNU Lesser General Public License for more details.               #
#                                                                       #
# You should have received a copy of the GNU Lesser General Public      #
# License along with this program.  If not, see                         #
# <http://www.gnu.org/licenses/>.                                       #
#########################################################################

"""
Dimension: a high-performance photo-realistic 3D renderer.
"""

import os

###########
# Helpers #
###########

cdef _raise_OSError(filename = None):
  if filename is None:
    raise OSError(errno, os.strerror(errno))
  else:
    raise OSError(errno, os.strerror(errno), filename)

###########
# Globals #
###########

def die_on_warnings(always_die):
  """Whether to treat Dimension warnings as errors."""
  dmnsn_die_on_warnings(always_die)

def terminal_width():
  """Return the width of the terminal, if present."""
  return dmnsn_terminal_width()

#########
# Pools #
#########

cdef class _Pool:
  cdef dmnsn_pool *_pool

  def __cinit__(self):
    self._pool = dmnsn_new_pool()

  def __dealloc__(self):
    dmnsn_delete_pool(self._pool)

cdef _Pool _global_pool = _Pool()

cdef dmnsn_pool *_get_pool():
  return _global_pool._pool

###########
# Futures #
###########

cdef class Future:
  cdef dmnsn_future *_future
  cdef _finalizer

  def __cinit__(self):
    self._future = NULL
    self._finalizer = None

  def __init__(self):
    raise RuntimeError("attempt to create a Future object.")

  def __dealloc__(self):
    if self._future != NULL:
      self.join()

  def join(self):
    self._assert_unfinished()
    cdef int retcode
    try:
      with nogil:
        retcode = dmnsn_future_join(self._future)
      if retcode != 0:
        raise RuntimeError("background task failed.")
      if self._finalizer is not None:
        self._finalizer()
    finally:
      self._future = NULL

  def cancel(self):
    self._assert_unfinished()
    dmnsn_future_cancel(self._future)

  def progress(self):
    self._assert_unfinished()
    return dmnsn_future_progress(self._future)
  def is_done(self):
    self._assert_unfinished()
    return dmnsn_future_is_done(self._future)

  def wait(self, double progress):
    self._assert_unfinished()
    with nogil:
      dmnsn_future_wait(self._future, progress)

  def pause(self):
    with nogil:
      dmnsn_future_pause(self._future)
  def resume(self):
    dmnsn_future_resume(self._future)

  # Let Futures be used as context managers
  def __enter__(self):
    return self
  def __exit__(self, exc_type, exc_value, traceback):
    if self._future != NULL:
      if exc_value is not None:
        self.cancel()
      self.join()
    return False

  def _assert_unfinished(self):
    if self._future == NULL:
      raise RuntimeError("background task finished.")

cdef Future _Future(dmnsn_future *future):
  """Wrap a Future object around an existing dmnsn_future *."""
  cdef Future self = Future.__new__(Future)
  self._future = future
  return self

##########
# Timers #
##########

cdef class Timer:
  """A timer for Dimension tasks."""
  cdef dmnsn_timer _timer
  cdef bool _stopped

  def __init__(self):
    """
    Create a Timer.

    Timing starts as soon as the object is created.
    """
    self._stopped = False
    dmnsn_timer_start(&self._timer)

  def stop(self):
    """Stop the Timer."""
    if self._stopped:
      raise RuntimeError("timer already stopped.")

    dmnsn_timer_stop(&self._timer)
    self._stopped = True

  property real:
    """Real (wall clock) time."""
    def __get__(self):
      self._assert_stopped()
      return self._timer.real
  property user:
    """User (CPU) time."""
    def __get__(self):
      self._assert_stopped()
      return self._timer.user
  property system:
    """System time."""
    def __get__(self):
      self._assert_stopped()
      return self._timer.system

  def __str__(self):
    self._assert_stopped()
    return "%.2fs (user: %.2fs; system: %.2fs)" % \
           (self._timer.real, self._timer.user, self._timer.system)

  def _assert_stopped(self):
    if not self._stopped:
      raise RuntimeError("timer still running.")

cdef Timer _Timer(dmnsn_timer timer):
  """Wrap a Timer object around a dmnsn_timer."""
  cdef Timer self = Timer.__new__(Timer)
  self._timer = timer
  self._stopped = True
  return self

############
# Geometry #
############

cdef class Vector:
  """A vector (or point or pseudovector) in 3D space."""
  cdef dmnsn_vector _v

  def __init__(self, *args, **kwargs):
    """
    Create a Vector.

    Keyword arguments:
    x -- The x coordinate
    y -- The y coordinate
    z -- The z coordinate

    Alternatively, you can pass another Vector, the value 0, or a tuple or other
    sequence (x, y, z).
    """
    if len(args) == 1:
      if isinstance(args[0], Vector):
        self._v = (<Vector>args[0])._v
      elif hasattr(args[0], "__iter__"): # Faster than try: ... except:
        self._real_init(*args[0])
      elif args[0] == 0:
        self._v = dmnsn_zero
      else:
        raise TypeError("expected a sequence or 0")
    else:
      self._real_init(*args, **kwargs)

  def _real_init(self, double x, double y, double z):
    self._v = dmnsn_new_vector(x, y, z)

  property x:
    """The x coordinate."""
    def __get__(self):
      return self._v.x
  property y:
    """The y coordinate."""
    def __get__(self):
      return self._v.y
  property z:
    """The z coordinate."""
    def __get__(self):
      return self._v.z

  def __pos__(self):
    return self
  def __neg__(self):
    return _Vector(dmnsn_vector_negate(self._v))
  def __nonzero__(self):
    return dmnsn_vector_norm(self._v) >= dmnsn_epsilon

  def __add__(lhs, rhs):
    return _Vector(dmnsn_vector_add(Vector(lhs)._v, Vector(rhs)._v))
  def __sub__(lhs, rhs):
    return _Vector(dmnsn_vector_sub(Vector(lhs)._v, Vector(rhs)._v))
  def __mul__(lhs, rhs):
    if isinstance(lhs, Vector):
      return _Vector(dmnsn_vector_mul(rhs, (<Vector>lhs)._v))
    else:
      return _Vector(dmnsn_vector_mul(lhs, (<Vector?>rhs)._v))
  def __truediv__(Vector lhs not None, double rhs):
    return _Vector(dmnsn_vector_div(lhs._v, rhs))

  def __richcmp__(lhs, rhs, int op):
    equal = (Vector(lhs) - Vector(rhs)).norm() < dmnsn_epsilon
    if op == 2:   # ==
      return equal
    elif op == 3: # !=
      return not equal
    else:
      return NotImplemented

  def norm(self):
    """Return the magnitude of the vector."""
    return dmnsn_vector_norm(self._v)
  def normalized(self):
    """Return the direction of the vector."""
    return _Vector(dmnsn_vector_normalized(self._v))

  def __repr__(self):
    return "dimension.Vector(%r, %r, %r)" % (self.x, self.y, self.z)

  def __str__(self):
    return "<%s, %s, %s>" % (self.x, self.y, self.z)

cdef Vector _Vector(dmnsn_vector v):
  """Wrap a Vector object around a dmnsn_vector."""
  cdef Vector self = Vector.__new__(Vector)
  self._v = v
  return self

def cross(Vector lhs not None, Vector rhs not None):
  """Vector cross product."""
  return _Vector(dmnsn_vector_cross(lhs._v, rhs._v))
def dot(Vector lhs not None, Vector rhs not None):
  """Vector dot product."""
  return dmnsn_vector_dot(lhs._v, rhs._v)
def proj(Vector u not None, Vector d not None):
  """Vector projection (of u onto d)."""
  return _Vector(dmnsn_vector_proj(u._v, d._v))

X = _Vector(dmnsn_x)
Y = _Vector(dmnsn_y)
Z = _Vector(dmnsn_z)

cdef class Matrix:
  """An affine transformation matrix."""
  cdef dmnsn_matrix _m

  def __init__(self,
               double a1, double a2, double a3, double a4,
               double b1, double b2, double b3, double b4,
               double c1, double c2, double c3, double c4):
    """Create a Matrix."""
    self._m = dmnsn_new_matrix(a1, a2, a3, a4,
                               b1, b2, b3, b4,
                               c1, c2, c3, c4)

  def __nonzero__(self):
    cdef double sum = 0.0
    for i in range(3):
      for j in range(4):
        sum += self._m.n[i][j]
    return sqrt(sum) >= dmnsn_epsilon

  def __mul__(Matrix lhs not None, rhs):
    if isinstance(rhs, Matrix):
      return _Matrix(dmnsn_matrix_mul(lhs._m, (<Matrix>rhs)._m))
    else:
      return _Vector(dmnsn_transform_point(lhs._m, (<Vector?>rhs)._v))

  def __richcmp__(Matrix lhs not None, Matrix rhs not None, int op):
    cdef double sum = 0.0
    for i in range(3):
      for j in range(4):
        diff = lhs._m.n[i][j] - rhs._m.n[i][j]
        sum += diff*diff
    equal = sqrt(sum) < dmnsn_epsilon

    if op == 2:   # ==
      return equal
    elif op == 3: # !=
      return not equal
    else:
      return NotImplemented

  def inverse(self):
    """Return the inverse of a matrix."""
    return _Matrix(dmnsn_matrix_inverse(self._m))

  def __repr__(self):
    return \
      "dimension.Matrix(%r, %r, %r, %r, %r, %r, %r, %r, %r, %r, %r, %r)" % \
      (self._m.n[0][0], self._m.n[0][1], self._m.n[0][2], self._m.n[0][3],
       self._m.n[1][0], self._m.n[1][1], self._m.n[1][2], self._m.n[1][3],
       self._m.n[2][0], self._m.n[2][1], self._m.n[2][2], self._m.n[2][3])

  def __str__(self):
    return \
      "\n[%s\t%s\t%s\t%s]" \
      "\n[%s\t%s\t%s\t%s]" \
      "\n[%s\t%s\t%s\t%s]" \
      "\n[%s\t%s\t%s\t%s]" %\
      (self._m.n[0][0], self._m.n[0][1], self._m.n[0][2], self._m.n[0][3],
       self._m.n[1][0], self._m.n[1][1], self._m.n[1][2], self._m.n[1][3],
       self._m.n[2][0], self._m.n[2][1], self._m.n[2][2], self._m.n[2][3],
       0.0, 0.0, 0.0, 1.0)

cdef Matrix _Matrix(dmnsn_matrix m):
  """Wrap a Matrix object around a dmnsn_matrix."""
  cdef Matrix self = Matrix.__new__(Matrix)
  self._m = m
  return self

def scale(*args, **kwargs):
  """
  Return a scale transformation.

  Accepts the same arguments that Vector(...) does.  The transformation scales
  by a factor of x in the x direction, y in the y direction, and z in the z
  direction.  In particular, this means that scale(2*X) is probably a mistake,
  as the y and z coordinates will disappear.

  Alternatively, a single argument may be passed, which specifies the scaling
  factor in every component.
  """
  cdef Vector s
  try:
    s = Vector(*args, **kwargs)
  except:
    s = args[0]*(X + Y + Z)
  return _Matrix(dmnsn_scale_matrix(s._v))
def translate(*args, **kwargs):
  """
  Return a translation.

  Accepts the same arguments that Vector(...) does.
  """
  return _Matrix(dmnsn_translation_matrix(Vector(*args, **kwargs)._v))
def rotate(*args, **kwargs):
  """
  Return a rotation.

  Accepts the same arguments that Vector(...) does. theta.norm() is the left-
  handed angle of rotation, and theta.normalized() is the axis of rotation.
  theta is specified in degrees.
  """
  cdef Vector rad = dmnsn_radians(1.0)*Vector(*args, **kwargs)
  return _Matrix(dmnsn_rotation_matrix(rad._v))

cdef class _Transformable:
  def scale(self, *args, **kwargs):
    """Scale.  Equivalent to self.transform(scale(...))."""
    return self.transform(scale(*args, **kwargs))
  def translate(self, *args, **kwargs):
    """Translate.  Equivalent to self.transform(translate(...))."""
    return self.transform(translate(*args, **kwargs))
  def rotate(self, *args, **kwargs):
    """Rotate.  Equivalent to self.transform(rotate(...))."""
    return self.transform(rotate(*args, **kwargs))

##########
# Colors #
##########

cdef class _BaseColor:
  cdef dmnsn_color _c
  cdef dmnsn_color _clin

  def __init__(self, *args, **kwargs):
    """
    Create a color.

    Keyword arguments:
    red    -- The red component
    green  -- The green component
    blue   -- The blue component

    Alternatively, you can pass another Color, a gray intensity like 0.5, or a
    tuple or other sequence (red, green, blue).
    """
    if len(args) == 1 and len(kwargs) == 0:
      if isinstance(args[0], _BaseColor):
        self._clin = (<_BaseColor>args[0])._clin
        self._unlinearize()
        return
      elif hasattr(args[0], "__iter__"):
        self._real_init(*args[0])
      else:
        self._c = dmnsn_color_mul(args[0], dmnsn_white)
    else:
      self._real_init(*args, **kwargs)

    self._linearize()

  def _real_init(self, double red, double green, double blue):
    self._c = dmnsn_new_color(red, green, blue)

  property red:
    """The red component."""
    def __get__(self):
      return self._c.R
  property green:
    """The green component."""
    def __get__(self):
      return self._c.G
  property blue:
    """The blue component."""
    def __get__(self):
      return self._c.B

  def intensity(self):
    return dmnsn_color_intensity(self._c)
  def gray(self):
    return _Color(dmnsn_color_mul(self.intensity(), dmnsn_white), type(self))

  def __add__(lhs, rhs):
    if isinstance(lhs, _BaseColor):
      if isinstance(rhs, _BaseColor):
        if type(lhs) is not type(rhs):
          return NotImplemented
      else:
        rhs = type(lhs)(rhs)
    elif isinstance(rhs, _BaseColor):
      lhs = type(rhs)(lhs)
    else:
      return NotImplemented

    return _Color(dmnsn_color_add((<_BaseColor>lhs)._c, (<_BaseColor>rhs)._c),
                  type(lhs))

  def __mul__(lhs, rhs):
    if isinstance(lhs, _BaseColor):
      return _Color(dmnsn_color_mul(rhs, (<_BaseColor>lhs)._c), type(lhs))
    else:
      return _Color(dmnsn_color_mul(lhs, (<_BaseColor?>rhs)._c), type(rhs))

  def __truediv__(_BaseColor lhs not None, double rhs):
    return _Color(dmnsn_color_mul(1/rhs, lhs._c), type(lhs))

  def __richcmp__(lhs, rhs, int op):
    cdef clhs = Color(lhs)
    cdef crhs = Color(rhs)

    cdef double rdiff = clhs.red    - crhs.red
    cdef double gdiff = clhs.green  - crhs.green
    cdef double bdiff = clhs.blue   - crhs.blue
    cdef double sum = rdiff*rdiff + gdiff*gdiff + bdiff*bdiff
    equal = sqrt(sum) < dmnsn_epsilon
    if op == 2:   # ==
      return equal
    elif op == 3: # !=
      return not equal
    else:
      return NotImplemented

  def __repr__(self):
    return "dimension.%s(%r, %r, %r)" % \
           (type(self).__name__, self.red, self.green, self.blue)
  def __str__(self):
    return "%s<%s, %s, %s>" % \
           (type(self).__name__, self.red, self.green, self.blue)

cdef class Color(_BaseColor):
  """
  An object or light color.

  These colors are in a linear RGB space.  For colors in the sRGB space, which
  is used by the web and computer displays, see the sRGB class.
  """

  def _linearize(self):
    self._clin = self._c
  def _unlinearize(self):
    self._c = self._clin

cdef class sRGB(_BaseColor):
  """
  An sRGB color.

  Color operations with these colors occur in sRGB space, which is used by the
  web and computer displays.  However, it is not linear, so (for example) two
  lights with intensity sRGB(0.5) is not the same as one light with intensity
  sRGB(1).
  """

  def _linearize(self):
    self._clin = dmnsn_color_from_sRGB(self._c)
  def _unlinearize(self):
    self._c = dmnsn_color_to_sRGB(self._clin)

cdef _BaseColor _Color(dmnsn_color c, type t):
  """Wrap a _BaseColor subclass around a dmnsn_color."""
  cdef _BaseColor self = t.__new__(t)
  self._c = c
  self._linearize()
  return self

Black   = _Color(dmnsn_black, Color)
White   = _Color(dmnsn_white, Color)
Red     = _Color(dmnsn_red, Color)
Green   = _Color(dmnsn_green, Color)
Blue    = _Color(dmnsn_blue, Color)
Magenta = _Color(dmnsn_magenta, Color)
Orange  = _Color(dmnsn_orange, Color)
Yellow  = _Color(dmnsn_yellow, Color)
Cyan    = _Color(dmnsn_cyan, Color)

cdef class TColor:
  """
  A transparent color.

  This type is used for representing pigments and pixels, as it carries color as
  well as transparency information.
  """
  cdef dmnsn_tcolor _tc

  def __init__(self, *args, **kwargs):
    """
    Create a transparent color.

    Keyword arguments:
    color  -- The Color() (or sRGB()) component
    trans  -- The transparency component
    filter -- The proportion of the transparency that is filtered

    Alternatively, you can pass another TColor.
    """
    if len(args) == 1 and len(kwargs) == 0:
      if isinstance(args[0], TColor):
        self._tc = (<TColor>args[0])._tc
      else:
        self._real_init(*args, **kwargs)
    else:
      self._real_init(*args, **kwargs)

  def _real_init(self, color, double trans = 0, double filter = 0):
    self._tc = dmnsn_new_tcolor(Color(color)._clin, trans, filter)

  property color:
    """The color component."""
    def __get__(self):
      return _Color(self._tc.c, Color)
  property trans:
    """The transparency component."""
    def __get__(self):
      return self._tc.T
  property filter:
    """The filter proportion."""
    def __get__(self):
      return self._tc.F

  def __repr__(self):
    return "dimension.TColor(%r, %r, %r)" % \
           (self.color, self.trans, self.filter)
  def __str__(self):
    return "TColor<%s, %s, %s>" % \
           (self.color, self.trans, self.filter)

cdef TColor _TColor(dmnsn_tcolor tc):
  """Wrap a TColor around a dmnsn_tcolor."""
  cdef TColor self = TColor.__new__(TColor)
  self._tc = tc
  return self

Clear = _TColor(dmnsn_clear)

############
# Canvases #
############

cdef class Canvas:
  """A rendering target."""
  cdef dmnsn_canvas *_canvas

  def __init__(self, width, height):
    """
    Create a Canvas.

    Keyword arguments:
    width  -- the width of the canvas
    height -- the height of the canvas
    """
    self._canvas = dmnsn_new_canvas(width, height)
    self.clear(Black)

  def __dealloc__(self):
    dmnsn_delete_canvas(self._canvas)

  property width:
    """The width of the canvas."""
    def __get__(self):
      return self._canvas.width
  property height:
    """The height of the canvas."""
    def __get__(self):
      return self._canvas.height

  def __len__(self):
    """The width of the canvas."""
    return self.width
  def __getitem__(self, int x):
    """Get a column of the canvas."""
    if x < 0 or x >= self.width:
      raise IndexError("x coordinate out of bounds.")
    return _CanvasProxy(self, x)

  def optimize_PNG(self):
    """Optimize a canvas for PNG output."""
    if dmnsn_png_optimize_canvas(self._canvas) != 0:
      _raise_OSError()

  def optimize_GL(self):
    """Optimize a canvas for OpenGL output."""
    if dmnsn_gl_optimize_canvas(self._canvas) != 0:
      _raise_OSError()

  def clear(self, c):
    """Clear a canvas with a solid color."""
    dmnsn_canvas_clear(self._canvas, TColor(c)._tc)

  def write_PNG(self, path):
    """Export the canvas as a PNG file."""
    self.write_PNG_async(path).join()
  def write_PNG_async(self, path):
    """Export the canvas as a PNG file, in the background."""
    bpath = path.encode("UTF-8")
    cdef char *cpath = bpath
    cdef FILE *file = fopen(cpath, "wb")
    if file == NULL:
      _raise_OSError(path)

    def finalize():
      if fclose(file) != 0:
        _raise_OSError()

    cdef dmnsn_future *future = dmnsn_png_write_canvas_async(self._canvas, file)

    try:
      if future == NULL:
        _raise_OSError()

      ret = _Future(future)
      ret._finalizer = finalize
      return ret
    except:
      finalize()
      raise

  def draw_GL(self):
    """Export the canvas to the current OpenGL context."""
    if dmnsn_gl_write_canvas(self._canvas) != 0:
      _raise_OSError()

cdef class _CanvasProxy:
  cdef dmnsn_canvas *_canvas
  cdef int _x

  def __init__(self, Canvas canvas not None, int x):
    self._canvas = canvas._canvas
    self._x = x

  def __len__(self):
    """The height of the canvas."""
    return self._canvas.height
  def __getitem__(self, int y):
    self._bounds_check(y)
    return _TColor(dmnsn_canvas_get_pixel(self._canvas, self._x, y))
  def __setitem__(self, int y, color):
    self._bounds_check(y)
    dmnsn_canvas_set_pixel(self._canvas, self._x, y, TColor(color)._tc)

  def _bounds_check(self, int y):
    if y < 0 or y >= self._canvas.height:
      raise IndexError("y coordinate out of bounds.")

############
# Patterns #
############

cdef class Pattern:
  """A function which maps points in 3D space to scalar values."""
  cdef dmnsn_pattern *_pattern

  def __cinit__(self):
    self._pattern = NULL

  def __dealloc__(self):
    dmnsn_delete_pattern(self._pattern)

cdef class Checker(Pattern):
  """A checkerboard pattern."""
  def __init__(self):
    self._pattern = dmnsn_new_checker_pattern()
    Pattern.__init__(self)

cdef class Gradient(Pattern):
  """A gradient pattern."""
  def __init__(self, orientation):
    """
    Create a gradient pattern.

    Keyword arguments:
    orientation -- The direction of the linear gradient.
    """
    self._pattern = dmnsn_new_gradient_pattern(Vector(orientation)._v)
    Pattern.__init__(self)

cdef class Leopard(Pattern):
  """A leopard pattern."""
  def __init__(self):
    self._pattern = dmnsn_new_leopard_pattern()
    Pattern.__init__(self)

############
# Pigments #
############

cdef class Pigment(_Transformable):
  """Object surface coloring."""
  cdef dmnsn_pigment *_pigment

  def __cinit__(self):
    self._pigment = NULL

  def __init__(self, quick_color = None):
    """
    Create a Pigment.

    With an arguement, create a solid pigment of that color.  Otherwise, create
    a base Pigment.

    Keyword arguments:
    quick_color  -- the object's quick color for low-quality renders
    """
    if quick_color is not None:
      if self._pigment == NULL:
        if isinstance(quick_color, Pigment):
          self._pigment = (<Pigment>quick_color)._pigment
          DMNSN_INCREF(self._pigment)
        else:
          self._pigment = dmnsn_new_solid_pigment(TColor(quick_color)._tc)
      else:
        self._pigment.quick_color = TColor(quick_color)._tc

  def __dealloc__(self):
    dmnsn_delete_pigment(self._pigment)

  def transform(self, Matrix trans not None):
    """Transform a pigment."""
    self._pigment.trans = dmnsn_matrix_mul(trans._m, self._pigment.trans)
    return self

cdef Pigment _Pigment(dmnsn_pigment *pigment):
  """Wrap a Pigment object around a dmnsn_pigment *."""
  cdef Pigment self = Pigment.__new__(Pigment)
  self._pigment = pigment
  DMNSN_INCREF(self._pigment)
  return self

cdef class ImageMap(Pigment):
  """An image-mapped pigment."""
  def __init__(self, path, *args, **kwargs):
    """
    Create an ImageMap.

    Keyword arguments:
    path -- the path of the PNG file to open
    """
    bpath = path.encode("UTF-8")
    cdef char *cpath = bpath
    cdef FILE *file = fopen(cpath, "rb")
    if file == NULL:
      _raise_OSError(path)
    cdef dmnsn_canvas *canvas = dmnsn_png_read_canvas(file)
    if canvas == NULL:
      _raise_OSError(path)
    if fclose(file) != 0:
      _raise_OSError()

    self._pigment = dmnsn_new_canvas_pigment(canvas)
    Pigment.__init__(self, *args, **kwargs)

cdef class PigmentMap(Pigment):
  """A pigment map."""
  def __init__(self, Pattern pattern not None, map, bool sRGB not None = True,
               *args, **kwargs):
    """
    Create a PigmentMap.

    Keyword arguments:
    pattern -- the pattern to use for the mapping
    map     -- a dictionary of the form { val1: color1, val2: pigment2, ... },
               or a list of the form [color1, pigment2, ...]
    sRGB    -- whether the gradients should be in sRGB or linear space
               (default True)
    """
    cdef dmnsn_map *pigment_map = dmnsn_new_pigment_map()
    cdef dmnsn_pigment *real_pigment
    if hasattr(map, "items"):
      for i, pigment in map.items():
        pigment = Pigment(pigment)
        real_pigment = (<Pigment>pigment)._pigment
        DMNSN_INCREF(real_pigment)
        dmnsn_map_add_entry(pigment_map, i, &real_pigment)
    else:
      for i, pigment in enumerate(map):
        pigment = Pigment(pigment)
        real_pigment = (<Pigment>pigment)._pigment
        DMNSN_INCREF(real_pigment)
        dmnsn_map_add_entry(pigment_map, i/len(map), &real_pigment)

    cdef dmnsn_pigment_map_flags flags
    if sRGB:
      flags = DMNSN_PIGMENT_MAP_SRGB
    else:
      flags = DMNSN_PIGMENT_MAP_REGULAR

    DMNSN_INCREF(pattern._pattern)
    self._pigment = dmnsn_new_pigment_map_pigment(pattern._pattern, pigment_map,
                                                  flags)
    Pigment.__init__(self, *args, **kwargs)

############
# Finishes #
############

cdef class Finish:
  """Object surface qualities."""
  cdef dmnsn_finish _finish

  def __cinit__(self):
    self._finish = dmnsn_new_finish()

  def __dealloc__(self):
    dmnsn_delete_finish(self._finish)

  def __add__(Finish lhs not None, Finish rhs not None):
    """
    Combine two finishes.

    In lhs + rhs, the attributes of rhs override those of lhs if any conflict;
    thus, Ambient(0.1) + Ambient(0.2) is the same as Ambient(0.2)
    """
    cdef Finish ret = Finish()
    dmnsn_finish_cascade(&lhs._finish, &ret._finish)
    dmnsn_finish_cascade(&rhs._finish, &ret._finish) # rhs gets priority
    return ret

cdef Finish _Finish(dmnsn_finish finish):
  """Wrap a Finish object around a dmnsn_finish."""
  cdef Finish self = Finish.__new__(Finish)
  self._finish = finish
  dmnsn_finish_incref(&self._finish)
  return self

cdef class Ambient(Finish):
  """Ambient light reflected."""
  def __init__(self, color):
    """
    Create an Ambient finish.

    Keyword arguments:
    color -- the color and intensity of the ambient light
    """
    self._finish.ambient = dmnsn_new_ambient(Color(color)._c)

cdef class Diffuse(Finish):
  """Lambertian diffuse reflection."""
  def __init__(self, diffuse):
    """
    Create a Diffuse finish.

    Keyword arguments:
    diffuse -- the intensity of the diffuse reflection
    """
    self._finish.diffuse = dmnsn_new_lambertian(Color(diffuse).intensity())

cdef class Phong(Finish):
  """Phong specular highlight."""
  def __init__(self, strength, double size = 40.0):
    """
    Create a Phong highlight.

    Keyword arguments:
    strength -- the strength of the Phong highlight
    size -- the "shininess" of the material
    """
    self._finish.specular = dmnsn_new_phong(Color(strength).intensity(), size)

cdef class Reflection(Finish):
  """Reflective finish."""
  def __init__(self, min, max = None, double falloff = 1.0):
    """
    Create a Reflection.

    Keyword arguments:
    min     -- color and intensity of reflection at indirect angles
    max     -- color and intensity of reflection at direct angles (default: min)
    falloff -- exponent for reflection falloff (default: 1.0)
    """
    if max is None:
      max = min

    self._finish.reflection = dmnsn_new_basic_reflection(Color(min)._c,
                                                         Color(max)._c,
                                                         falloff)

############
# Textures #
############

cdef class Texture(_Transformable):
  """Object surface properties."""
  cdef dmnsn_texture *_texture

  def __init__(self, pigment = None, Finish finish = None):
    """
    Create a Texture.

    Keyword arguments:
    pigment -- the Pigment for the texture, or a color (default: None)
    finish  -- the Finish for the texture (default: None)
    """
    self._texture = dmnsn_new_texture()

    if pigment is not None:
      self.pigment = Pigment(pigment)

    if finish is not None:
      self.finish = finish

  def __dealloc__(self):
    dmnsn_delete_texture(self._texture)

  property pigment:
    """The texture's pigment."""
    def __get__(self):
      if self._texture.pigment == NULL:
        return None
      else:
        return _Pigment(self._texture.pigment)
    def __set__(self, pigment):
      dmnsn_delete_pigment(self._texture.pigment)
      cdef Pigment real_pigment
      if pigment is None:
        self._texture.pigment = NULL
      else:
        real_pigment = Pigment(pigment)
        self._texture.pigment = real_pigment._pigment
        DMNSN_INCREF(self._texture.pigment)

  property finish:
    """The texture's finish."""
    def __get__(self):
      return _Finish(self._texture.finish)
    def __set__(self, Finish finish not None):
      dmnsn_delete_finish(self._texture.finish)
      self._texture.finish = finish._finish
      dmnsn_finish_incref(&self._texture.finish)

  def transform(self, Matrix trans not None):
    """Transform a texture."""
    self._texture.trans = dmnsn_matrix_mul(trans._m, self._texture.trans)
    return self

cdef Texture _Texture(dmnsn_texture *texture):
  """Wrap a Texture object around a dmnsn_texture *."""
  cdef Texture self = Texture.__new__(Texture)
  self._texture = texture
  DMNSN_INCREF(self._texture)
  return self

#############
# Interiors #
#############

cdef class Interior:
  """Object interior properties."""
  cdef dmnsn_interior *_interior

  def __init__(self, double ior = 1.0):
    """
    Create an Interior.

    Keyword arguments:
    ior -- index of reflection
    """
    self._interior = dmnsn_new_interior()
    self._interior.ior = ior

  def __dealloc__(self):
    dmnsn_delete_interior(self._interior)

  property ior:
    """Index of reflection."""
    def __get__(self):
      return self._interior.ior
    def __set__(self, double ior):
      self._interior.ior = ior

cdef Interior _Interior(dmnsn_interior *interior):
  """Wrap an Interior object around a dmnsn_interior *."""
  cdef Interior self = Interior.__new__(Interior)
  self._interior = interior
  DMNSN_INCREF(self._interior)
  return self

###########
# Objects #
###########

cdef class Object(_Transformable):
  """Physical objects."""
  cdef dmnsn_object *_object

  def __cinit__(self):
    self._object = NULL

  def __init__(self, Texture texture = None, pigment = None,
               Finish finish = None, Interior interior = None):
    """
    Initialize an Object.

    Keyword arguments:
    texture  -- the object's Texture
    pigment  -- shorthand for specifying the texture's pigment
    finish   -- shorthand for specifying the texture's finish
    interior -- the object's Interior
    """
    if self._object == NULL:
      raise TypeError("attempt to initialize base Object")

    self.texture = texture
    if pigment is not None:
      if texture is not None:
        raise TypeError("both texture and pigment specified.")
      else:
        if self.texture is None:
          self.texture = Texture()
        self.texture.pigment = pigment

    if finish is not None:
      if texture is not None:
        raise TypeError("both texture and finish specified.")
      else:
        if self.texture is None:
          self.texture = Texture()
        self.texture.finish = finish

    if interior is not None:
      self.interior = interior

  def __dealloc__(self):
    dmnsn_delete_object(self._object)

  property texture:
    """The object's Texture."""
    def __get__(self):
      if self._object.texture == NULL:
        return None
      else:
        return _Texture(self._object.texture)
    def __set__(self, Texture texture):
      dmnsn_delete_texture(self._object.texture)
      if texture is None:
        self._object.texture = NULL
      else:
        self._object.texture = texture._texture
        DMNSN_INCREF(self._object.texture)

  property interior:
    """The object's Interior."""
    def __get__(self):
      return _Interior(self._object.interior)
    def __set__(self, Interior interior not None):
      self._object.interior = interior._interior
      DMNSN_INCREF(self._object.interior)

  def transform(self, Matrix trans not None):
    """Transform an object."""
    self._object.trans = dmnsn_matrix_mul(trans._m, self._object.trans)
    return self

  # Transform an object without affecting the texture
  cdef _intrinsic_transform(self, Matrix trans):
    self._object.intrinsic_trans = dmnsn_matrix_mul(
      trans._m,
      self._object.intrinsic_trans
    )

cdef class Triangle(Object):
  """A triangle."""
  def __init__(self, a, b, c, a_normal = None, b_normal = None, c_normal = None,
               *args, **kwargs):
    """
    Create a Triangle.

    Keyword arguments:
    a, b, c                      -- the corners of the triangle
    a_normal, b_normal, c_normal -- the optional normal vectors at those corners

    Additionally, Triangle() accepts any arguments that Object() accepts.
    """
    if a_normal is None and b_normal is None and c_normal is None:
      a_normal = cross(b - a, c - a)
      b_normal = a_normal
      c_normal = a_normal
    self._object = dmnsn_new_triangle(Vector(a)._v, Vector(b)._v, Vector(c)._v,
                                      Vector(a_normal)._v,
                                      Vector(b_normal)._v,
                                      Vector(c_normal)._v)
    Object.__init__(self, *args, **kwargs)

cdef class Plane(Object):
  """A plane."""
  def __init__(self, normal, double distance, *args, **kwargs):
    """
    Create a Plane.

    Keyword arguments:
    normal   -- a vector perpendicular to the plane
    distance -- the distance from the origin to the plane, in the direction of
                normal

    Additionally, Plane() accepts any arguments that Object() accepts.
    """
    self._object = dmnsn_new_plane(Vector(normal)._v)
    Object.__init__(self, *args, **kwargs)

    self._intrinsic_transform(translate(distance*Vector(normal)))

cdef class Sphere(Object):
  """A sphere."""
  def __init__(self, center, double radius, *args, **kwargs):
    """
    Create a Sphere.

    Keyword arguments:
    center -- the center of the sphere
    radius -- the radius of the sphere

    Additionally, Sphere() accepts any arguments that Object() accepts.
    """
    self._object = dmnsn_new_sphere()
    Object.__init__(self, *args, **kwargs)

    cdef Matrix trans = translate(Vector(center))
    trans *= scale(radius, radius, radius)
    self._intrinsic_transform(trans)

cdef class Box(Object):
  """An axis-aligned rectangular prism."""
  def __init__(self, min, max, *args, **kwargs):
    """
    Create a Box.

    Keyword arguments:
    min -- the coordinate-wise minimal extent of the box
    max -- the coordinate-wise maximal extent of the box

    Additionally, Box() accepts any arguments that Object() accepts.
    """
    self._object = dmnsn_new_cube()
    Object.__init__(self, *args, **kwargs)

    min = Vector(min)
    max = Vector(max)
    cdef Matrix trans = translate((max + min)/2)
    trans *= scale((max - min)/2)
    self._intrinsic_transform(trans)

cdef class Cone(Object):
  """A cone or cone slice."""
  def __init__(self, bottom, double bottom_radius, top, double top_radius = 0.0,
               bool open not None = False, *args, **kwargs):
    """
    Create a Cone.

    Keyword arguments:
    bottom        -- the location of the bottom of the cone
    bottom_radius -- the radius at the bottom of the cone
    top           -- the location of the top of the cone
    top_radius    -- the radius at the top of the cone/cone slice (default 0.0)
    open          -- whether to draw the cone cap(s)

    Additionally, Cone() accepts any arguments that Object() accepts.
    """
    self._object = dmnsn_new_cone(bottom_radius, top_radius, open)
    Object.__init__(self, *args, **kwargs)

    # Lift the cone to start at the origin, then scale, rotate, and translate
    # properly

    cdef Vector dir = Vector(top) - Vector(bottom)

    cdef Matrix trans = translate(Y)
    trans = scale(1.0, dir.norm()/2, 1.0)*trans
    trans = _Matrix(dmnsn_alignment_matrix(dmnsn_y, dir._v, dmnsn_x, dmnsn_z))*trans
    trans = translate(bottom)*trans

    self._intrinsic_transform(trans)

cdef class Cylinder(Cone):
  """A cylinder."""
  def __init__(self, bottom, top, double radius, bool open not None = False,
               *args, **kwargs):
    """
    Create a Cylinder.

    Keyword arguments:
    bottom  -- the location of the bottom of the cylinder
    top     -- the location of the top of the cylinder
    radius  -- the radius of the cylinder
    open    -- whether to draw the cylinder caps

    Additionally, Cylinder() accepts any arguments that Object() accepts.
    """
    Cone.__init__(self,
                  bottom = bottom, bottom_radius = radius,
                  top    = top,    top_radius    = radius,
                  open = open,
                  *args, **kwargs)

cdef class Torus(Object):
  """A torus."""
  def __init__(self, double major_radius, double minor_radius, *args, **kwargs):
    """
    Create a Torus.

    Keyword arguments:
    major_radius -- the distance from the center of the torus to the center of
                    a circular cross-section of the torus
    minor_radius -- the radius of the circular cross-sections of the torus

    Additionally, Torus() accepts any arguments that Object() accepts.
    """
    self._object = dmnsn_new_torus(major_radius, minor_radius)
    Object.__init__(self, *args, **kwargs)

cdef class Union(Object):
  """A CSG union."""
  def __init__(self, objects, *args, **kwargs):
    """
    Create a Union.

    Keyword arguments:
    objects -- a list of objects to include in the union

    Additionally, Union() accepts any arguments that Object() accepts.
    """
    if len(objects) < 1:
      raise TypeError("expected a list of one or more Objects")

    cdef dmnsn_array *array = dmnsn_new_array(sizeof(dmnsn_object *))
    cdef dmnsn_object *o

    try:
      for obj in objects:
        o = (<Object?>obj)._object
        DMNSN_INCREF(o)
        dmnsn_array_push(array, &o)

      self._object = dmnsn_new_csg_union(array)
    finally:
      dmnsn_delete_array(array)

    Object.__init__(self, *args, **kwargs)

cdef class Intersection(Object):
  """A CSG intersection."""
  def __init__(self, objects, *args, **kwargs):
    """
    Create an Intersection.

    Keyword arguments:
    objects -- a list of objects to include in the intersection

    Additionally, Intersection() accepts any arguments that Object() accepts.
    """
    if len(objects) < 1:
      raise TypeError("expected a list of one or more Objects")

    cdef dmnsn_object *o

    for obj in objects:
      if self._object == NULL:
        self._object = (<Object?>obj)._object
        DMNSN_INCREF(self._object)
      else:
        o = (<Object?>obj)._object
        DMNSN_INCREF(o)
        self._object = dmnsn_new_csg_intersection(self._object, o)

    Object.__init__(self, *args, **kwargs)

cdef class Difference(Object):
  """A CSG difference."""
  def __init__(self, objects, *args, **kwargs):
    """
    Create a Difference.

    Keyword arguments:
    objects -- a list of objects to include in the difference

    Additionally, Difference() accepts any arguments that Object() accepts.
    """
    if len(objects) < 1:
      raise TypeError("expected a list of one or more Objects")

    cdef dmnsn_object *o

    for obj in objects:
      if self._object == NULL:
        self._object = (<Object?>obj)._object
        DMNSN_INCREF(self._object)
      else:
        o = (<Object?>obj)._object
        DMNSN_INCREF(o)
        self._object = dmnsn_new_csg_difference(self._object, o)

    Object.__init__(self, *args, **kwargs)

cdef class Merge(Object):
  """A CSG merge."""
  def __init__(self, objects, *args, **kwargs):
    """
    Create a Merge.

    Keyword arguments:
    objects -- a list of objects to include in the merge

    Additionally, Merge() accepts any arguments that Object() accepts.
    """
    if len(objects) < 1:
      raise TypeError("expected a list of one or more Objects")

    cdef dmnsn_object *o

    for obj in objects:
      if self._object == NULL:
        self._object = (<Object?>obj)._object
        DMNSN_INCREF(self._object)
      else:
        o = (<Object?>obj)._object
        DMNSN_INCREF(o)
        self._object = dmnsn_new_csg_merge(self._object, o)

    Object.__init__(self, *args, **kwargs)

##########
# Lights #
##########

cdef class Light:
  """A light."""
  cdef dmnsn_light *_light

  def __dealloc__(self):
    dmnsn_delete_light(self._light)

cdef class PointLight(Light):
  """A point light."""
  def __init__(self, location, color):
    """
    Create a PointLight.

    Keyword arguments:
    location -- the origin of the light rays
    color    -- the color and intensity of the light
    """
    self._light = dmnsn_new_point_light(Vector(location)._v, Color(color)._c)
    Light.__init__(self)

###########
# Cameras #
###########

cdef class Camera(_Transformable):
  """A camera."""
  cdef dmnsn_camera *_camera

  def __cinit__(self):
    self._camera = NULL

  def __init__(self):
    if self._camera == NULL:
      raise TypeError("attempt to initialize base Camera")

  def __dealloc__(self):
    dmnsn_delete_camera(self._camera)

  def transform(self, Matrix trans not None):
    """Transform a camera."""
    self._camera.trans = dmnsn_matrix_mul(trans._m, self._camera.trans)
    return self

cdef class PerspectiveCamera(Camera):
  """A regular perspective camera."""
  def __init__(self, location = -Z, look_at = 0, sky = Y,
               angle = dmnsn_degrees(atan(1.0))):
    """
    Create a PerspectiveCamera.

    Keyword arguments:
    location -- the location of the camera (default: -Z)
    look_at  -- where to aim the camera (default: 0)
    sky      -- the direction of the top of the camera (default: Y)
    angle    -- the field of view angle (from bottom to top) (default: 45)
    """
    self._camera = dmnsn_new_perspective_camera()
    Camera.__init__(self)

    # Apply the field of view angle
    self.scale(tan(dmnsn_radians(angle))*(X + Y) + Z)

    cdef Vector dir = Vector(look_at) - Vector(location)
    cdef Vector vsky = Vector(sky)

    # Line up the top of the viewport with the sky vector
    cdef Matrix align_sky = _Matrix(dmnsn_alignment_matrix(dmnsn_y, vsky._v,
                                                           dmnsn_z, dmnsn_x))
    cdef Vector forward = align_sky*Z
    cdef Vector right   = align_sky*X

    # Line up the look at point with look_at
    self.transform(_Matrix(dmnsn_alignment_matrix(forward._v, dir._v,
                                                  vsky._v, right._v)))

    # Move the camera into position
    self.translate(Vector(location))

##########
# Scenes #
##########

cdef class Scene:
  """An entire scene."""
  cdef dmnsn_scene *_scene

  def __init__(self, Canvas canvas not None, objects, lights,
               Camera camera not None):
    """
    Create a Scene.

    Keyword arguments:
    canvas  -- the rendering Canvas
    objects -- the list of objects in the scene
    lights  -- the list of lights in the scene
    camera  -- the camera for the scene
    """
    self._scene = dmnsn_new_scene(_get_pool())

    self._scene.canvas = canvas._canvas
    DMNSN_INCREF(self._scene.canvas)
    self.outer_width = self._scene.canvas.width
    self.outer_height = self._scene.canvas.height
    self.background = Black

    cdef dmnsn_object *o
    for obj in objects:
      o = (<Object?>obj)._object
      DMNSN_INCREF(o)
      dmnsn_array_push(self._scene.objects, &o)

    cdef dmnsn_light *l
    for light in lights:
      l = (<Light?>light)._light
      DMNSN_INCREF(l)
      dmnsn_array_push(self._scene.lights, &l)

    self._scene.camera = camera._camera
    DMNSN_INCREF(self._scene.camera)

  # Subregion render support
  property region_x:
    """The x-coordinate of the subregion in the broader image."""
    def __get__(self):
      return self._scene.region_x
    def __set__(self, x):
      self._scene.region_x = x
  property region_y:
    """The y-coordinate of the subregion in the broader image."""
    def __get__(self):
      return self._scene.region_y
    def __set__(self, y):
      self._scene.region_y = y
  property outer_width:
    """The width of the broader image."""
    def __get__(self):
      return self._scene.outer_width
    def __set__(self, width):
      self._scene.outer_width = width
  property outer_height:
    """The height of the broader image."""
    def __get__(self):
      return self._scene.outer_height
    def __set__(self, height):
      self._scene.outer_height = height

  property default_texture:
    """The default Texture for objects."""
    def __get__(self):
      return _Texture(self._scene.default_texture)
    def __set__(self, Texture texture not None):
      dmnsn_delete_texture(self._scene.default_texture)
      self._scene.default_texture = texture._texture
      DMNSN_INCREF(self._scene.default_texture)
  property default_interior:
    """The default Interior for objects."""
    def __get__(self):
      return _Interior(self._scene.default_interior)
    def __set__(self, Interior interior not None):
      dmnsn_delete_interior(self._scene.default_interior)
      self._scene.default_interior = interior._interior
      DMNSN_INCREF(self._scene.default_interior)

  property background:
    """The background pigment of the scene (default: Black)."""
    def __get__(self):
      return _Pigment(self._scene.background)
    def __set__(self, pigment):
      dmnsn_delete_pigment(self._scene.background)
      cdef Pigment real_pigment = Pigment(pigment)
      self._scene.background = real_pigment._pigment
      DMNSN_INCREF(self._scene.background)

  property adc_bailout:
    """The adaptive depth control bailout (default: 1/255)."""
    def __get__(self):
      return self._scene.adc_bailout
    def __set__(self, double bailout):
      self._scene.adc_bailout = bailout

  property recursion_limit:
    """The rendering recursion limit (default: 5)."""
    def __get__(self):
      return self._scene.reclimit
    def __set__(self, level):
      self._scene.reclimit = level

  property nthreads:
    """The number of threads to use for the render."""
    def __get__(self):
      return self._scene.nthreads
    def __set__(self, n):
      if n <= 0:
        raise ValueError("%d is an invalid thread count." % n)
      self._scene.nthreads = n

  property quality:
    """The render quality."""
    def __get__(self):
      return _quality_to_string(self._scene.quality)
    def __set__(self, q):
      self._scene.quality = _string_to_quality(q)

  property bounding_timer:
    """The Timer for building the bounding hierarchy."""
    def __get__(self):
      return _Timer(self._scene.bounding_timer)
  property render_timer:
    """The Timer for the actual render."""
    def __get__(self):
      return _Timer(self._scene.render_timer)

  def ray_trace(self):
    """Render the scene."""
    self.ray_trace_async().join()
  def ray_trace_async(self):
    """Render the scene, in the background."""
    # Account for image dimensions in the camera
    # Do this here so subregion renders can tell us the broader image size
    self._scene.camera.trans = dmnsn_matrix_mul(
      self._scene.camera.trans,
      dmnsn_scale_matrix(
        dmnsn_new_vector(
          self.outer_width/self.outer_height,
          1.0,
          1.0
        )
      )
    )
    # Ensure the default texture is complete
    cdef Texture default = Texture(pigment = Black)
    dmnsn_texture_cascade(default._texture, &self._scene.default_texture)
    return _Future(dmnsn_ray_trace_async(self._scene))

  def __dealloc__(self):
    dmnsn_delete_scene(self._scene)

def _quality_to_string(int quality):
  cdef str s = ""

  if quality & DMNSN_RENDER_PIGMENT:
    s += 'p'
  if quality & DMNSN_RENDER_LIGHTS:
    s += 'l'
  if quality & DMNSN_RENDER_FINISH:
    s += 'f'
  if quality & DMNSN_RENDER_TRANSPARENCY:
    s += 't'
  if quality & DMNSN_RENDER_REFLECTION:
    s += 'r'

  if s == "":
    return "0"
  else:
    return s

def _string_to_quality(str quality not None):
  cdef int q = DMNSN_RENDER_NONE
  inverse = False

  if quality == "":
    return q

  if quality[0] == '^':
    inverse = True
    quality = quality[1:]

  if quality != "0":
    while len(quality) > 0:
      ch = quality[0]
      quality = quality[1:]

      if ch == 'p':
        flag = DMNSN_RENDER_PIGMENT
      elif ch == 'l':
        flag = DMNSN_RENDER_LIGHTS
      elif ch == 'f':
        flag = DMNSN_RENDER_FINISH
      elif ch == 't':
        flag = DMNSN_RENDER_TRANSPARENCY
      elif ch == 'r':
        flag = DMNSN_RENDER_REFLECTION
      else:
        raise ValueError("unknown quality flag '%c'" % ch)

      if q & flag:
        raise ValueError("flag '%c' specified twice" % ch)
      else:
        q |= flag

  if inverse:
    q = ~q

  return q
