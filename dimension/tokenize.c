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
#include "utility.h"
#include <stdlib.h>   /* For strtoul(), etc. */
#include <string.h>
#include <ctype.h>    /* For isalpha(), etc. */
#include <sys/mman.h> /* For mmap()          */
#include <libgen.h>   /* For dirname()       */
#include <locale.h>

static int
dmnsn_tokenize_comment(const char *filename,
                       unsigned int *linep, unsigned int *colp,
                       char *map, size_t size, char **nextp)
{
  unsigned int line = *linep, col = *colp;
  char *next = *nextp;

  if (*next != '/')
    return 1;

  if (*(next + 1) == '/') {
    /* A '//' comment block */
    do {
      ++next;
    } while (next - map < size && *next != '\n');

    ++next;
    ++line;
    col = 0;
  } else if (*(next + 1) == '*') {
    /* A multi-line comment block (like this one) */
    do {
      ++col;
      if (*next == '\n') {
        ++line;
        col = 0;
      }

      ++next;
    } while (next - map < size - 1
             && !(*next == '*' && *(next + 1) == '/'));
    next += 2;
  } else {
    return 1;
  }

  *linep = line;
  *colp  = col;
  *nextp = next;
  return 0;
}

static int
dmnsn_tokenize_number(const char *filename,
                      unsigned int *linep, unsigned int *colp,
                      char *map, size_t size, char **nextp, dmnsn_token *token)
{
  unsigned int line = *linep, col = *colp;
  char *next = *nextp;
  char *endf, *endi;

  strtoul(next, &endi, 0);
  strtod(next, &endf);
  if (endf > endi
      /* These next conditions catch invalid octal integers being parsed as
         floats, eg 08 */
      && (*endi == '.' || *endi == 'e' || *endi == 'E' || *endi == 'p'
          || *endi == 'P'))
  {
    token->type = DMNSN_T_FLOAT;
    token->value = malloc(endf - next + 1);
    strncpy(token->value, next, endf - next);
    token->value[endf - next] = '\0';

    col += endf - next;
    next = endf;
  } else if (endi > next) {
    token->type = DMNSN_T_INTEGER;
    token->value = malloc(endi - next + 1);
    strncpy(token->value, next, endi - next);
    token->value[endi - next] = '\0';

    col += endi - next;
    next = endi;
  } else {
    return 1;
  }

  *linep = line;
  *colp  = col;
  *nextp = next;
  return 0;
}

/* Tokenize a string */
static int
dmnsn_tokenize_string(const char *filename,
                      unsigned int *linep, unsigned int *colp,
                      char *map, size_t size, char **nextp, dmnsn_token *token)
{
  unsigned int line = *linep, col = *colp;
  char *next = *nextp;
  unsigned int i = 0, alloc = 32;
  char unicode[5] = { 0 }, *end;
  unsigned long wchar;

  if (*next != '"') {
    return 1;
  }
  
  token->type  = DMNSN_T_STRING;
  token->value = malloc(alloc);

  ++next;
  while (next - map < size && *next != '"') {
    if (i + 1 >= alloc) {
      alloc *= 2;
      token->value = realloc(token->value, alloc);
    }

    if (*next == '\\') {
      ++col;
      ++next;

      switch (*next) {
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
        strncpy(unicode, next + 1, 4);
        wchar = strtoul(unicode, &end, 16);
        if (next - map >= size - 4) {
          dmnsn_diagnostic(filename, line, col,
                           "EOF before end of escape sequence");
          free(token->value);
          return 1;
        }
        if (end != &unicode[4]) {
          dmnsn_diagnostic(filename, line, col,
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

          col  += 4;
          next += 4;
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
        dmnsn_diagnostic(filename, line, col,
                         "WARNING: unrecognised escape sequence '\\%c'",
                         (int)*next);
        token->value[i] = *next;
        break;
      }
    } else {
      token->value[i] = *next;
    }

    ++i;
    ++col;
    ++next;
  } 

  if (*next != '"') {
    dmnsn_diagnostic(filename, line, col, "Non-terminated string");
    free(token->value);
    return 1;
  }

  ++next;
  token->value[i] = '\0';

  *linep = line;
  *colp  = col;
  *nextp = next;
  return 0;
}

/* Tokenize a keyword or an identifier */
static int
dmnsn_tokenize_label(const char *filename,
                     unsigned int *linep, unsigned int *colp,
                     char *map, size_t size, char **nextp, dmnsn_token *token)
{
  unsigned int line = *linep, col = *colp;
  char *next = *nextp;
  unsigned int i = 0, alloc = 32;

  if (!isalpha(*next) && *next != '_') {
    return 1;
  }

  token->type  = DMNSN_T_IDENTIFIER;
  token->value = malloc(alloc);

  do {
    if (i + 1 >= alloc) {
      alloc *= 2;
      token->value = realloc(token->value, alloc);
    }

    token->value[i] = *next;

    ++i;
    ++col;
    ++next;
  } while (next - map < size && (isalnum(*next) || *next == '_'));

  token->value[i] = '\0';

  /* Now check if we really found a keyword */

#define dmnsn_keyword(str, tp)                                                 \
  do {                                                                         \
    if (strcmp(token->value, str) == 0) {                                      \
      free(token->value);                                                      \
      token->value = NULL;                                                     \
      token->type  = tp;                                                       \
                                                                               \
      *linep = line;                                                           \
      *colp  = col;                                                            \
      *nextp = next;                                                           \
      return 0;                                                                \
    }                                                                          \
  } while (0)

  dmnsn_keyword("aa_level", DMNSN_T_AA_LEVEL);
  dmnsn_keyword("aa_threshold", DMNSN_T_AA_THRESHOLD);
  dmnsn_keyword("abs", DMNSN_T_ABS);
  dmnsn_keyword("absorption", DMNSN_T_ABSORPTION);
  dmnsn_keyword("accuracy", DMNSN_T_ACCURACY);
  dmnsn_keyword("acos", DMNSN_T_ACOS);
  dmnsn_keyword("acosh", DMNSN_T_ACOSH);
  dmnsn_keyword("adaptive", DMNSN_T_ADAPTIVE);
  dmnsn_keyword("adc_bailout", DMNSN_T_ADC_BAILOUT);
  dmnsn_keyword("agate", DMNSN_T_AGATE);
  dmnsn_keyword("agate_turb", DMNSN_T_AGATE_TURB);
  dmnsn_keyword("all", DMNSN_T_ALL);
  dmnsn_keyword("all_intersections", DMNSN_T_ALL_INTERSECTIONS);
  dmnsn_keyword("alpha", DMNSN_T_ALPHA);
  dmnsn_keyword("altitude", DMNSN_T_ALTITUDE);
  dmnsn_keyword("always_sample", DMNSN_T_ALWAYS_SAMPLE);
  dmnsn_keyword("ambient", DMNSN_T_AMBIENT);
  dmnsn_keyword("ambient_light", DMNSN_T_AMBIENT_LIGHT);
  dmnsn_keyword("angle", DMNSN_T_ANGLE);
  dmnsn_keyword("aperture", DMNSN_T_APERTURE);
  dmnsn_keyword("append", DMNSN_T_APPEND);
  dmnsn_keyword("arc_angle", DMNSN_T_ARC_ANGLE);
  dmnsn_keyword("area_light", DMNSN_T_AREA_LIGHT);
  dmnsn_keyword("array", DMNSN_T_ARRAY);
  dmnsn_keyword("asc", DMNSN_T_ASC);
  dmnsn_keyword("ascii", DMNSN_T_ASCII);
  dmnsn_keyword("asin", DMNSN_T_ASIN);
  dmnsn_keyword("asinh", DMNSN_T_ASINH);
  dmnsn_keyword("assumed_gamma", DMNSN_T_ASSUMED_GAMMA);
  dmnsn_keyword("atan", DMNSN_T_ATAN);
  dmnsn_keyword("atan2", DMNSN_T_ATAN2);
  dmnsn_keyword("atanh", DMNSN_T_ATANH);
  dmnsn_keyword("autostop", DMNSN_T_AUTOSTOP);
  dmnsn_keyword("average", DMNSN_T_AVERAGE);
  dmnsn_keyword("b_spline", DMNSN_T_B_SPLINE);
  dmnsn_keyword("background", DMNSN_T_BACKGROUND);
  dmnsn_keyword("bezier_spline", DMNSN_T_BEZIER_SPLINE);
  dmnsn_keyword("bicubic_patch", DMNSN_T_BICUBIC_PATCH);
  dmnsn_keyword("black_hole", DMNSN_T_BLACK_HOLE);
  dmnsn_keyword("blob", DMNSN_T_BLOB);
  dmnsn_keyword("blue", DMNSN_T_BLUE);
  dmnsn_keyword("blur_samples", DMNSN_T_BLUR_SAMPLES);
  dmnsn_keyword("bounded_by", DMNSN_T_BOUNDED_BY);
  dmnsn_keyword("box", DMNSN_T_BOX);
  dmnsn_keyword("boxed", DMNSN_T_BOXED);
  dmnsn_keyword("bozo", DMNSN_T_BOZO);
  dmnsn_keyword("brick", DMNSN_T_BRICK);
  dmnsn_keyword("brick_size", DMNSN_T_BRICK_SIZE);
  dmnsn_keyword("brightness", DMNSN_T_BRIGHTNESS);
  dmnsn_keyword("brilliance", DMNSN_T_BRILLIANCE);
  dmnsn_keyword("bump_map", DMNSN_T_BUMP_MAP);
  dmnsn_keyword("bump_size", DMNSN_T_BUMP_SIZE);
  dmnsn_keyword("bumps", DMNSN_T_BUMPS);
  dmnsn_keyword("camera", DMNSN_T_CAMERA);
  dmnsn_keyword("caustics", DMNSN_T_CAUSTICS);
  dmnsn_keyword("ceil", DMNSN_T_CEIL);
  dmnsn_keyword("cells", DMNSN_T_CELLS);
  dmnsn_keyword("charset", DMNSN_T_CHARSET);
  dmnsn_keyword("checker", DMNSN_T_CHECKER);
  dmnsn_keyword("chr", DMNSN_T_CHR);
  dmnsn_keyword("circular", DMNSN_T_CIRCULAR);
  dmnsn_keyword("clipped_by", DMNSN_T_CLIPPED_BY);
  dmnsn_keyword("clock", DMNSN_T_CLOCK);
  dmnsn_keyword("clock_delta", DMNSN_T_CLOCK_DELTA);
  dmnsn_keyword("clock_on", DMNSN_T_CLOCK_ON);
  dmnsn_keyword("collect", DMNSN_T_COLLECT);
  dmnsn_keyword("color", DMNSN_T_COLOR);
  dmnsn_keyword("colour", DMNSN_T_COLOR);
  dmnsn_keyword("color_map", DMNSN_T_COLOR_MAP);
  dmnsn_keyword("colour_map", DMNSN_T_COLOR_MAP);
  dmnsn_keyword("component", DMNSN_T_COMPONENT);
  dmnsn_keyword("composite", DMNSN_T_COMPOSITE);
  dmnsn_keyword("concat", DMNSN_T_CONCAT);
  dmnsn_keyword("cone", DMNSN_T_CONE);
  dmnsn_keyword("confidence", DMNSN_T_CONFIDENCE);
  dmnsn_keyword("conic_sweep", DMNSN_T_CONIC_SWEEP);
  dmnsn_keyword("conserve_energy", DMNSN_T_CONSERVE_ENERGY);
  dmnsn_keyword("contained_by", DMNSN_T_CONTAINED_BY);
  dmnsn_keyword("control0", DMNSN_T_CONTROL0);
  dmnsn_keyword("control1", DMNSN_T_CONTROL1);
  dmnsn_keyword("coords", DMNSN_T_COORDS);
  dmnsn_keyword("cos", DMNSN_T_COS);
  dmnsn_keyword("cosh", DMNSN_T_COSH);
  dmnsn_keyword("count", DMNSN_T_COUNT);
  dmnsn_keyword("crackle", DMNSN_T_CRACKLE);
  dmnsn_keyword("crand", DMNSN_T_CRAND);
  dmnsn_keyword("cube", DMNSN_T_CUBE);
  dmnsn_keyword("cubic", DMNSN_T_CUBIC);
  dmnsn_keyword("cubic_spline", DMNSN_T_CUBIC_SPLINE);
  dmnsn_keyword("cubic_wave", DMNSN_T_CUBIC_WAVE);
  dmnsn_keyword("cutaway_textures", DMNSN_T_CUTAWAY_TEXTURES);
  dmnsn_keyword("cylinder", DMNSN_T_CYLINDER);
  dmnsn_keyword("cylindrical", DMNSN_T_CYLINDRICAL);
  dmnsn_keyword("defined", DMNSN_T_DEFINED);
  dmnsn_keyword("degrees", DMNSN_T_DEGREES);
  dmnsn_keyword("density", DMNSN_T_DENSITY);
  dmnsn_keyword("density_file", DMNSN_T_DENSITY_FILE);
  dmnsn_keyword("density_map", DMNSN_T_DENSITY_MAP);
  dmnsn_keyword("dents", DMNSN_T_DENTS);
  dmnsn_keyword("df3", DMNSN_T_DF3);
  dmnsn_keyword("difference", DMNSN_T_DIFFERENCE);
  dmnsn_keyword("diffuse", DMNSN_T_DIFFUSE);
  dmnsn_keyword("dimension_size", DMNSN_T_DIMENSION_SIZE);
  dmnsn_keyword("dimensions", DMNSN_T_DIMENSIONS);
  dmnsn_keyword("direction", DMNSN_T_DIRECTION);
  dmnsn_keyword("disc", DMNSN_T_DISC);
  dmnsn_keyword("dispersion", DMNSN_T_DISPERSION);
  dmnsn_keyword("dispersion_samples", DMNSN_T_DISPERSION_SAMPLES);
  dmnsn_keyword("dist_exp", DMNSN_T_DIST_EXP);
  dmnsn_keyword("distance", DMNSN_T_DISTANCE);
  dmnsn_keyword("div", DMNSN_T_DIV);
  dmnsn_keyword("double_illuminate", DMNSN_T_DOUBLE_ILLUMINATE);
  dmnsn_keyword("eccentricity", DMNSN_T_ECCENTRICITY);
  dmnsn_keyword("emission", DMNSN_T_EMISSION);
  dmnsn_keyword("error_bound", DMNSN_T_ERROR_BOUND);
  dmnsn_keyword("evaluate", DMNSN_T_EVALUATE);
  dmnsn_keyword("exp", DMNSN_T_EXP);
  dmnsn_keyword("expand_thresholds", DMNSN_T_EXPAND_THRESHOLDS);
  dmnsn_keyword("exponent", DMNSN_T_EXPONENT);
  dmnsn_keyword("exterior", DMNSN_T_EXTERIOR);
  dmnsn_keyword("extinction", DMNSN_T_EXTINCTION);
  dmnsn_keyword("face_indices", DMNSN_T_FACE_INDICES);
  dmnsn_keyword("facets", DMNSN_T_FACETS);
  dmnsn_keyword("fade_color", DMNSN_T_FADE_COLOR);
  dmnsn_keyword("fade_colour", DMNSN_T_FADE_COLOR);
  dmnsn_keyword("fade_distance", DMNSN_T_FADE_DISTANCE);
  dmnsn_keyword("fade_power", DMNSN_T_FADE_POWER);
  dmnsn_keyword("falloff", DMNSN_T_FALLOFF);
  dmnsn_keyword("falloff_angle", DMNSN_T_FALLOFF_ANGLE);
  dmnsn_keyword("false", DMNSN_T_FALSE);
  dmnsn_keyword("file_exists", DMNSN_T_FILE_EXISTS);
  dmnsn_keyword("filter", DMNSN_T_FILTER);
  dmnsn_keyword("final_clock", DMNSN_T_FINAL_CLOCK);
  dmnsn_keyword("final_frame", DMNSN_T_FINAL_FRAME);
  dmnsn_keyword("finish", DMNSN_T_FINISH);
  dmnsn_keyword("fisheye", DMNSN_T_FISHEYE);
  dmnsn_keyword("flatness", DMNSN_T_FLATNESS);
  dmnsn_keyword("flip", DMNSN_T_FLIP);
  dmnsn_keyword("floor", DMNSN_T_FLOOR);
  dmnsn_keyword("focal_point", DMNSN_T_FOCAL_POINT);
  dmnsn_keyword("fog", DMNSN_T_FOG);
  dmnsn_keyword("fog_alt", DMNSN_T_FOG_ALT);
  dmnsn_keyword("fog_offset", DMNSN_T_FOG_OFFSET);
  dmnsn_keyword("fog_type", DMNSN_T_FOG_TYPE);
  dmnsn_keyword("form", DMNSN_T_FORM);
  dmnsn_keyword("frame_number", DMNSN_T_FRAME_NUMBER);
  dmnsn_keyword("frequency", DMNSN_T_FREQUENCY);
  dmnsn_keyword("fresnel", DMNSN_T_FRESNEL);
  dmnsn_keyword("function", DMNSN_T_FUNCTION);
  dmnsn_keyword("gather", DMNSN_T_GATHER);
  dmnsn_keyword("gif", DMNSN_T_GIF);
  dmnsn_keyword("global_lights", DMNSN_T_GLOBAL_LIGHTS);
  dmnsn_keyword("global_settings", DMNSN_T_GLOBAL_SETTINGS);
  dmnsn_keyword("gradient", DMNSN_T_GRADIENT);
  dmnsn_keyword("granite", DMNSN_T_GRANITE);
  dmnsn_keyword("gray", DMNSN_T_GRAY);
  dmnsn_keyword("gray_threshold", DMNSN_T_GRAY_THRESHOLD);
  dmnsn_keyword("green", DMNSN_T_GREEN);
  dmnsn_keyword("height_field", DMNSN_T_HEIGHT_FIELD);
  dmnsn_keyword("hexagon", DMNSN_T_HEXAGON);
  dmnsn_keyword("hf_gray_16", DMNSN_T_HF_GRAY_16);
  dmnsn_keyword("hierarchy", DMNSN_T_HIERARCHY);
  dmnsn_keyword("hypercomplex", DMNSN_T_HYPERCOMPLEX);
  dmnsn_keyword("hollow", DMNSN_T_HOLLOW);
  dmnsn_keyword("iff", DMNSN_T_IFF);
  dmnsn_keyword("image_height", DMNSN_T_IMAGE_HEIGHT);
  dmnsn_keyword("image_map", DMNSN_T_IMAGE_MAP);
  dmnsn_keyword("image_pattern", DMNSN_T_IMAGE_PATTERN);
  dmnsn_keyword("image_width", DMNSN_T_IMAGE_WIDTH);
  dmnsn_keyword("initial_clock", DMNSN_T_INITIAL_CLOCK);
  dmnsn_keyword("initial_frame", DMNSN_T_INITIAL_FRAME);
  dmnsn_keyword("inside", DMNSN_T_INSIDE);
  dmnsn_keyword("inside_vector", DMNSN_T_INSIDE_VECTOR);
  dmnsn_keyword("int", DMNSN_T_INT);
  dmnsn_keyword("interior", DMNSN_T_INTERIOR);
  dmnsn_keyword("interior_texture", DMNSN_T_INTERIOR_TEXTURE);
  dmnsn_keyword("internal", DMNSN_T_INTERNAL);
  dmnsn_keyword("interpolate", DMNSN_T_INTERPOLATE);
  dmnsn_keyword("intersection", DMNSN_T_INTERSECTION);
  dmnsn_keyword("intervals", DMNSN_T_INTERVALS);
  dmnsn_keyword("inverse", DMNSN_T_INVERSE);
  dmnsn_keyword("ior", DMNSN_T_IOR);
  dmnsn_keyword("irid", DMNSN_T_IRID);
  dmnsn_keyword("irid_wavelength", DMNSN_T_IRID_WAVELENGTH);
  dmnsn_keyword("isosurface", DMNSN_T_ISOSURFACE);
  dmnsn_keyword("jitter", DMNSN_T_JITTER);
  dmnsn_keyword("jpeg", DMNSN_T_JPEG);
  dmnsn_keyword("julia", DMNSN_T_JULIA);
  dmnsn_keyword("julia_fractal", DMNSN_T_JULIA_FRACTAL);
  dmnsn_keyword("lambda", DMNSN_T_LAMBDA);
  dmnsn_keyword("lathe", DMNSN_T_LATHE);
  dmnsn_keyword("leopard", DMNSN_T_LEOPARD);
  dmnsn_keyword("light_group", DMNSN_T_LIGHT_GROUP);
  dmnsn_keyword("light_source", DMNSN_T_LIGHT_SOURCE);
  dmnsn_keyword("linear_spline", DMNSN_T_LINEAR_SPLINE);
  dmnsn_keyword("linear_sweep", DMNSN_T_LINEAR_SWEEP);
  dmnsn_keyword("ln", DMNSN_T_LN);
  dmnsn_keyword("load_file", DMNSN_T_LOAD_FILE);
  dmnsn_keyword("location", DMNSN_T_LOCATION);
  dmnsn_keyword("log", DMNSN_T_LOG);
  dmnsn_keyword("look_at", DMNSN_T_LOOK_AT);
  dmnsn_keyword("looks_like", DMNSN_T_LOOKS_LIKE);
  dmnsn_keyword("low_error_factor", DMNSN_T_LOW_ERROR_FACTOR);
  dmnsn_keyword("magnet", DMNSN_T_MAGNET);
  dmnsn_keyword("major_radius", DMNSN_T_MAJOR_RADIUS);
  dmnsn_keyword("mandel", DMNSN_T_MANDEL);
  dmnsn_keyword("map_type", DMNSN_T_MAP_TYPE);
  dmnsn_keyword("marble", DMNSN_T_MARBLE);
  dmnsn_keyword("material", DMNSN_T_MATERIAL);
  dmnsn_keyword("material_map", DMNSN_T_MATERIAL_MAP);
  dmnsn_keyword("matrix", DMNSN_T_MATRIX);
  dmnsn_keyword("max", DMNSN_T_MAX);
  dmnsn_keyword("max_extent", DMNSN_T_MAX_EXTENT);
  dmnsn_keyword("max_gradient", DMNSN_T_MAX_GRADIENT);
  dmnsn_keyword("max_intersections", DMNSN_T_MAX_INTERSECTIONS);
  dmnsn_keyword("max_iteration", DMNSN_T_MAX_ITERATION);
  dmnsn_keyword("max_sample", DMNSN_T_MAX_SAMPLE);
  dmnsn_keyword("max_trace", DMNSN_T_MAX_TRACE);
  dmnsn_keyword("max_trace_level", DMNSN_T_MAX_TRACE_LEVEL);
  dmnsn_keyword("media", DMNSN_T_MEDIA);
  dmnsn_keyword("media_attenuation", DMNSN_T_MEDIA_ATTENUATION);
  dmnsn_keyword("media_interaction", DMNSN_T_MEDIA_INTERACTION);
  dmnsn_keyword("merge", DMNSN_T_MERGE);
  dmnsn_keyword("mesh", DMNSN_T_MESH);
  dmnsn_keyword("mesh2", DMNSN_T_MESH2);
  dmnsn_keyword("metallic", DMNSN_T_METALLIC);
  dmnsn_keyword("method", DMNSN_T_METHOD);
  dmnsn_keyword("metric", DMNSN_T_METRIC);
  dmnsn_keyword("min", DMNSN_T_MIN);
  dmnsn_keyword("min_extent", DMNSN_T_MIN_EXTENT);
  dmnsn_keyword("minimum_reuse", DMNSN_T_MINIMUM_REUSE);
  dmnsn_keyword("mod", DMNSN_T_MOD);
  dmnsn_keyword("mortar", DMNSN_T_MORTAR);
  dmnsn_keyword("natural_spline", DMNSN_T_NATURAL_SPLINE);
  dmnsn_keyword("nearest_count", DMNSN_T_NEAREST_COUNT);
  dmnsn_keyword("no", DMNSN_T_NO);
  dmnsn_keyword("no_bump_scale", DMNSN_T_NO_BUMP_SCALE);
  dmnsn_keyword("no_image", DMNSN_T_NO_IMAGE);
  dmnsn_keyword("no_reflection", DMNSN_T_NO_REFLECTION);
  dmnsn_keyword("no_shadow", DMNSN_T_NO_SHADOW);
  dmnsn_keyword("noise_generator", DMNSN_T_NOISE_GENERATOR);
  dmnsn_keyword("normal", DMNSN_T_NORMAL);
  dmnsn_keyword("normal_indices", DMNSN_T_NORMAL_INDICES);
  dmnsn_keyword("normal_map", DMNSN_T_NORMAL_MAP);
  dmnsn_keyword("normal_vectors", DMNSN_T_NORMAL_VECTORS);
  dmnsn_keyword("number_of_waves", DMNSN_T_NUMBER_OF_WAVES);
  dmnsn_keyword("object", DMNSN_T_OBJECT);
  dmnsn_keyword("octaves", DMNSN_T_OCTAVES);
  dmnsn_keyword("off", DMNSN_T_OFF);
  dmnsn_keyword("offset", DMNSN_T_OFFSET);
  dmnsn_keyword("omega", DMNSN_T_OMEGA);
  dmnsn_keyword("omnimax", DMNSN_T_OMNIMAX);
  dmnsn_keyword("on", DMNSN_T_ON);
  dmnsn_keyword("once", DMNSN_T_ONCE);
  dmnsn_keyword("onion", DMNSN_T_ONION);
  dmnsn_keyword("open", DMNSN_T_OPEN);
  dmnsn_keyword("orient", DMNSN_T_ORIENT);
  dmnsn_keyword("orientation", DMNSN_T_ORIENTATION);
  dmnsn_keyword("orthographic", DMNSN_T_ORTHOGRAPHIC);
  dmnsn_keyword("panoramic", DMNSN_T_PANORAMIC);
  dmnsn_keyword("parallel", DMNSN_T_PARALLEL);
  dmnsn_keyword("parametric", DMNSN_T_PARAMETRIC);
  dmnsn_keyword("pass_through", DMNSN_T_PASS_THROUGH);
  dmnsn_keyword("pattern", DMNSN_T_PATTERN);
  dmnsn_keyword("perspective", DMNSN_T_PERSPECTIVE);
  dmnsn_keyword("pgm", DMNSN_T_PGM);
  dmnsn_keyword("phase", DMNSN_T_PHASE);
  dmnsn_keyword("phong", DMNSN_T_PHONG);
  dmnsn_keyword("phong_size", DMNSN_T_PHONG_SIZE);
  dmnsn_keyword("photons", DMNSN_T_PHOTONS);
  dmnsn_keyword("pi", DMNSN_T_PI);
  dmnsn_keyword("pigment", DMNSN_T_PIGMENT);
  dmnsn_keyword("pigment_map", DMNSN_T_PIGMENT_MAP);
  dmnsn_keyword("pigment_pattern", DMNSN_T_PIGMENT_PATTERN);
  dmnsn_keyword("planar", DMNSN_T_PLANAR);
  dmnsn_keyword("plane", DMNSN_T_PLANE);
  dmnsn_keyword("png", DMNSN_T_PNG);
  dmnsn_keyword("point_at", DMNSN_T_POINT_AT);
  dmnsn_keyword("poly", DMNSN_T_POLY);
  dmnsn_keyword("poly_wave", DMNSN_T_POLY_WAVE);
  dmnsn_keyword("polygon", DMNSN_T_POLYGON);
  dmnsn_keyword("pot", DMNSN_T_POT);
  dmnsn_keyword("pow", DMNSN_T_POW);
  dmnsn_keyword("ppm", DMNSN_T_PPM);
  dmnsn_keyword("precision", DMNSN_T_PRECISION);
  dmnsn_keyword("precompute", DMNSN_T_PRECOMPUTE);
  dmnsn_keyword("pretrace_end", DMNSN_T_PRETRACE_END);
  dmnsn_keyword("pretrace_start", DMNSN_T_PRETRACE_START);
  dmnsn_keyword("prism", DMNSN_T_PRISM);
  dmnsn_keyword("prod", DMNSN_T_PROD);
  dmnsn_keyword("projected_through", DMNSN_T_PROJECTED_THROUGH);
  dmnsn_keyword("pwr", DMNSN_T_PWR);
  dmnsn_keyword("quadratic_spline", DMNSN_T_QUADRATIC_SPLINE);
  dmnsn_keyword("quadric", DMNSN_T_QUADRIC);
  dmnsn_keyword("quartic", DMNSN_T_QUARTIC);
  dmnsn_keyword("quaternion", DMNSN_T_QUATERNION);
  dmnsn_keyword("quick_color", DMNSN_T_QUICK_COLOR);
  dmnsn_keyword("quick_colour", DMNSN_T_QUICK_COLOR);
  dmnsn_keyword("quilted", DMNSN_T_QUILTED);
  dmnsn_keyword("radial", DMNSN_T_RADIAL);
  dmnsn_keyword("radians", DMNSN_T_RADIANS);
  dmnsn_keyword("radiosity", DMNSN_T_RADIOSITY);
  dmnsn_keyword("radius", DMNSN_T_RADIUS);
  dmnsn_keyword("rainbow", DMNSN_T_RAINBOW);
  dmnsn_keyword("ramp_wave", DMNSN_T_RAMP_WAVE);
  dmnsn_keyword("rand", DMNSN_T_RAND);
  dmnsn_keyword("ratio", DMNSN_T_RATIO);
  dmnsn_keyword("reciprocal", DMNSN_T_RECIPROCAL);
  dmnsn_keyword("recursion_limit", DMNSN_T_RECURSION_LIMIT);
  dmnsn_keyword("red", DMNSN_T_RED);
  dmnsn_keyword("reflection", DMNSN_T_REFLECTION);
  dmnsn_keyword("reflection_exponent", DMNSN_T_REFLECTION_EXPONENT);
  dmnsn_keyword("refraction", DMNSN_T_REFRACTION);
  dmnsn_keyword("repeat", DMNSN_T_REPEAT);
  dmnsn_keyword("rgb", DMNSN_T_RGB);
  dmnsn_keyword("rgbf", DMNSN_T_RGBF);
  dmnsn_keyword("rgbft", DMNSN_T_RGBFT);
  dmnsn_keyword("rgbt", DMNSN_T_RGBT);
  dmnsn_keyword("right", DMNSN_T_RIGHT);
  dmnsn_keyword("ripples", DMNSN_T_RIPPLES);
  dmnsn_keyword("rotate", DMNSN_T_ROTATE);
  dmnsn_keyword("roughness", DMNSN_T_ROUGHNESS);
  dmnsn_keyword("samples", DMNSN_T_SAMPLES);
  dmnsn_keyword("save_file", DMNSN_T_SAVE_FILE);
  dmnsn_keyword("scale", DMNSN_T_SCALE);
  dmnsn_keyword("scallop_wave", DMNSN_T_SCALLOP_WAVE);
  dmnsn_keyword("scattering", DMNSN_T_SCATTERING);
  dmnsn_keyword("seed", DMNSN_T_SEED);
  dmnsn_keyword("select", DMNSN_T_SELECT);
  dmnsn_keyword("shadowless", DMNSN_T_SHADOWLESS);
  dmnsn_keyword("sin", DMNSN_T_SIN);
  dmnsn_keyword("sine_wave", DMNSN_T_SINE_WAVE);
  dmnsn_keyword("sinh", DMNSN_T_SINH);
  dmnsn_keyword("size", DMNSN_T_SIZE);
  dmnsn_keyword("sky", DMNSN_T_SKY);
  dmnsn_keyword("sky_sphere", DMNSN_T_SKY_SPHERE);
  dmnsn_keyword("slice", DMNSN_T_SLICE);
  dmnsn_keyword("slope", DMNSN_T_SLOPE);
  dmnsn_keyword("slope_map", DMNSN_T_SLOPE_MAP);
  dmnsn_keyword("smooth", DMNSN_T_SMOOTH);
  dmnsn_keyword("smooth_triangle", DMNSN_T_SMOOTH_TRIANGLE);
  dmnsn_keyword("solid", DMNSN_T_SOLID);
  dmnsn_keyword("sor", DMNSN_T_SOR);
  dmnsn_keyword("spacing", DMNSN_T_SPACING);
  dmnsn_keyword("specular", DMNSN_T_SPECULAR);
  dmnsn_keyword("sphere", DMNSN_T_SPHERE);
  dmnsn_keyword("sphere_sweep", DMNSN_T_SPHERE_SWEEP);
  dmnsn_keyword("spherical", DMNSN_T_SPHERICAL);
  dmnsn_keyword("spiral1", DMNSN_T_SPIRAL1);
  dmnsn_keyword("spiral2", DMNSN_T_SPIRAL2);
  dmnsn_keyword("spline", DMNSN_T_SPLINE);
  dmnsn_keyword("split_union", DMNSN_T_SPLIT_UNION);
  dmnsn_keyword("spotlight", DMNSN_T_SPOTLIGHT);
  dmnsn_keyword("spotted", DMNSN_T_SPOTTED);
  dmnsn_keyword("sqr", DMNSN_T_SQR);
  dmnsn_keyword("sqrt", DMNSN_T_SQRT);
  dmnsn_keyword("str", DMNSN_T_STR);
  dmnsn_keyword("strcmp", DMNSN_T_STRCMP);
  dmnsn_keyword("strength", DMNSN_T_STRENGTH);
  dmnsn_keyword("strlen", DMNSN_T_STRLEN);
  dmnsn_keyword("strlwr", DMNSN_T_STRLWR);
  dmnsn_keyword("strupr", DMNSN_T_STRUPR);
  dmnsn_keyword("sturm", DMNSN_T_STURM);
  dmnsn_keyword("substr", DMNSN_T_SUBSTR);
  dmnsn_keyword("sum", DMNSN_T_SUM);
  dmnsn_keyword("superellipsoid", DMNSN_T_SUPERELLIPSOID);
  dmnsn_keyword("sys", DMNSN_T_SYS);
  dmnsn_keyword("t", DMNSN_T_T);
  dmnsn_keyword("tan", DMNSN_T_TAN);
  dmnsn_keyword("tanh", DMNSN_T_TANH);
  dmnsn_keyword("target", DMNSN_T_TARGET);
  dmnsn_keyword("text", DMNSN_T_TEXT);
  dmnsn_keyword("texture", DMNSN_T_TEXTURE);
  dmnsn_keyword("texture_list", DMNSN_T_TEXTURE_LIST);
  dmnsn_keyword("texture_map", DMNSN_T_TEXTURE_MAP);
  dmnsn_keyword("tga", DMNSN_T_TGA);
  dmnsn_keyword("thickness", DMNSN_T_THICKNESS);
  dmnsn_keyword("threshold", DMNSN_T_THRESHOLD);
  dmnsn_keyword("tiff", DMNSN_T_TIFF);
  dmnsn_keyword("tightness", DMNSN_T_TIGHTNESS);
  dmnsn_keyword("tile2", DMNSN_T_TILE2);
  dmnsn_keyword("tiles", DMNSN_T_TILES);
  dmnsn_keyword("tolerance", DMNSN_T_TOLERANCE);
  dmnsn_keyword("toroidal", DMNSN_T_TOROIDAL);
  dmnsn_keyword("torus", DMNSN_T_TORUS);
  dmnsn_keyword("trace", DMNSN_T_TRACE);
  dmnsn_keyword("transform", DMNSN_T_TRANSFORM);
  dmnsn_keyword("translate", DMNSN_T_TRANSLATE);
  dmnsn_keyword("transmit", DMNSN_T_TRANSMIT);
  dmnsn_keyword("triangle", DMNSN_T_TRIANGLE);
  dmnsn_keyword("triangle_wave", DMNSN_T_TRIANGLE_WAVE);
  dmnsn_keyword("true", DMNSN_T_TRUE);
  dmnsn_keyword("ttf", DMNSN_T_TTF);
  dmnsn_keyword("turb_depth", DMNSN_T_TURB_DEPTH);
  dmnsn_keyword("turbulence", DMNSN_T_TURBULENCE);
  dmnsn_keyword("type", DMNSN_T_TYPE);
  dmnsn_keyword("u", DMNSN_T_U);
  dmnsn_keyword("u_steps", DMNSN_T_U_STEPS);
  dmnsn_keyword("ultra_wide_angle", DMNSN_T_ULTRA_WIDE_ANGLE);
  dmnsn_keyword("union", DMNSN_T_UNION);
  dmnsn_keyword("up", DMNSN_T_UP);
  dmnsn_keyword("use_alpha", DMNSN_T_USE_ALPHA);
  dmnsn_keyword("use_color", DMNSN_T_USE_COLOR);
  dmnsn_keyword("use_colour", DMNSN_T_USE_COLOR);
  dmnsn_keyword("use_index", DMNSN_T_USE_INDEX);
  dmnsn_keyword("utf8", DMNSN_T_UTF8);
  dmnsn_keyword("uv_indices", DMNSN_T_UV_INDICES);
  dmnsn_keyword("uv_mapping", DMNSN_T_UV_MAPPING);
  dmnsn_keyword("uv_vectors", DMNSN_T_UV_VECTORS);
  dmnsn_keyword("v", DMNSN_T_V);
  dmnsn_keyword("v_steps", DMNSN_T_V_STEPS);
  dmnsn_keyword("val", DMNSN_T_VAL);
  dmnsn_keyword("variance", DMNSN_T_VARIANCE);
  dmnsn_keyword("vaxis_rotate", DMNSN_T_VAXIS_ROTATE);
  dmnsn_keyword("vcross", DMNSN_T_VCROSS);
  dmnsn_keyword("vdot", DMNSN_T_VDOT);
  dmnsn_keyword("vertex_vectors", DMNSN_T_VERTEX_VECTORS);
  dmnsn_keyword("vlength", DMNSN_T_VLENGTH);
  dmnsn_keyword("vnormalize", DMNSN_T_VNORMALIZE);
  dmnsn_keyword("vrotate", DMNSN_T_VROTATE);
  dmnsn_keyword("vstr", DMNSN_T_VSTR);
  dmnsn_keyword("vturbulence", DMNSN_T_VTURBULENCE);
  dmnsn_keyword("warp", DMNSN_T_WARP);
  dmnsn_keyword("water_level", DMNSN_T_WATER_LEVEL);
  dmnsn_keyword("waves", DMNSN_T_WAVES);
  dmnsn_keyword("width", DMNSN_T_WIDTH);
  dmnsn_keyword("wood", DMNSN_T_WOOD);
  dmnsn_keyword("wrinkles", DMNSN_T_WRINKLES);
  dmnsn_keyword("x", DMNSN_T_X);
  dmnsn_keyword("y", DMNSN_T_Y);
  dmnsn_keyword("yes", DMNSN_T_YES);
  dmnsn_keyword("z", DMNSN_T_Z);

  *linep = line;
  *colp  = col;
  *nextp = next;
  return 0;
}

/* Tokenize a language directive (#include, #declare, etc.) */
static int
dmnsn_tokenize_directive(const char *filename,
                         unsigned int *linep, unsigned int *colp,
                         char *map, size_t size, char **nextp,
                         dmnsn_token *token)
{
  unsigned int line = *linep, col = *colp;
  char *next = *nextp;
  unsigned int i = 0, alloc = 32;

  if (*next != '#') {
    return 1;
  }

  ++next;
  /* Handle spaces between `#' and directive */
  while (next - map < size && (*next == ' ' || *next == '\t')) {
    ++next;
  }

  char *directive = malloc(alloc);

  while (next - map < size && (isalnum(*next) || *next == '_')) {
    if (i + 1 >= alloc) {
      alloc *= 2;
      directive = realloc(directive, alloc);
    }

    directive[i] = *next;

    ++i;
    ++col;
    ++next;
  }

  directive[i] = '\0';

  /* Now check if we really found a directive */

#define dmnsn_directive(str, tp)                                               \
  do {                                                                         \
    if (strcmp(directive, str) == 0) {                                         \
      free(directive);                                                         \
      token->type = tp;                                                        \
                                                                               \
      *linep = line;                                                           \
      *colp  = col;                                                            \
      *nextp = next;                                                           \
      return 0;                                                                \
    }                                                                          \
  } while (0)

  dmnsn_directive("break",      DMNSN_T_BREAK);
  dmnsn_directive("case",       DMNSN_T_CASE);
  dmnsn_directive("debug",      DMNSN_T_DEBUG);
  dmnsn_directive("declare",    DMNSN_T_DECLARE);
  dmnsn_directive("default",    DMNSN_T_DEFAULT);
  dmnsn_directive("else",       DMNSN_T_ELSE);
  dmnsn_directive("end",        DMNSN_T_END);
  dmnsn_directive("error",      DMNSN_T_ERROR);
  dmnsn_directive("fclose",     DMNSN_T_FCLOSE);
  dmnsn_directive("fopen",      DMNSN_T_FOPEN);
  dmnsn_directive("if",         DMNSN_T_IF);
  dmnsn_directive("ifdef",      DMNSN_T_IFDEF);
  dmnsn_directive("ifndef",     DMNSN_T_IFNDEF);
  dmnsn_directive("include",    DMNSN_T_INCLUDE);
  dmnsn_directive("local",      DMNSN_T_LOCAL);
  dmnsn_directive("macro",      DMNSN_T_MACRO);
  dmnsn_directive("range",      DMNSN_T_RANGE);
  dmnsn_directive("read",       DMNSN_T_READ);
  dmnsn_directive("render",     DMNSN_T_RENDER);
  dmnsn_directive("statistics", DMNSN_T_STATISTICS);
  dmnsn_directive("switch",     DMNSN_T_SWITCH);
  dmnsn_directive("undef",      DMNSN_T_UNDEF);
  dmnsn_directive("version",    DMNSN_T_VERSION);
  dmnsn_directive("warning",    DMNSN_T_WARNING);
  dmnsn_directive("while",      DMNSN_T_WHILE);
  dmnsn_directive("write",      DMNSN_T_WRITE);

  free(directive);
  return 1;
}

dmnsn_array *
dmnsn_tokenize(const char *filename, FILE *file)
{
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

  /* Save the current locale */
  char *lc_ctype   = strdup(setlocale(LC_CTYPE, NULL));
  char *lc_numeric = strdup(setlocale(LC_NUMERIC, NULL));

  /* Set the locale to `C' to make isalpha(), strtoul(), etc. consistent */
  setlocale(LC_CTYPE, "C");
  setlocale(LC_NUMERIC, "C");

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
    dmnsn_simple_token('+', DMNSN_T_PLUS);
    dmnsn_simple_token('-', DMNSN_T_MINUS);
    dmnsn_simple_token('*', DMNSN_T_STAR);
    dmnsn_simple_token(',', DMNSN_T_COMMA);
    dmnsn_simple_token(';', DMNSN_T_SEMICOLON);
    dmnsn_simple_token('?', DMNSN_T_QUESTION);
    dmnsn_simple_token(':', DMNSN_T_COLON);
    dmnsn_simple_token('&', DMNSN_T_AND);
    dmnsn_simple_token('|', DMNSN_T_PIPE);
    dmnsn_simple_token('=', DMNSN_T_EQUALS);

    case '<':
      if (*(next + 1) == '=') {
        token.type = DMNSN_T_LESS_EQUAL;
        ++col;
        ++next;
      } else {
        token.type = DMNSN_T_LESS;
      }
      ++col;
      ++next;
      break;

    case '>':
      if (*(next + 1) == '=') {
        token.type = DMNSN_T_GREATER_EQUAL;
        ++col;
        ++next;
      } else {
        token.type = DMNSN_T_GREATER;
      }
      ++col;
      ++next;
      break;

    case '!':
      if (*(next + 1) == '=') {
        token.type = DMNSN_T_NOT_EQUAL;
        ++col;
        ++next;
      } else {
        token.type = DMNSN_T_BANG;
      }
      ++col;
      ++next;
      break;

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

    case '.': /* Number may begin with a decimal point, as in `.2' */
      if (dmnsn_tokenize_number(filename, &line, &col,
                                map, size, &next, &token) != 0) {
        token.type = DMNSN_T_DOT;
        ++col;
        ++next;
      }
      break;

    case '#':
      /* Language directive */
      if (dmnsn_tokenize_directive(filename, &line, &col,
                                   map, size, &next, &token) != 0) {
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

static void
dmnsn_print_token(FILE *file, const dmnsn_token *token)
{
  const char *tname;
  if (token->type == DMNSN_T_LPAREN) {
    tname = "\\(";
  } else if (token->type == DMNSN_T_RPAREN) {
    tname = "\\)";
  } else {
    tname = dmnsn_token_name(token->type);
  }

  if (token->value) {
    fprintf(file, "(%s \"%s\")", tname, token->value);
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
    dmnsn_print_token(file, &token);

    for (i = 1; i < dmnsn_array_size(tokens); ++i) {
      fprintf(file, " ");
      dmnsn_array_get(tokens, i, &token);
      dmnsn_print_token(file, &token);
    }

    fprintf(file, ")");
  }

  fprintf(file, "\n");
}

const char *
dmnsn_token_name(dmnsn_token_type token_type)
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
  dmnsn_token_map(DMNSN_T_INTEGER, "int");
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
