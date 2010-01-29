###########################################################################
## Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               ##
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

INCLUDES = -I$(top_srcdir)/libdimension

bin_PROGRAMS = dimension

AM_YFLAGS = -d
BUILT_SOURCES = grammar.h
EXTRA_DIST = grammar.prologue                                                  \
             grammar.declarations                                              \
             grammar.terminals                                                 \
             grammar.nonterminals                                              \
             grammar.rules                                                     \
             grammar.epilogue

grammar.y: grammar.prologue grammar.declarations grammar.terminals grammar.nonterminals grammar.rules grammar.epilogue
	echo "%{" >$@
	cat grammar.prologue >>$@
	echo "%}" >>$@
	cat grammar.{declarations,terminals,nonterminals} >>$@
	echo "%%" >>$@
	cat grammar.rules >>$@
	echo "%%" >>$@
	cat grammar.epilogue >>$@

dimension_SOURCES = grammar.y                                                  \
                    lexer.l                                                    \
                    main.c                                                     \
                    parse.c                                                    \
                    parse.h                                                    \
                    progressbar.c                                              \
                    progressbar.h                                              \
                    realize.c                                                  \
                    realize.h                                                  \
                    tokenize.c                                                 \
                    tokenize.h                                                 \
                    utility.c                                                  \
                    utility.h                                                  \
                    y.tab.h
dimension_LDADD   = $(top_builddir)/libdimension/libdimension.la
