/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date  : 2006-04-14
 * Description : Load and cache tag thumbnails
 *
 * Copyright 2006 by Marcel Wiesweg
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

// Qt includes

#include <qmap.h>
#include <qpainter.h>
#include <qvaluelist.h>

// KDE includes

#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"
#include "thumbnailjob.h"
#include "thumbnailsize.h"

namespace Digikam
{

typedef QMap<KURL, QValueList<int> > UrlAlbumMap;
typedef QMap<int, QPixmap> TagThumbnailMap;

class AlbumThumbnailLoaderPrivate
{
public:
    AlbumThumbnailLoaderPrivate()
    {
        tagIconSize        = 20;
        iconAlbumThumbJob  = 0;
        iconTagThumbJob    = 0;
        //cache        = new QCache<QPixmap>(101, 211);
    }

    int                     tagIconSize;

    ThumbnailJob           *iconTagThumbJob;

    ThumbnailJob           *iconAlbumThumbJob;

    UrlAlbumMap             urlAlbumMap;

    TagThumbnailMap         tagThumbnailMap;

    //QCache<QPixmap>        *m_cache;
};

class AlbumThumbnailLoaderEvent : public QCustomEvent
{
public:
    AlbumThumbnailLoaderEvent(int albumID, const QPixmap &thumbnail)
        : QCustomEvent(QEvent::User),
          albumID(albumID), thumbnail(thumbnail)
        {};

    int albumID;
    QPixmap thumbnail;
};

AlbumThumbnailLoader *AlbumThumbnailLoader::m_instance = 0;

AlbumThumbnailLoader *AlbumThumbnailLoader::instance()
{
    if (!m_instance)
        m_instance = new AlbumThumbnailLoader;
    return m_instance;
}

void AlbumThumbnailLoader::cleanUp()
{
    delete m_instance;
}

AlbumThumbnailLoader::AlbumThumbnailLoader()
{
    d = new AlbumThumbnailLoaderPrivate;

    connect(AlbumManager::instance(), SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotIconChanged(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotIconChanged(Album*)));
}

AlbumThumbnailLoader::~AlbumThumbnailLoader()
{
    if (d->iconTagThumbJob)
        d->iconTagThumbJob->kill();

    if (d->iconAlbumThumbJob)
        d->iconAlbumThumbJob->kill();
    //delete d->cache;

    delete d;

    m_instance = 0;
}

QPixmap AlbumThumbnailLoader::getStandardTagIcon(int size)
{
    return loadIcon("tag", size);
}

QPixmap AlbumThumbnailLoader::getStandardTagRootIcon(int size)
{
    return loadIcon("tag-folder", size);
}

QPixmap AlbumThumbnailLoader::getStandardTagIcon(TAlbum *album, int size)
{
    if (album->isRoot())
        return getStandardTagRootIcon(size);
    else
        return getStandardTagIcon(size);
}

QPixmap AlbumThumbnailLoader::getStandardAlbumIcon(int size)
{
    return loadIcon("folder", size);
}

QPixmap AlbumThumbnailLoader::getStandardAlbumRootIcon(int size)
{
    return loadIcon("folder_image", size);
}

QPixmap AlbumThumbnailLoader::getStandardAlbumIcon(PAlbum *album, int size)
{
    if (album->isRoot())
        return getStandardAlbumRootIcon(size);
    else
        return getStandardAlbumIcon(size);
}

QPixmap AlbumThumbnailLoader::loadIcon(const QString &name, int size)
{
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
    return iconLoader->loadIcon(name, KIcon::NoGroup,
                                size, KIcon::DefaultState,
                                0, true);
}

bool AlbumThumbnailLoader::getTagThumbnail(TAlbum *album, QPixmap &icon)
{
    if(!album->icon().isEmpty())
    {
        if(album->icon().startsWith("/"))
        {
            KURL iconKURL;
            iconKURL.setPath(album->icon());
            addURL(album, iconKURL);
            icon = QPixmap();
            return true;
        }
        else
        {
            icon = loadIcon(album->icon(), d->tagIconSize);
            return false;
        }
    }
    else
    {
        icon = QPixmap();
        return false;
    }
}

bool AlbumThumbnailLoader::getAlbumThumbnail(PAlbum *album)
{
    if(!album->icon().isEmpty())
    {
        addURL(album, album->iconKURL());
    }
    else
    {
        return false;
    }

    return true;
}

void AlbumThumbnailLoader::addURL(Album *album, const KURL &url)
{
    /*
    QPixmap* pix = d->cache->find(album->iconKURL().path());
    if (pix)
    return pix;
    */

    // First check cached thumbnails.
    // At startup, this is not relevant, as the views will add their requests in a row.
    // This is to speed up context menu and IE imagedescedit
    TagThumbnailMap::iterator ttit = d->tagThumbnailMap.find(album->globalID());
    if (ttit != d->tagThumbnailMap.end())
    {
        // It is not necessary to return cached icon asynchronously - they could be
        // returned by getTagThumbnail already - but this would make the API
        // less elegant, it feels much better this way.
        QApplication::postEvent(this, new AlbumThumbnailLoaderEvent(album->globalID(), *ttit));
        return;
    }

    // Check if the URL has already been added (ThumbnailJob will _not_ check this)
    UrlAlbumMap::iterator it = d->urlAlbumMap.find(url);

    if (it == d->urlAlbumMap.end())
    {
        // use two IOslaves so that tag and album thumbnails are loaded
        // in parallel and not first album, then tag thumbnails
        if (album->type() == Album::TAG)
        {
            if(!d->iconTagThumbJob)
            {
                d->iconTagThumbJob = new ThumbnailJob(url,
                        (int)ThumbnailSize::Tiny,
                        true,
                        AlbumSettings::instance()->getExifRotate());
                connect(d->iconTagThumbJob,
                        SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                        SLOT(slotGotThumbnailFromIcon(const KURL&, const QPixmap&)));
                connect(d->iconTagThumbJob,
                        SIGNAL(signalFailed(const KURL&)),
                        SLOT(slotThumbnailLost(const KURL&)));
            }
            else
            {
                d->iconTagThumbJob->addItem(url);
            }
        }
        else
        {
            if(!d->iconAlbumThumbJob)
            {
                d->iconAlbumThumbJob = new ThumbnailJob(url,
                        (int)ThumbnailSize::Tiny,
                        true,
                        AlbumSettings::instance()->getExifRotate());
                connect(d->iconAlbumThumbJob,
                        SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                        SLOT(slotGotThumbnailFromIcon(const KURL&, const QPixmap&)));
                connect(d->iconAlbumThumbJob,
                        SIGNAL(signalFailed(const KURL&)),
                        SLOT(slotThumbnailLost(const KURL&)));
            }
            else
            {
                d->iconAlbumThumbJob->addItem(url);
            }
        }

        // insert new entry to map, add album globalID
        QValueList<int> &list = d->urlAlbumMap[url];
        list.remove(album->globalID());
        list.push_back(album->globalID());
    }
    else
    {
        // only add album global ID to list which is already inserted in map
        (*it).remove(album->globalID());
        (*it).push_back(album->globalID());
    }
}

/*
void AlbumThumbnailLoader::setThumbnailSize(int size)
{
    if (d->size == size)
        return;

    d->size = size;
    d->cache->clear();
    if (d->thumbJob)
    {
        d->thumbJob->kill();
        d->thumbJob = 0;
    }
}
*/

void AlbumThumbnailLoader::slotGotThumbnailFromIcon(const KURL &url, const QPixmap &thumbnail)
{
    // We need to find all albums for which the given url has been requested,
    // and emit a signal for each album.

    UrlAlbumMap::iterator it = d->urlAlbumMap.find(url);

    if (it != d->urlAlbumMap.end())
    {
        QPixmap tagThumbnail;

        AlbumManager *manager = AlbumManager::instance();
        for (QValueList<int>::iterator vit = (*it).begin(); vit != (*it).end(); ++vit)
        {
            // look up with global id
            Album *album = manager->findAlbum(*vit);
            if (album)
            {
                if (album->type() == Album::TAG)
                {
                    // create tag thumbnail if needed
                    if (tagThumbnail.isNull())
                    {
                        tagThumbnail = createTagThumbnail(thumbnail);
                        d->tagThumbnailMap.insert(album->globalID(), tagThumbnail);
                    }

                    emit signalThumbnail(album, tagThumbnail);
                }
                else
                {
                    emit signalThumbnail(album, thumbnail);
                }
            }
        }

        d->urlAlbumMap.remove(it);
    }

}

void AlbumThumbnailLoader::customEvent(QCustomEvent *e)
{
    // for cached thumbnails

    AlbumThumbnailLoaderEvent *atle = (AlbumThumbnailLoaderEvent *)e;
    AlbumManager *manager = AlbumManager::instance();
    Album *album = manager->findAlbum(atle->albumID);
    if (album)
    {
        if (atle->thumbnail.isNull())
            emit signalFailed(album);
        else
            emit signalThumbnail(album, atle->thumbnail);
    }
}

void AlbumThumbnailLoader::slotIconChanged(Album* album)
{
    if(!album || album->type() != Album::TAG)
        return;

    d->tagThumbnailMap.remove(album->globalID());
}

QPixmap AlbumThumbnailLoader::createTagThumbnail(const QPixmap &albumThumbnail)
{
    // tag thumbnails are cropped

    QPixmap tagThumbnail;

    if(!albumThumbnail.isNull() && (d->tagIconSize < ThumbnailSize::Tiny))
    {
        int w1 = albumThumbnail.width();
        int w2 = d->tagIconSize;
        int h1 = albumThumbnail.height();
        int h2 = d->tagIconSize;
        tagThumbnail.resize(w2,h2);
        bitBlt(&tagThumbnail, 0, 0, &albumThumbnail, (w1-w2)/2, (h1-h2)/2, w2, h2);
    }
    else
    {
        tagThumbnail = albumThumbnail;
    }

    return tagThumbnail;
}

void AlbumThumbnailLoader::slotThumbnailLost(const KURL &url)
{
    // Same code as above, only different signal

    UrlAlbumMap::iterator it = d->urlAlbumMap.find(url);

    if (it != d->urlAlbumMap.end())
    {
        AlbumManager *manager = AlbumManager::instance();
        for (QValueList<int>::iterator vit = (*it).begin(); vit != (*it).end(); ++vit)
        {
            Album *album = manager->findAlbum(*vit);
            if (album)
                emit signalFailed(album);
        }

        d->urlAlbumMap.remove(it);
    }
}

QPixmap AlbumThumbnailLoader::blendIcons(QPixmap dstIcon, const QPixmap &tagIcon)
{
    if(!tagIcon.isNull())
    {
        QPainter p(&dstIcon);
        p.drawPixmap(6, 9, tagIcon, 0, 0, -1, -1);
        p.end();
    }
    return dstIcon;
}

} // namespace Digikam

#include "albumthumbnailloader.moc"


