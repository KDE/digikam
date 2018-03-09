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
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
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

    explicit Private()
    {
        videoStream           = -1;
        pFormatContext        = 0;
        pVideoCodecContext    = 0;
        pVideoCodecParameters = 0;
        pVideoCodec           = 0;
        pVideoStream          = 0;
        pFrame                = 0;
        pFrameBuffer          = 0;
        pPacket               = 0;
        allowSeek             = true;
        initialized           = false;
        bufferSinkContext     = 0;
        bufferSourceContext   = 0;
        filterGraph           = 0;
        filterFrame           = 0;
        lastWidth             = 0;
        lastHeight            = 0;
        lastPixfmt            = AV_PIX_FMT_NONE;
    }

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
    bool decodeVideoPacket() const;

    int decodeVideoNew(AVCodecContext* const avContext,
                       AVFrame* const avFrame,
                       int* gotFrame,
                       AVPacket* const avPacket) const;

    bool getVideoPacket();

    void convertAndScaleFrame(AVPixelFormat format,
                              int scaledSize,
                              bool maintainAspectRatio,
                              int& scaledWidth,
                              int& scaledHeight);

    void createAVFrame(AVFrame** const avFrame,
                       quint8** const frameBuffer,
                       int width,
                       int height,
                       AVPixelFormat format);

    void calculateDimensions(int squareSize,
                             bool maintainAspectRatio,
                             int& destWidth,
                             int& destHeight);

    void deleteFilterGraph();
    bool initFilterGraph(enum AVPixelFormat pixfmt, int width, int height);

    bool processFilterGraph(AVFrame* const dst,
                            const AVFrame* const src,
                            enum AVPixelFormat pixfmt,
                            int width,
                            int height);
};

void MovieDecoder::Private::createAVFrame(AVFrame** const avFrame,
                                          quint8** const frameBuffer,
                                          int width,
                                          int height,
                                          AVPixelFormat format)
{
    *avFrame     = av_frame_alloc();
    int numBytes = av_image_get_buffer_size(format, width, height, 1);
    *frameBuffer = reinterpret_cast<quint8*>(av_malloc(numBytes));

    av_image_fill_arrays((*avFrame)->data, (*avFrame)->linesize, *frameBuffer, format, width, height, 1);
}

void MovieDecoder::Private::initializeVideo()
{
    for (unsigned int i = 0 ; i < pFormatContext->nb_streams ; i++)
    {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            pVideoStream = pFormatContext->streams[i];
            videoStream  = i;
            break;
        }
    }

    if (videoStream == -1)
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "Could not find video stream";
        return;
    }

    pVideoCodecParameters = pFormatContext->streams[videoStream]->codecpar;
    pVideoCodec           = avcodec_find_decoder(pVideoCodecParameters->codec_id);

    if (pVideoCodec == 0)
    {
        // set to 0, otherwise avcodec_close(d->pVideoCodecContext) crashes
        pVideoCodecContext = 0;
        qDebug(DIGIKAM_GENERAL_LOG) << "Video Codec not found";
        return;
    }

    pVideoCodecContext = avcodec_alloc_context3(pVideoCodec);
    avcodec_parameters_to_context(pVideoCodecContext, pVideoCodecParameters);

    if (avcodec_open2(pVideoCodecContext, pVideoCodec, 0) < 0)
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "Could not open video codec";
    }
}

bool MovieDecoder::Private::decodeVideoPacket() const
{
    if (pPacket->stream_index != videoStream)
    {
        return false;
    }

    av_frame_unref(pFrame);

    int frameFinished = 0;

#if LIBAVCODEC_VERSION_MAJOR < 53
    int bytesDecoded = avcodec_decode_video(pVideoCodecContext,
                                            pFrame,
                                            &frameFinished,
                                            pPacket->data,
                                            pPacket->size);
#else
    int bytesDecoded = decodeVideoNew(pVideoCodecContext,
                                      pFrame,
                                      &frameFinished,
                                      pPacket);
#endif

    if (bytesDecoded < 0)
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "Failed to decode video frame: bytesDecoded < 0";
    }

    return (frameFinished > 0);
}

int MovieDecoder::Private::decodeVideoNew(AVCodecContext* const avContext,
                                          AVFrame* const avFrame,
                                          int* gotFrame,
                                          AVPacket* const avPacket) const
{
    int ret   = 0;
    *gotFrame = 0;

    if (avPacket)
    {
        ret = avcodec_send_packet(avContext, avPacket);
        // In particular, we don't expect AVERROR(EAGAIN), because we read all
        // decoded frames with avcodec_receive_frame() until done.
        if (ret < 0)
        {
            return (ret == AVERROR_EOF ? 0 : ret);
        }
    }

    ret = avcodec_receive_frame(avContext, avFrame);

    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
    {
        return ret;
    }

    if (ret >= 0)
    {
        *gotFrame = 1;
    }

    return 0;
}

bool MovieDecoder::Private::getVideoPacket()
{
    bool framesAvailable = true;
    bool frameDecoded    = false;

    int attempts = 0;

    if (pPacket)
    {
        av_packet_unref(pPacket);
        delete pPacket;
    }

    pPacket = new AVPacket();

    while (framesAvailable &&
           !frameDecoded &&
           (attempts++ < 1000))
    {
        framesAvailable = (av_read_frame(pFormatContext, pPacket) >= 0);

        if (framesAvailable)
        {
            frameDecoded = (pPacket->stream_index == videoStream);

            if (!frameDecoded)
            {
                av_packet_unref(pPacket);
            }
        }
    }

    return frameDecoded;
}

void MovieDecoder::Private::deleteFilterGraph()
{
    if (filterGraph)
    {
        av_frame_free(&filterFrame);
        avfilter_graph_free(&filterGraph);
        filterGraph = 0;
    }
}

bool MovieDecoder::Private::initFilterGraph(enum AVPixelFormat pixfmt,
                                            int width, int height)
{
    AVFilterInOut* inputs  = 0;
    AVFilterInOut* outputs = 0;

    deleteFilterGraph();
    filterGraph            = avfilter_graph_alloc();

    QByteArray arguments("buffer=");
    arguments += "video_size=" + QByteArray::number(width)  + "x" + QByteArray::number(height) + ":";
    arguments += "pix_fmt="    + QByteArray::number(pixfmt) + ":";
    arguments += "time_base=1/1:pixel_aspect=0/1[in];";
    arguments += "[in]yadif[out];";
    arguments += "[out]buffersink";

    int ret = avfilter_graph_parse2(filterGraph, arguments.constData(), &inputs, &outputs);

    if (ret < 0)
    {
        qWarning(DIGIKAM_GENERAL_LOG) << "Unable to parse filter graph";
        return false;
    }

    if (inputs || outputs)
    {
        return -1;
    }

    ret = avfilter_graph_config(filterGraph, nullptr);

    if (ret < 0)
    {
        qWarning(DIGIKAM_GENERAL_LOG) << "Unable to validate filter graph";
        return false;
    }

    bufferSourceContext = avfilter_graph_get_filter(filterGraph, "Parsed_buffer_0");
    bufferSinkContext   = avfilter_graph_get_filter(filterGraph, "Parsed_buffersink_2");

    if (!bufferSourceContext || !bufferSinkContext)
    {
        qWarning(DIGIKAM_GENERAL_LOG) << "Unable to get source or sink";
        return false;
    }

    filterFrame = av_frame_alloc();
    lastWidth   = width;
    lastHeight  = height;
    lastPixfmt  = pixfmt;

    return true;
}

bool MovieDecoder::Private::processFilterGraph(AVFrame* const dst,
                                               const AVFrame* const src,
                                               enum AVPixelFormat pixfmt,
                                               int width, int height)
{
    if (!filterGraph         ||
        width  != lastWidth  ||
        height != lastHeight ||
        pixfmt != lastPixfmt)
    {

        if (!initFilterGraph(pixfmt, width, height))
        {
            return false;
        }
    }

    memcpy(filterFrame->data,     src->data,     sizeof(src->data));
    memcpy(filterFrame->linesize, src->linesize, sizeof(src->linesize));

    filterFrame->width  = width;
    filterFrame->height = height;
    filterFrame->format = pixfmt;
    int ret             = av_buffersrc_add_frame(bufferSourceContext, filterFrame);

    if (ret < 0)
    {
        return false;
    }

    ret = av_buffersink_get_frame(bufferSinkContext, filterFrame);

    if (ret < 0)
    {
        return false;
    }

    av_image_copy(dst->data, dst->linesize, (const uint8_t**)filterFrame->data, filterFrame->linesize, pixfmt, width, height);
    av_frame_unref(filterFrame);

    return true;
}

void MovieDecoder::Private::convertAndScaleFrame(AVPixelFormat format,
                                                 int scaledSize,
                                                 bool maintainAspectRatio,
                                                 int& scaledWidth,
                                                 int& scaledHeight)
{
    calculateDimensions(scaledSize, maintainAspectRatio, scaledWidth, scaledHeight);
    SwsContext* const scaleContext = sws_getContext(pVideoCodecContext->width,
                                                    pVideoCodecContext->height,
                                                    pVideoCodecContext->pix_fmt,
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
              pFrame->data,
              pFrame->linesize,
              0,
              pVideoCodecContext->height,
              convertedFrame->data,
              convertedFrame->linesize);

    sws_freeContext(scaleContext);

    av_frame_free(&pFrame);
    av_free(pFrameBuffer);

    pFrame       = convertedFrame;
    pFrameBuffer = convertedFrameBuffer;
}

void MovieDecoder::Private::calculateDimensions(int squareSize,
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
        int srcWidth            = pVideoCodecContext->width;
        int srcHeight           = pVideoCodecContext->height;
        int ascpectNominator    = pVideoCodecContext->sample_aspect_ratio.num;
        int ascpectDenominator  = pVideoCodecContext->sample_aspect_ratio.den;

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

// ----------------------------------------------------------------------------

MovieDecoder::MovieDecoder(const QString& filename)
    : d(new Private)
{
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

    if (avformat_open_input(&d->pFormatContext,
                            fileInfo.absoluteFilePath().toLocal8Bit().data(), NULL, NULL) != 0)
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

    d->initializeVideo();
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
    d->deleteFilterGraph();

    if (d->pVideoCodecContext)
    {
        avcodec_close(d->pVideoCodecContext);
        d->pVideoCodecContext = 0;
    }

    if (d->pFormatContext)
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
        avcodec_flush_buffers(d->pVideoCodecContext);
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
            d->getVideoPacket();
            gotFrame = d->decodeVideoPacket();
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

    while (!frameFinished && d->getVideoPacket())
    {
        frameFinished = d->decodeVideoPacket();
    }

    if (!frameFinished)
    {
        qDebug(DIGIKAM_GENERAL_LOG) << "decodeVideoFrame() failed: frame not finished";
        return;
    }
}

void MovieDecoder::getScaledVideoFrame(int scaledSize,
                                       bool maintainAspectRatio,
                                       VideoFrame& videoFrame)
{
    if (d->pFrame->interlaced_frame)
    {
        d->processFilterGraph(d->pFrame,
                              d->pFrame,
                              d->pVideoCodecContext->pix_fmt,
                              d->pVideoCodecContext->width,
                              d->pVideoCodecContext->height);
    }

    int scaledWidth, scaledHeight;
    d->convertAndScaleFrame(AV_PIX_FMT_RGB24,
                            scaledSize,
                            maintainAspectRatio,
                            scaledWidth,
                            scaledHeight);

    videoFrame.width    = scaledWidth;
    videoFrame.height   = scaledHeight;
    videoFrame.lineSize = d->pFrame->linesize[0];

    videoFrame.frameData.clear();
    videoFrame.frameData.resize(videoFrame.lineSize * videoFrame.height);
    memcpy((&(videoFrame.frameData.front())),
           d->pFrame->data[0],
           videoFrame.lineSize * videoFrame.height);
}

} // namespace Digikam
