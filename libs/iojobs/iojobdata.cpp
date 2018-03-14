/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-02-24
 * Description : Container for IOJob data.
 *
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "iojobdata.h"

// Local includes

#include "album.h"
#include "imageinfo.h"
#include "digikam_debug.h"

namespace Digikam
{

class IOJobData::Private
{
public:

    Private()
        : operation(0),
          srcAlbum(0),
          destAlbum(0)
    {
    }

    int              operation;

    PAlbum*          srcAlbum;
    PAlbum*          destAlbum;

    QList<ImageInfo> imageInfoList;
    QList<QUrl>      sourceUrlList;

    QUrl             destUrl;
};

IOJobData::IOJobData(int operation,
                     const QList<ImageInfo>& infos,
                     PAlbum* const dest)
    : d(new Private)
{
    d->operation = operation;
    d->destAlbum = dest;

    setImageInfos(infos);

    if (d->destAlbum)
    {
        d->destUrl = d->destAlbum->fileUrl();
    }
}

IOJobData::IOJobData(int operation,
                     const QList<QUrl>& urls,
                     PAlbum* const dest)
    : d(new Private)
{
    d->operation     = operation;
    d->sourceUrlList = urls;
    d->destAlbum     = dest;

    if (d->destAlbum)
    {
        d->destUrl = d->destAlbum->fileUrl();
    }
}

IOJobData::IOJobData(int operation,
                     PAlbum* const src,
                     PAlbum* const dest)
    : d(new Private)
{
    d->operation = operation;
    d->srcAlbum  = src;
    d->destAlbum = dest;

    d->sourceUrlList.clear();

    if (d->srcAlbum)
    {
        d->sourceUrlList << d->srcAlbum->fileUrl();
    }

    if (d->destAlbum)
    {
        d->destUrl = d->destAlbum->fileUrl();
    }
}

IOJobData::IOJobData(int operation,
                     const QList<QUrl>& urls,
                     const QUrl& dest)
    : d(new Private)
{
    d->operation     = operation;
    d->sourceUrlList = urls;
    d->destUrl       = dest;
}

IOJobData::IOJobData(int operation, const ImageInfo& info, const QString& newName)
    : d(new Private)
{
    d->operation = operation;

    setImageInfos(QList<ImageInfo>() << info);

    d->destUrl = srcUrl().adjusted(QUrl::RemoveFilename);
    d->destUrl.setPath(d->destUrl.path() + newName);
}

IOJobData::~IOJobData()
{
    delete d;
}

void IOJobData::setImageInfos(const QList<ImageInfo>& infos)
{
    d->imageInfoList = infos;

    d->sourceUrlList.clear();

    foreach(const ImageInfo& info, d->imageInfoList)
    {
        d->sourceUrlList << info.fileUrl();
    }
}

void IOJobData::setSourceUrls(const QList<QUrl>& urls)
{
    d->sourceUrlList = urls;
}

void IOJobData::setDestUrl(const QUrl& url)
{
    d->destUrl = url;
}

int IOJobData::operation() const
{
    return d->operation;
}

PAlbum* IOJobData::srcAlbum() const
{
    return d->srcAlbum;
}

PAlbum* IOJobData::destAlbum() const
{
    return d->destAlbum;
}

QUrl IOJobData::srcUrl() const
{
    if (d->sourceUrlList.isEmpty())
    {
        return QUrl();
    }

    return d->sourceUrlList.first();
}

QUrl IOJobData::destUrl() const
{
    return d->destUrl;
}

ImageInfo IOJobData::imageInfo() const
{
    if (d->imageInfoList.isEmpty())
    {
        return ImageInfo();
    }

    return d->imageInfoList.first();
}

QList<QUrl> IOJobData::sourceUrls() const
{
    return d->sourceUrlList;
}

QList<ImageInfo> IOJobData::imageInfos() const
{
    return d->imageInfoList;
}

} // namespace Digikam
