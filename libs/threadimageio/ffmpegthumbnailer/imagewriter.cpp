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

#include "imagewriter.h"

namespace Digikam
{

VideoFrame::VideoFrame()
    : width(0),
      height(0),
      lineSize(0)
{
}

VideoFrame::VideoFrame(int width, int height, int lineSize)
    : width(width),
      height(height),
      lineSize(lineSize)
{
}

VideoFrame::~VideoFrame()
{
}

// ------------------------------------------------------

ImageWriter::ImageWriter()
{
}

ImageWriter::~ImageWriter()
{
}

void ImageWriter::writeFrame(VideoFrame& frame, QImage& image)
{
    QImage previewImage(frame.width, frame.height, QImage::Format_RGB888);

    for (quint32 y = 0; y < frame.height; y++)
    {
        // Copy each line ..
        memcpy(previewImage.scanLine(y),
               &frame.frameData[y * frame.lineSize],
               frame.width * 3);
    }

    image = previewImage;
}

} // namespace Digikam
