/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-11
 * Description : a tool to show image using an OpenGL interface.
 *
 * Copyright (C) 2007-2008 by Markus Leuthold <kusi at forum dot titlis dot org>
 * Copyright (C) 2008-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "timer.h"

// Qt includes

#include <QDateTime>

// KDE includes

#include "digikam_debug.h"

namespace GenericGLViewerPlugin
{

class Timer::Private
{
public:

    Private()
    {
        meantime = 0;
    }

    QTime timer;
    int   meantime;
};

Timer::Timer()
    : d(new Private)
{
}

Timer::~Timer()
{
    delete d;
}

void Timer::start()
{
    d->timer.start();
    d->meantime = 0;
}

void Timer::at(const QString& s)
{
    d->meantime = d->timer.elapsed() - d->meantime;
    qCDebug(DIGIKAM_GENERAL_LOG) << "stopwatch:" << s << ": " << d->meantime << " ms    overall: " << d->timer.elapsed() << " ms";
}

} // namespace GenericGLViewerPlugin
