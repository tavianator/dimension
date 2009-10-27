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
  DMNSN_LBRACE,
  DMNSN_RBRACE
} dmnsn_token_type;

typedef struct dmnsn_token dmnsn_token;

struct dmnsn_token {
  dmnsn_token_type type;
  char *value;
};

dmnsn_array *dmnsn_tokenize(FILE *file);
void dmnsn_delete_tokens(dmnsn_array *tokens);
void dmnsn_print_token_sexpr(FILE *file, dmnsn_array *tokens);
