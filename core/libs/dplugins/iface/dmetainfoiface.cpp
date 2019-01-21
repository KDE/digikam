/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to item information for shared tools
 *               based on DMetadata.
 *
 * Copyright (C) 2017-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dmetainfoiface.h"

// Qt includes

#include <QFileInfo>
#include <QStandardPaths>

// Local includes

#include "dmetadata.h"
#include "photoinfocontainer.h"
#include "videoinfocontainer.h"
#include "template.h"
#include "dfileselector.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DMetaInfoIface::Private
{
public:

    explicit Private()
    {
        dirSelector = 0;
    }

    DFileSelector* dirSelector;
    QList<QUrl>    urls;
};

DMetaInfoIface::DMetaInfoIface(QObject* const parent, const QList<QUrl>& lst)
    : DInfoInterface(parent),
      d(new Private)
{
    d->urls = lst;
}

DMetaInfoIface::~DMetaInfoIface()
{
    delete d;
}

void DMetaInfoIface::slotDateTimeForUrl(const QUrl& url, const QDateTime& /*dt*/, bool /*updModDate*/)
{
    emit signalItemChanged(url);
}

void DMetaInfoIface::slotMetadataChangedForUrl(const QUrl& url)
{
    emit signalItemChanged(url);
}

QList<QUrl> DMetaInfoIface::currentAlbumItems() const
{
    return d->urls;
}

QList<QUrl> DMetaInfoIface::currentSelectedItems() const
{
    // No multiple items selection is available in DMeta.
    return currentAlbumItems();
}

QList<QUrl> DMetaInfoIface::allAlbumItems() const
{
    // No album management available in DMeta.
    return currentAlbumItems();
}

DMetaInfoIface::DInfoMap DMetaInfoIface::itemInfo(const QUrl& url) const
{
    DInfoMap map;

    if (d->urls.contains(url))
    {
        DMetadata meta(url.toLocalFile());
        QString   def = QLatin1String("x-default");
        QFileInfo info(url.toLocalFile());

        map.insert(QLatin1String("name"),        info.fileName());
        map.insert(QLatin1String("title"),       meta.getItemTitles()[def].caption);
        map.insert(QLatin1String("comment"),     meta.getItemComments()[def].caption);
        map.insert(QLatin1String("orientation"), (int)meta.getItemOrientation());
        map.insert(QLatin1String("datetime"),    meta.getItemDateTime());
        map.insert(QLatin1String("rating"),      meta.getItemRating());
        map.insert(QLatin1String("colorlabel"),  meta.getItemColorLabel());
        map.insert(QLatin1String("picklabel"),   meta.getItemPickLabel());
        map.insert(QLatin1String("filesize"),    (qlonglong)info.size());
        map.insert(QLatin1String("dimensions"),  meta.getItemDimensions());

        // Get digiKam Tags Path list of picture from database.
        // Ex.: "City/Paris/Monuments/Notre Dame"
        QStringList tagsPath;
        meta.getItemTagsPath(tagsPath);
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

        map.insert(QLatin1String("creators"),     temp.authors());
        map.insert(QLatin1String("credit"),       temp.credit());
        map.insert(QLatin1String("rights"),       temp.copyright()[def]);
        map.insert(QLatin1String("source"),       temp.source());

        PhotoInfoContainer photoInfo = meta.getPhotographInformation();
        map.insert(QLatin1String("make"),            photoInfo.make);
        map.insert(QLatin1String("model"),           photoInfo.model);
        map.insert(QLatin1String("exposuretime"),    photoInfo.exposureTime);
        map.insert(QLatin1String("sensitivity"),     photoInfo.sensitivity);
        map.insert(QLatin1String("aperture"),        photoInfo.aperture);
        map.insert(QLatin1String("focallength"),     photoInfo.focalLength);
        map.insert(QLatin1String("focalLength35mm"), photoInfo.focalLength35mm);

        // TODO: add more video metadata as needed
        VideoInfoContainer videoInfo = meta.getVideoInformation();
        map.insert(QLatin1String("videocodec"),   videoInfo.videoCodec);
    }

    return map;
}

void DMetaInfoIface::setItemInfo(const QUrl& url, const DInfoMap& map) const
{
    DMetadata meta(url.toLocalFile());
    QStringList keys = map.keys();

    if (map.contains(QLatin1String("rating")))
    {
        meta.setItemRating(map[QLatin1String("rating")].toInt());
        keys.removeAll(QLatin1String("rating"));
    }
    
    if  (map.contains(QLatin1String("colorlabel")))
    {
        meta.setItemColorLabel(map[QLatin1String("colorlabel")].toInt());
        keys.removeAll(QLatin1String("colorlabel"));
    }
    
    if  (map.contains(QLatin1String("picklabel")))
    {
        meta.setItemPickLabel(map[QLatin1String("picklabel")].toInt());
        keys.removeAll(QLatin1String("picklabel"));
    }
    
    if (!keys.isEmpty())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Keys not yet supported in DMetaInfoIface::setItemInfo():" << keys;
    }
}

bool DMetaInfoIface::supportAlbums() const
{
    return false;
}

QWidget* DMetaInfoIface::uploadWidget(QWidget* const parent) const
{
    if (!d->dirSelector)
    {
        d->dirSelector = new DFileSelector(parent);
        d->dirSelector->setFileDlgMode(QFileDialog::Directory);
        d->dirSelector->setFileDlgOptions(QFileDialog::ShowDirsOnly);
        d->dirSelector->setFileDlgTitle(i18n("Destination Folder"));
        d->dirSelector->lineEdit()->setPlaceholderText(i18n("Output Destination Path"));

        connect(d->dirSelector, SIGNAL(signalUrlSelected(QUrl)),
                this, SIGNAL(signalUploadUrlChanged()));
    }

    QFileInfo info(!d->urls.isEmpty() ? d->urls[0].toLocalFile() : QString());
    d->dirSelector->setFileDlgPath(info.absolutePath());

    return d->dirSelector;
}

QUrl DMetaInfoIface::uploadUrl() const
{
    return QUrl::fromLocalFile(d->dirSelector->fileDlgPath());
}

QUrl DMetaInfoIface::defaultUploadUrl() const
{
    QUrl place       = QUrl::fromLocalFile(QDir::homePath());
    QStringList pics = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    if (!pics.isEmpty())
        place = QUrl::fromLocalFile(pics.first());

    QList<QUrl> lst = currentAlbumItems();

    if (!lst.isEmpty())
    {
        QUrl trg = lst.first().adjusted(QUrl::RemoveFilename);

        if (!trg.isEmpty())
            place = trg;
    }

    return place;
}

} // namespace Digikam
