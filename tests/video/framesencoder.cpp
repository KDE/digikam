/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-23
 * Description : a command line tool to encode images as a
 *               video stream.
 *
 * Copyright (C) 2017 by Gilles Caulier, <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include <QDebug>

// Local includes

#include "metaengine.h"
#include "dimg.h"
#include "drawdecoding.h"

// QtAv includes

#include <QtAV/QtAV.h>
#include <QtAV/VideoFrame.h>
#include <QtAV/VideoEncoder.h>
#include <QtAV/AVMuxer.h>

using namespace Digikam;
using namespace QtAV;

int main(int argc, char** argv)
{
    // ---------------------------------------------
    // Get list of image files from CLI

    QStringList list;

    if (argc > 1)
    {
        for (int i = 1 ; i < argc ; i++)
            list.append(QString::fromUtf8(argv[i]));
    }

    if (list.isEmpty())
    {
        qDebug() << "framesencoder - images to encode as video stream";
        qDebug() << "Usage: <list of image files>";
        return -1;
    }

    // ---------------------------------------------
    // Common settings

    MetaEngine::initializeExiv2();

    // The Raw decoding settings for DImg loader.
    DRawDecoderSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = false;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

    // The output video size
    QSize outSize(1024, 768);

    // The output video file
    QString outFile = QLatin1String("./out.mp4");

    // Amount of frames to encode in video stream by image.
    // ex: 120 frames = 5 s at 24 img/s.
    int aframes = 120;

    // ---------------------------------------------
    // Setup Encoder

    VideoEncoder* const venc       = VideoEncoder::create("FFmpeg");
    venc->setCodecName(QLatin1String("libx264"));
    venc->setBitRate(1024*1024);
    venc->setWidth(outSize.width());
    venc->setHeight(outSize.height());

    if (!venc->open())
    {
        qWarning("failed to open encoder");
        return -1;
    }

    // ---------------------------------------------
    // Setup Muxer

    AVMuxer mux;
    mux.setMedia(outFile);
    mux.copyProperties(venc);

    // Segments muxer ffmpeg options. See : https://www.ffmpeg.org/ffmpeg-formats.html#Options-11
    QVariantHash avfopt;
    avfopt[QLatin1String("segment_time")]      = 4;
    avfopt[QLatin1String("segment_list_size")] = 0;
    avfopt[QLatin1String("segment_format")]    = QLatin1String("mpegts");
    avfopt[QLatin1String("segment_list")]      = outFile.left(
                                                            outFile.lastIndexOf(QLatin1Char('/'))+1)
                                                            .append(QLatin1String("index.m3u8"));
    QVariantHash muxopt;
    muxopt[QLatin1String("avformat")]          = avfopt;

    mux.setOptions(muxopt);

    if (!mux.open())
    {
        qWarning() << "failed to open muxer";
        return -1;
    }

    // ---------------------------------------------
    // Loop to encode frames with images list

    foreach(const QString& file, list)
    {
        DImg img(file, 0, DRawDecoding(settings));
        QImage qimg = img.copyQImage();
        qimg        = qimg.scaled(outSize, Qt::KeepAspectRatio);
        VideoFrame frame(qimg);

        int count = 0;

        do
        {
            if (frame.pixelFormat() != venc->pixelFormat())
            {
                frame = frame.to(venc->pixelFormat());
            }

            if (venc->encode(frame))
            {
                Packet pkt(venc->encoded());
                mux.writeVideo(pkt);
                count++;

                qDebug() << file
                         << " => encode count:" << count
                         << "frame size:"       << frame.width()
                         << "x"                 << frame.height();
            }
        }
        while (count < aframes);
    }

    // ---------------------------------------------
    // Get delayed frames

    while (venc->encode())
    {
        qDebug("encode delayed frames...");
        Packet pkt(venc->encoded());
        mux.writeVideo(pkt);
    }

    // ---------------------------------------------
    // Cleanup

    venc->close();
    mux.close();

    MetaEngine::cleanupExiv2();

    return 0;
}
