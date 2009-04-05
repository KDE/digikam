/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-02-29
 * Description : Drag object info containers.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes

#include <kdebug.h>


namespace Digikam
{

DItemDrag::DItemDrag(const KUrl::List &urls,
                     const KUrl::List &kioUrls,
                     const QList<int>& albumIDs,
                     const QList<int>& imageIDs)
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
    setData("digikam/image-ids", ba4);

    // commonly accessible mime data, for dragging to outside digikam
    urls.populateMimeData(this);
}

bool DItemDrag::canDecode(const QMimeData* e)
{
    // we do not want decode text/uri-list,
    // we only export this data above for dragging to outside digikam.
    return e->hasFormat("digikam/item-ids")  ||
           e->hasFormat("digikam/album-ids") ||
           e->hasFormat("digikam/image-ids") ||
           e->hasFormat("digikam/digikamalbums");
}

bool DItemDrag::decode(const QMimeData* e,
                       KUrl::List &urls,
                       KUrl::List &kioUrls,
                       QList<int>& albumIDs,
                       QList<int>& imageIDs)
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

    if(!urls.isEmpty())
    {
        QByteArray albumarray = e->data("digikam/album-ids");
        QByteArray imagearray = e->data("digikam/image-ids");
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

DTagDrag::DTagDrag(int albumid)
        : QMimeData()
{
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << albumid;
    setData("digikam/tag-id", ba);
}

bool DTagDrag::canDecode(const QMimeData *e)
{
    return e->hasFormat("digikam/tag-id");
}

bool DTagDrag::decode(const QMimeData* e, int &tagID)
{
    tagID = 0;

    QByteArray ba = e->data("digikam/tag-id");
    if (ba.size())
    {
        QDataStream ds(ba);
        if (!ds.atEnd())
        {
            ds >> tagID;
        }

        return true;
    }

    return false;
}

// ------------------------------------------------------------------------

DAlbumDrag::DAlbumDrag(const KUrl &url, int albumid)
          : QMimeData()
{
    QByteArray  ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << url;
    setData("text/uri-list", ba);

    QByteArray  ba2;
    QDataStream ds2(&ba2, QIODevice::WriteOnly);
    ds2 << albumid;
    setData("digikam/album-id", ba2);
}

bool DAlbumDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat("digikam/album-id");
}

bool DAlbumDrag::decode(const QMimeData* e, KUrl::List &urls, int &albumID)
{
    KUrl url;
    urls.clear();
    albumID = -1;

    QByteArray ba = e->data("text/uri-list");
    if (ba.size())
    {
        QDataStream ds(ba);
        if(!ds.atEnd())
        {
            ds >> urls;
        }
    }

    if(!urls.isEmpty())
    {
        QByteArray ba2 = e->data("digikam/album-id");
        if (ba2.size())
        {
            QDataStream ds2(ba2);
            if(!ds2.atEnd())
            {
                ds2 >> albumID;
            }
            return true;
        }
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

bool DTagListDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat("digikam/taglist");
}

bool DTagListDrag::decode(const QMimeData* e, QList<int> &tagIDs)
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

bool DCameraItemListDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat("digikam/cameraItemlist");
}

bool DCameraItemListDrag::decode(const QMimeData* e, QStringList &cameraItemPaths)
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
    ds << ctype.lastAccess();
    setData("camera/unknown", ba);
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
        QString   title, model, port, path;
        QDateTime lastAccess;

        QDataStream ds(ba);
        ds >> title;
        ds >> model;
        ds >> port;
        ds >> path;
        ds >> lastAccess;

        ctype = CameraType(title, model, port, path, lastAccess);

        return true;
    }
    else
    {
        return false;
    }
}

}  // namespace Digikam
