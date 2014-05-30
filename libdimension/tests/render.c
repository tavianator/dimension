/*************************************************************************
 * Copyright (C) 2010-2014 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
 *                                                                       *
 * The Dimension Test Suite is free software; you can redistribute it    *
 * and/or modify it under the terms of the GNU General Public License as *
 * published by the Free Software Foundation; either version 3 of the    *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Test Suite is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "tests.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

static void
dmnsn_test_scene_set_defaults(dmnsn_scene *scene)
{
  /* Default texture */
  scene->default_texture->pigment =
    dmnsn_new_solid_pigment(DMNSN_TCOLOR(dmnsn_black));
  dmnsn_finish *default_finish = &scene->default_texture->finish;
  default_finish->ambient = dmnsn_new_ambient(
    dmnsn_color_from_sRGB(dmnsn_color_mul(0.1, dmnsn_white))
  );
  default_finish->diffuse = dmnsn_new_lambertian(dmnsn_sRGB_inverse_gamma(0.7));
}

static void
dmnsn_test_scene_add_canvas(dmnsn_pool *pool, dmnsn_scene *scene)
{
  scene->canvas = dmnsn_new_canvas(pool, 768, 480);
}

static void
dmnsn_test_scene_add_camera(dmnsn_scene *scene)
{
  /* Set up the transformation matrix for the perspective camera */
  dmnsn_matrix trans = dmnsn_scale_matrix(
    dmnsn_new_vector(
      ((double)scene->canvas->width)/scene->canvas->height, 1.0, 1.0
    )
  );
  trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_new_vector(0.0624188099959577, 0.0, 0.0)),
    trans
  );
  trans = dmnsn_matrix_mul(
    dmnsn_translation_matrix(dmnsn_new_vector(0.0, 0.25, -4.0)),
    trans
  );
  trans = dmnsn_matrix_mul(
    dmnsn_rotation_matrix(dmnsn_new_vector(0.0, dmnsn_radians(53.0), 0.0)),
    trans
  );

  /* Create a perspective camera */
  scene->camera = dmnsn_new_perspective_camera();
  scene->camera->trans = trans;
}

static void
dmnsn_test_scene_add_background(dmnsn_pool *pool, dmnsn_scene *scene)
{
  dmnsn_pattern *sky_gradient = dmnsn_new_gradient_pattern(dmnsn_y);
  dmnsn_map *sky_gradient_pigment_map = dmnsn_new_pigment_map();

  dmnsn_canvas *png_canvas = NULL;
  dmnsn_pigment *png_pigment;
  FILE *png = fopen("png2.png", "rb");
  if (png) {
    png_canvas = dmnsn_png_read_canvas(pool, png);
    fclose(png);
  }
  if (png_canvas) {
    png_pigment = dmnsn_new_canvas_pigment(png_canvas);
    png_pigment->trans = dmnsn_rotation_matrix(
      dmnsn_new_vector(0.0, dmnsn_radians(53.0), 0.0)
    );
  } else {
    /* Loading png2.png failed, possibly compiled with --disable-png */
    fprintf(stderr, "--- WARNING: Couldn't open or read png2.png! ---\n");
    png_pigment = dmnsn_new_solid_pigment(DMNSN_TCOLOR(dmnsn_orange));
  }
  dmnsn_map_add_entry(sky_gradient_pigment_map, 0.0, &png_pigment);

  dmnsn_color background = dmnsn_color_from_sRGB(
    dmnsn_new_color(0.0, 0.1, 0.2)
  );
  dmnsn_tcolor tbackground = dmnsn_new_tcolor(background, 0.1, 0.0);
  dmnsn_pigment *bkgpigment = dmnsn_new_solid_pigment(tbackground);
  dmnsn_map_add_entry(sky_gradient_pigment_map, 0.35, &bkgpigment);

  scene->background =
    dmnsn_new_pigment_map_pigment(sky_gradient, sky_gradient_pigment_map,
                                  DMNSN_PIGMENT_MAP_SRGB);
}

static void
dmnsn_test_scene_add_lights(dmnsn_scene *scene)
{
  dmnsn_light *light = dmnsn_new_point_light(
    dmnsn_new_vector(-15.0, 20.0, 10.0),
    dmnsn_white
  );
  dmnsn_array_push(scene->lights, &light);
}

static void
dmnsn_test_scene_add_hollow_cube(dmnsn_scene *scene)
{
  dmnsn_object *cube = dmnsn_new_cube();
  cube->trans = dmnsn_rotation_matrix(
    dmnsn_new_vector(dmnsn_radians(45.0), 0.0, 0.0)
  );

  cube->texture = dmnsn_new_texture();
  dmnsn_tcolor cube_color = dmnsn_new_tcolor(dmnsn_blue, 0.75, 1.0/3.0);
  cube->texture->pigment = dmnsn_new_solid_pigment(cube_color);

  dmnsn_color reflect =
    dmnsn_color_from_sRGB(dmnsn_color_mul(0.5, dmnsn_white));
  cube->texture->finish.reflection =
    dmnsn_new_basic_reflection(dmnsn_black, reflect, 1.0);

  cube->interior = dmnsn_new_interior();
  cube->interior->ior = 1.1;

  dmnsn_object *sphere = dmnsn_new_sphere();
  sphere->texture = dmnsn_new_texture();
  sphere->texture->pigment = dmnsn_new_solid_pigment(DMNSN_TCOLOR(dmnsn_green));
  sphere->texture->finish.specular =
    dmnsn_new_phong(dmnsn_sRGB_inverse_gamma(0.2), 40.0);
  sphere->trans = dmnsn_scale_matrix(dmnsn_new_vector(1.25, 1.25, 1.25));

  dmnsn_object *hollow_cube = dmnsn_new_csg_difference(cube, sphere);
  dmnsn_array_push(scene->objects, &hollow_cube);
}

#define dmnsn_pigment_map_add_color(map, n, color)              \
  do {                                                          \
    dmnsn_tcolor tcolor = DMNSN_TCOLOR(color);                  \
    dmnsn_pigment *pigment = dmnsn_new_solid_pigment(tcolor);   \
    dmnsn_map_add_entry(map, n, &pigment);                      \
  } while (0)

static void
dmnsn_test_scene_add_spike(dmnsn_scene *scene)
{
  dmnsn_array *arrow_array = dmnsn_new_array(sizeof(dmnsn_object *));

  dmnsn_object *cylinder = dmnsn_new_cone(0.1, 0.1, false);
  cylinder->trans = dmnsn_scale_matrix(dmnsn_new_vector(1.0, 1.25, 1.0));
  dmnsn_array_push(arrow_array, &cylinder);

  dmnsn_object *cone = dmnsn_new_cone(0.1, 0.0, true);
  cone->trans = dmnsn_matrix_mul(
    dmnsn_translation_matrix(dmnsn_new_vector(0.0, 1.375, 0.0)),
    dmnsn_scale_matrix(dmnsn_new_vector(1.0, 0.125, 1.0))
  );
  dmnsn_array_push(arrow_array, &cone);

  dmnsn_object *arrow = dmnsn_new_csg_union(arrow_array);
  dmnsn_delete_array(arrow_array);
  dmnsn_pattern *gradient = dmnsn_new_gradient_pattern(dmnsn_y);
  dmnsn_map *gradient_pigment_map = dmnsn_new_pigment_map();
  dmnsn_pigment_map_add_color(gradient_pigment_map, 0.0,     dmnsn_red);
  dmnsn_pigment_map_add_color(gradient_pigment_map, 1.0/6.0, dmnsn_orange);
  dmnsn_pigment_map_add_color(gradient_pigment_map, 2.0/6.0, dmnsn_yellow);
  dmnsn_pigment_map_add_color(gradient_pigment_map, 3.0/6.0, dmnsn_green);
  dmnsn_pigment_map_add_color(gradient_pigment_map, 4.0/6.0, dmnsn_blue);
  dmnsn_pigment_map_add_color(gradient_pigment_map, 5.0/6.0, dmnsn_magenta);
  dmnsn_pigment_map_add_color(gradient_pigment_map, 1.0,     dmnsn_red);
  arrow->texture = dmnsn_new_texture();
  arrow->texture->pigment =
    dmnsn_new_pigment_map_pigment(gradient, gradient_pigment_map,
                                  DMNSN_PIGMENT_MAP_SRGB);

  arrow->texture->trans =
    dmnsn_matrix_mul(
      dmnsn_translation_matrix(dmnsn_new_vector(0.0, -1.25, 0.0)),
      dmnsn_scale_matrix(dmnsn_new_vector(1.0, 2.75, 1.0))
    );

  dmnsn_array *torus_array = dmnsn_new_array(sizeof(dmnsn_object *));

  dmnsn_object *torus1 = dmnsn_new_torus(0.15, 0.05);
  torus1->trans = dmnsn_translation_matrix(dmnsn_new_vector(0.0, -1.0, 0.0));
  dmnsn_array_push(torus_array, &torus1);

  dmnsn_object *torus2 = dmnsn_new_torus(0.15, 0.05);
  dmnsn_array_push(torus_array, &torus2);

  dmnsn_object *torus3 = dmnsn_new_torus(0.15, 0.05);
  torus3->trans = dmnsn_translation_matrix(dmnsn_new_vector(0.0, 1.0, 0.0));
  dmnsn_array_push(torus_array, &torus3);

  dmnsn_object *torii = dmnsn_new_csg_union(torus_array);
  dmnsn_delete_array(torus_array);
  torii->texture = dmnsn_new_texture();
  torii->texture->pigment = dmnsn_new_solid_pigment(DMNSN_TCOLOR(dmnsn_blue));
  torii->texture->finish.ambient = dmnsn_new_ambient(dmnsn_white);

  dmnsn_array *spike_array = dmnsn_new_array(sizeof(dmnsn_object *));
  dmnsn_array_push(spike_array, &arrow);
  dmnsn_array_push(spike_array, &torii);
  dmnsn_object *spike = dmnsn_new_csg_union(spike_array);
  dmnsn_delete_array(spike_array);
  spike->trans = dmnsn_rotation_matrix(
    dmnsn_new_vector(dmnsn_radians(-45.0), 0.0, 0.0)
  );
  dmnsn_array_push(scene->objects, &spike);
}

static void
dmnsn_test_scene_add_triangle_strip(dmnsn_scene *scene)
{
  dmnsn_array *strip_array = dmnsn_new_array(sizeof(dmnsn_object *));
  dmnsn_vector a = dmnsn_zero;
  dmnsn_vector b = dmnsn_new_vector(0.0, sqrt(3.0)/2.0, 0.5);
  dmnsn_vector c = dmnsn_z;
  dmnsn_texture *strip_textures[3] = {
    dmnsn_new_texture(),
    dmnsn_new_texture(),
    dmnsn_new_texture(),
  };
  strip_textures[0]->pigment =
    dmnsn_new_solid_pigment(DMNSN_TCOLOR(dmnsn_red));
  strip_textures[1]->pigment =
    dmnsn_new_solid_pigment(DMNSN_TCOLOR(dmnsn_orange));
  strip_textures[2]->pigment =
    dmnsn_new_solid_pigment(DMNSN_TCOLOR(dmnsn_yellow));
  for (unsigned int i = 0; i < 128; ++i) {
    dmnsn_object *triangle = dmnsn_new_flat_triangle(a, b, c);
    triangle->texture = strip_textures[i%3];
    DMNSN_INCREF(triangle->texture);
    dmnsn_array_push(strip_array, &triangle);

    a = b;
    b = c;
    c = dmnsn_vector_add(a, dmnsn_z);
  }
  for (unsigned int i = 0; i < 3; ++i) {
    dmnsn_delete_texture(strip_textures[i]);
  }

  dmnsn_object *strip = dmnsn_new_csg_union(strip_array);
  dmnsn_delete_array(strip_array);
  strip->trans = dmnsn_translation_matrix(dmnsn_new_vector(5.0, -2.0, -4.0));
  dmnsn_array_push(scene->objects, &strip);
}

static void
dmnsn_test_scene_add_ground(dmnsn_scene *scene)
{
  dmnsn_object *plane = dmnsn_new_plane(dmnsn_new_vector(0.0, 1.0, 0.0));
  plane->trans = dmnsn_translation_matrix(dmnsn_new_vector(0.0, -2.0, 0.0));
  dmnsn_pattern *checker1 = dmnsn_new_checker_pattern();
  dmnsn_pattern *checker2 = dmnsn_new_checker_pattern();
  dmnsn_map *small_map = dmnsn_new_pigment_map();
  dmnsn_pigment_map_add_color(small_map, 0.0, dmnsn_black);
  dmnsn_pigment_map_add_color(small_map, 1.0, dmnsn_white);
  dmnsn_pigment *small_pigment =
    dmnsn_new_pigment_map_pigment(checker1, small_map,
                                  DMNSN_PIGMENT_MAP_REGULAR);
  small_pigment->trans =
    dmnsn_scale_matrix(dmnsn_new_vector(1.0/3.0, 1.0/3.0, 1.0/3.0));
  dmnsn_map *big_map = dmnsn_new_pigment_map();
  dmnsn_pigment_map_add_color(big_map, 0.0, dmnsn_white);
  dmnsn_map_add_entry(big_map, 1.0, &small_pigment);
  plane->texture = dmnsn_new_texture();
  plane->texture->pigment =
    dmnsn_new_pigment_map_pigment(checker2, big_map, DMNSN_PIGMENT_MAP_REGULAR);
  plane->texture->pigment->quick_color = DMNSN_TCOLOR(
    dmnsn_color_from_sRGB(
      dmnsn_new_color(1.0, 0.5, 0.75)
    )
  );
  dmnsn_array_push(scene->objects, &plane);
}

static void
dmnsn_test_scene_add_objects(dmnsn_scene *scene)
{
  dmnsn_test_scene_add_hollow_cube(scene);
  dmnsn_test_scene_add_spike(scene);
  dmnsn_test_scene_add_triangle_strip(scene);
  dmnsn_test_scene_add_ground(scene);
}

/*
 * Test scene
 */
static dmnsn_scene *
dmnsn_new_test_scene(dmnsn_pool *pool)
{
  dmnsn_scene *scene = dmnsn_new_scene(pool);
  dmnsn_test_scene_set_defaults(scene);
  dmnsn_test_scene_add_canvas(pool, scene);
  dmnsn_test_scene_add_camera(scene);
  dmnsn_test_scene_add_background(pool, scene);
  dmnsn_test_scene_add_lights(scene);
  dmnsn_test_scene_add_objects(scene);
  return scene;
}

int
main(void)
{
  /* Treat warnings as errors for tests */
  dmnsn_die_on_warnings(true);

  /* Create the test scene */
  dmnsn_pool *pool = dmnsn_new_pool();
  dmnsn_scene *scene = dmnsn_new_test_scene(pool);

  /* Optimize the canvas for PNG export */
  bool have_png = true;
  errno = 0;
  if (dmnsn_png_optimize_canvas(scene->canvas) != 0) {
    if (errno == ENOSYS) {
      have_png = false;
    } else {
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Couldn't optimize canvas for PNG! ---\n");
      return EXIT_FAILURE;
    }
  }

  /* Optimize the canvas for GL drawing */
  bool have_gl = true;
  errno = 0;
  if (dmnsn_gl_optimize_canvas(scene->canvas) != 0) {
    if (errno == ENOSYS) {
      have_gl = false;
    } else {
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Couldn't optimize canvas for GL! ---\n");
      return EXIT_FAILURE;
    }
  }

  dmnsn_canvas_clear(scene->canvas, DMNSN_TCOLOR(dmnsn_black));

  /* Create a new glX display */
  dmnsn_display *display = NULL;
  if (have_gl) {
    display = dmnsn_new_display(scene->canvas);
    if (!display) {
      fprintf(stderr, "--- WARNING: Couldn't initialize X or glX! ---\n");
    }
  }

  /* Render the scene */

  printf("Rendering scene\n");
  dmnsn_future *future = dmnsn_ray_trace_async(scene);

  /* Display the scene as it's rendered */
  if (display) {
    while (!dmnsn_future_is_done(future)) {
      dmnsn_future_pause(future);
        if (dmnsn_gl_write_canvas(scene->canvas) != 0) {
          dmnsn_delete_display(display);
          dmnsn_delete_scene(scene);
          fprintf(stderr, "--- Drawing to OpenGL failed! ---\n");
          return EXIT_FAILURE;
        }
      dmnsn_future_resume(future);

      dmnsn_display_flush(display);
    }
  }

  if (dmnsn_future_join(future) != 0) {
    dmnsn_delete_display(display);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "--- Raytracing failed! ---\n");
    return EXIT_FAILURE;
  }

  /* Make sure we show the completed rendering */
  if (display) {
    printf("Drawing to OpenGL\n");
    if (dmnsn_gl_write_canvas(scene->canvas) != 0) {
      dmnsn_delete_display(display);
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Drawing to OpenGL failed! ---\n");
      return EXIT_FAILURE;
    }
    dmnsn_display_flush(display);
  }

  if (have_png) {
    printf("Writing scene to PNG\n");
    FILE *ofile = fopen("render.png", "wb");
    if (!ofile) {
      dmnsn_delete_display(display);
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Couldn't open 'render.png' for writing! ---\n");
      return EXIT_FAILURE;
    }

    if (dmnsn_png_write_canvas(scene->canvas, ofile) != 0) {
      fclose(ofile);
      dmnsn_delete_display(display);
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Writing canvas to PNG failed! ---\n");
      return EXIT_FAILURE;
    }

    fclose(ofile);
  }

  dmnsn_delete_display(display);
  dmnsn_delete_scene(scene);
  dmnsn_delete_pool(pool);
  return EXIT_SUCCESS;
}
