dnl Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>
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

AC_PREREQ(2.63)
AC_INIT(Dimension, 0.0.0, tavianator@gmail.com, dimension)
AM_INIT_AUTOMAKE(dimension, 0.0.0)

dnl Programs
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AM_PROG_AS
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

AC_MSG_CHECKING([for fopencookie()])
AC_LINK_IFELSE(AC_LANG_PROGRAM(
  [[ #define _GNU_SOURCE
     #include <stdio.h> ]],
  [[ cookie_io_functions_t io_funcs; 
     FILE* file = fopencookie(NULL, "r", io_funcs); ]]),
               [fopencookie=yes
                AC_MSG_RESULT([yes])],
               [AC_MSG_RESULT([no])])

AM_CONDITIONAL([FOPENCOOKIE], [test "$fopencookie" = "yes"])

dnl Generate Makefiles
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 libdimension/Makefile
                 libdimensionxx/Makefile
                 tests/Makefile])
AC_OUTPUT
