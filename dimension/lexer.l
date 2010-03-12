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

%option reentrant
%option stack
%option yylineno
%option noyywrap
%option never-interactive
%option prefix="dmnsn_yy"
%option outfile="lex.yy.c"

%{
#include "parse.h"
#include "tokenize.h"
#include "utility.h"
#include <stdlib.h>
#include <stdio.h>

#define YY_DECL int dmnsn_yylex_impl(dmnsn_parse_item *lvalp,           \
                                     dmnsn_parse_location *llocp,       \
                                     const char *filename,              \
                                     yyscan_t yyscanner)
%}

%x DMNSN_BLOCK_COMMENT
%x DMNSN_LINE_COMMENT
%x DMNSN_STRING
%x DMNSN_STRING_ESCAPE

%%

%{
/* Some helpful macros that set fields of a token correctly, and other stuff */

#define NEW_TOKEN(token_type)                                   \
  do {                                                          \
    token = token_type;                                         \
    lvalp->value = NULL;                                        \
    llocp->first_filename = llocp->last_filename = filename;    \
    llocp->first_line     = llocp->last_line     = yylineno;    \
    llocp->first_column   = llocp->last_column   = yycolumn;    \
  } while (0)

#define CALCULATE_COLUMN() do { yycolumn += yyleng; } while (0)

#define RETURN()                               \
  do {                                         \
    CALCULATE_COLUMN();                        \
    return token;                              \
  } while (0)

#define RETURN_TOKEN(token_type)                \
  do {                                          \
    NEW_TOKEN(token_type);                      \
    RETURN();                                   \
  } while (0)

#define RETURN_VALUE_TOKEN(token_type)                          \
  do {                                                          \
    NEW_TOKEN(token_type);                                      \
    lvalp->value = strdup(yytext);                              \
    if (!lvalp->value)                                          \
      dmnsn_error(DMNSN_SEVERITY_HIGH,                          \
                  "Couldn't allocate space for token value.");  \
    RETURN();                                                   \
  } while (0)

#define STRING_TOKEN()                                          \
  do {                                                          \
    NEW_TOKEN(DMNSN_T_STRING);                                  \
    lvalp->value    = malloc(string_extent);                    \
    if (!lvalp->value)                                          \
      dmnsn_error(DMNSN_SEVERITY_HIGH,                          \
                  "Couldn't allocate space for token value.");  \
    lvalp->value[0] = '\0';                                     \
    CALCULATE_COLUMN();                                         \
  } while (0)

#define STRCAT(str, len)                                                \
  do {                                                                  \
    if (string_length + len + 1 >= string_length) {                     \
      string_extent = 2*(string_length + len + 1);                      \
      lvalp->value = realloc(lvalp->value, string_extent);              \
    }                                                                   \
                                                                        \
    strncpy(lvalp->value + string_length, str, len + 1);                \
    string_length += len;                                               \
    CALCULATE_COLUMN();                                                 \
  } while(0)

int token;
size_t string_length = 0, string_extent = 8;
unsigned long wchar;

/* Silence some warnings */
(void)yyunput;
(void)input;
(void)yy_top_state;
%}

(?# Comments)

<INITIAL,DMNSN_BLOCK_COMMENT>"/*"       {
  yy_push_state(DMNSN_BLOCK_COMMENT, yyscanner);
  CALCULATE_COLUMN();
}
<DMNSN_BLOCK_COMMENT>"*/"       CALCULATE_COLUMN(); yy_pop_state(yyscanner);
<DMNSN_BLOCK_COMMENT>[^*/\n]*   CALCULATE_COLUMN();
<DMNSN_BLOCK_COMMENT>"/"        CALCULATE_COLUMN();
<DMNSN_BLOCK_COMMENT>"*"        CALCULATE_COLUMN();
<DMNSN_BLOCK_COMMENT>\n         ;

"//"    {
  yy_push_state(DMNSN_LINE_COMMENT, yyscanner);
  CALCULATE_COLUMN();
}
<DMNSN_LINE_COMMENT>\n          ; yy_pop_state(yyscanner);
<DMNSN_LINE_COMMENT>[^\n]+      CALCULATE_COLUMN();

(?# Punctuation)
"{"     RETURN_TOKEN(DMNSN_T_LBRACE);
"}"     RETURN_TOKEN(DMNSN_T_RBRACE);
"("     RETURN_TOKEN(DMNSN_T_LPAREN);
")"     RETURN_TOKEN(DMNSN_T_RPAREN);
"["     RETURN_TOKEN(DMNSN_T_LBRACKET);
"]"     RETURN_TOKEN(DMNSN_T_RBRACKET);
"+"     RETURN_TOKEN(DMNSN_T_PLUS);
"-"     RETURN_TOKEN(DMNSN_T_MINUS);
"*"     RETURN_TOKEN(DMNSN_T_STAR);
"/"     RETURN_TOKEN(DMNSN_T_SLASH);
","     RETURN_TOKEN(DMNSN_T_COMMA);
";"     RETURN_TOKEN(DMNSN_T_SEMICOLON);
"?"     RETURN_TOKEN(DMNSN_T_QUESTION);
":"     RETURN_TOKEN(DMNSN_T_COLON);
"&"     RETURN_TOKEN(DMNSN_T_AND);
"."     RETURN_TOKEN(DMNSN_T_DOT);
"|"     RETURN_TOKEN(DMNSN_T_PIPE);
"<"     RETURN_TOKEN(DMNSN_T_LESS);
">"     RETURN_TOKEN(DMNSN_T_GREATER);
"!"     RETURN_TOKEN(DMNSN_T_BANG);
"="     RETURN_TOKEN(DMNSN_T_EQUALS);
"<="    RETURN_TOKEN(DMNSN_T_LESS_EQUAL);
">="    RETURN_TOKEN(DMNSN_T_GREATER_EQUAL);
"!="    RETURN_TOKEN(DMNSN_T_NOT_EQUAL);

(?# Integers)
[[:digit:]]+                    |
0(x|X)[[:digit:]aAbBcCdDeEfF]+  RETURN_VALUE_TOKEN(DMNSN_T_INTEGER);

(?# Floats)
[[:digit:]]*\.?[[:digit:]]+((e|E)(\+|-)?[[:digit:]]+)?  {
  RETURN_VALUE_TOKEN(DMNSN_T_FLOAT);
}

(?# Keywords)
"ambient"               RETURN_TOKEN(DMNSN_T_AMBIENT);
"angle"                 RETURN_TOKEN(DMNSN_T_ANGLE);
"assumed_gamma"         RETURN_TOKEN(DMNSN_T_ASSUMED_GAMMA);
"background"            RETURN_TOKEN(DMNSN_T_BACKGROUND);
"box"                   RETURN_TOKEN(DMNSN_T_BOX);
"blue"                  RETURN_TOKEN(DMNSN_T_BLUE);
"camera"                RETURN_TOKEN(DMNSN_T_CAMERA);
"color"                 RETURN_TOKEN(DMNSN_T_COLOR);
"colour"                RETURN_TOKEN(DMNSN_T_COLOR);
"direction"             RETURN_TOKEN(DMNSN_T_DIRECTION);
"diffuse"               RETURN_TOKEN(DMNSN_T_DIFFUSE);
"falloff"               RETURN_TOKEN(DMNSN_T_FALLOFF);
"filter"                RETURN_TOKEN(DMNSN_T_FILTER);
"finish"                RETURN_TOKEN(DMNSN_T_FINISH);
"global_settings"       RETURN_TOKEN(DMNSN_T_GLOBAL_SETTINGS);
"gray"                  RETURN_TOKEN(DMNSN_T_GRAY);
"grey"                  RETURN_TOKEN(DMNSN_T_GRAY);
"green"                 RETURN_TOKEN(DMNSN_T_GREEN);
"interior"              RETURN_TOKEN(DMNSN_T_INTERIOR);
"ior"                   RETURN_TOKEN(DMNSN_T_IOR);
"location"              RETURN_TOKEN(DMNSN_T_LOCATION);
"look_at"               RETURN_TOKEN(DMNSN_T_LOOK_AT);
"light_source"          RETURN_TOKEN(DMNSN_T_LIGHT_SOURCE);
"max_trace_level"       RETURN_TOKEN(DMNSN_T_MAX_TRACE_LEVEL);
"perspective"           RETURN_TOKEN(DMNSN_T_PERSPECTIVE);
"phong"                 RETURN_TOKEN(DMNSN_T_PHONG);
"phong_size"            RETURN_TOKEN(DMNSN_T_PHONG_SIZE);
"pigment"               RETURN_TOKEN(DMNSN_T_PIGMENT);
"red"                   RETURN_TOKEN(DMNSN_T_RED);
"reflection"            RETURN_TOKEN(DMNSN_T_REFLECTION);
"rgb"                   RETURN_TOKEN(DMNSN_T_RGB);
"rgbf"                  RETURN_TOKEN(DMNSN_T_RGBF);
"rgbft"                 RETURN_TOKEN(DMNSN_T_RGBFT);
"rgbt"                  RETURN_TOKEN(DMNSN_T_RGBT);
"right"                 RETURN_TOKEN(DMNSN_T_RIGHT);
"rotate"                RETURN_TOKEN(DMNSN_T_ROTATE);
"sphere"                RETURN_TOKEN(DMNSN_T_SPHERE);
"sky"                   RETURN_TOKEN(DMNSN_T_SKY);
"t"                     RETURN_TOKEN(DMNSN_T_T);
"texture"               RETURN_TOKEN(DMNSN_T_TEXTURE);
"transmit"              RETURN_TOKEN(DMNSN_T_TRANSMIT);
"u"                     RETURN_TOKEN(DMNSN_T_U);
"up"                    RETURN_TOKEN(DMNSN_T_UP);
"v"                     RETURN_TOKEN(DMNSN_T_V);
"x"                     RETURN_TOKEN(DMNSN_T_X);
"y"                     RETURN_TOKEN(DMNSN_T_Y);
"z"                     RETURN_TOKEN(DMNSN_T_Z);

(?# Directives)
#[\b\r\t\v ]*break              RETURN_TOKEN(DMNSN_T_BREAK);
#[\b\r\t\v ]*case               RETURN_TOKEN(DMNSN_T_CASE);
#[\b\r\t\v ]*debug              RETURN_TOKEN(DMNSN_T_DEBUG);
#[\b\r\t\v ]*declare            RETURN_TOKEN(DMNSN_T_DECLARE);
#[\b\r\t\v ]*default            RETURN_TOKEN(DMNSN_T_DEFAULT);
#[\b\r\t\v ]*else               RETURN_TOKEN(DMNSN_T_ELSE);
#[\b\r\t\v ]*end                RETURN_TOKEN(DMNSN_T_END);
#[\b\r\t\v ]*error              RETURN_TOKEN(DMNSN_T_ERROR);
#[\b\r\t\v ]*fclose             RETURN_TOKEN(DMNSN_T_FCLOSE);
#[\b\r\t\v ]*fopen              RETURN_TOKEN(DMNSN_T_FOPEN);
#[\b\r\t\v ]*if                 RETURN_TOKEN(DMNSN_T_IF);
#[\b\r\t\v ]*ifdef              RETURN_TOKEN(DMNSN_T_IFDEF);
#[\b\r\t\v ]*ifndef             RETURN_TOKEN(DMNSN_T_IFNDEF);
#[\b\r\t\v ]*include            RETURN_TOKEN(DMNSN_T_INCLUDE);
#[\b\r\t\v ]*local              RETURN_TOKEN(DMNSN_T_LOCAL);
#[\b\r\t\v ]*macro              RETURN_TOKEN(DMNSN_T_MACRO);
#[\b\r\t\v ]*range              RETURN_TOKEN(DMNSN_T_RANGE);
#[\b\r\t\v ]*read               RETURN_TOKEN(DMNSN_T_READ);
#[\b\r\t\v ]*render             RETURN_TOKEN(DMNSN_T_RENDER);
#[\b\r\t\v ]*statistics         RETURN_TOKEN(DMNSN_T_STATISTICS);
#[\b\r\t\v ]*switch             RETURN_TOKEN(DMNSN_T_SWITCH);
#[\b\r\t\v ]*undef              RETURN_TOKEN(DMNSN_T_UNDEF);
#[\b\r\t\v ]*version            RETURN_TOKEN(DMNSN_T_VERSION);
#[\b\r\t\v ]*warning            RETURN_TOKEN(DMNSN_T_WARNING);
#[\b\r\t\v ]*while              RETURN_TOKEN(DMNSN_T_WHILE);
#[\b\r\t\v ]*write              RETURN_TOKEN(DMNSN_T_WRITE);
#[\b\r\t\v ]*[[:alnum:]_]*      {
  dmnsn_diagnostic(filename, yylineno, yycolumn,
                   "Unrecognized language directive '%s'", yytext);
  RETURN_TOKEN(DMNSN_T_LEX_ERROR);
}

(?# Identifiers)
[[:alpha:]][[:alnum:]_]*        RETURN_VALUE_TOKEN(DMNSN_T_IDENTIFIER);

(?# Strings)

"\""    STRING_TOKEN(); yy_push_state(DMNSN_STRING, yyscanner);
<DMNSN_STRING>[^\\\"\n]*        STRCAT(yytext, yyleng);
<DMNSN_STRING>"\""              yy_pop_state(yyscanner); RETURN();

(?# String escape sequences)

<DMNSN_STRING>"\\"      {
  yy_push_state(DMNSN_STRING_ESCAPE, yyscanner);
  CALCULATE_COLUMN();
}
<DMNSN_STRING_ESCAPE>"a"        STRCAT("\a", 1); yy_pop_state(yyscanner);
<DMNSN_STRING_ESCAPE>"b"        STRCAT("\b", 1); yy_pop_state(yyscanner);
<DMNSN_STRING_ESCAPE>"f"        STRCAT("\f", 1); yy_pop_state(yyscanner);
<DMNSN_STRING_ESCAPE>"n"        STRCAT("\n", 1); yy_pop_state(yyscanner);
<DMNSN_STRING_ESCAPE>"r"        STRCAT("\r", 1); yy_pop_state(yyscanner);
<DMNSN_STRING_ESCAPE>"t"        STRCAT("\t", 1); yy_pop_state(yyscanner);
<DMNSN_STRING_ESCAPE>"v"        STRCAT("\v", 1); yy_pop_state(yyscanner);
<DMNSN_STRING_ESCAPE>"\\"       STRCAT("\\", 1); yy_pop_state(yyscanner);
<DMNSN_STRING_ESCAPE>"'"        STRCAT("'", 1); yy_pop_state(yyscanner);
<DMNSN_STRING_ESCAPE>"\""       STRCAT("\"", 1); yy_pop_state(yyscanner);
<DMNSN_STRING_ESCAPE>"u"[[:digit:]aAbBcCdDeEfF]{4}      {
  wchar = strtoul(yytext + 1, NULL, 16);
  STRCAT("", 2);
  lvalp->value[string_length - 2] = wchar/256;
  lvalp->value[string_length - 1] = wchar%256;
  yy_pop_state(yyscanner);
}
<DMNSN_STRING_ESCAPE>.          {
  dmnsn_diagnostic(filename, yylineno, yycolumn,
                   "WARNING: unrecognised escape sequence '\\%c'",
                   (int)*yytext);
  STRCAT(yytext, yyleng);
  yy_pop_state(yyscanner);
}

(?# Ignore whitespace)
[\b\r\t\v ]+    CALCULATE_COLUMN();
\n              ;

(?# Fall-through)
.       {
  dmnsn_diagnostic(filename, yylineno, yycolumn,
                   "Unrecognized character '%c' (0x%X)",
                   (int)*yytext, (unsigned int)(unsigned char)*yytext);
  RETURN_TOKEN(DMNSN_T_LEX_ERROR);
}

%%

void *
dmnsn_yy_make_buffer(FILE *file, void *scanner)
{
  return dmnsn_yy_create_buffer(file, YY_BUF_SIZE, scanner);
}

void *
dmnsn_yy_make_string_buffer(const char *str, void *scanner)
{
  return dmnsn_yy_scan_string(str, scanner);
}

void
dmnsn_yy_push_buffer(void *buffer, void *scanner)
{
  dmnsn_yypush_buffer_state(buffer, scanner);
}

void
dmnsn_yy_pop_buffer(void *scanner)
{
  dmnsn_yypop_buffer_state(scanner);
}

dmnsn_array *
dmnsn_tokenize(FILE *file, const char *filename)
{
  dmnsn_token token;
  dmnsn_parse_item item;
  dmnsn_parse_location location;
  dmnsn_array *tokens = dmnsn_new_array(sizeof(dmnsn_token));

  yyscan_t scanner;

  yylex_init(&scanner);
  yyset_in(file, scanner);
  yyset_extra(NULL, scanner);

  while ((token.type = dmnsn_yylex_impl(&item, &location, filename, scanner))
         != 0) {
    token.value    = item.value;
    token.filename = location.first_filename;
    token.line     = location.first_line;
    token.col      = location.first_column;
    dmnsn_array_push(tokens, &token);
  }

  yylex_destroy(scanner);
  return tokens;
}

static void
dmnsn_delete_token(dmnsn_token token)
{
  free(token.value);
}

void
dmnsn_delete_tokens(dmnsn_array *tokens)
{
  dmnsn_token token;
  unsigned int i;
  for (i = 0; i < dmnsn_array_size(tokens); ++i) {
    dmnsn_array_get(tokens, i, &token);
    dmnsn_delete_token(token);
  }
  dmnsn_delete_array(tokens);
}

static void
dmnsn_print_token(FILE *file, dmnsn_token token)
{
  const char *tname;
  if (token.type == DMNSN_T_LPAREN) {
    tname = "\\(";
  } else if (token.type == DMNSN_T_RPAREN) {
    tname = "\\)";
  } else if (token.type == DMNSN_T_LEX_ERROR) {
    tname = "(error)";
  } else {
    tname = dmnsn_token_string(token.type);
  }

  if (token.value) {
    fprintf(file, "(%s \"%s\")", tname, token.value);
  } else {
    fprintf(file, "%s", tname);
  }
}

void
dmnsn_print_token_sexpr(FILE *file, const dmnsn_array *tokens)
{
  dmnsn_token token;
  unsigned int i;

  if (dmnsn_array_size(tokens) == 0) {
    fprintf(file, "()");
  } else {
    fprintf(file, "(");
    dmnsn_array_get(tokens, 0, &token);
    dmnsn_print_token(file, token);

    for (i = 1; i < dmnsn_array_size(tokens); ++i) {
      fprintf(file, " ");
      dmnsn_array_get(tokens, i, &token);
      dmnsn_print_token(file, token);
    }

    fprintf(file, ")");
  }

  fprintf(file, "\n");
}
