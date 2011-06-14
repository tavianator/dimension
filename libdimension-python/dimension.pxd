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

from cpython cimport bool
from libc.math cimport *
from libc.stdio cimport *

cdef extern from "errno.h":
  int errno

cdef extern from "../libdimension/dimension.h":

  ###########
  # Globals #
  ###########

  ctypedef void dmnsn_callback_fn(void *ptr)

  void DMNSN_INCREF(void *)

  void dmnsn_die_on_warnings(bint always_die)

  double dmnsn_epsilon

  ##########
  # Arrays #
  ##########

  ctypedef struct dmnsn_array:
    pass

  dmnsn_array *dmnsn_new_array(size_t objsize)
  void dmnsn_delete_array(dmnsn_array *array)

  size_t dmnsn_array_size(dmnsn_array *array)
  void dmnsn_array_resize(dmnsn_array *array, size_t length)
  dmnsn_array *dmnsn_array_copy(dmnsn_array *array)
  dmnsn_array *dmnsn_array_split(dmnsn_array *array)
  void dmnsn_array_get(dmnsn_array *array, size_t i, void *obj)
  void dmnsn_array_set(dmnsn_array *array, size_t i, void *obj)
  void *dmnsn_array_first(dmnsn_array *array)
  void *dmnsn_array_last(dmnsn_array *array)
  void *dmnsn_array_at(dmnsn_array *array, size_t i)
  void dmnsn_array_push(dmnsn_array *array, void *obj)
  void dmnsn_array_pop(dmnsn_array *array, void *obj)
  void dmnsn_array_insert(dmnsn_array *array, size_t i, void *obj)
  void dmnsn_array_remove(dmnsn_array *array, size_t i)
  void dmnsn_array_apply(dmnsn_array *array, dmnsn_callback_fn *callback)

  ############
  # Geometry #
  ############

  double dmnsn_radians(double degrees)
  double dmnsn_degrees(double radians)

  ctypedef struct dmnsn_vector:
    double x
    double y
    double z

  dmnsn_vector dmnsn_new_vector(double x, double y, double z)

  dmnsn_vector dmnsn_vector_negate(dmnsn_vector rhs)

  dmnsn_vector dmnsn_vector_add(dmnsn_vector lhs, dmnsn_vector rhs)
  dmnsn_vector dmnsn_vector_sub(dmnsn_vector lhs, dmnsn_vector rhs)
  dmnsn_vector dmnsn_vector_mul(double lhs, dmnsn_vector rhs)
  dmnsn_vector dmnsn_vector_div(dmnsn_vector lhs, double rhs)

  dmnsn_vector dmnsn_vector_cross(dmnsn_vector lhs, dmnsn_vector rhs)
  double dmnsn_vector_dot(dmnsn_vector lhs, dmnsn_vector rhs)
  dmnsn_vector dmnsn_vector_proj(dmnsn_vector u, dmnsn_vector d)

  double dmnsn_vector_norm(dmnsn_vector v)
  dmnsn_vector dmnsn_vector_normalized(dmnsn_vector v)

  dmnsn_vector dmnsn_zero
  dmnsn_vector dmnsn_x
  dmnsn_vector dmnsn_y
  dmnsn_vector dmnsn_z

  ctypedef struct dmnsn_matrix:
    double n[3][4]

  dmnsn_matrix dmnsn_new_matrix(double a1, double a2, double a3, double a4,
                                double b1, double b2, double b3, double b4,
                                double c1, double c2, double c3, double c4)

  dmnsn_matrix dmnsn_matrix_inverse(dmnsn_matrix m)

  dmnsn_matrix dmnsn_matrix_mul(dmnsn_matrix lhs, dmnsn_matrix rhs)
  dmnsn_vector dmnsn_transform_vector(dmnsn_matrix lhs, dmnsn_vector rhs)

  dmnsn_matrix dmnsn_scale_matrix(dmnsn_vector s)
  dmnsn_matrix dmnsn_translation_matrix(dmnsn_vector d)
  dmnsn_matrix dmnsn_rotation_matrix(dmnsn_vector theta)
  dmnsn_matrix dmnsn_alignment_matrix(dmnsn_vector frm, dmnsn_vector to,
                                      dmnsn_vector axis1, dmnsn_vector axis2)

  ##########
  # Colors #
  ##########

  ctypedef struct dmnsn_color:
    double R
    double G
    double B
    double trans
    double filter

  dmnsn_color dmnsn_new_color(double R, double G, double B)
  dmnsn_color dmnsn_new_color5(double R, double G, double B,
                               double trans, double filter)

  dmnsn_color dmnsn_color_from_sRGB(dmnsn_color color)
  dmnsn_color dmnsn_color_to_sRGB(dmnsn_color color)

  double dmnsn_color_intensity(dmnsn_color color)
  dmnsn_color dmnsn_color_add(dmnsn_color color1, dmnsn_color color2)
  dmnsn_color dmnsn_color_mul(double n, dmnsn_color color)

  bint dmnsn_color_is_black(dmnsn_color color)

  dmnsn_color dmnsn_black
  dmnsn_color dmnsn_white
  dmnsn_color dmnsn_clear
  dmnsn_color dmnsn_red
  dmnsn_color dmnsn_green
  dmnsn_color dmnsn_blue
  dmnsn_color dmnsn_magenta
  dmnsn_color dmnsn_orange
  dmnsn_color dmnsn_yellow
  dmnsn_color dmnsn_cyan

  ############
  # Canvases #
  ############

  ctypedef struct dmnsn_canvas:
    size_t width
    size_t height

  dmnsn_canvas *dmnsn_new_canvas(size_t width, size_t height)
  void dmnsn_delete_canvas(dmnsn_canvas *canvas)

  void dmnsn_clear_canvas(dmnsn_canvas *canvas, dmnsn_color color)

  int dmnsn_png_optimize_canvas(dmnsn_canvas *canvas)
  int dmnsn_png_write_canvas(dmnsn_canvas *canvas, FILE *file)

  int dmnsn_gl_optimize_canvas(dmnsn_canvas *canvas)
  int dmnsn_gl_write_canvas(dmnsn_canvas *canvas)

  ############
  # Patterns #
  ############

  ctypedef struct dmnsn_pattern:
    dmnsn_matrix trans

  void dmnsn_delete_pattern(dmnsn_pattern *pattern)

  dmnsn_pattern *dmnsn_new_checker_pattern()
  dmnsn_pattern *dmnsn_new_gradient_pattern(dmnsn_vector orientation)

  ########
  # Maps #
  ########

  ctypedef struct dmnsn_map:
    pass

  void dmnsn_delete_map(dmnsn_map *map)

  void dmnsn_add_map_entry(dmnsn_map *map, double n, void *obj)
  size_t dmnsn_map_size(dmnsn_map *map)

  dmnsn_map *dmnsn_new_color_map()
  dmnsn_map *dmnsn_new_pigment_map()

  ############
  # Pigments #
  ############

  ctypedef struct dmnsn_pigment:
    dmnsn_matrix trans

  ctypedef enum dmnsn_pigment_map_flags:
    DMNSN_PIGMENT_MAP_REGULAR
    DMNSN_PIGMENT_MAP_SRGB

  void dmnsn_delete_pigment(dmnsn_pigment *pigment)

  dmnsn_pigment *dmnsn_new_solid_pigment(dmnsn_color color)
  dmnsn_pigment *dmnsn_new_canvas_pigment(dmnsn_canvas *canvas)
  dmnsn_pigment *dmnsn_new_color_map_pigment(dmnsn_pattern *pattern,
                                             dmnsn_map *map,
                                             dmnsn_pigment_map_flags flags)
  dmnsn_pigment *dmnsn_new_pigment_map_pigment(dmnsn_pattern *pattern,
                                               dmnsn_map *map,
                                               dmnsn_pigment_map_flags flags)

  ############
  # Finishes #
  ############

  ctypedef struct dmnsn_ambient
  ctypedef struct dmnsn_diffuse
  ctypedef struct dmnsn_specular
  ctypedef struct dmnsn_reflection

  ctypedef struct dmnsn_finish:
    dmnsn_ambient    *ambient
    dmnsn_diffuse    *diffuse
    dmnsn_specular   *specular
    dmnsn_reflection *reflection

  dmnsn_finish dmnsn_new_finish()
  void dmnsn_delete_finish(dmnsn_finish finish)

  dmnsn_ambient *dmnsn_new_basic_ambient(dmnsn_color ambient)
  dmnsn_diffuse *dmnsn_new_lambertian(double diffuse)
  dmnsn_specular *dmnsn_new_phong(double specular, double exp)
  dmnsn_reflection *dmnsn_new_basic_reflection(dmnsn_color min, dmnsn_color max,
                                               double falloff)

  ############
  # Textures #
  ############

  ctypedef struct dmnsn_texture:
    dmnsn_pigment *pigment
    dmnsn_finish finish
    dmnsn_matrix trans

  dmnsn_texture *dmnsn_new_texture()
  void dmnsn_delete_texture(dmnsn_texture *texture)

  void dmnsn_texture_cascade(dmnsn_texture *default_texture,
                             dmnsn_texture **texture)

  #############
  # Interiors #
  #############

  ctypedef struct dmnsn_interior:
    double ior

  dmnsn_interior *dmnsn_new_interior()
  void dmnsn_delete_interior(dmnsn_interior *interior)

  ###########
  # Objects #
  ###########

  ctypedef struct dmnsn_object:
    dmnsn_texture *texture
    dmnsn_interior *interior
    dmnsn_matrix trans

  dmnsn_object *dmnsn_new_object()
  void dmnsn_delete_object(dmnsn_object *object)

  dmnsn_object *dmnsn_new_plane(dmnsn_vector normal)
  dmnsn_object *dmnsn_new_sphere()
  dmnsn_object *dmnsn_new_cube()
  dmnsn_object *dmnsn_new_cone(double r1, double r2, bint open)
  dmnsn_object *dmnsn_new_torus(double major, double minor)

  dmnsn_object *dmnsn_new_csg_union(dmnsn_array *objects)
  dmnsn_object *dmnsn_new_csg_intersection(dmnsn_object *A, dmnsn_object *B)
  dmnsn_object *dmnsn_new_csg_difference(dmnsn_object *A, dmnsn_object *B)
  dmnsn_object *dmnsn_new_csg_merge(dmnsn_object *A, dmnsn_object *B)

  ##########
  # Lights #
  ##########

  ctypedef struct dmnsn_light

  dmnsn_light *dmnsn_new_light()
  void dmnsn_delete_light(dmnsn_light *light)

  dmnsn_light *dmnsn_new_point_light(dmnsn_vector x0, dmnsn_color color)

  ###########
  # Cameras #
  ###########

  ctypedef struct dmnsn_camera:
    dmnsn_matrix trans

  dmnsn_camera *dmnsn_new_camera()
  void dmnsn_delete_camera(dmnsn_camera *camera)

  dmnsn_camera *dmnsn_new_perspective_camera()

  ###############
  # Sky Spheres #
  ###############

  ctypedef struct dmnsn_sky_sphere:
    dmnsn_array *pigments
    dmnsn_matrix trans

  dmnsn_sky_sphere *dmnsn_new_sky_sphere()
  void dmnsn_delete_sky_sphere(dmnsn_sky_sphere *sky_sphere)

  ##########
  # Scenes #
  ##########

  ctypedef enum dmnsn_quality:
    DMNSN_RENDER_NONE
    DMNSN_RENDER_PIGMENT
    DMNSN_RENDER_LIGHTS
    DMNSN_RENDER_FINISH
    DMNSN_RENDER_TRANSLUCENCY
    DMNSN_RENDER_REFLECTION
    DMNSN_RENDER_FULL

  ctypedef struct dmnsn_scene:
    dmnsn_color background
    dmnsn_sky_sphere *sky_sphere
    dmnsn_texture *default_texture
    dmnsn_interior *default_interior

    dmnsn_array *objects
    dmnsn_array *lights
    dmnsn_camera *camera
    dmnsn_canvas *canvas

    dmnsn_quality quality
    unsigned int reclimit
    double adc_bailout
    unsigned int nthreads

  dmnsn_scene *dmnsn_new_scene()
  void dmnsn_delete_scene(dmnsn_scene *scene)

  void dmnsn_raytrace_scene(dmnsn_scene *scene)
