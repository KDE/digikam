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

#ifndef MOVIE_DECODER_P_H
#define MOVIE_DECODER_P_H

#include "moviedecoder.h"

// FFMpeg includes

extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}

namespace Digikam
{

class MovieDecoder::Private
{
public:

    explicit Private();
    ~Private();

public:

    int                videoStream;
    AVFormatContext*   pFormatContext;
    AVCodecContext*    pVideoCodecContext;
    AVCodecParameters* pVideoCodecParameters;
    AVCodec*           pVideoCodec;
    AVStream*          pVideoStream;
    AVFrame*           pFrame;
    quint8*            pFrameBuffer;
    AVPacket*          pPacket;
    bool               allowSeek;
    bool               initialized;
    AVFilterContext*   bufferSinkContext;
    AVFilterContext*   bufferSourceContext;
    AVFilterGraph*     filterGraph;
    AVFrame*           filterFrame;
    int                lastWidth;
    int                lastHeight;
    enum AVPixelFormat lastPixfmt;

public:

    void initializeVideo();
    bool getVideoPacket();
    bool decodeVideoPacket() const;

    void convertAndScaleFrame(AVPixelFormat format,
                              int scaledSize,
                              bool maintainAspectRatio,
                              int& scaledWidth,
                              int& scaledHeight);

    bool processFilterGraph(AVFrame* const dst,
                            const AVFrame* const src,
                            enum AVPixelFormat pixfmt,
                            int width,
                            int height);

    void deleteFilterGraph();

private:

    bool initFilterGraph(enum AVPixelFormat pixfmt, int width, int height);

    void calculateDimensions(int squareSize,
                             bool maintainAspectRatio,
                             int& destWidth,
                             int& destHeight);

    void createAVFrame(AVFrame** const avFrame,
                       quint8** const frameBuffer,
                       int width,
                       int height,
                       AVPixelFormat format);

    int  decodeVideoNew(AVCodecContext* const avContext,
                        AVFrame* const avFrame,
                        int* gotFrame,
                        AVPacket* const avPacket) const;
};

} // namespace Digikam

#endif // MOVIE_DECODER_P_H
