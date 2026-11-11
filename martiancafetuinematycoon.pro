# Qt version of Martian Cafe Tuinema Tycoon.
# The original C++Builder/VCL version is in the v1.0 release.
#
# Build with:
#
#   qmake martiancafetuinematycoon.pro
#   make           # nmake or jom when using MSVC
TEMPLATE = app
TARGET = martiancafetuinematycoon
QT += core gui widgets

# Qt Multimedia is not available for WebAssembly
!wasm {
  QT += multimedia
}
CONFIG += c++17

SOURCES += \
  qtmain.cpp \
  qtmartiancafetuinematycoonwidget.cpp

HEADERS += \
  qtmartiancafetuinematycoonwidget.h

RESOURCES += martiancafetuinematycoon.qrc
