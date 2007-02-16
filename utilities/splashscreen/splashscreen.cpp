/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at gmail dot com> 
 * Date   : 2003-02-10
 * Description : a widget to display spash with progress bar
 *
 * Copyright 2003-2005 by Renchi Raju
 * Copyright 2006-2007 by Gilles Caulier
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
#include "splashscreen.moc"

#if defined(Q_WS_X11)
void qt_wait_for_window_manager( QWidget *widget );
#endif

namespace Digikam
{

class SplashScreenPriv
{
public:

    SplashScreenPriv()
    {
        currState       = 0;
        progressBarSize = 3;
        pix             = 0;
        timer           = 0;
    }

    bool     close;

    int      currAlign;
    int      currState;
    int      progressBarSize;

    QPixmap *pix;

    QTimer  *timer;

    QString  currStatus;

    QColor   currColor;
};

SplashScreen::SplashScreen(const QString& splash)
            : QWidget(0, 0, WStyle_Customize|WStyle_Splash)
{
    d = new SplashScreenPriv;
    
    QString file = locate( "appdata", splash );

    d->pix = new QPixmap(file);

    setErasePixmap( *d->pix );
    resize( d->pix->size() );
    QRect scr = QApplication::desktop()->screenGeometry();
    move( scr.center() - rect().center() );
    show();
    animate();

    d->close = false;
    
    d->timer = new QTimer;
    connect(d->timer, SIGNAL(timeout()),
            this,   SLOT(slotClose()));
    d->timer->start(1000, true);
}

SplashScreen::~SplashScreen()
{
    delete d->pix;
    delete d->timer;
    delete d;
}

void SplashScreen::finish( QWidget *mainWin )
{
#if defined(Q_WS_X11)
    qt_wait_for_window_manager( mainWin );
#endif
    d->close = true;
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
    if (!d->close) 
    {
        d->timer->start(500, true);
        return;
    }
    
    if (d->timer->isActive()) return;
    delete this;
}

void SplashScreen::message(const QString &message, int alignment,
                           const QColor &color )
{
    d->currStatus = message;
    d->currAlign = alignment;
    d->currColor = color;
    animate();
}

void SplashScreen::animate()
{
    d->currState = ((d->currState + 1) % (2*d->progressBarSize-1));
    repaint();
}

void SplashScreen::drawContents()
{
    QPixmap textPix = *d->pix;
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
    for (int i=0; i < d->progressBarSize; i++)
    {
        position = (d->currState+i)%(2*d->progressBarSize-1);
        if (position < 3)
        {
            painter->setBrush(QColor(basecolor.red()-18*i,
                              basecolor.green()-28*i,
                              basecolor.blue()-10*i));
            painter->drawEllipse(21+position*11,7,9,9);
        }
    }
    
    
    painter->setPen(d->currColor);
    
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
    painter->drawText(r, d->currAlign, d->currStatus);
}

}   // namespace Digikam

