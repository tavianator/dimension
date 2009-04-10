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

#ifndef DIMENSIONXX_COLOR_HPP
#define DIMENSIONXX_COLOR_HPP

namespace Dimension
{
  class CIE_XYZ;
  class CIE_xyY;
  class CIE_Lab;
  class CIE_Luv;
  class sRGB;

  extern const CIE_XYZ whitepoint;

  class Color
  {
  public:
    Color() { }
    inline Color(const CIE_XYZ& XYZ);
    inline Color(const CIE_xyY& xyY);
    inline Color(const CIE_Lab& Lab, const CIE_XYZ& white = whitepoint);
    inline Color(const CIE_Luv& Luv, const CIE_XYZ& white = whitepoint);
    inline Color(const sRGB& RGB);
    explicit Color(dmnsn_color c) : m_color(c) { }
    // Color(const Color& c);
    // ~Color();

    double filter() const { return m_color.filter; }
    double trans()  const { return m_color.trans; }

    double filter(double f) { m_color.filter = f; }
    double trans(double t)  { m_color.trans = t; }

    // Color& operator=(const Color& c);

    dmnsn_color dmnsn() const { return m_color; }

  private:
    dmnsn_color m_color;
  };

  class CIE_XYZ
  {
  public:
    CIE_XYZ(double X, double Y, double Z)
      { m_XYZ.X = X; m_XYZ.Y = Y; m_XYZ.Z = Z; }
    CIE_XYZ(const Color& c) : m_XYZ(dmnsn_XYZ_from_color(c.dmnsn())) { }
    explicit CIE_XYZ(dmnsn_CIE_XYZ XYZ) : m_XYZ(XYZ) { }
    // CIE_XYZ(const CIE_XYZ& XYZ);
    // ~CIE_XYZ();

    double X() const { return m_XYZ.X; }
    double Y() const { return m_XYZ.Y; }
    double Z() const { return m_XYZ.Z; }

    // CIE_XYZ& operator=(const CIE_XYZ& XYZ);
    CIE_XYZ& operator=(const Color& c)
      { m_XYZ = dmnsn_XYZ_from_color(c.dmnsn()); }

    dmnsn_CIE_XYZ dmnsn() const { return m_XYZ; }

  private:
    dmnsn_CIE_XYZ m_XYZ;
  };

  class CIE_xyY
  {
  public:
    CIE_xyY(double x, double y, double Y)
      { m_xyY.x = x; m_xyY.y = y; m_xyY.Y = Y; }
    CIE_xyY(const Color& c) : m_xyY(dmnsn_xyY_from_color(c.dmnsn())) { }
    explicit CIE_xyY(dmnsn_CIE_xyY xyY) : m_xyY(xyY) { }
    // CIE_xyY(const CIE_xyY& xyY);
    // ~CIE_xyY();

    double x() const { return m_xyY.x; }
    double y() const { return m_xyY.y; }
    double Y() const { return m_xyY.Y; }

    // CIE_xyY& operator=(const CIE_xyY& xyY);
    CIE_xyY& operator=(const Color& c)
      { m_xyY = dmnsn_xyY_from_color(c.dmnsn()); }

    dmnsn_CIE_xyY dmnsn() const { return m_xyY; }

  private:
    dmnsn_CIE_xyY m_xyY;
  };

  class CIE_Lab
  {
  public:
    CIE_Lab(double L, double a, double b)
      { m_Lab.L = L; m_Lab.a = a; m_Lab.b = b; }
    CIE_Lab(const Color& c, const CIE_XYZ& white = whitepoint)
      : m_Lab(dmnsn_Lab_from_color(c.dmnsn(), white.dmnsn())) { }
    explicit CIE_Lab(dmnsn_CIE_Lab Lab) : m_Lab(Lab) { }
    // CIE_Lab(const CIE_Lab& Lab);
    // ~CIE_Lab();

    double L() const { return m_Lab.L; }
    double a() const { return m_Lab.a; }
    double b() const { return m_Lab.b; }

    // CIE_Lab& operator=(const CIE_Lab& Lab);
    CIE_Lab& operator=(const Color& c)
      { m_Lab = dmnsn_Lab_from_color(c.dmnsn(), whitepoint.dmnsn()); }

    dmnsn_CIE_Lab dmnsn() const { return m_Lab; }

  private:
    dmnsn_CIE_Lab m_Lab;
  };

  class CIE_Luv
  {
  public:
    CIE_Luv(double L, double u, double v)
      { m_Luv.L = L; m_Luv.u = u; m_Luv.v = v; }
    CIE_Luv(const Color& c, const CIE_XYZ& white = whitepoint)
      : m_Luv(dmnsn_Luv_from_color(c.dmnsn(), white.dmnsn())) { }
    explicit CIE_Luv(dmnsn_CIE_Luv Luv) : m_Luv(Luv) { }
    // CIE_Luv(const CIE_Luv& Luv);
    // ~CIE_Luv();

    double L() const { return m_Luv.L; }
    double u() const { return m_Luv.u; }
    double v() const { return m_Luv.v; }

    // CIE_Luv& operator=(const CIE_Luv& Luv);
    CIE_Luv& operator=(const Color& c)
      { m_Luv = dmnsn_Luv_from_color(c.dmnsn(), whitepoint.dmnsn()); }

    dmnsn_CIE_Luv dmnsn() const { return m_Luv; }

  private:
    dmnsn_CIE_Luv m_Luv;
  };

  class sRGB
  {
  public:
    sRGB(double R, double G, double B)
      { m_RGB.R = R; m_RGB.G = G; m_RGB.B = B; }
    sRGB(const Color& c) : m_RGB(dmnsn_sRGB_from_color(c.dmnsn())) { }
    explicit sRGB(dmnsn_sRGB RGB) : m_RGB(RGB) { }
    // sRGB(const sRGB& RGB);
    // ~sRGB();

    double R() const { return m_RGB.R; }
    double G() const { return m_RGB.G; }
    double B() const { return m_RGB.B; }

    // sRGB& operator=(const sRGB& RGB);
    sRGB& operator=(const Color& c)
      { m_RGB = dmnsn_sRGB_from_color(c.dmnsn()); }

    dmnsn_sRGB dmnsn() const { return m_RGB; }

  private:
    dmnsn_sRGB m_RGB;
  };

  inline Color::Color(const CIE_XYZ& XYZ)
    : m_color(dmnsn_color_from_XYZ(XYZ.dmnsn())) { }

  inline Color::Color(const CIE_xyY& xyY)
    : m_color(dmnsn_color_from_xyY(xyY.dmnsn())) { }

  inline Color::Color(const CIE_Lab& Lab, const CIE_XYZ& white)
    : m_color(dmnsn_color_from_Lab(Lab.dmnsn(), white.dmnsn())) { }

  inline Color::Color(const CIE_Luv& Luv, const CIE_XYZ& white)
    : m_color(dmnsn_color_from_Luv(Luv.dmnsn(), white.dmnsn())) { }

  inline Color::Color(const sRGB& RGB)
    : m_color(dmnsn_color_from_sRGB(RGB.dmnsn())) { }
}

#endif /* DIMENSIONXX_COLOR_HPP */
