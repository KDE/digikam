/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-02-29
 * Description : Drag object info containers.
 *
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ddragobjects.h"

// Qt includes

#include <QByteArray>

namespace Digikam
{

DItemDrag::DItemDrag(const KUrl::List& urls,
                     const KUrl::List& kioUrls,
                     const QList<int>& albumIDs,
                     const QList<qlonglong>& imageIDs)
    : QMimeData()
{
    // Digikam specific mime data
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << urls;
    setData("digikam/item-ids", ba);

    QByteArray  ba2;
    QDataStream ds2(&ba2, QIODevice::WriteOnly);
    ds2 << kioUrls;
    setData("digikam/digikamalbums", ba2);

    QByteArray  ba3;
    QDataStream ds3(&ba3, QIODevice::WriteOnly);
    ds3 << albumIDs;
    setData("digikam/album-ids", ba3);

    QByteArray  ba4;
    QDataStream ds4(&ba4, QIODevice::WriteOnly);
    ds4 << imageIDs;
    setData("digikam/image-ids-long", ba4);

    // commonly accessible mime data, for dragging to outside digikam
    urls.populateMimeData(this);
}

QStringList DItemDrag::mimeTypes()
{
    // we do not want to decode text/uri-list with this object,
    // we only export this data above for dragging to outside digikam.
    return QStringList() << "digikam/item-ids"
           << "digikam/album-ids"
           << "digikam/image-ids-long"
           << "digikam/digikamalbums";
}

bool DItemDrag::canDecode(const QMimeData* e)
{
    foreach(const QString& mimeType, mimeTypes())
    {
        if (!e->hasFormat(mimeType))
        {
            return false;
        }
    }
    return true;
}

bool DItemDrag::decode(const QMimeData* e,
                       KUrl::List& urls,
                       KUrl::List& kioUrls,
                       QList<int>& albumIDs,
                       QList<qlonglong>& imageIDs)
{
    urls.clear();
    kioUrls.clear();
    albumIDs.clear();
    imageIDs.clear();

    QByteArray ba = e->data("digikam/item-ids");

    if (ba.size())
    {
        QDataStream ds(ba);

        if (!ds.atEnd())
        {
            ds >> urls;
        }
    }

    if (!urls.isEmpty())
    {
        QByteArray albumarray = e->data("digikam/album-ids");
        QByteArray imagearray = e->data("digikam/image-ids-long");
        QByteArray kioarray   = e->data("digikam/digikamalbums");

        if (albumarray.size() && imagearray.size() && kioarray.size())
        {
            QDataStream dsAlbums(albumarray);

            if (!dsAlbums.atEnd())
            {
                dsAlbums >> albumIDs;
            }

            QDataStream dsImages(imagearray);

            if (!dsImages.atEnd())
            {
                dsImages >> imageIDs;
            }

            KUrl u;
            QDataStream dsKio(kioarray);

            if (!dsKio.atEnd())
            {
                dsKio >> kioUrls;
            }

            return true;
        }
    }

    return false;
}

// ------------------------------------------------------------------------

DAlbumDrag::DAlbumDrag(const KUrl& databaseUrl, int albumid, const KUrl& fileUrl)
    : QMimeData()
{
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << (KUrl::List() << databaseUrl);
    setData("digikam/digikamalbums", ba);

    QByteArray  ba2;
    QDataStream ds2(&ba2, QIODevice::WriteOnly);
    ds2 << (QList<int>() << albumid);
    setData("digikam/album-ids", ba2);

    // commonly accessible mime data, for dragging to outside digikam
    if (!fileUrl.isEmpty())
    {
        fileUrl.populateMimeData(this);
    }
}

QStringList DAlbumDrag::mimeTypes()
{
    return QStringList() << "digikam/album-ids"
           << "digikam/digikamalbums";
}

bool DAlbumDrag::canDecode(const QMimeData* e)
{
    if (e->hasFormat("digikam/item-ids") || e->hasFormat("digikam/image-ids-long"))
    {
        return false;
    }

    foreach(const QString& mimeType, mimeTypes())
    {
        if (!e->hasFormat(mimeType))
        {
            return false;
        }
    }
    return true;
}

bool DAlbumDrag::decode(const QMimeData* e, KUrl::List& kioUrls, int& albumID)
{
    KUrl url;
    albumID = -1;

    QByteArray albumarray = e->data("digikam/album-ids");
    QByteArray kioarray   = e->data("digikam/digikamalbums");

    if (albumarray.size() && kioarray.size())
    {
        QDataStream dsAlbums(albumarray);

        if (!dsAlbums.atEnd())
        {
            QList<int> ids;
            dsAlbums >> ids;
            albumID = ids.first();
        }

        KUrl u;
        QDataStream dsKio(kioarray);

        if (!dsKio.atEnd())
        {
            dsKio >> kioUrls;
        }

        return true;
    }

    return false;
}

// ------------------------------------------------------------------------

DTagListDrag::DTagListDrag(const QList<int>& tagIDs)
    : QMimeData()
{
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << tagIDs;
    setData("digikam/taglist", ba);
}

QStringList DTagListDrag::mimeTypes()
{
    return QStringList() << "digikam/taglist";
}

bool DTagListDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat("digikam/taglist");
}

bool DTagListDrag::decode(const QMimeData* e, QList<int>& tagIDs)
{
    tagIDs.clear();

    QByteArray ba = e->data("digikam/taglist");

    if (ba.size())
    {
        QDataStream ds(ba);

        if (!ds.atEnd())
        {
            ds >> tagIDs;
        }

        return true;
    }

    return false;
}

// ------------------------------------------------------------------------

DCameraItemListDrag::DCameraItemListDrag(const QStringList& cameraItemPaths)
    : QMimeData()
{
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << cameraItemPaths;
    setData("digikam/cameraItemlist", ba);
}

QStringList DCameraItemListDrag::mimeTypes()
{
    return QStringList() << "digikam/cameraItemlist";
}

bool DCameraItemListDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat("digikam/cameraItemlist");
}

bool DCameraItemListDrag::decode(const QMimeData* e, QStringList& cameraItemPaths)
{
    cameraItemPaths.clear();

    QByteArray ba = e->data("digikam/cameraItemlist");

    if (ba.size())
    {
        QDataStream ds(ba);

        if (!ds.atEnd())
        {
            ds >> cameraItemPaths;
        }

        return true;
    }

    return false;
}

// ------------------------------------------------------------------------

DCameraDragObject::DCameraDragObject(const CameraType& ctype)
    : QMimeData()
{
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << ctype.title();
    ds << ctype.model();
    ds << ctype.port();
    ds << ctype.path();
    ds << ctype.startingNumber();
    setData("camera/unknown", ba);
}

QStringList DCameraDragObject::mimeTypes()
{
    return QStringList() << "camera/unknown";
}

bool DCameraDragObject::canDecode(const QMimeData* e)
{
    return e->hasFormat("camera/unknown");
}

bool DCameraDragObject::decode(const QMimeData* e, CameraType& ctype)
{
    QByteArray ba = e->data("camera/unknown");

    if (ba.size())
    {
        int     startingNumber;
        QString title, model, port, path;

        QDataStream ds(ba);
        ds >> title;
        ds >> model;
        ds >> port;
        ds >> path;
        ds >> startingNumber;

        ctype = CameraType(title, model, port, path, startingNumber);

        return true;
    }
    else
    {
        return false;
    }
}

}  // namespace Digikam
