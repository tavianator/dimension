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

#include "../libdimension/dimension.h"
#include <stdlib.h>
#include <getopt.h>

const char *output = NULL, *input = NULL;
int tokenize = 0;

int
main(int argc, char **argv) {
  /* Parse the command-line options */

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

  if (!output) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "No output file specified.");
  }
  if (!input) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "No input file specified.");
  }

  return EXIT_SUCCESS;
}
