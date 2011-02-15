/*************************************************************************
 * Copyright (C) 2009-2010 Tavian Barnes <tavianator@gmail.com>          *
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

#include "platform.h"
#if HAVE_UNISTD_H
  #include <unistd.h>
#endif
#if DMNSN_TIOCGWINSZ
  #include <sys/ioctl.h>
#endif

unsigned int
dmnsn_terminal_width(void)
{
#if DMNSN_TIOCGWINSZ
  struct winsize ws;
  unsigned int width = 80;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
    width = ws.ws_col;
  }
  return width;
#else
  return 80;
#endif
}
