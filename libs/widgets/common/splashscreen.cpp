/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : a widget to display spash with progress bar
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

#include <qtimer.h>
#include <qfont.h>
#include <qstring.h>
#include <qcolor.h>

// KDE includes.

#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>

// Local includes.

#include "splashscreen.h"
#include "splashscreen.moc"

namespace Digikam
{

class SplashScreenPriv
{
public:

    SplashScreenPriv()
    {
        state           = 0;
        progressBarSize = 3;
        state           = 0;
        color           = Qt::gray;
        alignment       = Qt::AlignLeft;
    }

    int     state;
    int     progressBarSize;
    int     alignment;

    QString string;

    QColor  color;
};

SplashScreen::SplashScreen(const QString& splash, WFlags f)
            : KSplashScreen(QPixmap(locate("appdata", splash)), f)
{
    d = new SplashScreenPriv;

    QTimer *timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()),
            this, SLOT(animate()));

    timer->start(150);
}

SplashScreen::~SplashScreen()
{
    delete d;
}

void SplashScreen::animate()
{
    d->state = ((d->state + 1) % (2*d->progressBarSize-1));
    repaint();
}

void SplashScreen::setColor(const QColor& color)
{
    d->color = color;
}
void SplashScreen::setAlignment(int alignment)
{
    d->alignment = alignment;
}

void SplashScreen::message(const QString& message)
{
    d->string    = message;
    QSplashScreen::message(d->string, d->alignment, d->color);
    animate();
}

void SplashScreen::drawContents(QPainter* painter)
{
    int position;
    QColor basecolor(155, 192, 231);

    // Draw background circles
    painter->setPen(NoPen);
    painter->setBrush(QColor(225, 234, 231));
    painter->drawEllipse(21, 7, 9, 9);
    painter->drawEllipse(32, 7, 9, 9);
    painter->drawEllipse(43, 7, 9, 9);

    // Draw animated circles, increments are chosen
    // to get close to background's color
    // (didn't work well with QColor::light function)
    for (int i=0; i < d->progressBarSize; i++)
    {
        position = (d->state+i)%(2*d->progressBarSize-1);
        if (position < 3)
        {
            painter->setBrush(QColor(basecolor.red()  -18*i,
                                     basecolor.green()-28*i,
                                     basecolor.blue() -10*i));

            painter->drawEllipse(21+position*11, 7, 9, 9);
        }
    }

    painter->setPen(d->color);

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

    // Draw message at given position, limited to 43 chars
    // If message is too long, string is truncated
    if (d->string.length() > 40) {d->string.truncate(39); d->string += "...";}
    painter->drawText(r, d->alignment, d->string);
}

}   // namespace Digikam
