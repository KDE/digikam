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
 * ============================================================ */

#ifndef PIXMAPMANAGER_H
#define PIXMAPMANAGER_H

#include <qobject.h>
#include <qcache.h>
#include <qguardedptr.h>
#include <qpixmap.h>
#include <kurl.h>

#include "thumbnailjob.h"

class KFileMetaInfo;
class QPixmap;
class QTimer;
class AlbumIconView;

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
    
signals:

    void signalPixmap(const KURL& url);

private slots:

    void slotGotThumbnail(const KURL& url, const QPixmap& pix,
                          const KFileMetaInfo*);
    void slotFailedThumbnail(const KURL& url);
    void slotCompleted();
};

#endif /* PIXMAPMANAGER_H */
