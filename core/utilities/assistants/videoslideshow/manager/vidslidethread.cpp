/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "vidslidethread.h"

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"

namespace Digikam
{

VidSlideThread::VidSlideThread(QObject* const parent)
    : ActionThreadBase(parent)
{
}

VidSlideThread::~VidSlideThread()
{
    cancel();
    wait();
}

void VidSlideThread::processStream(VidSlideSettings* const settings)
{
    ActionJobCollection collection;

    VidSlideTask* const t = new VidSlideTask(settings);

    connect(t, SIGNAL(signalProgress(int)),
            this, SIGNAL(signalProgress(int)));

    connect(t, SIGNAL(signalDone(bool)),
            this, SIGNAL(signalDone(bool)));

    connect(t, SIGNAL(signalMessage(QString, bool)),
            this, SIGNAL(signalMessage(QString, bool)));

    collection.insert(t, 0);

    appendJobs(collection);
}

} // namespace Digikam
