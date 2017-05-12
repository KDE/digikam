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

#include "albumselecttabs.h"
#include "applicationsettings.h"
#include "album.h"
#include "albummanager.h"
#include "coredb.h"
#include "coredbnamefilter.h"
#include "dbinfoiface.h"
#include "digikamapp.h"
#include "digikam_debug.h"
#include "digikamview.h"
#include "imagecopyright.h"
#include "imageinfo.h"
#include "imageposition.h"
#include "imagesortsettings.h"

namespace Digikam
{

class DBInfoIface::Private
{
public:

    Private()
      : albumManager(AlbumManager::instance()),
        albumChooser(0),
        withGroupedIsSet(false),
        withGrouped(false)
    {
    }

    AlbumManager*                      albumManager;
    AlbumSelectTabs*                   albumChooser;

    QList<QUrl>                        itemUrls;

    ApplicationSettings::OperationType operationType;
    bool                               withGroupedIsSet;
    bool                               withGrouped;

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

    /** Remove grouped images if user chose/chooses to.
     */
    QList<QUrl> resolveGroupsFromAlbums(QList<QUrl> urlList)
    {
        if (!(withGroupedIsSet && withGrouped))
        {
            foreach(const QUrl& url, urlList)
            {
                ImageInfo info = ImageInfo::fromUrl(url);
                if (info.hasGroupedImages())
                {
                    if (!withGroupedIsSet)
                    {
                        withGroupedIsSet = true;
                        withGrouped = ApplicationSettings::instance()->askGroupingOperateOnAll(operationType);
                        if (withGrouped)
                        {
                            break;
                        }
                    }

                    foreach(const ImageInfo& i, info.groupedImages())
                    {
                        urlList.removeOne(i.fileUrl());
                    }
                }
            }
        }

        return urlList;
    }

    bool includeGroupedFromSelected()
    {
        if (withGroupedIsSet)
        {
            return withGrouped;
        }

        withGroupedIsSet = true;
        withGrouped = DigikamApp::instance()->view()->needGroupResolving(operationType, true);

        return withGrouped;
    }
};

DBInfoIface::DBInfoIface(QObject* const parent, const QList<QUrl>& lst,
                         const ApplicationSettings::OperationType type)
    : DInfoInterface(parent),
      d(new Private)
{
    d->itemUrls      = lst;
    d->operationType = type;
}

DBInfoIface::~DBInfoIface()
{
    delete d;
}

QList<QUrl> DBInfoIface::currentAlbumItems() const
{
    if (!d->itemUrls.isEmpty())
    {
        return d->itemUrls;
    }

    if (d->albumManager->currentAlbums().isEmpty())
    {
        return QList<QUrl>();
    }

    Album* const currAlbum = d->albumManager->currentAlbums().first();
    QList<QUrl> imageList  = d->resolveGroupsFromAlbums(albumItems(currAlbum));

    if (imageList.isEmpty())
        imageList = DigikamApp::instance()->view()->allUrls(d->includeGroupedFromSelected());

    return imageList;
}

QList<QUrl> DBInfoIface::currentSelectedItems() const
{
    if (!d->itemUrls.isEmpty())
    {
        return d->itemUrls;
    }

    return DigikamApp::instance()->view()->selectedUrls(d->includeGroupedFromSelected());
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
    {
        return DInfoMap();
    }

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
        map.insert(QLatin1String("rating"),      info.rating());
        map.insert(QLatin1String("colorlabel"),  info.colorLabel());
        map.insert(QLatin1String("picklabel"),   info.pickLabel());
        map.insert(QLatin1String("filesize"),    info.fileSize());

        // Get digiKam Tags Path list of picture from database.
        // Ex.: "City/Paris/Monuments/Notre Dame"
        QList<int> tagIds    = info.tagIds();
        QStringList tagspath = AlbumManager::instance()->tagPaths(tagIds, false);
        map.insert(QLatin1String("tagspath"),    tagspath);

        // Get digiKam Tags name (keywords) list of picture from database.
        // Ex.: "Notre Dame"
        QStringList tags     = AlbumManager::instance()->tagNames(tagIds);
        map.insert(QLatin1String("keywords"),    tags);

        // Get GPS location of picture from database.
        ImagePosition pos    = info.imagePosition();

        if (!pos.isEmpty())
        {
            map.insert(QLatin1String("latitude"),  pos.latitudeNumber());
            map.insert(QLatin1String("longitude"), pos.longitudeNumber());
            map.insert(QLatin1String("altitude"),  pos.altitude());
        }

        // Get Copyright information of picture from database.
        ImageCopyright rights = info.imageCopyright();
        map.insert(QLatin1String("creators"),    rights.creator());
        map.insert(QLatin1String("credit"),      rights.credit());
        map.insert(QLatin1String("rights"),      rights.rights());
        map.insert(QLatin1String("source"),      rights.source());
    }

    return map;
}

QList<QUrl> DBInfoIface::albumItems(Album* const album) const
{
    if (!album)
    {
        return QList<QUrl>();
    }

    QList<QUrl> imageList;

    switch (album->type())
    {
        case Album::PHYSICAL:
        {
            PAlbum* const p = dynamic_cast<PAlbum*>(album);

            if (p)
            {
                imageList = d->imagesFromPAlbum(p);
            }

            break;
        }

        case Album::TAG:
        {
            TAlbum* const p = dynamic_cast<TAlbum*>(album);

            if (p)
            {
                imageList = d->imagesFromTAlbum(p);
            }

            break;
        }

        default:
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown album type";
            break;
        }
    }

    return d->resolveGroupsFromAlbums(imageList);
}

QList<QUrl> DBInfoIface::albumItems(int id) const
{
    return albumItems(d->albumManager->findAlbum(id));
}

QList<QUrl> DBInfoIface::albumsItems(const DAlbumIDs& lst) const
{
    QList<QUrl> imageList;

    foreach(int gid, lst)
    {
        imageList << albumItems(gid);
    }

    return d->resolveGroupsFromAlbums(imageList);
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

bool DBInfoIface::supportAlbums() const
{
    return true;
}

}  // namespace Digikam
