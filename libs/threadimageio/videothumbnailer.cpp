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
#include <QFile>

// Local includes

#include "thumbnailsize.h"
#include "digikam_debug.h"

namespace Digikam
{

QPointer<VideoThumbnailer> VideoThumbnailer::internalPtr = QPointer<VideoThumbnailer>();

VideoThumbnailer::VideoThumbnailer(QObject* const parent)
    : QObject(parent),
      d(new Private(this))
{
}

VideoThumbnailer::~VideoThumbnailer()
{
    // No need to delete d container, it's based on QObject.
}

VideoThumbnailer* VideoThumbnailer::instance()
{
    if (VideoThumbnailer::internalPtr.isNull())
    {
        VideoThumbnailer::internalPtr = new VideoThumbnailer();
    }

    return VideoThumbnailer::internalPtr;
}

bool VideoThumbnailer::isCreated()
{
    return (!internalPtr.isNull());
}

bool VideoThumbnailer::isReady() const
{
    return d->isReady;
}

bool VideoThumbnailer::getThumbnail(quintptr job, const QString& file, int size, bool strip)
{
    d->isReady     = false;
    d->thumbJob    = job;
    d->createStrip = strip;

    if (size < ThumbnailSize::Step || size > ThumbnailSize::HD)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid video thumbnail size : " << size;
        d->thumbSize = ThumbnailSize::Huge;
    }
    else
    {
        d->thumbSize = size;
    }

    if (!d->probe->setSource(d->player))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Video monitoring is not available.";
        d->isReady = true;
        return false;
    }

    if (!QFile::exists(file))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Video file " << file << " does not exist.";
        d->isReady = true;
        return false;
    }

    QMimeDatabase mimeDB;

    if (!mimeDB.mimeTypeForFile(file).name().startsWith(QLatin1String("video/")))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Mime type is not video from " << file;
        d->isReady = true;
        return false;
    }

    d->media = QMediaContent(QUrl::fromLocalFile(file));
    d->player->setMedia(d->media);
    d->player->setMuted(true);

    return true;
}

}  // namespace Digikam
