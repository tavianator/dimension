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

# Canvas
canvas = Canvas(width = 768, height = 480)

havePNG = True
try:
  canvas.optimizePNG()
except OSError as e:
  if e.errno == errno.ENOSYS:
    havePNG = False
  else:
    raise

# Camera
camera = PerspectiveCamera(location = (0, 0.25, -4),
                           lookAt   = 0)
camera.transform(rotate(53*Y))

# Lights
lights = [
  PointLight(location = (-15, 20, 10), color = White),
]

# Objects

hollowCube = Difference(
  [
    Box(
      (-1, -1, -1), (1, 1, 1),

      texture = Texture(
        pigment = Color(0, 0, 1, trans = 0.75, filter = 1/3),
        finish  = Reflection(0.5),
      ),
      interior = Interior(
        ior = 1.1,
      ),
    )
    .transform(rotate(45*X)),

    Sphere(
      center = 0, radius = 1.25,
      texture = Texture(
        pigment = Green,
        finish  = Phong(strength = 0.2, size = 40),
      ),
    )
  ],
)

arrow = Union(
  [
    Cylinder(bottom = -1.25*Y, top = 1.25*Y, radius = 0.1),
    Cone(
      bottom = 1.25*Y, bottomRadius = 0.1,
      top    = 1.5*Y,  topRadius    = 0,
      open = True
    ),
  ],
  texture = Texture(
    pigment = ColorMap(
      Gradient(Y),
      {
        0/6: Red,
        1/6: Orange,
        2/6: Yellow,
        3/6: Green,
        4/6: Blue,
        5/6: Magenta,
        6/6: Red,
      },
    )
    .transform(scale(1, 2.75, 1))
    .transform(translate(-1.25*Y)),
  ),
)
arrow.transform(rotate(-45*X))

torii = Union(
  [
    Torus(majorRadius = 0.15, minorRadius = 0.05)
      .transform(translate(-Y)),

    Torus(majorRadius = 0.15, minorRadius = 0.05),

    Torus(majorRadius = 0.15, minorRadius = 0.05)
      .transform(translate(Y)),
  ],
  texture = Texture(
    pigment = Blue,
    finish  = Ambient(1),
  ),
)
torii.transform(rotate(-45*X))

ground = Plane(
  normal = Y, distance = -2,

  texture = Texture(
    pigment = PigmentMap(
      Checker(),
      [
        White,
        ColorMap(Checker(), [Black, White]).transform(scale(1/3, 1/3, 1/3))
      ],
    ),
  ),
)

objects = [
  hollowCube,
  arrow,
  torii,
  ground,
]

# Sky sphere
skySphere = SkySphere(
  [
    ColorMap(
      pattern = Gradient(Y),
      map     = {
        0:    Orange,
        0.35: Color(0, 0.1, 0.2, trans = 0.1, filter = 0.0),
      },
    ),
  ]
)

# Scene
scene = Scene(canvas  = canvas,
              objects = objects,
              lights  = lights,
              camera  = camera)
scene.defaultTexture = Texture(finish = Ambient(0.1) + Diffuse(0.6))
scene.background     = Clear
scene.skySphere      = skySphere
scene.adcBailout     = 1/255
scene.recursionLimit = 5
scene.raytrace()

if havePNG:
  canvas.writePNG('demo.png')
