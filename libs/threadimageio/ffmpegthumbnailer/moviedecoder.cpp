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

#include "moviedecoder.h"

// FFMpeg includes

extern "C"
{
#include <libswscale/swscale.h>
}

// Qt includes

#include <QDebug>
#include <QFileInfo>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class MovieDecoder::Private
{
public:

    Private()
    {
        videoStream           = -1;
        pVideoCodecContext    = 0;
        pVideoCodec           = 0;
        pVideoStream          = 0;
        pFrame                = 0;
        pFrameBuffer          = 0;
        pPacket               = 0;
        formatContextWasGiven = false;
        allowSeek             = true;
        initialized           = false;
        bufferSinkContext     = 0;
        bufferSourceContext   = 0;
        filterGraph           = 0;
        filterFrame           = 0;
    }

    int                videoStream;
    AVFormatContext*   pFormatContext;
    AVCodecContext*    pVideoCodecContext;
    AVCodec*           pVideoCodec;
    AVStream*          pVideoStream;
    AVFrame*           pFrame;
    quint8*            pFrameBuffer;
    AVPacket*          pPacket;
    bool               formatContextWasGiven;
    bool               allowSeek;
    bool               initialized;
    AVFilterContext*   bufferSinkContext;
    AVFilterContext*   bufferSourceContext;
    AVFilterGraph*     filterGraph;
    AVFrame*           filterFrame;
    int                lastWidth;
    int                lastHeight;
    enum AVPixelFormat lastPixfmt;
};

MovieDecoder::MovieDecoder(const QString& filename,
                           AVFormatContext* const pavContext)
    : d(new Private)
{
    d->pFormatContext        = pavContext;
    d->formatContextWasGiven = pavContext ? true : false;

    initialize(filename);
}

MovieDecoder::~MovieDecoder()
{
    destroy();
    delete d;
}

void MovieDecoder::initialize(const QString& filename)
{
    d->lastWidth  = -1;
    d->lastHeight = -1;
    d->lastPixfmt = AV_PIX_FMT_NONE;
    av_register_all();
    avcodec_register_all();

    QFileInfo fileInfo(filename);

    if ((!d->formatContextWasGiven) &&
        avformat_open_input(&d->pFormatContext, fileInfo.absoluteFilePath().toLocal8Bit().data(), NULL, NULL) != 0)
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "Could not open input file: "
                                    << fileInfo.absoluteFilePath();
        return;
    }

    if (avformat_find_stream_info(d->pFormatContext, 0) < 0)
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "Could not find stream information";
        return;
    }

    initializeVideo();
    d->pFrame = av_frame_alloc();

    if (d->pFrame)
    {
        d->initialized=true;
    }
}

bool MovieDecoder::getInitialized() const
{
    return d->initialized;
}

void MovieDecoder::destroy()
{
    deleteFilterGraph();

    if (d->pVideoCodecContext)
    {
        avcodec_close(d->pVideoCodecContext);
        d->pVideoCodecContext = 0;
    }

    if ((!d->formatContextWasGiven) && d->pFormatContext)
    {
        avformat_close_input(&d->pFormatContext);
        d->pFormatContext = 0;
    }

    if (d->pPacket)
    {
        av_packet_unref(d->pPacket);
        delete d->pPacket;
        d->pPacket = 0;
    }

    if (d->pFrame)
    {
        av_frame_free(&d->pFrame);
        d->pFrame = 0;
    }

    if (d->pFrameBuffer)
    {
        av_free(d->pFrameBuffer);
        d->pFrameBuffer = 0;
    }
}

QString MovieDecoder::getCodec() const 
{
    QString codecName;

    if (d->pVideoCodec)
    {
        codecName = QString::fromLatin1(d->pVideoCodec->name);
    }

    return codecName;
}

void MovieDecoder::initializeVideo()
{
    for (unsigned int i = 0 ; i < d->pFormatContext->nb_streams ; i++)
    {
        if (d->pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            d->pVideoStream = d->pFormatContext->streams[i];
            d->videoStream  = i;
            break;
        }
    }

    if (d->videoStream == -1)
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "Could not find video stream";
        return;
    }

    d->pVideoCodecContext = d->pFormatContext->streams[d->videoStream]->codec;
    d->pVideoCodec        = avcodec_find_decoder(d->pVideoCodecContext->codec_id);

    if (d->pVideoCodec == 0)
    {
        // set to 0, otherwise avcodec_close(d->pVideoCodecContext) crashes
        d->pVideoCodecContext = 0;
        qDebug(DIGIKAM_GENERAL_LOG) << "Video Codec not found";
        return;
    }

    d->pVideoCodecContext->workaround_bugs = 1;

    if (avcodec_open2(d->pVideoCodecContext, d->pVideoCodec, 0) < 0)
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "Could not open video codec";
    }
}

int MovieDecoder::getWidth() const
{
    if (d->pVideoCodecContext)
    {
        return d->pVideoCodecContext->width;
    }

    return -1;
}

int MovieDecoder::getHeight() const
{
    if (d->pVideoCodecContext)
    {
        return d->pVideoCodecContext->height;
    }

    return -1;
}

int MovieDecoder::getDuration() const
{
    if (d->pFormatContext)
    {
        return static_cast<int>(d->pFormatContext->duration / AV_TIME_BASE);
    }

    return 0;
}

void MovieDecoder::seek(int timeInSeconds)
{
    if (!d->allowSeek)
    {
        return;
    }

    qint64 timestamp = AV_TIME_BASE * static_cast<qint64>(timeInSeconds);

    if (timestamp < 0)
    {
        timestamp = 0;
    }

    int ret = av_seek_frame(d->pFormatContext, -1, timestamp, 0);

    if (ret >= 0) 
    {
        avcodec_flush_buffers(d->pFormatContext->streams[d->videoStream]->codec);
    }
    else
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "Seeking in video failed";
        return;
    }

    int keyFrameAttempts = 0;
    bool gotFrame        = 0;

    do
    {
        int count = 0;
        gotFrame  = 0;

        while (!gotFrame && count < 20)
        {
            getVideoPacket();
            gotFrame = decodeVideoPacket();
            count++;
        }

        keyFrameAttempts++;
    }
    while ((!gotFrame || !d->pFrame->key_frame) &&
            keyFrameAttempts < 200);

    if (gotFrame == 0)
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "Seeking in video failed";
    }
}

void MovieDecoder::decodeVideoFrame()
{
    bool frameFinished = false;

    while (!frameFinished && getVideoPacket())
    {
        frameFinished = decodeVideoPacket();
    }

    if (!frameFinished)
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "decodeVideoFrame() failed: frame not finished";
        return;
    }
}

bool MovieDecoder::decodeVideoPacket()
{
    if (d->pPacket->stream_index != d->videoStream)
    {
        return false;
    }

    av_frame_unref(d->pFrame);

    int frameFinished = 0;

#if LIBAVCODEC_VERSION_MAJOR < 53
    int bytesDecoded = avcodec_decode_video(d->pVideoCodecContext,
                                            d->pFrame,
                                            &frameFinished,
                                            d->pPacket->data,
                                            d->pPacket->size);
#else
    int bytesDecoded = avcodec_decode_video2(d->pVideoCodecContext,
                                             d->pFrame,
                                             &frameFinished,
                                             d->pPacket);
#endif

    if (bytesDecoded < 0)
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "Failed to decode video frame: bytesDecoded < 0";
    }

    return (frameFinished > 0);
}

bool MovieDecoder::getVideoPacket()
{
    bool framesAvailable = true;
    bool frameDecoded    = false;

    int attempts = 0;

    if (d->pPacket)
    {
        av_packet_unref(d->pPacket);
        delete d->pPacket;
    }

    d->pPacket = new AVPacket();

    while (framesAvailable &&
           !frameDecoded &&
           (attempts++ < 1000))
    {
        framesAvailable = (av_read_frame(d->pFormatContext, d->pPacket) >= 0);

        if (framesAvailable)
        {
            frameDecoded = (d->pPacket->stream_index == d->videoStream);

            if (!frameDecoded)
            {
                av_packet_unref(d->pPacket);
            }
        }
    }

    return frameDecoded;
}

void MovieDecoder::deleteFilterGraph()
{
    if (d->filterGraph)
    {
        av_frame_free(&d->filterFrame);
        avfilter_graph_free(&d->filterGraph);
        d->filterGraph = 0;
    }
}

bool MovieDecoder::initFilterGraph(enum AVPixelFormat pixfmt, int width, int height)
{
    AVFilterInOut* inputs  = 0;
    AVFilterInOut* outputs = 0;

    deleteFilterGraph();
    d->filterGraph         = avfilter_graph_alloc();

    QByteArray arguments("buffer=");
    arguments += "video_size=" + QByteArray::number(width)  + "x" + QByteArray::number(height) + ":";
    arguments += "pix_fmt="    + QByteArray::number(pixfmt) + ":";
    arguments += "time_base=1/1:pixel_aspect=0/1[in];";
    arguments += "[in]yadif[out];";
    arguments += "[out]buffersink";

    int ret = avfilter_graph_parse2(d->filterGraph, arguments.constData(), &inputs, &outputs);

    if (ret < 0)
    {
        qWarning(DIGIKAM_GENERAL_LOG) << "Unable to parse filter graph";
        return false;
    }

    if (inputs || outputs)
    {
        return -1;
    }

    ret = avfilter_graph_config(d->filterGraph, nullptr);

    if (ret < 0)
    {
        qWarning(DIGIKAM_GENERAL_LOG) << "Unable to validate filter graph";
        return false;
    }

    d->bufferSourceContext = avfilter_graph_get_filter(d->filterGraph, "Parsed_buffer_0");
    d->bufferSinkContext   = avfilter_graph_get_filter(d->filterGraph, "Parsed_buffersink_2");

    if (!d->bufferSourceContext || !d->bufferSinkContext)
    {
        qWarning(DIGIKAM_GENERAL_LOG) << "Unable to get source or sink";
        return false;
    }

    d->filterFrame = av_frame_alloc();
    d->lastWidth   = width;
    d->lastHeight  = height;
    d->lastPixfmt  = pixfmt;

    return true;
}

bool MovieDecoder::processFilterGraph(AVPicture* const dst,
                                      const AVPicture* const src,
                                      enum AVPixelFormat pixfmt,
                                      int width, int height)
{
    if (!d->filterGraph         ||
        width  != d->lastWidth  ||
        height != d->lastHeight ||
        pixfmt != d->lastPixfmt)
    {

        if (!initFilterGraph(pixfmt, width, height))
        {
            return false;
        }
    }

    memcpy(d->filterFrame->data,     src->data,     sizeof(src->data));
    memcpy(d->filterFrame->linesize, src->linesize, sizeof(src->linesize));

    d->filterFrame->width  = width;
    d->filterFrame->height = height;
    d->filterFrame->format = pixfmt;
    int ret                = av_buffersrc_add_frame(d->bufferSourceContext, d->filterFrame);

    if (ret < 0)
    {
        return false;
    }

    ret = av_buffersink_get_frame(d->bufferSinkContext, d->filterFrame);

    if (ret < 0)
    {
        return false;
    }

    av_picture_copy(dst, (const AVPicture*)d->filterFrame, pixfmt, width, height);
    av_frame_unref(d->filterFrame);

    return true;
}

void MovieDecoder::getScaledVideoFrame(int scaledSize,
                                       bool maintainAspectRatio,
                                       VideoFrame& videoFrame)
{
    if (d->pFrame->interlaced_frame)
    {
        processFilterGraph((AVPicture*)d->pFrame,
                           (AVPicture*)d->pFrame,
                           d->pVideoCodecContext->pix_fmt,
                           d->pVideoCodecContext->width,
                           d->pVideoCodecContext->height);
    }

    int scaledWidth, scaledHeight;
    convertAndScaleFrame(AV_PIX_FMT_RGB24, scaledSize, maintainAspectRatio, scaledWidth, scaledHeight);

    videoFrame.width    = scaledWidth;
    videoFrame.height   = scaledHeight;
    videoFrame.lineSize = d->pFrame->linesize[0];

    videoFrame.frameData.clear();
    videoFrame.frameData.resize(videoFrame.lineSize * videoFrame.height);
    memcpy((&(videoFrame.frameData.front())),
           d->pFrame->data[0],
           videoFrame.lineSize * videoFrame.height);
}

void MovieDecoder::convertAndScaleFrame(AVPixelFormat format,
                                        int scaledSize,
                                        bool maintainAspectRatio,
                                        int& scaledWidth,
                                        int& scaledHeight)
{
    calculateDimensions(scaledSize, maintainAspectRatio, scaledWidth, scaledHeight);
    SwsContext* const scaleContext = sws_getContext(d->pVideoCodecContext->width,
                                                    d->pVideoCodecContext->height,
                                                    d->pVideoCodecContext->pix_fmt,
                                                    scaledWidth,
                                                    scaledHeight,
                                                    format,
                                                    SWS_BICUBIC,
                                                    NULL,
                                                    NULL,
                                                    NULL);

    if (!scaleContext)
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "Failed to create resize context";
        return;
    }

    AVFrame* convertedFrame       = 0;
    uint8_t* convertedFrameBuffer = 0;

    createAVFrame(&convertedFrame,
                  &convertedFrameBuffer,
                  scaledWidth,
                  scaledHeight,
                  format);

    sws_scale(scaleContext,
              d->pFrame->data,
              d->pFrame->linesize,
              0,
              d->pVideoCodecContext->height,
              convertedFrame->data,
              convertedFrame->linesize);

    sws_freeContext(scaleContext);

    av_frame_free(&d->pFrame);
    av_free(d->pFrameBuffer);

    d->pFrame       = convertedFrame;
    d->pFrameBuffer = convertedFrameBuffer;
}

void MovieDecoder::calculateDimensions(int squareSize,
                                       bool maintainAspectRatio,
                                       int& destWidth,
                                       int& destHeight)
{
    if (!maintainAspectRatio)
    {
        destWidth  = squareSize;
        destHeight = squareSize;
    }
    else
    {
        int srcWidth            = d->pVideoCodecContext->width;
        int srcHeight           = d->pVideoCodecContext->height;
        int ascpectNominator    = d->pVideoCodecContext->sample_aspect_ratio.num;
        int ascpectDenominator  = d->pVideoCodecContext->sample_aspect_ratio.den;

        if (ascpectNominator != 0 && ascpectDenominator != 0)
        {
            srcWidth = srcWidth * ascpectNominator / ascpectDenominator;
        }

        if (srcWidth > srcHeight)
        {
            destWidth  = squareSize;
            destHeight = static_cast<int>(static_cast<float>(squareSize) / srcWidth * srcHeight);
        }
        else
        {
            destWidth  = static_cast<int>(static_cast<float>(squareSize) / srcHeight * srcWidth);
            destHeight = squareSize;
        }
    }
}

void MovieDecoder::createAVFrame(AVFrame** const avFrame,
                                 quint8** const frameBuffer,
                                 int width,
                                 int height,
                                 AVPixelFormat format)
{
    *avFrame     = av_frame_alloc();
    int numBytes = avpicture_get_size(format, width, height);
    *frameBuffer = reinterpret_cast<quint8*>(av_malloc(numBytes));

    avpicture_fill((AVPicture*) *avFrame, *frameBuffer, format, width, height);
}

} // namespace Digikam
