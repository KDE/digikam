/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : a class to manage video thumbnails extraction
 *
 * Copyright (C) 2016-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2016-2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#ifndef VIDEO_THUMBNAILER_JOB_H
#define VIDEO_THUMBNAILER_JOB_H

// Qt includes

#include <QStringList>
#include <QImage>
#include <QList>
#include <QThread>
#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT VideoThumbnailerJob : public QThread
{
    Q_OBJECT

public:

    /** Standard constructor and destructor
     */
    explicit VideoThumbnailerJob(QObject* const parent);
    virtual ~VideoThumbnailerJob();

    /** Set size of thumbnails to generate
     */
    void setThumbnailSize(int size);

    /** Add a film strip on the left side of video thumnails
     */
    void setCreateStrip(bool strip);

    /** Set exif rotation of thumbnails to generate
     */
    void  setExifRotate(bool rotate);

    /** Add new video files to process on the pending list
     */
    void addItems(const QStringList&);

Q_SIGNALS:

    /** Emit when thumnail is generated and ready to use.
     */
    void signalThumbnailDone(const QString&, const QImage&);

    /** Emit when thumbnail cannot be generated for a video file
     */
    void signalThumbnailFailed(const QString&);

    /* Emit when the pending list is empty
     */
    void signalThumbnailJobFinished();

public Q_SLOTS:

    void slotCancel();

private:

    void run();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // VIDEO_THUMBNAILER_JOB_H
