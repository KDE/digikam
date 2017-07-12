/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to item informations for shared tools.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "showfotoinfoiface.h"

// Qt includes

#include <QFileInfo>

// Local includes

#include "dmetadata.h"
#include "infocontainer.h"
#include "template.h"

namespace ShowFoto
{

class ShowfotoInfoIface::Private
{
public:

    Private()
    {
    }

    QList<QUrl> urls;
};

ShowfotoInfoIface::ShowfotoInfoIface(QObject* const parent, const QList<QUrl>& lst)
    : DInfoInterface(parent),
      d(new Private)
{
    d->urls = lst;
}

ShowfotoInfoIface::~ShowfotoInfoIface()
{
    delete d;
}

QList<QUrl> ShowfotoInfoIface::currentAlbumItems() const
{
    return d->urls;
}

QList<QUrl> ShowfotoInfoIface::currentSelectedItems() const
{
    // No multiple items selection is avaialble in Showfoto.
    return currentAlbumItems();
}

QList<QUrl> ShowfotoInfoIface::allAlbumItems() const
{
    // No album management avaialble in Showfoto.
    return currentAlbumItems();
}

ShowfotoInfoIface::DInfoMap ShowfotoInfoIface::itemInfo(const QUrl& url) const
{
    DInfoMap map;

    if (d->urls.contains(url))
    {
        DMetadata meta(url.toLocalFile());
        QString   def = QLatin1String("x-default");
        QFileInfo info(url.toLocalFile());

        map.insert(QLatin1String("name"),        info.fileName());
        map.insert(QLatin1String("title"),       meta.getImageTitles()[def].caption);
        map.insert(QLatin1String("comment"),     meta.getImageComments()[def].caption);
        map.insert(QLatin1String("orientation"), (int)meta.getImageOrientation());
        map.insert(QLatin1String("datetime"),    meta.getImageDateTime());
        map.insert(QLatin1String("rating"),      meta.getImageRating());
        map.insert(QLatin1String("colorlabel"),  meta.getImageColorLabel());
        map.insert(QLatin1String("picklabel"),   meta.getImagePickLabel());
        map.insert(QLatin1String("filesize"),    (qlonglong)info.size());
        map.insert(QLatin1String("dimensions"),  meta.getImageDimensions());

        // Get digiKam Tags Path list of picture from database.
        // Ex.: "City/Paris/Monuments/Notre Dame"
        QStringList tagsPath;
        meta.getImageTagsPath(tagsPath);
        map.insert(QLatin1String("tagspath"),    tagsPath);

        // Get digiKam Tags name (keywords) list of picture from database.
        // Ex.: "Notre Dame"
        QStringList keywords = meta.getMetadataField(MetadataInfo::Keywords).toStringList();
        map.insert(QLatin1String("keywords"),    keywords);

        // Get GPS location of picture from database.
        double lat = 0.0;
        double lng = 0.0;
        double alt = 0.0;

        if (meta.getGPSInfo(lat, lng, alt))
        {
            map.insert(QLatin1String("latitude"),  lat);
            map.insert(QLatin1String("longitude"), lng);
            map.insert(QLatin1String("altitude"),  alt);
        }

        // Get Copyright information of picture from database.
        Template temp;
        meta.getCopyrightInformation(temp);

        map.insert(QLatin1String("creators"),    temp.authors());
        map.insert(QLatin1String("credit"),      temp.credit());
        map.insert(QLatin1String("rights"),      temp.copyright()[def]);
        map.insert(QLatin1String("source"),      temp.source());

        PhotoInfoContainer photoInfo = meta.getPhotographInformation();
        map.insert(QLatin1String("exposuretime"), photoInfo.exposureTime);
        map.insert(QLatin1String("sensitivity"),  photoInfo.sensitivity);
        map.insert(QLatin1String("aperture"),     photoInfo.aperture);
        map.insert(QLatin1String("focallength"),  photoInfo.focalLength);
    }

    return map;
}

bool ShowfotoInfoIface::supportAlbums() const
{
    return false;
}

}  // namespace ShowFoto
