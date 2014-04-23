#!/usr/bin/env python3

#########################################################################
# Copyright (C) 2011-2014 Tavian Barnes <tavianator@tavianator.com>     #
#                                                                       #
# This file is part of Dimension.                                       #
#                                                                       #
# Dimension is free software; you can redistribute it and/or modify it  #
# under the terms of the GNU General Public License as published by the #
# Free Software Foundation; either version 3 of the License, or (at     #
# your option) any later version.                                       #
#                                                                       #
# Dimension is distributed in the hope that it will be useful, but      #
# WITHOUT ANY WARRANTY; without even the implied warranty of            #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     #
# General Public License for more details.                              #
#                                                                       #
# You should have received a copy of the GNU General Public License     #
# along with this program.  If not, see <http://www.gnu.org/licenses/>. #
#########################################################################

from PyQt4 import QtCore, QtGui, QtOpenGL

class Preview(QtOpenGL.QGLWidget):
  """Surface that the scene is rendered to."""
  def __init__(self, parent, canvas, future):
    QtOpenGL.QGLWidget.__init__(self, parent)
    self.canvas = canvas
    self.future = future

  def paintGL(self):
    try:
      self.future.pause()
      self.canvas.draw_GL()
    except:
      self.future.cancel()
      self.parent().close()
      raise
    finally:
      self.future.resume()

class PreviewWindow(QtGui.QMainWindow):
  """Main window for a rendering preview."""
  def __init__(self, canvas, future):
    QtGui.QMainWindow.__init__(self)
    self.canvas = canvas
    self.future = future

    self.setMinimumSize(canvas.width, canvas.height)
    self.setMaximumSize(canvas.width, canvas.height)
    self.widget = Preview(self, canvas, future)
    self.setCentralWidget(self.widget)

    self.render_timer = QtCore.QTimer(self)
    self.render_timer.timeout.connect(self.update_preview)
    self.render_timer.start(0)

  @QtCore.pyqtSlot()
  def update_preview(self):
    self.widget.updateGL()
    if self.future.progress() == 1:
      self.render_timer.stop()
      self.close_timer = QtCore.QTimer(self)
      self.close_timer.singleShot(1000, self.close)

  @QtCore.pyqtSlot()
  def close(self):
    QtCore.QCoreApplication.instance().quit()

def show_preview(canvas, future):
  app = QtGui.QApplication(["Dimension Preview"])
  window = PreviewWindow(canvas, future)
  window.show()
  app.exec()
  del window
