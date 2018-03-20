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

#include "videothumbnailer.h"

// C++ includes

#include <cfloat>

// Qt includes

#include <QtGlobal>
#include <QtMath>
#include <QTime>

// Local includes

#include "moviedecoder.h"
#include "filmstripfilter.h"
#include "imagewriter.h"
#include "digikam_debug.h"

using namespace std;

namespace Digikam
{

class VideoThumbnailer::Private
{
public:

    Private()
      :  SMART_FRAME_ATTEMPTS(25)
    {
        thumbnailSize       = 256;
        seekPercentage      = 10;
        overlayFilmStrip    = false;
        workAroundIssues    = false;
        maintainAspectRatio = true;
        smartFrameSelection = false;
    }

    int                       thumbnailSize;
    quint16                   seekPercentage;
    bool                      overlayFilmStrip;
    bool                      workAroundIssues;
    bool                      maintainAspectRatio;
    bool                      smartFrameSelection;
    QString                   seekTime;
    QVector<FilmStripFilter*> filters;

    const int                 SMART_FRAME_ATTEMPTS;
};

VideoThumbnailer::VideoThumbnailer()
    : d(new Private)
{
}

VideoThumbnailer::VideoThumbnailer(int thumbnailSize,
                                   bool workaroundIssues,
                                   bool maintainAspectRatio,
                                   bool smartFrameSelection)
    : d(new Private)
{
    d->thumbnailSize       = thumbnailSize;
    d->workAroundIssues    = workaroundIssues;
    d->maintainAspectRatio = maintainAspectRatio;
    d->smartFrameSelection = smartFrameSelection;
}

VideoThumbnailer::~VideoThumbnailer()
{
    delete d;
}

void VideoThumbnailer::setSeekPercentage(int percentage)
{
    d->seekTime.clear();
    d->seekPercentage = percentage > 95 ? 95 : percentage;
}

void VideoThumbnailer::setSeekTime(const QString& seekTime)
{
    d->seekTime = seekTime;
}

void VideoThumbnailer::setThumbnailSize(int size)
{
    d->thumbnailSize = size;
}

void VideoThumbnailer::setWorkAroundIssues(bool workAround)
{
    d->workAroundIssues = workAround;
}

void VideoThumbnailer::setMaintainAspectRatio(bool enabled)
{
    d->maintainAspectRatio = enabled;
}

void VideoThumbnailer::setSmartFrameSelection(bool enabled)
{
    d->smartFrameSelection = enabled;
}

int VideoThumbnailer::timeToSeconds(const QString& time) const
{
    return QTime::fromString(time, QLatin1String("hh:mm:ss")).secsTo(QTime(0, 0, 0));
}

void VideoThumbnailer::generateThumbnail(const QString& videoFile,
                                         ImageWriter& imageWriter,
                                         QImage &image)
{
    MovieDecoder movieDecoder(videoFile);

    if (movieDecoder.getInitialized())
    {
        movieDecoder.decodeVideoFrame(); // before seeking, a frame has to be decoded

        if ((!d->workAroundIssues) || (movieDecoder.getCodec() != QLatin1String("h264")))
        {
            // workaround for bug in older ffmpeg (100% cpu usage when seeking in h264 files)
            int secondToSeekTo = d->seekTime.isEmpty() ? movieDecoder.getDuration() * d->seekPercentage / 100
                                                       : timeToSeconds(d->seekTime);
            movieDecoder.seek(secondToSeekTo);
        }

        VideoFrame videoFrame;

        if (d->smartFrameSelection)
        {
            generateSmartThumbnail(movieDecoder, videoFrame);
        }
        else
        {
            movieDecoder.getScaledVideoFrame(d->thumbnailSize, d->maintainAspectRatio, videoFrame);
        }

        applyFilters(videoFrame);
        imageWriter.writeFrame(videoFrame, image);
    }
}

void VideoThumbnailer::generateSmartThumbnail(MovieDecoder& movieDecoder,
                                              VideoFrame& videoFrame)
{
    vector<VideoFrame> videoFrames(d->SMART_FRAME_ATTEMPTS);
    vector<Histogram<int> > histograms(d->SMART_FRAME_ATTEMPTS);

    for (int i = 0 ; i < d->SMART_FRAME_ATTEMPTS ; i++)
    {
        movieDecoder.decodeVideoFrame();
        movieDecoder.getScaledVideoFrame(d->thumbnailSize, d->maintainAspectRatio, videoFrames[i]);
        generateHistogram(videoFrames[i], histograms[i]);
    }

    int bestFrame = getBestThumbnailIndex(videoFrames, histograms);

    Q_ASSERT(bestFrame != -1);

    videoFrame = videoFrames[bestFrame];
}

void VideoThumbnailer::generateThumbnail(const QString& videoFile,
                                         QImage &image)
{
    ImageWriter* const imageWriter = new  ImageWriter();
    generateThumbnail(videoFile, *imageWriter, image);
    delete imageWriter;
}

void VideoThumbnailer::addFilter(FilmStripFilter* const filter)
{
    d->filters.append(filter);
}

void VideoThumbnailer::removeFilter(FilmStripFilter* const filter)
{
    for (QVector<FilmStripFilter*>::iterator it = d->filters.begin();
         it != d->filters.end();
         ++it)
    {
        if (*it == filter)
        {
            d->filters.erase(it);
            break;
        }
    }
}

void VideoThumbnailer::clearFilters()
{
    d->filters.clear();
}

void VideoThumbnailer::applyFilters(VideoFrame& videoFrame)
{
    for (QVector<FilmStripFilter*>::iterator it = d->filters.begin();
         it != d->filters.end();
         ++it)
    {
        (*it)->process(videoFrame);
    }
}

void VideoThumbnailer::generateHistogram(const VideoFrame& videoFrame,
                                         Histogram<int>& histogram)
{
    for (quint32 i = 0 ; i < videoFrame.height ; i++)
    {
        int pixelIndex = i * videoFrame.lineSize;

        for (quint32 j = 0 ; j < videoFrame.width * 3 ; j += 3)
        {
            ++histogram.r[videoFrame.frameData[pixelIndex + j]];
            ++histogram.g[videoFrame.frameData[pixelIndex + j + 1]];
            ++histogram.b[videoFrame.frameData[pixelIndex + j + 2]];
        }
    }
}

int VideoThumbnailer::getBestThumbnailIndex(vector<VideoFrame>& videoFrames,
                                            const vector<Histogram<int> >& histograms)
{
    Q_UNUSED(videoFrames);
    Histogram<float> avgHistogram;

    for (size_t i = 0 ; i < histograms.size() ; i++)
    {
        for (int j = 0 ; j < 255 ; j++)
        {
            avgHistogram.r[j] += static_cast<float>(histograms[i].r[j]) / histograms.size();
            avgHistogram.g[j] += static_cast<float>(histograms[i].g[j]) / histograms.size();
            avgHistogram.b[j] += static_cast<float>(histograms[i].b[j]) / histograms.size();
        }
    }

    int bestFrame = -1;
    float minRMSE = FLT_MAX;

    for (size_t i = 0 ; i < histograms.size() ; i++)
    {
        // calculate root mean squared error
        float rmse = 0.0;

        for (int j = 0 ; j < 255 ; j++)
        {
            float error = qFabs(avgHistogram.r[j] - histograms[i].r[j]) +
                          qFabs(avgHistogram.g[j] - histograms[i].g[j]) +
                          qFabs(avgHistogram.b[j] - histograms[i].b[j]);
            rmse += (error * error) / 255;
        }

        rmse = qSqrt(rmse);

        if (rmse < minRMSE)
        {
            minRMSE   = rmse;
            bestFrame = i;
        }
    }
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "Best frame was: "
                                 << bestFrame
                                 << "(RMSE: "
                                 << minRMSE
                                 << ")" << endl;
*/
    return bestFrame;
}

} // namespace Digikam
