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
#include "../libdimension/dimension.h"
#include <stdlib.h>
#include <getopt.h>

static const char *output = NULL, *input = NULL;
static int tokenize = 0;

int
main(int argc, char **argv) {
  dmnsn_array *tokens;
  FILE *input_file, *output_file;

  /*
   * Parse the command-line options
   */

  static struct option long_options[] = {
    { "output",   required_argument, NULL,      'o' },
    { "input",    required_argument, NULL,      'i' },
    { "tokenize", no_argument,       &tokenize, 1   },
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
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Invalid extranious command line options.");
  }

  if (!output && !tokenize) {
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
  tokens = dmnsn_tokenize(input_file);
  if (!tokens) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Error tokenizing input file.");
  }

  /* Debugging option - output the list of tokens as an S-expression */
  if (tokenize) {
    dmnsn_print_token_sexpr(stdout, tokens);
    dmnsn_delete_tokens(tokens);
    fclose(input_file);
    return EXIT_SUCCESS;
  }

  /*
   * Now we render the scene
   */

  /* Open the output file */
  output_file = fopen(output, "wb");
  if (!output_file) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't open output file.");
  }

  /* Clean up and exit! */
  dmnsn_delete_tokens(tokens);
  fclose(output_file);
  fclose(input_file);
  return EXIT_SUCCESS;
}
