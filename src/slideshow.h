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
