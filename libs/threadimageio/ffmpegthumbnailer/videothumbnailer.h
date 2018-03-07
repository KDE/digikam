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

#ifndef VIDEO_THUMBNAILER_H
#define VIDEO_THUMBNAILER_H

#include <string>
#include <vector>
#include <map>
#include <inttypes.h>

#include "digikam_export.h"
#include "ifilter.h"
#include "histogram.h"
#include <QString>
#include <QImage>


namespace Digikam
{

class VideoFrame;
class ImageWriter;
class MovieDecoder;

class DIGIKAM_EXPORT VideoThumbnailer
{
public:
    VideoThumbnailer();
    VideoThumbnailer(int thumbnailSize, bool workaroundIssues, bool maintainAspectRatio, bool smartFrameSelection);
    ~VideoThumbnailer();

    void generateThumbnail(const QString& videoFile, QImage &image);

    void setThumbnailSize(int size);
    void setSeekPercentage(int percentage);
    void setSeekTime(const QString& seekTime);
    void setWorkAroundIssues(bool workAround);
    void setMaintainAspectRatio(bool enabled);
    void setSmartFrameSelection(bool enabled);
    void addFilter(IFilter* filter);
    void removeFilter(IFilter* filter);
    void clearFilters();

private:
    void generateThumbnail(const QString& videoFile, ImageWriter& imageWriter, QImage& image);
    void generateSmartThumbnail(MovieDecoder& movieDecoder, VideoFrame& videoFrame);

    QString getMimeType(const QString& videoFile);
    QString getExtension(const QString& videoFilename);

    void generateHistogram(const VideoFrame& videoFrame, Histogram<int>& histogram);
    int getBestThumbnailIndex(std::vector<VideoFrame>& videoFrames, const std::vector<Histogram<int> >& histograms);
    void applyFilters(VideoFrame& frameData);

private:
    int                         m_ThumbnailSize;
    quint16                     m_SeekPercentage;
    bool                        m_OverlayFilmStrip;
    bool                        m_WorkAroundIssues;
    bool                        m_MaintainAspectRatio;
    bool                        m_SmartFrameSelection;
    QString                     m_SeekTime;
    std::vector<IFilter*>       m_Filters;
};

}

#endif
