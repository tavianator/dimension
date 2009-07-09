/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of The Dimension Test Suite.                        *
 *                                                                       *
 * The Dimension Test Suite is free software; you can redistribute it    *
 * and/or modify it under the terms of the GNU General Public License as *
 * published by the Free Software Foundation; either version 3 of the    *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Test Suite is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * General Public License for more details.                              *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "testsxx.hpp"

namespace Dimension
{
  namespace Tests
  {
    Scene
    default_scene()
    {
      // Background color
      Color background = sRGB(0.0, 0.1, 0.25);
      background.filter(0.1);

      // Canvas
      Canvas canvas(768, 480);

      // Camera
      Perspective_Camera camera(
        Matrix::rotation(Vector(0.0, 1.0, 0.0))
        * Matrix::translation(Vector(0.0, 0.0, -4.0))
        * Matrix::scale(
            Vector(static_cast<double>(canvas.width())/canvas.height(), 1.0, 1.0)
          )
      );

      // Scene
      Scene scene(background, camera, canvas);

      // Objects in scene

      Sphere sphere;
      sphere.trans(inverse(Matrix::scale(Vector(1.25, 1.25, 1.25))));
      scene.push_object(sphere);

      Cube cube;
      cube.trans(inverse(Matrix::rotation(Vector(0.75, 0.0, 0.0))));
      scene.push_object(cube);

      return scene;
    }

    Display::Display(const Canvas& canvas)
      : m_display(dmnsn_new_display(canvas.dmnsn()))
    {
      if (!m_display) {
        throw Dimension_Error("Couldn't create display.");
      }
    }

    Display::~Display()
    {
      dmnsn_delete_display(m_display);
    }

    void
    Display::flush()
    {
      dmnsn_display_flush(m_display);
    }

    namespace
    {
      struct Progressbar_Payload
      {
      public:
        std::ostream* ostr;
        const Progress* progress;
      };

      void *
      progressbar_thread(void *ptr)
      {
        Progressbar_Payload* payload
          = reinterpret_cast<Progressbar_Payload*>(ptr);

        *payload->ostr << *payload->progress;

        int* ret = static_cast<int*>(std::malloc(sizeof(int)));
        if (ret) {
          *ret = 0;
        }
        return ret;
      }
    }

    Progress progressbar_async(std::ostream& ostr,
                               const Dimension::Progress& progress)
    {
      dmnsn_progress* barprogress = dmnsn_new_progress();
      if (!barprogress) {
        throw Dimension_Error("Couldn't allocate progress object.");
      }

      Progressbar_Payload* payload = new Progressbar_Payload;
      payload->ostr = &ostr;
      payload->progress = &progress;

      /* Create the worker thread */
      if (pthread_create(&barprogress->thread, NULL, &progressbar_thread,
                         reinterpret_cast<void*>(payload)) != 0)
      {
        throw Dimension_Error("Couldn't create background thread.");
      }

      return Progress(barprogress);
    }
  }

  // Print a progress bar of the progress of `progress'
  std::ostream&
  operator<<(std::ostream& ostr, const Dimension::Progress& progress)
  {
    const unsigned int increments = 32;

    ostr << "|" << std::flush;
    for (unsigned int i = 0; i < increments; ++i) {
      progress.wait(static_cast<double>(i + 1)/increments);
      ostr << "=" << std::flush;
    }
    return ostr << "|" << std::flush;
  }
}
