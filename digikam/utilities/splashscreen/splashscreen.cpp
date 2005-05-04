/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-10
 * Description :
 *
 * Copyright 2003 by Renchi Raju
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes. 
 
#include <qpixmap.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qpainter.h>

// KDE includes.

#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>

// Local includes.

#include "splashscreen.h"

SplashScreen::SplashScreen(const QString& splash)
            : QWidget(0, 0, WStyle_Customize|WStyle_Splash)
{
    currState_ = 0;
    progressBarSize_ = 3;
    
    QString file = locate( "appdata", splash );

    pix_ = new QPixmap(file);

    setErasePixmap( *pix_ );
    resize( pix_->size() );
    QRect scr = QApplication::desktop()->screenGeometry();
    move( scr.center() - rect().center() );
    show();
    animate();

    close_ = false;
    
    timer_ = new QTimer;
    connect(timer_, SIGNAL(timeout()),
            this,   SLOT(slotClose()));
    timer_->start(1000, true);
}

SplashScreen::~SplashScreen()
{
    delete pix_;
    delete timer_;
}

#if defined(Q_WS_X11)
void qt_wait_for_window_manager( QWidget *widget );
#endif

void SplashScreen::finish( QWidget *mainWin )
{
#if defined(Q_WS_X11)
    qt_wait_for_window_manager( mainWin );
#endif
    close_ = true;
    slotClose();
}

void SplashScreen::repaint()
{
    drawContents();
    QWidget::repaint();
    QApplication::flush();
}

void SplashScreen::mousePressEvent( QMouseEvent * )
{
    hide();
}

void SplashScreen::slotClose()
{
    if (!close_) {
        timer_->start(500, true);
        return;
    }
    
    if (timer_->isActive()) return;
    delete this;
}

void SplashScreen::message(const QString &message, int alignment,
                           const QColor &color )
{
    currStatus_ = message;
    currAlign_ = alignment;
    currColor_ = color;
    animate();
}


void SplashScreen::animate()
{
    currState_ = ((currState_ + 1) % (2*progressBarSize_-1));
    repaint();
}

void SplashScreen::drawContents()
{
    QPixmap textPix = *pix_;
    QPainter painter(&textPix, this);
    drawContents(&painter);
    setErasePixmap(textPix);
}

void SplashScreen::drawContents( QPainter *painter )
{
    int position;
    QColor basecolor (155, 192, 231);

    // Draw background circles
    painter->setPen(NoPen);
    painter->setBrush(QColor(225,234,231));
    painter->drawEllipse(21,7,9,9);
    painter->drawEllipse(32,7,9,9);
    painter->drawEllipse(43,7,9,9);
    
    // Draw animated circles, increments are chosen
    // to get close to background's color
    // (didn't work well with QColor::light function)
    for (int i=0; i < progressBarSize_; i++)
    {
        position = (currState_+i)%(2*progressBarSize_-1);
        if (position < 3)
        {
            painter->setBrush(QColor(basecolor.red()-18*i,
                              basecolor.green()-28*i,
                              basecolor.blue()-10*i));
            painter->drawEllipse(21+position*11,7,9,9);
        }
    }
    
    
    painter->setPen(currColor_);
    
    QFont fnt(KGlobalSettings::generalFont());
    int fntSize = fnt.pointSize();
    if (fntSize > 0)
    {
        fnt.setPointSize(fntSize-2);
    }
    else
    {
        fntSize = fnt.pixelSize();
        fnt.setPixelSize(fntSize-2);
    }
    painter->setFont(fnt);
   
    QRect r = rect();
    r.setRect( r.x() + 59, r.y() + 5, r.width() - 10, r.height() - 10 );
    painter->drawText(r, currAlign_, currStatus_);
}

#include "splashscreen.moc"
