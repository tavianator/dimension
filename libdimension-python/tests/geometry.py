#!/usr/bin/python3

#########################################################################
# Copyright (C) 2010-2011 Tavian Barnes <tavianator@tavianator.com>     #
#                                                                       #
# This file is part of The Dimension Test Suite.                        #
#                                                                       #
# The Dimension Test Suite is free software; you can redistribute it    #
# and/or modify it under the terms of the GNU General Public License as #
# published by the Free Software Foundation; either version 3 of the    #
# License, or (at your option) any later version.                       #
#                                                                       #
# The Dimension Test Suite is distributed in the hope that it will be   #
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty   #
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  #
# General Public License for more details.                              #
#                                                                       #
# You should have received a copy of the GNU General Public License     #
# along with this program.  If not, see <http://www.gnu.org/licenses/>. #
#########################################################################

from dimension import *

# Treat warnings as errors for tests
dieOnWarnings(True)

assert Zero == Vector(0, 0, 0), Zero
assert X    == Vector(1, 0, 0), X
assert Y    == Vector(0, 1, 0), Y
assert Z    == Vector(0, 0, 1), Z

v = Vector(1.5, 2.5, 3.5)

assert v.x == 1.5, v.x
assert v.y == 2.5, v.y
assert v.z == 3.5, v.z
assert repr(v) == 'dimension.Vector(1.5, 2.5, 3.5)', repr(v)
assert str(v) == '<1.5, 2.5, 3.5>', str(v)

v = Vector(x = 2, y = 3, z = 6)

assert v.norm() == 7, v.norm()
assert v.normalized() == v/7, v.normalized()
assert v + v == 2*v == v*2 == Vector(4, 6, 12), v + v
assert v/2 == v - v/2 == Vector(1, 1.5, 3), v/2
assert +v == v, +v
assert v + -v == Zero, v + -v
assert cross(v, v) == Zero, cross(v, v)
assert dot(v, v) == v.norm()**2, dot(v, v)
assert v, bool(v)
assert not Zero, not Zero
assert proj(v, X) == 2*X, proj(v, X)
