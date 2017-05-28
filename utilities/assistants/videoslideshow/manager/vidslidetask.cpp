/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "vidslidetask.h"

// Qt includes

#include <QImage>
#include <QSize>
#include <QPainter>

// QtAv includes

#include <QtAV/QtAV.h>
#include <QtAV/VideoFrame.h>
#include <QtAV/VideoEncoder.h>
#include <QtAV/AVMuxer.h>

// Local includes

#include "dimg.h"
#include "drawdecoding.h"
#include "transitionmngr.h"
#include "digikam_debug.h"
#include "digikam_config.h"

using namespace QtAV;

namespace Digikam
{

class VidSlideTask::Private
{
public:

    Private()
    {
        settings = 0;
        cancel   = false;
    }


    bool encodeFrame(VideoFrame& frame, VideoEncoder* const venc, AVMuxer& mux) const;
    QImage makeFramedImage(const QString& file, const QSize& outSize) const;

public:

    bool              cancel;

    VidSlideSettings* settings;
};


QImage VidSlideTask::Private::makeFramedImage(const QString& file, const QSize& outSize) const
{
    QImage qimg(outSize, QImage::Format_ARGB32);
    qimg.fill(QColor(0, 0, 0, 0));

    if (!file.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Load frame from" << file;

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

bool VidSlideTask::Private::encodeFrame(VideoFrame& frame, VideoEncoder* const venc, AVMuxer& mux) const
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

// -------------------------------------------------------

VidSlideTask::VidSlideTask(VidSlideSettings* const settings)
    : ActionJob(),
      d(new Private)
{
    d->settings = settings;
}

VidSlideTask::~VidSlideTask()
{
    slotCancel();
    delete d;
}

void VidSlideTask::slotCancel()
{
    d->cancel = true;
}

void VidSlideTask::run()
{
    if (d->cancel)
    {
        return;
    }

    // The output video file
    QString outFile = d->settings->outputVideo.toLocalFile();

    // ---------------------------------------------
    // Setup Encoder

    VideoEncoder* const venc       = VideoEncoder::create("FFmpeg");
    venc->setCodecName(QLatin1String("libx264"));
    venc->setBitRate(1024*1024);
    venc->setWidth(d->settings->outputSize.width());
    venc->setHeight(d->settings->outputSize.height());

    if (!venc->open())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "failed to open encoder";
        return;
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
    avfopt[QLatin1String("segment_list")]      = outFile.left(outFile.lastIndexOf(QLatin1Char('/'))+1)
                                                              .append(QLatin1String("index.m3u8"));
    QVariantHash muxopt;
    muxopt[QLatin1String("avformat")]          = avfopt;

    mux.setOptions(muxopt);

    if (!mux.open())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "failed to open muxer";
        return;
    }

    // ---------------------------------------------
    // Loop to encode frames with images list

    TransitionMngr tmngr;
    tmngr.setOutputSize(d->settings->outputSize);
    tmngr.setEffect(TransitionMngr::HorizontalLines);

    for (int i = -1 ; i < d->settings->inputImages.count() ; i++)
    {
        QString ifile = (i >= 0)
                        ? d->settings->inputImages[i].toLocalFile()
                        : QString();
        QString ofile = (i+1 < d->settings->inputImages.count())
                        ? d->settings->inputImages[i+1].toLocalFile()
                        : QString();

        QImage qiimg  = d->makeFramedImage(ifile, d->settings->outputSize);
        QImage qoimg  = d->makeFramedImage(ofile, d->settings->outputSize);

        tmngr.setInImage(qiimg);
        tmngr.setOutImage(qoimg);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Making transition between" << ifile << "and" << ofile;

        int tmout = 0;
        int j     = 0;

        do
        {
            VideoFrame frame(tmngr.currentframe(tmout));

            if (d->encodeFrame(frame, venc, mux))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Transition frame:" << j++ << tmout;
            }
        }
        while (tmout != -1);

        VideoFrame frame(qoimg);

        int count = 0;

        do
        {
            if (d->encodeFrame(frame, venc, mux))
            {
                count++;

                qCDebug(DIGIKAM_GENERAL_LOG) << ofile
                                             << " => encode count:" << count
                                             << "frame size:"       << frame.width()
                                             << "x"                 << frame.height();
            }
        }
        while (count < d->settings->aframes);
    }

    // ---------------------------------------------
    // Get delayed frames

    qCDebug(DIGIKAM_GENERAL_LOG) << "encode delayed frames...";

    while (venc->encode())
    {
        Packet pkt(venc->encoded());
        mux.writeVideo(pkt);
    }

    // ---------------------------------------------
    // Cleanup

    venc->close();
    mux.close();

    emit signalDone();
}

}  // namespace Digikam
