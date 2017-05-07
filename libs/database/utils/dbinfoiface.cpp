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
#include "albumselecttabs.h"

namespace Digikam
{

class DBInfoIface::Private
{
public:

    Private()
      : albumManager(AlbumManager::instance()),
        albumChooser(0)
    {
    }

    AlbumManager*    albumManager;
    AlbumSelectTabs* albumChooser;

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

    QList<QUrl> albumItems(Album* const album) const
    {
        QList<QUrl> imageList;

        if (album)
        {
            switch (album->type())
            {
                case Album::PHYSICAL:
                {
                    PAlbum* const p = dynamic_cast<PAlbum*>(album);

                    if (p)
                    {
                        imageList = imagesFromPAlbum(p);
                    }

                    break;
                }

                case Album::TAG:
                {
                    TAlbum* const p = dynamic_cast<TAlbum*>(album);

                    if (p)
                    {
                        imageList = imagesFromTAlbum(p);
                    }

                    break;
                }

                default:
                {
                    // Not supported.
                    break;
                }
            }
        }

        return imageList;
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
    QList<QUrl> imageList  = d->albumItems(currAlbum);

    if (imageList.isEmpty())
        imageList = DigikamApp::instance()->view()->allUrls();

    return imageList;
}

QList<QUrl> DBInfoIface::currentSelectedItems() const
{
    return DigikamApp::instance()->view()->selectedUrls();
}

QList<QUrl> DBInfoIface::allAlbumItems() const
{
    QList<QUrl> imageList;

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

DBInfoIface::DInfoMap DBInfoIface::albumInfo(int gid) const
{
    Album* const a = d->albumManager->findAlbum(gid);

    if (!a)
        return DInfoMap();

    DInfoMap map;
    map.insert(QLatin1String("title"), a->title());

    PAlbum* const p = dynamic_cast<PAlbum*>(a);

    if (p)
    {
        map.insert(QLatin1String("caption"),  p->caption());
        map.insert(QLatin1String("date"),     p->date());
    }

    return map;
}

DBInfoIface::DInfoMap DBInfoIface::itemInfo(const QUrl& url) const
{
    DInfoMap map;

    ImageInfo info = ImageInfo::fromUrl(url);

    if (!info.isNull())
    {
        map.insert(QLatin1String("name"),        info.name());
        map.insert(QLatin1String("title"),       info.title());
        map.insert(QLatin1String("comment"),     info.comment());
        map.insert(QLatin1String("orientation"), info.orientation());
        map.insert(QLatin1String("dateTime"),    info.dateTime());
    }

    return map;
}

QList<QUrl> DBInfoIface::albumsItems(const DAlbumIDs& lst) const
{
    QList<QUrl> imageList;

    foreach(int gid, lst)
    {
        Album* const a = d->albumManager->findAlbum(gid);

        if (a)
        {
            imageList << d->albumItems(a);
        }
    }

    return imageList;
}

QWidget* DBInfoIface::albumChooser(QWidget* const parent) const
{
    if (!d->albumChooser)
    {
        d->albumChooser = new AlbumSelectTabs(parent);
    }

    connect(d->albumChooser, SIGNAL(signalAlbumSelectionChanged()),
            this, SIGNAL(signalAlbumChooserSelectionChanged()));

    return d->albumChooser;
}

DBInfoIface::DAlbumIDs DBInfoIface::albumChooserItems() const
{
    if (!d->albumChooser)
    {
        return DAlbumIDs();
    }

    AlbumList lst = d->albumChooser->selectedAlbums();
    DAlbumIDs ids;

    foreach(Album* const a, lst)
    {
        if (a) ids << a->id();
    }

    return ids;
}

}  // namespace Digikam
