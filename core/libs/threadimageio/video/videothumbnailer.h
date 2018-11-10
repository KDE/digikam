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

#ifndef DIGIKAM_VIDEO_THUMBNAILER_H
#define DIGIKAM_VIDEO_THUMBNAILER_H

// Qt includes

#include <QString>
#include <QImage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class VideoFrame;
class VideoThumbWriter;
class VideoDecoder;
class VideoStripFilter;

class DIGIKAM_EXPORT VideoThumbnailer
{
public:

    VideoThumbnailer();
    VideoThumbnailer(int thumbnailSize,
                     bool workaroundIssues,
                     bool maintainAspectRatio,
                     bool smartFrameSelection);
    ~VideoThumbnailer();

public:

    void generateThumbnail(const QString& videoFile, QImage& image);

    void setThumbnailSize(int size);
    void setSeekPercentage(int percentage);
    void setSeekTime(const QString& seekTime);
    void setWorkAroundIssues(bool workAround);
    void setMaintainAspectRatio(bool enabled);
    void setSmartFrameSelection(bool enabled);
    void addFilter(VideoStripFilter* const filter);
    void removeFilter(VideoStripFilter* const filter);
    void clearFilters();

private:

    void generateThumbnail(const QString& videoFile, VideoThumbWriter& imageWriter, QImage& image);
    void generateSmartThumbnail(VideoDecoder& movieDecoder, VideoFrame& videoFrame);

    void applyFilters(VideoFrame& frameData);
    int  timeToSeconds(const QString& time) const;

private:

    VideoThumbnailer(const VideoThumbnailer&); // Disable

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_VIDEO_THUMBNAILER_H
