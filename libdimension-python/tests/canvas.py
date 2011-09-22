#!/usr/bin/env python3

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

import errno
from dimension import *

# Treat warnings as errors for tests
die_on_warnings(True)

canvas = Canvas(768, 480)

assert canvas.width  == 768, canvas.width
assert canvas.height == 480, canvas.height

have_PNG = True
try:
    canvas.optimize_PNG()
except OSError as e:
    if e.errno == errno.ENOSYS:
        have_PNG = False
    else:
        raise

have_GL = True
try:
    canvas.optimize_GL()
except OSError as e:
    if e.errno == errno.ENOSYS:
        haveGL = False
    else:
        raise

canvas.clear(Blue)

if have_PNG:
    canvas.write_PNG("png.png")

#if haveGL:
#    canvas.drawGL()
