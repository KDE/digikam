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

#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
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
        cancel            = false;
        previewLoadThread = new PreviewLoadThread();
    }

    bool               cancel;

    QString            path;

    PreviewLoadThread* previewLoadThread;
    HaarIface          haarIface;
};

// -------------------------------------------------------

FingerprintsTask::FingerprintsTask()
    : Job(0), d(new Private)
{
    connect(d->previewLoadThread, SIGNAL(signalImageLoaded(LoadingDescription, DImg)),
            this, SLOT(slotGotImagePreview(LoadingDescription, DImg)));
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
        LoadingDescription description(d->path, HaarIface::preferredSize(), LoadingDescription::ConvertToSRGB);
        description.rawDecodingSettings.rawPrm.sixteenBitsImage = false;
        d->previewLoadThread->load(description);
    }
}

void FingerprintsTask::slotGotImagePreview(const LoadingDescription& desc, const DImg& img)
{
    if (d->path == desc.filePath)
    {
        if (!img.isNull())
        {
            // compute Haar fingerprint
            d->haarIface.indexImage(desc.filePath, img);
        }

        QPixmap pix = DImg(img).smoothScale(22, 22, Qt::KeepAspectRatio).convertToPixmap();
        emit signalFinished(pix);
    }
}

}  // namespace Digikam
