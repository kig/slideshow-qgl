/*

Copyright (C) 2009  Ilmari Heikkinen <ilmari.heikkinen@gmail.com>

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

*/

#include "slideshow.h"
#include <QMouseEvent>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <QTime>
#include <QDir>
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
	texs = new GLuint[3];
	panX = panY = 0.0;
	dragX = dragY = 0;
	currentZoom = 1.0;
	fadeTime = 500.0;
	resize (sizeHint ());
	timer.start();
}

Slideshow::~Slideshow ()
{
	glDeleteTextures(3, texs);
	delete [] texs;
}

void Slideshow::setDirectory (QString dirname)
{
	currentFileIndex = -1;
	currentDir = QDir (dirname, "*.jpg *.png", QDir::Name | QDir::IgnoreCase, QDir::Files);
	std::cerr << "Showing images in " << dirname.toStdString() << "\n";
	if (currentDir.count() < 1) {
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
	QStringList entries = currentDir.entryList ();
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
	glBindTexture(GL_TEXTURE_2D, texs[0]);
	std::cerr << "Loading " << entries[currentFileIndex].toStdString() << "\n";
	fflush(stderr);
	loadTexture (currentDir.absoluteFilePath(entries[currentFileIndex]));
	timer.restart ();
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
}

void Slideshow::keyReleaseEvent (QKeyEvent* event)
{
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

void Slideshow::initializeGL ()
{
	glClearColor (0.0, 0.0, 0.0, 1.0);
	glDisable (GL_DEPTH_TEST);
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
	glGenTextures (3, texs);
	setDirectory(currentDir.absolutePath());
}

void Slideshow::paintGL ()
{
	int elapsed = timer.elapsed();
	
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
	glTexCoordPointer (2, GL_FLOAT, 0, trect);
	
	drawImage(texs[0], inImage, fadeInOpacity(elapsed));
	drawImage(texs[1], outImage, fadeOutOpacity(elapsed));
	
	glPopMatrix ();
	
	int err = glGetError();
	if (err != 0)
		std::cerr << "GL Error: " << err << "\n";
	if (elapsed < fadeTime) update ();
}

void Slideshow::drawImage (GLuint tex, QImage image, GLfloat opacity)
{
	if (opacity > 0.0) {
		glColor4f (1.0,1.0,1.0, opacity);
		glBindTexture (GL_TEXTURE_2D, tex);
		glPushMatrix ();
			sizeImage (image);
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
	QImage t = convertToGLFormat( QPixmap (filename).toImage() );
	inImage = t;
// 	QImage *ts;
// 	uint len;
// 	mipmap (t, &ts, &len);
// 	setTexture (ts, len);
	setTexture (&t, 1);
// 	delete [] ts;
}

void Slideshow::mipmap (QImage t, QImage **ts, uint *len)
{
	int larger = (t.width () > t.height ()) ? t.width() : t.height();
	int sz;
	if (larger <= 0)
		sz = 1;
	else
		sz = 1 + (int)floor(log2(larger));
	fprintf(stderr, "%d x %d\n", t.width(), t.height());
	fflush(stderr);
	QImage *res = new QImage[sz];
	res[0] = t;
	for (int i=1; i<sz; i++) {
		int w = std::max(1, t.width()/2);
		int h = std::max(1, t.height()/2);
		fprintf(stderr, "%d x %d\n", w, h);
		fflush(stderr);
		t = t.scaled (w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		res[i] = t;
	}
	*ts = res;
	*len = (uint)sz;
}

void Slideshow::setTexture (QImage ts[], uint length)
{
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	for (uint i=0; i<length; i++) {
		QImage t = ts[i];
		int w = t.width ();
		int h = t.height ();
		glTexImage2D (GL_TEXTURE_2D, i, 3 + (t.hasAlphaChannel() ? 1 : 0), w, h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, t.bits());
	}
}


