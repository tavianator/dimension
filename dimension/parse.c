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
                       dmnsn_token_name(type),
                       dmnsn_token_name(token.type));
      return 1;
    }
  } else {
    fprintf(stderr, "Expected '%s', found end of file\n",
            dmnsn_token_name(type));
    return 1;
  }

  ++*ip;
  return 0;
}

static int
dmnsn_parse_float(const dmnsn_array *tokens, unsigned int *ip,
                  dmnsn_array *astree)
{
  unsigned int i = *ip;

  if (i >= dmnsn_array_size(tokens)) {
    fprintf(stderr, "Expected '%s' or '%s', found end of file\n",
            dmnsn_token_name(DMNSN_T_INTEGER),
            dmnsn_token_name(DMNSN_T_FLOAT));
    return 1;
  }

  int negative = 0;
  dmnsn_token token;
  dmnsn_array_get(tokens, i, &token);

  if (token.type == DMNSN_T_MINUS) {
    negative = 1;

    ++i;
    if (i >= dmnsn_array_size(tokens)) {
      fprintf(stderr, "Expected '%s' or '%s', found end of file\n",
              dmnsn_token_name(DMNSN_T_INTEGER),
              dmnsn_token_name(DMNSN_T_FLOAT));
      return 1;
    }
    dmnsn_array_get(tokens, i, &token);
  }

  double *value = malloc(sizeof(double));
  if (!value) 
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate room for float.");

  if (token.type == DMNSN_T_INTEGER || token.type == DMNSN_T_FLOAT) {
    *value = strtod(token.value, NULL);
    ++i;
  } else {
    fprintf(stderr, "Expected '%s' or '%s', found '%s'\n",
            dmnsn_token_name(DMNSN_T_INTEGER),
            dmnsn_token_name(DMNSN_T_FLOAT),
            dmnsn_token_name(token.type));
    return 1;
  }

  if (negative)
    *value *= -1.0;

  dmnsn_astnode astnode;
  astnode.type     = DMNSN_AST_FLOAT;
  astnode.children = NULL;
  astnode.ptr      = value;

  dmnsn_array_push(astree, &astnode);
  *ip = i;
  return 0;
}

static int
dmnsn_parse_vector(const dmnsn_array *tokens, unsigned int *ip,
                   dmnsn_array *astree)
{
  unsigned int i = *ip;

  if (dmnsn_parse_expect(DMNSN_T_LESS, tokens, &i) != 0)
    return 1;

  dmnsn_astnode astnode;
  astnode.type     = DMNSN_AST_VECTOR;
  astnode.children = dmnsn_new_array(sizeof(dmnsn_astnode));
  astnode.ptr      = NULL;

  /* First element */
  if (dmnsn_parse_float(tokens, &i, astnode.children) != 0)
    goto bailout;

  if (dmnsn_parse_expect(DMNSN_T_COMMA, tokens, &i) != 0)
    goto bailout;

  /* Second element */
  if (dmnsn_parse_float(tokens, &i, astnode.children) != 0)
    goto bailout;

  if (dmnsn_parse_expect(DMNSN_T_COMMA, tokens, &i) != 0)
    goto bailout;

  /* Third element */
  if (dmnsn_parse_float(tokens, &i, astnode.children) != 0)
    goto bailout;

  if (dmnsn_parse_expect(DMNSN_T_GREATER, tokens, &i) != 0)
    goto bailout;

  dmnsn_array_push(astree, &astnode);
  *ip = i;
  return 0;

 bailout:
  dmnsn_delete_astree(astnode.children);
  return 1;
}

static int
dmnsn_parse_box(const dmnsn_array *tokens, unsigned int *ip,
                dmnsn_array *astree)
{
  unsigned int i = *ip;

  if (dmnsn_parse_expect(DMNSN_T_BOX, tokens, &i) != 0)
    return 1;
  if (dmnsn_parse_expect(DMNSN_T_LBRACE, tokens, &i) != 0)
    return 1;

  dmnsn_astnode astnode;
  astnode.type     = DMNSN_AST_BOX;
  astnode.children = dmnsn_new_array(sizeof(dmnsn_astnode));
  astnode.ptr      = NULL;

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

dmnsn_array *
dmnsn_parse(const dmnsn_array *tokens)
{
  unsigned int i;
  dmnsn_array *astree = dmnsn_new_array(sizeof(dmnsn_astnode));
  dmnsn_token token;

  /* Save the current locale */
  char *lc_ctype   = strdup(setlocale(LC_CTYPE, NULL));
  char *lc_numeric = strdup(setlocale(LC_NUMERIC, NULL));

  /* Set the locale to `C' to make strtoul(), etc. consistent */
  setlocale(LC_CTYPE, "C");
  setlocale(LC_NUMERIC, "C");

  for (i = 0; i < dmnsn_array_size(tokens); ++i) {
    dmnsn_array_get(tokens, i, &token);

    if (token.type == DMNSN_T_BOX) {
      if (dmnsn_parse_box(tokens, &i, astree) != 0) {
        dmnsn_diagnostic(token.filename, token.line, token.col,
                         "Invalid box",
                         dmnsn_token_name(DMNSN_T_BOX),
                         dmnsn_token_name(token.type));
        goto bailout;
      }
    } else {
      dmnsn_diagnostic(token.filename, token.line, token.col,
                       "Expected '%s', found '%s'",
                       dmnsn_token_name(DMNSN_T_BOX),
                       dmnsn_token_name(token.type));
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

void
dmnsn_print_astree_sexpr(FILE *file, const dmnsn_array *tokens)
{
}
