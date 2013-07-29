/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-27
 * Description : Widget showing a throbber ("working" animation)
 *
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 * Copyright (C) 2010-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "workingwidget.moc"

// Qt includes

#include <QImage>
#include <QTimer>
#include <QList>
#include <QPixmap>

// KDE includes

#include <kiconloader.h>
#include <kpixmapsequence.h>

namespace Digikam
{

class WorkingWidget::Private
{

public:

    Private()
    {
        currentPixmap = 0;
    }

    KPixmapSequence pixmaps;
    int             currentPixmap;
    QTimer          timer;
};

WorkingWidget::WorkingWidget(QWidget* const parent)
    : QLabel(parent), d(new Private)
{
    QPixmap pix(KIconLoader::global()->iconPath("process-working.png", KIconLoader::Dialog));

    d->pixmaps = KPixmapSequence(pix);

    connect(&d->timer, SIGNAL(timeout()),
            this, SLOT(changeImage()));

    d->timer.start(100);
    changeImage();
}

WorkingWidget::~WorkingWidget()
{
    delete d;
}

void WorkingWidget::changeImage()
{
    if (d->currentPixmap >= d->pixmaps.frameCount())
    {
        d->currentPixmap = 0;
    }

    setPixmap(d->pixmaps.frameAt(d->currentPixmap));

    d->currentPixmap++;

    emit animationStep();
}

void WorkingWidget::toggleTimer(bool turnOn)
{
    if (turnOn && !d->timer.isActive())
    {
        d->timer.start();
    }
    else if (!turnOn && d->timer.isActive())
    {
        d->timer.stop();
    }
}

} // namespace Digikam
