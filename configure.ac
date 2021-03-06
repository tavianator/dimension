dnl Copyright (C) 2009-2014 Tavian Barnes <tavianator@tavianator.com>
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
AC_PREREQ([2.68])
AC_INIT([Dimension], [0.0], [tavianator@tavianator.com], [dimension],
        [http://www.tavianator.com/dimension/])
AM_INIT_AUTOMAKE([subdir-objects parallel-tests color-tests std-options])
AM_SILENT_RULES([yes])

dnl Use C11 mode with GNU extensions by default
CFLAGS="-std=gnu11 $CFLAGS"

dnl Programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AM_PROG_AS
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
LT_INIT([win32-dll disable-static])
AM_PATH_PYTHON([3.2])
PKG_PROG_PKG_CONFIG

dnl Find the math library
AC_SEARCH_LIBS([sqrt], [m], [], [AC_MSG_ERROR([sqrt() function not found])])

dnl PNG canvas export
AC_ARG_ENABLE([png],
              [AS_HELP_STRING([--enable-png],
                              [Enable PNG canvas export [default=yes]])],
              [],
              [enable_png=maybe])
if test "$enable_png" != "no"; then
  PKG_CHECK_MODULES([libpng], [libpng], [enable_png=yes],
                    [test "$enable_png" = "yes" && AC_MSG_ERROR([libpng not found])
                     enable_png=no])
  AC_SUBST(libpng_CFLAGS)
  AC_SUBST(libpng_LIBS)
fi
AM_CONDITIONAL([PNG], [test "$enable_png" = "yes"])

dnl OpenGL canvas export
AC_ARG_ENABLE([gl],
              [AS_HELP_STRING([--enable-gl],
                              [Enable OpenGL canvas export [default=yes]])],
              [],
              [enable_gl=maybe])
if test "$enable_gl" != "no"; then
  AC_CHECK_LIB([GL], [glDrawPixels], [enable_gl=yes],
               [test "$enable_gl" = "yes" && AC_MSG_ERROR([libGL not found])
                enable_gl=no])
fi
AM_CONDITIONAL([GL], [test "$enable_gl" = "yes"])

dnl Built-in profiling support
AC_ARG_ENABLE([profile],
              [AS_HELP_STRING([--enable-profile],
                              [Enable built-in branch profiling support [default=no]])],
              [],
              [enable_profile=no])
AM_CONDITIONAL([PROFILE], [test "$enable_profile" = "yes"])

dnl Debug/release builds
AC_ARG_ENABLE([debug],
              [AS_HELP_STRING([--enable-debug],
                              [Perform potentially expensive sanity checks while running [default=no]])],
              [],
              [enable_debug=no])
if test "$enable_debug" = "yes"; then
  AC_DEFINE([DMNSN_DEBUG], [1])
else
  AC_DEFINE([DMNSN_DEBUG], [0])
  AC_DEFINE([NDEBUG], [1])
fi
AM_CONDITIONAL([DEBUG], [test "$enable_debug" = "yes"])

dnl Python headers
PKG_CHECK_MODULES([Python], [python3 >= 3.2],
                  [],
                  [AC_MSG_ERROR([Python not found])])
AC_SUBST(Python_CFLAGS)
AC_SUBST(Python_LIBS)

dnl Timing library for benchmarks
PKG_CHECK_MODULES([libsandglass], [libsandglass >= 0.2],
                  [],
                  [AC_MSG_WARN([libsandglass not found - benchmarking suite will not work])])
AC_SUBST(libsandglass_CFLAGS)
AC_SUBST(libsandglass_LIBS)

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4],
                  [],
                  [AC_MSG_WARN([check not found - test suite will not work])])

dnl Platform feature tests

m4_include(ax_pthread.m4)
AX_PTHREAD([], [AC_MSG_ERROR([pthread support not detected])])

AC_MSG_CHECKING([for strerror_r])
AC_RUN_IFELSE([
  AC_LANG_PROGRAM(
    [ #include <string.h>
      #include <errno.h> ],
    [ char buf@<:@256@:>@;
      return strerror_r(ENOMEM, buf, 256); ]
  )],
  [AC_DEFINE([DMNSN_STRERROR_R], [1])
   AC_MSG_RESULT([yes])],
  [AC_DEFINE([DMNSN_STRERROR_R], [0])
   AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for sys_errlist])
AC_LINK_IFELSE([
  AC_LANG_PROGRAM(
    [ #include <stdio.h>
      #include <errno.h> ],
    [ const char *errstr = sys_errlist@<:@sys_nerr - 1@:>@; ]
  )],
  [AC_DEFINE([DMNSN_SYS_ERRLIST], [1])
   AC_MSG_RESULT([yes])],
  [AC_DEFINE([DMNSN_SYS_ERRLIST], [0])
   AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for sched_getaffinity()])
AC_LINK_IFELSE([
  AC_LANG_PROGRAM(
    [ #define _GNU_SOURCE
      #include <sched.h> ],
    [ cpu_set_t cpuset;
      sched_getaffinity(0, sizeof(cpuset), &cpuset); ]
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

AC_MSG_CHECKING([for getrusage()])
AC_COMPILE_IFELSE([
  AC_LANG_PROGRAM(
    [
      #include <stddef.h>
      #include <sys/time.h>
      #include <sys/resource.h>
    ],
    [
      struct timeval real;
      gettimeofday(&real, NULL);
      struct rusage usage;
      getrusage(RUSAGE_SELF, &usage);
    ]
  )],
  [AC_DEFINE([DMNSN_GETRUSAGE], [1])
   AC_MSG_RESULT([yes])],
  [AC_DEFINE([DMNSN_GETRUSAGE], [0])
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

AC_CHECK_HEADER([GL/glx.h], [have_glx=yes], [have_glx=no])
if test "$enable_gl" = "yes" -a "$have_glx" = "yes"; then
  AC_CHECK_LIB([X11], [XOpenDisplay], [], [have_glx=no])
fi
AM_CONDITIONAL([GLX], [test "$enable_gl" = "yes" -a "$have_glx" = "yes"])

dnl Generate Makefiles
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile
                 dimension/Makefile
                 dimension/client.py
                 dimension/tests/Makefile
                 libdimension/Makefile
                 libdimension/Doxyfile
                 libdimension/bench/Makefile
                 libdimension/libdimension.pc
                 libdimension/tests/Makefile
                 libdimension-python/Makefile
                 libdimension-python/tests/Makefile])
AC_OUTPUT
