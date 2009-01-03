/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : class to get/set image collection
 *               information/properties using digiKam album database.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes.

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

// LibKIPI includes.

#include <libkipi/version.h>

// Local includes.

#include "constants.h"
#include "albumitemhandler.h"
#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "collectionmanager.h"
#include "databaseaccess.h"
#include "namefilter.h"

namespace Digikam
{

KipiImageCollection::KipiImageCollection(Type tp, Album *album, const QString& filter)
{
    m_tp        = tp;
    m_album     = album;
    m_imgFilter = filter;

    if (!album)
    {
        kWarning(50003) << "This should not happen. No album specified" << endl;
    }
}

KipiImageCollection::~KipiImageCollection()
{
}

QString KipiImageCollection::name()
{
    if ( m_album->type() == Album::TAG )
    {
        return i18n("Tag: %1", m_album->title());
    }
    else
        return m_album->title();
}

QString KipiImageCollection::category()
{
    if ( m_album->type() == Album::PHYSICAL )
    {
        PAlbum *p = dynamic_cast<PAlbum*>(m_album);
        return p->category();
    }
    else if ( m_album->type() == Album::TAG )
    {
        TAlbum *p = dynamic_cast<TAlbum*>(m_album);
        return i18n("Tag: %1", p->tagPath());
    }
    else
        return QString();
}

QDate KipiImageCollection::date()
{
    if ( m_album->type() == Album::PHYSICAL )
    {
        PAlbum *p = dynamic_cast<PAlbum*>(m_album);
        return p->date();
    }
    else
        return QDate();
}

QString KipiImageCollection::comment()
{
    if ( m_album->type() == Album::PHYSICAL )
    {
        PAlbum *p = dynamic_cast<PAlbum*>(m_album);
        return p->caption();
    }
    else
        return QString();
}

KUrl::List KipiImageCollection::images()
{
    switch ( m_tp )
    {
        case AllItems:
        {
            if (m_album->type() == Album::PHYSICAL)
            {
                return imagesFromPAlbum(dynamic_cast<PAlbum*>(m_album));
            }
            else if (m_album->type() == Album::TAG)
            {
                return imagesFromTAlbum(dynamic_cast<TAlbum*>(m_album));
            }
            else if (m_album->type() == Album::DATE ||
                    m_album->type() == Album::SEARCH)
            {
                AlbumItemHandler* handler = AlbumManager::instance()->getItemHandler();

                if (handler)
                {
                    return handler->allItems();
                }

                return KUrl::List();
        }
            else
            {
                kWarning(50003) << "Unknown album type" << endl;
                return KUrl::List();
            }

            break;
        }
        case SelectedItems:
        {
            AlbumItemHandler* handler = AlbumManager::instance()->getItemHandler();

            if (handler)
            {
                return handler->selectedItems();
            }

            return KUrl::List();

            break;
        }
        default:
            break;
    }

    // We should never reach here
    return KUrl::List();
}

/** get the images from the Physical album in database and return the items found */

KUrl::List KipiImageCollection::imagesFromPAlbum(PAlbum* album) const
{
    // get the images from the database and return the items found

    AlbumDB::ItemSortOrder sortOrder = AlbumDB::NoItemSorting;
    switch (AlbumSettings::instance()->getImageSortOrder())
    {
        default:
        case AlbumSettings::ByIName:
            sortOrder = AlbumDB::ByItemName;
            break;
        case AlbumSettings::ByIPath:
            sortOrder = AlbumDB::ByItemPath;
            break;
        case AlbumSettings::ByIDate:
            sortOrder = AlbumDB::ByItemDate;
            break;
        case AlbumSettings::ByIRating:
            sortOrder = AlbumDB::ByItemRating;
            break;
        // ByISize not supported
    }

    QStringList urls = DatabaseAccess().db()->getItemURLsInAlbum(album->id(), sortOrder);

    KUrl::List urlList;

    NameFilter nameFilter(m_imgFilter);

    for (QStringList::iterator it = urls.begin(); it != urls.end(); ++it)
    {
        if (nameFilter.matches(*it))
            urlList.append(*it);
    }

    return urlList;
}

/** get the images from the Tags album in database and return the items found */

KUrl::List KipiImageCollection::imagesFromTAlbum(TAlbum* album) const
{
    QStringList urls;
    urls = DatabaseAccess().db()->getItemURLsInTag(album->id());

    KUrl::List urlList;

    NameFilter nameFilter(m_imgFilter);

    for (QStringList::iterator it = urls.begin(); it != urls.end(); ++it)
    {
        if (nameFilter.matches(*it))
            urlList.append(*it);
    }

    return urlList;
}

KUrl KipiImageCollection::path()
{
    if (m_album->type() == Album::PHYSICAL)
    {
        PAlbum *p = dynamic_cast<PAlbum*>(m_album);
        KUrl url;
        url.setPath(p->folderPath());
        return url;
    }
    else
    {
        kWarning(50003) << "Requesting KUrl from a virtual album" << endl;
        return QString();
    }
}

KUrl KipiImageCollection::uploadPath()
{
    if (m_album->type() == Album::PHYSICAL)
    {
        PAlbum *p = dynamic_cast<PAlbum*>(m_album);
        KUrl url;
        url.setPath(p->folderPath());
        return url;
    }
    else
    {
        kWarning(50003) << "Requesting KUrl from a virtual album" << endl;
        return KUrl();
    }
}

KUrl KipiImageCollection::uploadRoot()
{
    return KUrl(CollectionManager::instance()->oneAlbumRootPath() + '/');
}

QString KipiImageCollection::uploadRootName()
{
    return i18n("My Albums");
}

bool KipiImageCollection::isDirectory()
{
    if (m_album->type() == Album::PHYSICAL)
        return true;
    else
        return false;
}

bool KipiImageCollection::operator==(ImageCollectionShared& imgCollection)
{
    KipiImageCollection* thatCollection = static_cast<KipiImageCollection*>(&imgCollection);
    return (m_album == thatCollection->m_album);
}

}  // namespace Digikam
