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

#ifndef PARSE_H
#define PARSE_H

#include "../libdimension/dimension.h"

typedef enum {
  DMNSN_AST_FLOAT,
  DMNSN_AST_INTEGER,
  DMNSN_AST_NEGATE,
  DMNSN_AST_ADD,
  DMNSN_AST_SUB,
  DMNSN_AST_MUL,
  DMNSN_AST_DIV,
  DMNSN_AST_VECTOR,
  DMNSN_AST_BOX,
  DMNSN_AST_SPHERE,
  DMNSN_AST_ROTATION,
  DMNSN_AST_OBJECT_MODIFIERS,
} dmnsn_astnode_type;

typedef struct dmnsn_astnode dmnsn_astnode;

struct dmnsn_astnode {
  dmnsn_astnode_type type;

  /* Child nodes */
  dmnsn_array *children;

  /* Generic data pointer */
  void *ptr;

  /* File name, and line and column numbers from source code */
  const char *filename;
  unsigned int line, col;
};

/* The workhorse */
dmnsn_array *dmnsn_parse(const dmnsn_array *tokens);

/* Free an abstract syntax tree */
void dmnsn_delete_astree(dmnsn_array *astree);

/* Evaluate an arithmetic expression */
dmnsn_astnode dmnsn_eval_scalar(dmnsn_astnode astnode);
dmnsn_astnode dmnsn_eval_vector(dmnsn_astnode astnode);

/* Print an S-expression of the abstract syntax tree to `file' */
void dmnsn_print_astree_sexpr(FILE *file, const dmnsn_array *astree);

/* Returns a readable name for a token type (ex. DMNSN_T_FLOAT -> float) */
const char *dmnsn_astnode_string(dmnsn_astnode_type astnode_type);

#endif /* PARSE_H */
