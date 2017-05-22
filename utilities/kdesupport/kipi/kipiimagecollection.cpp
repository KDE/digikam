/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : class to get/set image collection
 *               information/properties using digiKam album database.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well dot com>
 * Copyright (C) 2004-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "kipiimagecollection.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "album.h"
#include "coredb.h"
#include "applicationsettings.h"
#include "collectionmanager.h"
#include "coredbaccess.h"
#include "dbinfoiface.h"
#include "digikamapp.h"
#include "digikamview.h"
#include "imagesortsettings.h"
#include "coredbnamefilter.h"
#include "digikam_debug.h"

namespace Digikam
{

class KipiImageCollection::Private
{
public:

    Private(Type type, Album* const album, const QString& filter, QList<QUrl> imagesUrlList) :
        type(type),
        album(album),
        imgFilter(filter),
        readyImageUrlList(imagesUrlList),
        iface(new DBInfoIface(0, QList<QUrl>(), ApplicationSettings::Kipi))
    {
    }

public:

    KipiImageCollection::Type type;
    Album*                    album;
    QString                   imgFilter;
    QList<QUrl>               readyImageUrlList;
    DBInfoIface*              iface;
};

KipiImageCollection::KipiImageCollection(Type type, Album* const album, const QString& filter, QList<QUrl> imagesUrlList)
    : ImageCollectionShared(),
      d(new Private(type, album, filter, imagesUrlList))
{
    if (!album)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "This should not happen. No album specified";
    }
}

KipiImageCollection::~KipiImageCollection()
{
    delete d;
}

QString KipiImageCollection::name()
{
    if (d->album->type() == Album::TAG)
    {
        return i18n("Tag: %1", d->album->title());
    }
    else if ((d->album->type() == Album::SEARCH) && dynamic_cast<SAlbum*>(d->album)->isDuplicatesSearch())
    {
        ImageInfo imageInfo(d->album->title().toLongLong());
        return i18n("Duplicates of %1", imageInfo.name());
    }
    else
    {
        return d->album->title();
    }
}

QString KipiImageCollection::category()
{
    if (d->album->type() == Album::PHYSICAL)
    {
        PAlbum* const p = dynamic_cast<PAlbum*>(d->album);

        if (p)
            return p->category();
    }
    else if (d->album->type() == Album::TAG)
    {
        TAlbum* const p = dynamic_cast<TAlbum*>(d->album);

        if (p)
            return i18n("Tag: %1", p->tagPath());
    }

    return QString();
}

QDate KipiImageCollection::date()
{
    if (d->album->type() == Album::PHYSICAL)
    {
        PAlbum* const p = dynamic_cast<PAlbum*>(d->album);

        if (p)
            return p->date();
    }

    return QDate();
}

QString KipiImageCollection::comment()
{
    if (d->album->type() == Album::PHYSICAL)
    {
        PAlbum* const p = dynamic_cast<PAlbum*>(d->album);

        if (p)
            return p->caption();
    }

    return QString();
}

QList<QUrl> KipiImageCollection::images()
{
    if(!d->readyImageUrlList.isEmpty())
    {
        return d->readyImageUrlList;
    }

    switch (d->type)
    {
        case AllItems:
        {
            return d->iface->albumItems(d->album);
        }

        case SelectedItems:
        {
            return d->iface->currentSelectedItems();
        }
    }

    return QList<QUrl>();
}

QUrl KipiImageCollection::url()
{
    if (d->album->type() == Album::PHYSICAL)
    {
        PAlbum* const p = dynamic_cast<PAlbum*>(d->album);

        if (p)
        {
            return QUrl::fromLocalFile(p->folderPath());
        }
    }

    qCWarning(DIGIKAM_GENERAL_LOG) << "Requesting QUrl from a virtual album";
    return QUrl();
}

QUrl KipiImageCollection::uploadUrl()
{
    if (d->album->type() == Album::PHYSICAL)
    {
        PAlbum* const p = dynamic_cast<PAlbum*>(d->album);

        if (p)
        {
            return QUrl::fromLocalFile(p->folderPath());
        }
    }

    qCWarning(DIGIKAM_GENERAL_LOG) << "Requesting QUrl from a virtual album";
    return QUrl();
}

QUrl KipiImageCollection::uploadRoot()
{
    return QUrl::fromLocalFile(CollectionManager::instance()->oneAlbumRootPath() + QLatin1Char('/'));
}

QString KipiImageCollection::uploadRootName()
{
    return i18n("Albums");
}

bool KipiImageCollection::isDirectory()
{
    if (d->album->type() == Album::PHYSICAL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool KipiImageCollection::operator==(ImageCollectionShared& imgCollection)
{
    KipiImageCollection* const thatCollection = static_cast<KipiImageCollection*>(&imgCollection);
    return (d->album == thatCollection->d->album);
}

}  // namespace Digikam
