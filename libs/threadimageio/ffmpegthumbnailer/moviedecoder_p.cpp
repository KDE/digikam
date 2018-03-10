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

#include "moviedecoder_p.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

MovieDecoder::Private::Private()
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

MovieDecoder::Private::~Private()
{
}

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
    int  attempts        = 0;

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

} // namespace Digikam
