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
#include "transitionmngr.h"

// QtAv includes

#include <QtAV/QtAV.h>
#include <QtAV/VideoFrame.h>
#include <QtAV/VideoEncoder.h>
#include <QtAV/AVMuxer.h>

using namespace Digikam;
using namespace QtAV;

QImage makeFramedImage(const QString& file, const QSize& outSize)
{
    QImage qimg(outSize, QImage::Format_ARGB32);
    qimg.fill(QColor(0, 0, 0, 0));

    if (!file.isEmpty())
    {
        qDebug() << "Load frame from" << file;

        // The Raw decoding settings for DImg loader.
        DRawDecoderSettings settings;
        settings.halfSizeColorImage    = false;
        settings.sixteenBitsImage      = false;
        settings.RGBInterpolate4Colors = false;
        settings.RAWQuality            = DRawDecoderSettings::BILINEAR;

        DImg dimg(file, 0, DRawDecoding(settings));
        QImage timg = dimg.copyQImage();
        timg        = timg.scaled(outSize, Qt::KeepAspectRatio);

        QPainter p(&qimg);
        p.drawImage((qimg.width()  / 2) - (timg.width()  / 2),
                    (qimg.height() / 2) - (timg.height() / 2),
                    timg);
    }

    return qimg;
}

bool encodeFrame(VideoFrame& frame, VideoEncoder* const venc, AVMuxer& mux)
{
    if (frame.pixelFormat() != venc->pixelFormat())
    {
        frame = frame.to(venc->pixelFormat());
    }

    if (venc->encode(frame))
    {
        Packet pkt(venc->encoded());
        mux.writeVideo(pkt);
        return true;
    }

    return false;
}

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

    TransitionMngr tmngr;
    tmngr.setOutputSize(outSize);
    tmngr.setTransition(TransitionMngr::HorizontalLines);

    for (int i = -1 ; i < list.count() ; i++)
    {
        QString ifile = (i >= 0)             ? list[i]   : QString();
        QString ofile = (i+1 < list.count()) ? list[i+1] : QString();
        QImage qiimg  = makeFramedImage(ifile, outSize);
        QImage qoimg  = makeFramedImage(ofile, outSize);

        tmngr.setInImage(qiimg);
        tmngr.setOutImage(qoimg);

        qDebug() << "Making transition between" << ifile << "and" << ofile;

        int tmout = 0;
        int j     = 0;

        do
        {
            VideoFrame frame(tmngr.currentframe(tmout));

            if (encodeFrame(frame, venc, mux))
            {
                qDebug() << "Transition frame:" << j++ << tmout;
            }
        }
        while (tmout != -1 && j < 120);

        VideoFrame frame(qoimg);

        int count = 0;

        do
        {
            if (encodeFrame(frame, venc, mux))
            {
                count++;

                qDebug() << ofile
                         << " => encode count:" << count
                         << "frame size:"       << frame.width()
                         << "x"                 << frame.height();
            }
        }
        while (count < aframes);
    }

    // ---------------------------------------------
    // Get delayed frames

    qDebug("encode delayed frames...");

    while (venc->encode())
    {
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
