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

%{
#include "parse.h"
#include "tokenize.h"
#include "utility.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct dmnsn_token_iterator {
  const dmnsn_array *tokens;
  unsigned int i;
} dmnsn_token_iterator;

#define YYSTYPE const char *

typedef struct dmnsn_location {
  const char *first_filename, *last_filename;
  int first_line, last_line;
  int first_column, last_column;
} dmnsn_location;

#define YYLTYPE dmnsn_location
#define YYLLOC_DEFAULT(Current, Rhs, N)                                 \
  do {                                                                  \
    if (N) {                                                            \
      (Current).first_filename = YYRHSLOC(Rhs, 1).first_filename;       \
      (Current).first_line     = YYRHSLOC(Rhs, 1).first_line;           \
      (Current).first_column   = YYRHSLOC(Rhs, 1).first_column;         \
      (Current).last_filename  = YYRHSLOC(Rhs, N).last_filename;        \
      (Current).last_line      = YYRHSLOC(Rhs, N).last_line;            \
      (Current).last_column    = YYRHSLOC(Rhs, N).last_column;          \
    } else {                                                            \
      (Current).first_filename = (Current).last_filename =              \
        YYRHSLOC(Rhs, 0).last_filename;                                 \
      (Current).first_line     = (Current).last_line     =              \
        YYRHSLOC(Rhs, 0).last_line;                                     \
      (Current).first_column   = (Current).last_column   =              \
        YYRHSLOC(Rhs, 0).last_column;                                   \
    }                                                                   \
  } while (0)

static int
yylex(YYSTYPE *lvalp, YYLTYPE *llocp, dmnsn_token_iterator *iterator)
{
  if (iterator->i >= dmnsn_array_size(iterator->tokens)) {
    return 0;
  } else {
    dmnsn_token token;
    dmnsn_array_get(iterator->tokens, iterator->i, &token);
    ++iterator->i;

    *lvalp = token.value;

    llocp->first_filename = llocp->last_filename = token.filename;
    llocp->first_line     = llocp->last_line     = token.line;
    llocp->first_column   = llocp->last_column   = token.col;

    return token.type;
  }  
}

void
yyerror(YYLTYPE *locp, dmnsn_array *astree, dmnsn_token_iterator *iterator,
        const char *str)
{
  dmnsn_diagnostic(locp->first_filename, locp->first_line, locp->first_column,
                   "%s", str);
}
%}

%define api.pure
%locations
%error-verbose
%token-table

%parse-param {dmnsn_array *astree}
%parse-param {dmnsn_token_iterator *iterator}
%lex-param {dmnsn_token_iterator *iterator}

%token END 0 "end-of-file"

/* Punctuation */
%token DMNSN_T_LBRACE        "{"
%token DMNSN_T_RBRACE        "}"
%token DMNSN_T_LPAREN        "("
%token DMNSN_T_RPAREN        ")"
%token DMNSN_T_LBRACKET      "["
%token DMNSN_T_RBRACKET      "]"
%token DMNSN_T_PLUS          "+"
%token DMNSN_T_MINUS         "-"
%token DMNSN_T_STAR          "*"
%token DMNSN_T_SLASH         "/"
%token DMNSN_T_COMMA         ","
%token DMNSN_T_SEMICOLON     ";"
%token DMNSN_T_QUESTION      "?"
%token DMNSN_T_COLON         ":"
%token DMNSN_T_AND           "&"
%token DMNSN_T_DOT           "."
%token DMNSN_T_PIPE          "|"
%token DMNSN_T_LESS          "<"
%token DMNSN_T_GREATER       ">"
%token DMNSN_T_BANG          "!"
%token DMNSN_T_EQUALS        "="
%token DMNSN_T_LESS_EQUAL    "<="
%token DMNSN_T_GREATER_EQUAL ">="
%token DMNSN_T_NOT_EQUAL     "!="

/* Numeric values */
%token DMNSN_T_INTEGER "integer"
%token DMNSN_T_FLOAT   "float"

/* Keywords */
%token DMNSN_T_AA_LEVEL
%token DMNSN_T_AA_THRESHOLD
%token DMNSN_T_ABS
%token DMNSN_T_ABSORPTION
%token DMNSN_T_ACCURACY
%token DMNSN_T_ACOS
%token DMNSN_T_ACOSH
%token DMNSN_T_ADAPTIVE
%token DMNSN_T_ADC_BAILOUT
%token DMNSN_T_AGATE
%token DMNSN_T_AGATE_TURB
%token DMNSN_T_ALL
%token DMNSN_T_ALL_INTERSECTIONS
%token DMNSN_T_ALPHA
%token DMNSN_T_ALTITUDE
%token DMNSN_T_ALWAYS_SAMPLE
%token DMNSN_T_AMBIENT
%token DMNSN_T_AMBIENT_LIGHT
%token DMNSN_T_ANGLE
%token DMNSN_T_APERTURE
%token DMNSN_T_APPEND
%token DMNSN_T_ARC_ANGLE
%token DMNSN_T_AREA_LIGHT
%token DMNSN_T_ARRAY
%token DMNSN_T_ASC
%token DMNSN_T_ASCII
%token DMNSN_T_ASIN
%token DMNSN_T_ASINH
%token DMNSN_T_ASSUMED_GAMMA
%token DMNSN_T_ATAN
%token DMNSN_T_ATAN2
%token DMNSN_T_ATANH
%token DMNSN_T_AUTOSTOP
%token DMNSN_T_AVERAGE
%token DMNSN_T_B_SPLINE
%token DMNSN_T_BACKGROUND
%token DMNSN_T_BEZIER_SPLINE
%token DMNSN_T_BICUBIC_PATCH
%token DMNSN_T_BLACK_HOLE
%token DMNSN_T_BLOB
%token DMNSN_T_BLUE
%token DMNSN_T_BLUR_SAMPLES
%token DMNSN_T_BOUNDED_BY
%token DMNSN_T_BOX                      "box"
%token DMNSN_T_BOXED
%token DMNSN_T_BOZO
%token DMNSN_T_BRICK
%token DMNSN_T_BRICK_SIZE
%token DMNSN_T_BRIGHTNESS
%token DMNSN_T_BRILLIANCE
%token DMNSN_T_BUMP_MAP
%token DMNSN_T_BUMP_SIZE
%token DMNSN_T_BUMPS
%token DMNSN_T_CAMERA                   "camera"
%token DMNSN_T_CAUSTICS
%token DMNSN_T_CEIL
%token DMNSN_T_CELLS
%token DMNSN_T_CHARSET
%token DMNSN_T_CHECKER
%token DMNSN_T_CHR
%token DMNSN_T_CIRCULAR
%token DMNSN_T_CLIPPED_BY
%token DMNSN_T_CLOCK
%token DMNSN_T_CLOCK_DELTA
%token DMNSN_T_CLOCK_ON
%token DMNSN_T_COLLECT
%token DMNSN_T_COLOR                    "color"
%token DMNSN_T_COLOR_MAP
%token DMNSN_T_COMPONENT
%token DMNSN_T_COMPOSITE
%token DMNSN_T_CONCAT
%token DMNSN_T_CONE
%token DMNSN_T_CONFIDENCE
%token DMNSN_T_CONIC_SWEEP
%token DMNSN_T_CONSERVE_ENERGY
%token DMNSN_T_CONTAINED_BY
%token DMNSN_T_CONTROL0
%token DMNSN_T_CONTROL1
%token DMNSN_T_COORDS
%token DMNSN_T_COS
%token DMNSN_T_COSH
%token DMNSN_T_COUNT
%token DMNSN_T_CRACKLE
%token DMNSN_T_CRAND
%token DMNSN_T_CUBE
%token DMNSN_T_CUBIC
%token DMNSN_T_CUBIC_SPLINE
%token DMNSN_T_CUBIC_WAVE
%token DMNSN_T_CUTAWAY_TEXTURES
%token DMNSN_T_CYLINDER
%token DMNSN_T_CYLINDRICAL
%token DMNSN_T_DEFINED
%token DMNSN_T_DEGREES
%token DMNSN_T_DENSITY
%token DMNSN_T_DENSITY_FILE
%token DMNSN_T_DENSITY_MAP
%token DMNSN_T_DENTS
%token DMNSN_T_DF3
%token DMNSN_T_DIFFERENCE
%token DMNSN_T_DIFFUSE
%token DMNSN_T_DIMENSION_SIZE
%token DMNSN_T_DIMENSIONS
%token DMNSN_T_DIRECTION
%token DMNSN_T_DISC
%token DMNSN_T_DISPERSION
%token DMNSN_T_DISPERSION_SAMPLES
%token DMNSN_T_DIST_EXP
%token DMNSN_T_DISTANCE
%token DMNSN_T_DIV
%token DMNSN_T_DOUBLE_ILLUMINATE
%token DMNSN_T_ECCENTRICITY
%token DMNSN_T_EMISSION
%token DMNSN_T_ERROR_BOUND
%token DMNSN_T_EVALUATE
%token DMNSN_T_EXP
%token DMNSN_T_EXPAND_THRESHOLDS
%token DMNSN_T_EXPONENT
%token DMNSN_T_EXTERIOR
%token DMNSN_T_EXTINCTION
%token DMNSN_T_FACE_INDICES
%token DMNSN_T_FACETS
%token DMNSN_T_FADE_COLOR
%token DMNSN_T_FADE_DISTANCE
%token DMNSN_T_FADE_POWER
%token DMNSN_T_FALLOFF
%token DMNSN_T_FALLOFF_ANGLE
%token DMNSN_T_FALSE
%token DMNSN_T_FILE_EXISTS
%token DMNSN_T_FILTER
%token DMNSN_T_FINAL_CLOCK
%token DMNSN_T_FINAL_FRAME
%token DMNSN_T_FINISH
%token DMNSN_T_FISHEYE
%token DMNSN_T_FLATNESS
%token DMNSN_T_FLIP
%token DMNSN_T_FLOOR
%token DMNSN_T_FOCAL_POINT
%token DMNSN_T_FOG
%token DMNSN_T_FOG_ALT
%token DMNSN_T_FOG_OFFSET
%token DMNSN_T_FOG_TYPE
%token DMNSN_T_FORM
%token DMNSN_T_FRAME_NUMBER
%token DMNSN_T_FREQUENCY
%token DMNSN_T_FRESNEL
%token DMNSN_T_FUNCTION
%token DMNSN_T_GATHER
%token DMNSN_T_GIF
%token DMNSN_T_GLOBAL_LIGHTS
%token DMNSN_T_GLOBAL_SETTINGS
%token DMNSN_T_GRADIENT
%token DMNSN_T_GRANITE
%token DMNSN_T_GRAY
%token DMNSN_T_GRAY_THRESHOLD
%token DMNSN_T_GREEN
%token DMNSN_T_HEIGHT_FIELD
%token DMNSN_T_HEXAGON
%token DMNSN_T_HF_GRAY_16
%token DMNSN_T_HIERARCHY
%token DMNSN_T_HYPERCOMPLEX
%token DMNSN_T_HOLLOW
%token DMNSN_T_IFF
%token DMNSN_T_IMAGE_HEIGHT
%token DMNSN_T_IMAGE_MAP
%token DMNSN_T_IMAGE_PATTERN
%token DMNSN_T_IMAGE_WIDTH
%token DMNSN_T_INITIAL_CLOCK
%token DMNSN_T_INITIAL_FRAME
%token DMNSN_T_INSIDE
%token DMNSN_T_INSIDE_VECTOR
%token DMNSN_T_INT
%token DMNSN_T_INTERIOR
%token DMNSN_T_INTERIOR_TEXTURE
%token DMNSN_T_INTERNAL
%token DMNSN_T_INTERPOLATE
%token DMNSN_T_INTERSECTION
%token DMNSN_T_INTERVALS
%token DMNSN_T_INVERSE
%token DMNSN_T_IOR
%token DMNSN_T_IRID
%token DMNSN_T_IRID_WAVELENGTH
%token DMNSN_T_ISOSURFACE
%token DMNSN_T_JITTER
%token DMNSN_T_JPEG
%token DMNSN_T_JULIA
%token DMNSN_T_JULIA_FRACTAL
%token DMNSN_T_LAMBDA
%token DMNSN_T_LATHE
%token DMNSN_T_LEOPARD
%token DMNSN_T_LIGHT_GROUP
%token DMNSN_T_LIGHT_SOURCE
%token DMNSN_T_LINEAR_SPLINE
%token DMNSN_T_LINEAR_SWEEP
%token DMNSN_T_LN
%token DMNSN_T_LOAD_FILE
%token DMNSN_T_LOCATION
%token DMNSN_T_LOG
%token DMNSN_T_LOOK_AT
%token DMNSN_T_LOOKS_LIKE
%token DMNSN_T_LOW_ERROR_FACTOR
%token DMNSN_T_MAGNET
%token DMNSN_T_MAJOR_RADIUS
%token DMNSN_T_MANDEL
%token DMNSN_T_MAP_TYPE
%token DMNSN_T_MARBLE
%token DMNSN_T_MATERIAL
%token DMNSN_T_MATERIAL_MAP
%token DMNSN_T_MATRIX
%token DMNSN_T_MAX
%token DMNSN_T_MAX_EXTENT
%token DMNSN_T_MAX_GRADIENT
%token DMNSN_T_MAX_INTERSECTIONS
%token DMNSN_T_MAX_ITERATION
%token DMNSN_T_MAX_SAMPLE
%token DMNSN_T_MAX_TRACE
%token DMNSN_T_MAX_TRACE_LEVEL
%token DMNSN_T_MEDIA
%token DMNSN_T_MEDIA_ATTENUATION
%token DMNSN_T_MEDIA_INTERACTION
%token DMNSN_T_MERGE
%token DMNSN_T_MESH
%token DMNSN_T_MESH2
%token DMNSN_T_METALLIC
%token DMNSN_T_METHOD
%token DMNSN_T_METRIC
%token DMNSN_T_MIN
%token DMNSN_T_MIN_EXTENT
%token DMNSN_T_MINIMUM_REUSE
%token DMNSN_T_MOD
%token DMNSN_T_MORTAR
%token DMNSN_T_NATURAL_SPLINE
%token DMNSN_T_NEAREST_COUNT
%token DMNSN_T_NO
%token DMNSN_T_NO_BUMP_SCALE
%token DMNSN_T_NO_IMAGE
%token DMNSN_T_NO_REFLECTION
%token DMNSN_T_NO_SHADOW
%token DMNSN_T_NOISE_GENERATOR
%token DMNSN_T_NORMAL
%token DMNSN_T_NORMAL_INDICES
%token DMNSN_T_NORMAL_MAP
%token DMNSN_T_NORMAL_VECTORS
%token DMNSN_T_NUMBER_OF_WAVES
%token DMNSN_T_OBJECT
%token DMNSN_T_OCTAVES
%token DMNSN_T_OFF
%token DMNSN_T_OFFSET
%token DMNSN_T_OMEGA
%token DMNSN_T_OMNIMAX
%token DMNSN_T_ON
%token DMNSN_T_ONCE
%token DMNSN_T_ONION
%token DMNSN_T_OPEN
%token DMNSN_T_ORIENT
%token DMNSN_T_ORIENTATION
%token DMNSN_T_ORTHOGRAPHIC
%token DMNSN_T_PANORAMIC
%token DMNSN_T_PARALLEL
%token DMNSN_T_PARAMETRIC
%token DMNSN_T_PASS_THROUGH
%token DMNSN_T_PATTERN
%token DMNSN_T_PERSPECTIVE
%token DMNSN_T_PGM
%token DMNSN_T_PHASE
%token DMNSN_T_PHONG
%token DMNSN_T_PHONG_SIZE
%token DMNSN_T_PHOTONS
%token DMNSN_T_PI
%token DMNSN_T_PIGMENT
%token DMNSN_T_PIGMENT_MAP
%token DMNSN_T_PIGMENT_PATTERN
%token DMNSN_T_PLANAR
%token DMNSN_T_PLANE
%token DMNSN_T_PNG
%token DMNSN_T_POINT_AT
%token DMNSN_T_POLY
%token DMNSN_T_POLY_WAVE
%token DMNSN_T_POLYGON
%token DMNSN_T_POT
%token DMNSN_T_POW
%token DMNSN_T_PPM
%token DMNSN_T_PRECISION
%token DMNSN_T_PRECOMPUTE
%token DMNSN_T_PRETRACE_END
%token DMNSN_T_PRETRACE_START
%token DMNSN_T_PRISM
%token DMNSN_T_PROD
%token DMNSN_T_PROJECTED_THROUGH
%token DMNSN_T_PWR
%token DMNSN_T_QUADRATIC_SPLINE
%token DMNSN_T_QUADRIC
%token DMNSN_T_QUARTIC
%token DMNSN_T_QUATERNION
%token DMNSN_T_QUICK_COLOR
%token DMNSN_T_QUILTED
%token DMNSN_T_RADIAL
%token DMNSN_T_RADIANS
%token DMNSN_T_RADIOSITY
%token DMNSN_T_RADIUS
%token DMNSN_T_RAINBOW
%token DMNSN_T_RAMP_WAVE
%token DMNSN_T_RAND
%token DMNSN_T_RATIO
%token DMNSN_T_RECIPROCAL
%token DMNSN_T_RECURSION_LIMIT
%token DMNSN_T_RED
%token DMNSN_T_REFLECTION
%token DMNSN_T_REFLECTION_EXPONENT
%token DMNSN_T_REFRACTION
%token DMNSN_T_REPEAT
%token DMNSN_T_RGB
%token DMNSN_T_RGBF
%token DMNSN_T_RGBFT
%token DMNSN_T_RGBT
%token DMNSN_T_RIGHT
%token DMNSN_T_RIPPLES
%token DMNSN_T_ROTATE
%token DMNSN_T_ROUGHNESS
%token DMNSN_T_SAMPLES
%token DMNSN_T_SAVE_FILE
%token DMNSN_T_SCALE
%token DMNSN_T_SCALLOP_WAVE
%token DMNSN_T_SCATTERING
%token DMNSN_T_SEED
%token DMNSN_T_SELECT
%token DMNSN_T_SHADOWLESS
%token DMNSN_T_SIN
%token DMNSN_T_SINE_WAVE
%token DMNSN_T_SINH
%token DMNSN_T_SIZE
%token DMNSN_T_SKY
%token DMNSN_T_SKY_SPHERE
%token DMNSN_T_SLICE
%token DMNSN_T_SLOPE
%token DMNSN_T_SLOPE_MAP
%token DMNSN_T_SMOOTH
%token DMNSN_T_SMOOTH_TRIANGLE
%token DMNSN_T_SOLID
%token DMNSN_T_SOR
%token DMNSN_T_SPACING
%token DMNSN_T_SPECULAR
%token DMNSN_T_SPHERE                   "sphere"
%token DMNSN_T_SPHERE_SWEEP
%token DMNSN_T_SPHERICAL
%token DMNSN_T_SPIRAL1
%token DMNSN_T_SPIRAL2
%token DMNSN_T_SPLINE
%token DMNSN_T_SPLIT_UNION
%token DMNSN_T_SPOTLIGHT
%token DMNSN_T_SPOTTED
%token DMNSN_T_SQR
%token DMNSN_T_SQRT
%token DMNSN_T_STR
%token DMNSN_T_STRCMP
%token DMNSN_T_STRENGTH
%token DMNSN_T_STRLEN
%token DMNSN_T_STRLWR
%token DMNSN_T_STRUPR
%token DMNSN_T_STURM
%token DMNSN_T_SUBSTR
%token DMNSN_T_SUM
%token DMNSN_T_SUPERELLIPSOID
%token DMNSN_T_SYS
%token DMNSN_T_T
%token DMNSN_T_TAN
%token DMNSN_T_TANH
%token DMNSN_T_TARGET
%token DMNSN_T_TEXT
%token DMNSN_T_TEXTURE
%token DMNSN_T_TEXTURE_LIST
%token DMNSN_T_TEXTURE_MAP
%token DMNSN_T_TGA
%token DMNSN_T_THICKNESS
%token DMNSN_T_THRESHOLD
%token DMNSN_T_TIFF
%token DMNSN_T_TIGHTNESS
%token DMNSN_T_TILE2
%token DMNSN_T_TILES
%token DMNSN_T_TOLERANCE
%token DMNSN_T_TOROIDAL
%token DMNSN_T_TORUS
%token DMNSN_T_TRACE
%token DMNSN_T_TRANSFORM
%token DMNSN_T_TRANSLATE
%token DMNSN_T_TRANSMIT
%token DMNSN_T_TRIANGLE
%token DMNSN_T_TRIANGLE_WAVE
%token DMNSN_T_TRUE
%token DMNSN_T_TTF
%token DMNSN_T_TURB_DEPTH
%token DMNSN_T_TURBULENCE
%token DMNSN_T_TYPE
%token DMNSN_T_U
%token DMNSN_T_U_STEPS
%token DMNSN_T_ULTRA_WIDE_ANGLE
%token DMNSN_T_UNION
%token DMNSN_T_UP
%token DMNSN_T_USE_ALPHA
%token DMNSN_T_USE_COLOR
%token DMNSN_T_USE_INDEX
%token DMNSN_T_UTF8
%token DMNSN_T_UV_INDICES
%token DMNSN_T_UV_MAPPING
%token DMNSN_T_UV_VECTORS
%token DMNSN_T_V
%token DMNSN_T_V_STEPS
%token DMNSN_T_VAL
%token DMNSN_T_VARIANCE
%token DMNSN_T_VAXIS_ROTATE
%token DMNSN_T_VCROSS
%token DMNSN_T_VDOT
%token DMNSN_T_VERTEX_VECTORS
%token DMNSN_T_VLENGTH
%token DMNSN_T_VNORMALIZE
%token DMNSN_T_VROTATE
%token DMNSN_T_VSTR
%token DMNSN_T_VTURBULENCE
%token DMNSN_T_WARP
%token DMNSN_T_WATER_LEVEL
%token DMNSN_T_WAVES
%token DMNSN_T_WIDTH
%token DMNSN_T_WOOD
%token DMNSN_T_WRINKLES
%token DMNSN_T_X
%token DMNSN_T_Y
%token DMNSN_T_YES
%token DMNSN_T_Z

/* Directives (#declare etc.) */
%token DMNSN_T_BREAK
%token DMNSN_T_CASE
%token DMNSN_T_DEBUG
%token DMNSN_T_DECLARE    "#declare"
%token DMNSN_T_DEFAULT
%token DMNSN_T_ELSE
%token DMNSN_T_END
%token DMNSN_T_ERROR
%token DMNSN_T_FCLOSE
%token DMNSN_T_FOPEN
%token DMNSN_T_IF
%token DMNSN_T_IFDEF
%token DMNSN_T_IFNDEF
%token DMNSN_T_INCLUDE    "#include"
%token DMNSN_T_LOCAL
%token DMNSN_T_MACRO
%token DMNSN_T_RANGE
%token DMNSN_T_READ
%token DMNSN_T_RENDER
%token DMNSN_T_STATISTICS 
%token DMNSN_T_SWITCH
%token DMNSN_T_UNDEF
%token DMNSN_T_VERSION
%token DMNSN_T_WARNING
%token DMNSN_T_WHILE
%token DMNSN_T_WRITE

/* Identifiers */
%token DMNSN_T_IDENTIFIER "identifier"

/* Strings */
%token DMNSN_T_STRING "string"

%%

SCENE:   /* empty */
       | SCENE SCENE_ITEM
       ;

SCENE_ITEM:   OBJECT
            ;

OBJECT:   FINITE_SOLID_OBJECT
        ;

FINITE_SOLID_OBJECT:   BOX | SPHERE
                     ;

BOX:   "box" "{" "}"
     ;

SPHERE:   "sphere" "{" "}"
        ;

%%

dmnsn_array *
dmnsn_parse(const dmnsn_array *tokens)
{
  dmnsn_array *astree = dmnsn_new_array(sizeof(dmnsn_astnode));
  dmnsn_token_iterator iterator = { .tokens = tokens, .i = 0 };

  if (yyparse(astree, &iterator) != 0) {
    dmnsn_delete_astree(astree);
    return NULL;
  }

  return astree;
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

static void
dmnsn_print_astnode(FILE *file, dmnsn_astnode astnode)
{
  long ivalue;
  double dvalue;

  switch (astnode.type) {
  case DMNSN_AST_INTEGER:
    ivalue = *(long *)astnode.ptr;
    fprintf(file, "(%s %ld)", dmnsn_astnode_string(astnode.type), ivalue);
    break;

  case DMNSN_AST_FLOAT:
    dvalue = *(double *)astnode.ptr;
    fprintf(file, "(%s %g)", dmnsn_astnode_string(astnode.type), dvalue);
    break;

  default:
    fprintf(file, "%s", dmnsn_astnode_string(astnode.type));
  }
}

static void
dmnsn_print_astree(FILE *file, dmnsn_astnode astnode)
{
  unsigned int i;
  dmnsn_astnode child;

  if (astnode.children && dmnsn_array_size(astnode.children) > 0) {
    fprintf(file, "(");
    dmnsn_print_astnode(file, astnode);
    for (i = 0; i < dmnsn_array_size(astnode.children); ++i) {
      dmnsn_array_get(astnode.children, i, &child);
      fprintf(file, " ");
      dmnsn_print_astree(file, child);
    }
    fprintf(file, ")");
  } else {
    dmnsn_print_astnode(file, astnode);
  }
}

void
dmnsn_print_astree_sexpr(FILE *file, const dmnsn_array *astree)
{
  dmnsn_astnode astnode;
  unsigned int i;

  if (dmnsn_array_size(astree) == 0) {
    fprintf(file, "()");
  } else {
    fprintf(file, "(");
    dmnsn_array_get(astree, 0, &astnode);
    dmnsn_print_astree(file, astnode);

    for (i = 1; i < dmnsn_array_size(astree); ++i) {
      fprintf(file, " ");
      dmnsn_array_get(astree, i, &astnode);
      dmnsn_print_astree(file, astnode);
    }

    fprintf(file, ")");
  }

  fprintf(file, "\n");
}

const char *
dmnsn_token_string(dmnsn_token_type token_type)
{
#define TOKEN_SIZE 256
  static char token[TOKEN_SIZE];

  unsigned int i = YYTRANSLATE(token_type);
  if (i > YYNTOKENS) {
    fprintf(stderr, "Warning: unrecognised token %d.\n", (int)token_type);
    return "unrecognized-token";
  }

  if (yytnamerr(NULL, yytname[i]) >= TOKEN_SIZE) {
    fprintf(stderr, "Warning: name of token %d too long.\n", (int)token_type);
    return "unrepresentable-token";
  }

  yytnamerr(token, yytname[i]);
  return token;
#undef TOKEN_SIZE
}

const char *
dmnsn_astnode_string(dmnsn_astnode_type astnode_type)
{
  switch (astnode_type) {
  /* Macro to shorten this switch */
#define dmnsn_astnode_map(type, str)                                           \
  case type:                                                                   \
    return str;

  dmnsn_astnode_map(DMNSN_AST_FLOAT, "float");
  dmnsn_astnode_map(DMNSN_AST_INTEGER, "integer");
  dmnsn_astnode_map(DMNSN_AST_NEGATE, "-");
  dmnsn_astnode_map(DMNSN_AST_ADD, "+");
  dmnsn_astnode_map(DMNSN_AST_SUB, "-");
  dmnsn_astnode_map(DMNSN_AST_MUL, "*");
  dmnsn_astnode_map(DMNSN_AST_DIV, "/");
  dmnsn_astnode_map(DMNSN_AST_BOX, "box");
  dmnsn_astnode_map(DMNSN_AST_VECTOR, "vector");
  dmnsn_astnode_map(DMNSN_AST_SPHERE, "sphere");

  default:
    fprintf(stderr, "Warning: unrecognised astnode type %d.\n",
            (int)astnode_type);
    return "unrecognized-astnode";
  }
}
