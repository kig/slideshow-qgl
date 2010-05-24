/*
    QGL Slideshow - small Qt 4 OpenGL slideshow program

    Copyright (C) 2009  Ilmari Heikkinen <ilmari.heikkinen@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "slideshow.h"
#include <QMouseEvent>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <QtAlgorithms>
#include <QTime>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>


GLfloat Slideshow::rect[] = {
  0.0, 0.0, 0.0,
  1.0, 0.0, 0.0,
  0.0, 1.0, 0.0,
  1.0, 1.0, 0.0
};
GLfloat Slideshow::trect[] = {
  0.0, 1.0,
  1.0, 1.0,
  0.0, 0.0,
  1.0, 0.0
};




Slideshow::Slideshow (QString &dirname, QWidget* parent)
  : QGLWidget (parent)
{
  currentDir = QDir (dirname);
  currentEntries = QStringList ();
  slideshowRunning = false;
  texs = new GLuint[3];
  panX = panY = 0.0;
  dragX = dragY = 0;
  currentZoom = 1.0;
  fullscreen = false;
  fadeTime = 500;
  slideTime = 3000;
  resize (sizeHint ());
  timer.start();
  slideTimer = new QTimer(this);
  slideTimer->setSingleShot(true);
  connect(slideTimer, SIGNAL(timeout()), this, SLOT(update()));
}

Slideshow::~Slideshow ()
{
  glDeleteTextures(3, texs);
  delete [] texs;
}

static QRegExp fileNumber("(.*)/(.*)(\\d+)[^\\d]*$");
static QRegExp fileNumber2("(.*)/(.*)(\\d+)[^\\d]*$");
static bool caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
  int p1 = fileNumber.indexIn(s1.toLower());
  int p2 = fileNumber2.indexIn(s2.toLower());
  if (p1 >= 0 && p2 >= 0) {
    QString d1 = fileNumber.cap(1),
            d2 = fileNumber2.cap(1);
    if (d1 == d2) {
      d1 = fileNumber.cap(2);
      d2 = fileNumber2.cap(2);
      if (d1 == d2) {
        int i1 = fileNumber.cap(3).toInt(),
            i2 = fileNumber.cap(3).toInt();
        if (i1 == i2) {
          return s1.toLower() < s2.toLower();
        } else {
          return i1 < i2;
        }
      } else {
        return d1 < d2;
      }
    } else {
      return d1 < d2;
    }
  } else {
    return s1.toLower() < s2.toLower();
  }
}

void Slideshow::setDirectory (QString dirname)
{
  currentFileIndex = -1;
  currentDir = QDir (dirname);
  QStringList entries = QStringList ();
  QDirIterator dir(dirname, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

  while(dir.hasNext())
  {
    dir.next();
    if(dir.fileInfo().completeSuffix() == "jpg" || dir.fileInfo().completeSuffix() == "png")
      entries << dir.filePath();
  }
  qSort(entries.begin(), entries.end(), caseInsensitiveLessThan);

  std::cerr << "Showing images in " << dirname.toStdString() << "\n";
  currentFileIndex = -1;

  currentEntries = entries;
  if (currentEntries.count() < 1) {
    std::cerr << "No images in directory\n";
  }
  fflush(stderr);
  nextImage();
}

void Slideshow::nextImage ()
{
  gotoImage (currentFileIndex+1);
}

void Slideshow::previousImage ()
{
  gotoImage (currentFileIndex-1);
}

void Slideshow::gotoImage (int idx)
{
  QStringList entries = currentEntries;
  if (entries.count() == 0) {
    setWindowTitle(currentDir.absolutePath() + QString(" - Slideshow"));
    return;
  }
  currentFileIndex = idx % entries.count();
  if (currentFileIndex < 0) currentFileIndex += entries.count();
  setWindowTitle(entries[currentFileIndex] + QString(" - Slideshow"));
  GLuint tmp = texs[0];
  texs[0] = texs[1];
  texs[1] = tmp;
  outImage = inImage;
  glBindTexture(GL_TEXTURE_RECTANGLE, texs[0]);
  std::cerr << "Loading " << entries[currentFileIndex].toStdString() << "\n";
  fflush(stderr);
  loadTexture (currentDir.absoluteFilePath(entries[currentFileIndex]));
  timer.restart ();
  slideTimer->stop ();
  resetView ();
  update ();
}

GLfloat Slideshow::fadeInOpacity (float ms)
{
  return (ms > fadeTime) ? 1.0 : (ms / fadeTime);
}

GLfloat Slideshow::fadeOutOpacity (float ms)
{
  return (ms > fadeTime) ? 0.0 : (1.0 - (ms / fadeTime));
}

void Slideshow::resetView ()
{
  panX = 0.0;
  panY = 0.0;
  currentZoom = 1.0;
  update ();
}

void Slideshow::pan (double dx, double dy)
{
  panX -= dx / currentZoom;
  panY -= dy / currentZoom;
  update ();
}

void Slideshow::zoom (double z)
{
  double oz = currentZoom;
  currentZoom = std::max(1.0, z);
  double dz = oz / currentZoom;
  double aox = startDragX / oz + panX;
  double aoy = startDragY / oz + panY;
  panX = (aox - (aox - panX) * dz);
  panY = (aoy - (aoy - panY) * dz);
  update ();
}



void Slideshow::mouseMoveEvent (QMouseEvent* event)
{
  if (event->buttons() & (Qt::LeftButton | Qt::RightButton)) {
    int x = event->x ();
    int y = event->y ();
    int sdx = x - startDragX;
    int sdy = y - startDragY;
    dragging = dragging || (sdx*sdx + sdy*sdy > 9);
    if (dragging) {
      if (event->buttons() & Qt::LeftButton) {
        int dx = x - dragX;
        int dy = y - dragY;
        pan (dx, dy);
      } else {
        double dx = x - dragX;
        double zoomSpeed = dx > 0.0 ? 2.0 : 0.5;
        double z = currentZoom * pow( zoomSpeed, fabs(dx / 150.0));
        zoom (z);
      }
    }
    dragX = x;
    dragY = y;
  }
}

void Slideshow::mousePressEvent (QMouseEvent* event)
{
  dragging = false;
  startDragX = dragX = event->x ();
  startDragY = dragY = event->y ();
}

void Slideshow::mouseReleaseEvent (QMouseEvent* event)
{
  if (event->button() & (Qt::LeftButton | Qt::RightButton)) {
    if (!dragging) {
      if (event->button() == Qt::LeftButton)
        nextImage ();
      else
        previousImage ();
    }
  } else if (event->button() == Qt::MidButton) {
    QString dirname = QFileDialog::getExistingDirectory (this,
      tr("Open Directory"),
      currentDir.absolutePath());
    if (dirname.count() > 0)
      setDirectory (dirname);
  }
  dragging = false;
}

void Slideshow::keyPressEvent (QKeyEvent* event)
{
  int key = event->key();
  if (key == Qt::Key_S || key == Qt::Key_Space) {
    toggleSlideshow ();
  } else if (key == Qt::Key_Left) {
    previousImage ();
  } else if (key == Qt::Key_Right) {
    nextImage ();
  } else if (key == Qt::Key_Q) {
    exit (0);
  } else if (key == Qt::Key_F || key == Qt::Key_F11) {
    if (fullscreen)
      showNormal();
    else
      showFullScreen();
    fullscreen = !fullscreen;
  }
}

void Slideshow::keyReleaseEvent (QKeyEvent* event)
{
}

void Slideshow::toggleSlideshow ()
{
  if (slideshowRunning)
    stopSlideshow ();
  else
    startSlideshow ();
}

void Slideshow::startSlideshow ()
{
  slideshowRunning = true;
  slideTimer->start(slideTime);
}

void Slideshow::stopSlideshow ()
{
  slideshowRunning = false;
}

void Slideshow::resizeGL (int width, int height)
{
  glViewport (0, 0, width, height);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (0, width, height, 0, -100.0, 100.0);
  glMatrixMode (GL_MODELVIEW);
}

QSize Slideshow::minimumSizeHint () const
{
  return QSize (50, 50);
}

QSize Slideshow::sizeHint () const
{
  return QSize (800, 600);
}

static void logGLError(const char *msg)
{
  int err = 0;
  if ((err = glGetError()))
    std::cerr << "GL Error (" << msg << "): " << err << "\n";
}

void Slideshow::initializeGL ()
{

  logGLError("before initializeGL");

  glClearColor (0.0, 0.0, 0.0, 1.0);
  glDisable (GL_DEPTH_TEST);
  glEnable (GL_TEXTURE_RECTANGLE);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glGenTextures (3, texs);
  setDirectory(currentDir.absolutePath());

  logGLError("after initializeGL");
}

void Slideshow::paintGL ()
{
  int elapsed = timer.elapsed();
  
  logGLError("before paintGL");

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix ();

  glLoadIdentity ();
  double sz = std::min(width(), height());
  glScaled (currentZoom, currentZoom, 1.0);
  glTranslated (-panX + (width()-sz)/2.0, -panY + (height()-sz)/2.0, -10.0);
  glScaled (sz, sz, 1.0);

  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY);
  glVertexPointer (3, GL_FLOAT, 0, rect);

  drawImage(texs[0], inImage, fadeInOpacity(elapsed));
  drawImage(texs[1], outImage, fadeOutOpacity(elapsed));

  glPopMatrix ();

  logGLError("after paintGL");
  
  if (elapsed < fadeTime)
    update ();
  else if (slideshowRunning)
    slideTimer->start(slideTime);
  
  if (elapsed >= fadeTime + slideTime && slideshowRunning)
    nextImage ();
}

void Slideshow::drawImage (GLuint tex, QImage image, GLfloat opacity)
{
  if (opacity > 0.0) {
    glColor4f (1.0,1.0,1.0, opacity);
    glBindTexture (GL_TEXTURE_RECTANGLE, tex);
    glPushMatrix ();
      sizeImage (image);
      trect[1] = trect[3] = image.height();
      trect[2] = trect[6] = image.width();
      glTexCoordPointer (2, GL_FLOAT, 0, trect);
      glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
    glPopMatrix ();
  }
}

void Slideshow::sizeImage (QImage image)
{
  double larger = std::max(image.width(), image.height());
  double scale = 1.0;
  if (image.width() > image.height()) {
    if (width() > height()) {
      scale = std::min((double)image.width() / (double)image.height(),
          (double)width() / (double)height());
    }
  } else {
    if (width() < height()) {
      scale = std::min((double)image.height() / (double)image.width(),
          (double)height() / (double)width());
    }
  }
  glTranslated ((1.0 - scale * image.width() / larger)/2.0, (1.0 - scale * image.height() / larger)/2.0, 0.0);
  glScaled (scale * image.width() / larger, scale * image.height() / larger, 1.0);
}

void Slideshow::loadTexture (QString filename)
{
  logGLError("before loadTexture");
  QImage t = convertToGLFormat( QPixmap (filename).toImage() );
  logGLError("in convertToGLFormat");
  inImage = t;
  setTexture (&t, 1);
  logGLError("in setTexture");
}

void Slideshow::setTexture (QImage ts[], uint length)
{
  glTexParameteri (GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);
  logGLError("in glTexParameteri");
  for (uint i=0; i<length; i++) {
    QImage t = ts[i];
    int w = t.width ();
    int h = t.height ();
    glTexImage2D (GL_TEXTURE_RECTANGLE, i, 3 + (t.hasAlphaChannel() ? 1 : 0), w, h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, t.bits());
    logGLError("in glTexImage2D");
  }
}


