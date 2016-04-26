/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : Qt Multimedia based video thumbnailer 
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

#ifndef VIDEOTHUMBNAILER_H
#define VIDEOTHUMBNAILER_H

// Qt includes

#include <QObject>
#include <QString>
#include <QImage>

// Local includes 

#include "digikam_config.h"

namespace Digikam
{

#ifdef HAVE_MEDIAPLAYER

class VideoThumbnailer : public QObject
{
    Q_OBJECT

public:

    explicit VideoThumbnailer(QObject* const parent=0);
    virtual ~VideoThumbnailer();

    void setCreateStrip(bool strip);
    void setThumbnailSize(int size);

Q_SIGNALS:

    void signalThumbnailDone(const QString&, const QImage&);
    void signalThumbnailFailed(const QString&);

public Q_SLOTS:

    void slotGetThumbnail(const QString&);

private:

    class Private;
    Private* const d;
};

#else // HAVE_MEDIAPLAYER

class VideoThumbnailer : public QObject
{
    Q_OBJECT

public:

    explicit VideoThumbnailer(QObject* const) {};
    virtual ~VideoThumbnailer()               {};

    void setCreateStrip(bool)  {};
    void setThumbnailSize(int) {};

Q_SIGNALS:

    void signalThumbnailDone(const QString&, const QImage&);
    void signalThumbnailFailed(const QString&);

public Q_SLOTS:

    void slotGetThumbnail(const QString& f)
    {
        emit signalThumbnailFailed(f);
    };
};

#endif // HAVE_MEDIAPLAYER

}  // namespace Digikam

#endif /* VIDEOTHUMBNAILER_H */
