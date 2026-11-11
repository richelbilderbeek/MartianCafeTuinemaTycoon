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
#include "qtmartiancafetuinematycoonwidget.h"

#include <QBitmap>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>
#ifdef Q_OS_WASM
// Qt Multimedia is not available for WebAssembly:
// the web version plays the beeps with the browser's Web Audio
#include <emscripten/em_js.h>

EM_JS(void, mctt_beep_in_browser, (int frequency_hz, int duration_msecs), {
  if (!window.mcttAudio) {
    window.mcttAudio = new AudioContext();
    // Browsers only allow sound after the user has clicked the page
    var resume = function() { window.mcttAudio.resume(); };
    document.addEventListener('pointerdown', resume);
    document.addEventListener('mousedown', resume);
  }
  var context = window.mcttAudio;
  if (window.mcttOscillator) {
    try { window.mcttOscillator.stop(); } catch (e) {}
    window.mcttOscillator = null;
  }
  if (context.state !== 'running') return;
  var oscillator = context.createOscillator();
  var gain = context.createGain();
  oscillator.type = 'square'; // As the PC speaker makes
  oscillator.frequency.value = frequency_hz;
  gain.gain.value = 0.05;
  oscillator.connect(gain);
  gain.connect(context.destination);
  oscillator.start();
  oscillator.stop(context.currentTime + duration_msecs / 1000);
  window.mcttOscillator = oscillator;
});
#else
#include <QAudioFormat>
#include <QAudioSink>
#include <QMediaDevices>
#endif

#include <cmath>

namespace {

/// The VCL random: a random number from 0 to (and excluding) n
int random(const int n)
{
  return QRandomGenerator::global()->bounded(n);
}

const int sample_rate = 22050;

} // namespace

void Picture::load(const QString& name)
{
  const QImage image(":/images/" + name + ".bmp");
  Q_ASSERT(!image.isNull());
  m_pixmap = QPixmap::fromImage(image);
  if (m_transparent)
  {
    const QRgb transparent_color = image.pixel(0, image.height() - 1);
    m_pixmap.setMask(QBitmap::fromImage(
      image.createMaskFromColor(transparent_color, Qt::MaskInColor)));
  }
}

QtMartianCafeTuinemaTycoonWidget::QtMartianCafeTuinemaTycoonWidget(QWidget* parent)
  : QWidget(parent)
{
  //Plaatjes inladen:
  m_background.load("Background1");
  m_arrow.load("Arrow"); m_arrow.m_left = 10; m_arrow.m_top = 140;
  m_l0.load("L0");
  m_l1.load("L1");
  m_l2.load("L2");
  m_r0.load("R0");
  m_r1.load("R1");
  m_r2.load("R2");
  m_item.load("Bar");
  m_staff.load("Barman");
  m_item.m_left = 552; m_item.m_top = 312;
  m_staff.m_left = 650; m_staff.m_top = 310;
  m_glas.load("Glas");
  m_dienblad.load("Dienblad");
  m_orkest1.load("Band1");
  m_orkest2.load("Band2");
  m_wall.load("Wall");

  //Belangrijke waarden inladen in array a:
  //0=left, 1=top, 2=status voor veld 1 en 3=voor veld 2:
          //0 = plek leeg,
          //1 = bezet maar geen bier,
          //2 = drinkende.
  const int positions[24][2] = {
    //linker aliens
    {257, 297}, {217, 337}, {421, 298}, {381, 338}, {127, 425}, { 87, 465},
    {278, 428}, {237, 469}, {424, 430}, {384, 470}, {592, 430}, {552, 470},
    //rechter aliens
    {315, 289}, {270, 334}, {477, 293}, {436, 333}, {188, 414}, {145, 457},
    {339, 417}, {295, 461}, {494, 410}, {447, 457}, {655, 417}, {615, 457}
  };
  for (int i = 0; i < 24; ++i)
  {
    m_aliens[i][0] = positions[i][0];
    m_aliens[i][1] = positions[i][1];
    m_aliens[i][2] = 0;
    m_aliens[i][3] = 0;
  }

  //Glazen
  //c[x][0]=left, top is altijd 360, [1]=of het glas er staat...
  for (int i = 0; i < 10; ++i)
  {
    m_glazen[i][0] = 768 - i * 16;
    m_glazen[i][1] = 0;
  }

  // The original form is maximized and draws the background at the top-left.
  // Here, the window has the size of the background
  setWindowTitle("Martian Cafe Tuinema Tycoon I");
  setFixedSize(m_background.m_pixmap.size());
  QPalette p = palette();
  p.setColor(QPalette::Window, Qt::black);
  setPalette(p);
  setAutoFillBackground(true);
  createPanels();

#ifndef Q_OS_WASM
  QAudioFormat format;
  format.setSampleRate(sample_rate);
  format.setChannelCount(1);
  format.setSampleFormat(QAudioFormat::Int16);
  m_audio_sink = new QAudioSink(QMediaDevices::defaultAudioOutput(), format, this);
#endif

  connect(&m_bewegingstimer, &QTimer::timeout,
          this, &QtMartianCafeTuinemaTycoonWidget::onBewegingstimer);
  m_bewegingstimer.start(500);
}

void QtMartianCafeTuinemaTycoonWidget::createPanels()
{
  // Panel1
  {
    QFrame* const panel = new QFrame(this);
    panel->setGeometry(8, 8, 249, 41);
    panel->setFrameStyle(QFrame::Panel | QFrame::Raised);
    panel->setStyleSheet("QFrame { background-color: #FF00FF; }");
    QFont font("Arial");
    font.setPixelSize(24);
    font.setBold(true);
    QLabel* const label = new QLabel("Drinks Sold:", panel);
    label->setFont(font);
    label->move(3, 4);
    label->adjustSize();
    m_drink_label = new QLabel("0", panel);
    m_drink_label->setFont(font);
    m_drink_label->move(162, 7);
    m_drink_label->resize(80, 29);
  }
  // Panel2
  {
    QFrame* const panel = new QFrame(this);
    panel->setGeometry(368, 8, 497, 73);
    panel->setFrameStyle(QFrame::Panel | QFrame::Raised);
    panel->setStyleSheet("QFrame { background-color: #FFFF00; }");
    QFont font("Arial");
    font.setPixelSize(32);
    font.setBold(true);
    QLabel* const title = new QLabel("Martian Cafe Tuinema Tycoon I", panel);
    title->setFont(font);
    title->move(7, 6);
    title->adjustSize();
    QFont small_font("Arial");
    small_font.setPixelSize(11);
    QLabel* const copyright = new QLabel("(C) 2005 Joost van den Bogaart", panel);
    copyright->setFont(small_font);
    copyright->move(8, 52);
    copyright->adjustSize();
  }
}

void QtMartianCafeTuinemaTycoonWidget::beep(const int frequency_hz, const int duration_msecs)
{
#ifdef Q_OS_WASM
  // The Win32 Beep does not play frequencies below 37 Hz
  if (frequency_hz < 37) return;
  mctt_beep_in_browser(frequency_hz, duration_msecs);
#else
  m_audio_sink->stop();
  m_beep.close();
  // The Win32 Beep does not play frequencies below 37 Hz
  if (frequency_hz < 37) return;
  const int n_samples = sample_rate * duration_msecs / 1000;
  QByteArray data(n_samples * 2, 0);
  qint16* const samples = reinterpret_cast<qint16*>(data.data());
  for (int i = 0; i < n_samples; ++i)
  {
    // A square wave, as the PC speaker makes
    const double t = static_cast<double>(i) * frequency_hz / sample_rate;
    samples[i] = (t - std::floor(t)) < 0.5 ? 4000 : -4000;
  }
  m_beep.setData(data);
  m_beep.open(QIODevice::ReadOnly);
  m_audio_sink->start(&m_beep);
#endif
}

void QtMartianCafeTuinemaTycoonWidget::onBewegingstimer()
{
  //Neerzetten met double buffering:
  //achtergrond
  m_screen = m_background.m_pixmap.toImage().convertToFormat(QImage::Format_RGB32);
  QPainter canvas(&m_screen);
  const auto draw = [&canvas](const int x, const int y, const Picture& picture)
  {
    canvas.drawPixmap(x, y, picture.m_pixmap);
  };

  //musica
  // The original uses the Win32 Beep, which waits for the beep to finish
  beep(random(500), 500);

  //staff laten bewegen als barman
  if (m_veld == 0)
  {
    m_staff.m_left = m_staff.m_left + random(50) - 25;
    if (m_staff.m_left < 600) m_staff.m_left = 600;
    if (m_staff.m_left > 700) m_staff.m_left = 700;
    draw(m_staff.m_left, m_staff.m_top, m_staff);
    draw(m_item.m_left, m_item.m_top, m_item);
  }
  else //het dwijlorkest
  {
    switch (m_dwijlorkest)
    {
      case 0:
        draw(524, 100, m_orkest1);
        m_dwijlorkest = 1;
        break;
      case 1:
        draw(525, 100, m_orkest2);
        m_dwijlorkest = 0;
        break;
    }
  }

  //dienblad:
  draw(10, 140, m_arrow);
  draw(8, 80, m_dienblad);
  const int tray_positions[11][2] = {
    {48, 80}, {63, 82}, {28, 84}, {14, 94}, {30, 94}, {47, 89},
    {61, 90}, {56, 96}, {88, 88}, {90, 95}, {74, 95}
  };
  for (int t = 0; t < m_aantalglazen; ++t)
  {
    draw(tray_positions[t][0], tray_positions[t][1], m_glas);
  }

  //de glazen vullen:
  if (random(3) == 1) m_glazen[random(10)][1] = 1;

  //de glazen tekenen als de bar er is:
  if (m_veld == 0)
  {
    for (int we = 0; we < 10; ++we)
    {
      if (m_glazen[we][1] == 1) draw(m_glazen[we][0], 360, m_glas);
    }
  }

  //Aliens: links (0-11) en rechts (12-23)
  for (int we = 0; we < 24; ++we)
  {
    const bool is_left = we < 12;
    switch (m_aliens[we][2 + m_veld]) //wat doen ze? +veld=voor veld 2
    {
      case 0: //leeg
        break;
      case 1: //geen glas -snik-
        draw(m_aliens[we][0], m_aliens[we][1], is_left ? m_l0 : m_r0);
        break;
      case 2: //wel glas, al dan niet drinken
        if (random(2) == 0)
        {
          draw(m_aliens[we][0], m_aliens[we][1], is_left ? m_l2 : m_r2);
        }
        else
        {
          draw(m_aliens[we][0], m_aliens[we][1], is_left ? m_l1 : m_r1);
        }
        break;
    }
  }

  //de muur:
  draw(673, 124, m_wall);

  //klaar
  update();
}

void QtMartianCafeTuinemaTycoonWidget::onArrowClick()
{
  //Ander veld inladen:
  if (m_veld == 0) //nu veld2 met iets anders tonen
  {
    m_veld = 1;
    m_background.load("Background2");
  }
  else // veld met bar tonen
  {
    m_veld = 0;
    m_background.load("Background1");
    m_item.m_left = 552; m_item.m_top = 312;
    m_staff.m_left = 650; m_staff.m_top = 310;
  }

  //randomizeren aliens:
  //lege plekken moeten misschien gevuld, aliens met leeg glas moeten misschien weg,
  //drinkende aliens moeten misschien weer droog.
  for (int we = 0; we < 24; ++we)
  {
    int& status = m_aliens[we][2 + m_veld];
    //vullen lege plekken:
    if (status == 0)
    {
      status = random(2); //wordt misschien 1, dat is alien met leeg glas
    }
    //weghalen aliens met leeg glas na vullen lege plekken om te voorkomen dat de tent te snel volloopt!
    if (status == 1)
    {
      status = random(2); //wordt misschien 0, dat is leeg
    }
    // en de volle glazen gaan ooit leeg
    if (status == 2)
    {
      if (random(2) == 1) status = random(2) + 1; //wordt misschien 1, kans 1 op 4
    }
  }
}

void QtMartianCafeTuinemaTycoonWidget::onMouseDown(const int x, const int y)
{
  //Aliens bier geven:
  //Bepalen welke alien het is op basis van de volgende gegevens:
          //a[we][0] (left) +7-27 voor L en +21-44 voor R
          //a[we][1] (top) +18-53 voor L en +20-61 voor R
  for (int we = 0; we < 24; ++we)
  {
    const bool is_left = we < 12;
    const int x1 = m_aliens[we][0] + (is_left ? 7 : 21);
    const int x2 = m_aliens[we][0] + (is_left ? 27 : 44);
    const int y1 = m_aliens[we][1] + (is_left ? 18 : 20);
    const int y2 = m_aliens[we][1] + (is_left ? 53 : 61);
    if (x > x1 && x < x2 && y > y1 && y < y2
      && m_aantalglazen > 0 && m_aliens[we][2 + m_veld] == 1)
    {
      m_aliens[we][2 + m_veld] = 2;
      --m_aantalglazen;
      ++m_drinkssold;
      m_drink_label->setText(QString::number(m_drinkssold));
    }
  }

  //glazen pakken
  if (m_veld == 0)
  {
    for (int we = 0; we < 10; ++we)
    {
      if (x > m_glazen[we][0] && x < m_glazen[we][0] + 15
        && y > 360 && y < 380
        && m_glazen[we][1] == 1 && m_aantalglazen < 11) //niet meer dan 11 glazen op dienblad
      {
        ++m_aantalglazen;   //glas erbij zetten
        m_glazen[we][1] = 0; //van de bar af;
      }
    }
  }
}

void QtMartianCafeTuinemaTycoonWidget::mousePressEvent(QMouseEvent* event)
{
  const QPoint pos = event->position().toPoint();
  // In the original, the arrow is a TImage on the form that is clicked
  const QRect arrow(m_arrow.m_left, m_arrow.m_top,
                    m_arrow.m_pixmap.width(), m_arrow.m_pixmap.height());
  if (arrow.contains(pos))
  {
    onArrowClick();
  }
  else
  {
    onMouseDown(pos.x(), pos.y());
  }
}

void QtMartianCafeTuinemaTycoonWidget::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  if (m_screen.isNull())
  {
    // Before the first timer tick, only the arrow is visible
    painter.drawPixmap(m_arrow.m_left, m_arrow.m_top, m_arrow.m_pixmap);
    return;
  }
  painter.drawImage(0, 0, m_screen);
}
