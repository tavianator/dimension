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
      ((double)scene->canvas->x)/scene->canvas->y, 1.0, 1.0
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
  dmnsn_set_perspective_camera_trans(scene->camera, trans);

  /* Background color */
  scene->background = dmnsn_color_from_sRGB((dmnsn_sRGB){ 0.0, 0.1, 0.2 });
  scene->background.filter = 0.1;

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

  dmnsn_object *plane = dmnsn_new_plane(dmnsn_new_vector(0.0, 1.0, 0.0));
  plane->trans = dmnsn_translation_matrix(dmnsn_new_vector(0.0, -2.0, 0.0));
  plane->texture = dmnsn_new_texture();
  plane->texture->pigment = dmnsn_new_solid_pigment(dmnsn_white);
  dmnsn_array_push(scene->objects, &plane);

  dmnsn_object *cylinder = dmnsn_new_cylinder(0.1, 0.1, false);
  cylinder->trans =
    dmnsn_matrix_mul(
      dmnsn_rotation_matrix(dmnsn_new_vector(dmnsn_radians(-45.0), 0.0, 0.0)),
      dmnsn_scale_matrix(dmnsn_new_vector(1.0, 1.25, 1.0))
    );
  cylinder->texture = dmnsn_new_texture();
  cylinder->texture->pigment = dmnsn_new_solid_pigment(dmnsn_red);
  dmnsn_array_push(scene->objects, &cylinder);

  dmnsn_object *cone = dmnsn_new_cylinder(0.1, 0.0, true);
  cone->trans =
    dmnsn_matrix_mul(
      dmnsn_rotation_matrix(dmnsn_new_vector(dmnsn_radians(-45.0), 0.0, 0.0)),
      dmnsn_matrix_mul(
        dmnsn_translation_matrix(dmnsn_new_vector(0.0, 1.375, 0.0)),
        dmnsn_scale_matrix(dmnsn_new_vector(1.0, 0.125, 1.0))
      )
    );
  cone->texture = dmnsn_new_texture();
  cone->texture->pigment = dmnsn_new_solid_pigment(dmnsn_red);
  dmnsn_array_push(scene->objects, &cone);

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
