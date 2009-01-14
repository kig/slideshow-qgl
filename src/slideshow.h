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


#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <QGLWidget>
#include <QTime>
#include <QDir>


class Slideshow : public QGLWidget
{
	Q_OBJECT

public:
	Slideshow (QString &dirname, QWidget* parent = 0);
	~Slideshow ();

	QSize minimumSizeHint () const;
	QSize sizeHint () const;

	void nextImage ();
	void previousImage ();
	void gotoImage (int idx);

	void resetView ();
	void pan (double dx, double dy);
	void zoom (double z);

	void setDirectory (QString dirname);

	float fadeTime;

	QDir currentDir;
	int currentFileIndex;

protected:
	void initializeGL ();
	void paintGL ();
	void resizeGL (int width, int height);

	void setTexture (QImage ts[], uint length);
	void mipmap (QImage t, QImage **ts, uint *length);

	void loadTexture (QString filename);

	void drawImage (GLuint tex, QImage image, GLfloat opacity);
	void sizeImage (QImage image);

	void mouseMoveEvent (QMouseEvent* event);
	void mousePressEvent (QMouseEvent* event);
	void mouseReleaseEvent (QMouseEvent* event);
	void keyPressEvent (QKeyEvent* event);
	void keyReleaseEvent (QKeyEvent* event);

	GLfloat fadeInOpacity (float ms);
	GLfloat fadeOutOpacity (float ms);

	QTime timer;

	QImage inImage, outImage;

	int dragX;
	int dragY;
	int startDragX;
	int startDragY;
	bool dragging;

	double panX;
	double panY;

	double currentZoom;

	GLuint *texs;

	static GLfloat rect[];
	static GLfloat trect[];
};

#endif
