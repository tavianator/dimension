/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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

/*
 * Test scene -- code version of tests/dimension/demo.pov
 */
static dmnsn_scene *
dmnsn_new_test_scene(void)
{
  /* Allocate a new scene */
  dmnsn_scene *scene = dmnsn_new_scene();

  /* Default finish */
  scene->default_texture->finish = dmnsn_new_finish_combination(
    dmnsn_new_ambient_finish(
      dmnsn_color_mul(0.1, dmnsn_white)
    ),
    dmnsn_new_diffuse_finish(0.6)
  );

  /* Allocate a canvas */
  scene->canvas = dmnsn_new_canvas(768, 480);

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

  /* Background color */
  scene->background = dmnsn_clear;

  /* Sky sphere */
  scene->sky_sphere = dmnsn_new_sky_sphere();
  dmnsn_pattern *sky_gradient = dmnsn_new_gradient_pattern(dmnsn_y);
  dmnsn_map *sky_gradient_color_map = dmnsn_new_color_map();
  dmnsn_add_map_entry(sky_gradient_color_map, 0.0, &dmnsn_orange);
  dmnsn_color background = dmnsn_color_from_sRGB((dmnsn_sRGB){ 0.0, 0.1, 0.2 });
  background.filter = 0.1;
  dmnsn_add_map_entry(sky_gradient_color_map, 0.35, &background);
  dmnsn_pigment *sky_pigment
    = dmnsn_new_color_map_pigment(sky_gradient, sky_gradient_color_map);
  dmnsn_array_push(scene->sky_sphere->pigments, &sky_pigment);

  /* Light source */
  dmnsn_light *light = dmnsn_new_point_light(
    dmnsn_new_vector(-15.0, 20.0, 10.0),
    dmnsn_white
  );
  dmnsn_array_push(scene->lights, &light);

  /* Now make our objects */

  dmnsn_object *cube = dmnsn_new_cube();
  cube->trans = dmnsn_rotation_matrix(
    dmnsn_new_vector(dmnsn_radians(45.0), 0.0, 0.0)
  );
  cube->texture = dmnsn_new_texture();

  dmnsn_color cube_color = dmnsn_blue;
  cube_color.filter = 0.25;
  cube_color.trans  = 0.5;
  cube->texture->pigment = dmnsn_new_solid_pigment(cube_color);

  dmnsn_color reflect = dmnsn_color_mul(0.5, dmnsn_white);
  cube->texture->finish = dmnsn_new_reflective_finish(reflect, reflect, 1.0);

  cube->interior = dmnsn_new_interior();
  cube->interior->ior = 1.1;

  dmnsn_object *sphere = dmnsn_new_sphere();
  sphere->texture = dmnsn_new_texture();
  sphere->texture->pigment = dmnsn_new_solid_pigment(dmnsn_green);
  sphere->texture->finish  = dmnsn_new_phong_finish(0.2, 40.0);
  sphere->trans = dmnsn_scale_matrix(dmnsn_new_vector(1.25, 1.25, 1.25));

  dmnsn_object *csg = dmnsn_new_csg_difference(cube, sphere);
  dmnsn_array_push(scene->objects, &csg);

  dmnsn_array *arrow_array = dmnsn_new_array(sizeof(dmnsn_object *));

  dmnsn_object *cylinder = dmnsn_new_cone(0.1, 0.1, false);
  cylinder->trans = dmnsn_scale_matrix(dmnsn_new_vector(1.0, 1.25, 1.0));
  dmnsn_array_push(arrow_array, &cylinder);

  dmnsn_object *cone = dmnsn_new_cone(0.1, 0.0, true);
  cone->trans =
    dmnsn_matrix_mul(
      dmnsn_translation_matrix(dmnsn_new_vector(0.0, 1.375, 0.0)),
      dmnsn_scale_matrix(dmnsn_new_vector(1.0, 0.125, 1.0))
    );
  dmnsn_array_push(arrow_array, &cone);

  dmnsn_object *arrow = dmnsn_new_csg_union(arrow_array);
  arrow->trans = dmnsn_rotation_matrix(
    dmnsn_new_vector(dmnsn_radians(-45.0), 0.0, 0.0)
  );
  dmnsn_pattern *gradient = dmnsn_new_gradient_pattern(dmnsn_y);
  dmnsn_map *gradient_color_map = dmnsn_new_color_map();
  dmnsn_add_map_entry(gradient_color_map, 0.0,     &dmnsn_red);
  dmnsn_add_map_entry(gradient_color_map, 1.0/6.0, &dmnsn_orange);
  dmnsn_add_map_entry(gradient_color_map, 2.0/6.0, &dmnsn_yellow);
  dmnsn_add_map_entry(gradient_color_map, 3.0/6.0, &dmnsn_green);
  dmnsn_add_map_entry(gradient_color_map, 4.0/6.0, &dmnsn_blue);
  dmnsn_add_map_entry(gradient_color_map, 5.0/6.0, &dmnsn_magenta);
  dmnsn_add_map_entry(gradient_color_map, 1.0,     &dmnsn_red);
  arrow->texture = dmnsn_new_texture();
  arrow->texture->pigment
    = dmnsn_new_color_map_pigment(gradient, gradient_color_map);
  arrow->texture->trans =
    dmnsn_matrix_mul(
      dmnsn_translation_matrix(dmnsn_new_vector(0.0, -1.25, 0.0)),
      dmnsn_scale_matrix(dmnsn_new_vector(1.0, 2.75, 1.0))
    );
  dmnsn_array_push(scene->objects, &arrow);
  dmnsn_delete_array(arrow_array);

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
  torii->trans = dmnsn_rotation_matrix(
    dmnsn_new_vector(dmnsn_radians(-45.0), 0.0, 0.0)
  );
  torii->texture = dmnsn_new_texture();
  torii->texture->pigment = dmnsn_new_solid_pigment(dmnsn_blue);
  torii->texture->finish  = dmnsn_new_ambient_finish(dmnsn_white);
  dmnsn_array_push(scene->objects, &torii);
  dmnsn_delete_array(torus_array);

  dmnsn_object *plane = dmnsn_new_plane(dmnsn_new_vector(0.0, 1.0, 0.0));
  plane->trans = dmnsn_translation_matrix(dmnsn_new_vector(0.0, -2.0, 0.0));
  plane->texture = dmnsn_new_texture();
  dmnsn_pattern *checker = dmnsn_new_checker_pattern();
  dmnsn_map *checker_color_map = dmnsn_new_color_map();
  dmnsn_add_map_entry(checker_color_map, 0.0, &dmnsn_black);
  dmnsn_add_map_entry(checker_color_map, 1.0, &dmnsn_white);
  plane->texture->pigment
    = dmnsn_new_color_map_pigment(checker, checker_color_map);
  plane->texture->pigment->quick_color
    = dmnsn_color_from_sRGB((dmnsn_sRGB){ 1.0, 0.5, 0.75 });
  dmnsn_array_push(scene->objects, &plane);

  return scene;
}

int
main(void)
{
  bool have_png = true, have_gl = true;

  /* Set the resilience low for tests */
  dmnsn_set_resilience(DMNSN_SEVERITY_LOW);

  /* Create the test scene */
  dmnsn_scene *scene = dmnsn_new_test_scene();

  /* Optimize the canvas for PNG export */
  errno = 0;
  if (dmnsn_png_optimize_canvas(scene->canvas) != 0) {
    if (errno == ENOTSUP) {
      have_png = false;
    } else {
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Couldn't optimize canvas for PNG! ---\n");
      return EXIT_FAILURE;
    }
  }

  /* Optimize the canvas for GL drawing */
  errno = 0;
  if (dmnsn_gl_optimize_canvas(scene->canvas) != 0) {
    if (errno == ENOTSUP) {
      have_gl = false;
    } else {
      dmnsn_delete_scene(scene);
      fprintf(stderr, "--- Couldn't optimize canvas for GL! ---\n");
      return EXIT_FAILURE;
    }
  }

  dmnsn_clear_canvas(scene->canvas, dmnsn_black);

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
  dmnsn_progress *progress = dmnsn_raytrace_scene_async(scene);

  /* Display the scene as it's rendered */
  if (display) {
    while (dmnsn_get_progress(progress) < 1.0) {
      if (dmnsn_gl_write_canvas(scene->canvas) != 0) {
        dmnsn_delete_display(display);
        dmnsn_delete_scene(scene);
        fprintf(stderr, "--- Drawing to OpenGL failed! ---\n");
        return EXIT_FAILURE;
      }
      dmnsn_display_flush(display);
    }
  }

  if (dmnsn_finish_progress(progress) != 0) {
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
  return EXIT_SUCCESS;
}
