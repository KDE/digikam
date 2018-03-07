//    Copyright (C) 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "videothumbnailer.h"

#include "moviedecoder.h"
#include "filmstripfilter.h"
#include "imagewriter.h"

#include <iostream>
#include <cfloat>
#include <cmath>
#include <qglobal.h>
#include <sys/stat.h>
#include <QTime>


using namespace std;

namespace Digikam
{

static const int SMART_FRAME_ATTEMPTS = 25;

VideoThumbnailer::VideoThumbnailer()
        : m_ThumbnailSize(128)
        , m_SeekPercentage(10)
        , m_OverlayFilmStrip(false)
        , m_WorkAroundIssues(false)
        , m_MaintainAspectRatio(true)
        , m_SmartFrameSelection(false)
{
}

VideoThumbnailer::VideoThumbnailer(int thumbnailSize, bool workaroundIssues, bool maintainAspectRatio, bool smartFrameSelection)
        : m_ThumbnailSize(thumbnailSize)
        , m_SeekPercentage(10)
        , m_WorkAroundIssues(workaroundIssues)
        , m_MaintainAspectRatio(maintainAspectRatio)
        , m_SmartFrameSelection(smartFrameSelection)
{
}

VideoThumbnailer::~VideoThumbnailer()
{
}

void VideoThumbnailer::setSeekPercentage(int percentage)
{
    m_SeekTime.clear();
    m_SeekPercentage = percentage > 95 ? 95 : percentage;
}

void VideoThumbnailer::setSeekTime(const QString& seekTime)
{
    m_SeekTime = seekTime;
}

void VideoThumbnailer::setThumbnailSize(int size)
{
    m_ThumbnailSize = size;
}

void VideoThumbnailer::setWorkAroundIssues(bool workAround)
{
    m_WorkAroundIssues = workAround;
}

void VideoThumbnailer::setMaintainAspectRatio(bool enabled)
{
    m_MaintainAspectRatio = enabled;
}

void VideoThumbnailer::setSmartFrameSelection(bool enabled)
{
    m_SmartFrameSelection = enabled;
}

int timeToSeconds(const QString& time)
{
    return QTime::fromString(time, QLatin1String("hh:mm:ss")).secsTo(QTime(0, 0, 0));
}

void VideoThumbnailer::generateThumbnail(const QString& videoFile, ImageWriter& imageWriter, QImage &image)
{
    MovieDecoder movieDecoder(videoFile, NULL);
    if (movieDecoder.getInitialized()) {
        movieDecoder.decodeVideoFrame(); //before seeking, a frame has to be decoded
        
        if ((!m_WorkAroundIssues) || (movieDecoder.getCodec() != QLatin1String("h264"))) { //workaround for bug in older ffmpeg (100% cpu usage when seeking in h264 files)
            int secondToSeekTo = m_SeekTime.isEmpty() ? movieDecoder.getDuration() * m_SeekPercentage / 100 : timeToSeconds(m_SeekTime);
            movieDecoder.seek(secondToSeekTo);
        }
    
        VideoFrame videoFrame;
        
        if (m_SmartFrameSelection) {
            generateSmartThumbnail(movieDecoder, videoFrame);
        } else {
            movieDecoder.getScaledVideoFrame(m_ThumbnailSize, m_MaintainAspectRatio, videoFrame);
        }
        
        applyFilters(videoFrame);
        imageWriter.writeFrame(videoFrame, image);
    }
}

void VideoThumbnailer::generateSmartThumbnail(MovieDecoder& movieDecoder, VideoFrame& videoFrame)
{
    vector<VideoFrame> videoFrames(SMART_FRAME_ATTEMPTS);
    vector<Histogram<int> > histograms(SMART_FRAME_ATTEMPTS);

    for (int i = 0; i < SMART_FRAME_ATTEMPTS; ++i) {
        movieDecoder.decodeVideoFrame();
        movieDecoder.getScaledVideoFrame(m_ThumbnailSize, m_MaintainAspectRatio, videoFrames[i]);
        generateHistogram(videoFrames[i], histograms[i]);
    }

    int bestFrame = getBestThumbnailIndex(videoFrames, histograms);

    Q_ASSERT(bestFrame != -1);
    videoFrame = videoFrames[bestFrame];
}

void VideoThumbnailer::generateThumbnail(const QString& videoFile, QImage &image)
{
    ImageWriter* imageWriter = new  ImageWriter();
    generateThumbnail(videoFile, *imageWriter, image);
    delete imageWriter;
}

void VideoThumbnailer::addFilter(IFilter* filter)
{
    m_Filters.push_back(filter);
}

void VideoThumbnailer::removeFilter(IFilter* filter)
{
    for (vector<IFilter*>::iterator iter = m_Filters.begin();
            iter != m_Filters.end();
            ++iter) {
        if (*iter == filter) {
            m_Filters.erase(iter);
            break;
        }
    }
}

void VideoThumbnailer::clearFilters()
{
    m_Filters.clear();
}

void VideoThumbnailer::applyFilters(VideoFrame& videoFrame)
{
    for (vector<IFilter*>::iterator iter = m_Filters.begin();
            iter != m_Filters.end();
            ++iter) {
        (*iter)->process(videoFrame);
    }
}

void VideoThumbnailer::generateHistogram(const VideoFrame& videoFrame, Histogram<int>& histogram)
{
    for (quint32 i = 0; i < videoFrame.height; ++i) {
        int pixelIndex = i * videoFrame.lineSize;
        for (quint32 j = 0; j < videoFrame.width * 3; j += 3) {
            ++histogram.r[videoFrame.frameData[pixelIndex + j]];
            ++histogram.g[videoFrame.frameData[pixelIndex + j + 1]];
            ++histogram.b[videoFrame.frameData[pixelIndex + j + 2]];
        }
    }
}

int VideoThumbnailer::getBestThumbnailIndex(vector<VideoFrame>& videoFrames, const vector<Histogram<int> >& histograms)
{
    Q_UNUSED(videoFrames);
    Histogram<float> avgHistogram;
    for (size_t i = 0; i < histograms.size(); ++i) {
        for (int j = 0; j < 255; ++j) {
            avgHistogram.r[j] += static_cast<float>(histograms[i].r[j]) / histograms.size();
            avgHistogram.g[j] += static_cast<float>(histograms[i].g[j]) / histograms.size();
            avgHistogram.b[j] += static_cast<float>(histograms[i].b[j]) / histograms.size();
        }
    }

    int bestFrame = -1;
    float minRMSE = FLT_MAX;
    for (size_t i = 0; i < histograms.size(); ++i) {
        //calculate root mean squared error
        float rmse = 0.0;
        for (int j = 0; j < 255; ++j) {
            float error = fabsf(avgHistogram.r[j] - histograms[i].r[j])
                          + fabsf(avgHistogram.g[j] - histograms[i].g[j])
                          + fabsf(avgHistogram.b[j] - histograms[i].b[j]);
            rmse += (error * error) / 255;
        }

        rmse = sqrtf(rmse);
        if (rmse < minRMSE) {
            minRMSE = rmse;
            bestFrame = i;
        }
    }
#ifdef DEBUG_MODE
    cout << "Best frame was: " << bestFrame << "(RMSE: " << minRMSE << ")" << endl;
#endif
    return bestFrame;
}

}
