/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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

#include "dimension.h"
#include <sandglass.h>
#include <stdlib.h>

int
main(void)
{
  dmnsn_vector vector, vector2;
  dmnsn_matrix matrix, matrix2;
  dmnsn_line line;
  double result;

  sandglass_t sandglass;
  if (sandglass_init_monotonic(&sandglass, SANDGLASS_CPUTIME) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  /* dmnsn_new_vector() */
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_new_vector(1.0, 2.0, 3.0);
  });
  printf("dmnsn_new_vector(): %ld\n", sandglass.grains);

  /* dmnsn_matrix_construct() */
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_new_matrix(1.0, 1.0, 0.0, 0.0,
                              1.0, 1.0, 1.0, 0.0,
                              0.0, 1.0, 1.0, 0.0,
                              0.0, 0.0, 0.0, 1.0);
  });
  printf("dmnsn_new_matrix(): %ld\n", sandglass.grains);

  /* dmnsn_identity_matrix() */
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_identity_matrix();
  });
  printf("dmnsn_identity_matrix(): %ld\n", sandglass.grains);

  /* dmnsn_scale_matrix() */
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_scale_matrix(vector);
  });
  printf("dmnsn_scale_matrix(): %ld\n", sandglass.grains);

  /* dmnsn_identity_matrix() */
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_translation_matrix(vector);
  });
  printf("dmnsn_translation_matrix(): %ld\n", sandglass.grains);

  /* dmnsn_rotation_matrix() */
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_rotation_matrix(vector);
  });
  printf("dmnsn_rotation_matrix(): %ld\n", sandglass.grains);

  /* dmnsn_new_line() */
  vector2 = dmnsn_new_vector(3.0, 2.0, 1.0);
  sandglass_bench_fine(&sandglass, {
    line = dmnsn_new_line(vector, vector2);
  });
  printf("dmnsn_new_line(): %ld\n", sandglass.grains);

  /* dmnsn_vector_add() */
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_vector_add(vector, vector2);
  });
  printf("dmnsn_vector_add(): %ld\n", sandglass.grains);

  /* dmnsn_vector_sub() */
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_vector_sub(vector, vector2);
  });
  printf("dmnsn_vector_sub(): %ld\n", sandglass.grains);

  /* dmnsn_vector_mul() */
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_vector_mul(2.0, vector);
  });
  printf("dmnsn_vector_mul(): %ld\n", sandglass.grains);

  /* dmnsn_vector_div() */
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_vector_div(vector, 2.0);
  });
  printf("dmnsn_vector_div(): %ld\n", sandglass.grains);

  /* dmnsn_vector_cross() */
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_vector_cross(vector, vector2);
  });
  printf("dmnsn_vector_cross(): %ld\n", sandglass.grains);

  /* dmnsn_vector_dot() */
  sandglass_bench_fine(&sandglass, {
    result = dmnsn_vector_dot(vector, vector2);
  });
  printf("dmnsn_vector_dot(): %ld\n", sandglass.grains);

  /* dmnsn_vector_norm() */
  sandglass_bench_fine(&sandglass, {
    result = dmnsn_vector_norm(vector);
  });
  printf("dmnsn_vector_norm(): %ld\n", sandglass.grains);

  /* dmnsn_vector_normalize() */
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_vector_normalize(vector);
  });
  printf("dmnsn_vector_normalize(): %ld\n", sandglass.grains);

  /* dmnsn_matrix_inverse() */
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_matrix_inverse(matrix);
  });
  printf("dmnsn_matrix_inverse(): %ld\n", sandglass.grains);

  /* dmnsn_matrix_inverse(HARD) */
  matrix2 = dmnsn_new_matrix(1.0, 1.0, 0.0, 0.0,
                             1.0, 1.0, 1.0, 0.0,
                             0.0, 1.0, 1.0, 0.0,
                             0.0, 0.0, 0.0, 1.0);
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_matrix_inverse(matrix2);
  });
  printf("dmnsn_matrix_inverse(HARD): %ld\n", sandglass.grains);

  /* dmnsn_matrix_mul() */
  sandglass_bench_fine(&sandglass, {
    matrix = dmnsn_matrix_mul(matrix, matrix2);
  });
  printf("dmnsn_matrix_mul(): %ld\n", sandglass.grains);

  /* dmnsn_transform_vector() */
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_transform_vector(matrix, vector);
  });
  printf("dmnsn_transform_vector(): %ld\n", sandglass.grains);

  /* dmnsn_transform_line() */
  sandglass_bench_fine(&sandglass, {
    line = dmnsn_transform_line(matrix, line);
  });
  printf("dmnsn_transform_line(): %ld\n", sandglass.grains);

  /* dmnsn_line_point() */
  sandglass_bench_fine(&sandglass, {
    vector = dmnsn_line_point(line, result);
  });
  printf("dmnsn_line_point(): %ld\n", sandglass.grains);

  /* dmnsn_line_index() */
  sandglass_bench_fine(&sandglass, {
    result = dmnsn_line_index(line, vector);
  });
  printf("dmnsn_line_index(): %ld\n", sandglass.grains);

  return EXIT_SUCCESS;
}
