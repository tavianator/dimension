/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
#include "dimension.h"
#include <libgen.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static char *output = NULL, *input = NULL;
static bool free_output = false;
static int width = 640, height = 480;
static int nthreads = 0;
static dmnsn_quality quality = DMNSN_RENDER_FULL;
static int tokenize = 0, parse = 0;

static void print_usage(FILE *file, const char *arg0);
static void print_version(FILE *file);

int
main(int argc, char **argv)
{
  /*
   * Parse the command-line options
   */

  /* Long-only option codes */
  enum {
    DMNSN_OPT_VERSION = 256,
    DMNSN_OPT_THREADS,
    DMNSN_OPT_QUALITY,
    DMNSN_OPT_STRICT
  };

  static struct option long_options[] = {
    { "help",       no_argument,       NULL,      '?'                  },
    { "version",    no_argument,       NULL,      DMNSN_OPT_VERSION    },
    { "output",     required_argument, NULL,      'o'                  },
    { "width",      required_argument, NULL,      'w'                  },
    { "height",     required_argument, NULL,      'h'                  },
    { "threads",    required_argument, NULL,      DMNSN_OPT_THREADS    },
    { "quality",    required_argument, NULL,      DMNSN_OPT_QUALITY    },
    { "strict",     no_argument,       NULL,      DMNSN_OPT_STRICT     },
    { "tokenize",   no_argument,       &tokenize, 1                    },
    { "parse",      no_argument,       &parse,    1                    },
    { 0,            0,                 0,         0                    }
  };

  int opt, opt_index;

  while (1) {
    opt = getopt_long(argc, argv, "?o:w:h:", long_options, &opt_index);

    if (opt == -1)
      break;

    switch (opt) {
    case 0:
      /* Option set a flag - do nothing here */
      break;

    case '?':
      print_usage(stdout, argv[0]);
      /* '?' is returned as an error code too */
      if (optopt == 0) {
        return EXIT_SUCCESS;
      } else {
        return EXIT_FAILURE;
      }
    case DMNSN_OPT_VERSION:
      print_version(stdout);
      return EXIT_SUCCESS;

    case 'o':
      if (output) {
        fprintf(stderr, "--output specified more than once!\n");
        print_usage(stderr, argv[0]);
        return EXIT_FAILURE;
      } else {
        output = optarg;
      }
      break;

    case 'w':
      {
        char *endptr;
        width = strtol(optarg, &endptr, 10);
        if (*endptr != '\0' || endptr == optarg || width <= 0) {
          fprintf(stderr, "Invalid argument to --width!\n");
          print_usage(stderr, argv[0]);
          return EXIT_FAILURE;
        }
        break;
      }
    case 'h':
      {
        char *endptr;
        height = strtol(optarg, &endptr, 10);
        if (*endptr != '\0' || endptr == optarg || height <= 0) {
          fprintf(stderr, "Invalid argument to --height!\n");
          print_usage(stderr, argv[0]);
          return EXIT_FAILURE;
        }
        break;
      }

    case DMNSN_OPT_THREADS:
      {
        char *endptr;
        nthreads = strtol(optarg, &endptr, 10);
        if (*endptr != '\0' || endptr == optarg || nthreads <= 0) {
          fprintf(stderr, "Invalid argument to --threads!\n");
          print_usage(stderr, argv[0]);
          return EXIT_FAILURE;
        }
        break;
      }
    case DMNSN_OPT_QUALITY:
      {
        char *endptr;
        quality = strtoul(optarg, &endptr, 0);
        if (*endptr != '\0' || endptr == optarg) {
          fprintf(stderr, "Invalid argument to --quality!\n");
          print_usage(stderr, argv[0]);
          return EXIT_FAILURE;
        }
        break;
      }
    case DMNSN_OPT_STRICT:
      dmnsn_die_on_warnings(true);
      break;

    default:
      fprintf(stderr, "Invalid command line option!\n");
      print_usage(stderr, argv[0]);
      return EXIT_FAILURE;
    };
  }

  if (optind == argc - 1) {
    input = argv[optind];
  } else if (optind < argc) {
    fprintf(stderr, "Invalid extraneous command line options!\n");
    print_usage(stderr, argv[0]);
    return EXIT_FAILURE;
  }

  if (!input) {
    fprintf(stderr, "No input file specified!\n");
    print_usage(stderr, argv[0]);
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
  dmnsn_declare_symbol(symtable, "image_width", dmnsn_new_ast_integer(width));
  dmnsn_declare_symbol(symtable, "image_height", dmnsn_new_ast_integer(height));

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
  /* Time the parser */
  dmnsn_timer *parse_timer = dmnsn_new_timer();
    dmnsn_scene *scene = dmnsn_realize(input_file, symtable);
    if (!scene) {
      fprintf(stderr, "Error realizing input file!\n");
      dmnsn_delete_timer(parse_timer);
      dmnsn_delete_symbol_table(symtable);
      fclose(input_file);
      return EXIT_FAILURE;
    }
  dmnsn_complete_timer(parse_timer);

  dmnsn_delete_symbol_table(symtable);
  fclose(input_file);

  /* Allocate a canvas */
  scene->canvas = dmnsn_new_canvas(width, height);

  /* Set the new number of threads if --threads changed it */
  if (nthreads)
    scene->nthreads = nthreads;

  /* Set the render quality */
  scene->quality = quality;

  /*
   * Now we render the scene
   */

  /* Generate a default output filename by replacing the extension of the
     basename of the input file with ".png" */
  if (!output) {
    char *input_copy = dmnsn_strdup(input);
    char *base = basename(input_copy);
    char *ext = strrchr(base, '.');
    if (ext) {
      output = dmnsn_malloc(ext - base + 5);
      strncpy(output, base, ext - base + 5);
      ext = output + (ext - base);
    } else {
      size_t len = strlen(base);
      output = dmnsn_malloc(len + 5);
      strcpy(output, base);
      ext = output + len;
    }
    dmnsn_free(input_copy);
    strcpy(ext, ".png");
    free_output = true;
  }

  /* Open the output file */
  FILE *output_file = fopen(output, "wb");
  if (free_output)
    dmnsn_free(output);
  if (!output_file) {
    dmnsn_delete_timer(parse_timer);
    fprintf(stderr, "Couldn't open output file!");
    return EXIT_FAILURE;
  }

  if (dmnsn_png_optimize_canvas(scene->canvas) != 0) {
    fprintf(stderr, "WARNING: Couldn't optimize canvas for PNG\n");
  }

  dmnsn_progress *render_progress = dmnsn_raytrace_scene_async(scene);
  dmnsn_progressbar(scene->nthreads > 1
                    ? "Rendering scene with %u threads"
                    : "Rendering scene with %u thread",
                    render_progress,
                    scene->nthreads);

  if (dmnsn_finish_progress(render_progress) != 0) {
    fclose(output_file);
    dmnsn_delete_timer(parse_timer);
    dmnsn_delete_scene(scene);
    fprintf(stderr, "Error rendering scene!\n");
    return EXIT_FAILURE;
  }

  /* Time the export */
  dmnsn_timer *export_timer = dmnsn_new_timer();
    dmnsn_progress *output_progress
      = dmnsn_png_write_canvas_async(scene->canvas, output_file);
    if (!output_progress) {
      fclose(output_file);
      dmnsn_delete_timer(parse_timer);
      dmnsn_delete_scene(scene);
      fprintf(stderr, "Couldn't initialize PNG export!\n");
      return EXIT_FAILURE;
    }

    dmnsn_progressbar("Writing PNG", output_progress);

    if (dmnsn_finish_progress(output_progress) != 0) {
      dmnsn_delete_timer(export_timer);
      dmnsn_delete_timer(parse_timer);
      fclose(output_file);
      dmnsn_delete_scene(scene);
      fprintf(stderr, "Couldn't write output!\n");
      return EXIT_FAILURE;
    }
    fclose(output_file);
  dmnsn_complete_timer(export_timer);

  printf("\n"
         "  Parse time:    " DMNSN_TIMER_FORMAT "\n"
         "  Bounding time: " DMNSN_TIMER_FORMAT "\n"
         "  Render time:   " DMNSN_TIMER_FORMAT "\n"
         "  Export time:   " DMNSN_TIMER_FORMAT "\n",
         DMNSN_TIMER_PRINTF(parse_timer),
         DMNSN_TIMER_PRINTF(scene->bounding_timer),
         DMNSN_TIMER_PRINTF(scene->render_timer),
         DMNSN_TIMER_PRINTF(export_timer));

  dmnsn_delete_timer(export_timer);
  dmnsn_delete_timer(parse_timer);
  dmnsn_delete_scene(scene);
  return EXIT_SUCCESS;
}

static void
print_usage(FILE *file, const char *arg0)
{
  fprintf(file,
          "Usage: %s [OPTIONS...] INPUT_FILE\n\n"
          " Main options:\n"
          "  -?, --help          show this help\n"
          "  --version           show the version of %s\n"
          "  -o, --output=FILE   output to FILE\n\n"
          " Output options:\n"
          "  -w, --width=WIDTH   set canvas width to WIDTH (default 640)\n"
          "  -h, --height=HEIGHT set canvas height to HEIGHT (default 480)\n\n"
          " Rendering options:\n"
          "  --threads=THREADS   render with THREADS parallel threads\n"
          "  --quality=QUALITY   use the quality setting QUALITY\n"
          "  --strict            treat warnings as errors\n"
          " Debugging options:\n"
          "  --tokenize          tokenize the input and print the token list\n"
          "  --parse             parse the input and print the abstract syntax"
          " tree\n\n"
          " Special options:\n"
          "  --                  indicates the end of the argument list\n\n"
          "%s\n"
          "%s\n"
          "Copyright (C) 2009-2010 Tavian Barnes, <%s>\n"
          "Licensed under the GNU General Public License\n",
          arg0,
          PACKAGE_NAME,
          PACKAGE_STRING,
          PACKAGE_URL,
          PACKAGE_BUGREPORT);
}

static void
print_version(FILE *file)
{
  fprintf(file, "%s\n", PACKAGE_STRING);
}
