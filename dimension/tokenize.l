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

%option reentrant yylineno noyywrap

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
#define PUSH_TOKEN(token_type)                  \
  do {                                          \
    token.type     = token_type;                \
    token.filename = filename;                  \
    token.line     = yylineno;                  \
    token.col      = yycolumn;                  \
    token.value    = NULL;                      \
    PUSH();                                    \
  } while (0)

#define PUSH_VALUE_TOKEN(token_type)            \
  do {                                          \
    token.type     = token_type;                \
    token.filename = filename;                  \
    token.line     = yylineno;                  \
    token.col      = yycolumn;                  \
    token.value    = strdup(yytext);            \
    PUSH();                                     \
  } while (0)

#define STRING_TOKEN()                          \
  do {                                          \
    token.type     = DMNSN_T_STRING;            \
    token.filename = filename;                  \
    token.line     = yylineno;                  \
    token.col      = yycolumn;                  \
    token.value    = malloc(1);                 \
    token.value[0] = '\0';                      \
    string_length  = 0;                         \
  } while (0)

#define STRCAT(str, len)                                                \
  do {                                                                  \
    token.value = realloc(token.value, string_length + len + 1);        \
    strncpy(token.value + string_length, str, len);                     \
    string_length += len;                                               \
  } while(0)

#define PUSH() dmnsn_array_push(tokens, &token)

dmnsn_token token;
size_t string_length;
unsigned long wchar;
%}

"/*"    BEGIN(DMNSN_BLOCK_COMMENT);
"//"    BEGIN(DMNSN_LINE_COMMENT);

<DMNSN_BLOCK_COMMENT>"*/"       BEGIN(INITIAL);
<DMNSN_BLOCK_COMMENT>[^*\n]*    ;
<DMNSN_BLOCK_COMMENT>"*"        ;
<DMNSN_BLOCK_COMMENT>\n         ;

<DMNSN_LINE_COMMENT>\n          BEGIN(INITIAL);
<DMNSN_LINE_COMMENT>[^\n]+      ;

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
"box"           PUSH_TOKEN(DMNSN_T_BOX);
"camera"        PUSH_TOKEN(DMNSN_T_CAMERA);
"color"         PUSH_TOKEN(DMNSN_T_COLOR);
"colour"        PUSH_TOKEN(DMNSN_T_COLOR);
"sphere"        PUSH_TOKEN(DMNSN_T_SPHERE);

(?# Directives)
"#include"      PUSH_TOKEN(DMNSN_T_INCLUDE);
"#declare"      PUSH_TOKEN(DMNSN_T_DECLARE);

(?# Identifiers)
[[:alpha:]][[:alnum:]_]*        PUSH_VALUE_TOKEN(DMNSN_T_IDENTIFIER);

(?# Strings)

"\""    STRING_TOKEN(); BEGIN(DMNSN_STRING);
<DMNSN_STRING>[^\\\"\n]*        STRCAT(yytext, yyleng);
<DMNSN_STRING>"\""              PUSH(); BEGIN(INITIAL);

(?# String escape sequences)

<DMNSN_STRING>"\\"              BEGIN(DMNSN_STRING_ESCAPE);
<DMNSN_STRING_ESCAPE>"a"        STRCAT("\a", 1); BEGIN(DMNSN_STRING);
<DMNSN_STRING_ESCAPE>"b"        STRCAT("\b", 1); BEGIN(DMNSN_STRING);
<DMNSN_STRING_ESCAPE>"f"        STRCAT("\f", 1); BEGIN(DMNSN_STRING);
<DMNSN_STRING_ESCAPE>"n"        STRCAT("\n", 1); BEGIN(DMNSN_STRING);
<DMNSN_STRING_ESCAPE>"r"        STRCAT("\r", 1); BEGIN(DMNSN_STRING);
<DMNSN_STRING_ESCAPE>"t"        STRCAT("\t", 1); BEGIN(DMNSN_STRING);
<DMNSN_STRING_ESCAPE>"v"        STRCAT("\v", 1); BEGIN(DMNSN_STRING);
<DMNSN_STRING_ESCAPE>"\\"       STRCAT("\\", 1); BEGIN(DMNSN_STRING);
<DMNSN_STRING_ESCAPE>"'"        STRCAT("'", 1); BEGIN(DMNSN_STRING);
<DMNSN_STRING_ESCAPE>"\""       STRCAT("\"", 1); BEGIN(DMNSN_STRING);
<DMNSN_STRING_ESCAPE>"u"[[:digit:]aAbBcCdDeEfF]{4}      {
  wchar = strtoul(yytext + 1, NULL, 16);
  STRCAT("", 2);
  token.value[string_length - 2] = wchar/256;
  token.value[string_length - 1] = wchar%256;
  BEGIN(DMNSN_STRING);
}
<DMNSN_STRING_ESCAPE>.          {
  dmnsn_diagnostic(filename, yylineno, yycolumn,
                   "WARNING: unrecognised escape sequence '\\%c'",
                   (int)*yytext);
  STRCAT(yytext, yyleng);
  BEGIN(DMNSN_STRING);
}

(?# Ignore whitespace)
[[:space:]]+    ;

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

const char *
dmnsn_token_string(dmnsn_token_type token_type)
{
  switch (token_type) {
  /* Macro to shorten this huge switch */
#define dmnsn_token_map(type, str)                                             \
  case type:                                                                   \
    return str;

  /* Punctuation */

  dmnsn_token_map(DMNSN_T_LBRACE,    "{");
  dmnsn_token_map(DMNSN_T_RBRACE,    "}")
  dmnsn_token_map(DMNSN_T_LPAREN,    "(");
  dmnsn_token_map(DMNSN_T_RPAREN,    ")");
  dmnsn_token_map(DMNSN_T_LBRACKET,  "[");
  dmnsn_token_map(DMNSN_T_RBRACKET,  "]");
  dmnsn_token_map(DMNSN_T_PLUS,      "+");
  dmnsn_token_map(DMNSN_T_MINUS,     "-");
  dmnsn_token_map(DMNSN_T_STAR,      "*");
  dmnsn_token_map(DMNSN_T_SLASH,     "/");
  dmnsn_token_map(DMNSN_T_COMMA,     ",");
  dmnsn_token_map(DMNSN_T_SEMICOLON, ";");
  dmnsn_token_map(DMNSN_T_QUESTION,  "?");
  dmnsn_token_map(DMNSN_T_COLON,     ":");
  dmnsn_token_map(DMNSN_T_AND,       "&");
  dmnsn_token_map(DMNSN_T_DOT,       ".");
  dmnsn_token_map(DMNSN_T_PIPE,      "|");
  dmnsn_token_map(DMNSN_T_LESS,      "<");
  dmnsn_token_map(DMNSN_T_GREATER,   ">");
  dmnsn_token_map(DMNSN_T_BANG,      "!");
  dmnsn_token_map(DMNSN_T_EQUALS,    "=");

  dmnsn_token_map(DMNSN_T_LESS_EQUAL,    "<=");
  dmnsn_token_map(DMNSN_T_GREATER_EQUAL, ">=");
  dmnsn_token_map(DMNSN_T_NOT_EQUAL,     "!=");

  /* Numeric values */
  dmnsn_token_map(DMNSN_T_INTEGER, "integer");
  dmnsn_token_map(DMNSN_T_FLOAT,   "float");

  /* Keywords */
  dmnsn_token_map(DMNSN_T_AA_LEVEL, "aa_level");
  dmnsn_token_map(DMNSN_T_AA_THRESHOLD, "aa_threshold");
  dmnsn_token_map(DMNSN_T_ABS, "abs");
  dmnsn_token_map(DMNSN_T_ABSORPTION, "absorption");
  dmnsn_token_map(DMNSN_T_ACCURACY, "accuracy");
  dmnsn_token_map(DMNSN_T_ACOS, "acos");
  dmnsn_token_map(DMNSN_T_ACOSH, "acosh");
  dmnsn_token_map(DMNSN_T_ADAPTIVE, "adaptive");
  dmnsn_token_map(DMNSN_T_ADC_BAILOUT, "adc_bailout");
  dmnsn_token_map(DMNSN_T_AGATE, "agate");
  dmnsn_token_map(DMNSN_T_AGATE_TURB, "agate_turb");
  dmnsn_token_map(DMNSN_T_ALL, "all");
  dmnsn_token_map(DMNSN_T_ALL_INTERSECTIONS, "all_intersections");
  dmnsn_token_map(DMNSN_T_ALPHA, "alpha");
  dmnsn_token_map(DMNSN_T_ALTITUDE, "altitude");
  dmnsn_token_map(DMNSN_T_ALWAYS_SAMPLE, "always_sample");
  dmnsn_token_map(DMNSN_T_AMBIENT, "ambient");
  dmnsn_token_map(DMNSN_T_AMBIENT_LIGHT, "ambient_light");
  dmnsn_token_map(DMNSN_T_ANGLE, "angle");
  dmnsn_token_map(DMNSN_T_APERTURE, "aperture");
  dmnsn_token_map(DMNSN_T_APPEND, "append");
  dmnsn_token_map(DMNSN_T_ARC_ANGLE, "arc_angle");
  dmnsn_token_map(DMNSN_T_AREA_LIGHT, "area_light");
  dmnsn_token_map(DMNSN_T_ARRAY, "array");
  dmnsn_token_map(DMNSN_T_ASC, "asc");
  dmnsn_token_map(DMNSN_T_ASCII, "ascii");
  dmnsn_token_map(DMNSN_T_ASIN, "asin");
  dmnsn_token_map(DMNSN_T_ASINH, "asinh");
  dmnsn_token_map(DMNSN_T_ASSUMED_GAMMA, "assumed_gamma");
  dmnsn_token_map(DMNSN_T_ATAN, "atan");
  dmnsn_token_map(DMNSN_T_ATAN2, "atan2");
  dmnsn_token_map(DMNSN_T_ATANH, "atanh");
  dmnsn_token_map(DMNSN_T_AUTOSTOP, "autostop");
  dmnsn_token_map(DMNSN_T_AVERAGE, "average");
  dmnsn_token_map(DMNSN_T_B_SPLINE, "b_spline");
  dmnsn_token_map(DMNSN_T_BACKGROUND, "background");
  dmnsn_token_map(DMNSN_T_BEZIER_SPLINE, "bezier_spline");
  dmnsn_token_map(DMNSN_T_BICUBIC_PATCH, "bicubic_patch");
  dmnsn_token_map(DMNSN_T_BLACK_HOLE, "black_hole");
  dmnsn_token_map(DMNSN_T_BLOB, "blob");
  dmnsn_token_map(DMNSN_T_BLUE, "blue");
  dmnsn_token_map(DMNSN_T_BLUR_SAMPLES, "blur_samples");
  dmnsn_token_map(DMNSN_T_BOUNDED_BY, "bounded_by");
  dmnsn_token_map(DMNSN_T_BOX, "box");
  dmnsn_token_map(DMNSN_T_BOXED, "boxed");
  dmnsn_token_map(DMNSN_T_BOZO, "bozo");
  dmnsn_token_map(DMNSN_T_BRICK, "brick");
  dmnsn_token_map(DMNSN_T_BRICK_SIZE, "brick_size");
  dmnsn_token_map(DMNSN_T_BRIGHTNESS, "brightness");
  dmnsn_token_map(DMNSN_T_BRILLIANCE, "brilliance");
  dmnsn_token_map(DMNSN_T_BUMP_MAP, "bump_map");
  dmnsn_token_map(DMNSN_T_BUMP_SIZE, "bump_size");
  dmnsn_token_map(DMNSN_T_BUMPS, "bumps");
  dmnsn_token_map(DMNSN_T_CAMERA, "camera");
  dmnsn_token_map(DMNSN_T_CAUSTICS, "caustics");
  dmnsn_token_map(DMNSN_T_CEIL, "ceil");
  dmnsn_token_map(DMNSN_T_CELLS, "cells");
  dmnsn_token_map(DMNSN_T_CHARSET, "charset");
  dmnsn_token_map(DMNSN_T_CHECKER, "checker");
  dmnsn_token_map(DMNSN_T_CHR, "chr");
  dmnsn_token_map(DMNSN_T_CIRCULAR, "circular");
  dmnsn_token_map(DMNSN_T_CLIPPED_BY, "clipped_by");
  dmnsn_token_map(DMNSN_T_CLOCK, "clock");
  dmnsn_token_map(DMNSN_T_CLOCK_DELTA, "clock_delta");
  dmnsn_token_map(DMNSN_T_CLOCK_ON, "clock_on");
  dmnsn_token_map(DMNSN_T_COLLECT, "collect");
  dmnsn_token_map(DMNSN_T_COLOR, "color");
  dmnsn_token_map(DMNSN_T_COLOR_MAP, "color_map");
  dmnsn_token_map(DMNSN_T_COMPONENT, "component");
  dmnsn_token_map(DMNSN_T_COMPOSITE, "composite");
  dmnsn_token_map(DMNSN_T_CONCAT, "concat");
  dmnsn_token_map(DMNSN_T_CONE, "cone");
  dmnsn_token_map(DMNSN_T_CONFIDENCE, "confidence");
  dmnsn_token_map(DMNSN_T_CONIC_SWEEP, "conic_sweep");
  dmnsn_token_map(DMNSN_T_CONSERVE_ENERGY, "conserve_energy");
  dmnsn_token_map(DMNSN_T_CONTAINED_BY, "contained_by");
  dmnsn_token_map(DMNSN_T_CONTROL0, "control0");
  dmnsn_token_map(DMNSN_T_CONTROL1, "control1");
  dmnsn_token_map(DMNSN_T_COORDS, "coords");
  dmnsn_token_map(DMNSN_T_COS, "cos");
  dmnsn_token_map(DMNSN_T_COSH, "cosh");
  dmnsn_token_map(DMNSN_T_COUNT, "count");
  dmnsn_token_map(DMNSN_T_CRACKLE, "crackle");
  dmnsn_token_map(DMNSN_T_CRAND, "crand");
  dmnsn_token_map(DMNSN_T_CUBE, "cube");
  dmnsn_token_map(DMNSN_T_CUBIC, "cubic");
  dmnsn_token_map(DMNSN_T_CUBIC_SPLINE, "cubic_spline");
  dmnsn_token_map(DMNSN_T_CUBIC_WAVE, "cubic_wave");
  dmnsn_token_map(DMNSN_T_CUTAWAY_TEXTURES, "cutaway_textures");
  dmnsn_token_map(DMNSN_T_CYLINDER, "cylinder");
  dmnsn_token_map(DMNSN_T_CYLINDRICAL, "cylindrical");
  dmnsn_token_map(DMNSN_T_DEFINED, "defined");
  dmnsn_token_map(DMNSN_T_DEGREES, "degrees");
  dmnsn_token_map(DMNSN_T_DENSITY, "density");
  dmnsn_token_map(DMNSN_T_DENSITY_FILE, "density_file");
  dmnsn_token_map(DMNSN_T_DENSITY_MAP, "density_map");
  dmnsn_token_map(DMNSN_T_DENTS, "dents");
  dmnsn_token_map(DMNSN_T_DF3, "df3");
  dmnsn_token_map(DMNSN_T_DIFFERENCE, "difference");
  dmnsn_token_map(DMNSN_T_DIFFUSE, "diffuse");
  dmnsn_token_map(DMNSN_T_DIMENSION_SIZE, "dimension_size");
  dmnsn_token_map(DMNSN_T_DIMENSIONS, "dimensions");
  dmnsn_token_map(DMNSN_T_DIRECTION, "direction");
  dmnsn_token_map(DMNSN_T_DISC, "disc");
  dmnsn_token_map(DMNSN_T_DISPERSION, "dispersion");
  dmnsn_token_map(DMNSN_T_DISPERSION_SAMPLES, "dispersion_samples");
  dmnsn_token_map(DMNSN_T_DIST_EXP, "dist_exp");
  dmnsn_token_map(DMNSN_T_DISTANCE, "distance");
  dmnsn_token_map(DMNSN_T_DIV, "div");
  dmnsn_token_map(DMNSN_T_DOUBLE_ILLUMINATE, "double_illuminate");
  dmnsn_token_map(DMNSN_T_ECCENTRICITY, "eccentricity");
  dmnsn_token_map(DMNSN_T_EMISSION, "emission");
  dmnsn_token_map(DMNSN_T_ERROR_BOUND, "error_bound");
  dmnsn_token_map(DMNSN_T_EVALUATE, "evaluate");
  dmnsn_token_map(DMNSN_T_EXP, "exp");
  dmnsn_token_map(DMNSN_T_EXPAND_THRESHOLDS, "expand_thresholds");
  dmnsn_token_map(DMNSN_T_EXPONENT, "exponent");
  dmnsn_token_map(DMNSN_T_EXTERIOR, "exterior");
  dmnsn_token_map(DMNSN_T_EXTINCTION, "extinction");
  dmnsn_token_map(DMNSN_T_FACE_INDICES, "face_indices");
  dmnsn_token_map(DMNSN_T_FACETS, "facets");
  dmnsn_token_map(DMNSN_T_FADE_COLOR, "fade_color");
  dmnsn_token_map(DMNSN_T_FADE_DISTANCE, "fade_distance");
  dmnsn_token_map(DMNSN_T_FADE_POWER, "fade_power");
  dmnsn_token_map(DMNSN_T_FALLOFF, "falloff");
  dmnsn_token_map(DMNSN_T_FALLOFF_ANGLE, "falloff_angle");
  dmnsn_token_map(DMNSN_T_FALSE, "false");
  dmnsn_token_map(DMNSN_T_FILE_EXISTS, "file_exists");
  dmnsn_token_map(DMNSN_T_FILTER, "filter");
  dmnsn_token_map(DMNSN_T_FINAL_CLOCK, "final_clock");
  dmnsn_token_map(DMNSN_T_FINAL_FRAME, "final_frame");
  dmnsn_token_map(DMNSN_T_FINISH, "finish");
  dmnsn_token_map(DMNSN_T_FISHEYE, "fisheye");
  dmnsn_token_map(DMNSN_T_FLATNESS, "flatness");
  dmnsn_token_map(DMNSN_T_FLIP, "flip");
  dmnsn_token_map(DMNSN_T_FLOOR, "floor");
  dmnsn_token_map(DMNSN_T_FOCAL_POINT, "focal_point");
  dmnsn_token_map(DMNSN_T_FOG, "fog");
  dmnsn_token_map(DMNSN_T_FOG_ALT, "fog_alt");
  dmnsn_token_map(DMNSN_T_FOG_OFFSET, "fog_offset");
  dmnsn_token_map(DMNSN_T_FOG_TYPE, "fog_type");
  dmnsn_token_map(DMNSN_T_FORM, "form");
  dmnsn_token_map(DMNSN_T_FRAME_NUMBER, "frame_number");
  dmnsn_token_map(DMNSN_T_FREQUENCY, "frequency");
  dmnsn_token_map(DMNSN_T_FRESNEL, "fresnel");
  dmnsn_token_map(DMNSN_T_FUNCTION, "function");
  dmnsn_token_map(DMNSN_T_GATHER, "gather");
  dmnsn_token_map(DMNSN_T_GIF, "gif");
  dmnsn_token_map(DMNSN_T_GLOBAL_LIGHTS, "global_lights");
  dmnsn_token_map(DMNSN_T_GLOBAL_SETTINGS, "global_settings");
  dmnsn_token_map(DMNSN_T_GRADIENT, "gradient");
  dmnsn_token_map(DMNSN_T_GRANITE, "granite");
  dmnsn_token_map(DMNSN_T_GRAY, "gray");
  dmnsn_token_map(DMNSN_T_GRAY_THRESHOLD, "gray_threshold");
  dmnsn_token_map(DMNSN_T_GREEN, "green");
  dmnsn_token_map(DMNSN_T_HEIGHT_FIELD, "height_field");
  dmnsn_token_map(DMNSN_T_HEXAGON, "hexagon");
  dmnsn_token_map(DMNSN_T_HF_GRAY_16, "hf_gray_16");
  dmnsn_token_map(DMNSN_T_HIERARCHY, "hierarchy");
  dmnsn_token_map(DMNSN_T_HYPERCOMPLEX, "hypercomplex");
  dmnsn_token_map(DMNSN_T_HOLLOW, "hollow");
  dmnsn_token_map(DMNSN_T_IFF, "iff");
  dmnsn_token_map(DMNSN_T_IMAGE_HEIGHT, "image_height");
  dmnsn_token_map(DMNSN_T_IMAGE_MAP, "image_map");
  dmnsn_token_map(DMNSN_T_IMAGE_PATTERN, "image_pattern");
  dmnsn_token_map(DMNSN_T_IMAGE_WIDTH, "image_width");
  dmnsn_token_map(DMNSN_T_INITIAL_CLOCK, "initial_clock");
  dmnsn_token_map(DMNSN_T_INITIAL_FRAME, "initial_frame");
  dmnsn_token_map(DMNSN_T_INSIDE, "inside");
  dmnsn_token_map(DMNSN_T_INSIDE_VECTOR, "inside_vector");
  dmnsn_token_map(DMNSN_T_INT, "int");
  dmnsn_token_map(DMNSN_T_INTERIOR, "interior");
  dmnsn_token_map(DMNSN_T_INTERIOR_TEXTURE, "interior_texture");
  dmnsn_token_map(DMNSN_T_INTERNAL, "internal");
  dmnsn_token_map(DMNSN_T_INTERPOLATE, "interpolate");
  dmnsn_token_map(DMNSN_T_INTERSECTION, "intersection");
  dmnsn_token_map(DMNSN_T_INTERVALS, "intervals");
  dmnsn_token_map(DMNSN_T_INVERSE, "inverse");
  dmnsn_token_map(DMNSN_T_IOR, "ior");
  dmnsn_token_map(DMNSN_T_IRID, "irid");
  dmnsn_token_map(DMNSN_T_IRID_WAVELENGTH, "irid_wavelength");
  dmnsn_token_map(DMNSN_T_ISOSURFACE, "isosurface");
  dmnsn_token_map(DMNSN_T_JITTER, "jitter");
  dmnsn_token_map(DMNSN_T_JPEG, "jpeg");
  dmnsn_token_map(DMNSN_T_JULIA, "julia");
  dmnsn_token_map(DMNSN_T_JULIA_FRACTAL, "julia_fractal");
  dmnsn_token_map(DMNSN_T_LAMBDA, "lambda");
  dmnsn_token_map(DMNSN_T_LATHE, "lathe");
  dmnsn_token_map(DMNSN_T_LEOPARD, "leopard");
  dmnsn_token_map(DMNSN_T_LIGHT_GROUP, "light_group");
  dmnsn_token_map(DMNSN_T_LIGHT_SOURCE, "light_source");
  dmnsn_token_map(DMNSN_T_LINEAR_SPLINE, "linear_spline");
  dmnsn_token_map(DMNSN_T_LINEAR_SWEEP, "linear_sweep");
  dmnsn_token_map(DMNSN_T_LN, "ln");
  dmnsn_token_map(DMNSN_T_LOAD_FILE, "load_file");
  dmnsn_token_map(DMNSN_T_LOCATION, "location");
  dmnsn_token_map(DMNSN_T_LOG, "log");
  dmnsn_token_map(DMNSN_T_LOOK_AT, "look_at");
  dmnsn_token_map(DMNSN_T_LOOKS_LIKE, "looks_like");
  dmnsn_token_map(DMNSN_T_LOW_ERROR_FACTOR, "low_error_factor");
  dmnsn_token_map(DMNSN_T_MAGNET, "magnet");
  dmnsn_token_map(DMNSN_T_MAJOR_RADIUS, "major_radius");
  dmnsn_token_map(DMNSN_T_MANDEL, "mandel");
  dmnsn_token_map(DMNSN_T_MAP_TYPE, "map_type");
  dmnsn_token_map(DMNSN_T_MARBLE, "marble");
  dmnsn_token_map(DMNSN_T_MATERIAL, "material");
  dmnsn_token_map(DMNSN_T_MATERIAL_MAP, "material_map");
  dmnsn_token_map(DMNSN_T_MATRIX, "matrix");
  dmnsn_token_map(DMNSN_T_MAX, "max");
  dmnsn_token_map(DMNSN_T_MAX_EXTENT, "max_extent");
  dmnsn_token_map(DMNSN_T_MAX_GRADIENT, "max_gradient");
  dmnsn_token_map(DMNSN_T_MAX_INTERSECTIONS, "max_intersections");
  dmnsn_token_map(DMNSN_T_MAX_ITERATION, "max_iteration");
  dmnsn_token_map(DMNSN_T_MAX_SAMPLE, "max_sample");
  dmnsn_token_map(DMNSN_T_MAX_TRACE, "max_trace");
  dmnsn_token_map(DMNSN_T_MAX_TRACE_LEVEL, "max_trace_level");
  dmnsn_token_map(DMNSN_T_MEDIA, "media");
  dmnsn_token_map(DMNSN_T_MEDIA_ATTENUATION, "media_attenuation");
  dmnsn_token_map(DMNSN_T_MEDIA_INTERACTION, "media_interaction");
  dmnsn_token_map(DMNSN_T_MERGE, "merge");
  dmnsn_token_map(DMNSN_T_MESH, "mesh");
  dmnsn_token_map(DMNSN_T_MESH2, "mesh2");
  dmnsn_token_map(DMNSN_T_METALLIC, "metallic");
  dmnsn_token_map(DMNSN_T_METHOD, "method");
  dmnsn_token_map(DMNSN_T_METRIC, "metric");
  dmnsn_token_map(DMNSN_T_MIN, "min");
  dmnsn_token_map(DMNSN_T_MIN_EXTENT, "min_extent");
  dmnsn_token_map(DMNSN_T_MINIMUM_REUSE, "minimum_reuse");
  dmnsn_token_map(DMNSN_T_MOD, "mod");
  dmnsn_token_map(DMNSN_T_MORTAR, "mortar");
  dmnsn_token_map(DMNSN_T_NATURAL_SPLINE, "natural_spline");
  dmnsn_token_map(DMNSN_T_NEAREST_COUNT, "nearest_count");
  dmnsn_token_map(DMNSN_T_NO, "no");
  dmnsn_token_map(DMNSN_T_NO_BUMP_SCALE, "no_bump_scale");
  dmnsn_token_map(DMNSN_T_NO_IMAGE, "no_image");
  dmnsn_token_map(DMNSN_T_NO_REFLECTION, "no_reflection");
  dmnsn_token_map(DMNSN_T_NO_SHADOW, "no_shadow");
  dmnsn_token_map(DMNSN_T_NOISE_GENERATOR, "noise_generator");
  dmnsn_token_map(DMNSN_T_NORMAL, "normal");
  dmnsn_token_map(DMNSN_T_NORMAL_INDICES, "normal_indices");
  dmnsn_token_map(DMNSN_T_NORMAL_MAP, "normal_map");
  dmnsn_token_map(DMNSN_T_NORMAL_VECTORS, "normal_vectors");
  dmnsn_token_map(DMNSN_T_NUMBER_OF_WAVES, "number_of_waves");
  dmnsn_token_map(DMNSN_T_OBJECT, "object");
  dmnsn_token_map(DMNSN_T_OCTAVES, "octaves");
  dmnsn_token_map(DMNSN_T_OFF, "off");
  dmnsn_token_map(DMNSN_T_OFFSET, "offset");
  dmnsn_token_map(DMNSN_T_OMEGA, "omega");
  dmnsn_token_map(DMNSN_T_OMNIMAX, "omnimax");
  dmnsn_token_map(DMNSN_T_ON, "on");
  dmnsn_token_map(DMNSN_T_ONCE, "once");
  dmnsn_token_map(DMNSN_T_ONION, "onion");
  dmnsn_token_map(DMNSN_T_OPEN, "open");
  dmnsn_token_map(DMNSN_T_ORIENT, "orient");
  dmnsn_token_map(DMNSN_T_ORIENTATION, "orientation");
  dmnsn_token_map(DMNSN_T_ORTHOGRAPHIC, "orthographic");
  dmnsn_token_map(DMNSN_T_PANORAMIC, "panoramic");
  dmnsn_token_map(DMNSN_T_PARALLEL, "parallel");
  dmnsn_token_map(DMNSN_T_PARAMETRIC, "parametric");
  dmnsn_token_map(DMNSN_T_PASS_THROUGH, "pass_through");
  dmnsn_token_map(DMNSN_T_PATTERN, "pattern");
  dmnsn_token_map(DMNSN_T_PERSPECTIVE, "perspective");
  dmnsn_token_map(DMNSN_T_PGM, "pgm");
  dmnsn_token_map(DMNSN_T_PHASE, "phase");
  dmnsn_token_map(DMNSN_T_PHONG, "phong");
  dmnsn_token_map(DMNSN_T_PHONG_SIZE, "phong_size");
  dmnsn_token_map(DMNSN_T_PHOTONS, "photons");
  dmnsn_token_map(DMNSN_T_PI, "pi");
  dmnsn_token_map(DMNSN_T_PIGMENT, "pigment");
  dmnsn_token_map(DMNSN_T_PIGMENT_MAP, "pigment_map");
  dmnsn_token_map(DMNSN_T_PIGMENT_PATTERN, "pigment_pattern");
  dmnsn_token_map(DMNSN_T_PLANAR, "planar");
  dmnsn_token_map(DMNSN_T_PLANE, "plane");
  dmnsn_token_map(DMNSN_T_PNG, "png");
  dmnsn_token_map(DMNSN_T_POINT_AT, "point_at");
  dmnsn_token_map(DMNSN_T_POLY, "poly");
  dmnsn_token_map(DMNSN_T_POLY_WAVE, "poly_wave");
  dmnsn_token_map(DMNSN_T_POLYGON, "polygon");
  dmnsn_token_map(DMNSN_T_POT, "pot");
  dmnsn_token_map(DMNSN_T_POW, "pow");
  dmnsn_token_map(DMNSN_T_PPM, "ppm");
  dmnsn_token_map(DMNSN_T_PRECISION, "precision");
  dmnsn_token_map(DMNSN_T_PRECOMPUTE, "precompute");
  dmnsn_token_map(DMNSN_T_PRETRACE_END, "pretrace_end");
  dmnsn_token_map(DMNSN_T_PRETRACE_START, "pretrace_start");
  dmnsn_token_map(DMNSN_T_PRISM, "prism");
  dmnsn_token_map(DMNSN_T_PROD, "prod");
  dmnsn_token_map(DMNSN_T_PROJECTED_THROUGH, "projected_through");
  dmnsn_token_map(DMNSN_T_PWR, "pwr");
  dmnsn_token_map(DMNSN_T_QUADRATIC_SPLINE, "quadratic_spline");
  dmnsn_token_map(DMNSN_T_QUADRIC, "quadric");
  dmnsn_token_map(DMNSN_T_QUARTIC, "quartic");
  dmnsn_token_map(DMNSN_T_QUATERNION, "quaternion");
  dmnsn_token_map(DMNSN_T_QUICK_COLOR, "quick_color");
  dmnsn_token_map(DMNSN_T_QUILTED, "quilted");
  dmnsn_token_map(DMNSN_T_RADIAL, "radial");
  dmnsn_token_map(DMNSN_T_RADIANS, "radians");
  dmnsn_token_map(DMNSN_T_RADIOSITY, "radiosity");
  dmnsn_token_map(DMNSN_T_RADIUS, "radius");
  dmnsn_token_map(DMNSN_T_RAINBOW, "rainbow");
  dmnsn_token_map(DMNSN_T_RAMP_WAVE, "ramp_wave");
  dmnsn_token_map(DMNSN_T_RAND, "rand");
  dmnsn_token_map(DMNSN_T_RATIO, "ratio");
  dmnsn_token_map(DMNSN_T_RECIPROCAL, "reciprocal");
  dmnsn_token_map(DMNSN_T_RECURSION_LIMIT, "recursion_limit");
  dmnsn_token_map(DMNSN_T_RED, "red");
  dmnsn_token_map(DMNSN_T_REFLECTION, "reflection");
  dmnsn_token_map(DMNSN_T_REFLECTION_EXPONENT, "reflection_exponent");
  dmnsn_token_map(DMNSN_T_REFRACTION, "refraction");
  dmnsn_token_map(DMNSN_T_REPEAT, "repeat");
  dmnsn_token_map(DMNSN_T_RGB, "rgb");
  dmnsn_token_map(DMNSN_T_RGBF, "rgbf");
  dmnsn_token_map(DMNSN_T_RGBFT, "rgbft");
  dmnsn_token_map(DMNSN_T_RGBT, "rgbt");
  dmnsn_token_map(DMNSN_T_RIGHT, "right");
  dmnsn_token_map(DMNSN_T_RIPPLES, "ripples");
  dmnsn_token_map(DMNSN_T_ROTATE, "rotate");
  dmnsn_token_map(DMNSN_T_ROUGHNESS, "roughness");
  dmnsn_token_map(DMNSN_T_SAMPLES, "samples");
  dmnsn_token_map(DMNSN_T_SAVE_FILE, "save_file");
  dmnsn_token_map(DMNSN_T_SCALE, "scale");
  dmnsn_token_map(DMNSN_T_SCALLOP_WAVE, "scallop_wave");
  dmnsn_token_map(DMNSN_T_SCATTERING, "scattering");
  dmnsn_token_map(DMNSN_T_SEED, "seed");
  dmnsn_token_map(DMNSN_T_SELECT, "select");
  dmnsn_token_map(DMNSN_T_SHADOWLESS, "shadowless");
  dmnsn_token_map(DMNSN_T_SIN, "sin");
  dmnsn_token_map(DMNSN_T_SINE_WAVE, "sine_wave");
  dmnsn_token_map(DMNSN_T_SINH, "sinh");
  dmnsn_token_map(DMNSN_T_SIZE, "size");
  dmnsn_token_map(DMNSN_T_SKY, "sky");
  dmnsn_token_map(DMNSN_T_SKY_SPHERE, "sky_sphere");
  dmnsn_token_map(DMNSN_T_SLICE, "slice");
  dmnsn_token_map(DMNSN_T_SLOPE, "slope");
  dmnsn_token_map(DMNSN_T_SLOPE_MAP, "slope_map");
  dmnsn_token_map(DMNSN_T_SMOOTH, "smooth");
  dmnsn_token_map(DMNSN_T_SMOOTH_TRIANGLE, "smooth_triangle");
  dmnsn_token_map(DMNSN_T_SOLID, "solid");
  dmnsn_token_map(DMNSN_T_SOR, "sor");
  dmnsn_token_map(DMNSN_T_SPACING, "spacing");
  dmnsn_token_map(DMNSN_T_SPECULAR, "specular");
  dmnsn_token_map(DMNSN_T_SPHERE, "sphere");
  dmnsn_token_map(DMNSN_T_SPHERE_SWEEP, "sphere_sweep");
  dmnsn_token_map(DMNSN_T_SPHERICAL, "spherical");
  dmnsn_token_map(DMNSN_T_SPIRAL1, "spiral1");
  dmnsn_token_map(DMNSN_T_SPIRAL2, "spiral2");
  dmnsn_token_map(DMNSN_T_SPLINE, "spline");
  dmnsn_token_map(DMNSN_T_SPLIT_UNION, "split_union");
  dmnsn_token_map(DMNSN_T_SPOTLIGHT, "spotlight");
  dmnsn_token_map(DMNSN_T_SPOTTED, "spotted");
  dmnsn_token_map(DMNSN_T_SQR, "sqr");
  dmnsn_token_map(DMNSN_T_SQRT, "sqrt");
  dmnsn_token_map(DMNSN_T_STR, "str");
  dmnsn_token_map(DMNSN_T_STRCMP, "strcmp");
  dmnsn_token_map(DMNSN_T_STRENGTH, "strength");
  dmnsn_token_map(DMNSN_T_STRLEN, "strlen");
  dmnsn_token_map(DMNSN_T_STRLWR, "strlwr");
  dmnsn_token_map(DMNSN_T_STRUPR, "strupr");
  dmnsn_token_map(DMNSN_T_STURM, "sturm");
  dmnsn_token_map(DMNSN_T_SUBSTR, "substr");
  dmnsn_token_map(DMNSN_T_SUM, "sum");
  dmnsn_token_map(DMNSN_T_SUPERELLIPSOID, "superellipsoid");
  dmnsn_token_map(DMNSN_T_SYS, "sys");
  dmnsn_token_map(DMNSN_T_T, "t");
  dmnsn_token_map(DMNSN_T_TAN, "tan");
  dmnsn_token_map(DMNSN_T_TANH, "tanh");
  dmnsn_token_map(DMNSN_T_TARGET, "target");
  dmnsn_token_map(DMNSN_T_TEXT, "text");
  dmnsn_token_map(DMNSN_T_TEXTURE, "texture");
  dmnsn_token_map(DMNSN_T_TEXTURE_LIST, "texture_list");
  dmnsn_token_map(DMNSN_T_TEXTURE_MAP, "texture_map");
  dmnsn_token_map(DMNSN_T_TGA, "tga");
  dmnsn_token_map(DMNSN_T_THICKNESS, "thickness");
  dmnsn_token_map(DMNSN_T_THRESHOLD, "threshold");
  dmnsn_token_map(DMNSN_T_TIFF, "tiff");
  dmnsn_token_map(DMNSN_T_TIGHTNESS, "tightness");
  dmnsn_token_map(DMNSN_T_TILE2, "tile2");
  dmnsn_token_map(DMNSN_T_TILES, "tiles");
  dmnsn_token_map(DMNSN_T_TOLERANCE, "tolerance");
  dmnsn_token_map(DMNSN_T_TOROIDAL, "toroidal");
  dmnsn_token_map(DMNSN_T_TORUS, "torus");
  dmnsn_token_map(DMNSN_T_TRACE, "trace");
  dmnsn_token_map(DMNSN_T_TRANSFORM, "transform");
  dmnsn_token_map(DMNSN_T_TRANSLATE, "translate");
  dmnsn_token_map(DMNSN_T_TRANSMIT, "transmit");
  dmnsn_token_map(DMNSN_T_TRIANGLE, "triangle");
  dmnsn_token_map(DMNSN_T_TRIANGLE_WAVE, "triangle_wave");
  dmnsn_token_map(DMNSN_T_TRUE, "true");
  dmnsn_token_map(DMNSN_T_TTF, "ttf");
  dmnsn_token_map(DMNSN_T_TURB_DEPTH, "turb_depth");
  dmnsn_token_map(DMNSN_T_TURBULENCE, "turbulence");
  dmnsn_token_map(DMNSN_T_TYPE, "type");
  dmnsn_token_map(DMNSN_T_U, "u");
  dmnsn_token_map(DMNSN_T_U_STEPS, "u_steps");
  dmnsn_token_map(DMNSN_T_ULTRA_WIDE_ANGLE, "ultra_wide_angle");
  dmnsn_token_map(DMNSN_T_UNION, "union");
  dmnsn_token_map(DMNSN_T_UP, "up");
  dmnsn_token_map(DMNSN_T_USE_ALPHA, "use_alpha");
  dmnsn_token_map(DMNSN_T_USE_COLOR, "use_color");
  dmnsn_token_map(DMNSN_T_USE_INDEX, "use_index");
  dmnsn_token_map(DMNSN_T_UTF8, "utf8");
  dmnsn_token_map(DMNSN_T_UV_INDICES, "uv_indices");
  dmnsn_token_map(DMNSN_T_UV_MAPPING, "uv_mapping");
  dmnsn_token_map(DMNSN_T_UV_VECTORS, "uv_vectors");
  dmnsn_token_map(DMNSN_T_V, "v");
  dmnsn_token_map(DMNSN_T_V_STEPS, "v_steps");
  dmnsn_token_map(DMNSN_T_VAL, "val");
  dmnsn_token_map(DMNSN_T_VARIANCE, "variance");
  dmnsn_token_map(DMNSN_T_VAXIS_ROTATE, "vaxis_rotate");
  dmnsn_token_map(DMNSN_T_VCROSS, "vcross");
  dmnsn_token_map(DMNSN_T_VDOT, "vdot");
  dmnsn_token_map(DMNSN_T_VERTEX_VECTORS, "vertex_vectors");
  dmnsn_token_map(DMNSN_T_VLENGTH, "vlength");
  dmnsn_token_map(DMNSN_T_VNORMALIZE, "vnormalize");
  dmnsn_token_map(DMNSN_T_VROTATE, "vrotate");
  dmnsn_token_map(DMNSN_T_VSTR, "vstr");
  dmnsn_token_map(DMNSN_T_VTURBULENCE, "vturbulence");
  dmnsn_token_map(DMNSN_T_WARP, "warp");
  dmnsn_token_map(DMNSN_T_WATER_LEVEL, "water_level");
  dmnsn_token_map(DMNSN_T_WAVES, "waves");
  dmnsn_token_map(DMNSN_T_WIDTH, "width");
  dmnsn_token_map(DMNSN_T_WOOD, "wood");
  dmnsn_token_map(DMNSN_T_WRINKLES, "wrinkles");
  dmnsn_token_map(DMNSN_T_X, "x");
  dmnsn_token_map(DMNSN_T_Y, "y");
  dmnsn_token_map(DMNSN_T_YES, "yes");
  dmnsn_token_map(DMNSN_T_Z, "z");

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
    fprintf(stderr, "Warning: unrecognised token %d.\n", (int)token_type);
    return "unrecognized-token";
  }
}
