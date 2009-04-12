/*************************************************************************
 * Copyright (C) 2008 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of The Dimension Library.                           *
 *                                                                       *
 * The Dimension Library is free software; you can redistribute it and/  *
 * or modify it under the terms of the GNU Lesser General Public License *
 * as published by the Free Software Foundation; either version 3 of the *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Library is distributed in the hope that it will be      *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#ifndef DIMENSIONXX_PNG_HPP
#define DIMENSIONXX_PNG_HPP

#include <istream>
#include <ostream>

namespace Dimension
{
  class PNG_Canvas : public Canvas
  {
  public:
    explicit PNG_Canvas(std::istream& istr)
      : Canvas(), m_istr(&istr), m_ostr(0), m_written(false) { read(); }
    PNG_Canvas(unsigned int x, unsigned int y, std::ostream& ostr)
      : Canvas(x, y), m_istr(0), m_ostr(&ostr), m_written(false) { }
    PNG_Canvas(std::istream& istr, std::ostream& ostr)
      : Canvas(), m_istr(&istr), m_ostr(&ostr), m_written(false) { read(); }
    virtual ~PNG_Canvas();

    void write();

  protected:
    PNG_Canvas(std::ostream* ostr)
      : Canvas(), m_istr(0), m_ostr(ostr), m_written(false) { }

  private:
    std::istream* m_istr;
    std::ostream* m_ostr;
    bool m_written;

    void read();

    // Copying prohibited
    PNG_Canvas(const PNG_Canvas&);
    PNG_Canvas& operator=(const PNG_Canvas&);
  };
}

#endif /* DIMENSIONXX_PNG_HPP */
