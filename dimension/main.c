/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of Dimension.                                       *
 *                                                                       *
 * Dimension is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU General Public License as published by the *
 * Free Software Foundation; either version 3 of the License, or (at     *
 * your option) any later version.                                       *
 *                                                                       *
 * Dimension is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "tokenize.h"
#include "parse.h"
#include "realize.h"
#include "progressbar.h"
#include "../libdimension/dimension.h"
#include <stdlib.h>
#include <getopt.h>

static const char *output = NULL, *input = NULL;
static int tokenize = 0, parse = 0;

int
main(int argc, char **argv) {
  FILE *input_file, *output_file;

  /*
   * Parse the command-line options
   */

  static struct option long_options[] = {
    { "output",   required_argument, NULL,      'o' },
    { "input",    required_argument, NULL,      'i' },
    { "tokenize", no_argument,       &tokenize, 1   },
    { "parse",    no_argument,       &parse,    1   },
    { 0,          0,                 0,         0   }
  };
  int opt, opt_index;

  while (1) {
    opt = getopt_long(argc, argv, "o:i:", long_options, &opt_index);

    if (opt == -1)
      break;

    switch (opt) {
    case 0:
      /* Option set a flag - do nothing here */
      break;

    case 'o':
      if (output) {
        dmnsn_error(DMNSN_SEVERITY_HIGH, "--output specified more than once.");
      } else {
        output = optarg;
      }
      break;

    case 'i':
      if (input) {
        dmnsn_error(DMNSN_SEVERITY_HIGH, "--input specified more than once.");
      } else {
        input = optarg;
      }
      break;

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Error parsing command line.");
    };
  }

  if (optind == argc - 1) {
    if (input) {
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Multiple input files specified.");
    } else {
      input = argv[optind];
    }
  } else if (optind < argc) {
    dmnsn_error(DMNSN_SEVERITY_HIGH,
                "Invalid extranious command line options.");
  }

  if (!output && !(tokenize || parse)) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "No output file specified.");
  }
  if (!input) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "No input file specified.");
  }

  /* Open the input file */
  input_file = fopen(input, "r");
  if (!input_file) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't open input file.");
  }

  /* Tokenize the input file */
  printf("Tokenizing input...\n");
  dmnsn_array *tokens = dmnsn_tokenize(input, input_file);
  if (!tokens) {
    fclose(input_file);
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Error tokenizing input file.");
  }
  fclose(input_file);

  /* Debugging option - output the list of tokens as an S-expression */
  if (tokenize) {
    dmnsn_print_token_sexpr(stdout, tokens);

    if (!parse) {
      dmnsn_delete_tokens(tokens);
      return EXIT_SUCCESS;
    }
  }

  /* Parse the input */
  printf("Parsing input...\n");
  dmnsn_array *astree = dmnsn_parse(tokens);
  if (!astree) {
    dmnsn_delete_tokens(tokens);
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Error parsing input file.");
  }
  dmnsn_delete_tokens(tokens);

  /* Debugging option - output the abstract syntax tree as an S-expression */
  if (parse) {
    dmnsn_print_astree_sexpr(stdout, astree);
    dmnsn_delete_astree(astree);
    return EXIT_SUCCESS;
  }

  /* Realize the input */
  printf("Generating scene...\n");
  dmnsn_scene *scene = dmnsn_realize(astree);
  if (!scene) {
    dmnsn_delete_astree(astree);
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Error realizing input file.");
  }
  dmnsn_delete_astree(astree);

  /*
   * Now we render the scene
   */

  if (dmnsn_png_optimize_canvas(scene->canvas) != 0) {
    fprintf(stderr, "WARNING: Couldn't optimize canvas for PNG\n");
  }

  dmnsn_progress *render_progress = dmnsn_raytrace_scene_async(scene);
  if (!render_progress) {
    dmnsn_delete_scene(scene);
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Error starting render.");
  }

  dmnsn_progressbar("Rendering scene: ", render_progress);

  if (dmnsn_finish_progress(render_progress) != 0) {
    dmnsn_delete_scene(scene);
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Error rendering scene.");
  }

  /* Open the output file */
  output_file = fopen(output, "wb");
  if (!output_file) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't open output file.");
  }

  dmnsn_progress *output_progress
    = dmnsn_png_write_canvas_async(scene->canvas, output_file);
  if (!output_progress) {
    fclose(output_file);
    dmnsn_delete_scene(scene);
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't initialize PNG export.");
  }

  dmnsn_progressbar("Writing PNG:     ", output_progress);

  if (dmnsn_finish_progress(output_progress) != 0) {
    fclose(output_file);
    dmnsn_delete_scene(scene);
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't write output.");
  }
  fclose(output_file);

  dmnsn_delete_scene(scene);
  return EXIT_SUCCESS;
}
