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
  DMNSN_T_LBRACE,   /* { */
  DMNSN_T_RBRACE,   /* } */
  DMNSN_T_LPAREN,   /* ( */
  DMNSN_T_RPAREN,   /* ) */
  DMNSN_T_LBRACKET, /* [ */
  DMNSN_T_RBRACKET, /* ] */
  DMNSN_T_LT,       /* < */
  DMNSN_T_GT,       /* > */
  DMNSN_T_PLUS,     /* + */
  DMNSN_T_MINUS,    /* - */
  DMNSN_T_STAR,     /* * */
  DMNSN_T_SLASH,    /* / */
  DMNSN_T_COMMA,    /* , */

  /* Numeric values */
  DMNSN_T_INT,
  DMNSN_T_FLOAT,

  /* Keywords */
  DMNSN_T_CAMERA,
  DMNSN_T_COLOR,
  DMNSN_T_SPHERE,
  DMNSN_T_BOX,

  /* Directives (#declare, etc.) */
  DMNSN_T_BREAK,
  DMNSN_T_CASE,
  DMNSN_T_DEBUG,
  DMNSN_T_DECLARE,
  DMNSN_T_DEFAULT,
  DMNSN_T_ELSE,
  DMNSN_T_END,
  DMNSN_T_ERROR,
  DMNSN_T_FCLOSE,
  DMNSN_T_FOPEN,
  DMNSN_T_IF,
  DMNSN_T_IFDEF,
  DMNSN_T_IFNDEF,
  DMNSN_T_INCLUDE, /* Only used internally */
  DMNSN_T_LOCAL,
  DMNSN_T_MACRO,
  DMNSN_T_RANGE,
  DMNSN_T_READ,
  DMNSN_T_RENDER,
  DMNSN_T_STATISTICS,
  DMNSN_T_SWITCH,
  DMNSN_T_UNDEF,
  DMNSN_T_VERSION,
  DMNSN_T_WARNING,
  DMNSN_T_WHILE,
  DMNSN_T_WRITE,

  /* Identifiers */
  DMNSN_T_IDENTIFIER,

  /* Strings */
  DMNSN_T_STRING,
} dmnsn_token_type;

typedef struct dmnsn_token dmnsn_token;

struct dmnsn_token {
  dmnsn_token_type type;
  char *value;

  /* File name, and line and column numbers from source code */
  char *filename;
  unsigned int line, col;
};

dmnsn_array *dmnsn_tokenize(const char *filename, FILE *file);
void dmnsn_delete_tokens(dmnsn_array *tokens);
void dmnsn_print_token_sexpr(FILE *file, dmnsn_array *tokens);
