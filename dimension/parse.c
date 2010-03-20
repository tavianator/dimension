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
#include <math.h>
#include <fenv.h>

/*
 * Symbol table
 */

typedef struct dmnsn_patricia_trie {
  char *prefix;

  bool leaf;
  dmnsn_array *children;
  dmnsn_astnode value;
} dmnsn_patricia_trie;

void
dmnsn_delete_patricia_trie(dmnsn_patricia_trie *trie)
{
  if (trie) {
    free(trie->prefix);

    if (trie->leaf)
      dmnsn_delete_astnode(trie->value);

    unsigned int i;
    for (i = 0; i < dmnsn_array_size(trie->children); ++i) {
      dmnsn_patricia_trie *subtrie;
      dmnsn_array_get(trie->children, i, &subtrie);
      dmnsn_delete_patricia_trie(subtrie);
    }
    dmnsn_delete_array(trie->children);

    free(trie);
  }
}

dmnsn_patricia_trie *
dmnsn_new_patricia_trie()
{
  dmnsn_patricia_trie *trie = malloc(sizeof(dmnsn_patricia_trie));
  if (!trie) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate PATRICIA trie.");
  }

  trie->prefix = strdup("");
  if (!trie->prefix) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate PATRICIA trie.");
  }

  trie->leaf = false;
  trie->children = dmnsn_new_array(sizeof(dmnsn_patricia_trie *));
  return trie;
}

void
dmnsn_patricia_insert(dmnsn_patricia_trie *trie,
                      const char *id, dmnsn_astnode value)
{
  /*
   * PATRICIA trie insertion: O(k), where k is the length of the longest string
   * in the trie.
   */

  ++*value.refcount;

  while (true) {
    if (trie->prefix[0] == '\0'&& !trie->leaf
        && dmnsn_array_size(trie->children) == 0)
    {
      /* Replace an empty tree with a single-element tree */
      trie->prefix = realloc(trie->prefix, strlen(id) + 1);
      if (!trie->prefix)
        dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate room for prefix.");
      strcpy(trie->prefix, id);

      trie->leaf = true;
      trie->value = value;
      break;
    }

    char *prefix = trie->prefix;
    while (*prefix == *id && *prefix && *id) {
      ++prefix;
      ++id;
    }

    if (*id == '\0' && *prefix == '\0') {
      /* Complete match */
      if (trie->leaf) {
        dmnsn_delete_astnode(trie->value);
      }
      trie->leaf = true;
      trie->value = value;
      break;
    } else if (*prefix == '\0') {
      /* Partial match; id starts with prefix */
      size_t i, size = dmnsn_array_size(trie->children);
      for (i = 0; i < size; ++i) {
        dmnsn_patricia_trie *subtrie;
        dmnsn_array_get(trie->children, i, &subtrie);

        if (subtrie->prefix[0] == id[0]) {
          trie = subtrie;
          break;
        }
      }

      if (i == size) {
        /* No submatch found, add a new child */
        dmnsn_patricia_trie *subtrie = dmnsn_new_patricia_trie();
        dmnsn_array_push(trie->children, &subtrie);
        trie = subtrie;
      }
    } else {
      /* Split the tree */
      dmnsn_patricia_trie *copy = dmnsn_new_patricia_trie();
      copy->prefix = realloc(copy->prefix, strlen(prefix) + 1);
      if (!trie->prefix)
        dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate room for prefix.");
      strcpy(copy->prefix, prefix);
      *prefix = '\0';

      if (trie->leaf) {
        copy->leaf = true;
        copy->value = trie->value;
        trie->leaf = false;
      }

      dmnsn_delete_array(copy->children);
      copy->children = trie->children;
      trie->children = dmnsn_new_array(sizeof(dmnsn_patricia_trie *));

      dmnsn_patricia_trie *subtrie = dmnsn_new_patricia_trie();
      dmnsn_array_push(trie->children, &copy);
      dmnsn_array_push(trie->children, &subtrie);
      trie = subtrie;
    }
  }
}

bool
dmnsn_patricia_remove(dmnsn_patricia_trie *trie, const char *id)
{
  /*
   * PATRICIA trie removal: O(k), where k is the length of the longest string
   * in the trie.
   *
   * This implementation doesn't actually collapse the tree back upwards if a
   * node is left with only one child, to reduce complexity.
   */

  size_t len = strlen(trie->prefix);
  if (strncmp(id, trie->prefix, len) != 0)
    return false;
  id += len;

  while (true) {
    if (*id == '\0' && trie->leaf) {
      dmnsn_delete_astnode(trie->value);
      trie->leaf = false;
      return true;
    } else {
      size_t i, size = dmnsn_array_size(trie->children);
      for (i = 0; i < size; ++i) {
        dmnsn_patricia_trie *subtrie;
        dmnsn_array_get(trie->children, i, &subtrie);

        len = strlen(subtrie->prefix);
        if (strncmp(id, subtrie->prefix, len) == 0) {
          trie = subtrie;
          id += len;
          break;
        }
      }

      if (i == size)
        break;
    }
  }

  return false;
}

dmnsn_astnode *
dmnsn_patricia_find(dmnsn_patricia_trie *trie, const char *id)
{
  /*
   * PATRICIA trie search: O(k), where k is the length of the longest string
   * in the trie.
   */

  size_t len = strlen(trie->prefix);
  if (strncmp(id, trie->prefix, len) != 0)
    return NULL;
  id += len;

  while (true) {
    if (*id == '\0' && trie->leaf) {
      return &trie->value;
    } else {
      size_t i, size = dmnsn_array_size(trie->children);
      for (i = 0; i < size; ++i) {
        dmnsn_patricia_trie *subtrie;
        dmnsn_array_get(trie->children, i, &subtrie);

        len = strlen(subtrie->prefix);
        if (strncmp(id, subtrie->prefix, len) == 0) {
          trie = subtrie;
          id += len;
          break;
        }
      }

      if (i == size)
        break;
    }
  }

  return NULL;
}

dmnsn_symbol_table *
dmnsn_new_symbol_table()
{
  dmnsn_symbol_table *symtable = dmnsn_new_array(sizeof(dmnsn_patricia_trie *));
  dmnsn_push_scope(symtable);
  return symtable;
}

void
dmnsn_delete_symbol_table(dmnsn_symbol_table *symtable)
{
  while (dmnsn_array_size(symtable) > 0) {
    dmnsn_pop_scope(symtable);
  }
  dmnsn_delete_array(symtable);
}

void
dmnsn_push_scope(dmnsn_symbol_table *symtable)
{
  dmnsn_patricia_trie *scope = dmnsn_new_patricia_trie();
  dmnsn_array_push(symtable, &scope);
}

void dmnsn_pop_scope(dmnsn_symbol_table *symtable)
{
  dmnsn_patricia_trie *scope;
  dmnsn_array_pop(symtable, &scope);
  dmnsn_delete_patricia_trie(scope);
}

void dmnsn_local_symbol(dmnsn_symbol_table *symtable,
                        const char *id, dmnsn_astnode value)
{
  dmnsn_patricia_trie *trie;
  dmnsn_array_get(symtable, dmnsn_array_size(symtable) - 1, &trie);
  dmnsn_patricia_insert(trie, id, value);
}

void
dmnsn_declare_symbol(dmnsn_symbol_table *symtable,
                     const char *id, dmnsn_astnode value)
{
  dmnsn_astnode *node = dmnsn_find_symbol(symtable, id);
  if (node) {
    /* Always update the most local symbol */
    dmnsn_delete_astnode(*node);
    ++*value.refcount;
    *node = value;
  } else {
    /* but create new ones at the least local scope */
    dmnsn_patricia_trie *trie;
    dmnsn_array_get(symtable, 0, &trie);
    dmnsn_patricia_insert(trie, id, value);
  }
}

void
dmnsn_undef_symbol(dmnsn_symbol_table *symtable, const char *id)
{
  unsigned int i;
  for (i = 0; i < dmnsn_array_size(symtable); ++i) {
    dmnsn_patricia_trie *trie;
    dmnsn_array_get(symtable, dmnsn_array_size(symtable) - i - 1, &trie);
    if (dmnsn_patricia_remove(trie, id))
      break;
  }
}

dmnsn_astnode *
dmnsn_find_symbol(dmnsn_symbol_table *symtable, const char *id)
{
  dmnsn_astnode *symbol = NULL;

  unsigned int i;
  for (i = 0; i < dmnsn_array_size(symtable); ++i) {
    dmnsn_patricia_trie *trie;
    dmnsn_array_get(symtable, dmnsn_array_size(symtable) - i - 1, &trie);

    symbol = dmnsn_patricia_find(trie, id);
    if (symbol)
      break;
  }

  return symbol;
}

/*
 * Abstract syntax tree
 */

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
dmnsn_new_ast_array()
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_ARRAY);
  astnode.children = dmnsn_new_array(sizeof(dmnsn_astnode));
  return astnode;
}

static void
dmnsn_make_ast_integer(dmnsn_astnode *astnode, long value)
{
  astnode->type = DMNSN_AST_INTEGER;
  astnode->ptr = malloc(sizeof(long));
  if (!astnode->ptr)
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate room for integer.");
  *(long *)astnode->ptr = value;
}

dmnsn_astnode
dmnsn_new_ast_integer(long value)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_INTEGER);
  dmnsn_make_ast_integer(&astnode, value);
  return astnode;
}

static void
dmnsn_make_ast_float(dmnsn_astnode *astnode, double value)
{
  astnode->type = DMNSN_AST_FLOAT;
  astnode->ptr = malloc(sizeof(double));
  if (!astnode->ptr)
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate room for integer.");
  *(double *)astnode->ptr = value;
}

dmnsn_astnode
dmnsn_new_ast_float(double value)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_FLOAT);
  dmnsn_make_ast_float(&astnode, value);
  return astnode;
}

dmnsn_astnode
dmnsn_new_ast_ivector(long x, long y, long z, long f, long t)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_VECTOR);
  astnode.children = dmnsn_new_array(sizeof(dmnsn_astnode));

  dmnsn_astnode comp;

  comp = dmnsn_new_ast_integer(x);
  dmnsn_array_push(astnode.children, &comp);

  comp = dmnsn_new_ast_integer(y);
  dmnsn_array_push(astnode.children, &comp);

  comp = dmnsn_new_ast_integer(z);
  dmnsn_array_push(astnode.children, &comp);

  comp = dmnsn_new_ast_integer(f);
  dmnsn_array_push(astnode.children, &comp);

  comp = dmnsn_new_ast_integer(t);
  dmnsn_array_push(astnode.children, &comp);

  return astnode;
}

dmnsn_astnode
dmnsn_new_ast_vector(double x, double y, double z, double f, double t)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_VECTOR);
  astnode.children = dmnsn_new_array(sizeof(dmnsn_astnode));

  dmnsn_astnode comp;

  comp = dmnsn_new_ast_float(x);
  dmnsn_array_push(astnode.children, &comp);

  comp = dmnsn_new_ast_float(y);
  dmnsn_array_push(astnode.children, &comp);

  comp = dmnsn_new_ast_float(z);
  dmnsn_array_push(astnode.children, &comp);

  comp = dmnsn_new_ast_float(f);
  dmnsn_array_push(astnode.children, &comp);

  comp = dmnsn_new_ast_float(t);
  dmnsn_array_push(astnode.children, &comp);

  return astnode;
}

dmnsn_astnode
dmnsn_new_ast_string(const char *value)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_STRING);
  astnode.ptr = strdup(value);
  if (!astnode.ptr)
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate room for string.");
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
      if (!val)
        dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate room for integer.");
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

static void
dmnsn_make_ast_maybe_integer(dmnsn_astnode *ret, double n)
{
  feclearexcept(FE_ALL_EXCEPT);
  long l = lrint(n);
  if (fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW)) {
    dmnsn_make_ast_float(ret, n);
  } else {
    dmnsn_make_ast_integer(ret, l);
  }
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

    case DMNSN_AST_VLENGTH:
      {
        dmnsn_astnode dot = dmnsn_copy_astnode(astnode);
        dot.type = DMNSN_AST_VDOT;
        *rhs.refcount += 2;
        dmnsn_array_push(dot.children, &rhs);
        dmnsn_array_push(dot.children, &rhs);

        dmnsn_astnode rewrite = dmnsn_copy_astnode(astnode);
        rewrite.type = DMNSN_AST_SQRT;
        dmnsn_array_push(rewrite.children, &dot);

        ret = dmnsn_eval_unary(rewrite, symtable);
        dmnsn_delete_astnode(rewrite);
        break;
      }

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
    ret = dmnsn_copy_astnode(astnode);

    switch(astnode.type) {
    case DMNSN_AST_DOT_X:
    case DMNSN_AST_DOT_Y:
    case DMNSN_AST_DOT_Z:
    case DMNSN_AST_DOT_T:
    case DMNSN_AST_DOT_TRANSMIT:
      dmnsn_make_ast_integer(&ret, n);
      break;

    case DMNSN_AST_NEGATE:
      dmnsn_make_ast_integer(&ret, -n);
      break;

    case DMNSN_AST_ABS:
      dmnsn_make_ast_integer(&ret, labs(n));
      break;
    case DMNSN_AST_ACOS:
      dmnsn_make_ast_float(&ret, acos(n));
      break;
    case DMNSN_AST_ACOSH:
      dmnsn_make_ast_float(&ret, acosh(n));
      break;
    case DMNSN_AST_ASIN:
      dmnsn_make_ast_float(&ret, asin(n));
      break;
    case DMNSN_AST_ASINH:
      dmnsn_make_ast_float(&ret, asinh(n));
      break;
    case DMNSN_AST_ATAN:
      dmnsn_make_ast_float(&ret, atan(n));
      break;
    case DMNSN_AST_ATANH:
      dmnsn_make_ast_float(&ret, atanh(n));
      break;
    case DMNSN_AST_CEIL:
      dmnsn_make_ast_integer(&ret, n);
      break;
    case DMNSN_AST_COS:
      dmnsn_make_ast_float(&ret, cos(n));
      break;
    case DMNSN_AST_COSH:
      dmnsn_make_ast_float(&ret, cosh(n));
      break;
    case DMNSN_AST_DEGREES:
      dmnsn_make_ast_float(&ret, n*45.0/atan(1.0));
      break;
    case DMNSN_AST_EXP:
      dmnsn_make_ast_float(&ret, exp(n));
      break;
    case DMNSN_AST_FLOOR:
      dmnsn_make_ast_integer(&ret, n);
      break;
    case DMNSN_AST_INT:
      dmnsn_make_ast_integer(&ret, n);
      break;
    case DMNSN_AST_LN:
      dmnsn_make_ast_float(&ret, log(n));
      break;
    case DMNSN_AST_LOG:
      dmnsn_make_ast_float(&ret, log(n)/log(10.0));
      break;
    case DMNSN_AST_RADIANS:
      dmnsn_make_ast_float(&ret, n*atan(1.0)/45.0);
      break;
    case DMNSN_AST_SIN:
      dmnsn_make_ast_float(&ret, sin(n));
      break;
    case DMNSN_AST_SINH:
      dmnsn_make_ast_float(&ret, sinh(n));
      break;
    case DMNSN_AST_SQRT:
      dmnsn_make_ast_float(&ret, sqrt(n));
      break;
    case DMNSN_AST_TAN:
      dmnsn_make_ast_float(&ret, tan(n));
      break;
    case DMNSN_AST_TANH:
      dmnsn_make_ast_float(&ret, tanh(n));
      break;
    case DMNSN_AST_VLENGTH:
      dmnsn_make_ast_float(&ret, sqrt(3*n*n));
      break;

    default:
      dmnsn_diagnostic(astnode.filename, astnode.line, astnode.col,
                       "invalid unary operator '%s' on %s",
                       dmnsn_astnode_string(astnode.type),
                       dmnsn_astnode_string(rhs.type));
      ret.type = DMNSN_AST_NONE;
      break;
    }
  } else if (rhs.type == DMNSN_AST_FLOAT) {
    double n = *(double *)rhs.ptr;
    ret = dmnsn_copy_astnode(astnode);

    switch(astnode.type) {
    case DMNSN_AST_DOT_X:
    case DMNSN_AST_DOT_Y:
    case DMNSN_AST_DOT_Z:
    case DMNSN_AST_DOT_T:
    case DMNSN_AST_DOT_TRANSMIT:
      dmnsn_make_ast_float(&ret, n);
      break;

    case DMNSN_AST_NEGATE:
      dmnsn_make_ast_float(&ret, -n);
      break;

    case DMNSN_AST_ABS:
      dmnsn_make_ast_float(&ret, fabs(n));
      break;
    case DMNSN_AST_ACOS:
      dmnsn_make_ast_float(&ret, acos(n));
      break;
    case DMNSN_AST_ACOSH:
      dmnsn_make_ast_float(&ret, acosh(n));
      break;
    case DMNSN_AST_ASIN:
      dmnsn_make_ast_float(&ret, asin(n));
      break;
    case DMNSN_AST_ASINH:
      dmnsn_make_ast_float(&ret, asinh(n));
      break;
    case DMNSN_AST_ATAN:
      dmnsn_make_ast_float(&ret, atan(n));
      break;
    case DMNSN_AST_ATANH:
      dmnsn_make_ast_float(&ret, atanh(n));
      break;
    case DMNSN_AST_CEIL:
      dmnsn_make_ast_maybe_integer(&ret, ceil(n));
      break;
    case DMNSN_AST_COS:
      dmnsn_make_ast_float(&ret, cos(n));
      break;
    case DMNSN_AST_COSH:
      dmnsn_make_ast_float(&ret, cosh(n));
      break;
    case DMNSN_AST_DEGREES:
      dmnsn_make_ast_float(&ret, n*45.0/atan(1.0));
      break;
    case DMNSN_AST_EXP:
      dmnsn_make_ast_float(&ret, exp(n));
      break;
    case DMNSN_AST_FLOOR:
      dmnsn_make_ast_maybe_integer(&ret, floor(n));
      break;
    case DMNSN_AST_INT:
      dmnsn_make_ast_maybe_integer(&ret, trunc(n));
      break;
    case DMNSN_AST_LN:
      dmnsn_make_ast_float(&ret, log(n));
      break;
    case DMNSN_AST_LOG:
      dmnsn_make_ast_float(&ret, log(n)/log(10.0));
      break;
    case DMNSN_AST_RADIANS:
      dmnsn_make_ast_float(&ret, n*atan(1.0)/45.0);
      break;
    case DMNSN_AST_SIN:
      dmnsn_make_ast_float(&ret, sin(n));
      break;
    case DMNSN_AST_SINH:
      dmnsn_make_ast_float(&ret, sinh(n));
      break;
    case DMNSN_AST_SQRT:
      dmnsn_make_ast_float(&ret, sqrt(n));
      break;
    case DMNSN_AST_TAN:
      dmnsn_make_ast_float(&ret, tan(n));
      break;
    case DMNSN_AST_TANH:
      dmnsn_make_ast_float(&ret, tanh(n));
      break;
    case DMNSN_AST_VLENGTH:
      dmnsn_make_ast_float(&ret, sqrt(3.0*n*n));
      break;

    default:
      dmnsn_diagnostic(astnode.filename, astnode.line, astnode.col,
                       "invalid unary operator '%s' on %s",
                       dmnsn_astnode_string(astnode.type),
                       dmnsn_astnode_string(rhs.type));
      ret.type = DMNSN_AST_NONE;
      break;
    }
  } else if (rhs.type == DMNSN_AST_STRING) {
    ret = dmnsn_copy_astnode(astnode);

    switch(astnode.type) {
    case DMNSN_AST_ASC:
      dmnsn_make_ast_integer(&ret, ((char *)rhs.ptr)[0]);
      break;
    case DMNSN_AST_STRLEN:
      dmnsn_make_ast_integer(&ret, strlen(rhs.ptr));
      break;
    case DMNSN_AST_VAL:
      {
        char *endptr;
        long l = strtol(rhs.ptr, &endptr, 0);
        if (*endptr != '\0' || endptr == rhs.ptr) {
          double d = strtod(rhs.ptr, &endptr);
          if (*endptr != '\0' || endptr == rhs.ptr) {
            dmnsn_diagnostic(astnode.filename, astnode.line, astnode.col,
                             "invalid numeric string '%s'",
                             (const char *)rhs.ptr);
            ret.type = DMNSN_AST_NONE;
          } else {
            dmnsn_make_ast_float(&ret, d);
          }
        } else {
          dmnsn_make_ast_integer(&ret, l);
        }
        break;
      }

    default:
      dmnsn_diagnostic(astnode.filename, astnode.line, astnode.col,
                       "invalid unary operator '%s' on %s",
                       dmnsn_astnode_string(astnode.type),
                       dmnsn_astnode_string(rhs.type));
      ret.type = DMNSN_AST_NONE;
      break;
    }
  } else {
    dmnsn_diagnostic(rhs.filename, rhs.line, rhs.col,
                     "expected %s or %s or %s; found %s",
                     dmnsn_astnode_string(DMNSN_AST_INTEGER),
                     dmnsn_astnode_string(DMNSN_AST_FLOAT),
                     dmnsn_astnode_string(DMNSN_AST_STRING),
                     dmnsn_astnode_string(rhs.type));
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

    case DMNSN_AST_VDOT:
      {
        dmnsn_astnode rewrite = dmnsn_copy_astnode(astnode);
        rewrite.type = DMNSN_AST_MUL;

        dmnsn_astnode l, r;
        dmnsn_array_get(lhs.children, 0, &l);
        dmnsn_array_get(rhs.children, 0, &r);
        ++*l.refcount;
        ++*r.refcount;
        dmnsn_array_push(rewrite.children, &l);
        dmnsn_array_push(rewrite.children, &r);

        for (i = 1; i < 3; ++i) {
          dmnsn_astnode temp = dmnsn_copy_astnode(astnode);
          temp.type = DMNSN_AST_MUL;
          dmnsn_array_get(lhs.children, i, &l);
          dmnsn_array_get(rhs.children, i, &r);
          ++*l.refcount;
          ++*r.refcount;
          dmnsn_array_push(temp.children, &l);
          dmnsn_array_push(temp.children, &r);

          dmnsn_astnode next = dmnsn_copy_astnode(astnode);
          next.type = DMNSN_AST_ADD;
          dmnsn_array_push(next.children, &rewrite);
          dmnsn_array_push(next.children, &temp);
          rewrite = next;
        }

        ret = dmnsn_eval_binary(rewrite, symtable);
        dmnsn_delete_astnode(rewrite);
        break;
      }

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
  } else if (lhs.type == DMNSN_AST_STRING && rhs.type == DMNSN_AST_STRING) {
    ret = dmnsn_copy_astnode(astnode);

    switch (astnode.type) {
    case DMNSN_AST_STRCMP:
      dmnsn_make_ast_integer(&ret, strcmp(lhs.ptr, rhs.ptr));
      break;

    default:
      dmnsn_diagnostic(astnode.filename, astnode.line, astnode.col,
                       "invalid binary operator '%s' on %s and %s",
                       dmnsn_astnode_string(astnode.type),
                       dmnsn_astnode_string(lhs.type),
                       dmnsn_astnode_string(rhs.type));
      ret.type = DMNSN_AST_NONE;
      break;
    }
  } else if (lhs.type == DMNSN_AST_INTEGER && rhs.type == DMNSN_AST_INTEGER) {
    ret = dmnsn_copy_astnode(astnode);

    long l, r;
    l = *(long *)lhs.ptr;
    r = *(long *)rhs.ptr;

    switch (astnode.type) {
    case DMNSN_AST_ADD:
      dmnsn_make_ast_integer(&ret, l + r);
      break;
    case DMNSN_AST_SUB:
      dmnsn_make_ast_integer(&ret, l - r);
      break;
    case DMNSN_AST_MUL:
      dmnsn_make_ast_integer(&ret, l*r);
      break;
    case DMNSN_AST_DIV:
      if (l%r == 0) {
        dmnsn_make_ast_integer(&ret, l/r);
      } else {
        dmnsn_make_ast_float(&ret, ((double)l)/r);
      }
      break;

    case DMNSN_AST_EQUAL:
      dmnsn_make_ast_integer(&ret, l == r);
      break;
    case DMNSN_AST_NOT_EQUAL:
      dmnsn_make_ast_integer(&ret, l != r);
      break;
    case DMNSN_AST_LESS:
      dmnsn_make_ast_integer(&ret, l < r);
      break;
    case DMNSN_AST_LESS_EQUAL:
      dmnsn_make_ast_integer(&ret, l <= r);
      break;
    case DMNSN_AST_GREATER:
      dmnsn_make_ast_integer(&ret, l > r);
      break;
    case DMNSN_AST_GREATER_EQUAL:
      dmnsn_make_ast_integer(&ret, l >= r);
      break;
    case DMNSN_AST_AND:
      dmnsn_make_ast_integer(&ret, l && r);
      break;
    case DMNSN_AST_OR:
      dmnsn_make_ast_integer(&ret, l || r);
      break;

    case DMNSN_AST_ATAN2:
      dmnsn_make_ast_float(&ret, atan2(l, r));
      break;
    case DMNSN_AST_INT_DIV:
      dmnsn_make_ast_integer(&ret, l/r);
      break;
    case DMNSN_AST_MOD:
      dmnsn_make_ast_float(&ret, fmod(l, r));
      break;
    case DMNSN_AST_POW:
      dmnsn_make_ast_float(&ret, pow(l, r));
      break;
    case DMNSN_AST_VDOT:
      dmnsn_make_ast_integer(&ret, 3*l*r);
      break;

    default:
      dmnsn_diagnostic(astnode.filename, astnode.line, astnode.col,
                       "invalid binary operator '%s' on %s and %s",
                       dmnsn_astnode_string(astnode.type),
                       dmnsn_astnode_string(lhs.type),
                       dmnsn_astnode_string(rhs.type));
      ret.type = DMNSN_AST_NONE;
      break;
    }
  } else {
    ret = dmnsn_copy_astnode(astnode);

    double l, r;

    if (lhs.type == DMNSN_AST_INTEGER) {
      l = *(long *)lhs.ptr;
    } else if (lhs.type == DMNSN_AST_FLOAT) {
      l = *(double *)lhs.ptr;
    } else {
      dmnsn_diagnostic(lhs.filename, lhs.line, lhs.col,
                       "expected %s or %s; found %s",
                       dmnsn_astnode_string(DMNSN_AST_INTEGER),
                       dmnsn_astnode_string(DMNSN_AST_FLOAT),
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
                       "expected %s or %s; found %s",
                       dmnsn_astnode_string(DMNSN_AST_INTEGER),
                       dmnsn_astnode_string(DMNSN_AST_FLOAT),
                       dmnsn_astnode_string(rhs.type));
      ret.type = DMNSN_AST_NONE;
      dmnsn_delete_astnode(lhs);
      dmnsn_delete_astnode(rhs);
      return ret;
    }

    switch (astnode.type) {
    case DMNSN_AST_ADD:
      dmnsn_make_ast_float(&ret, l + r);
      break;
    case DMNSN_AST_SUB:
      dmnsn_make_ast_float(&ret, l - r);
      break;
    case DMNSN_AST_MUL:
      dmnsn_make_ast_float(&ret, l*r);
      break;
    case DMNSN_AST_DIV:
      dmnsn_make_ast_float(&ret, l/r);
      break;

    case DMNSN_AST_EQUAL:
      dmnsn_make_ast_integer(&ret, fabs(l - r) < dmnsn_epsilon);
      break;
    case DMNSN_AST_NOT_EQUAL:
      dmnsn_make_ast_integer(&ret, fabs(l - r) >= dmnsn_epsilon);
      break;
    case DMNSN_AST_LESS:
      dmnsn_make_ast_integer(&ret, l < r);
      break;
    case DMNSN_AST_LESS_EQUAL:
      dmnsn_make_ast_integer(&ret, l <= r);
      break;
    case DMNSN_AST_GREATER:
      dmnsn_make_ast_integer(&ret, l > r);
      break;
    case DMNSN_AST_GREATER_EQUAL:
      dmnsn_make_ast_integer(&ret, l >= r);
      break;
    case DMNSN_AST_AND:
      dmnsn_make_ast_integer(&ret, fabs(l) >= dmnsn_epsilon
                                   && fabs(r) >= dmnsn_epsilon);
      break;
    case DMNSN_AST_OR:
      dmnsn_make_ast_integer(&ret, fabs(l) >= dmnsn_epsilon
                                   || fabs(r) >= dmnsn_epsilon);
      break;

    case DMNSN_AST_ATAN2:
      dmnsn_make_ast_float(&ret, atan2(l, r));
      break;
    case DMNSN_AST_INT_DIV:
      dmnsn_make_ast_float(&ret, trunc(l/r));
      break;
    case DMNSN_AST_MOD:
      dmnsn_make_ast_float(&ret, fmod(l, r));
      break;
    case DMNSN_AST_POW:
      dmnsn_make_ast_float(&ret, pow(l, r));
      break;
    case DMNSN_AST_VDOT:
      dmnsn_make_ast_float(&ret, 3.0*l*r);
      break;

    default:
      dmnsn_diagnostic(astnode.filename, astnode.line, astnode.col,
                       "invalid binary operator '%s' on %s and %s",
                       dmnsn_astnode_string(astnode.type),
                       dmnsn_astnode_string(lhs.type),
                       dmnsn_astnode_string(rhs.type));
      ret.type = DMNSN_AST_NONE;
      break;
    }
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
  case DMNSN_AST_STRING:
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
        dmnsn_astnode id = *symbol;
        id.filename = astnode.filename;
        id.line     = astnode.line;
        id.col      = astnode.col;
        return dmnsn_eval(id, symtable);
      } else {
        dmnsn_diagnostic(astnode.filename, astnode.line, astnode.col,
                         "unbound identifier '%s'", (const char *)astnode.ptr);
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
  case DMNSN_AST_ABS:
  case DMNSN_AST_ACOS:
  case DMNSN_AST_ACOSH:
  case DMNSN_AST_ASC:
  case DMNSN_AST_ASIN:
  case DMNSN_AST_ASINH:
  case DMNSN_AST_ATAN:
  case DMNSN_AST_ATANH:
  case DMNSN_AST_CEIL:
  case DMNSN_AST_COS:
  case DMNSN_AST_COSH:
  case DMNSN_AST_DEGREES:
  case DMNSN_AST_EXP:
  case DMNSN_AST_FLOOR:
  case DMNSN_AST_INT:
  case DMNSN_AST_LN:
  case DMNSN_AST_LOG:
  case DMNSN_AST_RADIANS:
  case DMNSN_AST_SIN:
  case DMNSN_AST_SINH:
  case DMNSN_AST_SQRT:
  case DMNSN_AST_STRLEN:
  case DMNSN_AST_TAN:
  case DMNSN_AST_TANH:
  case DMNSN_AST_VAL:
  case DMNSN_AST_VLENGTH:
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
  case DMNSN_AST_ATAN2:
  case DMNSN_AST_INT_DIV:
  case DMNSN_AST_MOD:
  case DMNSN_AST_POW:
  case DMNSN_AST_STRCMP:
  case DMNSN_AST_VDOT:
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
  if (ret.type != DMNSN_AST_INTEGER && ret.type != DMNSN_AST_FLOAT) {
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
