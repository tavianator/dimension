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

#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "../libdimension/dimension.h"
#include "grammar.h"

typedef enum yytokentype dmnsn_token_type;

typedef struct dmnsn_token dmnsn_token;

struct dmnsn_token {
  dmnsn_token_type type;
  char *value;

  /* File name, and line and column numbers from source code */
  const char *filename;
  unsigned int line, col;
};

/* For debugging */
dmnsn_array *dmnsn_tokenize(FILE *file, const char *filename);

/* Token destruction */
void dmnsn_delete_token(dmnsn_token token);
void dmnsn_delete_tokens(dmnsn_array *tokens);

/* Print an S-expression of a list of tokens to `file' */
void dmnsn_print_token_sexpr(FILE *file, const dmnsn_array *tokens);

/* Returns a readable name for a token type (ex. DMNSN_T_FLOAT -> float) */
const char *dmnsn_token_string(dmnsn_token_type token_type);

#endif /* TOKENIZE_H */
