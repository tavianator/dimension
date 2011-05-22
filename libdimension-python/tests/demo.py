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

canvas = Canvas(width = 768, height = 480)

havePNG = True
try:
    canvas.optimizePNG()
except OSError as e:
    if e.errno == errno.ENOSYS:
        havePNG = False
    else:
        raise

camera = PerspectiveCamera(location = (0, 0.25, -4),
                           look_at  = Zero)
camera.transform(rotate(53*Y))

objects = []

sphere = Sphere(radius = 1, center = Zero)
objects.append(sphere)

scene = Scene(canvas = canvas,
              camera = camera,
              objects = objects)
scene.raytrace()

canvas.writePNG('demo.png')
