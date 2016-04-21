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

class VideoThumbnailer : public QObject
{
    Q_OBJECT

public:

    VideoThumbnailer(QObject* const parent=0);
    ~VideoThumbnailer();

    bool getThumbnail(const QString& file);

Q_SIGNALS:

    void signalVideoThumbDone();

private:

    class Private;
    Private* const d;
};

#endif /* VIDEOTHUMBNAILER_H */
