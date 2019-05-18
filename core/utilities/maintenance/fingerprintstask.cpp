/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-14
 * Description : Thread actions task for finger-prints generator.
 *
 * Copyright (C) 2013-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QIcon>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "haar.h"
#include "haariface.h"
#include "previewloadthread.h"
#include "maintenancedata.h"
#include "similaritydb.h"
#include "similaritydbaccess.h"

namespace Digikam
{

class Q_DECL_HIDDEN FingerprintsTask::Private
{
public:

    explicit Private()
        : data(nullptr)
    {
    }

    MaintenanceData* data;
    QImage           okImage;
};

// -------------------------------------------------------

FingerprintsTask::FingerprintsTask()
    : ActionJob(),
      d(new Private)
{
    QPixmap okPix = QIcon::fromTheme(QLatin1String("dialog-ok")).pixmap(22, 22);
    d->okImage    = okPix.toImage();
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

        qlonglong id = d->data->getImageId();

        if (id == -1)
        {
            break;
        }

        ItemInfo info(id);

        if ((info.isVisible() && info.category() == DatabaseItem::Category::Image) &&
            !m_cancel                                                              &&
            (d->data->getRebuildAllFingerprints()                                  ||
             SimilarityDbAccess().db()->hasDirtyOrMissingFingerprint(info)))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Updating fingerprints for file:" << info.filePath();

            DImg dimg = PreviewLoadThread::loadFastSynchronously(info.filePath(),
                                                                 HaarIface::preferredSize());

            if (!dimg.isNull())
            {
                // compute Haar fingerprint and store it to DB
                HaarIface haarIface;
                haarIface.indexImage(info.id(), dimg);
            }

            QImage qimg = dimg.smoothScale(22, 22, Qt::KeepAspectRatio).copyQImage();
            emit signalFinished(qimg);
        }
        else
        {
            emit signalFinished(d->okImage);
        }
    }

    emit signalDone();
}

} // namespace Digikam
