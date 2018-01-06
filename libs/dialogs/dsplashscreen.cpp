/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : a widget to display splash with progress bar
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dsplashscreen.h"

// Qt includes

#include <QApplication>
#include <QTimer>
#include <QFont>
#include <QString>
#include <QColor>
#include <QTime>
#include <QTextDocument>
#include <QFontDatabase>
#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"
#include "daboutdata.h"
#include "digikam_version.h"

namespace Digikam
{

class DSplashScreen::Private
{

public:

    Private()
    {
        state               = 0;
        progressBarSize     = 3;
        state               = 0;
        messageAlign        = Qt::AlignLeft;
        version             = QLatin1String(digikam_version_short);
        versionColor        = Qt::white;
        messageColor        = Qt::white;
        lastStateUpdateTime = QTime::currentTime();
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

DSplashScreen::DSplashScreen()
    : QSplashScreen(QPixmap()),
      d(new Private)
{
    QPixmap splash;

    if (QApplication::applicationName() == QLatin1String("digikam"))
    {
        splash = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/splash-digikam.png"));
    }
    else
    {
        splash = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("showfoto/data/splash-showfoto.png"));
    }

    // Under Linux, only test versions has Beta stage.
    bool isBeta = !QString::fromUtf8(digikam_version_suffix).isEmpty();

#if defined Q_OS_WIN
    isBeta = true;   // Windows version is always beta for the moment.
#elif defined Q_OS_OSX
    isBeta = true;   // MAC version is always beta for the moment.
#endif

    if (isBeta)
    {
        QPainter p(&splash);
        p.drawPixmap(380, 27, QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-beta.png")));
        p.end();
    }

    setPixmap(splash);

    QTimer* const timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()),
            this, SLOT(slotAnimate()));

    timer->start(150);
}

DSplashScreen::~DSplashScreen()
{
    delete d;
}

void DSplashScreen::setColor(const QColor& color)
{
    d->messageColor = color;
}

void DSplashScreen::setAlignment(int alignment)
{
    d->messageAlign = alignment;
}

void DSplashScreen::setMessage(const QString& message)
{
    d->message = message;
    QSplashScreen::showMessage(d->message, d->messageAlign, d->messageColor); // krazy:exclude=qclasses
    slotAnimate();
    qApp->processEvents();
}

void DSplashScreen::slotAnimate()
{
    QTime currentTime = QTime::currentTime();

    if (d->lastStateUpdateTime.msecsTo(currentTime) > 100)
    {
        d->state               = ((d->state + 1) % (2 * d->progressBarSize - 1));
        d->lastStateUpdateTime = currentTime;
    }

    update();
}

void DSplashScreen::drawContents(QPainter* p)
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

    // -- Draw animated circles --------------------------------------

    // Increments are chosen to get close to background's color
    // (didn't work well with QColor::light function)

    for (int i = 0 ; i < d->progressBarSize ; i++)
    {
        position = (d->state + i) % (2 * d->progressBarSize - 1);

        if (position < 3)
        {
            p->setBrush(QColor(basecolor.red()   - 18*i,
                               basecolor.green() - 28*i,
                               basecolor.blue()  - 10*i));

            p->drawEllipse(21 + position*11, 6, 9, 9);
        }
    }

    // We use a device dependant font with a fixed size.
    QFont fnt(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    fnt.setPixelSize(10);
    fnt.setBold(false);
    p->setFont(fnt);

    QRect r = rect();
    r.setCoords(r.x() + 60, r.y() + 4, r.width() - 10, r.height() - 10);

    // -- Draw message --------------------------------------------------------

    // Message is draw at given position, limited to 49 chars
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

    // NOTE: splashscreen size is 469*288 pixels
    r = rect();
    r.setCoords(r.x() + 210, r.y() + 225, r.x() + 462, r.y() + 275);
    p->translate(r.x(), r.y());
    QTextDocument slogan;
    slogan.setDefaultTextOption(QTextOption(Qt::AlignRight | Qt::AlignVCenter));
    slogan.setHtml(DAboutData::digiKamSloganFormated());
    slogan.setPageSize(r.size());
    slogan.setDefaultFont(fnt);
    slogan.drawContents(p, QRect(0, 0, r.width(), r.height()));
}

} // namespace Digikam
