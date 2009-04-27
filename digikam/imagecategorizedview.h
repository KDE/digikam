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

class Album;
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

    void openAlbum(Album *album);

    void setThumbnailSize(int size);
    /** Scroll the view to the given item when it becomes available */
    void scrollToWhenAvailable(qlonglong imageId);
    /** Set as current item the item identified by its file url */
    void setCurrentUrl(const KUrl &url);

Q_SIGNALS:

    void currentChanged(const ImageInfo &info);
    /// Emitted when any selection change occurs. Any of the signals below will be emitted before.
    void selectionChanged();
    /// Emitted when new items are selected. The parameter includes only the newly selected infos,
    /// there may be other already selected infos.
    void selected(const QList<ImageInfo> &newSelectedInfos);
    /// Emitted when items are deselected. There may be other selected infos left.
    /// This signal is not emitted when the model is reset; then only selectionCleared is emitted.
    void deselected(const QList<ImageInfo> &nowDeselectedInfos);
    /// Emitted when the selection is completely cleared.
    void selectionCleared();

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

    // reimplemented from parent class
    void reset();
    void currentChanged(const QModelIndex &index, const QModelIndex &previous);
    void selectionChanged(const QItemSelection &, const QItemSelection &);
    void contextMenuEvent(QContextMenuEvent* event);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    bool viewportEvent(QEvent *event);

private Q_SLOTS:

    void slotGridSizeChanged(const QSize &);
    void slotDelegateWaitsForThumbnail(const QModelIndex &);

private:

    void updateDelegateSizes();
    void scrollToStoredItem();

private:

    ImageCategorizedViewPriv* const d;
};

} // namespace Digikam

#endif /* IMAGECATEGORIZEDVIEW_H */
