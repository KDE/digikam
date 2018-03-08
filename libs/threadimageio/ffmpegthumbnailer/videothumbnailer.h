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

#ifndef VIDEO_THUMBNAILER_H
#define VIDEO_THUMBNAILER_H

// Qt includes

#include <QString>
#include <QImage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class VideoFrame;
class ImageWriter;
class MovieDecoder;
class FilmStripFilter;

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
    void addFilter(FilmStripFilter* const filter);
    void removeFilter(FilmStripFilter* const filter);
    void clearFilters();

private:

    template <typename T>
    class Histogram
    {

    public:

        explicit Histogram()
        {
            memset(r, 0, 255 * sizeof(T));
            memset(g, 0, 255 * sizeof(T));
            memset(b, 0, 255 * sizeof(T));
        }

        ~Histogram()
        {
        }

    public:

        T r[256];
        T g[256];
        T b[256];
    };

private:

    void generateThumbnail(const QString& videoFile, ImageWriter& imageWriter, QImage& image);
    void generateSmartThumbnail(MovieDecoder& movieDecoder, VideoFrame& videoFrame);

    void generateHistogram(const VideoFrame& videoFrame, Histogram<int>& histogram);
    int  getBestThumbnailIndex(std::vector<VideoFrame>& videoFrames,
                               const std::vector<Histogram<int> >& histograms);
    void applyFilters(VideoFrame& frameData);
    int  timeToSeconds(const QString& time) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // VIDEO_THUMBNAILER_H
