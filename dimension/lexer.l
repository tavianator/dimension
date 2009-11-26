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

%option reentrant stack yylineno noyywrap prefix="dmnsn_yy" outfile="lex.yy.c"

%{
#define YY_DECL static int yylex(const char *filename, dmnsn_array *tokens, \
                                 yyscan_t yyscanner)
#include "tokenize.h"
#include "utility.h"
#include <stdlib.h>
#include <stdio.h>
%}

%x DMNSN_BLOCK_COMMENT
%x DMNSN_LINE_COMMENT
%x DMNSN_STRING
%x DMNSN_STRING_ESCAPE

%%

%{
/* Some helpful macros that set fields of a token correctly, and other stuff */

#define NEW_TOKEN(token_type)                   \
  do {                                          \
    token.type     = token_type;                \
    token.filename = filename;                  \
    token.line     = yylineno;                  \
    token.col      = yycolumn;                  \
    token.value    = NULL;                      \
  } while (0)

#define CALCULATE_COLUMN() yycolumn += yyleng

#define PUSH()                                  \
  do {                                          \
    dmnsn_array_push(tokens, &token);           \
    CALCULATE_COLUMN();                         \
  } while (0)

#define PUSH_TOKEN(token_type)                  \
  do {                                          \
    NEW_TOKEN(token_type);                      \
    PUSH();                                     \
  } while (0)

#define PUSH_VALUE_TOKEN(token_type)            \
  do {                                          \
    NEW_TOKEN(token_type);                      \
    token.value    = strdup(yytext);            \
    PUSH();                                     \
  } while (0)

#define STRING_TOKEN()                          \
  do {                                          \
    NEW_TOKEN(DMNSN_T_STRING);                  \
    string_length  = 0;                         \
    string_extent  = 8;                         \
    token.value    = malloc(string_extent);     \
    token.value[0] = '\0';                      \
    CALCULATE_COLUMN();                         \
  } while (0)

#define STRCAT(str, len)                                                \
  do {                                                                  \
    if (string_length + len + 1 >= string_length) {                     \
      string_extent = 2*(string_length + len + 1);                      \
      token.value = realloc(token.value, string_extent);                \
    }                                                                   \
                                                                        \
    strncpy(token.value + string_length, str, len + 1);                 \
    string_length += len;                                               \
    CALCULATE_COLUMN();                                                 \
  } while(0)

dmnsn_token token;
size_t string_length, string_extent;
unsigned long wchar;
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
"{"     PUSH_TOKEN(DMNSN_T_LBRACE);
"}"     PUSH_TOKEN(DMNSN_T_RBRACE);
"("     PUSH_TOKEN(DMNSN_T_LPAREN);
")"     PUSH_TOKEN(DMNSN_T_RPAREN);
"["     PUSH_TOKEN(DMNSN_T_LBRACKET);
"]"     PUSH_TOKEN(DMNSN_T_RBRACKET);
"+"     PUSH_TOKEN(DMNSN_T_PLUS);
"-"     PUSH_TOKEN(DMNSN_T_MINUS);
"*"     PUSH_TOKEN(DMNSN_T_STAR);
"/"     PUSH_TOKEN(DMNSN_T_SLASH);
","     PUSH_TOKEN(DMNSN_T_COMMA);
";"     PUSH_TOKEN(DMNSN_T_SEMICOLON);
"?"     PUSH_TOKEN(DMNSN_T_QUESTION);
":"     PUSH_TOKEN(DMNSN_T_COLON);
"&"     PUSH_TOKEN(DMNSN_T_AND);
"."     PUSH_TOKEN(DMNSN_T_DOT);
"|"     PUSH_TOKEN(DMNSN_T_PIPE);
"<"     PUSH_TOKEN(DMNSN_T_LESS);
">"     PUSH_TOKEN(DMNSN_T_GREATER);
"!"     PUSH_TOKEN(DMNSN_T_BANG);
"="     PUSH_TOKEN(DMNSN_T_EQUALS);
"<="    PUSH_TOKEN(DMNSN_T_LESS_EQUAL);
">="    PUSH_TOKEN(DMNSN_T_GREATER_EQUAL);
"!="    PUSH_TOKEN(DMNSN_T_NOT_EQUAL);

(?# Integers)
[[:digit:]]+                    |
0(x|X)[[:digit:]aAbBcCdDeEfF]+  PUSH_VALUE_TOKEN(DMNSN_T_INTEGER);

(?# Floats)
[[:digit:]]*\.?[[:digit:]]+((e|E)(\+|-)?[[:digit:]]+)?  {
  PUSH_VALUE_TOKEN(DMNSN_T_FLOAT);
}

(?# Keywords)
"angle"         PUSH_TOKEN(DMNSN_T_ANGLE);
"background"    PUSH_TOKEN(DMNSN_T_BACKGROUND);
"box"           PUSH_TOKEN(DMNSN_T_BOX);
"blue"          PUSH_TOKEN(DMNSN_T_BLUE);
"camera"        PUSH_TOKEN(DMNSN_T_CAMERA);
"color"         PUSH_TOKEN(DMNSN_T_COLOR);
"colour"        PUSH_TOKEN(DMNSN_T_COLOR);
"direction"     PUSH_TOKEN(DMNSN_T_DIRECTION);
"filter"        PUSH_TOKEN(DMNSN_T_FILTER);
"gray"          PUSH_TOKEN(DMNSN_T_GRAY);
"grey"          PUSH_TOKEN(DMNSN_T_GRAY);
"green"         PUSH_TOKEN(DMNSN_T_GREEN);
"location"      PUSH_TOKEN(DMNSN_T_LOCATION);
"look_at"       PUSH_TOKEN(DMNSN_T_LOOK_AT);
"light_source"  PUSH_TOKEN(DMNSN_T_LIGHT_SOURCE);
"perspective"   PUSH_TOKEN(DMNSN_T_PERSPECTIVE);
"pigment"       PUSH_TOKEN(DMNSN_T_PIGMENT);
"red"           PUSH_TOKEN(DMNSN_T_RED);
"rgb"           PUSH_TOKEN(DMNSN_T_RGB);
"rgbf"          PUSH_TOKEN(DMNSN_T_RGBF);
"rgbft"         PUSH_TOKEN(DMNSN_T_RGBFT);
"rgbt"          PUSH_TOKEN(DMNSN_T_RGBT);
"right"         PUSH_TOKEN(DMNSN_T_RIGHT);
"rotate"        PUSH_TOKEN(DMNSN_T_ROTATE);
"sphere"        PUSH_TOKEN(DMNSN_T_SPHERE);
"sky"           PUSH_TOKEN(DMNSN_T_SKY);
"t"             PUSH_TOKEN(DMNSN_T_T);
"texture"       PUSH_TOKEN(DMNSN_T_TEXTURE);
"transmit"      PUSH_TOKEN(DMNSN_T_TRANSMIT);
"u"             PUSH_TOKEN(DMNSN_T_U);
"up"            PUSH_TOKEN(DMNSN_T_UP);
"v"             PUSH_TOKEN(DMNSN_T_V);
"x"             PUSH_TOKEN(DMNSN_T_X);
"y"             PUSH_TOKEN(DMNSN_T_Y);
"z"             PUSH_TOKEN(DMNSN_T_Z);

(?# Directives)
"#include"      PUSH_TOKEN(DMNSN_T_INCLUDE);
"#declare"      PUSH_TOKEN(DMNSN_T_DECLARE);

(?# Identifiers)
[[:alpha:]][[:alnum:]_]*        PUSH_VALUE_TOKEN(DMNSN_T_IDENTIFIER);

(?# Strings)

"\""    STRING_TOKEN(); yy_push_state(DMNSN_STRING, yyscanner);
<DMNSN_STRING>[^\\\"\n]*        STRCAT(yytext, yyleng);
<DMNSN_STRING>"\""              PUSH(); yy_pop_state(yyscanner);

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
  token.value[string_length - 2] = wchar/256;
  token.value[string_length - 1] = wchar%256;
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
                   (int)*yytext, (unsigned int)*yytext);
  return 1;
}

%%

dmnsn_array *
dmnsn_tokenize(const char *filename, FILE *file)
{
  dmnsn_array *tokens = dmnsn_new_array(sizeof(dmnsn_token));

  yyscan_t scanner;

  yylex_init(&scanner);
  yyset_in(file, scanner);

  if (yylex(filename, tokens, scanner) != 0) {
    dmnsn_delete_tokens(tokens);
    tokens = NULL;
  }

  yylex_destroy(scanner);

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

static void
dmnsn_print_token(FILE *file, dmnsn_token token)
{
  const char *tname;
  if (token.type == DMNSN_T_LPAREN) {
    tname = "\\(";
  } else if (token.type == DMNSN_T_RPAREN) {
    tname = "\\)";
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
