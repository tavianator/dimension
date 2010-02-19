dnl Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>
dnl
dnl This file is part of The Dimension Build Suite.
dnl
dnl The Dimension Build Suite is free software; you can redistribute it
dnl and/or modify it under the terms of the GNU General Public License as
dnl published by the Free Software Foundation; either version 3 of the
dnl License, or (at your option) any later version.
dnl
dnl The Dimension Build Suite is distributed in the hope that it will be
dnl useful, but WITHOUT ANY WARRANTY; without even the implied warranty
dnl of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ(2.64)
AC_INIT(Dimension, 0.0.0, tavianator@gmail.com, dimension)
AM_INIT_AUTOMAKE([parallel-tests])

dnl Programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AM_PROG_LEX
AC_PROG_YACC
AM_PROG_AS
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

dnl Profile-guided optimization (default disabled)
AC_ARG_ENABLE([pgo],
              [AS_HELP_STRING([--enable-pgo],
                              [Enable profile-guided optimization [default=no]])],
              [],
              [enable_pgo=no])
AM_CONDITIONAL([PGO], [test "$enable_pgo" != "no"])

dnl Timing library for benchmarks
AC_CHECK_LIB([sandglass], [sandglass_create],
             [],
             [AC_MSG_WARN([libsandglass not found - benchmarking suite will not work])])

dnl Ensure compilation in C99 mode
AC_DEFINE([_XOPEN_SOURCE], [600])

dnl Generate Makefiles
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile
                 libdimension/Makefile
                 dimension/Makefile
                 tests/Makefile
                 tests/libdimension/Makefile
                 tests/dimension/Makefile
                 bench/Makefile
                 bench/libdimension/Makefile
                 doc/Makefile])
AC_OUTPUT
