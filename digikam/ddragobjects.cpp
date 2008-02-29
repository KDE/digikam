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

// Qt includes.

#include <QByteArray>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albummanager.h"
#include "ddragobjects.h"

namespace Digikam
{

DItemDrag::DItemDrag(const KUrl::List &urls,
                     const KUrl::List &kioUrls,
                     const QList<int>& albumIDs,
                     const QList<int>& imageIDs,
                     const char* name)
         : QMimeData()
{
    setObjectName(name);
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << urls;
    setData("digikam/item-ids", ba);

    QByteArray ba2;
    QDataStream ds2(&ba2, QIODevice::WriteOnly);
    ds2 << kioUrls;
    setData("digikam/digikamalbums", ba2);

    QByteArray ba3;
    QDataStream ds3(&ba3, QIODevice::WriteOnly);
    ds3 << albumIDs;
    setData("digikam/album-ids", ba3);

    QByteArray ba4;
    QDataStream ds4(&ba4, QIODevice::WriteOnly);
    ds4 << imageIDs;
    setData("digikam/image-ids", ba4);
}

bool DItemDrag::canDecode(const QMimeData* e)
{
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
    KUrl url;

    QByteArray ba = e->data("digikam/item-ids");
    if (ba.size())
    {
        QDataStream ds(&ba, QIODevice::ReadOnly);
        if(!ds.atEnd())
        {
            ds >> url;
            urls.append(url);
        }
    }

    if(!urls.isEmpty())
    {
        QByteArray albumarray = e->data("digikam/album-ids");
        QByteArray imagearray = e->data("digikam/image-ids");
        QByteArray kioarray   = e->data("digikam/digikamalbums");

        if (albumarray.size() && imagearray.size() && kioarray.size())
        {
            int id;

            QDataStream dsAlbums(&albumarray, QIODevice::ReadOnly);
            while (!dsAlbums.atEnd())
            {
                dsAlbums >> id;
                albumIDs.append(id);
            }

            QDataStream dsImages(&imagearray, QIODevice::ReadOnly);
            while (!dsImages.atEnd())
            {
                dsImages >> id;
                imageIDs.append(id);
            }

            KUrl u;
            QDataStream dsKio(&kioarray, QIODevice::ReadOnly);
            while (!dsKio.atEnd())
            {
                dsKio >> u;
                kioUrls.append(u);
            }

            return true;
        }
    }

    return false;
}

const char* DItemDrag::format(int i) const
{
    if (i == 0)
        return "text/uri-list";
    else if (i == 1)
        return "digikam/item-ids";
    else if (i == 2)
        return "digikam/album-ids";
    else if (i == 3)
        return "digikam/image-ids";
    else if (i == 4)
        return "digikam/digikamalbums";
    else
        return 0;
}

QByteArray DItemDrag::encodedData(const char* mime) const
{
    return data(mime);
}

// ------------------------------------------------------------------------

DTagDrag::DTagDrag(int albumid, const char *name)
        : QMimeData()
{
    setObjectName(name);
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << albumid;
    setData("digikam/tag-id", ba);
}

bool DTagDrag::canDecode(const QMimeData *e)
{
    return e->hasFormat("digikam/tag-id");
}

const char* DTagDrag::format(int i) const
{
    if (i == 0)
        return "digikam/tag-id";

    return 0;
}

QByteArray DTagDrag::encodedData(const char* mime) const
{
    return data(mime);
}

// ------------------------------------------------------------------------

DAlbumDrag::DAlbumDrag(const KUrl &url, int albumid, const char *name) 
          : QMimeData()
{
    setObjectName(name);
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << url;
    setData("text/uri-list", ba);

    QByteArray ba2;
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
        QDataStream ds(&ba, QIODevice::ReadOnly);
        if(!ds.atEnd())
        {
            ds >> url;
            urls.append(url);
        }
    }

    if(!urls.isEmpty())
    {
        QByteArray ba2 = e->data("digikam/album-id");
        if (ba2.size())
        {
            QDataStream ds2(&ba2, QIODevice::ReadOnly);
            if(!ds2.atEnd())
            {
                ds2 >> albumID;
            }
            return true;
        }
    }

    return false;
}

const char* DAlbumDrag::format(int i) const
{
    if (i == 0)
        return "text/uri-list";
    else if ( i == 1 )
        return "digikam/album-id";

    return 0;
}

QByteArray DAlbumDrag::encodedData(const char *mime) const
{
    return data(mime);
}

// ------------------------------------------------------------------------

DTagListDrag::DTagListDrag(const QList<int>& tagIDs, const char *name)
            : QMimeData()
{
    setObjectName(name);
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << tagIDs;
    setData("digikam/taglist", ba);
}

bool DTagListDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat("digikam/taglist");
}

QByteArray DTagListDrag::encodedData(const char* mime) const
{
    return data(mime);
}

const char* DTagListDrag::format(int i) const
{
    if (i == 0)
        return "digikam/taglist";

    return 0;
}

// ------------------------------------------------------------------------

DCameraItemListDrag::DCameraItemListDrag(const QStringList& cameraItemPaths, const char *name)
                   : QMimeData()
{
    setObjectName(name);
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << cameraItemPaths;
    setData("digikam/cameraItemlist", ba);
}

bool DCameraItemListDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat("digikam/cameraItemlist");
}

QByteArray DCameraItemListDrag::encodedData(const char* mime) const
{
    return data(mime);
}

const char* DCameraItemListDrag::format(int i) const
{
    if (i == 0)
        return "digikam/cameraItemlist";

    return 0;
}

}  // namespace Digikam
