/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to database informations for shared tools.
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

// Local includes

#include "dbinfoiface.h"
#include "album.h"
#include "albummanager.h"
#include "imageinfo.h"
#include "imagesortsettings.h"
#include "coredbnamefilter.h"
#include "coredb.h"
#include "applicationsettings.h"
#include "digikamapp.h"
#include "digikamview.h"

namespace Digikam
{

class DBInfoIface::Private
{
public:

    Private()
      : albumManager(0)
    {
    }

    AlbumManager* albumManager;

public:

    /** get the images from the Physical album in database and return the items found.
     */
    QList<QUrl> imagesFromPAlbum(PAlbum* const album) const
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
        CoreDbNameFilter nameFilter(ApplicationSettings::instance()->getAllFileFilter());

        for (QStringList::const_iterator it = list.constBegin() ; it != list.constEnd() ; ++it)
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
    QList<QUrl> imagesFromTAlbum(TAlbum* const album) const
    {
        QStringList list = CoreDbAccess().db()->getItemURLsInTag(album->id());
        QList<QUrl> urlList;
        CoreDbNameFilter nameFilter(ApplicationSettings::instance()->getAllFileFilter());

        for (QStringList::const_iterator it = list.constBegin() ; it != list.constEnd() ; ++it)
        {
            if (nameFilter.matches(*it))
            {
                urlList << QUrl::fromLocalFile(*it);
            }
        }

        return urlList;
    }
};

DBInfoIface::DBInfoIface(QObject* const parent)
    : DInfoInterface(parent),
      d(new Private)
{
}

DBInfoIface::~DBInfoIface()
{
    delete d;
}

QList<QUrl> DBInfoIface::currentAlbumItems() const
{
    if (d->albumManager->currentAlbums().isEmpty())
    {
        return QList<QUrl>();
    }

    Album* const currAlbum = d->albumManager->currentAlbums().first();

    if (currAlbum)
    {
        QList<QUrl> imageList;

        switch (currAlbum->type())
        {
            case Album::PHYSICAL:
            {
                PAlbum* const p = dynamic_cast<PAlbum*>(currAlbum);

                if (p)
                {
                    imageList = d->imagesFromPAlbum(p);
                }

                break;
            }

            case Album::TAG:
            {
                TAlbum* const p = dynamic_cast<TAlbum*>(currAlbum);

                if (p)
                {
                    imageList = d->imagesFromTAlbum(p);
                }

                break;
            }

            default:
            {
                imageList = DigikamApp::instance()->view()->allUrls();
                break;
            }
        }
    }

    return QList<QUrl>();
}

QList<QUrl> DBInfoIface::currentSelectedItems() const
{
    return DigikamApp::instance()->view()->selectedUrls();
}

QList<QUrl> DBInfoIface::allAlbumItems() const
{
    QList<QUrl> imageList;
    QString fileFilter(ApplicationSettings::instance()->getAllFileFilter());

    const AlbumList palbumList = d->albumManager->allPAlbums();

    for (AlbumList::ConstIterator it = palbumList.constBegin();
         it != palbumList.constEnd(); ++it)
    {
        // don't add the root album
        if ((*it)->isRoot())
        {
            continue;
        }

        PAlbum* const p = dynamic_cast<PAlbum*>(*it);

        if (p)
        {
            imageList.append(d->imagesFromPAlbum(p));
        }
    }

    return imageList;
}

DBInfoIface::DInfoMap DBInfoIface::albumInfo(const QUrl&) const
{
    return DInfoMap();
}

DBInfoIface::DInfoMap DBInfoIface::itemInfo(const QUrl&) const
{
    return DInfoMap();
}

}  // namespace Digikam
