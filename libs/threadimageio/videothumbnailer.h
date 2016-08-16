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

#include <QPointer>
#include <QObject>
#include <QString>
#include <QImage>

// Local includes

#include "digikam_config.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT VideoThumbnailer : public QObject
{
    Q_OBJECT

public:

    explicit VideoThumbnailer(QObject* const parent=0);
    virtual ~VideoThumbnailer();

    static QPointer<VideoThumbnailer> internalPtr;
    static VideoThumbnailer*          instance();
    static bool                       isCreated();

    bool isReady() const;
    bool getThumbnail(quintptr job, const QString&, int size, bool strip);

Q_SIGNALS:

    void signalThumbnailDone(quintptr, const QString&, const QImage&);
    void signalThumbnailFailed(quintptr, const QString&);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* VIDEOTHUMBNAILER_H */
