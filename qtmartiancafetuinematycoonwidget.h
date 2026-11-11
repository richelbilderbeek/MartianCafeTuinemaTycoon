/*
  Martian Cafe Tuinema Tycoon, bartender game
  Copyright (C) 2005  Joost van den Bogaart

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
// Qt port of the C++Builder/VCL version (MCT1.cpp, MCT1.h and MCT1.dfm,
// see the v1.0 release). The game logic is kept as close to the original
// as possible.
#ifndef QTMARTIANCAFETUINEMATYCOONWIDGET_H
#define QTMARTIANCAFETUINEMATYCOONWIDGET_H

#include <QBuffer>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QTimer>
#include <QWidget>

class QAudioSink;
class QLabel;

/// The equivalent of a VCL TImage with AutoSize = true:
/// a picture at a position
struct Picture
{
  explicit Picture(bool transparent = true) : m_transparent{transparent} {}

  /// Load a picture from the resources, e.g. 'Bar'.
  /// As in the VCL, a transparent picture uses the colour of its
  /// bottom-left pixel as the transparent colour
  void load(const QString& name);

  bool m_transparent;
  QPixmap m_pixmap;
  int m_left = 0;
  int m_top = 0;
};

class QtMartianCafeTuinemaTycoonWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QtMartianCafeTuinemaTycoonWidget(QWidget* parent = nullptr);

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

private:
  int m_aliens[24][4]; //array met info over alle aliens
  int m_veld = 0;      //welk veld hij laat zien:
                       // 0 = veld met bar
                       // 1 = veld met iets anders
  int m_glazen[10][2]; //array met glazen
  int m_aantalglazen = 0; //aantal glazen op dienblad
  int m_dwijlorkest = 0;  //welk dwijlorkestplaatje
  int m_drinkssold = 0;

  Picture m_background{false};
  Picture m_arrow;
  Picture m_l0;
  Picture m_l1;
  Picture m_l2;
  Picture m_r0;
  Picture m_r1;
  Picture m_r2;
  Picture m_item;
  Picture m_staff;
  Picture m_glas;
  Picture m_dienblad;
  Picture m_orkest1;
  Picture m_orkest2;
  Picture m_wall;

  /// What is drawn on the form, i.e. the VCL ImageCanvas
  QImage m_screen;

  QTimer m_bewegingstimer;

  QLabel* m_drink_label;

  /// Plays the beeps, unused in the web version, which uses Web Audio
  QAudioSink* m_audio_sink = nullptr;
  QBuffer m_beep;

  void createPanels();

  /// Like the Win32 Beep, yet without waiting for the beep to finish
  void beep(int frequency_hz, int duration_msecs);

  void onBewegingstimer();
  void onArrowClick();
  void onMouseDown(int x, int y);
};

#endif // QTMARTIANCAFETUINEMATYCOONWIDGET_H
