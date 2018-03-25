/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-14
 * Description : Load and cache tag thumbnails
 *
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "albumthumbnailloader.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QList>
#include <QMap>
#include <QCache>
#include <QPair>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "imageinfo.h"
#include "metadatasettings.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"

namespace Digikam
{

typedef QMap<qlonglong, QList<int> > IdAlbumMap;
typedef QMap<int, QPixmap>           AlbumThumbnailMap;

class AlbumThumbnailLoaderCreator
{
public:

    AlbumThumbnailLoader object;
};

Q_GLOBAL_STATIC(AlbumThumbnailLoaderCreator, creator)

// ---------------------------------------------------------------------------------------------

class AlbumThumbnailLoader::Private
{
public:

    Private()
    {
        iconSize             = ApplicationSettings::instance()->getTreeViewIconSize();
        minBlendSize         = 20;
        iconAlbumThumbThread = 0;
        iconTagThumbThread   = 0;
    }

    int                                  iconSize;
    int                                  minBlendSize;

    ThumbnailLoadThread*                 iconTagThumbThread;
    ThumbnailLoadThread*                 iconAlbumThumbThread;

    IdAlbumMap                           idAlbumMap;

    AlbumThumbnailMap                    thumbnailMap;

    QCache<QPair<QString, int>, QPixmap> iconCache;
};

bool operator<(const ThumbnailIdentifier& a, const ThumbnailIdentifier& b)
{
    if (a.id || b.id)
    {
        return a.id < b.id;
    }
    else
    {
        return a.filePath < b.filePath;
    }
}

AlbumThumbnailLoader* AlbumThumbnailLoader::instance()
{
    return &creator->object;
}

AlbumThumbnailLoader::AlbumThumbnailLoader()
    : d(new Private)
{
    connect(this, SIGNAL(signalDispatchThumbnailInternal(int,QPixmap)),
            this, SLOT(slotDispatchThumbnailInternal(int,QPixmap)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotIconChanged(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotIconChanged(Album*)));
}

AlbumThumbnailLoader::~AlbumThumbnailLoader()
{
    delete d->iconTagThumbThread;
    delete d->iconAlbumThumbThread;
    delete d;
}

void AlbumThumbnailLoader::cleanUp()
{
    delete d->iconTagThumbThread;
    d->iconTagThumbThread   = 0;

    delete d->iconAlbumThumbThread;
    d->iconAlbumThumbThread = 0;
}

QPixmap AlbumThumbnailLoader::getStandardTagIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("tag"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardTagRootIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("document-open"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardTagIcon(TAlbum* const album, RelativeSize relativeSize)
{
    if (album->isRoot())
    {
        return getStandardTagRootIcon(relativeSize);
    }
    else
    {
        return getStandardTagIcon(relativeSize);
    }
}

QPixmap AlbumThumbnailLoader::getNewTagIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("tag-new"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardAlbumIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("folder"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardAlbumTrashIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("user-trash"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardAlbumRootIcon(RelativeSize relativeSize)
{
    return loadIcon(QLatin1String("folder-pictures"), computeIconSize(relativeSize));
}

QPixmap AlbumThumbnailLoader::getStandardAlbumIcon(PAlbum* const album, RelativeSize relativeSize)
{
    if (album->isRoot() || album->isAlbumRoot())
    {
        return getStandardAlbumRootIcon(relativeSize);
    }
    else if (album->isTrashAlbum())
    {
        return getStandardAlbumTrashIcon();
    }
    else
    {
        return getStandardAlbumIcon(relativeSize);
    }
}

int AlbumThumbnailLoader::computeIconSize(RelativeSize relativeSize) const
{
    if (relativeSize == SmallerSize)
    {
        // when size was 32 smaller was 20. Scale.
        return lround(20.0 / 32.0 * (double)d->iconSize);
    }

    return d->iconSize;
}

QPixmap AlbumThumbnailLoader::loadIcon(const QString& name, int size) const
{
    QPixmap* pix = d->iconCache[qMakePair(name, size)];

    if (!pix)
    {
        d->iconCache.insert(qMakePair(name, size), new QPixmap(QIcon::fromTheme(name).pixmap(size)));
        pix = d->iconCache[qMakePair(name, size)];
    }

    return (*pix); // ownership of the pointer is kept by the icon cache.
}

bool AlbumThumbnailLoader::getTagThumbnail(TAlbum* const album, QPixmap& icon)
{
    if (album->iconId() && d->iconSize > d->minBlendSize)
    {
        addUrl(album, album->iconId());
        icon = QPixmap();
        return true;
    }
    else if (!album->icon().isEmpty())
    {
        icon = loadIcon(album->icon(), d->iconSize);
        return false;
    }

    icon = QPixmap();
    return false;
}

QPixmap AlbumThumbnailLoader::getTagThumbnailDirectly(TAlbum* const album)
{
    if (album->iconId() && d->iconSize > d->minBlendSize)
    {
        // icon cached?
        AlbumThumbnailMap::const_iterator it = d->thumbnailMap.constFind(album->globalID());

        if (it != d->thumbnailMap.constEnd())
        {
            return *it;
        }

        addUrl(album, album->iconId());
    }
    else if (!album->icon().isEmpty())
    {
        QPixmap pixmap = loadIcon(album->icon(), d->iconSize);
        return pixmap;
    }

    return getStandardTagIcon(album);
}

bool AlbumThumbnailLoader::getAlbumThumbnail(PAlbum* const album)
{
    if (album->iconId() && d->iconSize > d->minBlendSize)
    {
        addUrl(album, album->iconId());
    }
    else
    {
        return false;
    }

    return true;
}

QPixmap AlbumThumbnailLoader::getAlbumThumbnailDirectly(PAlbum* const album)
{
    if (album->iconId() && d->iconSize > d->minBlendSize)
    {
        // icon cached?
        AlbumThumbnailMap::const_iterator it = d->thumbnailMap.constFind(album->globalID());

        if (it != d->thumbnailMap.constEnd())
        {
            return *it;
        }

        // schedule for loading
        addUrl(album, album->iconId());
    }

    return getStandardAlbumIcon(album);
}

void AlbumThumbnailLoader::addUrl(Album* const album, qlonglong id)
{
    // First check cached thumbnails.
    // We use a private cache which is actually a map to be sure to cache _all_ album thumbnails.
    // At startup, this is not relevant, as the views will add their requests in a row.
    // This is to speed up context menu and IE imagedescedit
    AlbumThumbnailMap::const_iterator ttit = d->thumbnailMap.constFind(album->globalID());

    if (ttit != d->thumbnailMap.constEnd())
    {
        // It is not necessary to return cached icon asynchronously - they could be
        // returned by getTagThumbnail already - but this would make the API
        // less elegant, it feels much better this way.
        emit signalDispatchThumbnailInternal(album->globalID(), *ttit);
        return;
    }

    // Check if the URL has already been added
    IdAlbumMap::iterator it = d->idAlbumMap.find(id);

    if (it == d->idAlbumMap.end())
    {
        // use two threads so that tag and album thumbnails are loaded
        // in parallel and not first album, then tag thumbnails
        if (album->type() == Album::TAG)
        {
            if (!d->iconTagThumbThread)
            {
                d->iconTagThumbThread = new ThumbnailLoadThread();
                d->iconTagThumbThread->setThumbnailSize(d->iconSize);
                d->iconTagThumbThread->setSendSurrogatePixmap(false);

                connect(d->iconTagThumbThread,
                        SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
                        SLOT(slotGotThumbnailFromIcon(LoadingDescription,QPixmap)),
                        Qt::QueuedConnection);
            }

            // use the asynchronous version - with queued connections, see above
            d->iconTagThumbThread->find(ImageInfo::thumbnailIdentifier(id));
        }
        else
        {
            if (!d->iconAlbumThumbThread)
            {
                d->iconAlbumThumbThread = new ThumbnailLoadThread();
                d->iconAlbumThumbThread->setThumbnailSize(d->iconSize);
                d->iconAlbumThumbThread->setSendSurrogatePixmap(false);

                connect(d->iconAlbumThumbThread,
                        SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
                        SLOT(slotGotThumbnailFromIcon(LoadingDescription,QPixmap)),
                        Qt::QueuedConnection);
            }

            d->iconAlbumThumbThread->find(ImageInfo::thumbnailIdentifier(id));
        }

        // insert new entry to map, add album globalID
        QList<int> &list = d->idAlbumMap[id];
        list.removeAll(album->globalID());
        list.push_back(album->globalID());
    }
    else
    {
        // only add album global ID to list which is already inserted in map
        (*it).removeAll(album->globalID());
        (*it).push_back(album->globalID());
    }
}

void AlbumThumbnailLoader::setThumbnailSize(int size)
{
    if (d->iconSize == size)
    {
        return;
    }

    d->iconSize = size;

    // clear task list
    d->idAlbumMap.clear();
    // clear cached thumbnails
    d->thumbnailMap.clear();

    if (d->iconAlbumThumbThread)
    {
        d->iconAlbumThumbThread->stopLoading();
        d->iconAlbumThumbThread->setThumbnailSize(size);
    }

    if (d->iconTagThumbThread)
    {
        d->iconTagThumbThread->stopLoading();
        d->iconTagThumbThread->setThumbnailSize(size);
    }

    emit signalReloadThumbnails();
}

int AlbumThumbnailLoader::thumbnailSize() const
{
    return d->iconSize;
}

void AlbumThumbnailLoader::slotGotThumbnailFromIcon(const LoadingDescription& loadingDescription, const QPixmap& thumbnail)
{
    // We need to find all albums for which the given url has been requested,
    // and emit a signal for each album.

    ThumbnailIdentifier id = loadingDescription.thumbnailIdentifier();
    IdAlbumMap::iterator it = d->idAlbumMap.find(id.id);

    if (it != d->idAlbumMap.end())
    {
        AlbumManager* const manager = AlbumManager::instance();

        if (thumbnail.isNull())
        {
            // Loading failed
            for (QList<int>::const_iterator vit = (*it).constBegin(); vit != (*it).constEnd(); ++vit)
            {
                Album* const album = manager->findAlbum(*vit);

                if (album)
                {
                    emit signalFailed(album);
                }
            }
        }
        else
        {
            // Loading succeeded

            for (QList<int>::const_iterator vit = (*it).constBegin(); vit != (*it).constEnd(); ++vit)
            {
                // look up with global id
                Album* const album = manager->findAlbum(*vit);

                if (album)
                {
                    d->thumbnailMap.insert(album->globalID(), thumbnail);
                    emit signalThumbnail(album, thumbnail);
                }
            }
        }

        d->idAlbumMap.erase(it);
    }
}

void AlbumThumbnailLoader::slotDispatchThumbnailInternal(int albumID, const QPixmap& thumbnail)
{
    // for cached thumbnails

    AlbumManager* const manager = AlbumManager::instance();
    Album* const album          = manager->findAlbum(albumID);

    if (album)
    {
        if (thumbnail.isNull())
        {
            emit signalFailed(album);
        }
        else
        {
            emit signalThumbnail(album, thumbnail);
        }
    }
}

void AlbumThumbnailLoader::slotIconChanged(Album* album)
{
    if (!album || (album->type() != Album::TAG && album->type() != Album::PHYSICAL))
    {
        return;
    }

    d->thumbnailMap.remove(album->globalID());
}

/*
 * This code is maximally inefficient
QImage AlbumThumbnailLoader::getAlbumPreviewDirectly(PAlbum* const album, int size)
{
    if (album->iconId())
    {
        ThumbnailLoadThread* const thread    = new ThumbnailLoadThread;
        thread->setPixmapRequested(false);
        thread->setThumbnailSize(size);
        ThumbnailImageCatcher* const catcher = new ThumbnailImageCatcher(thread);
        catcher->setActive(true);
        catcher->thread()->find(ThumbnailIdentifier(album->iconId());
        catcher->enqueue();
        QList<QImage> images                 = catcher->waitForThumbnails();
        catcher->setActive(false);
        delete thread;
        delete catcher;

        if (!images.isEmpty())
            return images[0];
    }

    return loadIcon("folder", size).toImage();
}
*/

} // namespace Digikam
