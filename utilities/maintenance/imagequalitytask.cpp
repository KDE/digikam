/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Thread actions task for image quality sorter.
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

#include "imagequalitytask.h"

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "previewloadthread.h"
#include "imagequalitysettings.h"
#include "imgqsort.h"
#include "imageinfo.h"
#include "maintenancedata.h"

namespace Digikam
{

class ImageQualityTask::Private
{
public:

    Private()
        : imgqsort(0),
          data(0)
    {
    }

    ImageQualitySettings quality;
    ImgQSort*            imgqsort;

    MaintenanceData*     data;
};

// -------------------------------------------------------

ImageQualityTask::ImageQualityTask()
    : ActionJob(),
      d(new Private)
{
}

ImageQualityTask::~ImageQualityTask()
{
    slotCancel();
    cancel();
    delete d;
}

void ImageQualityTask::setQuality(const ImageQualitySettings& quality)
{
    d->quality = quality;
}

void ImageQualityTask::setMaintenanceData(MaintenanceData* const data)
{
    d->data = data;
}

void ImageQualityTask::slotCancel()
{
    if (d->imgqsort)
    {
        d->imgqsort->cancelAnalyse();
    }
}

void ImageQualityTask::run()
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

        // Get item preview to perform quality analysis. No need to load whole image, this will be slower.
        // TODO : check if 1024 pixels size is enough to get suitable Quality results.
        DImg dimg = PreviewLoadThread::loadFastSynchronously(path, 1024);

        if (!dimg.isNull() && !m_cancel)
        {
            // TODO : run here Quality analysis backend and store Pick Label result to DB.
            // Backend Input : d->quality as Quality analysis settings,
            //                 dimg       as reduced size image data to parse,
            //                 path    as file path to patch DB properties.
            // Result        : Backend must scan Quality of image depending of settings and compute a Quality estimation accordingly.
            //                 Finaly, using file path, DB Pick Label properties must be assigned through ImageInfo interface.
            // Warning       : All code here will run in a separated thread and must be re-entrant/thread-safe. Only pure computation
            //                 must be processed. GUI calls are prohibited. ImageInfo and DImg can be used safety in thread.

            PickLabel pick;
            d->imgqsort = new ImgQSort(dimg, d->quality, &pick);
            d->imgqsort->startAnalyse();

            ImageInfo info = ImageInfo::fromLocalFile(path);
            info.setPickLabel(pick);

            delete d->imgqsort; //delete image data after setting label
            d->imgqsort = 0;
        }
        // Dispatch progress to Progress Manager
        QImage qimg = dimg.smoothScale(22, 22, Qt::KeepAspectRatio).copyQImage();
        emit signalFinished(qimg);
    }

    emit signalDone();
}

}  // namespace Digikam
