/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : a command line tool to load video thumbnail
 *
 * Copyright (C) 2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "loadvideothumb.h"

// Qt includes

#include <QApplication>
#include <QMediaPlayer>
#include <QVideoProbe>
#include <QImage>
#include <QDebug>

VideoThumbnailer::VideoThumbnailer(QObject* const parent)
    : QObject(parent)
{
    m_position = 0;
    m_player   = new QMediaPlayer(this);
    m_probe    = new QVideoProbe(this);

    connect(m_player, SIGNAL(error(QMediaPlayer::Error)),
            this, SLOT(slotHandlePlayerError()));

    connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)));

    connect(m_probe, SIGNAL(videoFrameProbed(QVideoFrame)),
            this, SLOT(slotProcessframe(QVideoFrame)));
}

VideoThumbnailer::~VideoThumbnailer()
{
}

bool VideoThumbnailer::getThumbnail(const QString& file)
{
    if (!m_probe->setSource(m_player))
    {
        qDebug() << "Video monitoring is not available.";
        return false;
    }

    m_media = QMediaContent(QUrl::fromLocalFile(file));
    m_player->setMedia(m_media);

    return true;
}

void VideoThumbnailer::slotMediaStatusChanged(QMediaPlayer::MediaStatus state)
{
    if (m_player->currentMedia() != m_media)
        return;

    switch (state)
    {
        case QMediaPlayer::LoadedMedia:
        {
            if (!m_player->isSeekable())
            {
                qDebug() << "Video seek is not available for " << m_media.canonicalUrl().fileName();
                emit signalVideoThumbDone();
            }

            qDebug() << "Video duration for " << m_media.canonicalUrl().fileName() << "is " << m_player->duration() << " seconds";

            m_position = (qint64)(m_player->duration() * 0.2);

            m_player->setPosition(m_position);    // Seek to 20% of the media to take a thumb.
            m_player->pause();

            qDebug() << "Trying to get thumbnail from " << m_media.canonicalUrl().fileName() << " at position " << m_position;
            break;
        }
        case QMediaPlayer::InvalidMedia:
        {
            qDebug() << "Video cannot be decoded for " << m_media.canonicalUrl().fileName();
            emit signalVideoThumbDone(); 
        }
        default:
            break;
    }
}

void VideoThumbnailer::slotHandlePlayerError()
{
    qDebug() << "Problem while video data extraction from " << m_media.canonicalUrl().fileName();
    qDebug() << "Error : " << m_player->errorString();

    emit signalVideoThumbDone();
}

void VideoThumbnailer::slotProcessframe(QVideoFrame frm)
{
    if (m_player->mediaStatus() != QMediaPlayer::BufferedMedia)
        return;

    if (m_player->position() != m_position)
        return;

    qDebug() << "Video frame extraction from " << m_media.canonicalUrl().fileName()
             << " at position " << m_position;

    if (!frm.isValid())
    {
        qDebug() << "Error : Video frame is not valid.";
        emit signalVideoThumbDone();
    }

    frm.map(QAbstractVideoBuffer::ReadOnly);

    if (frm.isReadable())
    {
        QImage::Format format = QVideoFrame::imageFormatFromPixelFormat(frm.pixelFormat());

        if (format != QImage::Format_Invalid)
        {
            // Frame pixels data can be imported to QImage as well.
            QImage img(frm.bits(), frm.width(), frm.height(), QVideoFrame::imageFormatFromPixelFormat(frm.pixelFormat()));
            img.save(QString::fromUtf8("%1-thumb.png").arg(m_media.canonicalUrl().fileName()), "PNG");
            qDebug() << "Video frame extracted with size " << img.size();
        }
        else
        {
            // TODO : convert frame pixels data from unsuported format (YUV for ex) to QImage supported format (RGB)
            //        See OpenCV API for this kind of conversions.
            qDebug() << "Video frame format is not supported. Video frame not extracted.";
        }
    }
    else
    {
        qDebug() << "Error : Video frame is not readable.";
    }

    frm.unmap();

    emit signalVideoThumbDone();
}

// --------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qDebug() << "loadvideothumb - Load video file to extract thumbnail";
        qDebug() << "Usage: <videofile>";
        return -1;
    }

    QApplication app(argc, argv);
    VideoThumbnailer* const vthumb = new VideoThumbnailer(&app);

    QObject::connect(vthumb, SIGNAL(signalVideoThumbDone()),
                     &app, SLOT(quit()));

    if (!vthumb->getThumbnail(QString::fromUtf8(argv[1])))
        return -1;

    return app.exec();
}
