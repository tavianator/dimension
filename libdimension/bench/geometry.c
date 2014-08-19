/*************************************************************************
 * Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Benchmark Suite.                   *
 *                                                                       *
 * The Dimension Benchmark Suite is free software; you can redistribute  *
 * it and/or modify it under the terms of the GNU General Public License *
 * as published by the Free Software Foundation; either version 3 of the *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Benchmark Suite is distributed in the hope that it will *
 * be useful, but WITHOUT ANY WARRANTY; without even the implied         *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See *
 * the GNU General Public License for more details.                      *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "dimension/math.h"
#include <sandglass.h>
#include <stdlib.h>
#include <stdio.h>

int
main(void)
{
  dmnsn_vector vector, vector2;
  dmnsn_matrix matrix, matrix2;
  dmnsn_ray ray;
  dmnsn_aabb box;
  double result;

  sandglass_t sandglass;
  if (sandglass_init_monotonic(&sandglass, SANDGLASS_CPUTIME) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  // dmnsn_new_vector()
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_new_vector(1.0, 2.0, 3.0);
  });
  printf("dmnsn_new_vector(): %ld\n", sandglass.grains);

  // dmnsn_new_matrix()
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_new_matrix(1.0, 1.0, 0.0, 0.0,
                              1.0, 1.0, 1.0, 0.0,
                              0.0, 1.0, 1.0, 0.0);
  });
  printf("dmnsn_new_matrix(): %ld\n", sandglass.grains);

  // dmnsn_identity_matrix()
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_identity_matrix();
  });
  printf("dmnsn_identity_matrix(): %ld\n", sandglass.grains);

  // dmnsn_scale_matrix()
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_scale_matrix(vector);
  });
  printf("dmnsn_scale_matrix(): %ld\n", sandglass.grains);

  // dmnsn_identity_matrix()
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_translation_matrix(vector);
  });
  printf("dmnsn_translation_matrix(): %ld\n", sandglass.grains);

  // dmnsn_rotation_matrix()
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_rotation_matrix(vector);
  });
  printf("dmnsn_rotation_matrix(): %ld\n", sandglass.grains);

  // dmnsn_new_ray()
  vector2 = dmnsn_new_vector(3.0, 2.0, 1.0);
  sandglass_bench_fine(&sandglass, {
    ray = dmnsn_new_ray(vector, vector2);
  });
  printf("dmnsn_new_ray(): %ld\n", sandglass.grains);

  // dmnsn_new_aabb()
  vector2 = dmnsn_new_vector(3.0, 4.0, 5.0);
  sandglass_bench_fine(&sandglass, {
    box = dmnsn_new_aabb(vector, vector2);
  });
  printf("dmnsn_new_aabb(): %ld\n", sandglass.grains);

  // dmnsn_vector_add()
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_vector_add(vector, vector2);
  });
  printf("dmnsn_vector_add(): %ld\n", sandglass.grains);

  // dmnsn_vector_sub()
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_vector_sub(vector, vector2);
  });
  printf("dmnsn_vector_sub(): %ld\n", sandglass.grains);

  // dmnsn_vector_mul()
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_vector_mul(2.0, vector);
  });
  printf("dmnsn_vector_mul(): %ld\n", sandglass.grains);

  // dmnsn_vector_div()
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_vector_div(vector, 2.0);
  });
  printf("dmnsn_vector_div(): %ld\n", sandglass.grains);

  // dmnsn_vector_cross()
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_vector_cross(vector, vector2);
  });
  printf("dmnsn_vector_cross(): %ld\n", sandglass.grains);

  // dmnsn_vector_dot()
  sandglass_bench_fine(&sandglass, {
    result = dmnsn_vector_dot(vector, vector2);
  });
  printf("dmnsn_vector_dot(): %ld\n", sandglass.grains);

  // dmnsn_vector_norm()
  sandglass_bench_fine(&sandglass, {
    result = dmnsn_vector_norm(vector);
  });
  printf("dmnsn_vector_norm(): %ld\n", sandglass.grains);

  // dmnsn_vector_normalized()
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_vector_normalized(vector);
  });
  printf("dmnsn_vector_normalized(): %ld\n", sandglass.grains);

  // dmnsn_matrix_inverse()
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_matrix_inverse(matrix);
  });
  printf("dmnsn_matrix_inverse(): %ld\n", sandglass.grains);

  // dmnsn_matrix_inverse(HARD)
  matrix2 = dmnsn_new_matrix(1.0, 1.0, 0.0, 0.0,
                             1.0, 1.0, 1.0, 0.0,
                             0.0, 1.0, 1.0, 0.0);
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_matrix_inverse(matrix2);
  });
  printf("dmnsn_matrix_inverse(HARD): %ld\n", sandglass.grains);

  // dmnsn_matrix_mul()
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_matrix_mul(matrix, matrix2);
  });
  printf("dmnsn_matrix_mul(): %ld\n", sandglass.grains);

  // dmnsn_transform_point()
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_transform_point(matrix, vector);
  });
  printf("dmnsn_transform_point(): %ld\n", sandglass.grains);

  // dmnsn_transform_direction()
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_transform_direction(matrix, vector);
  });
  printf("dmnsn_transform_direction(): %ld\n", sandglass.grains);

  // dmnsn_transform_normal()
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_transform_normal(matrix, vector);
  });
  printf("dmnsn_transform_normal(): %ld\n", sandglass.grains);

  // dmnsn_transform_ray()
  sandglass_bench_fine(&sandglass, {
    ray = dmnsn_transform_ray(matrix, ray);
  });
  printf("dmnsn_transform_ray(): %ld\n", sandglass.grains);

  // dmnsn_transform_aabb()
  sandglass_bench_fine(&sandglass, {
    box = dmnsn_transform_aabb(matrix, box);
  });
  printf("dmnsn_transform_aabb(): %ld\n", sandglass.grains);

  // dmnsn_ray_point()
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_ray_point(ray, result);
  });
  printf("dmnsn_ray_point(): %ld\n", sandglass.grains);

  return EXIT_SUCCESS;
}
