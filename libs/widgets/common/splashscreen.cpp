/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : a widget to display splash with progress bar
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "splashscreen.moc"

// Qt includes

#include <QApplication>
#include <QTimer>
#include <QFont>
#include <QString>
#include <QColor>
#include <QTime>
#include <QTextDocument>

// KDE includes

#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>

// Local includes

#include "daboutdata.h"
#include "version.h"

namespace Digikam
{

class SplashScreen::Private
{

public:

    Private()
    {
        state           = 0;
        progressBarSize = 3;
        state           = 0;
        messageAlign    = Qt::AlignLeft;
        version         = QString(digikam_version_short);
        versionColor    = Qt::white;
        messageColor    = Qt::white;
    }

    int     state;
    int     progressBarSize;
    int     messageAlign;

    QString message;
    QString version;

    QColor  messageColor;
    QColor  versionColor;

    QTime   lastStateUpdateTime;
};

SplashScreen::SplashScreen()
    : KSplashScreen(QPixmap()), d(new Private)
{
    QPixmap splash;

    if (KGlobal::mainComponent().aboutData()->appName() == QString("digikam"))
    {
        splash = KStandardDirs::locate("data","digikam/data/splash-digikam.png");
    }
    else
    {
        splash = KStandardDirs::locate("data","showfoto/data/splash-showfoto.png");
    }

    // Under Linux, only test versions has Beta stage.
    bool isBeta = !QString(digikam_version_suffix).isEmpty();

#if defined Q_OS_WIN
    isBeta = true;   // Windows version is always beta for the moment.
#elif defined Q_OS_MACX
    isBeta = true;   // MAC version is always beta for the moment.
#endif

    if (isBeta)
    {
        QPainter p(&splash);
        p.drawPixmap(412, 27, KStandardDirs::locate("data","digikam/data/logo-beta.png"));
        p.end();
    }

    setPixmap(splash);

    QTimer* const timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()),
            this, SLOT(animate()));

    timer->start(150);
}

SplashScreen::~SplashScreen()
{
    delete d;
}

void SplashScreen::setColor(const QColor& color)
{
    d->messageColor = color;
}

void SplashScreen::setAlignment(int alignment)
{
    d->messageAlign = alignment;
}

void SplashScreen::animate()
{
    QTime currentTime = QTime::currentTime();

    if (d->lastStateUpdateTime.msecsTo(currentTime) > 100)
    {
        d->state               = ((d->state + 1) % (2*d->progressBarSize-1));
        d->lastStateUpdateTime = currentTime;
    }

    update();
}

void SplashScreen::message(const QString& message)
{
    d->message = message;
    QSplashScreen::showMessage(d->message, d->messageAlign, d->messageColor); // krazy:exclude=qclasses
    animate();
    qApp->processEvents();
}

void SplashScreen::drawContents(QPainter* p)
{
    int    position = 0;
    QColor basecolor(155, 192, 231);

    // -- Draw background circles ------------------------------------

    QPainter::RenderHints hints = p->renderHints();
    p->setRenderHints(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(225, 234, 231));
    p->drawEllipse(21, 6, 9, 9);
    p->drawEllipse(32, 6, 9, 9);
    p->drawEllipse(43, 6, 9, 9);
    p->setRenderHints(hints);

    // Draw animated circles, increments are chosen
    // to get close to background's color
    // (didn't work well with QColor::light function)
    for (int i = 0; i < d->progressBarSize; ++i)
    {
        position = (d->state+i)%(2*d->progressBarSize-1);

        if (position < 3)
        {
            p->setBrush(QColor(basecolor.red()   - 18*i,
                               basecolor.green() - 28*i,
                               basecolor.blue()  - 10*i));

            p->drawEllipse(21 + position*11, 6, 9, 9);
        }
    }

    // We use a device dependant font with a fixed size.
    QFont fnt(KGlobalSettings::generalFont());
    fnt.setPixelSize(10);
    fnt.setBold(false);
    p->setFont(fnt);

    QRect r = rect();
    r.setCoords(r.x() + 60, r.y() + 4, r.width() - 10, r.height() - 10);

    // Draw message at given position, limited to 49 chars
    // If message is too long, string is truncated
    if (d->message.length() > 50)
    {
        d->message.truncate(49);
    }

    p->setPen(d->messageColor);
    p->drawText(r, d->messageAlign, d->message);

    // -- Draw version string -------------------------------------------------

    QFontMetrics fontMt(fnt);
    QRect r2 = fontMt.boundingRect(rect(), 0, d->version);
    r2.moveTopLeft(QPoint(width()-r2.width()-10, r.y()));
    p->setPen(d->versionColor);
    p->drawText(r2, Qt::AlignLeft, d->version);

    // -- Draw slogan ----------------------------------------------------------

    r = rect();
    r.setCoords(r.x() + 210, r.y() + 235, r.x() + 490, r.y() + 275);
    p->translate(r.x(), r.y());
    QTextDocument slogan;
    slogan.setDefaultTextOption(QTextOption(Qt::AlignRight | Qt::AlignVCenter));
    slogan.setHtml(DAboutData::digiKamSloganFormated().toString());
    slogan.setPageSize(r.size());
    slogan.setDefaultFont(fnt);
    slogan.drawContents(p, QRect(0, 0, r.width(), r.height()));
}

}   // namespace Digikam
