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

typedef struct dmnsn_buffered_token {
  int type;
  dmnsn_parse_item lval;
  dmnsn_parse_location lloc;
} dmnsn_buffered_token;

typedef struct dmnsn_yylex_extra {
  int type;
  /* Indicate that the first token should be returned as-is */
  #define DMNSN_T_LEX_VERBATIM DMNSN_T_EOF

  dmnsn_array *buffered;
  unsigned int i;

  struct dmnsn_yylex_extra *prev;
} dmnsn_yylex_extra;

dmnsn_yylex_extra *
dmnsn_new_yylex_extra(int type)
{
  dmnsn_yylex_extra *extra = malloc(sizeof(dmnsn_yylex_extra));
  if (!extra) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Failed to allocate token buffer.");
  }

  extra->type = type;
  extra->buffered = dmnsn_new_array(sizeof(dmnsn_buffered_token));
  extra->i = 0;
  extra->prev = NULL;
  return extra;
}

void
dmnsn_delete_yylex_extra(dmnsn_yylex_extra *extra)
{
  if (extra) {
    dmnsn_delete_array(extra->buffered);
    free(extra);
  }
}

int dmnsn_yylex_impl(dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                     const char *filename, void *yyscanner);
int dmnsn_ld_yyparse(const char *filename, void *yyscanner,
                     dmnsn_symbol_table *symtable);

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
   * doesn't support GLR push parsers.
   */

  dmnsn_yylex_extra *extra = dmnsn_yyget_extra(yyscanner);

  int token;
  while (1) {
    if (extra) {
      /* We are returning buffered tokens */
      dmnsn_buffered_token buffered;

      if (extra->i < dmnsn_array_size(extra->buffered)) {
        dmnsn_array_get(extra->buffered, extra->i, &buffered);
        token = buffered.type;
        *lvalp = buffered.lval;
        *llocp = buffered.lloc;
        ++extra->i;

        if (extra->type == DMNSN_T_LEX_VERBATIM && extra->i == 1) {
          /* Don't reprocess the first token in some situations */
          return token;
        }
      } else {
        dmnsn_yyset_extra(extra->prev, yyscanner);
        dmnsn_delete_yylex_extra(extra);
        extra = dmnsn_yyget_extra(yyscanner);
        continue;
      }
    } else {
      token = dmnsn_yylex_impl(lvalp, llocp, filename, yyscanner);
    }

    switch (token) {
    case DMNSN_T_DECLARE:
    case DMNSN_T_LOCAL:
      {
        dmnsn_yylex_extra *ex = dmnsn_new_yylex_extra(DMNSN_T_LEX_VERBATIM);
        ex->prev = extra;

        /* Buffer the current token */
        dmnsn_buffered_token buffered = {
          .type = token,
          .lval = *lvalp,
          .lloc = *llocp
        };
        dmnsn_array_push(ex->buffered, &buffered);

        /* Grab all the tokens belonging to the #declare/#local */
        int bracelevel = -1;
        while (1) {
          /* Recursive call */
          buffered.type = dmnsn_yylex(&buffered.lval, &buffered.lloc,
                                      filename, symtable, yyscanner);

          if (buffered.type == DMNSN_T_EOF) {
            dmnsn_diagnostic(filename, buffered.lloc.first_line,
                             buffered.lloc.first_column,
                             "unexpected end-of-file");
            dmnsn_delete_yylex_extra(ex);
            return DMNSN_T_LEX_ERROR;
          } else if (buffered.type == DMNSN_T_LEX_ERROR) {
            dmnsn_delete_yylex_extra(ex);
            return DMNSN_T_LEX_ERROR;
          }

          dmnsn_array_push(ex->buffered, &buffered);

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
        dmnsn_array_push(ex->buffered, &buffered);

        dmnsn_yyset_extra(ex, yyscanner);
        if (dmnsn_ld_yyparse(filename, yyscanner, symtable) != 0) {
          dmnsn_yyset_extra(ex->prev, yyscanner);
          dmnsn_delete_yylex_extra(ex);
          return DMNSN_T_LEX_ERROR;
        }

        /* Restore the previous extra pointer */
        dmnsn_yyset_extra(ex->prev, yyscanner);
        dmnsn_delete_yylex_extra(ex);
        break;
      }

    case DMNSN_T_UNDEF:
      {
        dmnsn_yylex_extra *ex = dmnsn_new_yylex_extra(DMNSN_T_LEX_VERBATIM);
        ex->prev = extra;

        /* Buffer the current token */
        dmnsn_buffered_token buffered = {
          .type = token,
          .lval = *lvalp,
          .lloc = *llocp
        };
        dmnsn_array_push(ex->buffered, &buffered);

        /* Recursive call */
        buffered.type = dmnsn_yylex(&buffered.lval, &buffered.lloc,
                                    filename, symtable, yyscanner);

        if (buffered.type == DMNSN_T_EOF) {
          dmnsn_diagnostic(filename, buffered.lloc.first_line,
                           buffered.lloc.first_column,
                           "unexpected end-of-file");
          dmnsn_delete_yylex_extra(ex);
          return DMNSN_T_LEX_ERROR;
        } else if (buffered.type == DMNSN_T_LEX_ERROR) {
          dmnsn_delete_yylex_extra(ex);
          return DMNSN_T_LEX_ERROR;
        }
        /* Buffer the next token */
        dmnsn_array_push(ex->buffered, &buffered);

        /* Fake EOF */
        buffered.type = DMNSN_T_EOF;
        dmnsn_array_push(ex->buffered, &buffered);

        dmnsn_yyset_extra(ex, yyscanner);
        if (dmnsn_ld_yyparse(filename, yyscanner, symtable) != 0) {
          dmnsn_yyset_extra(ex->prev, yyscanner);
          dmnsn_delete_yylex_extra(ex);
          return DMNSN_T_LEX_ERROR;
        }

        /* Restore the previous extra pointer */
        dmnsn_yyset_extra(ex->prev, yyscanner);
        dmnsn_delete_yylex_extra(ex);
        break;
      }

    default:
      return token;
    }
  }
}
