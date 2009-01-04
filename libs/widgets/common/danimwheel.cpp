/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-01-04
 * Description : an animated wheel
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "danimwheel.h"
#include "danimwheel.moc"

// C++ includes.

#include <cmath>

// Qt includes.

#include <QTimer>
#include <QPalette>
#include <QPainter>

// KDE includes

#include <kapplication.h>

namespace Digikam
{

class DAnimWheelPriv
{
public:

    DAnimWheelPriv()
    {
        timer = 0;
        angle = 0;
        asize = 16;
    }

    int      asize;
    int      angle;

    QTimer  *timer;
};

DAnimWheel::DAnimWheel(QObject* parent, int wheelSize)
          : QObject(parent), d(new DAnimWheelPriv)
{
    d->asize = wheelSize;
    d->timer = new QTimer(parent);

    connect(d->timer, SIGNAL(timeout()),
            this, SIGNAL(signalWheelTimeOut()));
}

DAnimWheel::~DAnimWheel()
{
    delete d;
}

void DAnimWheel::start()
{
    d->angle = 0;
    d->timer->start(100);
}

void DAnimWheel::cont()
{
    d->timer->start(500);
}

void DAnimWheel::stop()
{
    d->angle = 0;
    d->timer->stop();
}

bool DAnimWheel::running() const
{
    return d->timer->isActive();
}

int DAnimWheel::wheelSize() const
{
    return d->asize;
}

void DAnimWheel::drawWheel(QPainter *p, const QPoint& center)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->translate(center);
    d->angle = (d->angle + 10) % 360;
    p->setPen(QColor(Qt::black));//kapp->palette().color(QPalette::Active, QPalette::Text));
    p->rotate(d->angle);
    for (int i=0 ; i<12 ; i++)
    {
        p->drawLine(d->asize/2, 0, d->asize/2, 0);
        p->rotate(30);
    }
    p->restore();
}

} // namespace Digikam
