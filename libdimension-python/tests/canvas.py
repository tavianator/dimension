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
import errno

# Treat warnings as errors for tests
dieOnWarnings(True)

canvas = Canvas(768, 480)

assert canvas.width  == 768, canvas.width
assert canvas.height == 480, canvas.height

havePNG = True
try:
    canvas.optimizePNG()
except OSError as e:
    if e.errno == errno.ENOSYS:
        havePNG = False
    else:
        raise

haveGL = True
try:
    canvas.optimizeGL()
except OSError as e:
    if e.errno == errno.ENOSYS:
        haveGL = False
    else:
        raise

canvas.clear(Blue)

if havePNG:
    canvas.writePNG('png.png')

#if haveGL:
#    canvas.drawGL()
