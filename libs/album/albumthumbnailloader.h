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

#ifndef ALBUMTHUMBNAILLOADER_H
#define ALBUMTHUMBNAILLOADER_H

// Qt includes

#include <QObject>
#include <QPixmap>

// Local includes

#include "digikam_export.h"

namespace Digikam
{
class Album;
class TAlbum;
class PAlbum;
class LoadingDescription;

class DIGIKAM_EXPORT AlbumThumbnailLoader : public QObject
{
    Q_OBJECT

public:

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

public:

    void cleanUp();

    /**
     * Change the size of the thumbnails.
     * If the size differs from the current size,
     * signalReloadThumbnails will be emitted.
     */
    void setThumbnailSize(int size);

    /**
     * Get the current default icon size
     */
    int  thumbnailSize() const;

    /**
     * Request thumbnail for given album.
     * The thumbnail will be loaded
     * and returned asynchronously by the signals.
     * If no thumbnail is associated with given album,
     * no action will be taken, and false is returned.
     */
    bool getAlbumThumbnail(PAlbum* const album);

    /**
     * Request thumbnail for given album,
     * with slightly different behavior than the above method:
     * If the thumbnail is already available in the cache,
     * it is returned.
     * If the icon is not yet loaded, it will be returned asynchronously
     * by the signals, and a default icon is returned here.
     * If no icon is associated, the default icon is returned.
     */
    QPixmap getAlbumThumbnailDirectly(PAlbum* const album);

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
    bool getTagThumbnail(TAlbum* const album, QPixmap& icon);

    /**
     * Loads tag thumbnail,
     * with slightly different behavior than the above method:
     * If the thumbnail is already available in the cache,
     * it is returned, already blended with the standard icon, if requested.
     * If the icon is not yet loaded, it will be returned asynchronously
     * by the signals (unblended), and a default icon is returned here.
     * If no icon is associated, the default icon is returned.
     */
    QPixmap getTagThumbnailDirectly(TAlbum* const album);

    /**
     * Return standard tag and album icons.
     * The third methods check if album is the root,
     * and returns the standard icon or the root standard icon.
     */
    QPixmap getStandardTagIcon(RelativeSize size = NormalSize);
    QPixmap getStandardTagRootIcon(RelativeSize size = NormalSize);
    QPixmap getStandardTagIcon(TAlbum* const album, RelativeSize size = NormalSize);
    QPixmap getNewTagIcon(RelativeSize size = NormalSize);

    QPixmap getStandardAlbumIcon(RelativeSize size = NormalSize);
    QPixmap getStandardAlbumTrashIcon(RelativeSize size = NormalSize);
    QPixmap getStandardAlbumRootIcon(RelativeSize size = NormalSize);
    QPixmap getStandardAlbumIcon(PAlbum* const album, RelativeSize size = NormalSize);

    /** Return a preview of physical album directly without to use cache.
     *  Size of image can be passed as argument.
     */
    //QImage getAlbumPreviewDirectly(PAlbum* const album, int size);

public:

    static AlbumThumbnailLoader* instance();

Q_SIGNALS:

    /**
     * This signal is emitted as soon as a thumbnail has become available
     * for given album.
     * This class is a singleton, so any object connected to this
     * signal might not actually have requested a thumbnail for given url
     */
    void signalThumbnail(Album* album, const QPixmap&);

    /** This signal is emitted if thumbnail generation for given album failed.
     *  Same considerations as above.
     */
    void signalFailed(Album* album);

    /**
     * Indicates that all album and tag thumbnails need to be reloaded.
     * This is usually because the icon size has changed in the setup.
     */
    void signalReloadThumbnails();

    /**
     * Internal signal to dispatch Album thumbnail change.
     */
    void signalDispatchThumbnailInternal(int albumID, const QPixmap& thumbnail);

protected Q_SLOTS:

    void slotGotThumbnailFromIcon(const LoadingDescription& loadingDescription, const QPixmap& pixmap);
    void slotIconChanged(Album* album);
    void slotDispatchThumbnailInternal(int albumID, const QPixmap& thumbnail);

private:

    AlbumThumbnailLoader();
    ~AlbumThumbnailLoader();

    void    addUrl(Album* const album, qlonglong id);
    QPixmap loadIcon(const QString& name, int size = 0) const;
    int     computeIconSize(RelativeSize size)          const;

private:

    friend class AlbumThumbnailLoaderCreator;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ALBUMTHUMBNAILLOADER_H
