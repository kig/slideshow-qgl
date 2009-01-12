SOURCES += main.cpp slideshow.cpp
HEADERS += slideshow.h
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt
TARGET = ../bin/slideshow-qgl
QT += opengl
