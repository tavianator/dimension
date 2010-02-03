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

#include "tokenize.h"
#include "directives.h"
#include "utility.h"
#include <stdbool.h>

typedef struct dmnsn_buffered_token {
  int type;
  dmnsn_parse_item lval;
  dmnsn_parse_location lloc;
} dmnsn_buffered_token;

typedef struct dmnsn_token_buffer {
  int type;
  /* Indicate that the first token should be returned as-is */
  #define DMNSN_T_LEX_VERBATIM DMNSN_T_EOF

  dmnsn_array *buffered;
  unsigned int i;

  struct dmnsn_token_buffer *prev;
} dmnsn_token_buffer;

static dmnsn_token_buffer *
dmnsn_new_token_buffer(int type, dmnsn_token_buffer *prev)
{
  dmnsn_token_buffer *tbuffer = malloc(sizeof(dmnsn_token_buffer));
  if (!tbuffer) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Failed to allocate token buffer.");
  }

  tbuffer->type = type;
  tbuffer->buffered = dmnsn_new_array(sizeof(dmnsn_buffered_token));
  tbuffer->i = 0;
  tbuffer->prev = prev;
  return tbuffer;
}

static void
dmnsn_delete_token_buffer(dmnsn_token_buffer *tbuffer)
{
  if (tbuffer) {
    dmnsn_delete_array(tbuffer->buffered);
    free(tbuffer);
  }
}

int dmnsn_yylex_impl(dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                     const char *filename, void *yyscanner);
int dmnsn_ld_yyparse(const char *filename, void *yyscanner,
                     dmnsn_symbol_table *symtable);

static dmnsn_token_buffer *
dmnsn_declaration_buffer(int token, dmnsn_token_buffer *prev,
                         dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                         const char *filename, dmnsn_symbol_table *symtable,
                         void *yyscanner)
{
  dmnsn_token_buffer *tbuffer
    = dmnsn_new_token_buffer(DMNSN_T_LEX_VERBATIM, prev);

  /* Buffer the current token */
  dmnsn_buffered_token buffered = {
    .type = token,
    .lval = *lvalp,
    .lloc = *llocp
  };
  dmnsn_array_push(tbuffer->buffered, &buffered);

  /* Grab all the tokens belonging to the #declare/#local, i.e. until the braces
     balance or we hit a semicolon */
  int bracelevel = -1;
  while (1) {
    /* Recursive call - permit other directives inside the declaration */
    buffered.type = dmnsn_yylex(&buffered.lval, &buffered.lloc,
                                filename, symtable, yyscanner);

    if (buffered.type == DMNSN_T_EOF) {
      dmnsn_diagnostic(filename, buffered.lloc.first_line,
                       buffered.lloc.first_column,
                       "syntax error, unexpected end-of-file");
      dmnsn_delete_token_buffer(tbuffer);
      return NULL;
    } else if (buffered.type == DMNSN_T_LEX_ERROR) {
      dmnsn_delete_token_buffer(tbuffer);
      return NULL;
    }

    dmnsn_array_push(tbuffer->buffered, &buffered);

    if (buffered.type == DMNSN_T_LBRACE) {
      if (bracelevel < 0)
        bracelevel = 1;
      else
        ++bracelevel;
    } else if (buffered.type == DMNSN_T_RBRACE) {
      --bracelevel;
      if (bracelevel == 0) {
        break;
      }
    } else if (buffered.type == DMNSN_T_SEMICOLON) {
      break;
    }
  }

  /* Fake EOF */
  buffered.type = DMNSN_T_EOF;
  dmnsn_array_push(tbuffer->buffered, &buffered);

  return tbuffer;
}

static dmnsn_token_buffer *
dmnsn_undef_buffer(int token, dmnsn_token_buffer *prev,
                   dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                   const char *filename, dmnsn_symbol_table *symtable,
                   void *yyscanner)
{
  dmnsn_token_buffer *tbuffer
    = dmnsn_new_token_buffer(DMNSN_T_LEX_VERBATIM, prev);

  /* Buffer the current token */
  dmnsn_buffered_token buffered = {
    .type = token,
    .lval = *lvalp,
    .lloc = *llocp
  };
  dmnsn_array_push(tbuffer->buffered, &buffered);

  /* Recursive call */
  buffered.type = dmnsn_yylex(&buffered.lval, &buffered.lloc,
                              filename, symtable, yyscanner);

  if (buffered.type == DMNSN_T_EOF) {
    dmnsn_diagnostic(filename, buffered.lloc.first_line,
                     buffered.lloc.first_column,
                     "syntax error, unexpected end-of-file");
    dmnsn_delete_token_buffer(tbuffer);
    return NULL;
  } else if (buffered.type == DMNSN_T_LEX_ERROR) {
    dmnsn_delete_token_buffer(tbuffer);
    return NULL;
  }
  /* Buffer the next token */
  dmnsn_array_push(tbuffer->buffered, &buffered);

  /* Fake EOF */
  buffered.type = DMNSN_T_EOF;
  dmnsn_array_push(tbuffer->buffered, &buffered);

  return tbuffer;
}

static dmnsn_token_buffer *
dmnsn_if_buffer(int token, dmnsn_token_buffer *prev,
                dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                const char *filename, dmnsn_symbol_table *symtable,
                void *yyscanner)
{
  dmnsn_token_buffer *cond_buffer
    = dmnsn_new_token_buffer(DMNSN_T_LEX_VERBATIM, prev);

  /* Buffer the current token */
  dmnsn_buffered_token buffered = {
    .type = token,
    .lval = *lvalp,
    .lloc = *llocp
  };
  dmnsn_array_push(cond_buffer->buffered, &buffered);

  /* Grab all the tokens belonging to the #if (...)  */
  int parenlevel = -1;
  while (1) {
    /* Recursive call - permit other directives inside the condition */
    buffered.type = dmnsn_yylex(&buffered.lval, &buffered.lloc,
                                filename, symtable, yyscanner);

    if (buffered.type == DMNSN_T_EOF) {
      dmnsn_diagnostic(filename, buffered.lloc.first_line,
                       buffered.lloc.first_column,
                       "syntax error, unexpected end-of-file");
      dmnsn_delete_token_buffer(cond_buffer);
      return NULL;
    } else if (buffered.type == DMNSN_T_LEX_ERROR) {
      dmnsn_delete_token_buffer(cond_buffer);
      return NULL;
    }

    dmnsn_array_push(cond_buffer->buffered, &buffered);

    if (buffered.type == DMNSN_T_LPAREN) {
      if (parenlevel < 0)
        parenlevel = 1;
      else
        ++parenlevel;
    } else if (buffered.type == DMNSN_T_RPAREN) {
      --parenlevel;
      if (parenlevel == 0) {
        break;
      }
    }
  }

  /* Fake EOF */
  buffered.type = DMNSN_T_EOF;
  dmnsn_array_push(cond_buffer->buffered, &buffered);

  dmnsn_yyset_extra(cond_buffer, yyscanner);
  if (dmnsn_ld_yyparse(filename, yyscanner, symtable) != 0) {
    dmnsn_yyset_extra(cond_buffer->prev, yyscanner);
    dmnsn_delete_token_buffer(cond_buffer);
    return NULL;
  }

  dmnsn_yyset_extra(cond_buffer->prev, yyscanner);
  dmnsn_delete_token_buffer(cond_buffer);

  dmnsn_token_buffer *tbuffer= dmnsn_new_token_buffer(DMNSN_T_IF, prev);

  dmnsn_astnode *cnode = dmnsn_find_symbol(symtable, "__cond__");
  if (!cnode) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "__cond__ unset.");
  }

  bool cond = false;
  if (cnode->type == DMNSN_AST_INTEGER) {
    cond = (*(long *)cnode->ptr) ? true : false;
  } else if (cnode->type == DMNSN_AST_FLOAT) {
    cond = (*(double *)cnode->ptr) ? true : false;
  } else {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "__cond__ has wrong type.");
  }

  dmnsn_undef_symbol(symtable, "__cond__");

  int nesting = 1;
  while (1) {
    /* Non-recursive call */
    buffered.type = dmnsn_yylex_impl(&buffered.lval, &buffered.lloc,
                                     filename, yyscanner);

    if (buffered.type == DMNSN_T_EOF) {
      dmnsn_diagnostic(filename, buffered.lloc.first_line,
                       buffered.lloc.first_column,
                       "syntax error, unexpected end-of-file");
      dmnsn_delete_token_buffer(tbuffer);
      return NULL;
    }

    switch (buffered.type) {
    case DMNSN_T_IF:
    case DMNSN_T_IFDEF:
    case DMNSN_T_IFNDEF:
    case DMNSN_T_MACRO:
    case DMNSN_T_SWITCH:
    case DMNSN_T_WHILE:
      ++nesting;
      break;

    case DMNSN_T_END:
      --nesting;
      break;

    default:
      break;
    }

    if (nesting == 0) {
      break;
    } else if (nesting == 1 && buffered.type == DMNSN_T_ELSE) {
      cond = !cond;
      continue;
    }

    if (cond) {
      if (buffered.type == DMNSN_T_LEX_ERROR) {
        dmnsn_delete_token_buffer(tbuffer);
        return NULL;
      } else {
        dmnsn_array_push(tbuffer->buffered, &buffered);
      }
    } else {
      free(buffered.lval.value);
    }
  }

  return tbuffer;
}

int
dmnsn_yylex(dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
            const char *filename, dmnsn_symbol_table *symtable, void *yyscanner)
{
  /*
   * So... this is kind of ugly.  POV-Ray's language directives are not parsable
   * by reasonable bison grammar, since some require skipping arbitrary amounts
   * of tokens, or repeatedly parsing tokens.  Instead, they are implemented
   * transparently by the lexer.  The lexing function calls a separate parser
   * to handle language directives as they arrise, and returns only non-
   * directive tokens.
   *
   * Ideally we'd use a push parser for the language directives, but bison
   * doesn't support GLR push parsers.  Instead, we buffer all the appropriate
   * tokens and call a pull parser, then discard the buffer and continue.
   */

  dmnsn_token_buffer *tbuffer = dmnsn_yyget_extra(yyscanner);

  int token;
  while (1) {
    if (tbuffer) {
      /* Return buffered tokens */
      dmnsn_buffered_token buffered;

      if (tbuffer->i < dmnsn_array_size(tbuffer->buffered)) {
        dmnsn_array_get(tbuffer->buffered, tbuffer->i, &buffered);
        token = buffered.type;
        *lvalp = buffered.lval;
        *llocp = buffered.lloc;
        ++tbuffer->i;

        if (tbuffer->type == DMNSN_T_LEX_VERBATIM && tbuffer->i == 1) {
          /* Don't double-process the first token */
          return token;
        }
      } else {
        dmnsn_yyset_extra(tbuffer->prev, yyscanner);
        dmnsn_delete_token_buffer(tbuffer);
        tbuffer = dmnsn_yyget_extra(yyscanner);
        continue;
      }
    } else {
      token = dmnsn_yylex_impl(lvalp, llocp, filename, yyscanner);
    }

    switch (token) {
    case DMNSN_T_DECLARE:
    case DMNSN_T_LOCAL:
      {
        dmnsn_token_buffer *tb = dmnsn_declaration_buffer(
          token, tbuffer, lvalp, llocp, filename, symtable, yyscanner
        );
        if (!tb) {
          return DMNSN_T_LEX_ERROR;
        }

        dmnsn_yyset_extra(tb, yyscanner);
        if (dmnsn_ld_yyparse(filename, yyscanner, symtable) != 0) {
          dmnsn_yyset_extra(tb->prev, yyscanner);
          dmnsn_delete_token_buffer(tb);
          return DMNSN_T_LEX_ERROR;
        }

        /* Restore the previous extra pointer */
        dmnsn_yyset_extra(tb->prev, yyscanner);
        dmnsn_delete_token_buffer(tb);
        break;
      }

    case DMNSN_T_UNDEF:
      {
        dmnsn_token_buffer *tb = dmnsn_undef_buffer(
          token, tbuffer, lvalp, llocp, filename, symtable, yyscanner
        );
        if (!tb) {
          return DMNSN_T_LEX_ERROR;
        }

        dmnsn_yyset_extra(tb, yyscanner);
        if (dmnsn_ld_yyparse(filename, yyscanner, symtable) != 0) {
          dmnsn_yyset_extra(tb->prev, yyscanner);
          dmnsn_delete_token_buffer(tb);
          return DMNSN_T_LEX_ERROR;
        }

        /* Restore the previous extra pointer */
        dmnsn_yyset_extra(tb->prev, yyscanner);
        dmnsn_delete_token_buffer(tb);
        break;
      }

    case DMNSN_T_IF:
      {
        dmnsn_token_buffer *tb = dmnsn_if_buffer(
          token, tbuffer, lvalp, llocp, filename, symtable, yyscanner
        );
        if (!tb) {
          return DMNSN_T_LEX_ERROR;
        }

        dmnsn_yyset_extra(tb, yyscanner);
        tbuffer = tb;
        continue;
      }

    default:
      return token;
    }
  }
}
