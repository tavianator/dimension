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

#include "parse.h"
#include "utility.h"

static dmnsn_astnode
dmnsn_new_astnode(dmnsn_astnode_type type)
{
  dmnsn_astnode astnode = {
    .type     = type,
    .children = NULL,
    .ptr      = NULL,
    .refcount = malloc(sizeof(unsigned int)),
    .filename = "<environment>",
    .line     = -1,
    .col      = -1,
  };

  if (!astnode.refcount) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate reference count.");
  }
  *astnode.refcount = 0;

  return astnode;
}

dmnsn_astnode
dmnsn_new_ast_integer(long value)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_INTEGER);

  astnode.ptr = malloc(sizeof(long));
  if (!astnode.ptr)
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate room for integer.");

  *(long *)astnode.ptr = value;
  return astnode;
}

dmnsn_astnode
dmnsn_new_ast_float(double value)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_FLOAT);

  astnode.ptr = malloc(sizeof(double));
  if (!astnode.ptr)
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate room for float.");

  *(double *)astnode.ptr = value;
  return astnode;
}

dmnsn_astnode
dmnsn_new_ast_string(const char *value)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_STRING);
  astnode.ptr = strdup(value);
  return astnode;
}

void
dmnsn_delete_astnode(dmnsn_astnode astnode)
{
  if (*astnode.refcount <= 1) {
    dmnsn_delete_astree(astnode.children);
    free(astnode.ptr);
    free(astnode.refcount);
  } else {
    --*astnode.refcount;
  }
}

void
dmnsn_delete_astree(dmnsn_astree *astree)
{
  if (astree) {
    unsigned int i;
    for (i = 0; i < dmnsn_array_size(astree); ++i) {
      dmnsn_astnode node;
      dmnsn_array_get(astree, i, &node);
      dmnsn_delete_astnode(node);
    }
    dmnsn_delete_array(astree);
  }
}

/* TODO: use a hash table for symbol tables rather than an array */

typedef struct dmnsn_symbol {
  char *id;
  dmnsn_astnode value;
} dmnsn_symbol;

dmnsn_symbol_table *
dmnsn_new_symbol_table()
{
  dmnsn_symbol_table *symtable = dmnsn_new_array(sizeof(dmnsn_array *));
  dmnsn_push_scope(symtable);
  return symtable;
}

static void
dmnsn_delete_symbol(dmnsn_symbol symbol)
{
  free(symbol.id);
  dmnsn_delete_astnode(symbol.value);
}

static void
dmnsn_delete_scope(dmnsn_array *scope)
{
  while (dmnsn_array_size(scope) > 0) {
    dmnsn_symbol symbol;
    dmnsn_array_pop(scope, &symbol);
    dmnsn_delete_symbol(symbol);
  }
  dmnsn_delete_array(scope);
}

void
dmnsn_delete_symbol_table(dmnsn_symbol_table *symtable)
{
  while (dmnsn_array_size(symtable) > 0) {
    dmnsn_array *scope;
    dmnsn_array_pop(symtable, &scope);
    dmnsn_delete_scope(scope);
  }
  dmnsn_delete_array(symtable);
}

void
dmnsn_push_scope(dmnsn_symbol_table *symtable)
{
  dmnsn_array *scope = dmnsn_new_array(sizeof(dmnsn_symbol));
  dmnsn_array_push(symtable, &scope);
}

void dmnsn_pop_scope(dmnsn_symbol_table *symtable)
{
  dmnsn_array *scope;
  dmnsn_array_pop(symtable, &scope);
  dmnsn_delete_scope(scope);
}

void dmnsn_local_symbol(dmnsn_symbol_table *symtable,
                        const char *id, dmnsn_astnode value)
{
  ++*value.refcount;

  dmnsn_array *scope;
  dmnsn_array_get(symtable, dmnsn_array_size(symtable) - 1, &scope);

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(scope); ++i) {
    dmnsn_symbol *symbol = dmnsn_array_at(scope,
                                          dmnsn_array_size(scope) - i - 1);

    if (strcmp(id, symbol->id) == 0) {
      dmnsn_delete_astnode(symbol->value);
      symbol->value = value;
      return;
    }
  }

  dmnsn_symbol symbol = { .id = strdup(id), .value = value };
  dmnsn_array_push(scope, &symbol);
}

void
dmnsn_declare_symbol(dmnsn_symbol_table *symtable,
                     const char *id, dmnsn_astnode value)
{
  ++*value.refcount;

  dmnsn_astnode *node = dmnsn_find_symbol(symtable, id);
  if (node) {
    /* Always update the most local symbol */
    dmnsn_delete_astnode(*node);
    *node = value;
  } else {
    /* but create new ones at the least local scope */
    dmnsn_array *scope;
    dmnsn_array_get(symtable, 0, &scope);

    dmnsn_symbol symbol = { .id = strdup(id), .value = value };
    dmnsn_array_push(scope, &symbol);
  }
}

void
dmnsn_undef_symbol(dmnsn_symbol_table *symtable, const char *id)
{
  unsigned int i;
  for (i = 0; i < dmnsn_array_size(symtable); ++i) {
    dmnsn_array *scope;
    dmnsn_array_get(symtable, dmnsn_array_size(symtable) - i - 1, &scope);

    unsigned int j;
    for (j = 0; j < dmnsn_array_size(scope); ++j) {
      dmnsn_symbol *symbol = dmnsn_array_at(scope,
                                            dmnsn_array_size(scope) - j - 1);

      if (strcmp(id, symbol->id) == 0) {
        dmnsn_delete_symbol(*symbol);
        dmnsn_array_remove(scope, dmnsn_array_size(scope) - j - 1);
        return;
      }
    }
  }
}

dmnsn_astnode *
dmnsn_find_symbol(dmnsn_symbol_table *symtable, const char *id)
{
  unsigned int i;
  for (i = 0; i < dmnsn_array_size(symtable); ++i) {
    dmnsn_array *scope;
    dmnsn_array_get(symtable, dmnsn_array_size(symtable) - i - 1, &scope);

    unsigned int j;
    for (j = 0; j < dmnsn_array_size(scope); ++j) {
      dmnsn_symbol *symbol = dmnsn_array_at(scope,
                                            dmnsn_array_size(scope) - j - 1);

      if (strcmp(id, symbol->id) == 0)
        return &symbol->value;
    }
  }

  return NULL;
}

static dmnsn_astnode
dmnsn_copy_astnode(dmnsn_astnode astnode)
{
  dmnsn_astnode copy = {
    .type     = astnode.type,
    .children = dmnsn_new_array(sizeof(dmnsn_astnode)),
    .ptr      = NULL,
    .refcount = malloc(sizeof(unsigned int)),
    .filename = astnode.filename,
    .line     = astnode.line,
    .col      = astnode.col
  };

  if (!copy.refcount) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate reference count.");
  }
  *copy.refcount = 1;

  return copy;
}

/* 5-element vectors */
#define DMNSN_VECTOR_NELEM 5

static dmnsn_astnode
dmnsn_vector_promote(dmnsn_astnode astnode, dmnsn_symbol_table *symtable)
{
  dmnsn_astnode promoted = dmnsn_copy_astnode(astnode);

  if (astnode.type == DMNSN_AST_VECTOR) {
    dmnsn_astnode component;
    unsigned int i;
    for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
      dmnsn_array_get(astnode.children, i, &component);
      component = dmnsn_eval_scalar(component, symtable);

      if (component.type == DMNSN_AST_NONE) {
        dmnsn_delete_astnode(promoted);
        return component;
      } else {
        dmnsn_array_push(promoted.children, &component);
      }
    }

    while (dmnsn_array_size(promoted.children) < DMNSN_VECTOR_NELEM) {
      component = dmnsn_copy_astnode(component);
      component.type = DMNSN_AST_INTEGER;

      long *val = malloc(sizeof(long));
      *val      = 0;

      component.ptr = val;
      dmnsn_array_push(promoted.children, &component);
    }
  } else {
    dmnsn_astnode component = dmnsn_eval_scalar(astnode, symtable);

    if (component.type == DMNSN_AST_NONE) {
      promoted.type = DMNSN_AST_NONE;
    } else {
      promoted.type = DMNSN_AST_VECTOR;
      while (dmnsn_array_size(promoted.children) < DMNSN_VECTOR_NELEM) {
        dmnsn_array_push(promoted.children, &component);
        ++*component.refcount;
      }
    }

    dmnsn_delete_astnode(component);
  }

  return promoted;
}

static dmnsn_astnode
dmnsn_eval_unary(dmnsn_astnode astnode, dmnsn_symbol_table *symtable)
{
  unsigned int i;

  dmnsn_astnode rhs;
  dmnsn_array_get(astnode.children, 0, &rhs);
  rhs = dmnsn_eval(rhs, symtable);

  dmnsn_astnode ret;

  if (rhs.type == DMNSN_AST_NONE) {
    ret = dmnsn_copy_astnode(astnode);
    ret.type = DMNSN_AST_NONE;
  } else if (rhs.type == DMNSN_AST_VECTOR) {
    switch (astnode.type) {
    case DMNSN_AST_DOT_X:
      dmnsn_array_get(rhs.children, 0, &ret);
      ++*ret.refcount;
      break;

    case DMNSN_AST_DOT_Y:
      dmnsn_array_get(rhs.children, 1, &ret);
      ++*ret.refcount;
      break;

    case DMNSN_AST_DOT_Z:
      dmnsn_array_get(rhs.children, 2, &ret);
      ++*ret.refcount;
      break;

    case DMNSN_AST_DOT_T:
      dmnsn_array_get(rhs.children, 3, &ret);
      ++*ret.refcount;
      break;

    case DMNSN_AST_DOT_TRANSMIT:
      dmnsn_array_get(rhs.children, 4, &ret);
      ++*ret.refcount;
      break;

    default:
      {
        ret = dmnsn_copy_astnode(astnode);
        ret.type = DMNSN_AST_VECTOR;

        dmnsn_astnode op = dmnsn_copy_astnode(astnode);
        for (i = 0; i < DMNSN_VECTOR_NELEM; ++i) {
          dmnsn_array_get(rhs.children, i, dmnsn_array_at(op.children, 0));
          dmnsn_astnode temp = dmnsn_eval_unary(op, symtable);
          dmnsn_array_set(ret.children, i, &temp);
        }

        dmnsn_delete_array(op.children);
        op.children = NULL;
        dmnsn_delete_astnode(op);
        break;
      }
    }
  } else if (rhs.type == DMNSN_AST_INTEGER) {
    long n = *(long *)rhs.ptr;
    long *res = malloc(sizeof(long));
    if (!res)
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Failed to allocate room for integer.");

    switch(astnode.type) {
    case DMNSN_AST_DOT_X:
    case DMNSN_AST_DOT_Y:
    case DMNSN_AST_DOT_Z:
    case DMNSN_AST_DOT_T:
    case DMNSN_AST_DOT_TRANSMIT:
      *res = n;

    case DMNSN_AST_NEGATE:
      *res = -n;
      break;

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH,
                  "Attempt to evaluate wrong unary operator.");
    }

    ret = dmnsn_copy_astnode(astnode);
    ret.type = DMNSN_AST_INTEGER;
    ret.ptr  = res;
  } else if (rhs.type == DMNSN_AST_FLOAT) {
    double n = *(double *)rhs.ptr;
    double *res = malloc(sizeof(double));
    if (!res)
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Failed to allocate room for float.");

    switch(astnode.type) {
    case DMNSN_AST_DOT_X:
    case DMNSN_AST_DOT_Y:
    case DMNSN_AST_DOT_Z:
    case DMNSN_AST_DOT_T:
    case DMNSN_AST_DOT_TRANSMIT:
      *res = n;

    case DMNSN_AST_NEGATE:
      *res = -n;
      break;

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH,
                  "Attempt to evaluate wrong unary operator.");
    }

    ret = dmnsn_copy_astnode(astnode);
    ret.type = DMNSN_AST_FLOAT;
    ret.ptr  = res;
  } else {
    dmnsn_diagnostic(rhs.filename, rhs.line, rhs.col,
                     "expected %s, %s, or %s; found %s",
                     dmnsn_astnode_string(DMNSN_AST_INTEGER),
                     dmnsn_astnode_string(DMNSN_AST_FLOAT),
                     dmnsn_astnode_string(DMNSN_AST_VECTOR));
    ret = dmnsn_copy_astnode(astnode);
    ret.type = DMNSN_AST_NONE;
  }

  dmnsn_delete_astnode(rhs);
  return ret;
}

static dmnsn_astnode
dmnsn_eval_binary(dmnsn_astnode astnode, dmnsn_symbol_table *symtable)
{
  unsigned int i;

  dmnsn_astnode lhs, rhs;
  dmnsn_array_get(astnode.children, 0, &lhs);
  dmnsn_array_get(astnode.children, 1, &rhs);
  lhs = dmnsn_eval(lhs, symtable);
  rhs = dmnsn_eval(rhs, symtable);

  dmnsn_astnode ret;

  if (lhs.type == DMNSN_AST_NONE || rhs.type == DMNSN_AST_NONE) {
    ret = dmnsn_copy_astnode(astnode);
    ret.type = DMNSN_AST_NONE;
  } else if (lhs.type == DMNSN_AST_VECTOR || rhs.type == DMNSN_AST_VECTOR) {
    dmnsn_astnode oldlhs = lhs, oldrhs = rhs;
    lhs = dmnsn_vector_promote(lhs, symtable);
    rhs = dmnsn_vector_promote(rhs, symtable);
    dmnsn_delete_astnode(oldlhs);
    dmnsn_delete_astnode(oldrhs);

    switch (astnode.type) {
    case DMNSN_AST_EQUAL:
      {
        dmnsn_astnode rewrite = dmnsn_copy_astnode(astnode);

        dmnsn_astnode l, r;
        dmnsn_array_get(lhs.children, 0, &l);
        dmnsn_array_get(rhs.children, 0, &r);
        ++*l.refcount;
        ++*r.refcount;
        dmnsn_array_push(rewrite.children, &l);
        dmnsn_array_push(rewrite.children, &r);

        for (i = 1; i < DMNSN_VECTOR_NELEM; ++i) {
          dmnsn_astnode temp = dmnsn_copy_astnode(astnode);
          dmnsn_array_get(lhs.children, i, &l);
          dmnsn_array_get(rhs.children, i, &r);
          ++*l.refcount;
          ++*r.refcount;
          dmnsn_array_push(temp.children, &l);
          dmnsn_array_push(temp.children, &r);

          dmnsn_astnode next = dmnsn_copy_astnode(astnode);
          next.type = DMNSN_AST_AND;
          dmnsn_array_push(next.children, &rewrite);
          dmnsn_array_push(next.children, &temp);
          rewrite = next;
        }

        ret = dmnsn_eval_binary(rewrite, symtable);
        dmnsn_delete_astnode(rewrite);
        break;
      }

    case DMNSN_AST_NOT_EQUAL:
      {
        dmnsn_astnode rewrite = dmnsn_copy_astnode(astnode);

        dmnsn_astnode l, r;
        dmnsn_array_get(lhs.children, 0, &l);
        dmnsn_array_get(rhs.children, 0, &r);
        ++*l.refcount;
        ++*r.refcount;
        dmnsn_array_push(rewrite.children, &l);
        dmnsn_array_push(rewrite.children, &r);

        for (i = 1; i < DMNSN_VECTOR_NELEM; ++i) {
          dmnsn_astnode temp = dmnsn_copy_astnode(astnode);
          dmnsn_array_get(lhs.children, i, &l);
          dmnsn_array_get(rhs.children, i, &r);
          ++*l.refcount;
          ++*r.refcount;
          dmnsn_array_push(temp.children, &l);
          dmnsn_array_push(temp.children, &r);

          dmnsn_astnode next = dmnsn_copy_astnode(astnode);
          next.type = DMNSN_AST_OR;
          dmnsn_array_push(next.children, &rewrite);
          dmnsn_array_push(next.children, &temp);
          rewrite = next;
        }

        ret = dmnsn_eval_binary(rewrite, symtable);
        dmnsn_delete_astnode(rewrite);
        break;
      }

    case DMNSN_AST_AND:
      {
        dmnsn_astnode rewrite = dmnsn_copy_astnode(astnode);
        rewrite.type = DMNSN_AST_OR;

        dmnsn_astnode l, r;
        dmnsn_array_get(lhs.children, 0, &l);
        dmnsn_array_get(rhs.children, 0, &r);
        ++*l.refcount;
        ++*r.refcount;
        dmnsn_array_push(rewrite.children, &l);
        dmnsn_array_push(rewrite.children, &r);

        for (i = 1; i < DMNSN_VECTOR_NELEM; ++i) {
          dmnsn_astnode temp = dmnsn_copy_astnode(astnode);
          temp.type = DMNSN_AST_OR;
          dmnsn_array_get(lhs.children, i, &l);
          dmnsn_array_get(rhs.children, i, &r);
          ++*l.refcount;
          ++*r.refcount;
          dmnsn_array_push(temp.children, &l);
          dmnsn_array_push(temp.children, &r);

          dmnsn_astnode next = dmnsn_copy_astnode(astnode);
          next.type = DMNSN_AST_AND;
          dmnsn_array_push(next.children, &rewrite);
          dmnsn_array_push(next.children, &temp);
          rewrite = next;
        }

        ret = dmnsn_eval_binary(rewrite, symtable);
        dmnsn_delete_astnode(rewrite);
        break;
      }

    case DMNSN_AST_OR:
      {
        dmnsn_astnode rewrite = dmnsn_copy_astnode(astnode);
        rewrite.type = DMNSN_AST_OR;

        dmnsn_astnode l, r;
        dmnsn_array_get(lhs.children, 0, &l);
        dmnsn_array_get(rhs.children, 0, &r);
        ++*l.refcount;
        ++*r.refcount;
        dmnsn_array_push(rewrite.children, &l);
        dmnsn_array_push(rewrite.children, &r);

        for (i = 1; i < DMNSN_VECTOR_NELEM; ++i) {
          dmnsn_astnode temp = dmnsn_copy_astnode(astnode);
          temp.type = DMNSN_AST_OR;
          dmnsn_array_get(lhs.children, i, &l);
          dmnsn_array_get(rhs.children, i, &r);
          ++*l.refcount;
          ++*r.refcount;
          dmnsn_array_push(temp.children, &l);
          dmnsn_array_push(temp.children, &r);

          dmnsn_astnode next = dmnsn_copy_astnode(astnode);
          next.type = DMNSN_AST_OR;
          dmnsn_array_push(next.children, &rewrite);
          dmnsn_array_push(next.children, &temp);
          rewrite = next;
        }

        ret = dmnsn_eval_binary(rewrite, symtable);
        dmnsn_delete_astnode(rewrite);
        break;
      }

    case DMNSN_AST_LESS:
    case DMNSN_AST_LESS_EQUAL:
    case DMNSN_AST_GREATER:
    case DMNSN_AST_GREATER_EQUAL:
      dmnsn_diagnostic(astnode.filename, astnode.line, astnode.col,
                       "invalid comparison operator '%s' between vectors",
                       dmnsn_astnode_string(astnode.type));
      ret = dmnsn_copy_astnode(astnode);
      ret.type = DMNSN_AST_NONE;
      break;

    default:
      {
        ret = dmnsn_copy_astnode(astnode);
        ret.type = DMNSN_AST_VECTOR;

        dmnsn_astnode op = dmnsn_copy_astnode(astnode);
        for (i = 0; i < DMNSN_VECTOR_NELEM; ++i) {
          dmnsn_array_get(lhs.children, i, dmnsn_array_at(op.children, 0));
          dmnsn_array_get(rhs.children, i, dmnsn_array_at(op.children, 1));
          dmnsn_astnode temp = dmnsn_eval_binary(op, symtable);
          dmnsn_array_set(ret.children, i, &temp);
        }

        dmnsn_delete_array(op.children);
        op.children = NULL;
        dmnsn_delete_astnode(op);
        break;
      }
    }
  } else if (lhs.type == DMNSN_AST_INTEGER && rhs.type == DMNSN_AST_INTEGER
             && astnode.type != DMNSN_AST_DIV) {
    ret = dmnsn_copy_astnode(astnode);

    long l, r;
    l = *(long *)lhs.ptr;
    r = *(long *)rhs.ptr;

    long *res = malloc(sizeof(long));
    if (!res)
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Failed to allocate room for integer.");

    switch (astnode.type) {
    case DMNSN_AST_ADD:
      *res = l + r;
      break;
    case DMNSN_AST_SUB:
      *res = l - r;
      break;
    case DMNSN_AST_MUL:
      *res = l*r;
      break;
    case DMNSN_AST_EQUAL:
      *res = l == r;
      break;
    case DMNSN_AST_NOT_EQUAL:
      *res = l != r;
      break;
    case DMNSN_AST_LESS:
      *res = l < r;
      break;
    case DMNSN_AST_LESS_EQUAL:
      *res = l <= r;
      break;
    case DMNSN_AST_GREATER:
      *res = l > r;
      break;
    case DMNSN_AST_GREATER_EQUAL:
      *res = l >= r;
      break;
    case DMNSN_AST_AND:
      *res = l && r;
      break;
    case DMNSN_AST_OR:
      *res = l || r;
      break;

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH,
                  "Attempt to evaluate wrong binary operator.");
    }

    ret.type = DMNSN_AST_INTEGER;
    ret.ptr  = res;
  } else {
    ret = dmnsn_copy_astnode(astnode);

    double l = 0.0, r = 0.0;

    if (lhs.type == DMNSN_AST_INTEGER) {
      l = *(long *)lhs.ptr;
    } else if (lhs.type == DMNSN_AST_FLOAT) {
      l = *(double *)lhs.ptr;
    } else {
      dmnsn_diagnostic(lhs.filename, lhs.line, lhs.col,
                       "expected %s, %s, or %s; found %s",
                       dmnsn_astnode_string(DMNSN_AST_INTEGER),
                       dmnsn_astnode_string(DMNSN_AST_FLOAT),
                       dmnsn_astnode_string(DMNSN_AST_VECTOR),
                       dmnsn_astnode_string(lhs.type));
      ret.type = DMNSN_AST_NONE;
      dmnsn_delete_astnode(lhs);
      dmnsn_delete_astnode(rhs);
      return ret;
    }

    if (rhs.type == DMNSN_AST_INTEGER) {
      r = *(long *)rhs.ptr;
    } else if (rhs.type == DMNSN_AST_FLOAT) {
      r = *(double *)rhs.ptr;
    } else {
      dmnsn_diagnostic(rhs.filename, rhs.line, rhs.col,
                       "expected %s, %s, or %s; found %s",
                       dmnsn_astnode_string(DMNSN_AST_INTEGER),
                       dmnsn_astnode_string(DMNSN_AST_FLOAT),
                       dmnsn_astnode_string(DMNSN_AST_VECTOR),
                       dmnsn_astnode_string(rhs.type));
      ret.type = DMNSN_AST_NONE;
      dmnsn_delete_astnode(lhs);
      dmnsn_delete_astnode(rhs);
      return ret;
    }

    double *res = malloc(sizeof(double));
    if (!res)
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Failed to allocate room for float.");

    switch (astnode.type) {
    case DMNSN_AST_ADD:
      *res = l + r;
      break;
    case DMNSN_AST_SUB:
      *res = l - r;
      break;
    case DMNSN_AST_MUL:
      *res = l*r;
      break;
    case DMNSN_AST_DIV:
      *res = l/r;
      break;
    case DMNSN_AST_EQUAL:
      *res = l == r;
      break;
    case DMNSN_AST_NOT_EQUAL:
      *res = l != r;
      break;
    case DMNSN_AST_LESS:
      *res = l < r;
      break;
    case DMNSN_AST_LESS_EQUAL:
      *res = l <= r;
      break;
    case DMNSN_AST_GREATER:
      *res = l > r;
      break;
    case DMNSN_AST_GREATER_EQUAL:
      *res = l >= r;
      break;
    case DMNSN_AST_AND:
      *res = l && r;
      break;
    case DMNSN_AST_OR:
      *res = l || r;
      break;

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH,
                  "Attempt to evaluate wrong binary operator.");
    }

    ret.type = DMNSN_AST_FLOAT;
    ret.ptr  = res;
  }

  dmnsn_delete_astnode(lhs);
  dmnsn_delete_astnode(rhs);
  return ret;
}

dmnsn_astnode
dmnsn_eval(dmnsn_astnode astnode, dmnsn_symbol_table *symtable)
{
  switch (astnode.type) {
  case DMNSN_AST_NONE:
  case DMNSN_AST_INTEGER:
  case DMNSN_AST_FLOAT:
    do {
      ++*astnode.refcount;
    } while (*astnode.refcount <= 1);
    return astnode;

  case DMNSN_AST_VECTOR:
    return dmnsn_vector_promote(astnode, symtable);

  case DMNSN_AST_IDENTIFIER:
    {
      dmnsn_astnode *symbol = dmnsn_find_symbol(symtable, astnode.ptr);
      if (symbol) {
        return dmnsn_eval(*symbol, symtable);
      } else {
        dmnsn_diagnostic(astnode.filename, astnode.line, astnode.col,
                         "unbound identifier '%s'", astnode.ptr);
        dmnsn_astnode error = dmnsn_new_astnode(DMNSN_AST_NONE);
        ++*error.refcount;
        return error;
      }
    }

  case DMNSN_AST_DOT_X:
  case DMNSN_AST_DOT_Y:
  case DMNSN_AST_DOT_Z:
  case DMNSN_AST_DOT_T:
  case DMNSN_AST_DOT_TRANSMIT:
  case DMNSN_AST_NEGATE:
    return dmnsn_eval_unary(astnode, symtable);

  case DMNSN_AST_ADD:
  case DMNSN_AST_SUB:
  case DMNSN_AST_MUL:
  case DMNSN_AST_DIV:
  case DMNSN_AST_EQUAL:
  case DMNSN_AST_NOT_EQUAL:
  case DMNSN_AST_LESS:
  case DMNSN_AST_LESS_EQUAL:
  case DMNSN_AST_GREATER:
  case DMNSN_AST_GREATER_EQUAL:
  case DMNSN_AST_AND:
  case DMNSN_AST_OR:
    return dmnsn_eval_binary(astnode, symtable);

  default:
    dmnsn_diagnostic(astnode.filename, astnode.line, astnode.col,
                     "expected arithmetic expression; found %s",
                     dmnsn_astnode_string(astnode.type));
    dmnsn_astnode error = dmnsn_new_astnode(DMNSN_AST_NONE);
    ++*error.refcount;
    return error;
  }

  return astnode;
}

dmnsn_astnode
dmnsn_eval_scalar(dmnsn_astnode astnode, dmnsn_symbol_table *symtable)
{
  dmnsn_astnode ret = dmnsn_eval(astnode, symtable);
  if (ret.type == DMNSN_AST_VECTOR) {
    dmnsn_diagnostic(ret.filename, ret.line, ret.col,
                     "expected %s or %s; found %s",
                     dmnsn_astnode_string(DMNSN_AST_INTEGER),
                     dmnsn_astnode_string(DMNSN_AST_FLOAT),
                     dmnsn_astnode_string(ret.type));
    dmnsn_delete_astnode(ret);
    ret = dmnsn_new_astnode(DMNSN_AST_NONE);
  }
  return ret;
}

dmnsn_astnode
dmnsn_eval_vector(dmnsn_astnode astnode, dmnsn_symbol_table *symtable)
{
  dmnsn_astnode eval = dmnsn_eval(astnode, symtable);
  dmnsn_astnode ret  = dmnsn_vector_promote(eval, symtable);

  dmnsn_delete_astnode(eval);
  return ret;
}

static void
dmnsn_print_astnode(FILE *file, dmnsn_astnode astnode)
{
  long ivalue;
  double dvalue;
  const char *svalue;

  switch (astnode.type) {
  case DMNSN_AST_INTEGER:
    ivalue = *(long *)astnode.ptr;
    fprintf(file, "(%s %ld)", dmnsn_astnode_string(astnode.type), ivalue);
    break;

  case DMNSN_AST_FLOAT:
    dvalue = *(double *)astnode.ptr;
    fprintf(file, "(%s %g)", dmnsn_astnode_string(astnode.type), dvalue);
    break;

  case DMNSN_AST_STRING:
    svalue = astnode.ptr;
    fprintf(file, "(%s \"%s\")", dmnsn_astnode_string(astnode.type), svalue);
    break;

  default:
    fprintf(file, "%s", dmnsn_astnode_string(astnode.type));
  }
}

static void
dmnsn_print_astree(FILE *file, dmnsn_astnode astnode)
{
  unsigned int i;
  dmnsn_astnode child;

  if (astnode.children && dmnsn_array_size(astnode.children) > 0) {
    fprintf(file, "(");
    dmnsn_print_astnode(file, astnode);
    for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
      dmnsn_array_get(astnode.children, i, &child);
      fprintf(file, " ");
      dmnsn_print_astree(file, child);
    }
    fprintf(file, ")");
  } else {
    dmnsn_print_astnode(file, astnode);
  }
}

void
dmnsn_print_astree_sexpr(FILE *file, const dmnsn_astree *astree)
{
  dmnsn_astnode astnode;
  unsigned int i;

  if (dmnsn_array_size(astree) == 0) {
    fprintf(file, "()");
  } else {
    fprintf(file, "(");
    dmnsn_array_get(astree, 0, &astnode);
    dmnsn_print_astree(file, astnode);

    for (i = 1; i < dmnsn_array_size(astree); ++i) {
      fprintf(file, " ");
      dmnsn_array_get(astree, i, &astnode);
      dmnsn_print_astree(file, astnode);
    }

    fprintf(file, ")");
  }

  fprintf(file, "\n");
}
