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

typedef enum {
  /* Punctuation */
  DMNSN_LBRACE,   /* { */
  DMNSN_RBRACE,   /* } */
  DMNSN_LPAREN,   /* ( */
  DMNSN_RPAREN,   /* ) */
  DMNSN_LBRACKET, /* [ */
  DMNSN_RBRACKET, /* ] */
  DMNSN_LT,       /* < */
  DMNSN_GT,       /* > */
  DMNSN_PLUS,     /* + */
  DMNSN_MINUS,    /* - */
  DMNSN_STAR,     /* * */
  DMNSN_SLASH,    /* / */
  DMNSN_COMMA,    /* , */

  /* Numeric values */
  DMNSN_INT,
  DMNSN_FLOAT,
} dmnsn_token_type;

typedef struct dmnsn_token dmnsn_token;

struct dmnsn_token {
  dmnsn_token_type type;
  char *value;

  /* Line and column numbers from source code */
  unsigned int line, col;
};

dmnsn_array *dmnsn_tokenize(FILE *file);
void dmnsn_delete_tokens(dmnsn_array *tokens);
void dmnsn_print_token_sexpr(FILE *file, dmnsn_array *tokens);
