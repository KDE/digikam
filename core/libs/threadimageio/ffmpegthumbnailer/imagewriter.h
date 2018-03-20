/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : video thumbnails extraction based on ffmpeg
 *
 * Copyright (C) 2010      by Dirk Vanden Boer <dirk dot vdb at gmail dot com>
 * Copyright (C) 2016-2018 by Maik Qualmann <metzpinguin at gmail dot com>
 * Copyright (C) 2016-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGE_WRITER_H
#define IMAGE_WRITER_H

// Qt includes

#include <QtGlobal>
#include <QImage>
#include <QVector>

namespace Digikam
{

class VideoFrame
{
public:

    VideoFrame();
    VideoFrame(int width, int height, int lineSize);
    ~VideoFrame();

public:

    quint32         width;
    quint32         height;
    quint32         lineSize;
    QVector<quint8> frameData;
};

// -----------------------------------------------------------------

class ImageWriter
{
public:

    explicit ImageWriter();
    ~ImageWriter();

    void writeFrame(VideoFrame& frame, QImage& image);
};

} // namespace Digikam

#endif // IMAGE_WRITER_H
