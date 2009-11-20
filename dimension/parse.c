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

#include "parse.h"
#include "tokenize.h"
#include "utility.h"
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

/* Create a new astnode, populating filename, line, and col from a token */
static dmnsn_astnode
dmnsn_new_astnode(dmnsn_astnode_type type, dmnsn_token token)
{
  dmnsn_astnode astnode = {
    .type = type,
    .children = dmnsn_new_array(sizeof(dmnsn_astnode)),
    .ptr = NULL,
    .filename = token.filename, .line = token.line, .col = token.col
  };
  return astnode;
}

/* Delete a single, unused astnode */
static void
dmnsn_delete_astnode(dmnsn_astnode astnode)
{
  dmnsn_delete_astree(astnode.children);
  free(astnode.ptr);
}

/* Expect a particular token next, and complain if we see something else */
static int
dmnsn_parse_expect(dmnsn_token_type type, const dmnsn_array *tokens,
                   unsigned int *ip)
{
  dmnsn_token token;

  if (*ip < dmnsn_array_size(tokens)) {
    dmnsn_array_get(tokens, *ip, &token);
    if (token.type != type) {
      dmnsn_diagnostic(token.filename, token.line, token.col,
                       "Expected '%s', found '%s'",
                       dmnsn_token_string(type),
                       dmnsn_token_string(token.type));
      return 1;
    }
  } else {
    fprintf(stderr, "Expected '%s', found end of file\n",
            dmnsn_token_string(type));
    return 1;
  }

  ++*ip;
  return 0;
}

/* Parse a number - a float or an integer */
static int
dmnsn_parse_number(const dmnsn_array *tokens, unsigned int *ip,
                   dmnsn_array *astree)
{
  unsigned int i = *ip;

  if (i >= dmnsn_array_size(tokens)) {
    fprintf(stderr, "Expected '%s' or '%s', found end of file\n",
            dmnsn_token_string(DMNSN_T_INTEGER),
            dmnsn_token_string(DMNSN_T_FLOAT));
    return 1;
  }

  dmnsn_token token;
  dmnsn_array_get(tokens, i, &token);

  dmnsn_astnode astnode;

  if (token.type == DMNSN_T_INTEGER) {
    /* An exact integer */
    astnode = dmnsn_new_astnode(DMNSN_AST_INTEGER, token);

    long *value = malloc(sizeof(double));
    if (!value)
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate room for integer.");

    *value = strtol(token.value, NULL, 0);
    astnode.ptr = value;
    ++i;
  } else if (token.type == DMNSN_T_FLOAT) {
    /* A floating-point value */
    astnode = dmnsn_new_astnode(DMNSN_AST_FLOAT, token);

    double *value = malloc(sizeof(double));
    if (!value)
      dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate room for float.");

    *value = strtod(token.value, NULL);
    astnode.ptr = value;
    ++i;
  } else {
    dmnsn_diagnostic(token.filename, token.line, token.col,
                     "Expected '%s' or '%s', found '%s'",
                     dmnsn_token_string(DMNSN_T_INTEGER),
                     dmnsn_token_string(DMNSN_T_FLOAT),
                     dmnsn_token_string(token.type));
    goto bailout;
  }

  dmnsn_array_push(astree, &astnode);
  *ip = i;
  return 0;

 bailout:
  dmnsn_delete_astree(astnode.children);
  return 1;
}

/* Map an operator token to a unary operator node type */
static dmnsn_astnode_type
dmnsn_unary_op_map(dmnsn_token_type type)
{
  switch (type) {
  case DMNSN_T_MINUS:
    return DMNSN_AST_NEGATE;

  default:
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Invalid token mapped to operator.");
    return 0; /* Silence compiler warning */
  }
}

/* Map an operator token to a binary operator node type */
static dmnsn_astnode_type
dmnsn_binary_op_map(dmnsn_token_type type)
{
  switch (type) {
  case DMNSN_T_PLUS:
    return DMNSN_AST_ADD;

  case DMNSN_T_MINUS:
    return DMNSN_AST_SUB;

  case DMNSN_T_STAR:
    return DMNSN_AST_MUL;

  case DMNSN_T_SLASH:
    return DMNSN_AST_DIV;

  default:
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Invalid token mapped to operator.");
    return 0; /* Silence compiler warning */
  }
}

/* Return the precedence of a given operator */
static int
dmnsn_op_precedence(dmnsn_astnode_type type)
{
  switch(type) {
  case DMNSN_AST_ADD:
  case DMNSN_AST_SUB:
    return 0;

  case DMNSN_AST_MUL:
  case DMNSN_AST_DIV:
    return 1;

  default:
    dmnsn_error(DMNSN_SEVERITY_HIGH,
                "Precedence asked of invalid operator.");
    return 0; /* Silence compiler warning */
  }
}

/* Erhsuate a unnary arithmetic operation */
static dmnsn_astnode
dmnsn_eval_unary(dmnsn_astnode_type type, dmnsn_astnode rhs)
{
  dmnsn_astnode astnode = rhs; /* Copy filename, etc. from rhs */
  astnode.children = NULL;

  if (rhs.type == DMNSN_AST_INTEGER) {
    long n = *(long *)rhs.ptr;
    long *res = malloc(sizeof(long));

    switch(type) {
    case DMNSN_AST_NEGATE:
      *res = -n;
      break;

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH,
                  "Attempt to evaluate wrong unary operator.");
    }

    astnode.type = DMNSN_AST_INTEGER;
    astnode.ptr  = res;
  } else if (rhs.type == DMNSN_AST_FLOAT) {
    double n = *(double *)rhs.ptr;
    double *res = malloc(sizeof(double));

    switch(type) {
    case DMNSN_AST_NEGATE:
      *res = -n;
      break;

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH,
                  "Attempt to evaluate wrong unary operator.");
    }

    astnode.type = DMNSN_AST_FLOAT;
    astnode.ptr  = res;
  } else {
    dmnsn_error(DMNSN_SEVERITY_HIGH,
                "Invalid right hand side to unary operator.");
  }

  return astnode;
}

/* Evaluate a binary arithmetic operation */
static dmnsn_astnode
dmnsn_eval_binary(dmnsn_astnode_type type, dmnsn_astnode lhs, dmnsn_astnode rhs)
{
  dmnsn_astnode astnode = lhs; /* Copy filename, etc. from lhs */
  astnode.children = NULL;

  if (lhs.type == DMNSN_AST_INTEGER && rhs.type == DMNSN_AST_INTEGER
      && type != DMNSN_AST_DIV) {
    long l, r;
    l = *(long *)lhs.ptr;
    r = *(long *)rhs.ptr;

    long *res = malloc(sizeof(long));

    switch(type) {
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

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH,
                  "Attempt to evaluate wrong binary operator.");
    }

    astnode.type = DMNSN_AST_INTEGER;
    astnode.ptr  = res;
  } else {
    double l = 0.0, r = 0.0;

    if (lhs.type == DMNSN_AST_INTEGER)
      l = *(long *)lhs.ptr;
    else if (lhs.type == DMNSN_AST_FLOAT)
      l = *(double *)lhs.ptr;
    else
      dmnsn_error(DMNSN_SEVERITY_HIGH,
                  "Invalid left hand side to binary operator.");

    if (rhs.type == DMNSN_AST_INTEGER)
      r = *(long *)rhs.ptr;
    else if (rhs.type == DMNSN_AST_FLOAT)
      r = *(double *)rhs.ptr;
    else
      dmnsn_error(DMNSN_SEVERITY_HIGH,
                  "Invalid right hand side to binary operator.");

    double *res = malloc(sizeof(double));

    switch(type) {
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

    default:
      dmnsn_error(DMNSN_SEVERITY_HIGH,
                  "Attempt to evaluate wrong binary operator.");
    }

    astnode.type = DMNSN_AST_FLOAT;
    astnode.ptr  = res;
  }

  return astnode;
}

/* Parse an arithmetic expression */
static int
dmnsn_parse_arithexp(const dmnsn_array *tokens, unsigned int *ip,
                     dmnsn_array *astree)
{
  /*
   * We use a stack of numbers (numstack), and a stack of operators (opstack).
   * Each time we see a number (or a parenthetical sub-expression, which is
   * evaluated recursively), it goes on the numstack.  When we see an operator,
   * it goes on the opstack, but if it has the same or lower precedence than
   * the operator below it on the stack, the two top elements of numstack are
   * folded with the correct operator until either opstack is empty or the
   * current operator has a higher precedence.  Example:
   *
   *   1 - 2 * 3 - 4 * 5 --> ... - 4 * 5 -->  ... - 4 * 5   --> ... - 4 * 5
   *                         _____       -->  ___________   --> ___________
   *                         3 | *            6       | -       -5      |
   *                         2 | -            1       |
   *                         1 |
   *
   *   --> ...
   *   --> ______
   *       5  | *
   *       4  | -
   *       -5 |
   *
   * Then the stack is collapsed from the top down until we have a single value:
   *
   *   --> _________________ --> -25
   *       20 | -
   *       -5 |
   */
  dmnsn_token token;
  unsigned int i = *ip;

  if (i >= dmnsn_array_size(tokens)) {
    fprintf(stderr, "Expected arithmetic expression, found end of file\n");
    return 1;
  }

  dmnsn_array *numstack = dmnsn_new_array(sizeof(dmnsn_astnode)),
              *opstack  = dmnsn_new_array(sizeof(dmnsn_astnode_type));

  while (i < dmnsn_array_size(tokens)) {
    dmnsn_array_get(tokens, i, &token);

    if (token.type == DMNSN_T_INTEGER || token.type == DMNSN_T_FLOAT) {
      /* Item is a number */
      dmnsn_parse_number(tokens, &i, numstack);
    } else if (token.type == DMNSN_T_LPAREN) {
      ++i; /* Skip past '(' */

      /* Parse the contents of the parentheses */
      if (dmnsn_parse_arithexp(tokens, &i, numstack) != 0)
        goto bailout;

      /* Match a closing ')' */
      if (dmnsn_parse_expect(DMNSN_T_RPAREN, tokens, &i) != 0)
        goto bailout;
    } else if (token.type == DMNSN_T_PLUS
               || token.type == DMNSN_T_MINUS
               || token.type == DMNSN_T_STAR
               || token.type == DMNSN_T_SLASH)

    {
      /* Item is an operator */
      if (dmnsn_array_size(opstack) == dmnsn_array_size(numstack)) {
        /* The last item was an operator too, or this is the first item, so it
           must be a unary operator */
        if (token.type == DMNSN_T_MINUS) {
          dmnsn_astnode_type type = dmnsn_unary_op_map(token.type);
          dmnsn_astnode astnode = dmnsn_new_astnode(type, token);

          ++i;
          dmnsn_array_get(tokens, i, &token);
          if (token.type == DMNSN_T_LPAREN) {
            if (dmnsn_parse_arithexp(tokens, &i, astnode.children) != 0)
              goto bailout;
          } else {
            if (dmnsn_parse_number(tokens, &i, astnode.children) != 0)
              goto bailout;
          }

          dmnsn_astnode rhs;
          dmnsn_array_get(astnode.children, 0, &rhs);

          if (rhs.type == DMNSN_AST_INTEGER || rhs.type == DMNSN_AST_FLOAT) {
            dmnsn_astnode neg = dmnsn_eval_unary(DMNSN_AST_NEGATE, rhs);
            dmnsn_array_push(numstack, &neg);
            dmnsn_delete_astnode(astnode);
          } else {
            dmnsn_array_push(numstack, &astnode);
          }
        } else {
          dmnsn_diagnostic(token.filename, token.line, token.col,
                           "Unexpected '%s' when parsing arithmetic expression",
                           dmnsn_token_string(token.type));
          return 1;
        }
      } else if (dmnsn_array_size(opstack) == 0) {
        /* opstack is empty; push the operator */
        dmnsn_astnode_type type = dmnsn_binary_op_map(token.type);
        dmnsn_array_push(opstack, &type);
        ++i;
      } else {
        dmnsn_astnode_type type = dmnsn_binary_op_map(token.type), last_type;
        dmnsn_array_get(opstack, dmnsn_array_size(opstack) - 1, &last_type);

        while (dmnsn_op_precedence(type) <= dmnsn_op_precedence(last_type)) {
          /* Repeatedly collapse numstack */
          dmnsn_astnode lhs, rhs, astnode;

          dmnsn_array_pop(numstack, &rhs);
          dmnsn_array_pop(numstack, &lhs);

          if ((lhs.type == DMNSN_AST_INTEGER || lhs.type == DMNSN_AST_FLOAT)
              && (rhs.type == DMNSN_AST_INTEGER || rhs.type == DMNSN_AST_FLOAT))
          {
            astnode = dmnsn_eval_binary(last_type, lhs, rhs);
            dmnsn_delete_astnode(lhs);
            dmnsn_delete_astnode(rhs);
          } else {
            astnode = dmnsn_new_astnode(last_type, token);
            dmnsn_array_push(astnode.children, &lhs);
            dmnsn_array_push(astnode.children, &rhs);
          }

          dmnsn_array_push(numstack, &astnode);
          dmnsn_array_resize(opstack, dmnsn_array_size(opstack) - 1);

          if (dmnsn_array_size(opstack) > 0)
            dmnsn_array_get(opstack, dmnsn_array_size(opstack) - 1, &last_type);
          else
            break;
        }

        dmnsn_array_push(opstack, &type);
        ++i;
      }
    } else {
      /* Unrecognized token; arithmetic expression must be over */
      break;
    }
  }

  /* Prevent us from matching nothing */
  if (dmnsn_array_size(numstack) == 0) {
    dmnsn_diagnostic(token.filename, token.line, token.col,
                     "Expected arithmetic expression, found '%s'",
                     dmnsn_token_string(token.type));
    goto bailout;
  }

  while (dmnsn_array_size(numstack) > 1) {
    /* Collapse numstack */
    dmnsn_astnode_type type;
    dmnsn_array_pop(opstack, &type);

    dmnsn_astnode astnode, lhs, rhs;

    dmnsn_array_pop(numstack, &rhs);
    dmnsn_array_pop(numstack, &lhs);

    if ((lhs.type == DMNSN_AST_INTEGER || lhs.type == DMNSN_AST_FLOAT)
        && (rhs.type == DMNSN_AST_INTEGER || rhs.type == DMNSN_AST_FLOAT)) {
      astnode = dmnsn_eval_binary(type, lhs, rhs);
      dmnsn_delete_astnode(lhs);
      dmnsn_delete_astnode(rhs);
    } else {
      astnode = lhs; /* Steal filname, etc. from lhs */
      astnode.type     = type;
      astnode.children = dmnsn_new_array(sizeof(dmnsn_astnode));
      astnode.ptr      = NULL;

      dmnsn_array_push(astnode.children, &lhs);
      dmnsn_array_push(astnode.children, &rhs);
    }

    dmnsn_array_push(numstack, &astnode);
  }

  dmnsn_array_push(astree, dmnsn_array_at(numstack, 0));
  dmnsn_delete_array(opstack);
  dmnsn_delete_array(numstack);
  *ip = i;
  return 0;

 bailout:
  dmnsn_delete_array(opstack);
  dmnsn_delete_astree(numstack);
  return 1;
}

/* Parse a vector */
static int
dmnsn_parse_vector(const dmnsn_array *tokens, unsigned int *ip,
                   dmnsn_array *astree)
{
  dmnsn_token token;
  unsigned int i = *ip;

  dmnsn_array_get(tokens, i, &token);

  /* Vectors start with '<' */
  if (dmnsn_parse_expect(DMNSN_T_LESS, tokens, &i) != 0)
    return 1;

  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_VECTOR, token);

  do {
    /* Parse comma-separated aritexp's, closed by '>' */
    if (dmnsn_parse_arithexp(tokens, &i, astnode.children) != 0)
      goto bailout;

    dmnsn_array_get(tokens, i, &token);
    if (token.type != DMNSN_T_COMMA && token.type != DMNSN_T_GREATER) {
      dmnsn_diagnostic(token.filename, token.line, token.col,
                       "Expected '%s' or '%s'; found '%s'",
                       dmnsn_token_string(DMNSN_T_COMMA),
                       dmnsn_token_string(DMNSN_T_GREATER),
                       dmnsn_token_string(token.type));
      goto bailout;
    }

    ++i;
  } while (token.type != DMNSN_T_GREATER);

  dmnsn_array_push(astree, &astnode);
  *ip = i;
  return 0;

 bailout:
  dmnsn_delete_astree(astnode.children);
  return 1;
}

/* Parse a box definition */
static int
dmnsn_parse_box(const dmnsn_array *tokens, unsigned int *ip,
                dmnsn_array *astree)
{
  dmnsn_token token;
  unsigned int i = *ip;

  dmnsn_array_get(tokens, i, &token);

  /* Opens with "box {" */
  if (dmnsn_parse_expect(DMNSN_T_BOX, tokens, &i) != 0)
    return 1;
  if (dmnsn_parse_expect(DMNSN_T_LBRACE, tokens, &i) != 0)
    return 1;

  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_BOX, token);

  /* First corner */
  if (dmnsn_parse_vector(tokens, &i, astnode.children) != 0)
    goto bailout;

  if (dmnsn_parse_expect(DMNSN_T_COMMA, tokens, &i) != 0)
    goto bailout;

  /* Second corner */
  if (dmnsn_parse_vector(tokens, &i, astnode.children) != 0)
    goto bailout;

  if (dmnsn_parse_expect(DMNSN_T_RBRACE, tokens, &i) != 0)
    goto bailout;

  dmnsn_array_push(astree, &astnode);
  *ip = i;
  return 0;

 bailout:
  dmnsn_delete_astree(astnode.children);
  return 1;
}

/* Parse a sphere definition */
static int
dmnsn_parse_sphere(const dmnsn_array *tokens, unsigned int *ip,
                   dmnsn_array *astree)
{
  dmnsn_token token;
  unsigned int i = *ip;

  dmnsn_array_get(tokens, i, &token);

  /* Opens with "sphere {" */
  if (dmnsn_parse_expect(DMNSN_T_SPHERE, tokens, &i) != 0)
    return 1;
  if (dmnsn_parse_expect(DMNSN_T_LBRACE, tokens, &i) != 0)
    return 1;

  dmnsn_astnode astnode = dmnsn_new_astnode(DMNSN_AST_SPHERE, token);

  /* Center */
  if (dmnsn_parse_vector(tokens, &i, astnode.children) != 0)
    goto bailout;

  if (dmnsn_parse_expect(DMNSN_T_COMMA, tokens, &i) != 0)
    goto bailout;

  /* Radius */
  if (dmnsn_parse_number(tokens, &i, astnode.children) != 0)
    goto bailout;

  if (dmnsn_parse_expect(DMNSN_T_RBRACE, tokens, &i) != 0)
    goto bailout;

  dmnsn_array_push(astree, &astnode);
  *ip = i;
  return 0;

 bailout:
  dmnsn_delete_astree(astnode.children);
  return 1;
}

dmnsn_array *
dmnsn_parse(const dmnsn_array *tokens)
{
  unsigned int i;
  dmnsn_array *astree = dmnsn_new_array(sizeof(dmnsn_astnode));
  dmnsn_token token;

  /* Save the current locale */
  char *lc_ctype   = strdup(setlocale(LC_CTYPE, NULL));
  char *lc_numeric = strdup(setlocale(LC_NUMERIC, NULL));

  /* Set the locale to `C' to make strtol(), etc. consistent */
  setlocale(LC_CTYPE, "C");
  setlocale(LC_NUMERIC, "C");

  i = 0;
  while (i < dmnsn_array_size(tokens)) {
    dmnsn_array_get(tokens, i, &token);

    switch (token.type) {
    case DMNSN_T_BOX:
      if (dmnsn_parse_box(tokens, &i, astree) != 0)
        goto bailout;
      break;

    case DMNSN_T_SPHERE:
      if (dmnsn_parse_sphere(tokens, &i, astree) != 0)
        goto bailout;
      break;

    default:
      dmnsn_diagnostic(token.filename, token.line, token.col,
                       "Unexpected token '%s'",
                       dmnsn_token_string(token.type));
      goto bailout;
    }
  }

  /* Restore the original locale */
  setlocale(LC_CTYPE, lc_ctype);
  setlocale(LC_NUMERIC, lc_numeric);
  free(lc_ctype);
  free(lc_numeric);

  return astree;

 bailout:
  /* Restore the original locale */
  setlocale(LC_CTYPE, lc_ctype);
  setlocale(LC_NUMERIC, lc_numeric);
  free(lc_ctype);
  free(lc_numeric);

  dmnsn_delete_astree(astree);
  return NULL;
}

void
dmnsn_delete_astree(dmnsn_array *astree)
{
  unsigned int i;
  dmnsn_astnode node;

  if (astree) {
    for (i = 0; i < dmnsn_array_size(astree); ++i) {
      dmnsn_array_get(astree, i, &node);
      dmnsn_delete_astree(node.children);
      free(node.ptr);
    }
    dmnsn_delete_array(astree);
  }
}

static void
dmnsn_print_astnode(FILE *file, dmnsn_astnode astnode)
{
  long ivalue;
  double dvalue;

  switch (astnode.type) {
  case DMNSN_AST_INTEGER:
    ivalue = *(long *)astnode.ptr;
    fprintf(file, "(%s %ld)", dmnsn_astnode_string(astnode.type), ivalue);
    break;

  case DMNSN_AST_FLOAT:
    dvalue = *(double *)astnode.ptr;
    fprintf(file, "(%s %g)", dmnsn_astnode_string(astnode.type), dvalue);
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
dmnsn_print_astree_sexpr(FILE *file, const dmnsn_array *astree)
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

const char *
dmnsn_astnode_string(dmnsn_astnode_type astnode_type)
{
  switch (astnode_type) {
  /* Macro to shorten this switch */
#define dmnsn_astnode_map(type, str)                                           \
  case type:                                                                   \
    return str;

  dmnsn_astnode_map(DMNSN_AST_FLOAT, "float");
  dmnsn_astnode_map(DMNSN_AST_INTEGER, "integer");
  dmnsn_astnode_map(DMNSN_AST_NEGATE, "-");
  dmnsn_astnode_map(DMNSN_AST_ADD, "+");
  dmnsn_astnode_map(DMNSN_AST_SUB, "-");
  dmnsn_astnode_map(DMNSN_AST_MUL, "*");
  dmnsn_astnode_map(DMNSN_AST_DIV, "/");
  dmnsn_astnode_map(DMNSN_AST_BOX, "box");
  dmnsn_astnode_map(DMNSN_AST_VECTOR, "vector");
  dmnsn_astnode_map(DMNSN_AST_SPHERE, "sphere");

  default:
    fprintf(stderr, "Warning: unrecognised astnode type %d.\n",
            (int)astnode_type);
    return "unrecognized-astnode";
  }
}
