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
 * Copyright (C) 2004-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "applicationsettings.h"
#include "collectionmanager.h"
#include "databaseaccess.h"
#include "digikamapp.h"
#include "digikamview.h"
#include "imagesortsettings.h"
#include "namefilter.h"

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

    KUrl::List imagesFromPAlbum(PAlbum* const album) const;
    KUrl::List imagesFromTAlbum(TAlbum* const album) const;

public:

    QString                   imgFilter;
    KipiImageCollection::Type type;
    Album*                    album;
    KUrl::List                readyImageUrlList;
};

KipiImageCollection::KipiImageCollection(Type type, Album* const album, const QString& filter, KUrl::List imagesUrlList)
    : ImageCollectionShared(), d(new Private)
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
        kWarning() << "This should not happen. No album specified";
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

KUrl::List KipiImageCollection::images()
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

            kWarning() << "Unknown album type";
            break;
        }

        case SelectedItems:
        {
            return DigikamApp::instance()->view()->selectedUrls();
        }

        default:
            break;
    }

    return KUrl::List();
}

/** get the images from the Physical album in database and return the items found.
 */
KUrl::List KipiImageCollection::Private::imagesFromPAlbum(PAlbum* const album) const
{
    // get the images from the database and return the items found

    AlbumDB::ItemSortOrder sortOrder = AlbumDB::NoItemSorting;

    switch (ApplicationSettings::instance()->getImageSortOrder())
    {
        default:
        case ImageSortSettings::SortByFileName:
            sortOrder = AlbumDB::ByItemName;
            break;

        case ImageSortSettings::SortByFilePath:
            sortOrder = AlbumDB::ByItemPath;
            break;

        case ImageSortSettings::SortByCreationDate:
            sortOrder = AlbumDB::ByItemDate;
            break;

        case ImageSortSettings::SortByRating:
            sortOrder = AlbumDB::ByItemRating;
            break;
            // ByISize not supported
    }

    QStringList urls = DatabaseAccess().db()->getItemURLsInAlbum(album->id(), sortOrder);
    KUrl::List  urlList;
    NameFilter  nameFilter(imgFilter);

    for (QStringList::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        if (nameFilter.matches(*it))
        {
            urlList.append(*it);
        }
    }

    return urlList;
}

/** get the images from the Tags album in database and return the items found.
 */
KUrl::List KipiImageCollection::Private::imagesFromTAlbum(TAlbum* const album) const
{
    QStringList urls = DatabaseAccess().db()->getItemURLsInTag(album->id());
    KUrl::List  urlList;
    NameFilter  nameFilter(imgFilter);

    for (QStringList::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        if (nameFilter.matches(*it))
        {
            urlList.append(*it);
        }
    }

    return urlList;
}

KUrl KipiImageCollection::path()
{
    if (d->album->type() == Album::PHYSICAL)
    {
        PAlbum* const p = dynamic_cast<PAlbum*>(d->album);

        if (p)
        {
            KUrl url;
            url.setPath(p->folderPath());
            return url;
        }
    }

    kWarning() << "Requesting KUrl from a virtual album";
    return KUrl();
}

KUrl KipiImageCollection::uploadPath()
{
    if (d->album->type() == Album::PHYSICAL)
    {
        PAlbum* const p = dynamic_cast<PAlbum*>(d->album);

        if (p)
        {
            KUrl url;
            url.setPath(p->folderPath());
            return url;
        }
    }

    kWarning() << "Requesting KUrl from a virtual album";
    return KUrl();
}

KUrl KipiImageCollection::uploadRoot()
{
    return KUrl(CollectionManager::instance()->oneAlbumRootPath() + '/');
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
