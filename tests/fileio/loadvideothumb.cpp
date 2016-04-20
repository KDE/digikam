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
    m_player = new QMediaPlayer(this);
    m_probe  = new QVideoProbe(this);

    connect(m_probe, SIGNAL(videoFrameProbed(QVideoFrame)),
            this, SLOT(slotProcessframe(QVideoFrame)));
}

bool VideoThumbnailer::getThumbnail(const QString& file)
{
    m_videoFile = file;

    if (m_probe->setSource(m_player))
    {
        qDebug() << "Monitoring is not available.";
        return false;
    }

    m_player->setMedia(QUrl::fromLocalFile(m_videoFile));
    m_player->setPosition(1000);

    qDebug() << "Trying to get thumbnail from " << file << "...";

    return true;
}

VideoThumbnailer::~VideoThumbnailer()
{
}

void VideoThumbnailer::slotProcessframe(QVideoFrame frm)
{
    frm.map(QAbstractVideoBuffer::ReadOnly);
    QImage img(frm.bits(), frm.width(), frm.height(), QVideoFrame::imageFormatFromPixelFormat(frm.pixelFormat()));
    img.save(QString::fromUtf8("%1-thumb.png").arg(m_videoFile), "PNG");
    frm.unmap();

    qDebug() << "Video thumbnail from " << m_videoFile <<" extracted.";

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
