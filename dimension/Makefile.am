###########################################################################
## Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          ##
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

# Make dmnsn_error() backtraces useful
AM_LDFLAGS = -rdynamic

INCLUDES = -I$(top_srcdir)/libdimension

bin_PROGRAMS = dimension

AM_YFLAGS = -d
BUILT_SOURCES = directives.h grammar.h
EXTRA_DIST = common.prologue                                                   \
             common.declarations                                               \
             common.terminals                                                  \
             common.nonterminals                                               \
             common.rules                                                      \
             directives.prologue                                               \
             directives.declarations                                           \
             directives.nonterminals                                           \
             directives.rules                                                  \
             grammar.prologue                                                  \
             grammar.declarations                                              \
             grammar.nonterminals                                              \
             grammar.rules                                                     \
             grammar.epilogue

grammar.y: grammar.prologue grammar.declarations grammar.nonterminals grammar.rules grammar.epilogue common.prologue common.declarations common.terminals common.nonterminals common.rules
	@echo "%{" >$@
	@cat {common,grammar}.prologue >>$@
	@echo "%}" >>$@
	@cat common.terminals {grammar,common}.{declarations,nonterminals} >>$@
	@echo "%%" >>$@
	@cat grammar.rules common.rules >>$@
	@echo "%%" >>$@
	@cat grammar.epilogue >>$@

directives.y: directives.prologue directives.declarations directives.nonterminals directives.rules common.prologue common.declarations common.terminals common.nonterminals common.rules
	@echo "%{" >$@
	@cat {common,directives}.prologue >>$@
	@echo "%}" >>$@
	@cat common.terminals {directives,common}.{declarations,nonterminals} >>$@
	@echo "%%" >>$@
	@cat directives.rules common.rules >>$@

dimension_SOURCES = directives.y                                               \
                    grammar.y                                                  \
                    lexer.l                                                    \
                    main.c                                                     \
                    parse.c                                                    \
                    parse.h                                                    \
                    platform.c                                                 \
                    platform.h                                                 \
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
dimension_LDFLAGS = $(AM_LDFLAGS)
