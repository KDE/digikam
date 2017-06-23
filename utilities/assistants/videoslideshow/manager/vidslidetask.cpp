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

// C++ includes

#include <cmath>

// Qt includes

#include <QImage>
#include <QSize>
#include <QPainter>
#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>

// QtAv includes

#include <QtAV/VideoFrame.h>
#include <QtAV/AudioFrame.h>
#include <QtAV/VideoEncoder.h>
#include <QtAV/AudioEncoder.h>
#include <QtAV/AudioDecoder.h>
#include <QtAV/AVMuxer.h>
#include <QtAV/AVDemuxer.h>

// Local includes

#include "frameutils.h"
#include "dfileoperations.h"
#include "transitionmngr.h"
#include "effectmngr.h"
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
        astream  = 0;
        adec     = AudioDecoder::create("FFmpeg");
    }

    ~Private()
    {
        adec->close();
    }

    bool          encodeFrame(VideoFrame& vframe,
                              VideoEncoder* const venc,
                              AudioEncoder* const aenc,
                              AVMuxer& mux);

    AudioFrame    nextAudioFrame(const AudioFormat& afmt);

public:

    VidSlideSettings*           settings;

    AVDemuxer                   demuxer;
    Packet                      apkt;
    int                         astream;
    AudioDecoder*               adec;
    QList<QUrl>::const_iterator curAudioFile;
};

bool VidSlideTask::Private::encodeFrame(VideoFrame& vframe,
                                        VideoEncoder* const venc,
                                        AudioEncoder* const aenc,
                                        AVMuxer& mux)
{
    Packet apkt;
    Packet vpkt;

    if (curAudioFile != settings->inputAudio.constEnd())
    {
        AudioFrame aframe = nextAudioFrame(aenc->audioFormat());

        if (!apkt.isValid())
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Invalid audio frame";
        }
        else
        {
            if (aenc->encode(aframe))
            {
                apkt = aenc->encoded();
            }
            else
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to encode audio frame";
            }
        }
    }

    if (vframe.pixelFormat() != venc->pixelFormat())
    {
        vframe = vframe.to(venc->pixelFormat());
    }

    if (venc->encode(vframe))
    {
        vpkt = venc->encoded();

        if (vpkt.isValid())
            mux.writeVideo(vpkt);

        if (apkt.isValid())
            mux.writeAudio(apkt);

        return true;
    }

    return false;
}

AudioFrame VidSlideTask::Private::nextAudioFrame(const AudioFormat& afmt)
{
    if (curAudioFile == settings->inputAudio.constEnd())
        return AudioFrame();

    if (demuxer.atEnd() || demuxer.fileName().isEmpty())
    {
        if (demuxer.fileName().isEmpty())
        {
            curAudioFile = settings->inputAudio.constBegin();
        }
        else
        {
            curAudioFile++;
        }

        if (curAudioFile != settings->inputAudio.constEnd())
        {
            demuxer.setMedia((*curAudioFile).toLocalFile());

            if (!demuxer.load())
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to open audio file" << demuxer.fileName();
                return AudioFrame();
            }

            adec->setCodecContext(demuxer.audioCodecContext());

            if (!adec->open())
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to open audio stream in decode"
                                               << demuxer.fileName();
                return AudioFrame();
            }
        }
        else
        {
            return AudioFrame();
        }
    }

    while (!demuxer.atEnd())
    {
        if (!apkt.isValid())
        {
            if (!demuxer.readFrame() || demuxer.stream() != astream)
                continue;

            apkt = demuxer.packet();
        }

        if (!adec->decode(apkt))
        {
            apkt = Packet();
            continue;
        }

        apkt.data = QByteArray::fromRawData(apkt.data.constData() + apkt.data.size() -
                                            adec->undecodedSize(), adec->undecodedSize());

        AudioFrame aframe = adec->frame();

        if (aframe.format() != afmt)
        {
            qDebug() << "Audio transcoding:";
            qDebug() << "current format =" << aframe.format();
            qDebug() << "target format  =" << afmt;
/*
            adec->resampler()->setOutAudioFormat(afmt);
            adec->resampler()->prepare();
            aframe.setAudioResampler(adec->resampler());
*/
            aframe = aframe.to(afmt);
        }

        return aframe;
    }

    return AudioFrame();
}

// -------------------------------------------------------

VidSlideTask::VidSlideTask(VidSlideSettings* const settings)
    : ActionJob(),
      d(new Private)
{
    d->settings = settings;

    if (d->settings->inputAudio.isEmpty())
    {
        d->curAudioFile = d->settings->inputAudio.constEnd();
    }
}

VidSlideTask::~VidSlideTask()
{
    cancel();
    delete d;
}

void VidSlideTask::run()
{
    // ---------------------------------------------
    // Setup output video file

    QUrl dest       = d->settings->outputDir;
    dest            = dest.adjusted(QUrl::StripTrailingSlash);
    dest.setPath(dest.path() + QLatin1String("/videoslideshow.") + d->settings->videoFormat());
    QString outFile = dest.toLocalFile();
    QFileInfo fi(outFile);

    if (fi.exists() && d->settings->conflictRule != FileSaveConflictBox::OVERWRITE)
    {
        outFile = DFileOperations::getUniqueFileUrl(dest).toLocalFile();
    }

    // ---------------------------------------------
    // Setup Video Encoder

    VideoEncoder* const venc = VideoEncoder::create("FFmpeg");
    venc->setCodecName(d->settings->videoCodec());
    venc->setBitRate(d->settings->videoBitRate());
    venc->setFrameRate(d->settings->videoFrameRate());

    QSize osize = d->settings->videoSize();
    venc->setWidth(osize.width());
    venc->setHeight(osize.height());

    if (!venc->open())
    {
        emit signalMessage(i18n("Failed to open video encoder"), true);
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to open video encoder";
        emit signalDone(false);
        return;
    }

    // ---------------------------------------------
    // Setup Audio Encoder

    AudioEncoder* const aenc = AudioEncoder::create("FFmpeg");
    aenc->setCodecName(QLatin1String("mp2"));
    aenc->setBitRate(d->settings->abitRate);

    if (!aenc->open())
    {
        emit signalMessage(i18n("Failed to open audio encoder"), true);
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to open audio encoder";
        emit signalDone(false);
        return;
    }

    // ---------------------------------------------
    // Setup Muxer

    AVMuxer mux;
    mux.setMedia(outFile);
    mux.copyProperties(venc);  // Setup video encoder
    mux.copyProperties(aenc);  // Setup audio encoder
/*
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
*/
    if (!mux.open())
    {
        emit signalMessage(i18n("Failed to open muxer"), true);
        qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to open muxer";
        emit signalDone(false);
        return;
    }

    QImage qiimg;

    // ---------------------------------------------
    // Loop to encode frames with images list

    TransitionMngr transmngr;
    transmngr.setOutputSize(osize);

    EffectMngr effmngr;
    effmngr.setOutputSize(osize);
    effmngr.setFrames(d->settings->imgFrames);

    for (int i = 0 ; i < d->settings->inputImages.count()+1 && !m_cancel ; i++)
    {
        if (i == 0)
            qiimg = FrameUtils::makeFramedImage(QString(), osize);

        QString ofile;

        if (i < d->settings->inputImages.count())
            ofile = d->settings->inputImages[i].toLocalFile();

        QImage qoimg = FrameUtils::makeFramedImage(ofile, osize);

        // -- Transition encoding ----------

        transmngr.setInImage(qiimg);
        transmngr.setOutImage(qoimg);
        transmngr.setTransition(d->settings->transition);

        int tmout = 0;

        do
        {
            VideoFrame frame(transmngr.currentFrame(tmout));

            if (!d->encodeFrame(frame, venc, aenc, mux))
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot encode transition frame";
            }
        }
        while (tmout != -1 && !m_cancel);

        // -- Images encoding ----------

        if (i < d->settings->inputImages.count())
        {
            VideoFrame frame;
            int count = 0;
            int tmout = 0;
            effmngr.setImage(qoimg);
            effmngr.setEffect(d->settings->vEffect);

            do
            {
                qiimg = effmngr.currentFrame(tmout);
                frame = VideoFrame(qiimg);

                if (d->encodeFrame(frame, venc, aenc, mux))
                {

                    count++;
/*
                    qCDebug(DIGIKAM_GENERAL_LOG) << ofile
                                                 << " => encode count:" << count
                                                 << "frame size:"       << frame.width()
                                                 << "x"                 << frame.height();
*/
                }
            }
            while (count < d->settings->imgFrames && !m_cancel);
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Encoded image" << i << "done";

        emit signalMessage(i18n("Encoding %1 Done", ofile), false);
        emit signalProgress(i);
    }

    // ---------------------------------------------
    // Get delayed frames

    qCDebug(DIGIKAM_GENERAL_LOG) << "Encode delayed frames...";

    while (venc->encode() && !m_cancel)
    {
        Packet vpkt(venc->encoded());

        if (vpkt.isValid())
            mux.writeVideo(vpkt);

        Packet apkt(aenc->encoded());

        if (apkt.isValid())
            mux.writeAudio(apkt);
    }

    // ---------------------------------------------
    // Cleanup

    venc->close();
    aenc->close();
    mux.close();

    if (!m_cancel)
    {
        emit signalMessage(i18n("Output video is %1", outFile), false);
        d->settings->outputVideo = outFile;
    }

    emit signalDone(!m_cancel);
}

} // namespace Digikam
