/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@tavianator.com>     *
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

// Test the language directives

#version 3.6;

#debug "debug"
#warning "warning"

#include "directives.inc"

#declare R = 1;
#local Color = rgb <1, 0, 1>;

#declare Unused = -1;
#undef Unused

#ifdef (Local)
  #error "Local escaped from include file"
#end

#ifdef (Unused)
  #error "#undef failed"
#end

#macro Make_Sphere(n)
  sphere {
    Center + <0, n, 0>, R
    pigment {
      color Color green 1
    }
  }
#end

#macro Inc(n)
  #declare n = n + 1;
#end

#declare Counter = 0;
#while (Counter < 2)
  #if (#if (1 = 1) 0 #end = 0 & !1)
    #error "Nested #if parsing failed"
  #else
    Make_Sphere(Counter)
  #end

  Inc(Counter)
#end

#declare Box =
  box {
    <-1, -1, -1>, <1, 1, 1>
    pigment {
      color rgb <1, 1, 1>
    }
  }

object {
  Box
  finish {
    phong 0.2
  }
}
