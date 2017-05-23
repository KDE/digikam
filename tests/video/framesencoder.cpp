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

#include <QFileInfo>
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
    if (argc != 2)
    {
        qDebug() << "framesencoder - images to encode as video stream";
        qDebug() << "Usage: <image>";
        return -1;
    }

    MetaEngine::initializeExiv2();

    QFileInfo input(QString::fromUtf8(argv[1]));

    DRawDecoderSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = false;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

    DImg img(input.filePath(), 0, DRawDecoding(settings));
    VideoFrame frame(img.copyQImage());

    // ---------------------------------------------

    VideoEncoder* const venc       = VideoEncoder::create("FFmpeg");
    venc->setCodecName(QLatin1String("libx264"));
    venc->setBitRate(1024*1024);
    venc->setWidth(frame.width());
    venc->setHeight(frame.height());

    if (!venc->open())
    {
        qWarning("failed to open encoder");
        return -1;
    }

    // ---------------------------------------------

    QString outFile                            = QLatin1String("./out.mp4");
    AVMuxer mux;
    mux.setMedia(outFile);
    mux.copyProperties(venc);

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

            qDebug() << "encode count:" << count
                     << "frame size:" << frame.width()
                     << "x" <<  frame.height()
                     << "::" << frame.data().size();
        }
    }
    while (count < 1000);

    // get delayed frames

    while (venc->encode())
    {
        qDebug("encode delayed frames...");
        Packet pkt(venc->encoded());
        mux.writeVideo(pkt);
    }

    // ---------------------------------------------

    venc->close();
    mux.close();

    MetaEngine::cleanupExiv2();

    return 0;
}
