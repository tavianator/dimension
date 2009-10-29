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
#include <stdlib.h>   /* For strtoul(), etc. */
#include <string.h>
#include <stdarg.h>
#include <ctype.h>    /* For isalpha(), etc. */
#include <sys/mman.h> /* For mmap()          */
#include <libgen.h>   /* For dirname()       */
#include <locale.h>

static void
dmnsn_diagnostic(const char *filename, unsigned int line, unsigned int col,
                 const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  fprintf(stderr, "%s:%u:%u: ", filename, line, col);
  vfprintf(stderr, format, ap);
  fprintf(stderr, "\n");

  va_end(ap);
}

static int
dmnsn_tokenize_comment(const char *filename,
                       unsigned int *line, unsigned int *col,
                       char *map, size_t size, char **next)
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
    /* A multi-line comment block (like this one) */
    do {
      ++*col;
      if (**next == '\n') {
        ++*line;
        *col = 0;
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
dmnsn_tokenize_number(const char *filename,
                      unsigned int *line, unsigned int *col,
                      char *map, size_t size, char **next, dmnsn_token *token)
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
    token->type = DMNSN_T_FLOAT;
    token->value = malloc(endf - *next + 1);
    strncpy(token->value, *next, endf - *next);
    token->value[endf - *next] = '\0';

    *col += endf - *next;
    *next = endf;
  } else if (endi > *next) {
    token->type = DMNSN_T_INT;
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

/* Tokenize a keyword or an identifier */
static int
dmnsn_tokenize_label(const char *filename,
                     unsigned int *line, unsigned int *col,
                     char *map, size_t size, char **next, dmnsn_token *token)
{
  unsigned int i = 0, alloc = 32;

  if (!isalpha(**next) && **next != '_') {
    return 1;
  }

  token->type  = DMNSN_T_IDENTIFIER;
  token->value = malloc(alloc);

  do {
    if (i + 1 >= alloc) {
      alloc *= 2;
      token->value = realloc(token->value, alloc);
    }

    token->value[i] = **next;

    ++i;
    ++*col;
    ++*next;
  } while (*next - map < size && (isalnum(**next) || **next == '_'));

  token->value[i] = '\0';

  /* Now check if we really found a keyword */

#define dmnsn_keyword(str, tp)                                                 \
  do {                                                                         \
    if (strcmp(token->value, str) == 0) {                                      \
      free(token->value);                                                      \
      token->value = NULL;                                                     \
      token->type  = tp;                                                       \
      return 0;                                                                \
    }                                                                          \
  } while (0)

  dmnsn_keyword("camera", DMNSN_T_CAMERA);
  dmnsn_keyword("color",  DMNSN_T_COLOR);
  dmnsn_keyword("colour", DMNSN_T_COLOR);
  dmnsn_keyword("sphere", DMNSN_T_SPHERE);
  dmnsn_keyword("box",    DMNSN_T_BOX);

  return 0;
}

/* Tokenize a language directive (#include, #declare, etc.) */
static int
dmnsn_tokenize_directive(const char *filename,
                         unsigned int *line, unsigned int *col,
                         char *map, size_t size, char **next,
                         dmnsn_token *token)
{
  unsigned int i = 0, alloc = 32;

  if (**next != '#') {
    return 1;
  }

  char *directive = malloc(alloc);

  do {
    if (i + 1 >= alloc) {
      alloc *= 2;
      directive = realloc(directive, alloc);
    }

    directive[i] = **next;

    ++i;
    ++*col;
    ++*next;
  } while (*next - map < size && (isalnum(**next) || **next == '_'));

  directive[i] = '\0';

  /* Now check if we really found a directive */

#define dmnsn_directive(str, tp)                                               \
  do {                                                                         \
    if (strcmp(directive, str) == 0) {                                         \
      free(directive);                                                         \
      token->type = tp;                                                        \
      return 0;                                                                \
    }                                                                          \
  } while (0)

  dmnsn_directive("#break",      DMNSN_T_BREAK);
  dmnsn_directive("#case",       DMNSN_T_CASE);
  dmnsn_directive("#debug",      DMNSN_T_DEBUG);
  dmnsn_directive("#declare",    DMNSN_T_DECLARE);
  dmnsn_directive("#default",    DMNSN_T_DEFAULT);
  dmnsn_directive("#else",       DMNSN_T_ELSE);
  dmnsn_directive("#end",        DMNSN_T_END);
  dmnsn_directive("#error",      DMNSN_T_ERROR);
  dmnsn_directive("#fclose",     DMNSN_T_FCLOSE);
  dmnsn_directive("#fopen",      DMNSN_T_FOPEN);
  dmnsn_directive("#if",         DMNSN_T_IF);
  dmnsn_directive("#ifdef",      DMNSN_T_IFDEF);
  dmnsn_directive("#ifndef",     DMNSN_T_IFNDEF);
  dmnsn_directive("#include",    DMNSN_T_INCLUDE);
  dmnsn_directive("#local",      DMNSN_T_LOCAL);
  dmnsn_directive("#macro",      DMNSN_T_MACRO);
  dmnsn_directive("#range",      DMNSN_T_RANGE);
  dmnsn_directive("#read",       DMNSN_T_READ);
  dmnsn_directive("#render",     DMNSN_T_RENDER);
  dmnsn_directive("#statistics", DMNSN_T_STATISTICS);
  dmnsn_directive("#switch",     DMNSN_T_SWITCH);
  dmnsn_directive("#undef",      DMNSN_T_UNDEF);
  dmnsn_directive("#version",    DMNSN_T_VERSION);
  dmnsn_directive("#warning",    DMNSN_T_WARNING);
  dmnsn_directive("#while",      DMNSN_T_WHILE);
  dmnsn_directive("#write",      DMNSN_T_WRITE);

  free(directive);
  return 1;
}

/* Tokenize a string */
static int
dmnsn_tokenize_string(const char *filename,
                      unsigned int *line, unsigned int *col,
                      char *map, size_t size, char **next, dmnsn_token *token)
{
  unsigned int i = 0, alloc = 32;
  char unicode[5] = { 0 }, *end;
  unsigned long wchar;

  if (**next != '"') {
    return 1;
  }
  
  token->type  = DMNSN_T_STRING;
  token->value = malloc(alloc);

  ++*next;
  while (*next - map < size && **next != '"') {
    if (i + 1 >= alloc) {
      alloc *= 2;
      token->value = realloc(token->value, alloc);
    }

    if (**next == '\\') {
      ++*col;
      ++*next;

      switch (**next) {
      case 'a':
        token->value[i] = '\a';
        break;

      case 'b':
        token->value[i] = '\b';
        break;

      case 'f':
        token->value[i] = '\f';
        break;

      case 'n':
        token->value[i] = '\n';
        break;

      case 'r':
        token->value[i] = '\r';
        break;

      case 't':
        token->value[i] = '\t';
        break;

      case 'u':
        /* Escaped unicode character */
        strncpy(unicode, *next + 1, 4);
        wchar = strtoul(unicode, &end, 16);
        if (*next - map >= size - 4) {
          dmnsn_diagnostic(filename, *line, *col,
                           "EOF before end of escape sequence");
          free(token->value);
          return 1;
        }
        if (end != &unicode[4]) {
          dmnsn_diagnostic(filename, *line, *col,
                           "WARNING: Invalid unicode character \"\\u%s\"",
                           unicode);
        } else {
          token->value[i] = wchar/256;
          ++i;
          if (i + 1 >= alloc) {
            alloc *= 2;
            token->value = realloc(token->value, alloc);
          }
          token->value[i] = wchar%256;

          *col  += 4;
          *next += 4;
        }
        break;

      case 'v':
        token->value[i] = '\v';
        break;

      case '\\':
        token->value[i] = '\\';
        break;

      case '\'':
        token->value[i] = '\'';
        break;

      case '"':
        token->value[i] = '"';
        break;

      default:
        dmnsn_diagnostic(filename, *line, *col,
                         "WARNING: unrecognised escape sequence '\\%c'",
                         (int)**next);
        token->value[i] = **next;
        break;
      }
    } else {
      token->value[i] = **next;
    }

    ++i;
    ++*col;
    ++*next;
  } 

  if (**next != '"') {
    dmnsn_diagnostic(filename, *line, *col, "Non-terminated string");
    free(token->value);
    return 1;
  }

  ++*next;
  token->value[i] = '\0';
  return 0;
}

dmnsn_array *
dmnsn_tokenize(const char *filename, FILE *file)
{
  /* Save the current locale */
  char *lc_ctype   = strdup(setlocale(LC_CTYPE, NULL));
  char *lc_numeric = strdup(setlocale(LC_NUMERIC, NULL));

  /* Set the locale to `C' to make isalpha(), strtoul(), etc. consistent */
  setlocale(LC_CTYPE, "C");
  setlocale(LC_NUMERIC, "C");

  if (fseeko(file, 0, SEEK_END) != 0) {
    fprintf(stderr, "Couldn't seek on input stream\n");
    return NULL;
  }

  off_t size = ftello(file);

  int fd = fileno(file);
  if (fd == -1) {
    fprintf(stderr, "Couldn't get file descriptor to input stream\n");
    return NULL;
  }

  char *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0), *next = map;

  if (map == MAP_FAILED) {
    fprintf(stderr, "Couldn't mmap() input stream\n");
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
#define dmnsn_simple_token(c, tp)                                              \
    case c:                                                                    \
      token.type = tp;                                                         \
      ++col;                                                                   \
      ++next;                                                                  \
      break

    /* Some simple punctuation marks */
    dmnsn_simple_token('{', DMNSN_T_LBRACE);
    dmnsn_simple_token('}', DMNSN_T_RBRACE);
    dmnsn_simple_token('(', DMNSN_T_LPAREN);
    dmnsn_simple_token(')', DMNSN_T_RPAREN);
    dmnsn_simple_token('[', DMNSN_T_LBRACKET);
    dmnsn_simple_token(']', DMNSN_T_RBRACKET);
    dmnsn_simple_token('<', DMNSN_T_LT);
    dmnsn_simple_token('>', DMNSN_T_GT);
    dmnsn_simple_token('+', DMNSN_T_PLUS);
    dmnsn_simple_token('-', DMNSN_T_MINUS);
    dmnsn_simple_token('*', DMNSN_T_STAR);
    dmnsn_simple_token(',', DMNSN_T_COMMA);

    /* Possible comment */
    case '/':
      if (dmnsn_tokenize_comment(filename, &line, &col,
                                 map, size, &next) == 0) {
        continue;
      } else {
        /* Just the normal punctuation mark */
        token.type = DMNSN_T_SLASH;
        ++col;
        ++next;
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
      if (dmnsn_tokenize_number(filename, &line, &col,
                                map, size, &next, &token) != 0) {
        dmnsn_diagnostic(filename, line, col, "Invalid numeric value");
        goto bailout;
      }
      break;

    case '#':
      /* Language directive */
      if (dmnsn_tokenize_directive(filename, &line, &col,
                                   map, size, &next, &token) == 0) {
        if (token.type == DMNSN_T_INCLUDE) {
          /* Skip whitespace */
          while (next - map < size && isspace(*next) && *next != '\n') {
            ++next;
          }

          if (dmnsn_tokenize_string(filename, &line, &col,
                                    map, size, &next, &token) != 0) {
            dmnsn_diagnostic(filename, line, col,
                             "Expected string after #include");
            goto bailout;
          }

          /* Search in same directory as current file */
          char *filename_copy = strdup(filename);
          char *localdir = dirname(filename_copy);
          char *local_include = malloc(strlen(localdir)
                                       + strlen(token.value)
                                       + 2);
          strcpy(local_include, localdir);
          strcat(local_include, "/");
          strcat(local_include, token.value);
          free(filename_copy);
          free(token.value);

          /* Try to open the included file */
          FILE *include = fopen(local_include, "r");
          if (!include) {
            dmnsn_diagnostic(filename, line, col,
                             "Couldn't open included file \"%s\"",
                             local_include);
            free(local_include);
            goto bailout;
          }

          /* Parse it recursively */
          dmnsn_array *included_tokens = dmnsn_tokenize(local_include, include);
          if (!included_tokens) {
            dmnsn_diagnostic(filename, line, col,
                             "Error tokenizing included file \"%s\"",
                             local_include);
            free(local_include);
            goto bailout;
          }

          fclose(include);
          free(local_include);

          /* Append the tokens from the included file */
          unsigned int i;
          for (i = 0; i < dmnsn_array_size(included_tokens); ++i) {
            dmnsn_array_push(tokens, dmnsn_array_at(included_tokens, i));
          }

          dmnsn_delete_array(included_tokens);
          continue;
        }
      } else {
        dmnsn_diagnostic(filename, line, col, "Invalid language directive");
        goto bailout;
      }
      break;

    case '"':
      if (dmnsn_tokenize_string(filename, &line, &col,
                                map, size, &next, &token) != 0) {
        dmnsn_diagnostic(filename, line, col, "Invalid string");
        goto bailout;
      }
      break;

    default:
      if (dmnsn_tokenize_label(filename, &line, &col,
                               map, size, &next, &token) != 0) {
        /* Unrecognised character */
        dmnsn_diagnostic(filename, line, col,
                         "Unrecognized character '%c' (0x%X)",
                         (int)*next, (unsigned int)*next);
        goto bailout;
      }
      break;
    }

    token.filename = strdup(filename);
    dmnsn_array_push(tokens, &token);
  }

  munmap(map, size);

  /* Restore the original locale */
  setlocale(LC_CTYPE, lc_ctype);
  setlocale(LC_NUMERIC, lc_numeric);
  free(lc_ctype);
  free(lc_numeric);

  return tokens;

 bailout:
  dmnsn_delete_tokens(tokens);
  munmap(map, size);

  /* Restore the original locale */
  setlocale(LC_CTYPE, lc_ctype);
  setlocale(LC_NUMERIC, lc_numeric);
  free(lc_ctype);
  free(lc_numeric);

  return NULL;
}

void
dmnsn_delete_tokens(dmnsn_array *tokens)
{
  dmnsn_token *token;
  unsigned int i;
  for (i = 0; i < dmnsn_array_size(tokens); ++i) {
    token = dmnsn_array_at(tokens, i);
    free(token->filename);
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
#define dmnsn_token_map(type, str)                                             \
  case type:                                                                   \
    return str;

  /* Punctuation */
  dmnsn_token_map(DMNSN_T_LBRACE,   "{");
  dmnsn_token_map(DMNSN_T_RBRACE,   "}")
  dmnsn_token_map(DMNSN_T_LPAREN,   "\\(");
  dmnsn_token_map(DMNSN_T_RPAREN,   "\\)");
  dmnsn_token_map(DMNSN_T_LBRACKET, "[");
  dmnsn_token_map(DMNSN_T_RBRACKET, "]");
  dmnsn_token_map(DMNSN_T_LT,       "<");
  dmnsn_token_map(DMNSN_T_GT,       ">");
  dmnsn_token_map(DMNSN_T_PLUS,     "+");
  dmnsn_token_map(DMNSN_T_MINUS,    "-");
  dmnsn_token_map(DMNSN_T_STAR,     "*");
  dmnsn_token_map(DMNSN_T_SLASH,    "/");
  dmnsn_token_map(DMNSN_T_COMMA,    ",");

  /* Numeric values */
  dmnsn_token_map(DMNSN_T_INT,   "int");
  dmnsn_token_map(DMNSN_T_FLOAT, "float");

  /* Keywords */
  dmnsn_token_map(DMNSN_T_CAMERA, "camera");
  dmnsn_token_map(DMNSN_T_COLOR,  "color");
  dmnsn_token_map(DMNSN_T_SPHERE, "sphere");
  dmnsn_token_map(DMNSN_T_BOX,    "box");

  /* Directives */
  dmnsn_token_map(DMNSN_T_BREAK,      "#break");
  dmnsn_token_map(DMNSN_T_CASE,       "#case");
  dmnsn_token_map(DMNSN_T_DEBUG,      "#debug");
  dmnsn_token_map(DMNSN_T_DECLARE,    "#declare");
  dmnsn_token_map(DMNSN_T_DEFAULT,    "#default");
  dmnsn_token_map(DMNSN_T_ELSE,       "#else");
  dmnsn_token_map(DMNSN_T_END,        "#end");
  dmnsn_token_map(DMNSN_T_ERROR,      "#error");
  dmnsn_token_map(DMNSN_T_FCLOSE,     "#fclose");
  dmnsn_token_map(DMNSN_T_FOPEN,      "#fopen");
  dmnsn_token_map(DMNSN_T_IF,         "#if");
  dmnsn_token_map(DMNSN_T_IFDEF,      "#ifdef");
  dmnsn_token_map(DMNSN_T_IFNDEF,     "#ifndef");
  dmnsn_token_map(DMNSN_T_INCLUDE,    "#include");
  dmnsn_token_map(DMNSN_T_LOCAL,      "#local");
  dmnsn_token_map(DMNSN_T_MACRO,      "#macro");
  dmnsn_token_map(DMNSN_T_RANGE,      "#range");
  dmnsn_token_map(DMNSN_T_READ,       "#read");
  dmnsn_token_map(DMNSN_T_RENDER,     "#render");
  dmnsn_token_map(DMNSN_T_STATISTICS, "#statistics");
  dmnsn_token_map(DMNSN_T_SWITCH,     "#switch");
  dmnsn_token_map(DMNSN_T_UNDEF,      "#undef");
  dmnsn_token_map(DMNSN_T_VERSION,    "#version");
  dmnsn_token_map(DMNSN_T_WARNING,    "#warning");
  dmnsn_token_map(DMNSN_T_WHILE,      "#while");
  dmnsn_token_map(DMNSN_T_WRITE,      "#write");

  /* Strings */
  dmnsn_token_map(DMNSN_T_STRING, "string");

  /* Identifiers */
  dmnsn_token_map(DMNSN_T_IDENTIFIER, "identifier");

  default:
    printf("Warning: unrecognised token %d.\n", (int)token_type);
    return "unrecognized-token";
  }
}
