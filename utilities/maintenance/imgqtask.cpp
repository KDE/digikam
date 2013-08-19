/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Thread actions task for image quality sorter.
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

#include "imgqtask.moc"

// KDE includes

#include <kdebug.h>
#include <threadweaver/ThreadWeaver.h>

// Local includes

#include "dimg.h"
#include "previewloadthread.h"
#include "imagequalitysettings.h"

namespace Digikam
{

class ImgQTask::Private
{
public:

    Private()
    {
        cancel  = false;
    }

    bool                 cancel;
    ImageQualitySettings quality;
    QString              path;
};

// -------------------------------------------------------

ImgQTask::ImgQTask()
    : Job(0), d(new Private)
{
}

ImgQTask::~ImgQTask()
{
    slotCancel();
    delete d;
}

void ImgQTask::setItem(const QString& path, const ImageQualitySettings& quality)
{
    d->path    = path;
    d->quality = quality;
}

void ImgQTask::slotCancel()
{
    d->cancel = true;
}

void ImgQTask::run()
{
    if(!d->cancel)
    {
        // Get item preview to perform quality analysis. No need to load whole image, this will be slower.
        // TODO : check if 1024 pixels size is enough to get suitable Quality results.
        LoadingDescription description(d->path, 1024, LoadingDescription::ConvertToSRGB);
        description.rawDecodingSettings.optimizeTimeLoading();
        description.rawDecodingSettings.rawPrm.sixteenBitsImage   = false;
        description.rawDecodingSettings.rawPrm.halfSizeColorImage = true;
        description.rawDecodingHint                               = LoadingDescription::RawDecodingTimeOptimized;
        DImg dimg = PreviewLoadThread::loadSynchronously(description);

        if(d->cancel)
            return;

        if (!dimg.isNull())
        {
            // TODO : run here Quality analysis backend and store Pick Label result to DB
            // Warning : all code here run in a separated thread must be re-entrant.
        }

        // Dispatch progress to Progress Manager
        QImage qimg = dimg.smoothScale(22, 22, Qt::KeepAspectRatio).copyQImage();
        emit signalFinished(qimg);
    }
}

}  // namespace Digikam
