/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-02-29
 * Description : Drag object info containers.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QDataStream>

namespace Digikam
{

DItemDrag::DItemDrag(const QList<QUrl>& urls,
                     const QList<QUrl>& kioUrls,
                     const QList<int>& albumIDs,
                     const QList<qlonglong>& imageIDs)
    : QMimeData()
{
    // Digikam specific mime data
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << urls;
    setData(QLatin1String("digikam/item-ids"), ba);

    QByteArray  ba2;
    QDataStream ds2(&ba2, QIODevice::WriteOnly);
    ds2 << kioUrls;
    setData(QLatin1String("digikam/digikamalbums"), ba2);

    QByteArray  ba3;
    QDataStream ds3(&ba3, QIODevice::WriteOnly);
    ds3 << albumIDs;
    setData(QLatin1String("digikam/album-ids"), ba3);

    QByteArray  ba4;
    QDataStream ds4(&ba4, QIODevice::WriteOnly);
    ds4 << imageIDs;
    setData(QLatin1String("digikam/image-ids-long"), ba4);

    // commonly accessible mime data, for dragging to outside digikam
    setUrls(urls);
}

QStringList DItemDrag::mimeTypes()
{
    // we do not want to decode text/uri-list with this object,
    // we only export this data above for dragging to outside digikam.
    return QStringList() << QLatin1String("digikam/item-ids")
                         << QLatin1String("digikam/album-ids")
                         << QLatin1String("digikam/image-ids-long")
                         << QLatin1String("digikam/digikamalbums");
}

bool DItemDrag::canDecode(const QMimeData* e)
{
    foreach (const QString& mimeType, mimeTypes())
    {
        if (!e->hasFormat(mimeType))
        {
            return false;
        }
    }

    return true;
}

bool DItemDrag::decode(const QMimeData* e,
                       QList<QUrl>& urls,
                       QList<QUrl>& kioUrls,
                       QList<int>& albumIDs,
                       QList<qlonglong>& imageIDs)
{
    urls.clear();
    kioUrls.clear();
    albumIDs.clear();
    imageIDs.clear();

    QByteArray ba = e->data(QLatin1String("digikam/item-ids"));

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
        QByteArray albumarray = e->data(QLatin1String("digikam/album-ids"));
        QByteArray imagearray = e->data(QLatin1String("digikam/image-ids-long"));
        QByteArray kioarray   = e->data(QLatin1String("digikam/digikamalbums"));

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

            QUrl u;
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

DAlbumDrag::DAlbumDrag(const QUrl& databaseUrl, int albumid, const QUrl& fileUrl)
    : QMimeData()
{
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << (QList<QUrl>() << databaseUrl);
    setData(QLatin1String("digikam/digikamalbums"), ba);

    QByteArray  ba2;
    QDataStream ds2(&ba2, QIODevice::WriteOnly);
    ds2 << (QList<int>() << albumid);
    setData(QLatin1String("digikam/album-ids"), ba2);

    // commonly accessible mime data, for dragging to outside digikam
    if (!fileUrl.isEmpty())
    {
        setUrls(QList<QUrl>() << fileUrl);
    }
}

QStringList DAlbumDrag::mimeTypes()
{
    return QStringList() << QLatin1String("digikam/album-ids")
                         << QLatin1String("digikam/digikamalbums");
}

bool DAlbumDrag::canDecode(const QMimeData* e)
{
    if (e->hasFormat(QLatin1String("digikam/item-ids")) || e->hasFormat(QLatin1String("digikam/image-ids-long")))
    {
        return false;
    }

    foreach (const QString& mimeType, mimeTypes())
    {
        if (!e->hasFormat(mimeType))
        {
            return false;
        }
    }

    return true;
}

bool DAlbumDrag::decode(const QMimeData* e, QList<QUrl>& kioUrls, int& albumID)
{
    albumID = -1;

    QByteArray albumarray = e->data(QLatin1String("digikam/album-ids"));
    QByteArray kioarray   = e->data(QLatin1String("digikam/digikamalbums"));

    if (albumarray.size() && kioarray.size())
    {
        QDataStream dsAlbums(albumarray);

        if (!dsAlbums.atEnd())
        {
            QList<int> ids;
            dsAlbums >> ids;
            albumID = ids.first();
        }

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
    setData(QLatin1String("digikam/taglist"), ba);
}

QStringList DTagListDrag::mimeTypes()
{
    return QStringList() << QLatin1String("digikam/taglist");
}

bool DTagListDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat(QLatin1String("digikam/taglist"));
}

bool DTagListDrag::decode(const QMimeData* e, QList<int>& tagIDs)
{
    tagIDs.clear();

    QByteArray ba = e->data(QLatin1String("digikam/taglist"));

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
    setData(QLatin1String("digikam/cameraItemlist"), ba);
}

QStringList DCameraItemListDrag::mimeTypes()
{
    return QStringList() << QLatin1String("digikam/cameraItemlist");
}

bool DCameraItemListDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat(QLatin1String("digikam/cameraItemlist"));
}

bool DCameraItemListDrag::decode(const QMimeData* e, QStringList& cameraItemPaths)
{
    cameraItemPaths.clear();

    QByteArray ba = e->data(QLatin1String("digikam/cameraItemlist"));

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
    setData(QLatin1String("camera/unknown"), ba);
}

QStringList DCameraDragObject::mimeTypes()
{
    return QStringList() << QLatin1String("camera/unknown");
}

bool DCameraDragObject::canDecode(const QMimeData* e)
{
    return e->hasFormat(QLatin1String("camera/unknown"));
}

bool DCameraDragObject::decode(const QMimeData* e, CameraType& ctype)
{
    QByteArray ba = e->data(QLatin1String("camera/unknown"));

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
