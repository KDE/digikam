/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2005-04-14
 * Description : a pixmap manager for album icon view.  
 * 
 * Copyright 2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

// C Ansi includes.

extern "C"
{
#include <unistd.h>
}

// Qt includes.

#include <qpixmap.h>
#include <qdir.h>
#include <qfile.h>
#include <qtimer.h>
#include <qimage.h>

// KDE includes.

#include <kurl.h>
#include <kmdcodec.h>
#include <kiconloader.h>

// Local includes.

#include "thumbnailjob.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "albumsettings.h"
#include "pixmapmanager.h"
#include "pixmapmanager.moc"

/** @file pixmapmanager.cpp */

namespace Digikam
{

PixmapManager::PixmapManager(AlbumIconView* view)
{
    m_view  = view;
    m_cache = new QCache<QPixmap>(101, 211);
    m_cache->setAutoDelete(true);
    m_size  = 0;
    m_thumbCacheDir = QDir::homeDirPath() + "/.thumbnails/";
    
    m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()),
            this, SLOT(slotCompleted()));
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
    {
        m_thumbJob->kill();
        m_thumbJob = 0;
    }
}

QPixmap* PixmapManager::find(const KURL& url)
{
    QPixmap* pix = m_cache->find(url.path());
    if (pix)
        return pix;
    
    if (m_thumbJob.isNull())
    {
        m_thumbJob = new ThumbnailJob(url, m_size, true, AlbumSettings::instance()->getExifRotate());
        
        connect(m_thumbJob, SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                this, SLOT(slotGotThumbnail(const KURL&, const QPixmap&)));

        connect(m_thumbJob, SIGNAL(signalFailed(const KURL&)),
                this, SLOT(slotFailedThumbnail(const KURL&)));

        connect(m_thumbJob, SIGNAL(signalCompleted()),
                this, SLOT(slotCompleted()));
    }
    
    return 0;
}

void PixmapManager::remove(const KURL& url)
{
    m_cache->remove(url.path());

    if (!m_thumbJob.isNull())
        m_thumbJob->removeItem(url);

    // remove the items from the thumbnail cache directory as well.
    QString uri = "file://" + QDir::cleanDirPath(url.path());
    KMD5 md5(QFile::encodeName(uri));
    uri = md5.hexDigest();

    QString smallThumbPath = m_thumbCacheDir + "normal/" + uri + ".png";
    QString bigThumbPath   = m_thumbCacheDir + "large/"  + uri + ".png";

    ::unlink(QFile::encodeName(smallThumbPath));
    ::unlink(QFile::encodeName(bigThumbPath));
}

void PixmapManager::clear()
{
    if (!m_thumbJob.isNull())
    {
        m_thumbJob->kill();
        m_thumbJob = 0;
    }

    m_cache->clear();
}

void PixmapManager::slotGotThumbnail(const KURL& url, const QPixmap& pix)
{
    m_cache->remove(url.path());
    QPixmap* thumb = new QPixmap(pix);
    m_cache->insert(url.path(), thumb);
    emit signalPixmap(url);
}

void PixmapManager::slotFailedThumbnail(const KURL& url)
{
    QImage img;
    QString ext = QFileInfo(url.path()).extension(false);

    // Wrapper around mime type of item to get the right icon.

    AlbumSettings* settings = AlbumSettings::instance();
    if (settings)
    {
        if (settings->getImageFileFilter().upper().contains(ext.upper()) ||
            settings->getRawFileFilter().upper().contains(ext.upper()))
        { 
            img = DesktopIcon("image", KIcon::SizeEnormous).convertToImage();
        }
        else if (settings->getMovieFileFilter().upper().contains(ext.upper()))
        {
            img = DesktopIcon("video", KIcon::SizeEnormous).convertToImage();
        }
        else if (settings->getAudioFileFilter().upper().contains(ext.upper()))
        {
            img = DesktopIcon("sound", KIcon::SizeEnormous).convertToImage();
        }
    }

    if (img.isNull())
        img = DesktopIcon("file_broken", KIcon::SizeEnormous).convertToImage();

    // Resize icon to the right size depending of current settings.

    QSize size(img.size());
    size.scale(m_size, m_size, QSize::ScaleMin);
    if (size.width() < img.width() && size.height() < img.height())
    {
        // only scale down
        // do not scale up, looks bad
        img = img.smoothScale(size);
    }

    m_cache->remove(url.path());
    QPixmap* thumb = new QPixmap(img);
    m_cache->insert(url.path(), thumb);
    emit signalPixmap(url);
}

void PixmapManager::slotCompleted()
{
    if (!m_thumbJob.isNull())
    {
        m_thumbJob->kill();
        m_thumbJob = 0;
    }

    AlbumIconItem* item = m_view->nextItemToThumbnail();
    if (!item)
        return;

    find(item->imageInfo()->kurl());
}

int PixmapManager::cacheSize() const
{
    return m_cache->maxCost();    
}

}  // namespace Digikam
