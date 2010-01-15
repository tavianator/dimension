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

#define YYSTYPE dmnsn_parse_item
#define YYLTYPE dmnsn_parse_location

int dmnsn_yylex(YYSTYPE *lvalp, YYLTYPE *llocp, const char *filename,
                void *yyscanner);
void dmnsn_yylex_init(void **scannerp);
void dmnsn_yyset_in(FILE *file, void *scanner);
void dmnsn_yylex_destroy(void *scanner);

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

/* Create a new astnode, populating filename, line, and col */

static dmnsn_astnode
dmnsn_new_astnode(dmnsn_astnode_type type, YYLTYPE lloc)
{
  dmnsn_astnode astnode = {
    .type     = type,
    .children = dmnsn_new_array(sizeof(dmnsn_astnode)),
    .ptr      = NULL,
    .refcount = malloc(sizeof(unsigned int)),
    .filename = lloc.first_filename,
    .line     = lloc.first_line,
    .col      = lloc.first_column
  };

  if (!astnode.refcount) {
    dmnsn_error(DMNSN_SEVERITY_HIGH, "Couldn't allocate reference count.");
  }
  *astnode.refcount = 1;

  return astnode;
}

/* Semi-shallow copy */
static void
dmnsn_copy_children(dmnsn_astnode dest, dmnsn_astnode src)
{
  unsigned int i;
  for (i = 0; i < dmnsn_array_size(src.children); ++i) {
    dmnsn_astnode node;
    dmnsn_array_get(src.children, i, &node);
    ++*node.refcount;

    if (i < dmnsn_array_size(dest.children)) {
      dmnsn_astnode clobbered;
      dmnsn_array_get(dest.children, i, &clobbered);
      dmnsn_delete_astnode(clobbered);
    }

    dmnsn_array_set(dest.children, i, &node);
  }
}

static dmnsn_astnode
dmnsn_new_astnode1(dmnsn_astnode_type type, YYLTYPE lloc, dmnsn_astnode n1)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(type, lloc);
  dmnsn_array_push(astnode.children, &n1);
  return astnode;
}

static dmnsn_astnode
dmnsn_new_astnode2(dmnsn_astnode_type type, YYLTYPE lloc,
                   dmnsn_astnode n1, dmnsn_astnode n2)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(type, lloc);
  dmnsn_array_push(astnode.children, &n1);
  dmnsn_array_push(astnode.children, &n2);
  return astnode;
}

static dmnsn_astnode
dmnsn_new_astnode3(dmnsn_astnode_type type, YYLTYPE lloc,
                   dmnsn_astnode n1, dmnsn_astnode n2, dmnsn_astnode n3)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(type, lloc);
  dmnsn_array_push(astnode.children, &n1);
  dmnsn_array_push(astnode.children, &n2);
  dmnsn_array_push(astnode.children, &n3);
  return astnode;
}

static dmnsn_astnode
dmnsn_new_astnode4(dmnsn_astnode_type type, YYLTYPE lloc,
                   dmnsn_astnode n1, dmnsn_astnode n2, dmnsn_astnode n3,
                   dmnsn_astnode n4)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(type, lloc);
  dmnsn_array_push(astnode.children, &n1);
  dmnsn_array_push(astnode.children, &n2);
  dmnsn_array_push(astnode.children, &n3);
  dmnsn_array_push(astnode.children, &n4);
  return astnode;
}

static dmnsn_astnode
dmnsn_new_astnode5(dmnsn_astnode_type type, YYLTYPE lloc,
                   dmnsn_astnode n1, dmnsn_astnode n2, dmnsn_astnode n3,
                   dmnsn_astnode n4, dmnsn_astnode n5)
{
  dmnsn_astnode astnode = dmnsn_new_astnode(type, lloc);
  dmnsn_array_push(astnode.children, &n1);
  dmnsn_array_push(astnode.children, &n2);
  dmnsn_array_push(astnode.children, &n3);
  dmnsn_array_push(astnode.children, &n4);
  dmnsn_array_push(astnode.children, &n5);
  return astnode;
}

void
yyerror(YYLTYPE *locp, const char *filename, void *yyscanner,
        dmnsn_astree *astree, dmnsn_symbol_table *symtable, const char *str)
{
  dmnsn_diagnostic(locp->first_filename, locp->first_line, locp->first_column,
                   "%s", str);
}
%}

%define api.pure
%locations
%error-verbose
%token-table

%name-prefix "dmnsn_yy"

%expect 10
%glr-parser

%parse-param {const char *filename}
%parse-param {void *yyscanner}
%parse-param {dmnsn_astree *astree}
%parse-param {dmnsn_symbol_table *symtable}
%lex-param {const char *filename}
%lex-param {void *yyscanner}

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

/* Operators */
%left "+" "-"
%left "*" "/"
%left "."
%left DMNSN_T_NEGATE

/* Numeric values */
%token <value> DMNSN_T_INTEGER "integer"
%token <value> DMNSN_T_FLOAT   "float"

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
%token DMNSN_T_AMBIENT                  "ambient"
%token DMNSN_T_AMBIENT_LIGHT
%token DMNSN_T_ANGLE                    "angle"
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
%token DMNSN_T_BACKGROUND               "background"
%token DMNSN_T_BEZIER_SPLINE
%token DMNSN_T_BICUBIC_PATCH
%token DMNSN_T_BLACK_HOLE
%token DMNSN_T_BLOB
%token DMNSN_T_BLUE                     "blue"
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
%token DMNSN_T_DIFFUSE                  "diffuse"
%token DMNSN_T_DIMENSION_SIZE
%token DMNSN_T_DIMENSIONS
%token DMNSN_T_DIRECTION                "direction"
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
%token DMNSN_T_FALLOFF                  "falloff"
%token DMNSN_T_FALLOFF_ANGLE
%token DMNSN_T_FALSE
%token DMNSN_T_FILE_EXISTS
%token DMNSN_T_FILTER                   "filter"
%token DMNSN_T_FINAL_CLOCK
%token DMNSN_T_FINAL_FRAME
%token DMNSN_T_FINISH                   "finish"
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
%token DMNSN_T_GRAY                     "gray"
%token DMNSN_T_GRAY_THRESHOLD
%token DMNSN_T_GREEN                    "green"
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
%token DMNSN_T_LIGHT_SOURCE             "light_source"
%token DMNSN_T_LINEAR_SPLINE
%token DMNSN_T_LINEAR_SWEEP
%token DMNSN_T_LN
%token DMNSN_T_LOAD_FILE
%token DMNSN_T_LOCATION                 "location"
%token DMNSN_T_LOG
%token DMNSN_T_LOOK_AT                  "look_at"
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
%token DMNSN_T_PERSPECTIVE              "perspective"
%token DMNSN_T_PGM
%token DMNSN_T_PHASE
%token DMNSN_T_PHONG                    "phong"
%token DMNSN_T_PHONG_SIZE               "phong_size"
%token DMNSN_T_PHOTONS
%token DMNSN_T_PI
%token DMNSN_T_PIGMENT                  "pigment"
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
%token DMNSN_T_RED                      "red"
%token DMNSN_T_REFLECTION               "reflection"
%token DMNSN_T_REFLECTION_EXPONENT
%token DMNSN_T_REFRACTION
%token DMNSN_T_REPEAT
%token DMNSN_T_RGB                      "rgb"
%token DMNSN_T_RGBF                     "rgbf"
%token DMNSN_T_RGBFT                    "rgbft"
%token DMNSN_T_RGBT                     "rgbt"
%token DMNSN_T_RIGHT                    "right"
%token DMNSN_T_RIPPLES
%token DMNSN_T_ROTATE                   "rotate"
%token DMNSN_T_ROUGHNESS
%token DMNSN_T_SAMPLES
%token DMNSN_T_SAVE_FILE
%token DMNSN_T_SCALE                    "scale"
%token DMNSN_T_SCALLOP_WAVE
%token DMNSN_T_SCATTERING
%token DMNSN_T_SEED
%token DMNSN_T_SELECT
%token DMNSN_T_SHADOWLESS
%token DMNSN_T_SIN
%token DMNSN_T_SINE_WAVE
%token DMNSN_T_SINH
%token DMNSN_T_SIZE
%token DMNSN_T_SKY                      "sky"
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
%token DMNSN_T_T                        "t"
%token DMNSN_T_TAN
%token DMNSN_T_TANH
%token DMNSN_T_TARGET
%token DMNSN_T_TEXT
%token DMNSN_T_TEXTURE                  "texture"
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
%token DMNSN_T_TRANSLATE                "translate"
%token DMNSN_T_TRANSMIT                 "transmit"
%token DMNSN_T_TRIANGLE
%token DMNSN_T_TRIANGLE_WAVE
%token DMNSN_T_TRUE
%token DMNSN_T_TTF
%token DMNSN_T_TURB_DEPTH
%token DMNSN_T_TURBULENCE
%token DMNSN_T_TYPE
%token DMNSN_T_U                        "u"
%token DMNSN_T_U_STEPS
%token DMNSN_T_ULTRA_WIDE_ANGLE
%token DMNSN_T_UNION
%token DMNSN_T_UP                       "up"
%token DMNSN_T_USE_ALPHA
%token DMNSN_T_USE_COLOR
%token DMNSN_T_USE_INDEX
%token DMNSN_T_UTF8
%token DMNSN_T_UV_INDICES
%token DMNSN_T_UV_MAPPING
%token DMNSN_T_UV_VECTORS
%token DMNSN_T_V                        "v"
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
%token DMNSN_T_X                        "x"
%token DMNSN_T_Y                        "y"
%token DMNSN_T_YES
%token DMNSN_T_Z                        "z"

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
%token DMNSN_T_LOCAL      "#local"
%token DMNSN_T_MACRO
%token DMNSN_T_RANGE
%token DMNSN_T_READ
%token DMNSN_T_RENDER
%token DMNSN_T_STATISTICS
%token DMNSN_T_SWITCH
%token DMNSN_T_UNDEF      "#undef"
%token DMNSN_T_VERSION
%token DMNSN_T_WARNING
%token DMNSN_T_WHILE
%token DMNSN_T_WRITE

/* Identifiers */
%token <value> DMNSN_T_IDENTIFIER "identifier"

/* Strings */
%token <value> DMNSN_T_STRING "string"

/* Top-level items */
%type <astnode> SCENE_ITEM

/* Language directives */
%type <astnode> RVALUE
%type <astnode> IDENTIFIER

/* Transformations */
%type <astnode> TRANSFORMATION

/* The camera */
%type <astnode> CAMERA
%type <astnode> CAMERA_ITEMS
%type <astnode> CAMERA_ITEM
%type <astnode> CAMERA_TYPE
%type <astnode> CAMERA_VECTOR
%type <astnode> CAMERA_MODIFIER

/* Atmospheric effects */
%type <astnode> ATMOSPHERIC_EFFECT
%type <astnode> BACKGROUND

/* Objects */
%type <astnode> OBJECT
%type <astnode> FINITE_SOLID_OBJECT
%type <astnode> BOX
%type <astnode> LIGHT_SOURCE
%type <astnode> SPHERE

/* Object modifiers */
%type <astnode> OBJECT_MODIFIERS
%type <astnode> OBJECT_MODIFIER

/* Textures */
%type <astnode> TEXTURE
%type <astnode> TEXTURE_ITEMS

/* Pigments */
%type <astnode> PIGMENT
%type <astnode> PIGMENT_TYPE

/* Finishes */
%type <astnode> FINISH
%type <astnode> FINISH_ITEMS
%type <astnode> REFLECTION
%type <astnode> REFLECTION_ITEMS

/* Floats */
%type <astnode> FLOAT
%type <astnode> FLOAT_LITERAL

/* Vectors */
%type <astnode> VECTOR
%type <astnode> VECTOR_LITERAL

/* Generalized arithmetic expressions */
%type <astnode> ARITH_EXPR

/* Colors */
%type <astnode> COLOR
%type <astnode> COLOR_BODY
%type <astnode> COLOR_VECTOR
%type <astnode> COLOR_KEYWORD_GROUP
%type <astnode> COLOR_KEYWORD_GROUP_INIT

%destructor { free($$); } <value>
%destructor { dmnsn_delete_astnode($$); } <astnode>

%%

/* Start symbol */

SCENE: /* empty */
     | SCENE LANGUAGE_DIRECTIVE
     | SCENE SCENE_ITEM {
       dmnsn_array_push(astree, &$2);
     }
;

/* Language directives */

LANGUAGE_DIRECTIVE: DECLARATION
;

DECLARATION: "#declare" "identifier" "=" RVALUE {
             dmnsn_declare_symbol(symtable, $2, $4);
             free($2);
             dmnsn_delete_astnode($4);
           }
           | "#local" "identifier" "=" RVALUE {
             dmnsn_local_symbol(symtable, $2, $4);
             free($2);
             dmnsn_delete_astnode($4);
           }
           | "#undef" "identifier" {
             dmnsn_undef_symbol(symtable, $2);
             free($2);
           }
;

RVALUE: ARITH_EXPR ";" %dprec 2 {
        $$ = dmnsn_eval($1, symtable);
        dmnsn_delete_astnode($1);

        if ($$.type == DMNSN_AST_NONE) {
          dmnsn_delete_astnode($$);
          YYERROR;
        }
      }
      | COLOR ";" %dprec 1
;

IDENTIFIER: "identifier" {
            $$ = dmnsn_new_astnode(DMNSN_AST_IDENTIFIER, @$);
            $$.ptr = $1;
          }
;

/* Top-level scene item */
SCENE_ITEM: ATMOSPHERIC_EFFECT
          | CAMERA
          | OBJECT
;

/* Transformations */

TRANSFORMATION: "rotate" VECTOR {
                $$ = dmnsn_new_astnode1(DMNSN_AST_ROTATION, @$, $2);
              }
              | "scale" VECTOR {
                $$ = dmnsn_new_astnode1(DMNSN_AST_SCALE, @$, $2);
              }
              | "translate" VECTOR {
                $$ = dmnsn_new_astnode1(DMNSN_AST_TRANSLATION, @$, $2);
              }
;

/* Cameras */

CAMERA: "camera" "{"
          CAMERA_ITEMS
        "}"
      {
        $$ = $3;
      }
;

CAMERA_ITEMS: /* empty */ {
              $$ = dmnsn_new_astnode(DMNSN_AST_CAMERA, @$);
            }
            | CAMERA_ITEMS CAMERA_ITEM {
              $$ = $1;
              dmnsn_array_push($$.children, &$2);
            }
;

CAMERA_ITEM: CAMERA_TYPE
           | CAMERA_VECTOR
           | CAMERA_MODIFIER
;

CAMERA_TYPE: "perspective" {
             $$ = dmnsn_new_astnode(DMNSN_AST_PERSPECTIVE, @$);
           }
;

CAMERA_VECTOR: "location" VECTOR {
               $$ = dmnsn_new_astnode1(DMNSN_AST_LOCATION, @$, $2);
             }
             | "right" VECTOR {
               $$ = dmnsn_new_astnode1(DMNSN_AST_RIGHT, @$, $2);
             }
             | "up" VECTOR {
               $$ = dmnsn_new_astnode1(DMNSN_AST_UP, @$, $2);
             }
             | "sky" VECTOR {
               $$ = dmnsn_new_astnode1(DMNSN_AST_SKY, @$, $2);
             }
             | "direction" VECTOR {
               $$ = dmnsn_new_astnode1(DMNSN_AST_DIRECTION, @$, $2);
             }
;

CAMERA_MODIFIER: "angle" FLOAT {
                 $$ = dmnsn_new_astnode1(DMNSN_AST_ANGLE, @$, $2);
               }
               | "look_at" VECTOR {
                 $$ = dmnsn_new_astnode1(DMNSN_AST_LOOK_AT, @$, $2);
               }
               | TRANSFORMATION
;

/* Atmospheric effects */

ATMOSPHERIC_EFFECT: BACKGROUND
;

BACKGROUND: "background" "{" COLOR "}" {
            $$ = dmnsn_new_astnode1(DMNSN_AST_BACKGROUND, @$, $3);
          }
;

/* Objects */

OBJECT: FINITE_SOLID_OBJECT
      | LIGHT_SOURCE
;

FINITE_SOLID_OBJECT: BOX
                   | SPHERE
;

BOX: "box" "{"
       VECTOR "," VECTOR
       OBJECT_MODIFIERS
     "}"
   {
     $$ = dmnsn_new_astnode3(DMNSN_AST_BOX, @$, $3, $5, $6);
   }
;

LIGHT_SOURCE: "light_source" "{"
                VECTOR "," COLOR
              "}"
            {
              $$ = dmnsn_new_astnode2(DMNSN_AST_LIGHT_SOURCE, @$, $3, $5);
            }
;

SPHERE: "sphere" "{"
          VECTOR "," FLOAT
          OBJECT_MODIFIERS
        "}"
      {
        $$ = dmnsn_new_astnode3(DMNSN_AST_SPHERE, @$, $3, $5, $6);
      }
;

/* Object modifiers */

OBJECT_MODIFIERS: /* empty */ {
                  $$ = dmnsn_new_astnode(DMNSN_AST_OBJECT_MODIFIERS, @$);
                }
                | OBJECT_MODIFIERS OBJECT_MODIFIER {
                  $$ = $1;
                  dmnsn_array_push($$.children, &$2);
                }
;

OBJECT_MODIFIER: TRANSFORMATION
               | TEXTURE
               | PIGMENT {
                 $$ = dmnsn_new_astnode1(DMNSN_AST_TEXTURE, @$, $1);
               }
;

/* Textures */

TEXTURE: "texture" "{"
           TEXTURE_ITEMS
         "}"
       { $$ = $3; }
;

TEXTURE_ITEMS: /* empty */ {
               $$ = dmnsn_new_astnode(DMNSN_AST_TEXTURE, @$);
             }
             | TEXTURE_ITEMS PIGMENT {
               $$ = $1;
               dmnsn_array_push($$.children, &$2);
             }
             | TEXTURE_ITEMS FINISH {
               $$ = $1;
               dmnsn_array_push($$.children, &$2);
             }
;

/* Pigments */

PIGMENT: "pigment" "{"
           PIGMENT_TYPE
         "}"
       {
         $$ = dmnsn_new_astnode1(DMNSN_AST_PIGMENT, @$, $3);
       }
;

PIGMENT_TYPE: /* empty */ {
              $$ = dmnsn_new_astnode(DMNSN_AST_NONE, @$);
            }
            | COLOR
;

/* Finishes */
FINISH: "finish" "{"
          FINISH_ITEMS
        "}"
      { $$ = $3; }
;

FINISH_ITEMS: /* empty */ {
               $$ = dmnsn_new_astnode(DMNSN_AST_FINISH, @$);
            }
            | FINISH_ITEMS "ambient" COLOR {
              dmnsn_astnode ambient = dmnsn_new_astnode1(DMNSN_AST_AMBIENT,
                                                         @2, $3);
              $$ = $1;
              dmnsn_array_push($$.children, &ambient);
            }
            | FINISH_ITEMS "diffuse" FLOAT {
              dmnsn_astnode diffuse = dmnsn_new_astnode1(DMNSN_AST_DIFFUSE,
                                                         @2, $3);
              $$ = $1;
              dmnsn_array_push($$.children, &diffuse);
            }
            | FINISH_ITEMS "phong" FLOAT {
              dmnsn_astnode phong = dmnsn_new_astnode1(DMNSN_AST_PHONG, @2, $3);
              $$ = $1;
              dmnsn_array_push($$.children, &phong);
            }
            | FINISH_ITEMS "phong_size" FLOAT {
              dmnsn_astnode phong_size
                = dmnsn_new_astnode1(DMNSN_AST_PHONG_SIZE, @2, $3);
              $$ = $1;
              dmnsn_array_push($$.children, &phong_size);
            }
            | FINISH_ITEMS REFLECTION {
              $$ = $1;
              dmnsn_array_push($$.children, &$2);
            }
;

REFLECTION: "reflection" "{"
              COLOR
              REFLECTION_ITEMS
            "}"
          {
            ++*$3.refcount;
            $$ = dmnsn_new_astnode3(DMNSN_AST_REFLECTION, @$, $3, $3, $4);
          }
          | "reflection" "{"
              COLOR "," COLOR
              REFLECTION_ITEMS
            "}"
          {
            $$ = dmnsn_new_astnode3(DMNSN_AST_REFLECTION, @$, $3, $5, $6);
          }
;

REFLECTION_ITEMS: /* empty */ {
                  $$ = dmnsn_new_astnode(DMNSN_AST_REFLECTION_ITEMS, @$);
                }
                | REFLECTION_ITEMS "falloff" FLOAT {
                  dmnsn_astnode falloff
                    = dmnsn_new_astnode1(DMNSN_AST_FALLOFF, @2, $3);
                  $$ = $1;
                  dmnsn_array_push($$.children, &falloff);
                }
;

/* Floats */

FLOAT: ARITH_EXPR {
       $$ = dmnsn_eval_scalar($1, symtable);
       dmnsn_delete_astnode($1);

       if ($$.type == DMNSN_AST_NONE) {
         dmnsn_delete_astnode($$);
         YYERROR;
       }
     }
;

FLOAT_LITERAL: "integer" {
               $$ = dmnsn_new_astnode(DMNSN_AST_INTEGER, @$);
               $$.ptr = malloc(sizeof(long));
               if (!$$.ptr)
                 dmnsn_error(DMNSN_SEVERITY_HIGH,
                             "Failed to allocate room for integer.");

               *(long *)$$.ptr = strtol($1, NULL, 0);
               free($1);
             }
             | "float" {
               $$ = dmnsn_new_astnode(DMNSN_AST_FLOAT, @$);
               $$.ptr = malloc(sizeof(double));
               if (!$$.ptr)
                 dmnsn_error(DMNSN_SEVERITY_HIGH,
                             "Failed to allocate room for float.");

               *(double *)$$.ptr = strtod($1, NULL);
               free($1);
             }
;

/* Vectors */

VECTOR: ARITH_EXPR {
        $$ = dmnsn_eval_vector($1, symtable);
        dmnsn_delete_astnode($1);

        if ($$.type == DMNSN_AST_NONE) {
          dmnsn_delete_astnode($$);
          YYERROR;
        }
      }
;

VECTOR_LITERAL: "<" ARITH_EXPR "," ARITH_EXPR ">" {
                $$ = dmnsn_new_astnode2(DMNSN_AST_VECTOR, @$, $2, $4);
              }
              |  "<" ARITH_EXPR "," ARITH_EXPR "," ARITH_EXPR ">" {
                $$ = dmnsn_new_astnode3(DMNSN_AST_VECTOR, @$, $2, $4, $6);
              }
              |  "<" ARITH_EXPR "," ARITH_EXPR "," ARITH_EXPR ","
                     ARITH_EXPR ">" {
                $$ = dmnsn_new_astnode4(DMNSN_AST_VECTOR, @$, $2, $4, $6, $8);
              }
              |  "<" ARITH_EXPR "," ARITH_EXPR "," ARITH_EXPR ","
                     ARITH_EXPR "," ARITH_EXPR ">" {
                $$ = dmnsn_new_astnode5(DMNSN_AST_VECTOR, @$,
                                        $2, $4, $6, $8, $10);
              }
;

/* Generalized arithmetic expressions */

ARITH_EXPR: FLOAT_LITERAL
          | VECTOR_LITERAL
          | ARITH_EXPR "+" ARITH_EXPR {
            $$ = dmnsn_new_astnode2(DMNSN_AST_ADD, @$, $1, $3);
          }
          | ARITH_EXPR "-" ARITH_EXPR {
            $$ = dmnsn_new_astnode2(DMNSN_AST_SUB, @$, $1, $3);
          }
          | ARITH_EXPR "*" ARITH_EXPR {
            $$ = dmnsn_new_astnode2(DMNSN_AST_MUL, @$, $1, $3);
          }
          | ARITH_EXPR "/" ARITH_EXPR {
            $$ = dmnsn_new_astnode2(DMNSN_AST_DIV, @$, $1, $3);
          }
          | "+" ARITH_EXPR %prec DMNSN_T_NEGATE { $$ = $2; }
          | "-" ARITH_EXPR %prec DMNSN_T_NEGATE {
            $$ = dmnsn_new_astnode1(DMNSN_AST_NEGATE, @$, $2);
          }
          | ARITH_EXPR "." "x" {
            $$ = dmnsn_new_astnode1(DMNSN_AST_DOT_X, @$, $1);
          }
          | ARITH_EXPR "." "u" {
            $$ = dmnsn_new_astnode1(DMNSN_AST_DOT_X, @$, $1);
          }
          | ARITH_EXPR "." "red" {
            $$ = dmnsn_new_astnode1(DMNSN_AST_DOT_X, @$, $1);
          }
          | ARITH_EXPR "." "y" {
            $$ = dmnsn_new_astnode1(DMNSN_AST_DOT_Y, @$, $1);
          }
          | ARITH_EXPR "." "v" {
            $$ = dmnsn_new_astnode1(DMNSN_AST_DOT_Y, @$, $1);
          }
          | ARITH_EXPR "." "green" {
            $$ = dmnsn_new_astnode1(DMNSN_AST_DOT_Y, @$, $1);
          }
          | ARITH_EXPR "." "z" {
            $$ = dmnsn_new_astnode1(DMNSN_AST_DOT_Z, @$, $1);
          }
          | ARITH_EXPR "." "blue" {
            $$ = dmnsn_new_astnode1(DMNSN_AST_DOT_Z, @$, $1);
          }
          | ARITH_EXPR "." "t" {
            $$ = dmnsn_new_astnode1(DMNSN_AST_DOT_T, @$, $1);
          }
          | ARITH_EXPR "." "filter" {
            $$ = dmnsn_new_astnode1(DMNSN_AST_DOT_T, @$, $1);
          }
          | ARITH_EXPR "." "transmit" {
            $$ = dmnsn_new_astnode1(DMNSN_AST_DOT_TRANSMIT, @$, $1);
          }
          | "(" ARITH_EXPR ")" { $$ = $2; }
          | IDENTIFIER
;

/* Colors */

COLOR: COLOR_BODY {
       $$ = $1;
       $$.type = DMNSN_AST_COLOR;
     }
     | "color" COLOR_BODY {
       $$ = $2;
       $$.type = DMNSN_AST_COLOR;
     }
;

COLOR_BODY: COLOR_VECTOR        %dprec 1
          | COLOR_KEYWORD_GROUP %dprec 2
;

COLOR_VECTOR: "rgb" VECTOR {
              dmnsn_astnode f, t;
              dmnsn_array_get($2.children, 3, &f);
              dmnsn_array_get($2.children, 4, &t);
              dmnsn_delete_astnode(f);
              dmnsn_delete_astnode(t);

              dmnsn_array_resize($2.children, 3);

              $$ = dmnsn_eval_vector($2, symtable);
              dmnsn_delete_astnode($2);
            }
            | "rgbf" VECTOR {
              dmnsn_astnode t;
              dmnsn_array_get($2.children, 4, &t);
              dmnsn_delete_astnode(t);

              dmnsn_array_resize($2.children, 4);

              $$ = dmnsn_eval_vector($2, symtable);
              dmnsn_delete_astnode($2);
            }
            | "rgbt" VECTOR {
              /* Chop off the fifth component */
              dmnsn_astnode t;
              dmnsn_array_get($2.children, 4, &t);
              dmnsn_delete_astnode(t);

              dmnsn_array_resize($2.children, 4);

              $$ = dmnsn_eval_vector($2, symtable);
              dmnsn_delete_astnode($2);

              /* Swap the transmit and filter components */
              dmnsn_astnode temp;
              dmnsn_array_get($$.children, 4, &temp);
              dmnsn_array_set($$.children, 4, dmnsn_array_at($$.children, 3));
              dmnsn_array_set($$.children, 3, &temp);
            }
            | "rgbft" VECTOR { $$ = $2; }
            | VECTOR
;

COLOR_KEYWORD_GROUP: COLOR_KEYWORD_GROUP_INIT COLOR_KEYWORD_ITEM
                   | COLOR_KEYWORD_GROUP      COLOR_KEYWORD_ITEM
;

COLOR_KEYWORD_GROUP_INIT: /* empty */ {
                          dmnsn_astnode zero =
                            dmnsn_new_astnode(DMNSN_AST_INTEGER, @$);
                          zero.ptr = malloc(sizeof(long));
                          if (!zero.ptr)
                            dmnsn_error(DMNSN_SEVERITY_HIGH,
                                        "Failed to allocate room for integer.");
                          *(long *)zero.ptr = 0;

                          $$ = dmnsn_eval_vector(zero, symtable);
                          dmnsn_delete_astnode(zero);
                        }
;

COLOR_KEYWORD_ITEM: "identifier" {
                    dmnsn_astnode *symbol = dmnsn_find_symbol(symtable, $1);
                    if (!symbol) {
                      dmnsn_diagnostic(@1.first_filename, @1.first_line,
                                       @1.first_column,
                                       "unbound identifier '%s'", $1);
                      free($1);
                      YYERROR;
                    } else if (symbol->type != DMNSN_AST_COLOR) {
                      dmnsn_astnode eval = dmnsn_eval_vector(*symbol, symtable);
                      if (eval.type == DMNSN_AST_NONE) {
                        free($1);
                        YYERROR;
                      }

                      dmnsn_copy_children($<astnode>0, eval);
                      dmnsn_delete_astnode(eval);
                    } else {
                      dmnsn_copy_children($<astnode>0, *symbol);
                    }

                    free($1);
                  }
                  | "red" FLOAT {
                    dmnsn_astnode old;
                    dmnsn_array_get($<astnode>0.children, 0, &old);
                    dmnsn_array_set($<astnode>0.children, 0, &$2);
                    dmnsn_delete_astnode(old);
                  }
                  | "green" FLOAT {
                    dmnsn_astnode old;
                    dmnsn_array_get($<astnode>0.children, 1, &old);
                    dmnsn_array_set($<astnode>0.children, 1, &$2);
                    dmnsn_delete_astnode(old);
                  }
                  | "blue" FLOAT {
                    dmnsn_astnode old;
                    dmnsn_array_get($<astnode>0.children, 2, &old);
                    dmnsn_array_set($<astnode>0.children, 2, &$2);
                    dmnsn_delete_astnode(old);
                  }
                  | "filter" FLOAT {
                    dmnsn_astnode old;
                    dmnsn_array_get($<astnode>0.children, 3, &old);
                    dmnsn_array_set($<astnode>0.children, 3, &$2);
                    dmnsn_delete_astnode(old);
                  }
                  | "transmit" FLOAT {
                    dmnsn_astnode old;
                    dmnsn_array_get($<astnode>0.children, 4, &old);
                    dmnsn_array_set($<astnode>0.children, 4, &$2);
                    dmnsn_delete_astnode(old);
                  }
;

%%

dmnsn_astree *
dmnsn_parse(FILE *file, dmnsn_symbol_table *symtable)
{
  const char *filename;
  dmnsn_astnode *fnode = dmnsn_find_symbol(symtable, "__file__");
  if (fnode && fnode->type == DMNSN_AST_STRING) {
    filename = fnode->ptr;
  } else {
    filename = "<>";
    dmnsn_declare_symbol(symtable, "__file__", dmnsn_new_ast_string(filename));
  }

  void *scanner;
  dmnsn_astree *astree = dmnsn_new_array(sizeof(dmnsn_astnode));

  dmnsn_yylex_init(&scanner);
  dmnsn_yyset_in(file, scanner);

  if (yyparse(filename, scanner, astree, symtable) != 0) {
    dmnsn_delete_astree(astree);
    astree = NULL;
  }

  dmnsn_yylex_destroy(scanner);
  return astree;
}

const char *
dmnsn_token_string(dmnsn_token_type token_type)
{
#define TOKEN_SIZE 255
  static char token[TOKEN_SIZE + 1];

  unsigned int i = YYTRANSLATE(token_type);
  if (i > YYNTOKENS) {
    fprintf(stderr, "Warning: unrecognised token %d.\n", (int)token_type);
    return "unrecognized-token";
  }

  /* Trim the quotation marks */

  if (strlen(yytname[i]) - 1 >= TOKEN_SIZE) {
    fprintf(stderr, "Warning: name of token %d too long.\n", (int)token_type);
    return "unrepresentable-token";
  }

  strcpy(token, yytname[i] + 1);
  token[strlen(token) - 1] = '\0';

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

  dmnsn_astnode_map(DMNSN_AST_NONE, "none");

  dmnsn_astnode_map(DMNSN_AST_ROTATION, "rotate");
  dmnsn_astnode_map(DMNSN_AST_SCALE, "scale");
  dmnsn_astnode_map(DMNSN_AST_TRANSLATION, "translate");

  dmnsn_astnode_map(DMNSN_AST_CAMERA, "camera");
  dmnsn_astnode_map(DMNSN_AST_PERSPECTIVE, "perspective");
  dmnsn_astnode_map(DMNSN_AST_LOCATION, "location");
  dmnsn_astnode_map(DMNSN_AST_RIGHT, "right");
  dmnsn_astnode_map(DMNSN_AST_UP, "up");
  dmnsn_astnode_map(DMNSN_AST_SKY, "sky");
  dmnsn_astnode_map(DMNSN_AST_ANGLE, "angle");
  dmnsn_astnode_map(DMNSN_AST_LOOK_AT, "look_at");
  dmnsn_astnode_map(DMNSN_AST_DIRECTION, "direction");

  dmnsn_astnode_map(DMNSN_AST_BACKGROUND, "background");

  dmnsn_astnode_map(DMNSN_AST_BOX, "box");
  dmnsn_astnode_map(DMNSN_AST_SPHERE, "sphere");
  dmnsn_astnode_map(DMNSN_AST_LIGHT_SOURCE, "light_source");

  dmnsn_astnode_map(DMNSN_AST_OBJECT_MODIFIERS, "object-modifiers");

  dmnsn_astnode_map(DMNSN_AST_TEXTURE, "texture");

  dmnsn_astnode_map(DMNSN_AST_PIGMENT, "pigment");

  dmnsn_astnode_map(DMNSN_AST_FINISH, "finish");
  dmnsn_astnode_map(DMNSN_AST_AMBIENT, "ambient");
  dmnsn_astnode_map(DMNSN_AST_DIFFUSE, "diffuse");
  dmnsn_astnode_map(DMNSN_AST_PHONG, "phong");
  dmnsn_astnode_map(DMNSN_AST_PHONG_SIZE, "phong_size");

  dmnsn_astnode_map(DMNSN_AST_REFLECTION, "reflection");
  dmnsn_astnode_map(DMNSN_AST_REFLECTION_ITEMS, "reflection-items");
  dmnsn_astnode_map(DMNSN_AST_FALLOFF, "falloff");

  dmnsn_astnode_map(DMNSN_AST_FLOAT, "float");
  dmnsn_astnode_map(DMNSN_AST_INTEGER, "integer");

  dmnsn_astnode_map(DMNSN_AST_VECTOR, "vector");

  dmnsn_astnode_map(DMNSN_AST_ADD, "+");
  dmnsn_astnode_map(DMNSN_AST_SUB, "-");
  dmnsn_astnode_map(DMNSN_AST_MUL, "*");
  dmnsn_astnode_map(DMNSN_AST_DIV, "/");

  dmnsn_astnode_map(DMNSN_AST_NEGATE, "-");
  dmnsn_astnode_map(DMNSN_AST_DOT_X, ".x");
  dmnsn_astnode_map(DMNSN_AST_DOT_Y, ".y");
  dmnsn_astnode_map(DMNSN_AST_DOT_Z, ".z");
  dmnsn_astnode_map(DMNSN_AST_DOT_T, ".t");
  dmnsn_astnode_map(DMNSN_AST_DOT_TRANSMIT, ".transmit");

  dmnsn_astnode_map(DMNSN_AST_COLOR, "color");

  dmnsn_astnode_map(DMNSN_AST_IDENTIFIER, "identifier");

  dmnsn_astnode_map(DMNSN_AST_STRING, "string");

  default:
    fprintf(stderr, "Warning: unrecognised astnode type %d.\n",
            (int)astnode_type);
    return "unrecognized-astnode";
  }
}
