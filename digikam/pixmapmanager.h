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

#ifndef PIXMAPMANAGER_H
#define PIXMAPMANAGER_H

// Qt includes.

#include <qobject.h>

/** @file pixmapmanager.h */

class QPixmap;

class KURL;

namespace Digikam
{

class AlbumIconView;
class PixmapManagerPriv;

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
   
signals:

    void signalPixmap(const KURL& url);

private slots:

    void slotGotThumbnail(const KURL& url, const QPixmap& pix);
    void slotFailedThumbnail(const KURL& url);
    void slotCompleted();

private:

    PixmapManagerPriv *d;
};

}  // namespace Digikam

#endif /* PIXMAPMANAGER_H */
