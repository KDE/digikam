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

    Private()
    {
        album = 0;
        type  = SelectedItems;
    }

    QList<QUrl> imagesFromPAlbum(PAlbum* const album) const;
    QList<QUrl> imagesFromTAlbum(TAlbum* const album) const;

public:

    QString                   imgFilter;
    KipiImageCollection::Type type;
    Album*                    album;
    QList<QUrl>               readyImageUrlList;
};

KipiImageCollection::KipiImageCollection(Type type, Album* const album, const QString& filter, QList<QUrl> imagesUrlList)
    : ImageCollectionShared(),
      d(new Private)
{
    d->type      = type;
    d->album     = album;
    d->imgFilter = filter;

    if(!imagesUrlList.isEmpty())
    {
        d->readyImageUrlList = imagesUrlList;
    }

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
            if (d->album->type() == Album::PHYSICAL)
            {
                PAlbum* const p = dynamic_cast<PAlbum*>(d->album);

                if (p)
                    return d->imagesFromPAlbum(p);
            }
            else if (d->album->type() == Album::TAG)
            {
                TAlbum* const p = dynamic_cast<TAlbum*>(d->album);

                if (p)
                    return d->imagesFromTAlbum(p);
            }
            else if (d->album->type() == Album::DATE ||
                     d->album->type() == Album::SEARCH)
            {
                return DigikamApp::instance()->view()->allUrls();
            }

            qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown album type";
            break;
        }

        case SelectedItems:
        {
            return DigikamApp::instance()->view()->selectedUrls();
        }

        default:
            break;
    }

    return QList<QUrl>();
}

/** get the images from the Physical album in database and return the items found.
 */
QList<QUrl> KipiImageCollection::Private::imagesFromPAlbum(PAlbum* const album) const
{
    // get the images from the database and return the items found

    CoreDB::ItemSortOrder sortOrder = CoreDB::NoItemSorting;

    switch (ApplicationSettings::instance()->getImageSortOrder())
    {
        default:
        case ImageSortSettings::SortByFileName:
            sortOrder = CoreDB::ByItemName;
            break;

        case ImageSortSettings::SortByFilePath:
            sortOrder = CoreDB::ByItemPath;
            break;

        case ImageSortSettings::SortByCreationDate:
            sortOrder = CoreDB::ByItemDate;
            break;

        case ImageSortSettings::SortByRating:
            sortOrder = CoreDB::ByItemRating;
            break;
            // ByISize not supported
    }

    QStringList list = CoreDbAccess().db()->getItemURLsInAlbum(album->id(), sortOrder);
    QList<QUrl> urlList;
    CoreDbNameFilter  nameFilter(imgFilter);

    for (QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        if (nameFilter.matches(*it))
        {
            urlList << QUrl::fromLocalFile(*it);
        }
    }

    return urlList;
}

/** get the images from the Tags album in database and return the items found.
 */
QList<QUrl> KipiImageCollection::Private::imagesFromTAlbum(TAlbum* const album) const
{
    QStringList list = CoreDbAccess().db()->getItemURLsInTag(album->id());
    QList<QUrl> urlList;
    CoreDbNameFilter  nameFilter(imgFilter);

    for (QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        if (nameFilter.matches(*it))
        {
            urlList << QUrl::fromLocalFile(*it);
        }
    }

    return urlList;
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
