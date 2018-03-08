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

#ifndef MOVIE_DECODER_H
#define MOVIE_DECODER_H

// FFMpeg includes

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}

// Qt includes

#include <QString>

// Local includes

#include "imagewriter.h"

namespace Digikam
{

class MovieDecoder
{
public:

    explicit MovieDecoder(const QString& filename,
                          AVFormatContext* const pavContext = NULL);
    ~MovieDecoder();

public:

    QString getCodec()       const;
    int     getWidth()       const;
    int     getHeight()      const;
    int     getDuration()    const;
    bool    getInitialized() const;

    void seek(int timeInSeconds);
    void decodeVideoFrame();
    void getScaledVideoFrame(int scaledSize,
                             bool maintainAspectRatio,
                             VideoFrame& videoFrame);

    void initialize(const QString& filename);
    void destroy();

private:

    void initializeVideo();

    bool decodeVideoPacket();
    bool getVideoPacket();
    void convertAndScaleFrame(AVPixelFormat format,
                              int scaledSize,
                              bool maintainAspectRatio,
                              int& scaledWidth,
                              int& scaledHeight);
    void createAVFrame(AVFrame** avFrame,
                       quint8** frameBuffer,
                       int width,
                       int height,
                       AVPixelFormat format);
    void calculateDimensions(int squareSize,
                             bool maintainAspectRatio,
                             int& destWidth,
                             int& destHeight);

    void deleteFilterGraph();
    bool initFilterGraph(enum AVPixelFormat pixfmt, int width, int height);
    bool processFilterGraph(AVPicture* dst,
                            const AVPicture* src,
                            enum AVPixelFormat pixfmt,
                            int width,
                            int height);

private:

    int                m_VideoStream;
    AVFormatContext*   m_pFormatContext;
    AVCodecContext*    m_pVideoCodecContext;
    AVCodec*           m_pVideoCodec;
    AVStream*          m_pVideoStream;
    AVFrame*           m_pFrame;
    quint8*            m_pFrameBuffer;
    AVPacket*          m_pPacket;
    bool               m_FormatContextWasGiven;
    bool               m_AllowSeek;
    bool               m_initialized;
    AVFilterContext*   m_bufferSinkContext;
    AVFilterContext*   m_bufferSourceContext;
    AVFilterGraph*     m_filterGraph;
    AVFrame*           m_filterFrame;
    int                m_lastWidth;
    int                m_lastHeight;
    enum AVPixelFormat m_lastPixfmt;
};

} // namespace Digikam

#endif // MOVIE_DECODER_H
