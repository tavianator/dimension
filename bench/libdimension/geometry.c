/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
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

#include <dimension.h>
#include <sandglass.h>
#include <stdlib.h>

int
main()
{
  dmnsn_vector vector, vector2;
  dmnsn_matrix matrix, matrix2;
  dmnsn_line line;
  double result;

  sandglass_t sandglass;
  sandglass_attributes_t attr = { SANDGLASS_MONOTONIC, SANDGLASS_REALTICKS };

  if (sandglass_create(&sandglass, &attr, &attr) != 0) {
    perror("sandglass_create()");
    return EXIT_FAILURE;
  }

  /* dmnsn_vector_construct() */
  sandglass_bench(&sandglass, {
    vector = dmnsn_vector_construct(1.0, 2.0, 3.0);
  });
  printf("dmnsn_vector_construct(): %ld\n", sandglass.grains);

  /* dmnsn_matrix_construct() */
  sandglass_bench(&sandglass, {
    matrix = dmnsn_matrix_construct(1.0, 1.0, 0.0, 0.0,
                                    1.0, 1.0, 1.0, 0.0,
                                    0.0, 1.0, 1.0, 0.0,
                                    0.0, 0.0, 0.0, 1.0);
  });
  printf("dmnsn_matrix_construct(): %ld\n", sandglass.grains);

  /* dmnsn_identity_matrix() */
  sandglass_bench(&sandglass, {
    matrix = dmnsn_identity_matrix();
  });
  printf("dmnsn_identity_matrix(): %ld\n", sandglass.grains);

  /* dmnsn_scale_matrix() */
  sandglass_bench(&sandglass, {
    matrix = dmnsn_scale_matrix(vector);
  });
  printf("dmnsn_scale_matrix(): %ld\n", sandglass.grains);

  /* dmnsn_identity_matrix() */
  sandglass_bench(&sandglass, {
    matrix = dmnsn_translation_matrix(vector);
  });
  printf("dmnsn_translation_matrix(): %ld\n", sandglass.grains);

  /* dmnsn_rotation_matrix() */
  sandglass_bench(&sandglass, {
    matrix = dmnsn_rotation_matrix(vector);
  });
  printf("dmnsn_rotation_matrix(): %ld\n", sandglass.grains);

  /* dmnsn_line_construct() */
  vector2 = dmnsn_vector_construct(3.0, 2.0, 1.0);
  sandglass_bench(&sandglass, {
    line = dmnsn_line_construct(vector, vector2);
  });
  printf("dmnsn_line_construct(): %ld\n", sandglass.grains);

  /* dmnsn_vector_add() */
  sandglass_bench(&sandglass, {
    vector = dmnsn_vector_add(vector, vector2);
  });
  printf("dmnsn_vector_add(): %ld\n", sandglass.grains);

  /* dmnsn_vector_sub() */
  sandglass_bench(&sandglass, {
    vector = dmnsn_vector_sub(vector, vector2);
  });
  printf("dmnsn_vector_sub(): %ld\n", sandglass.grains);

  /* dmnsn_vector_mul() */
  sandglass_bench(&sandglass, {
    vector = dmnsn_vector_mul(2.0, vector);
  });
  printf("dmnsn_vector_mul(): %ld\n", sandglass.grains);

  /* dmnsn_vector_div() */
  sandglass_bench(&sandglass, {
    vector = dmnsn_vector_div(vector, 2.0);
  });
  printf("dmnsn_vector_div(): %ld\n", sandglass.grains);

  /* dmnsn_vector_cross() */
  sandglass_bench(&sandglass, {
    vector = dmnsn_vector_cross(vector, vector2);
  });
  printf("dmnsn_vector_cross(): %ld\n", sandglass.grains);

  /* dmnsn_vector_dot() */
  sandglass_bench(&sandglass, {
    result = dmnsn_vector_dot(vector, vector2);
  });
  printf("dmnsn_vector_dot(): %ld\n", sandglass.grains);

  /* dmnsn_vector_norm() */
  sandglass_bench(&sandglass, {
    result = dmnsn_vector_norm(vector);
  });
  printf("dmnsn_vector_norm(): %ld\n", sandglass.grains);

  /* dmnsn_vector_normalize() */
  sandglass_bench(&sandglass, {
    vector = dmnsn_vector_normalize(vector);
  });
  printf("dmnsn_vector_normalize(): %ld\n", sandglass.grains);

  /* dmnsn_matrix_inverse() */
  sandglass_bench(&sandglass, {
    matrix = dmnsn_matrix_inverse(matrix);
  });
  printf("dmnsn_matrix_inverse(): %ld\n", sandglass.grains);

  /* dmnsn_matrix_inverse(HARD) */
  matrix2 = dmnsn_matrix_construct(1.0, 1.0, 0.0, 0.0,
                                   1.0, 1.0, 1.0, 0.0,
                                   0.0, 1.0, 1.0, 0.0,
                                   0.0, 0.0, 0.0, 1.0);
  sandglass_bench(&sandglass, {
    matrix2 = dmnsn_matrix_inverse(matrix2);
  });
  printf("dmnsn_matrix_inverse(HARD): %ld\n", sandglass.grains);

  /* dmnsn_matrix_mul() */
  sandglass_bench(&sandglass, {
    matrix = dmnsn_matrix_mul(matrix, matrix2);
  });
  printf("dmnsn_matrix_mul(): %ld\n", sandglass.grains);

  /* dmnsn_matrix_vector_mul() */
  sandglass_bench(&sandglass, {
    vector = dmnsn_matrix_vector_mul(matrix, vector);
  });
  printf("dmnsn_matrix_vector_mul(): %ld\n", sandglass.grains);

  /* dmnsn_matrix_line_mul() */
  sandglass_bench(&sandglass, {
    line = dmnsn_matrix_line_mul(matrix, line);
  });
  printf("dmnsn_matrix_line_mul(): %ld\n", sandglass.grains);

  /* dmnsn_line_point() */
  sandglass_bench(&sandglass, {
    vector = dmnsn_line_point(line, result);
  });
  printf("dmnsn_line_point(): %ld\n", sandglass.grains);

  /* dmnsn_line_index() */
  sandglass_bench(&sandglass, {
    result = dmnsn_line_index(line, vector);
  });
  printf("dmnsn_line_index(): %ld\n", sandglass.grains);

  return EXIT_SUCCESS;
}
