/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-09
 * Description : resize image job.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Andi Clemens <andi dot clemens at googlemail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "imageresizejob.h"

// Qt includes

#include <QDir>
#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "previewloadthread.h"
#include "dmetadata.h"

namespace Digikam
{

ImageResizeJob::ImageResizeJob(int* count)
    : ActionJob(),
      m_settings(0),
      m_count(count)
{
}

ImageResizeJob::~ImageResizeJob()
{
}

void ImageResizeJob::run()
{
    emit signalStarted();

    QString errString;

    emit startingResize(m_orgUrl);

    m_mutex.lock();
    (*m_count)++;
    m_mutex.unlock();

    int percent = (int)(((float)(*m_count)/(float)m_settings->itemsList.count())*100.0);

    if (imageResize(m_settings, m_orgUrl, m_destName, errString))
    {
        QUrl emailUrl(QUrl::fromLocalFile(m_destName));
        emit finishedResize(m_orgUrl, emailUrl, percent);
    }
    else
    {
        emit failedResize(m_orgUrl, errString, percent);
    }

    if (m_settings->itemsList.count() == *m_count)
    {
        m_mutex.lock();
        *m_count = 0;
        m_mutex.unlock();
    }

    emit signalDone();
}

bool ImageResizeJob::imageResize(MailSettings* const settings,
                                 const QUrl& orgUrl,
                                 const QString& destName,
                                 QString& err)
{
    QFileInfo fi(orgUrl.toLocalFile());

    if (!fi.exists() || !fi.isReadable())
    {
        err = i18n("Error opening input file");
        return false;
    }

    QFileInfo tmp(destName);
    QFileInfo tmpDir(tmp.dir().absolutePath());

    qCDebug(DIGIKAM_GENERAL_LOG) << "tmpDir: " << tmp.dir().absolutePath();

    if (!tmpDir.exists() || !tmpDir.isWritable())
    {
        err = i18n("Error opening temporary folder");
        return false;
    }

    DImg img = PreviewLoadThread::loadFastSynchronously(orgUrl.toLocalFile(), settings->imageSize);

    if (img.isNull())
    {
        img.load(orgUrl.toLocalFile());
    }

    uint sizeFactor = settings->imageSize;

    if (!img.isNull())
    {
        uint w = img.width();
        uint h = img.height();

        if (w > sizeFactor || h > sizeFactor)
        {
            if (w > h)
            {
                h = (uint)((double)(h * sizeFactor) / w);

                if (h == 0) h = 1;

                w = sizeFactor;
                Q_ASSERT(h <= sizeFactor);
            }
            else
            {
                w = (uint)((double)(w * sizeFactor) / h);

                if (w == 0) w = 1;

                h = sizeFactor;
                Q_ASSERT(w <= sizeFactor);
            }

            DImg scaledImg = img.smoothScale(w, h, Qt::IgnoreAspectRatio);

            if (scaledImg.width() != w || scaledImg.height() != h)
            {
                err = i18n("Cannot resize image. Aborting.");
                return false;
            }

            img = scaledImg;
        }

        if (settings->format() == QLatin1String("JPEG"))
        {
            img.setAttribute(QLatin1String("quality"), settings->imageCompression);

            if (!img.save(destName, settings->format()))
            {
                err = i18n("Cannot save resized image (JPEG). Aborting.");
                return false;
            }
        }
        else if (settings->format() == QLatin1String("PNG"))
        {
            if (!img.save(destName, settings->format()))
            {
                err = i18n("Cannot save resized image (PNG). Aborting.");
                return false;
            }
        }

        if (settings->removeMetadata)
        {
            DMetadata meta;

            if (!meta.load(destName))
            {
                return false;
            }

            meta.clearExif();
            meta.clearIptc();
            meta.clearXmp();

            if (!meta.save(destName))
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

}  // namespace Digikam
