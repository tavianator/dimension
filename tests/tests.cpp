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

#include "tests.hpp"

namespace Dimension
{
  namespace Tests
  {
    Scene
    default_scene()
    {
      // Canvas
      Canvas canvas(768, 480);

      // Camera
      Perspective_Camera camera;
      camera.trans(
        Matrix::rotation(Vector(0.0, 1.0, 0.0))
        * Matrix::translation(Vector(0.0, 0.0, -4.0))
        * Matrix::scale(
            Vector(static_cast<double>(canvas.width())/canvas.height(),
                   1.0,
                   1.0)
          )
      );

      // Scene
      Scene scene(camera, canvas);

      // Objects in scene

      Sphere sphere;
      sphere.trans(inverse(Matrix::scale(Vector(1.25, 1.25, 1.25))));
      scene.objects().push(sphere);

      Cube cube;
      cube.trans(inverse(Matrix::rotation(Vector(0.75, 0.0, 0.0))));
      scene.objects().push(cube);

      // Background color
      Color background = sRGB(0.0, 0.1, 0.25);
      background.filter(0.1);
      scene.background(background);

      return scene;
    }

    namespace
    {
      /* XIfEvent callback */
      Bool
      WaitForNotify(::Display *d, XEvent *e, char *arg)
      {
        return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
      }
    }

    Display::Display(const Canvas& canvas)
    {
      int attributeList[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE, 1,
        GLX_GREEN_SIZE, 1,
        GLX_BLUE_SIZE, 1,
        None
      };
      XVisualInfo *vi;
      XSetWindowAttributes swa;

      /* Get an X connection */
      m_dpy = XOpenDisplay(0);
      if (!m_dpy) {
        throw Dimension_Error("Couldn't create display.");
      }

      /* Get an appropriate visual */
      vi = glXChooseVisual(m_dpy, DefaultScreen(m_dpy),
                           attributeList);
      if (!vi) {
        XCloseDisplay(m_dpy);
        throw Dimension_Error("Couldn't create display.");
      }

      /* Create a GLX context */
      m_cx = glXCreateContext(m_dpy, vi, 0, GL_TRUE);
      if (!m_cx) {
        XCloseDisplay(m_dpy);
        throw Dimension_Error("Couldn't create display.");
      }

      /* Create a color map */
      m_cmap = XCreateColormap(m_dpy,
                               RootWindow(m_dpy, vi->screen),
                               vi->visual, AllocNone);
      if (!m_cmap) {
        glXDestroyContext(m_dpy, m_cx);
        XCloseDisplay(m_dpy);
        throw Dimension_Error("Couldn't create display.");
      }

      /* Create a window */
      swa.colormap = m_cmap;
      swa.border_pixel = 0;
      swa.event_mask = StructureNotifyMask;
      m_win = XCreateWindow(m_dpy,
                            RootWindow(m_dpy, vi->screen),
                            0, 0, canvas.width(), canvas.height(),
                            0, vi->depth, InputOutput, vi->visual,
                            CWBorderPixel|CWColormap|CWEventMask, &swa);
      if (!m_win) {
        XFreeColormap(m_dpy, m_cmap);
        glXDestroyContext(m_dpy, m_cx);
        XCloseDisplay(m_dpy);
        throw Dimension_Error("Couldn't create display.");
      }

      XStoreName(m_dpy, m_win, "glX");

      XMapWindow(m_dpy, m_win);
      XIfEvent(m_dpy, &m_event, &WaitForNotify, (char*)m_win);

      /* Connect the context to the window */
      glXMakeCurrent(m_dpy, m_win, m_cx);
    }

    Display::~Display()
    {
      XDestroyWindow(m_dpy, m_win);
      XFreeColormap(m_dpy, m_cmap);
      glXDestroyContext(m_dpy, m_cx);
      XCloseDisplay(m_dpy);
    }

    void
    Display::flush()
    {
      glFlush();
      glXSwapBuffers(m_dpy, m_win);
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
