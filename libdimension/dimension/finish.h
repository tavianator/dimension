/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Library.                           *
 *                                                                       *
 * The Dimension Library is free software; you can redistribute it and/  *
 * or modify it under the terms of the GNU Lesser General Public License *
 * as published by the Free Software Foundation; either version 3 of the *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Library is distributed in the hope that it will be      *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

/**
 * @file
 * Object finishes.
 */

/* Ambient component */

typedef struct dmnsn_ambient dmnsn_ambient;

/**
 * Ambient light callback.
 * @param[in] ambient  The ambient object itself.
 * @param[in] pigment  The pigment of the object.
 * @return The ambient contribution to the object's color.
 */
typedef dmnsn_color dmnsn_ambient_fn(const dmnsn_ambient *ambient,
                                     dmnsn_color pigment);

/** Ambient finish component. */
struct dmnsn_ambient {
  dmnsn_ambient_fn *ambient_fn; /**< Ambient callback. */
  dmnsn_free_fn    *free_fn;    /**< Destructor callback. */
  void             *ptr;        /**< Generic data pointer. */
  dmnsn_refcount   refcount;    /**< @internal Reference count. */
};

/** Allocate a dummy ambient component. */
dmnsn_ambient *dmnsn_new_ambient(void);
/** Delete an ambient component. */
void dmnsn_delete_ambient(dmnsn_ambient *ambient);

/* Diffuse component */

typedef struct dmnsn_diffuse dmnsn_diffuse;

/**
 * Diffuse reflection callback.
 * @param[in] diffuse  The diffuse object itself.
 * @param[in] light    The color of the light illuminating the object.
 * @param[in] color    The pigment of the object.
 * @param[in] ray      The direction of the light source.
 * @param[in] normal   The normal vector of the surface.
 * @return The diffuse reflection component of the object's color.
 */
typedef dmnsn_color dmnsn_diffuse_fn(const dmnsn_diffuse *diffuse,
                                     dmnsn_color light, dmnsn_color color,
                                     dmnsn_vector ray, dmnsn_vector normal);

/** Diffuse finish component. */
struct dmnsn_diffuse {
  dmnsn_diffuse_fn *diffuse_fn; /**< Diffuse callback. */
  dmnsn_free_fn    *free_fn;    /**< Destructor callback. */
  void             *ptr;        /**< Generic data pointer. */
  dmnsn_refcount   refcount;    /**< @internal Reference count. */
};

/** Allocate a dummy diffuse component. */
dmnsn_diffuse *dmnsn_new_diffuse(void);
/** Delete a diffuse component. */
void dmnsn_delete_diffuse(dmnsn_diffuse *diffuse);

/* Specular component */

typedef struct dmnsn_specular dmnsn_specular;

/**
 * Specular highlight callback.
 * @param[in] specular  The specular object itself.
 * @param[in] light     The color of the light illuminating the object.
 * @param[in] color     The pigment of the object.
 * @param[in] ray       The direction of the light source.
 * @param[in] normal    The normal vector of the surface.
 * @param[in] viewer    The direction of the viewer.
 * @return The specular reflection component of the object's color.
 */
typedef dmnsn_color dmnsn_specular_fn(const dmnsn_specular *specular,
                                      dmnsn_color light, dmnsn_color color,
                                      dmnsn_vector ray, dmnsn_vector normal,
                                      dmnsn_vector viewer);

/** Specular finish component. */
struct dmnsn_specular {
  dmnsn_specular_fn *specular_fn; /**< Specular callback. */
  dmnsn_free_fn     *free_fn;     /**< Destructor callback. */
  void              *ptr;         /**< Generic data pointer. */
  dmnsn_refcount    refcount;     /**< @internal Reference count. */
};

/** Allocate a dummy specular component. */
dmnsn_specular *dmnsn_new_specular(void);
/** Delete a specular component. */
void dmnsn_delete_specular(dmnsn_specular *specular);

/* Reflection component */

typedef struct dmnsn_reflection dmnsn_reflection;

/**
 * Reflected light callback.
 * @param[in] reflection  The reflection object itself.
 * @param[in] reflect     The color of the reflected ray.
 * @param[in] color       The pigment of the object.
 * @param[in] ray         The direction of the reflected ray.
 * @param[in] normal      The normal vector of the surface.
 * @return The contribution of the reflected ray to the object's color.
 */
typedef dmnsn_color dmnsn_reflection_fn(const dmnsn_reflection *reflection,
                                        dmnsn_color reflect, dmnsn_color color,
                                        dmnsn_vector ray, dmnsn_vector normal);

/** The reflection component. */
struct dmnsn_reflection {
  dmnsn_reflection_fn *reflection_fn; /**< Reflection callback. */
  dmnsn_free_fn       *free_fn;       /**< Destructor callback. */
  void                *ptr;           /**< Generic data pointer. */
  dmnsn_refcount      refcount;       /**< @internal Reference count. */
};

/** Allocate a dummy reflection component. */
dmnsn_reflection *dmnsn_new_reflection(void);
/** Delete a reflection component. */
void dmnsn_delete_reflection(dmnsn_reflection *reflection);

/* Entire finishes */

/** A finish. */
typedef struct dmnsn_finish {
  dmnsn_ambient    *ambient;    /**< Ambient component. */
  dmnsn_diffuse    *diffuse;    /**< Diffuse component. */
  dmnsn_specular   *specular;   /**< Specular component. */
  dmnsn_reflection *reflection; /**< Reflection component. */
} dmnsn_finish;

/**
 * Create a new blank finish.
 * @return The new finish.
 */
dmnsn_finish dmnsn_new_finish(void);

/**
 * Delete a finish.
 * @param[in,out] finish  The finish to delete.
 */
void dmnsn_delete_finish(dmnsn_finish finish);

/**
 * Increment a finish's reference count.
 * @param[in,out] finish  The finish to acquire.
 */
void dmnsn_finish_incref(dmnsn_finish *finish);

/**
 * Fill missing finish properties from a default finish.
 * @param[in]     default_finish  The default finish.
 * @param[in,out] finish          The finish to fill.
 */
void dmnsn_finish_cascade(const dmnsn_finish *default_finish,
                          dmnsn_finish *finish);
