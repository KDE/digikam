/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-12-31
 * Description : time adjust actions using threads.
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (c) 2018      by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "timeadjusttask.h"

// C ANSI includes

extern "C"
{
#include <unistd.h>
#include <utime.h>
}

// Qt includes

#include <QFile>

// Local includes

#include "dmetadata.h"
#include "digikam_debug.h"
#include "dinfointerface.h"
#include "timeadjustlist.h"
#include "timeadjustthread.h"

namespace Digikam
{

class Q_DECL_HIDDEN TimeAdjustTask::Private
{
public:

    explicit Private()
    {
    }

    QUrl                  url;

    // Settings from GUI.
    TimeAdjustContainer   settings;

    // Map of item urls and Updated Timestamps.
    QMap<QUrl, QDateTime> itemsMap;
};

TimeAdjustTask::TimeAdjustTask(const QUrl& url)
    : ActionJob(),
      d(new Private)
{
    d->url   = url;
}

TimeAdjustTask::~TimeAdjustTask()
{
    cancel();
    delete d;
}

void TimeAdjustTask::setSettings(const TimeAdjustContainer& settings)
{
    d->settings = settings;
}

void TimeAdjustTask::setItemsMap(const QMap<QUrl, QDateTime>& itemsMap)
{
    d->itemsMap = itemsMap;
}

void TimeAdjustTask::run()
{
    if (m_cancel)
        return;

    QDateTime dt = d->itemsMap.value(d->url);

    if (!dt.isValid()) return;

    emit signalProcessStarted(d->url);

    bool metadataChanged = d->settings.updEXIFModDate || d->settings.updEXIFOriDate ||
                           d->settings.updEXIFDigDate || d->settings.updEXIFThmDate ||
                           d->settings.updIPTCDate    || d->settings.updXMPDate;

    int status = TimeAdjustList::NOPROCESS_ERROR;

    if (metadataChanged)
    {
        bool ret = true;

        DMetadata meta;

        ret &= meta.load(d->url.path());

        if (ret)
        {
            if (meta.canWriteExif(d->url.path()))
            {
                if (d->settings.updEXIFModDate)
                {
                    ret &= meta.setExifTagString("Exif.Image.DateTime",
                                                 dt.toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
                }

                if (d->settings.updEXIFOriDate)
                {
                    ret &= meta.setExifTagString("Exif.Photo.DateTimeOriginal",
                                                 dt.toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
                }

                if (d->settings.updEXIFDigDate)
                {
                    ret &= meta.setExifTagString("Exif.Photo.DateTimeDigitized",
                                                 dt.toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
                }

                if (d->settings.updEXIFThmDate)
                {
                    ret &= meta.setExifTagString("Exif.Image.PreviewDateTime",
                                                 dt.toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
                }
            }
            else if (d->settings.updEXIFModDate || d->settings.updEXIFOriDate || 
                     d->settings.updEXIFDigDate || d->settings.updEXIFThmDate)
            {
                ret = false;
            }

            if (d->settings.updIPTCDate)
            {
                if (meta.canWriteIptc(d->url.path()))
                {
                    ret &= meta.setIptcTagString("Iptc.Application2.DateCreated",
                                                 dt.date().toString(Qt::ISODate));
                    ret &= meta.setIptcTagString("Iptc.Application2.TimeCreated",
                                                 dt.time().toString(Qt::ISODate));
                }
                else
                {
                    ret = false;
                }
            }

            if (d->settings.updXMPDate)
            {
                if (meta.supportXmp() && meta.canWriteXmp(d->url.path()))
                {
                    ret &= meta.setXmpTagString("Xmp.exif.DateTimeOriginal",
                                                dt.toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
                    ret &= meta.setXmpTagString("Xmp.photoshop.DateCreated",
                                                dt.toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
                    ret &= meta.setXmpTagString("Xmp.tiff.DateTime",
                                                dt.toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
                    ret &= meta.setXmpTagString("Xmp.xmp.CreateDate",
                                                dt.toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
                    ret &= meta.setXmpTagString("Xmp.xmp.MetadataDate",
                                                dt.toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
                    ret &= meta.setXmpTagString("Xmp.xmp.ModifyDate",
                                                dt.toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
                }
                else
                {
                    ret = false;
                }
            }

            ret &= meta.save(d->url.path());

            if (!ret)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to update metadata in file " << d->url.fileName();
            }
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to load metadata from file " << d->url.fileName();
        }

        if (!ret)
        {
            status |= TimeAdjustList::META_TIME_ERROR;
        }
    }

    if (d->settings.updFileModDate)
    {
        // Since QFileInfo does not support timestamp updates, see Qt suggestion #79427 at
        // http://www.qtsoftware.com/developer/task-tracker/index_html?id=79427&method=entry
        // we have to use the utime() system call.

        int modtime;
        QDateTime unixDate;
        unixDate.setDate(QDate(1970, 1, 1));
        unixDate.setTime(QTime(0, 0, 0, 0));

        if (dt < unixDate)
            modtime = -(dt.secsTo(unixDate) + (60 * 60));
        else
            modtime = dt.toTime_t();

        utimbuf times;
        times.actime  = QDateTime::currentDateTime().toTime_t();
        times.modtime = modtime;

        if (utime(QFile::encodeName(d->url.toLocalFile()).constData(), &times) != 0)
        {
            status |= TimeAdjustList::FILE_TIME_ERROR;
        }
    }

    emit signalProcessEnded(d->url, status);
    emit signalDone();
}

} // namespace Digikam
