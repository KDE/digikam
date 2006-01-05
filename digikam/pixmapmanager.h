/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-14
 * Copyright 2005 by Renchi Raju
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

#ifndef PIXMAPMANAGER_H
#define PIXMAPMANAGER_H

// Qt includes.

#include <qobject.h>
#include <qcache.h>
#include <qguardedptr.h>
#include <qpixmap.h>

// KDE includes.

#include <kurl.h>

/** @file pixmapmanager.h */

class QPixmap;
class QTimer;

namespace Digikam
{

class AlbumIconView;
class ThumbnailJob;

/**
 * Since there are date based folders, the number of pixmaps which
 * could be kept in memory could potentially become too large. The
 * pixmapmanager maintains a fixed size cache of thumbnails and loads
 * pixmaps on demand.
 */
class PixmapManager : public QObject
{

    Q_OBJECT
    
public:

    PixmapManager(AlbumIconView* view);
    ~PixmapManager();

    QPixmap* find(const KURL& path);
    void     remove(const KURL& path);
    void     clear();
    void     setThumbnailSize(int size);
    int      cacheSize() const;

private:

    QCache<QPixmap>*          m_cache;
    QGuardedPtr<ThumbnailJob> m_thumbJob;
    int                       m_size;
    AlbumIconView*            m_view;
    QTimer*                   m_timer;
    KURL                      m_url;
    QString                   m_thumbCacheDir;
    
signals:

    void signalPixmap(const KURL& url);

private slots:

    void slotGotThumbnail(const KURL& url, const QPixmap& pix);
    void slotFailedThumbnail(const KURL& url);
    void slotCompleted();
};

}  // namespace Digikam

#endif /* PIXMAPMANAGER_H */
