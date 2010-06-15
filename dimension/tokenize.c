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
#include <libgen.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct dmnsn_buffered_token {
  dmnsn_token_type type;
  dmnsn_parse_item lval;
  dmnsn_parse_location lloc;
} dmnsn_buffered_token;

typedef struct dmnsn_token_buffer {
  dmnsn_token_type type;
  /* Indicate that the first token should be returned as-is */
  #define DMNSN_T_LEX_VERBATIM DMNSN_T_EOF

  dmnsn_parse_location lloc;
  dmnsn_array *buffered;
  size_t i;

  struct dmnsn_token_buffer *prev;

  const char *filename;
  void *ptr;
} dmnsn_token_buffer;

static dmnsn_token_buffer *
dmnsn_new_token_buffer(dmnsn_parse_location lloc, dmnsn_token_type type,
                       dmnsn_token_buffer *prev, const char *filename)
{
  dmnsn_token_buffer *tbuffer = dmnsn_malloc(sizeof(dmnsn_token_buffer));

  tbuffer->type = type;
  tbuffer->lloc = lloc;
  tbuffer->buffered = dmnsn_new_array(sizeof(dmnsn_buffered_token));
  tbuffer->i = 0;
  tbuffer->prev = prev;
  tbuffer->filename = filename;
  tbuffer->ptr = NULL;
  return tbuffer;
}

static void
dmnsn_delete_token_buffer(void *ptr)
{
  dmnsn_token_buffer *tbuffer = ptr;
  if (tbuffer) {
    DMNSN_ARRAY_FOREACH (dmnsn_buffered_token *, buffered, tbuffer->buffered) {
      free(buffered->lval.value);
    }

    dmnsn_delete_array(tbuffer->buffered);
    free(tbuffer);
  }
}

static int
dmnsn_yylex_wrapper(dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                    const char *filename, dmnsn_symbol_table *symtable,
                    void *yyscanner);
int dmnsn_ld_yyparse(const char *filename, void *yyscanner,
                     dmnsn_symbol_table *symtable);

static int
dmnsn_buffer_balanced(dmnsn_token_buffer *tbuffer, bool recursive,
                      dmnsn_token_type left, dmnsn_token_type right,
                      const char *filename, dmnsn_symbol_table *symtable,
                      void *yyscanner)
{
  dmnsn_buffered_token buffered;

  int nesting = -1;
  while (1) {
    if (recursive) {
      buffered.type = dmnsn_yylex(&buffered.lval, &buffered.lloc,
                                  filename, symtable, yyscanner);
    } else {
      buffered.type = dmnsn_yylex_wrapper(&buffered.lval, &buffered.lloc,
                                          filename, symtable, yyscanner);
    }

    if (buffered.type == DMNSN_T_EOF) {
      dmnsn_diagnostic(buffered.lloc, "syntax error, unexpected end-of-file");
      return 1;
    } else if (buffered.type == DMNSN_T_LEX_ERROR) {
      return 1;
    }

    dmnsn_array_push(tbuffer->buffered, &buffered);

    if (buffered.type == left) {
      if (nesting < 0)
        nesting = 1;
      else
        ++nesting;
    } else if (buffered.type == right) {
      --nesting;
      if (nesting == 0) {
        break;
      }
    }
  }

  return 0;
}

static int
dmnsn_buffer_strexp(dmnsn_token_buffer *tbuffer, bool recursive,
                    const char *filename, dmnsn_symbol_table *symtable,
                    void *yyscanner)
{
  dmnsn_buffered_token buffered;

  if (recursive) {
    buffered.type = dmnsn_yylex(&buffered.lval, &buffered.lloc,
                                filename, symtable, yyscanner);
  } else {
    buffered.type = dmnsn_yylex_wrapper(&buffered.lval, &buffered.lloc,
                                        filename, symtable, yyscanner);
  }

  if (buffered.type == DMNSN_T_EOF) {
    dmnsn_diagnostic(buffered.lloc, "syntax error, unexpected end-of-file");
    return 1;
  } else if (buffered.type == DMNSN_T_LEX_ERROR) {
    return 1;
  }
  /* Buffer the first token */
  dmnsn_array_push(tbuffer->buffered, &buffered);

  bool is_strexp = buffered.type != DMNSN_T_STRING;
  if (buffered.type == DMNSN_T_IDENTIFIER) {
    /* Check if it's a string identifier or a macro */
    dmnsn_astnode *inode = dmnsn_find_symbol(symtable, buffered.lval.value);
    if (!inode || inode->type == DMNSN_AST_STRING) {
      is_strexp = false;
    }
  }

  if (is_strexp) {
    /* Grab all the tokens belonging to the string expression  */
    return dmnsn_buffer_balanced(tbuffer, recursive,
                                 DMNSN_T_LPAREN, DMNSN_T_RPAREN,
                                 filename, symtable, yyscanner);
  }

  return 0;
}

static dmnsn_token_buffer *
dmnsn_include_buffer(int token, dmnsn_token_buffer *prev,
                     dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                     const char *filename, dmnsn_symbol_table *symtable,
                     void *yyscanner)
{
  dmnsn_token_buffer *include_buffer
    = dmnsn_new_token_buffer(*llocp, DMNSN_T_LEX_VERBATIM, prev, filename);

  /* Buffer the current token */
  dmnsn_buffered_token buffered = {
    .type = token,
    .lval = *lvalp,
    .lloc = *llocp
  };
  dmnsn_array_push(include_buffer->buffered, &buffered);

  /* Buffer the following string expression */
  if (dmnsn_buffer_strexp(include_buffer, true, filename, symtable, yyscanner)
      != 0)
  {
    dmnsn_delete_token_buffer(include_buffer);
    return NULL;
  }

  /* Fake EOF */
  buffered.type = DMNSN_T_EOF;
  buffered.lval.value = NULL;
  dmnsn_array_push(include_buffer->buffered, &buffered);

  dmnsn_yyset_extra(include_buffer, yyscanner);
  if (dmnsn_ld_yyparse(filename, yyscanner, symtable) != 0) {
    dmnsn_yyset_extra(include_buffer->prev, yyscanner);
    dmnsn_delete_token_buffer(include_buffer);
    return NULL;
  }

  dmnsn_yyset_extra(include_buffer->prev, yyscanner);
  dmnsn_delete_token_buffer(include_buffer);

  dmnsn_token_buffer *tbuffer
    = dmnsn_new_token_buffer(*llocp, token, prev, filename);

  dmnsn_astnode *inode = dmnsn_find_symbol(symtable, "$include");
  dmnsn_assert(inode, "$include unset.");
  dmnsn_assert(inode->type == DMNSN_AST_STRING, "$include has wrong type.");

  const char *include = inode->ptr;
  char *filename_copy = dmnsn_strdup(filename);
  char *localdir = dirname(filename_copy);
  char *local_include = dmnsn_malloc(strlen(localdir) + strlen(include) + 2);
  strcpy(local_include, localdir);
  strcat(local_include, "/");
  strcat(local_include, include);
  free(filename_copy);

  FILE *file = fopen(local_include, "r");
  if (!file) {
    dmnsn_diagnostic(*llocp, "Couldn't open include file '%s'", include);
    dmnsn_undef_symbol(symtable, "$include");
    free(local_include);
    dmnsn_delete_token_buffer(tbuffer);
    return NULL;
  }
  tbuffer->ptr = file;

  void *buffer = dmnsn_yy_make_buffer(file, yyscanner);
  if (!buffer) {
    dmnsn_diagnostic(*llocp, "Couldn't allocate buffer for include file '%s'",
                     include);
    dmnsn_undef_symbol(symtable, "$include");
    fclose(file);
    free(local_include);
    dmnsn_delete_token_buffer(tbuffer);
    return NULL;
  }
  dmnsn_yy_push_buffer(buffer, yyscanner);

  /* Stuff the filename in the symbol table to persist it */
  dmnsn_astnode *includes = dmnsn_find_symbol(symtable, "$includes");
  if (!includes) {
    dmnsn_declare_symbol(symtable, "$includes", dmnsn_new_ast_array());
    includes = dmnsn_find_symbol(symtable, "$includes");
  }
  dmnsn_assert(includes, "$includes unset.");
  dmnsn_assert(includes->type == DMNSN_AST_ARRAY,
               "$includes has wrong type.");

  dmnsn_astnode fnode = dmnsn_new_ast_string(local_include);
  free(local_include);
  tbuffer->filename = fnode.ptr;
  dmnsn_array_push(includes->children, &fnode);

  dmnsn_push_scope(symtable);

  dmnsn_undef_symbol(symtable, "$include");
  return tbuffer;
}

static dmnsn_token_buffer *
dmnsn_declaration_buffer(int token, dmnsn_token_buffer *prev,
                         dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                         const char *filename, dmnsn_symbol_table *symtable,
                         void *yyscanner)
{
  dmnsn_token_buffer *tbuffer
    = dmnsn_new_token_buffer(*llocp, DMNSN_T_LEX_VERBATIM, prev, filename);

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
      dmnsn_diagnostic(buffered.lloc, "syntax error, unexpected end-of-file");
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
  buffered.lval.value = NULL;
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
    = dmnsn_new_token_buffer(*llocp, DMNSN_T_LEX_VERBATIM, prev, filename);

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
    dmnsn_diagnostic(buffered.lloc, "syntax error, unexpected end-of-file");
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
  buffered.lval.value = NULL;
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
    = dmnsn_new_token_buffer(*llocp, DMNSN_T_LEX_VERBATIM, prev, filename);

  /* Buffer the current token */
  dmnsn_buffered_token buffered = {
    .type = token,
    .lval = *lvalp,
    .lloc = *llocp
  };
  dmnsn_array_push(cond_buffer->buffered, &buffered);

  /* Grab all the tokens belonging to the #if (...)  */
  if (dmnsn_buffer_balanced(cond_buffer, true, DMNSN_T_LPAREN, DMNSN_T_RPAREN,
                            filename, symtable, yyscanner)
      != 0)
  {
    dmnsn_delete_token_buffer(cond_buffer);
    return NULL;
  }

  /* Fake EOF */
  buffered.type = DMNSN_T_EOF;
  buffered.lval.value = NULL;
  dmnsn_array_push(cond_buffer->buffered, &buffered);

  dmnsn_yyset_extra(cond_buffer, yyscanner);
  if (dmnsn_ld_yyparse(filename, yyscanner, symtable) != 0) {
    dmnsn_yyset_extra(cond_buffer->prev, yyscanner);
    dmnsn_delete_token_buffer(cond_buffer);
    return NULL;
  }

  dmnsn_yyset_extra(cond_buffer->prev, yyscanner);
  dmnsn_delete_token_buffer(cond_buffer);

  dmnsn_token_buffer *tbuffer
    = dmnsn_new_token_buffer(*llocp, token, prev, filename);

  dmnsn_astnode *cnode = dmnsn_find_symbol(symtable, "$cond");
  dmnsn_assert(cnode, "$cond unset.");

  bool cond = false;
  if (cnode->type == DMNSN_AST_INTEGER) {
    cond = (*(long *)cnode->ptr) ? true : false;
  } else if (cnode->type == DMNSN_AST_FLOAT) {
    cond = (*(double *)cnode->ptr) ? true : false;
  } else {
    dmnsn_assert(false, "$cond has wrong type.");
  }

  dmnsn_undef_symbol(symtable, "$cond");

  int nesting = 1, else_seen = 0;
  while (1) {
    /* Non-recursive call */
    buffered.type = dmnsn_yylex_wrapper(&buffered.lval, &buffered.lloc,
                                        filename, symtable, yyscanner);

    if (buffered.type == DMNSN_T_EOF) {
      dmnsn_diagnostic(buffered.lloc, "syntax error, unexpected end-of-file");
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
      if (else_seen
          || (tbuffer->prev && tbuffer->prev->type == DMNSN_T_WHILE))
      {
        dmnsn_diagnostic(buffered.lloc, "syntax error, unexpected #else");
        dmnsn_delete_token_buffer(tbuffer);
        return NULL;
      } else {
        cond = !cond;
        else_seen = 1;
        continue;
      }
    }

    if (cond) {
      dmnsn_array_push(tbuffer->buffered, &buffered);
    } else {
      free(buffered.lval.value);
    }
  }

  return tbuffer;
}

static dmnsn_token_buffer *
dmnsn_while_buffer(int token, dmnsn_token_buffer *prev,
                   dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                   const char *filename, dmnsn_symbol_table *symtable,
                   void *yyscanner)
{
  dmnsn_token_buffer *tbuffer
    = dmnsn_new_token_buffer(*llocp, token, prev, filename);

  /* Pretend to be an if */
  dmnsn_buffered_token buffered = {
    .type = DMNSN_T_IF,
    .lval = *lvalp,
    .lloc = *llocp
  };
  dmnsn_array_push(tbuffer->buffered, &buffered);

  /* Grab all the tokens belonging to the #while ... #end */
  int nesting = 1;
  while (1) {
    /* Non-recursive call */
    buffered.type = dmnsn_yylex_wrapper(&buffered.lval, &buffered.lloc,
                                        filename, symtable, yyscanner);

    if (buffered.type == DMNSN_T_EOF) {
      dmnsn_diagnostic(buffered.lloc, "syntax error, unexpected end-of-file");
      dmnsn_delete_token_buffer(tbuffer);
      return NULL;
    }

    dmnsn_array_push(tbuffer->buffered, &buffered);

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
    }
  }

  return tbuffer;
}

static dmnsn_token_buffer *
dmnsn_version_buffer(int token, dmnsn_token_buffer *prev,
                     dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                     const char *filename, dmnsn_symbol_table *symtable,
                     void *yyscanner)
{
  dmnsn_token_buffer *tbuffer
    = dmnsn_new_token_buffer(*llocp, DMNSN_T_LEX_VERBATIM, prev, filename);

  /* Buffer the current token */
  dmnsn_buffered_token buffered = {
    .type = token,
    .lval = *lvalp,
    .lloc = *llocp
  };
  dmnsn_array_push(tbuffer->buffered, &buffered);

  while (buffered.type != DMNSN_T_SEMICOLON) {
    /* Recursive call */
    buffered.type = dmnsn_yylex(&buffered.lval, &buffered.lloc,
                                filename, symtable, yyscanner);

    if (buffered.type == DMNSN_T_EOF) {
      dmnsn_diagnostic(buffered.lloc, "syntax error, unexpected end-of-file");
      dmnsn_delete_token_buffer(tbuffer);
      return NULL;
    } else if (buffered.type == DMNSN_T_LEX_ERROR) {
      dmnsn_delete_token_buffer(tbuffer);
      return NULL;
    }

    dmnsn_array_push(tbuffer->buffered, &buffered);
  }

  /* Fake EOF */
  buffered.type = DMNSN_T_EOF;
  buffered.lval.value = NULL;
  dmnsn_array_push(tbuffer->buffered, &buffered);

  return tbuffer;
}

static dmnsn_token_buffer *
dmnsn_stream_buffer(int token, dmnsn_token_buffer *prev,
                    dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                    const char *filename, dmnsn_symbol_table *symtable,
                    void *yyscanner)
{
  dmnsn_token_buffer *tbuffer
    = dmnsn_new_token_buffer(*llocp, DMNSN_T_LEX_VERBATIM, prev, filename);

  /* Buffer the current token */
  dmnsn_buffered_token buffered = {
    .type = token,
    .lval = *lvalp,
    .lloc = *llocp
  };
  dmnsn_array_push(tbuffer->buffered, &buffered);

  /* Buffer the following string expression */
  if (dmnsn_buffer_strexp(tbuffer, true, filename, symtable, yyscanner)
      != 0)
  {
    dmnsn_delete_token_buffer(tbuffer);
    return NULL;
  }

  /* Fake EOF */
  buffered.type = DMNSN_T_EOF;
  buffered.lval.value = NULL;
  dmnsn_array_push(tbuffer->buffered, &buffered);

  return tbuffer;
}

static bool
dmnsn_declare_macro(int token, dmnsn_token_buffer *prev,
                    dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                    const char *filename, dmnsn_symbol_table *symtable,
                    void *yyscanner)
{
  dmnsn_token_buffer *decl_buffer
    = dmnsn_new_token_buffer(*llocp, DMNSN_T_LEX_VERBATIM, prev, filename);

  /* Buffer the current token */
  dmnsn_buffered_token buffered = {
    .type = token,
    .lval = *lvalp,
    .lloc = *llocp
  };
  dmnsn_array_push(decl_buffer->buffered, &buffered);

  /* Grab all the tokens belonging to the #macro ID (...)  */
  if (dmnsn_buffer_balanced(decl_buffer, true, DMNSN_T_LPAREN, DMNSN_T_RPAREN,
                            filename, symtable, yyscanner)
      != 0)
  {
    dmnsn_delete_token_buffer(decl_buffer);
    return false;
  }

  /* Fake EOF */
  buffered.type = DMNSN_T_EOF;
  buffered.lval.value = NULL;
  dmnsn_array_push(decl_buffer->buffered, &buffered);

  dmnsn_yyset_extra(decl_buffer, yyscanner);
  if (dmnsn_ld_yyparse(filename, yyscanner, symtable) != 0) {
    dmnsn_yyset_extra(decl_buffer->prev, yyscanner);
    dmnsn_delete_token_buffer(decl_buffer);
    return false;
  }

  dmnsn_yyset_extra(decl_buffer->prev, yyscanner);
  dmnsn_delete_token_buffer(decl_buffer);

  dmnsn_token_buffer *tbuffer
    = dmnsn_new_token_buffer(*llocp, token, NULL, filename);

  dmnsn_astnode *mname = dmnsn_find_symbol(symtable, "$macro");
  dmnsn_assert(mname, "$macro unset.");
  dmnsn_assert(mname->type == DMNSN_AST_STRING, "$macro has wrong type.");
  dmnsn_astnode *mnode = dmnsn_find_symbol(symtable, mname->ptr);
  dmnsn_assert(mnode, "#macro unset.");
  dmnsn_assert(mnode->type == DMNSN_AST_MACRO, "#macro has wrong type.");
  dmnsn_undef_symbol(symtable, "$macro");

  int nesting = 1;
  while (1) {
    /* Non-recursive call */
    buffered.type = dmnsn_yylex_wrapper(&buffered.lval, &buffered.lloc,
                                        filename, symtable, yyscanner);

    if (buffered.type == DMNSN_T_EOF) {
      dmnsn_diagnostic(buffered.lloc, "syntax error, unexpected end-of-file");
      dmnsn_delete_token_buffer(tbuffer);
      return false;
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

    if (nesting == 0)
      break;

    dmnsn_array_push(tbuffer->buffered, &buffered);
  }

  mnode->ptr     = tbuffer;
  mnode->free_fn = &dmnsn_delete_token_buffer;
  return true;
}

static dmnsn_token_buffer *
dmnsn_macro_buffer(int token, dmnsn_astnode *mnode, dmnsn_token_buffer *prev,
                   dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                   const char *filename, dmnsn_symbol_table *symtable,
                   void *yyscanner)
{
  dmnsn_token_buffer *invoke_buffer
    = dmnsn_new_token_buffer(*llocp, DMNSN_T_LEX_VERBATIM, prev, filename);

  /* Buffer the current token */
  dmnsn_buffered_token buffered = {
    .type = token,
    .lval = *lvalp,
    .lloc = *llocp
  };
  dmnsn_array_push(invoke_buffer->buffered, &buffered);

  /* Grab all the tokens belonging to the #macro ID (...)  */
  if (dmnsn_buffer_balanced(invoke_buffer, true, DMNSN_T_LPAREN, DMNSN_T_RPAREN,
                            filename, symtable, yyscanner)
      != 0)
  {
    dmnsn_delete_token_buffer(invoke_buffer);
    return NULL;
  }

  /* Fake EOF */
  buffered.type = DMNSN_T_EOF;
  buffered.lval.value = NULL;
  dmnsn_array_push(invoke_buffer->buffered, &buffered);

  dmnsn_yyset_extra(invoke_buffer, yyscanner);
  dmnsn_push_scope(symtable);
  if (dmnsn_ld_yyparse(filename, yyscanner, symtable) != 0) {
    dmnsn_yyset_extra(invoke_buffer->prev, yyscanner);
    dmnsn_delete_token_buffer(invoke_buffer);
    return NULL;
  }

  dmnsn_yyset_extra(invoke_buffer->prev, yyscanner);
  dmnsn_delete_token_buffer(invoke_buffer);

  dmnsn_token_buffer *tbuffer = mnode->ptr;
  tbuffer->lloc = *llocp;
  tbuffer->i    = 0;
  tbuffer->prev = prev;
  return tbuffer;
}

int dmnsn_yylex_impl(dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                     const char *filename, void *yyscanner);

static int
dmnsn_yylex_wrapper(dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
                    const char *filename, dmnsn_symbol_table *symtable,
                    void *yyscanner)
{
  dmnsn_token_buffer *tbuffer = dmnsn_yyget_extra(yyscanner);

  while (tbuffer && tbuffer->type != DMNSN_T_INCLUDE
         && tbuffer->i >= dmnsn_array_size(tbuffer->buffered))
  {
    if (tbuffer->type == DMNSN_T_WHILE) {
      tbuffer->i = 0;
    } else {
      if (dmnsn_array_size(tbuffer->buffered) == 0
          && tbuffer->prev && tbuffer->prev->type == DMNSN_T_WHILE)
      {
        dmnsn_yyset_extra(tbuffer->prev, yyscanner);
        dmnsn_delete_token_buffer(tbuffer);
        tbuffer = dmnsn_yyget_extra(yyscanner);
      }

      dmnsn_yyset_extra(tbuffer->prev, yyscanner);
      if (tbuffer->type == DMNSN_T_MACRO) {
        dmnsn_pop_scope(symtable);
      } else {
        dmnsn_delete_token_buffer(tbuffer);
      }
      tbuffer = dmnsn_yyget_extra(yyscanner);
    }
  }

  int token;

  if (tbuffer && tbuffer->type != DMNSN_T_INCLUDE) {
    /* Return buffered tokens */
    dmnsn_buffered_token buffered;

    dmnsn_array_get(tbuffer->buffered, tbuffer->i, &buffered);
    token = buffered.type;

    if (buffered.lval.value) {
      lvalp->value = dmnsn_strdup(buffered.lval.value);
    } else {
      lvalp->value = NULL;
    }

    *llocp = buffered.lloc;
    if (tbuffer->type == DMNSN_T_MACRO)
      llocp->parent = &tbuffer->lloc;
    ++tbuffer->i;
  } else {
    const char *real_filename = tbuffer ? tbuffer->filename : filename;
    token = dmnsn_yylex_impl(lvalp, llocp, real_filename, yyscanner);

    if (tbuffer && tbuffer->type == DMNSN_T_INCLUDE) {
      if (token == DMNSN_T_EOF) {
        dmnsn_yy_pop_buffer(yyscanner);
        fclose(tbuffer->ptr);
        dmnsn_pop_scope(symtable);
        dmnsn_yyset_extra(tbuffer->prev, yyscanner);
        dmnsn_delete_token_buffer(tbuffer);
        return dmnsn_yylex_wrapper(lvalp, llocp, filename, symtable,
                                   yyscanner);
      } else {
        llocp->parent = &tbuffer->lloc;
      }
    }
  }

  return token;
}

int
dmnsn_yylex(dmnsn_parse_item *lvalp, dmnsn_parse_location *llocp,
            const char *filename, dmnsn_symbol_table *symtable, void *yyscanner)
{
  /*
   * So... this is kind of ugly.  POV-Ray's language directives are not parsable
   * by any reasonable bison grammar, since some require skipping arbitrary
   * amounts of tokens, or repeatedly parsing tokens.  Instead, they are
   * implemented transparently by the lexer.  The lexing function calls a
   * separate parser to handle language directives as they arrise, and returns
   * only non-directive tokens.
   *
   * Ideally we'd use a push parser for the language directives, but bison
   * doesn't support GLR push parsers.  Instead, we buffer all the appropriate
   * tokens and call a pull parser, then discard the buffer and continue.
   */

  while (1) {
    int token = dmnsn_yylex_wrapper(
      lvalp, llocp, filename, symtable, yyscanner
    );
    dmnsn_token_buffer *tbuffer = dmnsn_yyget_extra(yyscanner);

    if (tbuffer && tbuffer->type == DMNSN_T_LEX_VERBATIM && tbuffer->i == 1) {
      return token;
    }

    switch (token) {
    case DMNSN_T_INCLUDE:
      {
        dmnsn_token_buffer *tb = dmnsn_include_buffer(
          token, tbuffer, lvalp, llocp, filename, symtable, yyscanner
        );
        if (!tb) {
          return DMNSN_T_LEX_ERROR;
        }

        dmnsn_yyset_extra(tb, yyscanner);
        break;
      }

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
    case DMNSN_T_IFDEF:
    case DMNSN_T_IFNDEF:
      {
        dmnsn_token_buffer *tb = dmnsn_if_buffer(
          token, tbuffer, lvalp, llocp, filename, symtable, yyscanner
        );
        if (!tb) {
          return DMNSN_T_LEX_ERROR;
        }

        dmnsn_yyset_extra(tb, yyscanner);
        break;
      }

    case DMNSN_T_WHILE:
      {
        dmnsn_token_buffer *tb = dmnsn_while_buffer(
          token, tbuffer, lvalp, llocp, filename, symtable, yyscanner
        );
        if (!tb) {
          return DMNSN_T_LEX_ERROR;
        }

        dmnsn_yyset_extra(tb, yyscanner);
        break;
      }

    case DMNSN_T_VERSION:
      {
        dmnsn_token_buffer *tb = dmnsn_version_buffer(
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

    case DMNSN_T_DEBUG:
    case DMNSN_T_ERROR:
    case DMNSN_T_WARNING:
      {
        dmnsn_token_buffer *tb = dmnsn_stream_buffer(
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

    case DMNSN_T_MACRO:
      {
        bool status = dmnsn_declare_macro(
          token, tbuffer, lvalp, llocp, filename, symtable, yyscanner
        );
        if (!status) {
          return DMNSN_T_LEX_ERROR;
        }
        break;
      }

    case DMNSN_T_IDENTIFIER:
      {
        dmnsn_astnode *symbol = dmnsn_find_symbol(symtable, lvalp->value);
        if (symbol && symbol->type == DMNSN_AST_MACRO) {
          dmnsn_token_buffer *tb = dmnsn_macro_buffer(
            token, symbol, tbuffer, lvalp, llocp, filename, symtable, yyscanner
          );
          if (!tb) {
            return DMNSN_T_LEX_ERROR;
          }

          dmnsn_yyset_extra(tb, yyscanner);
          break;
        } else {
          return token;
        }
      }

    default:
      return token;
    }
  }
}

void
dmnsn_yylex_cleanup(void *yyscanner)
{
  dmnsn_token_buffer *tbuffer = dmnsn_yyget_extra(yyscanner);
  while (tbuffer) {
    if (tbuffer->type == DMNSN_T_INCLUDE) {
      dmnsn_yy_pop_buffer(yyscanner);
      fclose(tbuffer->ptr);
    }

    dmnsn_token_buffer *prev = tbuffer->prev;
    if (tbuffer->type != DMNSN_T_MACRO)
      dmnsn_delete_token_buffer(tbuffer);
    tbuffer = prev;
  }
  dmnsn_yyset_extra(NULL, yyscanner);
}
