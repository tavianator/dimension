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

assert 0 == Vector(0, 0, 0), Vector(0)
assert X == Vector(1, 0, 0), X
assert Y == Vector(0, 1, 0), Y
assert Z == Vector(0, 0, 1), Z

assert Vector((1, 2, 3)) == Vector(1, 2, 3), Vector((1, 2, 3))
assert Vector(X) == X, Vector(X)

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
assert v + -v == 0, v + -v
assert cross(v, v) == 0, cross(v, v)
assert dot(v, v) == v.norm()**2, dot(v, v)
assert v, bool(v)
assert not Vector(0), not Vector(0)
assert proj(v, X) == 2*X, proj(v, X)

m = Matrix(1,  2,  3,  4,
           5,  6,  7,  8,
           9, 10, 11, 12)

assert repr(m) == 'dimension.Matrix(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, \
9.0, 10.0, 11.0, 12.0)', repr(m)
assert str(m) == '\n' \
                 '[1.0\t2.0\t3.0\t4.0]\n' \
                 '[5.0\t6.0\t7.0\t8.0]\n' \
                 '[9.0\t10.0\t11.0\t12.0]\n' \
                 '[0.0\t0.0\t0.0\t1.0]', str(m)

s = scale(Vector(1, 2, 3))
assert s == Matrix(1, 0, 0, 0,
                   0, 2, 0, 0,
                   0, 0, 3, 0), s

t = translate(1, 2, 3)
assert t == Matrix(1, 0, 0, 1,
                   0, 1, 0, 2,
                   0, 0, 1, 3), t

r = rotate(90*Y).inverse()
assert r == Matrix(0, 0, -1, 0,
                   0, 1,  0, 0,
                   1, 0,  0, 0), r
