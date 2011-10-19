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

import os
import os.path
import errno
from math import *
from dimension import *

# Treat warnings as errors for tests
die_on_warnings(True)

# Canvas
canvas = Canvas(width = 768, height = 480)

have_PNG = True
try:
  canvas.optimize_PNG()
except OSError as e:
  if e.errno == errno.ENOSYS:
    have_PNG = False
  else:
    raise

path = os.path.join(os.environ["top_srcdir"], "dimension/tests/demo.dmnsn")
with open(path) as fh:
  exec(compile(fh.read(), path, "exec"))

# Scene
scene = Scene(canvas  = canvas,
              objects = objects,
              lights  = lights,
              camera  = camera)
scene.default_texture = Texture(finish = Ambient(0.1) + Diffuse(0.7))
scene.background      = background
scene.adc_bailout     = 1/255
scene.recursion_limit = 5
scene.ray_trace()

if have_PNG:
  canvas.write_PNG("demo.png")
