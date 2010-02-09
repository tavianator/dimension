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

#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "../libdimension/dimension.h"
#include "parse.h"

#define yytokentype dmnsn_yytokentype
#define YYSTYPE
#define YYLTYPE

#include "grammar.h"

#undef YYLTYPE
#undef YYSTYPE
#undef yytokentype

typedef enum dmnsn_yytokentype dmnsn_token_type;

typedef struct dmnsn_token dmnsn_token;

struct dmnsn_token {
  dmnsn_token_type type;
  char *value;

  /* File name, and line and column numbers from source code */
  const char *filename;
  int line, col;
};

/* Scanner manipulation */

int   dmnsn_yylex_init(void **scannerp);
void  dmnsn_yyset_in(FILE *file, void *scanner);
int   dmnsn_yylex_destroy(void *scanner);
void *dmnsn_yyget_extra(void *scanner);
void  dmnsn_yyset_extra(void *arbitrary_data, void *scanner);

void *dmnsn_yy_make_buffer(FILE *file, void *scanner);
void *dmnsn_yy_make_string_buffer(const char *str, void *scanner);
void  dmnsn_yy_push_buffer(void *buffer, void *scanner);
void  dmnsn_yy_pop_buffer(void *scanner);

/* Actual lexer */
int dmnsn_yylex(dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                const char *filename, dmnsn_symbol_table *symtable,
                void *yyscanner);

/* For debugging - returns an array of raw tokens */
dmnsn_array *dmnsn_tokenize(FILE *file, const char *filename);

/* Token destruction */
void dmnsn_delete_tokens(dmnsn_array *tokens);

/* Print an S-expression of a list of tokens to `file' */
void dmnsn_print_token_sexpr(FILE *file, const dmnsn_array *tokens);

/* Returns a readable name for a token type (ex. DMNSN_T_FLOAT -> float) */
const char *dmnsn_token_string(dmnsn_token_type token_type);

#endif /* TOKENIZE_H */
