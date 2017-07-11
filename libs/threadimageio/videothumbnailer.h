/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : QtAV based video thumbnailer
 *
 * Copyright (C) 2016-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// QtAV includes

#include <QtAV/VideoFrame.h>

// Local includes

#include "digikam_config.h"
#include "digikam_export.h"

using namespace QtAV;

namespace Digikam
{

class DIGIKAM_EXPORT VideoThumbnailer : public QObject
{
    Q_OBJECT

public:

    explicit VideoThumbnailer(QObject* const parent=0);
    virtual ~VideoThumbnailer();

public Q_SLOTS:

    void slotGetThumbnail(const QString&, int size, bool strip);

Q_SIGNALS:

    void signalThumbnailDone(const QString&, const QImage&);
    void signalThumbnailFailed(const QString&);

private:

    void tryExtractVideoFrame();

private Q_SLOTS:

    void slotFrameError();
    void slotFrameExtracted(const QtAV::VideoFrame& frame);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* VIDEOTHUMBNAILER_H */
