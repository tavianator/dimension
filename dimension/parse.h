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

/*
 * Abstract syntax tree
 */

/* Abstract syntax tree node types */
typedef enum {
  DMNSN_AST_NONE,

  DMNSN_AST_ROTATION,
  DMNSN_AST_SCALE,
  DMNSN_AST_TRANSLATION,

  DMNSN_AST_CAMERA,
  DMNSN_AST_PERSPECTIVE,
  DMNSN_AST_LOCATION,
  DMNSN_AST_RIGHT,
  DMNSN_AST_UP,
  DMNSN_AST_SKY,
  DMNSN_AST_ANGLE,
  DMNSN_AST_LOOK_AT,
  DMNSN_AST_DIRECTION,

  DMNSN_AST_BACKGROUND,

  DMNSN_AST_BOX,
  DMNSN_AST_LIGHT_SOURCE,
  DMNSN_AST_SPHERE,

  DMNSN_AST_OBJECT_MODIFIERS,

  DMNSN_AST_TEXTURE,
  DMNSN_AST_PIGMENT,

  DMNSN_AST_FLOAT,
  DMNSN_AST_INTEGER,

  DMNSN_AST_VECTOR,

  DMNSN_AST_NEGATE,
  DMNSN_AST_ADD,
  DMNSN_AST_SUB,
  DMNSN_AST_MUL,
  DMNSN_AST_DIV,

  DMNSN_AST_STRING,
} dmnsn_astnode_type;

/* Abstract syntax tree node (a dmnsn_array* of these is an AST) */
typedef struct dmnsn_astnode {
  dmnsn_astnode_type type;

  /* Child nodes */
  dmnsn_array *children;

  /* Generic data pointer */
  void *ptr;

  /* Reference count */
  unsigned int *refcount;

  /* File name, and line and column numbers from source code */
  const char *filename;
  int line, col;
} dmnsn_astnode;

typedef dmnsn_array dmnsn_astree;

dmnsn_astnode dmnsn_new_ast_integer(long value);
dmnsn_astnode dmnsn_new_ast_float(double value);
dmnsn_astnode dmnsn_new_ast_string(const char *value);

void dmnsn_delete_astnode(dmnsn_astnode astnode);
void dmnsn_delete_astree(dmnsn_astree *astree);

/* Print an S-expression of the abstract syntax tree to `file' */
void dmnsn_print_astree_sexpr(FILE *file, const dmnsn_astree *astree);

/* Returns a readable name for an astnode type (ex. DMNSN_AST_FLOAT -> float) */
const char *dmnsn_astnode_string(dmnsn_astnode_type astnode_type);

/*
 * Symbol table
 */

typedef dmnsn_array dmnsn_symbol_table;

dmnsn_symbol_table *dmnsn_new_symbol_table();

void dmnsn_delete_symbol_table(dmnsn_symbol_table *symtable);

void dmnsn_push_scope(dmnsn_symbol_table *symtable);
void dmnsn_pop_scope(dmnsn_symbol_table *symtable);

void dmnsn_local_symbol(dmnsn_symbol_table *symtable,
                        const char *id, dmnsn_astnode value);
void dmnsn_declare_symbol(dmnsn_symbol_table *symtable,
                          const char *id, dmnsn_astnode value);
void dmnsn_undef_symbol(dmnsn_symbol_table *symtable, const char *id);

dmnsn_astnode *dmnsn_find_symbol(dmnsn_symbol_table *symtable, const char *id);

/* Evaluate an arithmetic expression */
dmnsn_astnode dmnsn_eval_scalar(dmnsn_astnode astnode,
                                dmnsn_symbol_table *symtable);
dmnsn_astnode dmnsn_eval_vector(dmnsn_astnode astnode,
                                dmnsn_symbol_table *symtable);


/*
 * The workhorse -- parse a file
 */
dmnsn_astree *dmnsn_parse(FILE *file, dmnsn_symbol_table *symtable);

/*
 * Parser internals
 */

typedef struct dmnsn_parse_location {
  const char *first_filename, *last_filename;
  int first_line, last_line;
  int first_column, last_column;
} dmnsn_parse_location;

typedef union dmnsn_parse_item {
  char *value;
  dmnsn_astnode astnode;
} dmnsn_parse_item;

#endif /* PARSE_H */
