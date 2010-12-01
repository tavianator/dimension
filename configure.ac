dnl Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>
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

dnl Initialization
AC_PREREQ(2.65)
AC_INIT(Dimension, 0.0.0, tavianator@gmail.com, dimension,
        [http://dmnsn.googlecode.com])
AM_INIT_AUTOMAKE([parallel-tests color-tests std-options])
AM_SILENT_RULES([yes])

dnl Use C99 mode with GNU extensions
CFLAGS="-std=gnu99 $CFLAGS"

dnl Programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AM_PROG_LEX
AC_PROG_YACC
AM_PROG_AS
AC_PROG_INSTALL
AC_LIBTOOL_WIN32_DLL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

dnl PNG canvas export
AC_ARG_ENABLE([png],
              [AS_HELP_STRING([--enable-png],
                              [Enable PNG canvas export [default=yes]])],
              [],
              [enable_png=yes])
AM_CONDITIONAL([PNG], [test "$enable_png" != "no"])

dnl OpenGL canvas export
AC_ARG_ENABLE([gl],
              [AS_HELP_STRING([--enable-gl],
                              [Enable OpenGL canvas export [default=yes]])],
              [],
              [enable_gl=yes])
AM_CONDITIONAL([GL], [test "$enable_gl" != "no"])

dnl Profile-guided optimization (default disabled)
AC_ARG_ENABLE([pgo],
              [AS_HELP_STRING([--enable-pgo],
                              [Enable profile-guided optimization [default=no]])],
              [],
              [enable_pgo=no])
AM_CONDITIONAL([PGO], [test "$enable_pgo" != "no"])

dnl Timing library for benchmarks
PKG_CHECK_MODULES([libsandglass], [libsandglass >= 0.2],
                  [],
                  [AC_MSG_WARN([libsandglass not found - benchmarking suite will not work])])
AC_SUBST(libsandglass_CFLAGS)
AC_SUBST(libsandglass_LIBS)

dnl Platform feature tests

AC_MSG_CHECKING([for sched_getaffinity()])
AC_LINK_IFELSE([
  AC_LANG_PROGRAM(
    [
      #define _GNU_SOURCE
      #include <sched.h>
    ],
    [
      cpu_set_t cpuset;
      sched_getaffinity(0, sizeof(cpuset), &cpuset);
    ]
  )],
  [AC_DEFINE([DMNSN_SCHED_GETAFFINITY], [1])
   AC_MSG_RESULT([yes])],
  [AC_DEFINE([DMNSN_SCHED_GETAFFINITY], [0])
   AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for sysconf(_SC_NPROCESSORS_ONLN)])
AC_COMPILE_IFELSE([
  AC_LANG_PROGRAM(
    [ #include <unistd.h> ],
    [ sysconf(_SC_NPROCESSORS_ONLN); ]
  )],
  [AC_DEFINE([DMNSN_SC_NPROCESSORS_ONLN], [1])
   AC_MSG_RESULT([yes])],
  [AC_DEFINE([DMNSN_SC_NPROCESSORS_ONLN], [0])
   AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for backtrace()])
AC_LINK_IFELSE([
  AC_LANG_PROGRAM(
    [ #include <execinfo.h> ],
    [ backtrace(0, 0); ]
  )],
  [AC_DEFINE([DMNSN_BACKTRACE], [1])
   AC_MSG_RESULT([yes])],
  [AC_DEFINE([DMNSN_BACKTRACE], [0])
   AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for gettid()])
AC_COMPILE_IFELSE([
  AC_LANG_PROGRAM(
    [ #include <sys/syscall.h> ],
    [ syscall(SYS_gettid); ]
  )],
  [AC_DEFINE([DMNSN_GETTID], [1])
   AC_MSG_RESULT([yes])],
  [AC_DEFINE([DMNSN_GETTID], [0])
   AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for times()])
AC_COMPILE_IFELSE([
  AC_LANG_PROGRAM(
    [
      #include <sys/times.h>
      #include <unistd.h>
    ],
    [
      long clk_tck = sysconf(_SC_CLK_TCK);
      struct tms buf;
      clock_t real = times(&buf);
    ]
  )],
  [AC_DEFINE([DMNSN_TIMES], [1])
   AC_MSG_RESULT([yes])],
  [AC_DEFINE([DMNSN_TIMES], [0])
   AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for ioctl(TIOCGWINSZ)])
AC_COMPILE_IFELSE([
  AC_LANG_PROGRAM(
    [
      #include <sys/ioctl.h>
      #include <unistd.h>
    ],
    [
      struct winsize ws;
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    ]
  )],
  [AC_DEFINE([DMNSN_TIOCGWINSZ], [1])
   AC_MSG_RESULT([yes])],
  [AC_DEFINE([DMNSN_TIOCGWINSZ], [0])
   AC_MSG_RESULT([no])]
)

AC_CHECK_HEADER([GL/glx.h], [enable_glx=yes], [enable_glx=no])
AM_CONDITIONAL([GLX], [test "$enable_glx" != "no"])

dnl Generate Makefiles
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile
                 libdimension/Makefile
                 libdimension/Doxyfile
                 libdimension/libdimension.pc
                 dimension/Makefile
                 tests/Makefile
                 tests/libdimension/Makefile
                 tests/dimension/Makefile
                 bench/Makefile
                 bench/libdimension/Makefile
                 bench/dimension/Makefile
                 doc/Makefile])
AC_OUTPUT
