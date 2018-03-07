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

#ifndef MOVIEDECODER_H
#define MOVIEDECODER_H

#include "videoframe.h"
#include <QString>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}

namespace Digikam
{

class MovieDecoder
{
public:
    MovieDecoder(const QString& filename, AVFormatContext* pavContext = NULL);
    ~MovieDecoder();

    QString getCodec();
    void seek(int timeInSeconds);
    void decodeVideoFrame();
    void getScaledVideoFrame(int scaledSize, bool maintainAspectRatio, VideoFrame& videoFrame);

    int getWidth();
    int getHeight();
    int getDuration();

    void initialize(const QString& filename);
    void destroy();
    bool getInitialized();

private:
    void initializeVideo();

    bool decodeVideoPacket();
    bool getVideoPacket();
    void convertAndScaleFrame(AVPixelFormat format, int scaledSize, bool maintainAspectRatio, int& scaledWidth, int& scaledHeight);
    void createAVFrame(AVFrame** avFrame, quint8** frameBuffer, int width, int height, AVPixelFormat format);
    void calculateDimensions(int squareSize, bool maintainAspectRatio, int& destWidth, int& destHeight);

    void deleteFilterGraph();
    bool initFilterGraph(enum AVPixelFormat pixfmt, int width, int height);
    bool processFilterGraph(AVPicture *dst, const AVPicture *src, enum AVPixelFormat pixfmt, int width, int height);

private:
    int                     m_VideoStream;
    AVFormatContext*        m_pFormatContext;
    AVCodecContext*         m_pVideoCodecContext;
    AVCodec*                m_pVideoCodec;
    AVStream*               m_pVideoStream;
    AVFrame*                m_pFrame;
    quint8*                 m_pFrameBuffer;
    AVPacket*               m_pPacket;
    bool                    m_FormatContextWasGiven;
    bool                    m_AllowSeek;
    bool                    m_initialized;
    AVFilterContext*        m_bufferSinkContext;
    AVFilterContext*        m_bufferSourceContext;
    AVFilterGraph*          m_filterGraph;
    AVFrame*                m_filterFrame;
    int                     m_lastWidth;
    int                     m_lastHeight;
    enum AVPixelFormat      m_lastPixfmt;
};

}

#endif
