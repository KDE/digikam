/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : a class to manage video thumbnails extraction
 *
 * Copyright (C) 2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef VIDEOTHUMBNAILERJOB_H
#define VIDEOTHUMBNAILERJOB_H

// Qt includes

#include <QStringList>
#include <QImage>
#include <QList>
#include <QThread>
#include <QObject>

namespace Digikam
{

class VideoThumbnailerJob : public QThread
{
    Q_OBJECT

public:

    explicit VideoThumbnailerJob(QObject* const parent);
    virtual ~VideoThumbnailerJob();

    void setCreateStrip(bool);
    void setThumbnailSize(int);

    void addItems(const QStringList&);

Q_SIGNALS:

    void signalGetThumbnail(const QString&);

    void signalThumbnailDone(const QString&, const QImage&);
    void signalThumbnailFailed(const QString&);

    void signalComplete();

public Q_SLOTS:

    void slotCancel();

private Q_SLOTS:

    void slotThumbnailDone(const QString&, const QImage&);
    void slotThumbnailFailed(const QString&);

private:

    void run();
    void processOne();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* VIDEOTHUMBNAILERJOB_H */
