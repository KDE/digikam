/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-14
 * Description : Thread actions task for finger-prints generator.
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "fingerprintstask.h"

// Qt includes
#include <QQueue>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "haar.h"
#include "haariface.h"
#include "previewloadthread.h"
#include "maintenancedata.h"

namespace Digikam
{

class FingerprintsTask::Private
{
public:

    Private()
        : data(0)
    {
    }

    MaintenanceData* data;
};

// -------------------------------------------------------

FingerprintsTask::FingerprintsTask()
    : ActionJob(),
      d(new Private)
{
}

FingerprintsTask::~FingerprintsTask()
{
    cancel();
    delete d;
}

void FingerprintsTask::setMaintenanceData(MaintenanceData* const data)
{
    d->data = data;
}

void FingerprintsTask::run()
{
    // While we have data (using this as check for non-null)
    while (d->data)
    {
        if (m_cancel)
        {
            return;
        }

        QString path = d->data->getImagePath();

        if (path.isEmpty())
        {
            break;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Updating fingerprints for file: " << path ;

        DImg dimg = PreviewLoadThread::loadFastSynchronously(path, HaarIface::preferredSize());

        if (!dimg.isNull())
        {
            // compute Haar fingerprint and store it to DB
            HaarIface haarIface;
            haarIface.indexImage(path, dimg);
        }

        QImage qimg = dimg.smoothScale(22, 22, Qt::KeepAspectRatio).copyQImage();
        emit signalFinished(qimg);
    }

    emit signalDone();
}

}  // namespace Digikam
