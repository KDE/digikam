/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-22
 * Description : Qt item view for images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGECATEGORIZEDVIEW_H
#define IMAGECATEGORIZEDVIEW_H

// Qt includes

// KDE includes

#include <kcategorizedview.h>

// Local includes

#include "imageinfo.h"
#include "thumbnailsize.h"

namespace Digikam
{

class ImageAlbumModel;
class ImageAlbumFilterModel;
class ImageDelegate;
class ImageCategorizedViewPriv;

class ImageCategorizedView : public KCategorizedView
{
    Q_OBJECT

public:

    ImageCategorizedView(QWidget *parent = 0);
    ~ImageCategorizedView();

    ImageAlbumModel *imageModel() const;
    ImageAlbumFilterModel *imageFilterModel() const;

    ImageDelegate *delegate() const;

    ImageInfo currentInfo() const;
    QList<ImageInfo> selectedImageInfos() const;
    QList<ImageInfo> selectedImageInfosCurrentFirst() const;
    QList<ImageInfo> imageInfos() const;

    KUrl::List urls() const;
    KUrl::List selectedUrls() const;

    ThumbnailSize thumbnailSize() const;
    void setThumbnailSize(const ThumbnailSize &size);

public Q_SLOTS:

    void setThumbnailSize(int size);
    /** Scroll the view to the given item when it becomes available */
    void scrollToWhenAvailable(qlonglong imageId);
    /** Set as current item the item identified by its file url */
    void setCurrentUrl(const KUrl &url);

protected Q_SLOTS:

    void slotThemeChanged();
    void slotSetupChanged();

    void slotImageInfosAdded();

    void slotActivated(const QModelIndex &index);

protected:

    /// Reimplement these in a subclass
    virtual void activated(const ImageInfo &info);
    virtual void showContextMenu(QContextMenuEvent *event, const ImageInfo &info);
    virtual void showContextMenu(QContextMenuEvent *event);
    virtual void copy();
    virtual void paste();

    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent* event);
    bool viewportEvent(QEvent *event);

private:

    void scrollToStoredItem();

    ImageCategorizedViewPriv* const d;
};

}

#endif

