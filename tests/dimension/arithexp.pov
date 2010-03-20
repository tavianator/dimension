/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
 *                                                                       *
 * The Dimension Test Suite is free software; you can redistribute it    *
 * and/or modify it under the terms of the GNU General Public License as *
 * published by the Free Software Foundation; either version 3 of the    *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Test Suite is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

// Test arithmetic expression handling

sphere {
  2*<<2.0 - 1.0, 3.0, 4.0>.x, (1.0 + 2)*2 - 5, 1.0 + 2*2 - 4> - -<0, 0, 1>,
  exp(1) - 1*2
}

#if (abs(-1) != 1)
  #error "abs"
#end

#if (acos(0) != 1.570796326794897)
  #error "acos"
#end

#if (acosh(2) != 1.316957896924817)
  #error "acosh"
#end

#if (asc("ABC") != 65)
  #error "asc"
#end

#if (asin(1) != 1.570796326794897)
  #error "asin"
#end

#if (asinh(2) != 1.44363547517881)
  #error "asinh"
#end

#if (atan(1) != 0.7853981633974483)
  #error "atan"
#end

#if (atan2(-1, -1) != -2.35619449019234)
  #error "atan2"
#end

#if (atanh(0.5) != 0.5493061443340548)
  #error "atanh"
#end

#if (ceil(-1.5) != -1)
  #error "ceil"
#end

#if (cos(1.570796326794897) != 0)
  #error "cos"
#end

#if (cosh(1.316957896924817) != 2)
  #error "cosh"
#end

#if (degrees(1.570796326794897) != 90)
  #error "degrees"
#end

#if (div(3,2) != 1)
  #error "div"
#end

#if (exp(1) != 2.718281828459045)
  #error "exp"
#end

#if (floor(-1.5) != -2)
  #error "floor"
#end
