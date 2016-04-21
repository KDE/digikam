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

#include "loadvideothumb_p.h"

// Qt includes

#include <QApplication>
#include <QImage>
#include <QDebug>

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
    d->thumbSize = size;
}

void VideoThumbnailer::setCreateStrip(bool strip)
{
    d->createStrip = strip;
}

bool VideoThumbnailer::getThumbnail(const QString& file)
{
    if (!d->probe->setSource(d->player))
    {
        qDebug() << "Video monitoring is not available.";
        return false;
    }

    d->media = QMediaContent(QUrl::fromLocalFile(file));
    d->player->setMedia(d->media);

    return true;
}
