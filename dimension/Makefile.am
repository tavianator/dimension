###########################################################################
## Copyright (C) 2011 Tavian Barnes <tavianator@tavianator.com>          ##
##                                                                       ##
## This file is part of The Dimension Build Suite.                       ##
##                                                                       ##
## The Dimension Build Suite is free software; you can redistribute it   ##
## and/or modify it under the terms of the GNU General Public License as ##
## published by the Free Software Foundation; either version 3 of the    ##
## License, or (at your option) any later version.                       ##
##                                                                       ##
## The Dimension Build Suite is distributed in the hope that it will be  ##
## useful, but WITHOUT ANY WARRANTY; without even the implied warranty   ##
## of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  ##
## General Public License for more details.                              ##
##                                                                       ##
## You should have received a copy of the GNU General Public License     ##
## along with this program.  If not, see <http://www.gnu.org/licenses/>. ##
###########################################################################

SUBDIRS = .                                                                    \
          tests

dist_bin_SCRIPTS = dimension

# make distcheck fails on the client because Python cannot find the module,
# since it's not really installed, so disable the --help and --version checks
AM_INSTALLCHECK_STD_OPTIONS_EXEMPT = dimension

pkgpython_PYTHON = __init__.py                                                 \
                   preview.py
nodist_pkgpython_PYTHON = client.py

clean-local:
	rm -rf __pycache__/
