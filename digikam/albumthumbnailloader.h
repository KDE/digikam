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
     * Album thumbnail size is configurable via the settings menu.
     * Some widgets use smaller icons than other widgets.
     * These widgets do not need to know the currently set icon size from
     * the setup and calculate a smaller size, but can simply request
     * a relatively smaller icon.
     * Depending on the user-chosen icon size, this size may in fact not
     * be smaller than the normal size.
     */
    enum RelativeSize
    {
        NormalSize,
        SmallerSize
    };

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
      * Tag thumbnails will be processed as appropriate.
      * Tags may have associated an icon that is loaded
      * synchronously by the system icon loader.
      * In this case, icon is set to this icon, and false is returned.
      * If no icon is associated with the tag, icon is set to null,
      * and false is returned.
      * If a custom icon is associated with the tag,
      * it is loaded asynchronously, icon is set to null,
      * and true is returned.
      * Tag thumbnails are always smaller than album thumbnails -
      * as small as an album thumbnail with SmallerSize.
      * They are supposed to be blended into the standard tag icon
      * obtained below, or used as is when SmallerSize is requested anyway.
      * @return Returns true if icon is loaded asynchronously.
      */
    bool getTagThumbnail(TAlbum *album, QPixmap &icon);

    /**
     * Return standard tag and album icons.
     * The third methods check if album is the root,
     * and returns the standard icon or the root standard icon.
     */
    QPixmap getStandardTagIcon(RelativeSize size = NormalSize);
    QPixmap getStandardTagRootIcon(RelativeSize size = NormalSize);
    QPixmap getStandardTagIcon(TAlbum *album, RelativeSize size = NormalSize);

    QPixmap getStandardAlbumIcon(RelativeSize size = NormalSize);
    QPixmap getStandardAlbumRootIcon(RelativeSize size = NormalSize);
    QPixmap getStandardAlbumIcon(PAlbum *album, RelativeSize size = NormalSize);

    /**
     * Blend tagIcon centered on dstIcon, where dstIcon is a standard
     * icon of variable size and tagIcon is 12 pixels smaller.
     * If height(dstIcon) < minBlendSize we return tagIcon verbatim.
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
    QPixmap loadIcon(const QString &name, int size = 0);
    QPixmap createTagThumbnail(const QPixmap &albumThumbnail);
    int computeIconSize(RelativeSize size);
    QRect computeBlendRect(int iconSize);

};

}

#endif
