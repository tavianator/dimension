/*************************************************************************
 * Copyright (C) 2010 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of Dimension.                                       *
 *                                                                       *
 * Dimension is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU General Public License as published by the *
 * Free Software Foundation; either version 3 of the License, or (at     *
 * your option) any later version.                                       *
 *                                                                       *
 * Dimension is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "progressbar.h"
#include <sys/ioctl.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

void
dmnsn_progressbar(const char *format, const dmnsn_progress *progress, ...)
{
  va_list ap;
  va_start(ap, progress);

  int len = vprintf(format, ap) + 1;
  if (len < 1)
    len = 1;
  printf(" ");

  va_end(ap);

  unsigned int increments = 48;

  /* Try to fill the terminal with the progress bar; this is non-portable */
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
    increments = ws.ws_col - (len % ws.ws_col);
  }

  fflush(stdout);

  unsigned int i;
  for (i = 0; i < increments; ++i) {
    dmnsn_wait_progress(progress, ((double)(i + 1))/increments);

    printf(".");
    fflush(stdout);
  }
  printf("\n");
  fflush(stdout);
}
