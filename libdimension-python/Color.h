/*************************************************************************
 * Copyright (C) 2009-2011 Tavian Barnes <tavianator@tavianator.com>     *
 *                                                                       *
 * This file is part of The Dimension Python Module.                     *
 *                                                                       *
 * The Dimension Python Module is free software; you can redistribute it *
 * and/ or modify it under the terms of the GNU Lesser General Public    *
 * License as published by the Free Software Foundation; either version  *
 * 3 of the License, or (at your option) any later version.              *
 *                                                                       *
 * The Dimension Python Module is distributed in the hope that it will   *
 * be useful, but WITHOUT ANY WARRANTY; without even the implied         *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See *
 * the GNU Lesser General Public License for more details.               *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

typedef struct dmnsn_py_Color {
  PyObject_HEAD
  dmnsn_color c;
  dmnsn_color sRGB;
} dmnsn_py_Color;

extern PyTypeObject dmnsn_py_ColorType;

bool dmnsn_py_init_ColorType(void);

/* Color constants */
extern dmnsn_py_Color dmnsn_py_Black;
extern dmnsn_py_Color dmnsn_py_White;
extern dmnsn_py_Color dmnsn_py_Clear;
extern dmnsn_py_Color dmnsn_py_Red;
extern dmnsn_py_Color dmnsn_py_Green;
extern dmnsn_py_Color dmnsn_py_Blue;
extern dmnsn_py_Color dmnsn_py_Magenta;
extern dmnsn_py_Color dmnsn_py_Orange;
extern dmnsn_py_Color dmnsn_py_Yellow;
extern dmnsn_py_Color dmnsn_py_Cyan;
