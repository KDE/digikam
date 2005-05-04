/* ============================================================
 * File  : pixmapmanager.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-14
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include <qpixmap.h>
#include <qdir.h>
#include <qtimer.h>
#include <qimage.h>
#include <kurl.h>

#include "albumiconview.h"
#include "albumiconitem.h"
#include "thumbdb.h"
#include "albumsettings.h"
#include "pixmapmanager.h"

PixmapManager::PixmapManager(AlbumIconView* view)
{
    m_view  = view;
    m_cache = new QCache<QPixmap>(101, 211);
    m_cache->setAutoDelete(true);
    m_size  = 0;
    
    m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()),
            SLOT(slotCompleted()));
}

PixmapManager::~PixmapManager()
{
    delete m_timer;
    
    if (!m_thumbJob.isNull())
    {
        m_thumbJob->kill();
    }

    delete m_cache;
}

void PixmapManager::setThumbnailSize(int size)
{
    if (m_size == size)
        return;

    m_size = size;
    m_cache->clear();
    if (!m_thumbJob.isNull())
        m_thumbJob->kill();
}

QPixmap* PixmapManager::find(const KURL& url)
{
    QPixmap* pix = m_cache->find(url.path());
    if (pix)
        return pix;
    
    if (m_thumbJob.isNull())
    {
        m_thumbJob = new ThumbnailJob(url, m_size, true);
        connect(m_thumbJob,
                SIGNAL(signalThumbnailMetaInfo(const KURL&,
                                               const QPixmap&,
                                               const KFileMetaInfo*)),
                SLOT(slotGotThumbnail(const KURL&,
                                      const QPixmap&,
                                      const KFileMetaInfo*)));
        connect(m_thumbJob, 
                SIGNAL(signalCompleted()),
                SLOT(slotCompleted()));
    }
    
    return 0;
}

void PixmapManager::remove(const KURL& url)
{
    m_cache->remove(url.path());

    if (!m_thumbJob.isNull())
        m_thumbJob->removeItem(url);
}

void PixmapManager::clear()
{
    if (!m_thumbJob.isNull())
    {
        m_thumbJob->kill();
    }

    m_cache->clear();
}

void PixmapManager::slotGotThumbnail(const KURL& url, const QPixmap& pix,
                                     const KFileMetaInfo*)
{
    m_cache->remove(url.path());
    QPixmap* thumb = new QPixmap(pix);
    m_cache->insert(url.path(), thumb);
    emit signalPixmap(url);
}

void PixmapManager::slotCompleted()
{
    if (!m_thumbJob.isNull())
        m_thumbJob->kill();

    AlbumIconItem* item = m_view->nextItemToThumbnail();
    if (!item)
        return;

    find(item->imageInfo()->kurl());
}

int PixmapManager::cacheSize() const
{
    return m_cache->maxCost();    
}


#include "pixmapmanager.moc"
