/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : Qt Multimedia based video thumbnailer 
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

#include "videothumbnailer_p.h"

// Qt includes

#include <QMimeDatabase>
#include <QApplication>
#include <QImage>
#include <QDebug>

namespace Digikam
{

VideoThumbnailer::VideoThumbnailer(QObject* const parent)
    : QObject(parent),
      d(new Private(this))
{
}

VideoThumbnailer::~VideoThumbnailer()
{
    // No need to delete d container, it's based on QObject.
}

void VideoThumbnailer::setThumbnailSize(int size)
{
    if (size <= 0)
    {
        qDebug() << "Invalid video thumbnail size : " << size;
        return;
    }

    d->thumbSize = size;
}

void VideoThumbnailer::setCreateStrip(bool strip)
{
    d->createStrip = strip;
}

void VideoThumbnailer::slotGetThumbnail(const QString& file)
{
    if (!d->probe->setSource(d->player))
    {
        qDebug() << "Video monitoring is not available.";
        emit signalThumbnailFailed(file);
        return;
    }

    QMimeDatabase mimeDB;

    if (!mimeDB.mimeTypeForFile(file).name().startsWith(QLatin1String("video")))
    {
        qDebug() << "Mime type is not a video from " << file;
        emit signalThumbnailFailed(file);
        return;
    }

    d->media = QMediaContent(QUrl::fromLocalFile(file));
    d->player->setMedia(d->media);
}

}  // namespace Digikam
