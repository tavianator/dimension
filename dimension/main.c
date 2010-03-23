/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
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
#include <libgen.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static char *output = NULL, *input = NULL;
static bool free_output = false;
static unsigned int width = 640, height = 480;
static unsigned int nthreads = 0;
static int tokenize = 0, parse = 0;

int
main(int argc, char **argv) {
  /*
   * Parse the command-line options
   */

  /* Long-only option codes */
  enum {
    DMNSN_OPT_THREADS = 256
  };

  static struct option long_options[] = {
    { "output",   required_argument, NULL,      'o'               },
    { "input",    required_argument, NULL,      'i'               },
    { "width",    required_argument, NULL,      'w'               },
    { "height",   required_argument, NULL,      'h'               },
    { "threads",  required_argument, NULL,      DMNSN_OPT_THREADS },
    { "tokenize", no_argument,       &tokenize, 1                 },
    { "parse",    no_argument,       &parse,    1                 },
    { 0,          0,                 0,         0                 }
  };

  int opt, opt_index;

  while (1) {
    opt = getopt_long(argc, argv, "o:i:w:h:", long_options, &opt_index);

    if (opt == -1)
      break;

    switch (opt) {
    case 0:
      /* Option set a flag - do nothing here */
      break;

    case 'o':
      if (output) {
        fprintf(stderr, "--output specified more than once!\n");
        return EXIT_FAILURE;
      } else {
        output = optarg;
      }
      break;

    case 'i':
      if (input) {
        fprintf(stderr, "--input specified more than once!\n");
        return EXIT_FAILURE;
      } else {
        input = optarg;
      }
      break;

    case 'w':
      {
        dmnsn_assert(optarg, "NULL argument.");

        char *endptr;
        width = strtoul(optarg, &endptr, 10);
        if (*endptr != '\0' || endptr == optarg) {
          fprintf(stderr, "Invalid argument to --width!\n");
          return EXIT_FAILURE;
        }
        break;
      }
    case 'h':
      {
        dmnsn_assert(optarg, "NULL argument.");

        char *endptr;
        height = strtoul(optarg, &endptr, 10);
        if (*endptr != '\0' || endptr == optarg) {
          fprintf(stderr, "Invalid argument to --height!\n");
          return EXIT_FAILURE;
        }
        break;
      }

    case DMNSN_OPT_THREADS:
      {
        dmnsn_assert(optarg, "NULL argument.");

        char *endptr;
        nthreads = strtoul(optarg, &endptr, 10);
        if (*endptr != '\0' || endptr == optarg) {
          fprintf(stderr, "Invalid argument to --threads!\n");
          return EXIT_FAILURE;
        }
        break;
      }

    default:
      fprintf(stderr, "Invalid command line option!\n");
      return EXIT_FAILURE;
    };
  }

  if (optind == argc - 1) {
    if (input) {
      fprintf(stderr, "Multiple input files specified!\n");
      return EXIT_FAILURE;
    } else {
      input = argv[optind];
    }
  } else if (optind < argc) {
    fprintf(stderr, "Invalid extranious command line options!\n");
    return EXIT_FAILURE;
  }

  if (!input) {
    fprintf(stderr, "No input file specified!\n");
    return EXIT_FAILURE;
  }

  /*
   * Now do the work
   */

  /* Open the input file */
  FILE *input_file = fopen(input, "r");
  if (!input_file) {
    fprintf(stderr, "Couldn't open input file!\n");
    return EXIT_FAILURE;
  }

  /* Debugging option - output the list of tokens as an S-expression */
  if (tokenize) {
    dmnsn_array *tokens = dmnsn_tokenize(input_file, input);
    if (!tokens) {
      fclose(input_file);
      fprintf(stderr, "Error tokenizing input file!\n");
      return EXIT_FAILURE;
    }
    dmnsn_print_token_sexpr(stdout, tokens);
    rewind(input_file);
    dmnsn_delete_tokens(tokens);

    if (!parse) {
      fclose(input_file);
      return EXIT_SUCCESS;
    }
  }

  /* Construct the symbol table */
  dmnsn_symbol_table *symtable = dmnsn_new_symbol_table();
  dmnsn_declare_symbol(symtable, "$file", dmnsn_new_ast_string(input));
  dmnsn_declare_symbol(symtable, "version", dmnsn_new_ast_float(3.6));

  /* Debugging option - output the abstract syntax tree as an S-expression */
  if (parse) {
    dmnsn_astree *astree = dmnsn_parse(input_file, symtable);
    if (!astree) {
      dmnsn_delete_symbol_table(symtable);
      fclose(input_file);
      fprintf(stderr, "Error parsing input file!\n");
      return EXIT_FAILURE;
    }
    dmnsn_print_astree_sexpr(stdout, astree);
    dmnsn_delete_astree(astree);

    dmnsn_delete_symbol_table(symtable);
    fclose(input_file);
    return EXIT_SUCCESS;
  }

  /* Realize the input */
  printf("Parsing scene ...\n");
  dmnsn_scene *scene = dmnsn_realize(input_file, symtable);
  if (!scene) {
    fprintf(stderr, "Error realizing input file!\n");
    dmnsn_delete_symbol_table(symtable);
    fclose(input_file);
    return EXIT_FAILURE;
  }

  dmnsn_delete_symbol_table(symtable);
  fclose(input_file);

  /* Allocate a canvas */
  scene->canvas = dmnsn_new_canvas(width, height);
  if (!scene->canvas) {
    dmnsn_delete_scene(scene);
    fprintf(stderr, "Couldn't allocate canvas!\n");
    return EXIT_FAILURE;
  }

  /* Set the new number of threads if --threads changed it */
  if (nthreads)
    scene->nthreads = nthreads;

  /*
   * Now we render the scene
   */

  /* Generate a default output filename by replacing the extension of the
     basename of the input file with ".png" */
  if (!output) {
    char *input_copy = strdup(input);
    if (!input_copy) {
      fprintf(stderr, "Couldn't allocate space for output filename!\n");
      return EXIT_FAILURE;
    }

    char *base = basename(input_copy);
    char *ext = strrchr(base, '.');
    if (ext) {
      output = malloc(ext - base + 5);
      if (!output) {
        fprintf(stderr, "Couldn't allocate space for output filename!\n");
        return EXIT_FAILURE;
      }

      strncpy(output, base, ext - base + 5);
      ext = output + (ext - base);
    } else {
      size_t len = strlen(base);
      output = malloc(len + 5);
      if (!output) {
        fprintf(stderr, "Couldn't allocate space for output filename!\n");
        return EXIT_FAILURE;
      }
      strcpy(output, base);
      ext = output + len;
    }
    free(input_copy);
    strcpy(ext, ".png");
    free_output = true;
  }

  /* Open the output file */
  FILE *output_file = fopen(output, "wb");
  if (free_output)
    free(output);
  if (!output_file) {
    fprintf(stderr, "Couldn't open output file!");
    return EXIT_FAILURE;
  }

  if (dmnsn_png_optimize_canvas(scene->canvas) != 0) {
    fprintf(stderr, "WARNING: Couldn't optimize canvas for PNG\n");
  }

  dmnsn_progress *render_progress = dmnsn_raytrace_scene_async(scene);
  if (!render_progress) {
    dmnsn_delete_scene(scene);
    fprintf(stderr, "Error starting render!\n");
    return EXIT_FAILURE;
  }

  dmnsn_progressbar(scene->nthreads > 1
                    ? "Rendering scene with %u threads"
                    : "Rendering scene with %u thread",
                    render_progress,
                    scene->nthreads);

  if (dmnsn_finish_progress(render_progress) != 0) {
    dmnsn_delete_scene(scene);
    fprintf(stderr, "Error rendering scene!\n");
    return EXIT_FAILURE;
  }

  dmnsn_progress *output_progress
    = dmnsn_png_write_canvas_async(scene->canvas, output_file);
  if (!output_progress) {
    fclose(output_file);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "Couldn't initialize PNG export!\n");
    return EXIT_FAILURE;
  }

  dmnsn_progressbar("Writing PNG", output_progress);

  if (dmnsn_finish_progress(output_progress) != 0) {
    fclose(output_file);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "Couldn't write output!\n");
  }
  fclose(output_file);

  dmnsn_delete_scene(scene);
  return EXIT_SUCCESS;
}
