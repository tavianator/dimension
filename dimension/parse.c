/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
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
#include <errno.h>
#include <fenv.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * Symbol table
 */

dmnsn_symbol_table *
dmnsn_new_symbol_table(void)
{
  dmnsn_symbol_table *symtable = dmnsn_new_array(sizeof(dmnsn_dictionary *));
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
  dmnsn_dictionary *scope = dmnsn_new_dictionary(sizeof(dmnsn_astnode));
  dmnsn_array_push(symtable, &scope);
}

static void
dmnsn_delete_symbol_table_entry(void *ptr)
{
  dmnsn_astnode *astnode = ptr;
  dmnsn_delete_astnode(*astnode);
}

void dmnsn_pop_scope(dmnsn_symbol_table *symtable)
{
  dmnsn_dictionary *scope;
  dmnsn_array_pop(symtable, &scope);
  dmnsn_dictionary_apply(scope, dmnsn_delete_symbol_table_entry);
  dmnsn_delete_dictionary(scope);
}

void dmnsn_local_symbol(dmnsn_symbol_table *symtable,
                        const char *id, dmnsn_astnode value)
{
  ++*value.refcount;

  dmnsn_dictionary *dict;
  dmnsn_array_get(symtable, dmnsn_array_size(symtable) - 1, &dict);
  dmnsn_astnode *node = dmnsn_dictionary_at(dict, id);
  if (node) {
    dmnsn_delete_astnode(*node);
    *node = value;
  } else {
    dmnsn_dictionary_insert(dict, id, &value);
  }
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
    dmnsn_dictionary *dict;
    dmnsn_array_get(symtable, 0, &dict);

    node = dmnsn_dictionary_at(dict, id);
    if (node) {
      dmnsn_delete_astnode(*node);
      *node = value;
    } else {
      dmnsn_dictionary_insert(dict, id, &value);
    }
  }
}

void
dmnsn_undef_symbol(dmnsn_symbol_table *symtable, const char *id)
{
  DMNSN_ARRAY_FOREACH (dmnsn_dictionary **, dict, symtable) {
    dmnsn_astnode *node = dmnsn_dictionary_at(*dict, id);
    if (node) {
      dmnsn_delete_astnode(*node);
      dmnsn_dictionary_remove(*dict, id);
      break;
    }
  }
}

dmnsn_astnode *
dmnsn_find_symbol(dmnsn_symbol_table *symtable, const char *id)
{
  dmnsn_astnode *symbol = NULL;

  DMNSN_ARRAY_FOREACH_REVERSE (dmnsn_dictionary **, dict, symtable) {
    symbol = dmnsn_dictionary_at(*dict, id);
    if (symbol) {
      if (symbol->type == DMNSN_AST_IDENTIFIER) {
        id = symbol->ptr;
      } else {
        break;
      }
    }
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
    .free_fn  = NULL,
    .refcount = dmnsn_malloc(sizeof(unsigned int)),
    .location = { "<environment>", "<environment>", -1, -1, -1, -1, NULL }
  };

  *astnode.refcount = 0;
  return astnode;
}

dmnsn_astnode
dmnsn_new_ast_array(void)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_ARRAY);
  astnode.children = dmnsn_new_array(sizeof(dmnsn_astnode));
  return astnode;
}

static void
dmnsn_make_ast_integer(dmnsn_astnode *astnode, long value)
{
  astnode->type = DMNSN_AST_INTEGER;
  astnode->ptr = dmnsn_malloc(sizeof(long));
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
  astnode->ptr = dmnsn_malloc(sizeof(double));
  *(double *)astnode->ptr = value;
}

dmnsn_astnode
dmnsn_new_ast_float(double value)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_FLOAT);
  dmnsn_make_ast_float(&astnode, value);
  return astnode;
}

static void
dmnsn_make_ast_ivector(dmnsn_astnode *astnode,
                       long x, long y, long z, long f, long t)
{
  astnode->type = DMNSN_AST_VECTOR;
  if (!astnode->children)
    astnode->children = dmnsn_new_array(sizeof(dmnsn_astnode));

  dmnsn_astnode comp;

  comp = dmnsn_new_ast_integer(x);
  dmnsn_array_push(astnode->children, &comp);

  comp = dmnsn_new_ast_integer(y);
  dmnsn_array_push(astnode->children, &comp);

  comp = dmnsn_new_ast_integer(z);
  dmnsn_array_push(astnode->children, &comp);

  comp = dmnsn_new_ast_integer(f);
  dmnsn_array_push(astnode->children, &comp);

  comp = dmnsn_new_ast_integer(t);
  dmnsn_array_push(astnode->children, &comp);
}

dmnsn_astnode
dmnsn_new_ast_ivector(long x, long y, long z, long f, long t)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_VECTOR);
  dmnsn_make_ast_ivector(&astnode, x, y, z, f, t);
  return astnode;
}

static void
dmnsn_make_ast_vector(dmnsn_astnode *astnode,
                      double x, double y, double z, double f, double t)
{
  astnode->type = DMNSN_AST_VECTOR;
  if (!astnode->children)
    astnode->children = dmnsn_new_array(sizeof(dmnsn_astnode));

  dmnsn_astnode comp;

  comp = dmnsn_new_ast_float(x);
  dmnsn_array_push(astnode->children, &comp);

  comp = dmnsn_new_ast_float(y);
  dmnsn_array_push(astnode->children, &comp);

  comp = dmnsn_new_ast_float(z);
  dmnsn_array_push(astnode->children, &comp);

  comp = dmnsn_new_ast_float(f);
  dmnsn_array_push(astnode->children, &comp);

  comp = dmnsn_new_ast_float(t);
  dmnsn_array_push(astnode->children, &comp);
}

dmnsn_astnode
dmnsn_new_ast_vector(double x, double y, double z, double f, double t)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_VECTOR);
  dmnsn_make_ast_vector(&astnode, x, y, z, f, t);
  return astnode;
}

dmnsn_astnode
dmnsn_new_ast_string(const char *value)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_STRING);
  astnode.ptr = dmnsn_strdup(value);
  return astnode;
}

void
dmnsn_delete_astnode(dmnsn_astnode astnode)
{
  if (*astnode.refcount <= 1) {
    dmnsn_delete_astree(astnode.children);
    if (astnode.free_fn) {
      astnode.free_fn(astnode.ptr);
    } else {
      dmnsn_free(astnode.ptr);
    }
    dmnsn_free(astnode.refcount);
  } else {
    --*astnode.refcount;
  }
}

void
dmnsn_delete_astree(dmnsn_astree *astree)
{
  if (astree) {
    DMNSN_ARRAY_FOREACH (dmnsn_astnode *, node, astree) {
      dmnsn_delete_astnode(*node);
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
    .refcount = dmnsn_malloc(sizeof(unsigned int)),
    .location = astnode.location
  };

  *copy.refcount = 1;
  return copy;
}

static dmnsn_astnode
dmnsn_new_astnode2(dmnsn_astnode_type type, dmnsn_astnode loc,
                   dmnsn_astnode n1, dmnsn_astnode n2)
{
  dmnsn_astnode astnode = dmnsn_copy_astnode(loc);
  astnode.type = type;
  dmnsn_array_push(astnode.children, &n1);
  dmnsn_array_push(astnode.children, &n2);
  return astnode;
}

/* 5-element vectors */
#define DMNSN_VECTOR_NELEM 5

static dmnsn_astnode
dmnsn_vector_promote(dmnsn_astnode astnode, dmnsn_symbol_table *symtable)
{
  dmnsn_astnode promoted = dmnsn_copy_astnode(astnode);

  if (astnode.type == DMNSN_AST_VECTOR) {
    dmnsn_astnode component;
    DMNSN_ARRAY_FOREACH (dmnsn_astnode *, i, astnode.children) {
      component = dmnsn_eval_scalar(*i, symtable);

      if (component.type == DMNSN_AST_NONE) {
        dmnsn_delete_astnode(promoted);
        return component;
      } else {
        dmnsn_array_push(promoted.children, &component);
      }
    }

    while (dmnsn_array_size(promoted.children) < DMNSN_VECTOR_NELEM) {
      component = dmnsn_copy_astnode(component);
      dmnsn_make_ast_integer(&component, 0);
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
dmnsn_eval_zeroary(dmnsn_astnode astnode, dmnsn_symbol_table *symtable)
{
  dmnsn_astnode ret = dmnsn_copy_astnode(astnode);

  switch (astnode.type) {
  case DMNSN_AST_PI:
    dmnsn_make_ast_float(&ret, 4.0*atan(1.0));
    break;
  case DMNSN_AST_TRUE:
    dmnsn_make_ast_integer(&ret, 1);
    break;
  case DMNSN_AST_FALSE:
    dmnsn_make_ast_integer(&ret, 0);
    break;

  case DMNSN_AST_X:
    dmnsn_make_ast_ivector(&ret, 1, 0, 0, 0, 0);
    break;
  case DMNSN_AST_Y:
    dmnsn_make_ast_ivector(&ret, 0, 1, 0, 0, 0);
    break;
  case DMNSN_AST_Z:
    dmnsn_make_ast_ivector(&ret, 0, 0, 1, 0, 0);
    break;
  case DMNSN_AST_T:
    dmnsn_make_ast_ivector(&ret, 0, 0, 0, 1, 0);
    break;

  default:
    dmnsn_diagnostic(astnode.location, "invalid constant '%s'",
                     dmnsn_astnode_string(astnode.type));
    ret.type = DMNSN_AST_NONE;
    break;
  }

  return ret;
}

static dmnsn_astnode
dmnsn_eval_unary(dmnsn_astnode astnode, dmnsn_symbol_table *symtable)
{
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

        ret = dmnsn_eval(rewrite, symtable);
        dmnsn_delete_astnode(rewrite);
        break;
      }
    case DMNSN_AST_VNORMALIZE:
      {
        dmnsn_astnode norm = dmnsn_copy_astnode(astnode);
        norm.type = DMNSN_AST_VLENGTH;
        ++*rhs.refcount;
        dmnsn_array_push(norm.children, &rhs);

        dmnsn_astnode rewrite = dmnsn_copy_astnode(astnode);
        rewrite.type = DMNSN_AST_DIV;
        ++*rhs.refcount;
        dmnsn_array_push(rewrite.children, &rhs);
        dmnsn_array_push(rewrite.children, &norm);

        ret = dmnsn_eval(rewrite, symtable);
        dmnsn_delete_astnode(rewrite);
        break;
      }

    default:
      {
        ret = dmnsn_copy_astnode(astnode);
        ret.type = DMNSN_AST_VECTOR;

        dmnsn_astnode op = dmnsn_copy_astnode(astnode);
        dmnsn_array_resize(op.children, 1);
        for (size_t i = 0; i < DMNSN_VECTOR_NELEM; ++i) {
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

    switch (astnode.type) {
    case DMNSN_AST_DOT_X:
    case DMNSN_AST_DOT_Y:
    case DMNSN_AST_DOT_Z:
    case DMNSN_AST_DOT_T:
    case DMNSN_AST_DOT_TRANSMIT:
      dmnsn_make_ast_integer(&ret, n);
      break;

    case DMNSN_AST_NEGATE:
      if ((n >= 0 && LONG_MIN + n <= 0) || (n < 0 && LONG_MAX + n >= 0)) {
        dmnsn_make_ast_integer(&ret, -n);
      } else {
        dmnsn_make_ast_float(&ret, -(double)n);
      }
      break;

    case DMNSN_AST_NOT:
      dmnsn_make_ast_integer(&ret, !n);
      break;

    case DMNSN_AST_ABS:
      if (n >= 0 || LONG_MAX + n >= 0) {
        dmnsn_make_ast_integer(&ret, labs(n));
      } else {
        dmnsn_make_ast_float(&ret, fabs(n));
      }
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
      dmnsn_make_ast_float(&ret, dmnsn_degrees(n));
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
      dmnsn_make_ast_float(&ret, dmnsn_radians(n));
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
    case DMNSN_AST_VNORMALIZE:
      {
        double elem = 1.0/sqrt(DMNSN_VECTOR_NELEM);
        dmnsn_make_ast_vector(&ret, elem, elem, elem, elem, elem);
        break;
      }

    default:
      dmnsn_diagnostic(astnode.location, "invalid unary operator '%s' on %s",
                       dmnsn_astnode_string(astnode.type),
                       dmnsn_astnode_string(rhs.type));
      ret.type = DMNSN_AST_NONE;
      break;
    }
  } else if (rhs.type == DMNSN_AST_FLOAT) {
    double n = *(double *)rhs.ptr;
    ret = dmnsn_copy_astnode(astnode);

    switch (astnode.type) {
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

    case DMNSN_AST_NOT:
      dmnsn_make_ast_integer(&ret, !n);
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
      dmnsn_make_ast_float(&ret, dmnsn_degrees(n));
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
      dmnsn_make_ast_float(&ret, dmnsn_radians(n));
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
    case DMNSN_AST_VNORMALIZE:
      {
        double elem = 1.0/sqrt(DMNSN_VECTOR_NELEM);
        dmnsn_make_ast_vector(&ret, elem, elem, elem, elem, elem);
        break;
      }

    default:
      dmnsn_diagnostic(astnode.location, "invalid unary operator '%s' on %s",
                       dmnsn_astnode_string(astnode.type),
                       dmnsn_astnode_string(rhs.type));
      ret.type = DMNSN_AST_NONE;
      break;
    }
  } else if (rhs.type == DMNSN_AST_STRING) {
    ret = dmnsn_copy_astnode(astnode);

    switch (astnode.type) {
    case DMNSN_AST_ASC:
      dmnsn_make_ast_integer(&ret, ((char *)rhs.ptr)[0]);
      break;
    case DMNSN_AST_STRLEN:
      dmnsn_make_ast_integer(&ret, strlen(rhs.ptr));
      break;
    case DMNSN_AST_VAL:
      {
        long l;
        if (dmnsn_strtol(&l, rhs.ptr, 0)) {
          dmnsn_make_ast_integer(&ret, l);
        } else {
          double d;
          if (dmnsn_strtod(&d, rhs.ptr)) {
            dmnsn_make_ast_float(&ret, d);
          } else if (errno == ERANGE) {
            dmnsn_diagnostic(astnode.location, "float value overflowed");
            ret.type = DMNSN_AST_NONE;
          } else {
            dmnsn_diagnostic(astnode.location, "invalid numeric string '%s'",
                             (const char *)rhs.ptr);
            ret.type = DMNSN_AST_NONE;
          }
        }
        break;
      }

    default:
      dmnsn_diagnostic(astnode.location, "invalid unary operator '%s' on %s",
                       dmnsn_astnode_string(astnode.type),
                       dmnsn_astnode_string(rhs.type));
      ret.type = DMNSN_AST_NONE;
      break;
    }
  } else {
    dmnsn_diagnostic(rhs.location, "expected %s or %s or %s; found %s",
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

    if (lhs.type == DMNSN_AST_NONE || rhs.type == DMNSN_AST_NONE) {
      ret = dmnsn_copy_astnode(astnode);
      ret.type = DMNSN_AST_NONE;
      dmnsn_delete_astnode(lhs);
      dmnsn_delete_astnode(rhs);
      return ret;
    }

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

        for (size_t i = 1; i < DMNSN_VECTOR_NELEM; ++i) {
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

        for (size_t i = 1; i < DMNSN_VECTOR_NELEM; ++i) {
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

        for (size_t i = 1; i < DMNSN_VECTOR_NELEM; ++i) {
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

        for (size_t i = 1; i < DMNSN_VECTOR_NELEM; ++i) {
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
      dmnsn_diagnostic(astnode.location,
                       "invalid comparison operator '%s' between vectors",
                       dmnsn_astnode_string(astnode.type));
      ret = dmnsn_copy_astnode(astnode);
      ret.type = DMNSN_AST_NONE;
      break;

    case DMNSN_AST_VAXIS_ROTATE:
      {
        dmnsn_astnode rx, ry, rz;
        dmnsn_array_get(lhs.children, 0, &rx);
        dmnsn_array_get(lhs.children, 1, &ry);
        dmnsn_array_get(lhs.children, 2, &rz);
        dmnsn_astnode ax, ay, az;
        dmnsn_array_get(rhs.children, 0, &ax);
        dmnsn_array_get(rhs.children, 1, &ay);
        dmnsn_array_get(rhs.children, 2, &az);

        dmnsn_vector r = dmnsn_new_vector(
          rx.type == DMNSN_AST_INTEGER ? *(long *)rx.ptr : *(double *)rx.ptr,
          ry.type == DMNSN_AST_INTEGER ? *(long *)ry.ptr : *(double *)ry.ptr,
          rz.type == DMNSN_AST_INTEGER ? *(long *)rz.ptr : *(double *)rz.ptr
        );
        dmnsn_vector axis = dmnsn_new_vector(
          ax.type == DMNSN_AST_INTEGER ? *(long *)ax.ptr : *(double *)ax.ptr,
          ay.type == DMNSN_AST_INTEGER ? *(long *)ay.ptr : *(double *)ay.ptr,
          az.type == DMNSN_AST_INTEGER ? *(long *)az.ptr : *(double *)az.ptr
        );

        axis = dmnsn_vector_mul(dmnsn_radians(1.0), axis);
        r = dmnsn_transform_vector(dmnsn_rotation_matrix(axis), r);

        ret = dmnsn_copy_astnode(astnode);
        dmnsn_make_ast_vector(&ret, r.x, r.y, r.z, 0.0, 0.0);
        break;
      }
    case DMNSN_AST_VCROSS:
      {
        dmnsn_astnode ux, uy, uz;
        dmnsn_array_get(lhs.children, 0, &ux);
        dmnsn_array_get(lhs.children, 1, &uy);
        dmnsn_array_get(lhs.children, 2, &uz);
        *ux.refcount += 2;
        *uy.refcount += 2;
        *uz.refcount += 2;
        dmnsn_astnode vx, vy, vz;
        dmnsn_array_get(rhs.children, 0, &vx);
        dmnsn_array_get(rhs.children, 1, &vy);
        dmnsn_array_get(rhs.children, 2, &vz);
        *vx.refcount += 2;
        *vy.refcount += 2;
        *vz.refcount += 2;

        dmnsn_astnode uvx = dmnsn_new_astnode2(
          DMNSN_AST_SUB, astnode,
          dmnsn_new_astnode2(DMNSN_AST_MUL, astnode, uy, vz),
          dmnsn_new_astnode2(DMNSN_AST_MUL, astnode, uz, vy)
        );
        dmnsn_astnode uvy = dmnsn_new_astnode2(
          DMNSN_AST_SUB, astnode,
          dmnsn_new_astnode2(DMNSN_AST_MUL, astnode, uz, vx),
          dmnsn_new_astnode2(DMNSN_AST_MUL, astnode, ux, vz)
        );
        dmnsn_astnode uvz = dmnsn_new_astnode2(
          DMNSN_AST_SUB, astnode,
          dmnsn_new_astnode2(DMNSN_AST_MUL, astnode, ux, vy),
          dmnsn_new_astnode2(DMNSN_AST_MUL, astnode, uy, vx)
        );

        dmnsn_astnode rewrite = dmnsn_copy_astnode(astnode);
        rewrite.type = DMNSN_AST_VECTOR;
        dmnsn_array_push(rewrite.children, &uvx);
        dmnsn_array_push(rewrite.children, &uvy);
        dmnsn_array_push(rewrite.children, &uvz);

        ret = dmnsn_eval(rewrite, symtable);
        dmnsn_delete_astnode(rewrite);
        break;
      }
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

        for (size_t i = 1; i < 3; ++i) {
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
    case DMNSN_AST_VROTATE:
      {
        dmnsn_astnode rx, ry, rz;
        dmnsn_array_get(lhs.children, 0, &rx);
        dmnsn_array_get(lhs.children, 1, &ry);
        dmnsn_array_get(lhs.children, 2, &rz);
        dmnsn_astnode ax, ay, az;
        dmnsn_array_get(rhs.children, 0, &ax);
        dmnsn_array_get(rhs.children, 1, &ay);
        dmnsn_array_get(rhs.children, 2, &az);

        dmnsn_vector r = dmnsn_new_vector(
          rx.type == DMNSN_AST_INTEGER ? *(long *)rx.ptr : *(double *)rx.ptr,
          ry.type == DMNSN_AST_INTEGER ? *(long *)ry.ptr : *(double *)ry.ptr,
          rz.type == DMNSN_AST_INTEGER ? *(long *)rz.ptr : *(double *)rz.ptr
        );
        dmnsn_vector axis = dmnsn_new_vector(
          ax.type == DMNSN_AST_INTEGER ? *(long *)ax.ptr : *(double *)ax.ptr,
          ay.type == DMNSN_AST_INTEGER ? *(long *)ay.ptr : *(double *)ay.ptr,
          az.type == DMNSN_AST_INTEGER ? *(long *)az.ptr : *(double *)az.ptr
        );

        axis = dmnsn_vector_mul(dmnsn_radians(1.0), axis);

        r = dmnsn_transform_vector(
          dmnsn_rotation_matrix(dmnsn_new_vector(axis.x, 0.0, 0.0)),
          r
        );
        r = dmnsn_transform_vector(
          dmnsn_rotation_matrix(dmnsn_new_vector(0.0, axis.y, 0.0)),
          r
        );
        r = dmnsn_transform_vector(
          dmnsn_rotation_matrix(dmnsn_new_vector(0.0, 0.0, axis.z)),
          r
        );

        ret = dmnsn_copy_astnode(astnode);
        dmnsn_make_ast_vector(&ret, r.x, r.y, r.z, 0.0, 0.0);
        break;
      }

    default:
      {
        ret = dmnsn_copy_astnode(astnode);
        ret.type = DMNSN_AST_VECTOR;

        dmnsn_astnode op = dmnsn_copy_astnode(astnode);
        dmnsn_array_resize(op.children, 2);
        for (size_t i = 0; i < DMNSN_VECTOR_NELEM; ++i) {
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
      dmnsn_diagnostic(astnode.location,
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
      if ((r < 0 && LONG_MIN - r <= l) || (r >= 0 && LONG_MAX - r >= l)) {
        dmnsn_make_ast_integer(&ret, l + r);
      } else {
        dmnsn_make_ast_float(&ret, (double)l + r);
      }
      break;
    case DMNSN_AST_SUB:
      if ((r >= 0 && LONG_MIN + r <= l) || (r < 0 && LONG_MAX + r >= l)) {
        dmnsn_make_ast_integer(&ret, l - r);
      } else {
        dmnsn_make_ast_float(&ret, (double)l - r);
      }
      break;
    case DMNSN_AST_MUL:
      {
#if (ULONG_MAX > 4294967295)
        __int128_t prod = l;
#else
        int64_t prod = l;
#endif
        prod *= r;
        if (prod >= LONG_MIN && prod <= LONG_MAX) {
          dmnsn_make_ast_integer(&ret, prod);
        } else {
          dmnsn_make_ast_float(&ret, prod);
        }
        break;
      }
    case DMNSN_AST_DIV:
      if (r == -1
          && ((l >= 0 && LONG_MIN + l <= 0) || (l < 0 && LONG_MAX + l >= 0)))
      {
        dmnsn_make_ast_float(&ret, -(double)l);
      } else if (r == 0) {
        dmnsn_diagnostic(astnode.location, "WARNING: division by zero");
        dmnsn_make_ast_float(&ret, (double)l/0.0);
      } else if (l%r == 0) {
        dmnsn_make_ast_integer(&ret, l/r);
      } else {
        dmnsn_make_ast_float(&ret, (double)l/(double)r);
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
      if (r == -1
          && ((l >= 0 && LONG_MIN + l <= 0) || (l < 0 && LONG_MAX + l >= 0)))
      {
        dmnsn_make_ast_float(&ret, -(double)l);
      } else if (r == 0) {
        dmnsn_diagnostic(astnode.location, "WARNING: division by zero");
        dmnsn_make_ast_float(&ret, (double)l/0.0);
      } else {
        dmnsn_make_ast_integer(&ret, l/r);
      }
      break;
    case DMNSN_AST_MAX:
      dmnsn_make_ast_integer(&ret, l > r ? l : r);
      break;
    case DMNSN_AST_MIN:
      dmnsn_make_ast_integer(&ret, l < r ? l : r);
      break;
    case DMNSN_AST_MOD:
      if (r == -1) {
        /* LONG_MIN % -1 is potentially an overflow */
        dmnsn_make_ast_integer(&ret, 0);
      } else if (r == 0) {
        dmnsn_diagnostic(astnode.location, "WARNING: division by zero");
        dmnsn_make_ast_float(&ret, fmod(l, 0.0));
      } else {
        dmnsn_make_ast_integer(&ret, l%r);
      }
      break;
    case DMNSN_AST_POW:
      dmnsn_make_ast_float(&ret, pow(l, r));
      break;
    case DMNSN_AST_VCROSS:
      dmnsn_make_ast_integer(&ret, 0);
      break;
    case DMNSN_AST_VDOT:
      dmnsn_make_ast_integer(&ret, 3*l*r);
      break;
    case DMNSN_AST_VROTATE:
      ret.type = DMNSN_AST_VECTOR;
      *lhs.refcount += 3;
      dmnsn_array_push(ret.children, &lhs);
      dmnsn_array_push(ret.children, &lhs);
      dmnsn_array_push(ret.children, &lhs);
      break;

    default:
      dmnsn_diagnostic(astnode.location,
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
      dmnsn_diagnostic(lhs.location, "expected %s or %s; found %s",
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
      dmnsn_diagnostic(rhs.location, "expected %s or %s; found %s",
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
      if (r == 0.0)
        dmnsn_diagnostic(astnode.location, "WARNING: division by zero");
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
      if (r == 0.0)
        dmnsn_diagnostic(astnode.location, "WARNING: division by zero");
      dmnsn_make_ast_maybe_integer(&ret, trunc(l/r));
      break;
    case DMNSN_AST_MAX:
      if (l > r) {
        if (lhs.type == DMNSN_AST_INTEGER) {
          dmnsn_make_ast_maybe_integer(&ret, l);
        } else {
          dmnsn_make_ast_float(&ret, l);
        }
      } else {
        if (rhs.type == DMNSN_AST_INTEGER) {
          dmnsn_make_ast_maybe_integer(&ret, r);
        } else {
          dmnsn_make_ast_float(&ret, r);
        }
      }
      break;
    case DMNSN_AST_MIN:
      if (l < r) {
        if (lhs.type == DMNSN_AST_INTEGER) {
          dmnsn_make_ast_maybe_integer(&ret, l);
        } else {
          dmnsn_make_ast_float(&ret, l);
        }
      } else {
        if (rhs.type == DMNSN_AST_INTEGER) {
          dmnsn_make_ast_maybe_integer(&ret, r);
        } else {
          dmnsn_make_ast_float(&ret, r);
        }
      }
      break;
    case DMNSN_AST_MOD:
      if (r == 0.0)
        dmnsn_diagnostic(astnode.location, "WARNING: division by zero");
      dmnsn_make_ast_float(&ret, fmod(l, r));
      break;
    case DMNSN_AST_POW:
      dmnsn_make_ast_float(&ret, pow(l, r));
      break;
    case DMNSN_AST_VCROSS:
      dmnsn_make_ast_integer(&ret, 0);
      break;
    case DMNSN_AST_VDOT:
      dmnsn_make_ast_float(&ret, 3.0*l*r);
      break;
    case DMNSN_AST_VROTATE:
      ret.type = DMNSN_AST_VECTOR;
      *lhs.refcount += 3;
      dmnsn_array_push(ret.children, &lhs);
      dmnsn_array_push(ret.children, &lhs);
      dmnsn_array_push(ret.children, &lhs);
      break;

    default:
      dmnsn_diagnostic(astnode.location,
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

static dmnsn_astnode
dmnsn_eval_ternary(dmnsn_astnode astnode, dmnsn_symbol_table *symtable)
{
  dmnsn_astnode test, iftrue, iffalse, ret;
  dmnsn_array_get(astnode.children, 0, &test);
  dmnsn_array_get(astnode.children, 1, &iftrue);
  dmnsn_array_get(astnode.children, 2, &iffalse);
  test = dmnsn_eval_scalar(test, symtable);

  switch (astnode.type) {
  case DMNSN_AST_TERNARY:
    dmnsn_assert(test.type == DMNSN_AST_INTEGER,
                 "Conditional expression evaluated to non-integer.");
    ret = dmnsn_eval(*(long *)test.ptr ? iftrue : iffalse, symtable);
    break;

  default:
    dmnsn_diagnostic(astnode.location, "invalid ternary operator '%s'",
                     dmnsn_astnode_string(astnode.type));
    ret = dmnsn_copy_astnode(astnode);
    ret.type = DMNSN_AST_NONE;
    break;
  }

  dmnsn_delete_astnode(test);
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
        id.location = astnode.location;
        return dmnsn_eval(id, symtable);
      } else {
        dmnsn_diagnostic(astnode.location, "unbound identifier '%s'",
                         (const char *)astnode.ptr);
        dmnsn_astnode error = dmnsn_new_astnode(DMNSN_AST_NONE);
        ++*error.refcount;
        return error;
      }
    }

  case DMNSN_AST_PI:
  case DMNSN_AST_TRUE:
  case DMNSN_AST_FALSE:
  case DMNSN_AST_X:
  case DMNSN_AST_Y:
  case DMNSN_AST_Z:
  case DMNSN_AST_T:
    return dmnsn_eval_zeroary(astnode, symtable);

  case DMNSN_AST_DOT_X:
  case DMNSN_AST_DOT_Y:
  case DMNSN_AST_DOT_Z:
  case DMNSN_AST_DOT_T:
  case DMNSN_AST_DOT_TRANSMIT:
  case DMNSN_AST_NEGATE:
  case DMNSN_AST_NOT:
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
  case DMNSN_AST_VNORMALIZE:
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
  case DMNSN_AST_MAX:
  case DMNSN_AST_MIN:
  case DMNSN_AST_MOD:
  case DMNSN_AST_POW:
  case DMNSN_AST_STRCMP:
  case DMNSN_AST_VAXIS_ROTATE:
  case DMNSN_AST_VCROSS:
  case DMNSN_AST_VDOT:
  case DMNSN_AST_VROTATE:
    return dmnsn_eval_binary(astnode, symtable);

  case DMNSN_AST_TERNARY:
    return dmnsn_eval_ternary(astnode, symtable);

  default:
    dmnsn_diagnostic(astnode.location,
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
  if (ret.type != DMNSN_AST_INTEGER && ret.type != DMNSN_AST_FLOAT
      && ret.type != DMNSN_AST_NONE)
  {
    dmnsn_diagnostic(ret.location, "expected %s or %s; found %s",
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
    /* Don't print -0 */
    if (dvalue == 0.0)
      fprintf(file, "(%s 0)", dmnsn_astnode_string(astnode.type));
    else
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
  if (astnode.children && dmnsn_array_size(astnode.children) > 0) {
    fprintf(file, "(");
    dmnsn_print_astnode(file, astnode);
    DMNSN_ARRAY_FOREACH (dmnsn_astnode *, child, astnode.children) {
      fprintf(file, " ");
      dmnsn_print_astree(file, *child);
    }
    fprintf(file, ")");
  } else {
    dmnsn_print_astnode(file, astnode);
  }
}

void
dmnsn_print_astree_sexpr(FILE *file, const dmnsn_astree *astree)
{
  if (dmnsn_array_size(astree) == 0) {
    fprintf(file, "()");
  } else {
    fprintf(file, "(");
    dmnsn_astnode *astnode = dmnsn_array_first(astree);
    dmnsn_print_astree(file, *astnode);

    for (++astnode;
         astnode <= (dmnsn_astnode *)dmnsn_array_last(astree);
         ++astnode)
    {
      fprintf(file, " ");
      dmnsn_print_astree(file, *astnode);
    }

    fprintf(file, ")");
  }

  fprintf(file, "\n");
}
