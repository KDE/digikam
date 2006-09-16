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

#ifndef TAGTHUMBNAILLOADER_H
#define TAGTHUMBNAILLOADER_H

// Qt includes

#include <qobject.h>
#include <qpixmap.h>

// KDE includes

#include <kurl.h>

class QCustomEvent;

namespace Digikam
{

class Album;
class TAlbum;
class PAlbum;
class AlbumThumbnailLoaderPrivate;

class AlbumThumbnailLoader : public QObject
{
    Q_OBJECT

public:

    static AlbumThumbnailLoader *instance();
    static void cleanUp();

    /**
     * Request thumbnail for given album.
     * The thumbnail will be loaded
     * and returned asynchronously by the signals.
     * If no thumbnail is associated with given album,
     * no action will be taken, and false is returned.
     * 
    */
    bool getAlbumThumbnail(PAlbum *album);

    /**
      * Behaves similar to the above method.
      * Tag thumbnails will be processed as appropriate (Size 20).
      * Tags may have associated an icon that is loaded
      * synchronously by the system icon loader (size 20).
      * In this case, icon is set to this icon, and false is returned.
      * If no icon is associated with the tag, icon is set to null,
      * and false is returned.
      * If a custom icon is associated with the tag,
      * it is loaded asynchronously, icon is set to null,
      * and true is returned.
      * @return Returns true if icon is loaded asynchronously.
      */
    bool getTagThumbnail(TAlbum *album, QPixmap &icon);

    /**
     * Return standard tag and album icons.
     * The third methods check if album is the root,
     * and returns the standard icon or the root standard icon.
     */
    QPixmap getStandardTagIcon(int size = 32);
    QPixmap getStandardTagRootIcon(int size = 32);
    QPixmap getStandardTagIcon(TAlbum *album, int size = 32);

    QPixmap getStandardAlbumIcon(int size = 32);
    QPixmap getStandardAlbumRootIcon(int size = 32);
    QPixmap getStandardAlbumIcon(PAlbum *album, int size = 32);

    /**
     * Blend tagIcon centered on dstIcon, where dstIcon is a standard
     * icon of size 32 and tagIcon has size 20.
     */
    QPixmap blendIcons(QPixmap dstIcon, const QPixmap &tagIcon);

signals:

    /**
     * This signal is emitted as soon as a thumbnail has become available
     * for given album.
     * This class is a singleton, so any object connected to this
     * signal might not actually have requested a thumbnail for given url
     */
    void signalThumbnail(Album *album, const QPixmap&);

    /** This signal is emitted if thumbnail generation for given album failed.
     *  Same considerations as above.
     */
    void signalFailed(Album *album);

protected slots:

    void slotGotThumbnailFromIcon(const KURL&, const QPixmap&);
    void slotThumbnailLost(const KURL&);
    void slotIconChanged(Album* album);

protected:

    void customEvent(QCustomEvent *e);

private:

    AlbumThumbnailLoader();
    ~AlbumThumbnailLoader();
    AlbumThumbnailLoaderPrivate *d;
    static AlbumThumbnailLoader *m_instance;

    void addURL(Album *album, const KURL &url);
    QPixmap loadIcon(const QString &name, int size = 32);
    QPixmap createTagThumbnail(const QPixmap &albumThumbnail);

};

}

#endif
