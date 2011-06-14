#########################################################################
# Copyright (C) 2011 Tavian Barnes <tavianator@tavianator.com>          #
#                                                                       #
# This file is part of The Dimension Python Module.                     #
#                                                                       #
# The Dimension Python Module is free software; you can redistribute it #
# and/or modify it under the terms of the GNU General Public License as #
# published by the Free Software Foundation; either version 3 of the    #
# License, or (at your option) any later version.                       #
#                                                                       #
# The Dimension Python Module is distributed in the hope that it will   #
# be useful, but WITHOUT ANY WARRANTY; without even the implied         #
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See #
# the GNU General Public License for more details.                      #
#                                                                       #
# You should have received a copy of the GNU General Public License     #
# along with this program.  If not, see <http://www.gnu.org/licenses/>. #
#########################################################################

import os

###########
# Globals #
###########

# Make warnings fatal
def dieOnWarnings(alwaysDie):
  dmnsn_die_on_warnings(alwaysDie)

############
# Geometry #
############

cdef class Vector:
  cdef dmnsn_vector _v

  def __init__(self, *args, **kwargs):
    if len(args) == 1:
      if (isinstance(args[0], Vector)):
        self._v = (<Vector>args[0])._v
      elif (isinstance(args[0], tuple)):
        self._realInit(*args[0])
      elif (args[0] == 0):
        self._v = dmnsn_zero
      else:
        raise TypeError, 'expected a tuple or 0'
    else:
      self._realInit(*args, **kwargs)

  def _realInit(self, double x, double y, double z):
    self._v = dmnsn_new_vector(x, y, z)

  property x:
    def __get__(self):
      return self._v.x
  property y:
    def __get__(self):
      return self._v.y
  property z:
    def __get__(self):
      return self._v.z

  def __pos__(self):
    return self
  def __neg__(self):
    return _rawVector(dmnsn_vector_negate(self._v))
  def __nonzero__(self):
    return dmnsn_vector_norm(self._v) >= dmnsn_epsilon

  def __add__(lhs, rhs):
    return _rawVector(dmnsn_vector_add(Vector(lhs)._v, Vector(rhs)._v))
  def __sub__(lhs, rhs):
    return _rawVector(dmnsn_vector_sub(Vector(lhs)._v, Vector(rhs)._v))
  def __mul__(lhs, rhs):
    if (isinstance(lhs, Vector)):
      return _rawVector(dmnsn_vector_mul(rhs, (<Vector>lhs)._v))
    else:
      return _rawVector(dmnsn_vector_mul(lhs, (<Vector>rhs)._v))
  def __truediv__(Vector lhs not None, double rhs):
    return _rawVector(dmnsn_vector_div(lhs._v, rhs))

  def __richcmp__(lhs, rhs, int op):
    equal = (Vector(lhs) - Vector(rhs)).norm() < dmnsn_epsilon
    if (op == 2):   # ==
      return equal
    elif (op == 3): # !=
      return not equal
    else:
      return NotImplemented

  def norm(self):
    return dmnsn_vector_norm(self._v)
  def normalized(self):
    return _rawVector(dmnsn_vector_normalized(self._v))

  def __repr__(self):
    return 'dimension.Vector(%r, %r, %r)' % (self.x, self.y, self.z)

  def __str__(self):
    return '<%s, %s, %s>' % (self.x, self.y, self.z)

cdef _rawVector(dmnsn_vector v):
  cdef Vector self = Vector.__new__(Vector)
  self._v = v
  return self

def cross(Vector lhs not None, Vector rhs not None):
  return _rawVector(dmnsn_vector_cross(lhs._v, rhs._v))
def dot(Vector lhs not None, Vector rhs not None):
  return dmnsn_vector_dot(lhs._v, rhs._v)
def proj(Vector u not None, Vector d not None):
  return _rawVector(dmnsn_vector_proj(u._v, d._v))

X = _rawVector(dmnsn_x)
Y = _rawVector(dmnsn_y)
Z = _rawVector(dmnsn_z)

cdef class Matrix:
  cdef dmnsn_matrix _m

  def __init__(self,
               double a1, double a2, double a3, double a4,
               double b1, double b2, double b3, double b4,
               double c1, double c2, double c3, double c4):
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
    if (isinstance(rhs, Matrix)):
      return _rawMatrix(dmnsn_matrix_mul(lhs._m, (<Matrix>rhs)._m))
    else:
      return _rawVector(dmnsn_transform_vector(lhs._m, (<Vector>rhs)._v))

  def __richcmp__(Matrix lhs not None, Matrix rhs not None, int op):
    cdef double sum = 0.0
    for i in range(3):
      for j in range(4):
        diff = lhs._m.n[i][j] - rhs._m.n[i][j]
        sum += diff*diff
    equal = sqrt(sum) < dmnsn_epsilon

    if (op == 2):   # ==
      return equal
    elif (op == 3): # !=
      return not equal
    else:
      return NotImplemented

  cpdef Matrix inverse(self):
    return _rawMatrix(dmnsn_matrix_inverse(self._m));

  def __repr__(self):
    return \
      'dimension.Matrix(%r, %r, %r, %r, %r, %r, %r, %r, %r, %r, %r, %r)' % \
      (self._m.n[0][0], self._m.n[0][1], self._m.n[0][2], self._m.n[0][3],
       self._m.n[1][0], self._m.n[1][1], self._m.n[1][2], self._m.n[1][3],
       self._m.n[2][0], self._m.n[2][1], self._m.n[2][2], self._m.n[2][3])

  def __str__(self):
    return \
      '\n[%s\t%s\t%s\t%s]' \
      '\n[%s\t%s\t%s\t%s]' \
      '\n[%s\t%s\t%s\t%s]' \
      '\n[%s\t%s\t%s\t%s]' %\
      (self._m.n[0][0], self._m.n[0][1], self._m.n[0][2], self._m.n[0][3],
       self._m.n[1][0], self._m.n[1][1], self._m.n[1][2], self._m.n[1][3],
       self._m.n[2][0], self._m.n[2][1], self._m.n[2][2], self._m.n[2][3],
       0.0, 0.0, 0.0, 1.0)

cdef Matrix _rawMatrix(dmnsn_matrix m):
  cdef Matrix self = Matrix.__new__(Matrix)
  self._m = m
  return self

def scale(*args, **kwargs):
  return _rawMatrix(dmnsn_scale_matrix(Vector(*args, **kwargs)._v))
def translate(*args, **kwargs):
  return _rawMatrix(dmnsn_translation_matrix(Vector(*args, **kwargs)._v))
def rotate(*args, **kwargs):
  cdef Vector rad = dmnsn_radians(1.0)*Vector(*args, **kwargs)
  return _rawMatrix(dmnsn_rotation_matrix(rad._v))
def _rawRotate(*args, **kwargs):
  return _rawMatrix(dmnsn_rotation_matrix(Vector(*args, **kwargs)._v))

##########
# Colors #
##########

cdef class Color:
  cdef dmnsn_color _c
  cdef dmnsn_color _sRGB

  def __init__(self, *args, **kwargs):
    if len(args) == 1:
      if (isinstance(args[0], Color)):
        self._sRGB = (<Color>args[0])._sRGB
      elif (isinstance(args[0], tuple)):
        self._realInit(*args[0])
      else:
        self._sRGB = dmnsn_color_mul(args[0], dmnsn_white)
    else:
      self._realInit(*args, **kwargs)

    self._c = dmnsn_color_from_sRGB(self._sRGB)

  def _realInit(self, double red, double green, double blue,
                double trans = 0.0, double filter = 0.0):
    self._sRGB = dmnsn_new_color5(red, green, blue, trans, filter)

  property red:
    def __get__(self):
      return self._sRGB.R
  property green:
    def __get__(self):
      return self._sRGB.G
  property blue:
    def __get__(self):
      return self._sRGB.B
  property trans:
    def __get__(self):
      return self._sRGB.trans
  property filter:
    def __get__(self):
      return self._sRGB.filter

  def __nonzero__(self):
    return not dmnsn_color_is_black(self._c)

  def __add__(lhs, rhs):
    return _rawsRGBColor(dmnsn_color_add(Color(lhs)._sRGB, Color(rhs)._sRGB))
  def __mul__(lhs, rhs):
    if (isinstance(lhs, Color)):
      return _rawsRGBColor(dmnsn_color_mul(rhs, (<Color>lhs)._sRGB))
    else:
      return _rawsRGBColor(dmnsn_color_mul(lhs, (<Color>rhs)._sRGB))

  def __richcmp__(lhs, rhs, int op):
    cdef clhs = Color(lhs)
    cdef crhs = Color(rhs)

    cdef double rdiff = clhs.red    - crhs.red
    cdef double gdiff = clhs.green  - crhs.green
    cdef double bdiff = clhs.blue   - crhs.blue
    cdef double tdiff = clhs.trans  - crhs.trans
    cdef double fdiff = clhs.filter - crhs.filter
    cdef double sum = rdiff*rdiff + gdiff*gdiff + bdiff*bdiff \
                      + tdiff*tdiff + fdiff*fdiff
    equal = sqrt(sum) < dmnsn_epsilon
    if (op == 2):   # ==
      return equal
    elif (op == 3): # !=
      return not equal
    else:
      return NotImplemented

  def __repr__(self):
    return 'dimension.Color(%r, %r, %r, %r, %r)' % \
           (self.red, self.green, self.blue, self.trans, self.filter)

  def __str__(self):
    if (self.trans >= dmnsn_epsilon):
      return '<red = %s, green = %s, blue = %s, trans = %s, filter = %s>' % \
             (self.red, self.green, self.blue, self.trans, self.filter)
    else:
      return '<red = %s, green = %s, blue = %s>' % \
             (self.red, self.green, self.blue)

cdef _rawsRGBColor(dmnsn_color sRGB):
  cdef Color self = Color.__new__(Color)
  self._sRGB = sRGB
  self._c    = dmnsn_color_from_sRGB(sRGB)
  return self

cdef _rawColor(dmnsn_color c):
  cdef Color self = Color.__new__(Color)
  self._c    = c
  self._sRGB = dmnsn_color_to_sRGB(c)
  return self

Black   = _rawColor(dmnsn_black)
White   = _rawColor(dmnsn_white)
Clear   = _rawColor(dmnsn_clear)
Red     = _rawColor(dmnsn_red)
Green   = _rawColor(dmnsn_green)
Blue    = _rawColor(dmnsn_blue)
Magenta = _rawColor(dmnsn_magenta)
Orange  = _rawColor(dmnsn_orange)
Yellow  = _rawColor(dmnsn_yellow)
Cyan    = _rawColor(dmnsn_cyan)

############
# Canvases #
############

cdef class Canvas:
  cdef dmnsn_canvas *_canvas

  def __cinit__(self, size_t width, size_t height):
    self._canvas = dmnsn_new_canvas(width, height)

  def __dealloc__(self):
    dmnsn_delete_canvas(self._canvas)

  property width:
    def __get__(self):
      return self._canvas.width
  property height:
    def __get__(self):
      return self._canvas.height

  def optimizePNG(self):
    if dmnsn_png_optimize_canvas(self._canvas) != 0:
      raise OSError(errno, os.strerror(errno))

  def optimizeGL(self):
    if dmnsn_gl_optimize_canvas(self._canvas) != 0:
      raise OSError(errno, os.strerror(errno))

  def clear(self, c):
    dmnsn_clear_canvas(self._canvas, Color(c)._c)

  def writePNG(self, str path not None):
    bpath = path.encode('UTF-8')
    cdef char *cpath = bpath
    cdef FILE *file = fopen(cpath, "wb")
    if file == NULL:
      raise OSError(errno, os.strerror(errno))

    if dmnsn_png_write_canvas(self._canvas, file) != 0:
      raise OSError(errno, os.strerror(errno))

  def drawGL(self):
    if dmnsn_gl_write_canvas(self._canvas) != 0:
      raise OSError(errno, os.strerror(errno))

############
# Patterns #
############

cdef class Pattern:
  cdef dmnsn_pattern *_pattern

  def __cinit__(self):
    self._pattern = NULL

  def __dealloc__(self):
    dmnsn_delete_pattern(self._pattern)

  def transform(self, Matrix trans not None):
    if self._pattern == NULL:
      raise TypeError('attempt to transform base Pattern')

    self._pattern.trans = dmnsn_matrix_mul(trans._m, self._pattern.trans)
    return self

cdef class Checker(Pattern):
  def __init__(self):
    self._pattern = dmnsn_new_checker_pattern()
    Pattern.__init__(self)

cdef class Gradient(Pattern):
  def __init__(self, orientation):
    self._pattern = dmnsn_new_gradient_pattern(Vector(orientation)._v)
    Pattern.__init__(self)

############
# Pigments #
############

cdef class Pigment:
  cdef dmnsn_pigment *_pigment

  def __cinit__(self):
    self._pigment = NULL

  def __init__(self, arg = None):
    if arg is not None:
      if (isinstance(arg, Pigment)):
        self._pigment = (<Pigment>arg)._pigment
        DMNSN_INCREF(self._pigment)
      else:
        self._pigment = dmnsn_new_solid_pigment(Color(arg)._c)

  def __dealloc__(self):
    dmnsn_delete_pigment(self._pigment)

  def transform(self, Matrix trans not None):
    if self._pigment == NULL:
      raise TypeError('attempt to transform base Pigment')

    self._pigment.trans = dmnsn_matrix_mul(trans._m, self._pigment.trans)
    return self

cdef class ColorMap(Pigment):
  def __init__(self, Pattern pattern not None, map, bool sRGB not None = True):
    cdef dmnsn_map *color_map = dmnsn_new_color_map()
    if hasattr(map, 'items'):
      for i, color in map.items():
        dmnsn_add_map_entry(color_map, i, &Color(color)._c)
    else:
      for i, color in enumerate(map):
        dmnsn_add_map_entry(color_map, i/len(map), &Color(color)._c)

    cdef dmnsn_pigment_map_flags flags
    if sRGB:
      flags = DMNSN_PIGMENT_MAP_SRGB
    else:
      flags = DMNSN_PIGMENT_MAP_REGULAR

    DMNSN_INCREF(pattern._pattern)
    self._pigment = dmnsn_new_color_map_pigment(pattern._pattern, color_map,
                                                flags)
    Pigment.__init__(self)

cdef class PigmentMap(Pigment):
  def __init__(self, Pattern pattern not None, map, bool sRGB not None = True):
    cdef dmnsn_map *pigment_map = dmnsn_new_pigment_map()
    cdef dmnsn_pigment *realPigment
    if hasattr(map, 'items'):
      for i, pigment in map.items():
        pigment = Pigment(pigment)
        realPigment = (<Pigment>pigment)._pigment
        DMNSN_INCREF(realPigment)
        dmnsn_add_map_entry(pigment_map, i, &realPigment)
    else:
      for i, pigment in enumerate(map):
        pigment = Pigment(pigment)
        realPigment = (<Pigment>pigment)._pigment
        DMNSN_INCREF(realPigment)
        dmnsn_add_map_entry(pigment_map, i/len(map), &realPigment)

    cdef dmnsn_pigment_map_flags flags
    if sRGB:
      flags = DMNSN_PIGMENT_MAP_SRGB
    else:
      flags = DMNSN_PIGMENT_MAP_REGULAR

    DMNSN_INCREF(pattern._pattern)
    self._pigment = dmnsn_new_pigment_map_pigment(pattern._pattern, pigment_map,
                                                  flags)
    Pigment.__init__(self)

############
# Finishes #
############

cdef class Finish:
  cdef dmnsn_finish _finish

  def __cinit__(self):
    self._finish = dmnsn_new_finish()

  def __dealloc__(self):
    dmnsn_delete_finish(self._finish)

  def __add__(Finish lhs not None, Finish rhs not None):
    cdef Finish ret = Finish()

    if lhs._finish.ambient != NULL and rhs._finish.ambient != NULL:
      raise ValueError('both Finishes provide an ambient contribution')
    elif lhs._finish.ambient != NULL:
      ret._finish.ambient = lhs._finish.ambient
      DMNSN_INCREF(ret._finish.ambient)
    elif rhs._finish.ambient != NULL:
      ret._finish.ambient = rhs._finish.ambient
      DMNSN_INCREF(ret._finish.ambient)

    if lhs._finish.diffuse != NULL and rhs._finish.diffuse != NULL:
      raise ValueError('both Finishes provide a diffuse contribution')
    elif lhs._finish.diffuse != NULL:
      ret._finish.diffuse = lhs._finish.diffuse
      DMNSN_INCREF(ret._finish.diffuse)
    elif rhs._finish.diffuse != NULL:
      ret._finish.diffuse = rhs._finish.diffuse
      DMNSN_INCREF(ret._finish.diffuse)

    if lhs._finish.specular != NULL and rhs._finish.specular != NULL:
      raise ValueError('both Finishes provide a specular contribution')
    elif lhs._finish.specular != NULL:
      ret._finish.specular = lhs._finish.specular
      DMNSN_INCREF(ret._finish.specular)
    elif rhs._finish.specular != NULL:
      ret._finish.specular = rhs._finish.specular
      DMNSN_INCREF(ret._finish.specular)

    if lhs._finish.reflection != NULL and rhs._finish.reflection != NULL:
      raise ValueError('both Finishes provide a reflection contribution')
    elif lhs._finish.reflection != NULL:
      ret._finish.reflection = lhs._finish.reflection
      DMNSN_INCREF(ret._finish.reflection)
    elif rhs._finish.reflection != NULL:
      ret._finish.reflection = rhs._finish.reflection
      DMNSN_INCREF(ret._finish.reflection)

    return ret

cdef class Ambient(Finish):
  def __init__(self, color):
    self._finish.ambient = dmnsn_new_basic_ambient(Color(color)._c)

cdef class Diffuse(Finish):
  def __init__(self, double diffuse):
    cdef dmnsn_color gray = dmnsn_color_mul(diffuse, dmnsn_white)
    diffuse = dmnsn_color_intensity(dmnsn_color_from_sRGB(gray))
    self._finish.diffuse = dmnsn_new_lambertian(diffuse)

cdef class Phong(Finish):
  def __init__(self, double strength, double size = 40.0):
    self._finish.specular = dmnsn_new_phong(strength, size)

cdef class Reflection(Finish):
  def __init__(self, min, max = None, double falloff = 1.0):
    if max is None:
      max = min
    self._finish.reflection = dmnsn_new_basic_reflection(Color(min)._c,
                                                         Color(max)._c,
                                                         falloff)

############
# Textures #
############

cdef class Texture:
  cdef dmnsn_texture *_texture

  def __init__(self, pigment = None, Finish finish = None):
    self._texture = dmnsn_new_texture()

    cdef Pigment realPigment
    if pigment is not None:
      realPigment = Pigment(pigment)
      self._texture.pigment = realPigment._pigment
      DMNSN_INCREF(self._texture.pigment)

    if finish is not None:
      self._texture.finish = finish._finish
      if self._texture.finish.ambient != NULL:
        DMNSN_INCREF(self._texture.finish.ambient)
      if self._texture.finish.diffuse != NULL:
        DMNSN_INCREF(self._texture.finish.diffuse)
      if self._texture.finish.specular != NULL:
        DMNSN_INCREF(self._texture.finish.specular)
      if self._texture.finish.reflection != NULL:
        DMNSN_INCREF(self._texture.finish.reflection)

  def __dealloc__(self):
    dmnsn_delete_texture(self._texture)

#############
# Interiors #
#############

cdef class Interior:
  cdef dmnsn_interior *_interior

  def __init__(self, double ior = 1.0):
    self._interior = dmnsn_new_interior()
    self._interior.ior = ior

  def __dealloc__(self):
    dmnsn_delete_interior(self._interior)

###########
# Objects #
###########

cdef class Object:
  cdef dmnsn_object *_object

  def __cinit__(self):
    self._object = NULL

  def __init__(self, Texture texture = None, Interior interior = None):
    if texture is not None:
      self._object.texture = texture._texture
      DMNSN_INCREF(self._object.texture)
    if interior is not None:
      self._object.interior = interior._interior
      DMNSN_INCREF(self._object.interior)

  def __dealloc__(self):
    dmnsn_delete_object(self._object)

  def transform(self, Matrix trans not None):
    if self._object == NULL:
      raise TypeError('attempt to transform base Object')

    self._object.trans = dmnsn_matrix_mul(trans._m, self._object.trans)
    return self

  # Transform an object without affecting the texture
  cdef _intrinsicTransform(self, Matrix trans):
    self._object.trans = dmnsn_matrix_mul(self._object.trans, trans._m)
    if self._object.texture != NULL:
      self._object.texture.trans = dmnsn_matrix_mul(self._object.texture.trans,
                                                    trans.inverse()._m)

cdef class Plane(Object):
  def __init__(self, normal, double distance, *args, **kwargs):
    self._object = dmnsn_new_plane(Vector(normal)._v)
    Object.__init__(self, *args, **kwargs)

    self._intrinsicTransform(translate(distance*Vector(normal)))

cdef class Sphere(Object):
  def __init__(self, center, double radius, *args, **kwargs):
    self._object = dmnsn_new_sphere()
    Object.__init__(self, *args, **kwargs)

    cdef Matrix trans = translate(Vector(center))
    trans *= scale(radius, radius, radius)
    self._intrinsicTransform(trans)

cdef class Box(Object):
  def __init__(self, min, max, *args, **kwargs):
    self._object = dmnsn_new_cube()
    Object.__init__(self, *args, **kwargs)

    min = Vector(min)
    max = Vector(max)
    cdef Matrix trans = translate((max + min)/2)
    trans *= scale((max - min)/2)
    self._intrinsicTransform(trans)

cdef class Cone(Object):
  def __init__(self, bottom, double bottomRadius, top, double topRadius,
               bool open not None = False, *args, **kwargs):
    self._object = dmnsn_new_cone(bottomRadius, topRadius, open)
    Object.__init__(self, *args, **kwargs)

    # Lift the cone to start at the origin, then scale, rotate, and translate
    # properly

    cdef Vector dir = Vector(top) - Vector(bottom)

    cdef Matrix trans = translate(Y)
    trans = scale(1.0, dir.norm()/2, 1.0)*trans
    trans = _rawMatrix(dmnsn_alignment_matrix(dmnsn_y, dir._v, dmnsn_x, dmnsn_z))*trans
    trans = translate(bottom)*trans

    self._intrinsicTransform(trans)

cdef class Cylinder(Cone):
  def __init__(self, bottom, top, double radius, bool open not None = False):
    Cone.__init__(self,
                  bottom = bottom, bottomRadius = radius,
                  top    = top,    topRadius    = radius,
                  open = open)

cdef class Torus(Object):
  def __init__(self, double majorRadius, double minorRadius, *args, **kwargs):
    self._object = dmnsn_new_torus(majorRadius, minorRadius)
    Object.__init__(self, *args, **kwargs)

cdef class Union(Object):
  def __init__(self, objects, *args, **kwargs):
    if len(objects) < 2:
      raise TypeError('expected a list of two or more Objects')

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
  def __init__(self, objects, *args, **kwargs):
    if len(objects) < 2:
      raise TypeError('expected a list of two or more Objects')

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
  def __init__(self, objects, *args, **kwargs):
    if len(objects) < 2:
      raise TypeError('expected a list of two or more Objects')

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
  def __init__(self, objects, *args, **kwargs):
    if len(objects) < 2:
      raise TypeError('expected a list of two or more Objects')

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
  cdef dmnsn_light *_light

  def __dealloc__(self):
    dmnsn_delete_light(self._light)

cdef class PointLight(Light):
  def __init__(self, location, color):
    self._light = dmnsn_new_point_light(Vector(location)._v, Color(color)._c)
    Light.__init__(self)

###########
# Cameras #
###########

cdef class Camera:
  cdef dmnsn_camera *_camera

  def __cinit__(self):
    self._camera = NULL

  def __dealloc__(self):
    dmnsn_delete_camera(self._camera)

  def transform(self, Matrix trans not None):
    if self._camera == NULL:
      raise TypeError('attempt to transform base Camera')

    self._camera.trans = dmnsn_matrix_mul(trans._m, self._camera.trans)
    return self

cdef class PerspectiveCamera(Camera):
  def __init__(self, location = -Z, lookAt = 0, sky = Y,
               angle = dmnsn_degrees(atan(0.5))):
    self._camera = dmnsn_new_perspective_camera()
    Camera.__init__(self)

    # Apply the field of view angle
    self.transform(scale(2*tan(dmnsn_radians(angle))*(X + Y) + Z))

    cdef Vector dir = Vector(lookAt) - Vector(location)
    cdef Vector vsky = Vector(sky)

    # Line up the top of the viewport with the sky vector
    cdef Matrix alignSky = _rawMatrix(dmnsn_alignment_matrix(dmnsn_y, vsky._v,
                                                             dmnsn_z, dmnsn_x))
    cdef Vector forward = alignSky*Z
    cdef Vector right   = alignSky*X

    # Line up the look at point with lookAt
    self.transform(_rawMatrix(dmnsn_alignment_matrix(forward._v, dir._v,
                                                     vsky._v, right._v)))

    # Move the camera into position
    self.transform(translate(Vector(location)))

###############
# Sky Spheres #
###############

cdef class SkySphere:
  cdef dmnsn_sky_sphere *_skySphere

  def __init__(self, pigments):
    self._skySphere = dmnsn_new_sky_sphere()

    cdef Pigment realPigment
    for pigment in pigments:
      realPigment = Pigment(pigment)
      DMNSN_INCREF(realPigment._pigment)
      dmnsn_array_push(self._skySphere.pigments, &realPigment._pigment)

  def __dealloc__(self):
    dmnsn_delete_sky_sphere(self._skySphere)

  def transform(self, Matrix trans not None):
    self._skySphere.trans = dmnsn_matrix_mul(trans._m, self._skySphere.trans)
    return self

##########
# Scenes #
##########

cdef class Scene:
  cdef dmnsn_scene *_scene

  def __init__(self, Canvas canvas not None, objects, lights,
               Camera camera not None):
    self._scene = dmnsn_new_scene()

    self._scene.canvas = canvas._canvas
    DMNSN_INCREF(self._scene.canvas)

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

    # Account for image dimensions in the camera
    camera._camera.trans = dmnsn_matrix_mul(
      camera._camera.trans,
      dmnsn_scale_matrix(dmnsn_new_vector(canvas.width/canvas.height, 1.0, 1.0))
    )
    self._scene.camera = camera._camera
    DMNSN_INCREF(self._scene.camera)

  property defaultTexture:
    def __set__(self, Texture texture not None):
      dmnsn_delete_texture(self._scene.default_texture)
      self._scene.default_texture = texture._texture
      DMNSN_INCREF(self._scene.default_texture)

  property background:
    def __get__(self):
      return _rawColor(self._scene.background)
    def __set__(self, color):
      self._scene.background = Color(color)._c

  property skySphere:
    def __set__(self, SkySphere skySphere not None):
      dmnsn_delete_sky_sphere(self._scene.sky_sphere)
      self._scene.sky_sphere = skySphere._skySphere
      DMNSN_INCREF(self._scene.sky_sphere)

  property adcBailout:
    def __get__(self):
      return self._scene.adc_bailout
    def __set__(self, double bailout):
      self._scene.adc_bailout = bailout

  property recursionLimit:
    def __get__(self):
      return self._scene.reclimit
    def __set__(self, level):
      self._scene.reclimit = level

  property nThreads:
    def __get__(self):
      return self._scene.nthreads
    def __set__(self, n):
      self._scene.nthreads = n

  def raytrace(self):
    # Ensure the default texture is complete
    cdef Texture default = Texture(Black)
    dmnsn_texture_cascade(default._texture, &self._scene.default_texture)

    dmnsn_raytrace_scene(self._scene)

  def __dealloc__(self):
    dmnsn_delete_scene(self._scene)
