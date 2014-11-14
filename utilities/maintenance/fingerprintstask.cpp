/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-14
 * Description : Thread actions task for finger-prints generator.
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "fingerprintstask.moc"

// KDE includes

#include <kdebug.h>
#include <threadweaver/ThreadWeaver.h>

// Local includes

#include "dimg.h"
#include "haar.h"
#include "haariface.h"
#include "previewloadthread.h"

namespace Digikam
{

class FingerprintsTask::Private
{
public:

    Private()
    {
        cancel = false;
    }

    bool      cancel;
    QString   path;
};

// -------------------------------------------------------

FingerprintsTask::FingerprintsTask()
    : Job(0), d(new Private)
{
}

FingerprintsTask::~FingerprintsTask()
{
    slotCancel();
    delete d;
}

void FingerprintsTask::setItem(const QString& path)
{
    d->path = path;
}

void FingerprintsTask::slotCancel()
{
    d->cancel = true;
}

void FingerprintsTask::run()
{
    if(!d->cancel)
    {
        DImg dimg = PreviewLoadThread::loadFastSynchronously(d->path, HaarIface::preferredSize());

        if(d->cancel)
            return;

        if (!dimg.isNull())
        {
            // compute Haar fingerprint and store it to DB
            HaarIface haarIface;
            haarIface.indexImage(d->path, dimg);
        }

        QImage qimg = dimg.smoothScale(22, 22, Qt::KeepAspectRatio).copyQImage();
        emit signalFinished(qimg);
    }
}

}  // namespace Digikam
