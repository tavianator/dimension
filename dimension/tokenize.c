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

#include "tokenize.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>
#include <unistd.h>

dmnsn_array *
dmnsn_tokenize(FILE *file)
{
  int fd = fileno(file);
  off_t size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  char *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0), *next = map;

  dmnsn_token token;
  dmnsn_array *tokens = dmnsn_new_array(sizeof(dmnsn_token));

  while (next - map < size) {
    switch (*next) {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
    case '\f':
    case '\v':
      /* Skip whitespace */
      break;

    case '{':
      token.type = DMNSN_LBRACE;
      token.value = NULL;
      dmnsn_array_push(tokens, &token);
      break;

    case '}':
      token.type = DMNSN_LBRACE;
      token.value = NULL;
      dmnsn_array_push(tokens, &token);
      break;

    default:
      /* Unrecognised character */
      fprintf(stderr, "Unrecognized character 0x%X in input.\n", (unsigned int)*next);
      dmnsn_delete_tokens(tokens);
      munmap(map, size);
      return NULL;
    }

    ++next;
  }

  munmap(map, size);
  return tokens;
}

void
dmnsn_delete_tokens(dmnsn_array *tokens)
{
  dmnsn_token *token;
  unsigned int i;
  for (i = 0; i < dmnsn_array_size(tokens); ++i) {
    token = dmnsn_array_at(tokens, i);
    free(token->value);
  }
  dmnsn_delete_array(tokens);
}

static const char *dmnsn_token_name(dmnsn_token_type token_type);

static void
dmnsn_print_token(FILE *file, dmnsn_token *token)
{
  if (token->value) {
    fprintf(file, "(%s \"%s\")", dmnsn_token_name(token->type), token->value);
  } else {
    fprintf(file, "%s", dmnsn_token_name(token->type));
  }
}

void
dmnsn_print_token_sexpr(FILE *file, dmnsn_array *tokens)
{
  unsigned int i;
  if (dmnsn_array_size(tokens) == 0) {
    fprintf(file, "()");
  } else {
    fprintf(file, "(");
    dmnsn_print_token(file, dmnsn_array_at(tokens, 0));

    for (i = 1; i < dmnsn_array_size(tokens); ++i) {
      fprintf(file, " ");
      dmnsn_print_token(file, dmnsn_array_at(tokens, i));
    }

    fprintf(file, ")");
  }

  fprintf(file, "\n");
}

static const char *
dmnsn_token_name(dmnsn_token_type token_type)
{
  switch (token_type) {
  case DMNSN_LBRACE:
    return "DMNSN_LBRACE";

  case DMNSN_RBRACE:
    return "DMNSN_RBRACE";

  default:
    return "UNRECOGNIZED-TOKEN";
  }
}
