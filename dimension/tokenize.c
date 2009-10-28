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

static int
dmnsn_tokenize_comment(char *map, size_t size,
                       char **next, unsigned int *line, unsigned int *col)
{
  if (**next != '/')
    return 1;

  if (*(*next + 1) == '/') {
    /* A '//' comment block */
    do {
      ++*next;
    } while (*next - map < size && **next != '\n');

    ++*next;
    ++*line;
    *col = 0;
  } else if (*(*next + 1) == '*') {
    /* A '/*' comment block */
    do {
      ++col;
      if (**next == '\n') {
        ++line;
        col = 0;
      }

      ++*next;
    } while (*next - map < size - 1
             && !(**next == '*' && *(*next + 1) == '/'));
    *next += 2;
  } else {
    return 1;
  }

  return 0;
}

static int
dmnsn_tokenize_number(char *map, size_t size, dmnsn_token *token,
                      char **next, unsigned int *line, unsigned int *col)
{
  char *endf, *endi;

  strtoul(*next, &endi, 0);
  strtod(*next, &endf);
  if (endf > endi
      /* These *next conditions catch invalid octal integers being parsed as
         floats, eg 08 */
      && (*endi == '.' || *endi == 'e' || *endi == 'E' || *endi == 'p'
          || *endi == 'P'))
  {
    token->type = DMNSN_FLOAT;
    token->value = malloc(endf - *next + 1);
    strncpy(token->value, *next, endf - *next);
    token->value[endf - *next] = '\0';

    *col += endf - *next;
    *next = endf;
  } else if (endi > *next) {
    token->type = DMNSN_INT;
    token->value = malloc(endi - *next + 1);
    strncpy(token->value, *next, endi - *next);
    token->value[endi - *next] = '\0';

    *col += endi - *next;
    *next = endi;
  } else {
    return 1;
  }

  return 0;
}

static int
dmnsn_tokenize_identifier(char *map, size_t size, dmnsn_token *token,
                          char **next, unsigned int *line, unsigned int *col)
{
  unsigned int i = 0, alloc = 32;

  if (!isalpha(**next) && **next != '_') {
    return 1;
  }

  token->type  = DMNSN_IDENTIFIER;
  token->value = malloc(alloc);

  while (*next - map < size && (isalnum(**next) || **next == '_')) {
    if (i + 1 >= alloc) {
      alloc *= 2;
      token->value = realloc(token->value, alloc);
    }

    token->value[i] = **next;

    ++i;
    ++*col;
    ++*next;
  }

  token->value[i] = '\0';
  return 0;
}

dmnsn_array *
dmnsn_tokenize(FILE *file)
{
  if (fseeko(file, 0, SEEK_END) != 0) {
    fprintf(stderr, "Couldn't seek on input stream.\n");
    return NULL;
  }

  off_t size = ftello(file);

  int fd = fileno(file);
  if (fd == -1) {
    fprintf(stderr, "Couldn't get file descriptor to input stream.\n");
    return NULL;
  }

  char *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0), *next = map;

  if (map == MAP_FAILED) {
    fprintf(stderr, "Couldn't mmap() input stream.\n");
    return NULL;
  }

  dmnsn_token token;
  dmnsn_array *tokens = dmnsn_new_array(sizeof(dmnsn_token));

  unsigned int line = 1, col = 0;

  while (next - map < size) {
    /* Saves some code repetition */
    token.value = NULL;
    token.line  = line;
    token.col   = col;

    switch (*next) {
    case ' ':
    case '\r':
    case '\t':
    case '\f':
    case '\v':
      /* Skip whitespace */
      ++next;
      ++col;
      continue;

    case '\n':
      ++next;
      ++line;
      col = 0;
      continue;

    /* Macro to make basic symbol tokens easier */
    #define dmnsn_simple_token(c, tp)                                          \
    case c:                                                                    \
      token.type = tp;                                                         \
      break

    /* Some simple punctuation marks */
    dmnsn_simple_token('{', DMNSN_LBRACE);
    dmnsn_simple_token('}', DMNSN_RBRACE);
    dmnsn_simple_token('(', DMNSN_LPAREN);
    dmnsn_simple_token(')', DMNSN_RPAREN);
    dmnsn_simple_token('[', DMNSN_LBRACKET);
    dmnsn_simple_token(']', DMNSN_RBRACKET);
    dmnsn_simple_token('<', DMNSN_LT);
    dmnsn_simple_token('>', DMNSN_GT);
    dmnsn_simple_token('+', DMNSN_PLUS);
    dmnsn_simple_token('-', DMNSN_MINUS);
    dmnsn_simple_token('*', DMNSN_STAR);
    dmnsn_simple_token(',', DMNSN_COMMA);

    /* Possible comment */
    case '/':
      if (dmnsn_tokenize_comment(map, size, &next, &line, &col) == 0) {
        continue;
      } else {
        /* Just the normal punctuation mark */
        token.type = DMNSN_SLASH;
      }
      break;

    /* Numeric values */
    case '.': /* Number begins with a decimal point, as in `.2' */
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if (dmnsn_tokenize_number(map, size, &token, &next, &line, &col) != 0) {
        fprintf(stderr, "Invalid numeric value on line %u, column %u.\n",
                line, col);
        goto bailout;
      }
      break;

    default:
      if (dmnsn_tokenize_identifier(map, size, &token, &next, &line, &col) != 0)
      {
        /* Unrecognised character */
        fprintf(stderr,
                "Unrecognized character 0x%X in input at line %u, column %u.\n",
                (unsigned int)*next, line, col);
        goto bailout;
      }
    }

    dmnsn_array_push(tokens, &token);
    ++next;
    ++col;
  }

  munmap(map, size);
  return tokens;

 bailout:
  dmnsn_delete_tokens(tokens);
  munmap(map, size);
  return NULL;
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
  /* Macro to shorten this huge switch */
  #define dmnsn_token_map(type, str)                                           \
  case type:                                                                   \
    return str;

  /* Punctuation */
  dmnsn_token_map(DMNSN_LBRACE,   "{");
  dmnsn_token_map(DMNSN_RBRACE,   "}")
  dmnsn_token_map(DMNSN_LPAREN,   "\\(");
  dmnsn_token_map(DMNSN_RPAREN,   "\\)");
  dmnsn_token_map(DMNSN_LBRACKET, "[");
  dmnsn_token_map(DMNSN_RBRACKET, "]");
  dmnsn_token_map(DMNSN_LT,       "<");
  dmnsn_token_map(DMNSN_GT,       ">");
  dmnsn_token_map(DMNSN_PLUS,     "+");
  dmnsn_token_map(DMNSN_MINUS,    "-");
  dmnsn_token_map(DMNSN_STAR,     "*");
  dmnsn_token_map(DMNSN_SLASH,    "/");
  dmnsn_token_map(DMNSN_COMMA,    ",");

  /* Numeric values */
  dmnsn_token_map(DMNSN_INT,   "int");
  dmnsn_token_map(DMNSN_FLOAT, "float");

  /* Identifiers */
  dmnsn_token_map(DMNSN_IDENTIFIER, "identifier");

  default:
    printf("Warning: unrecognised token %d.\n", (int)token_type);
    return "unrecognized-token";
  }
}
