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

// Local includes

#include "imageinfo.h"
#include "kcategorizedview.h"
#include "thumbnailsize.h"

namespace Digikam
{

class Album;
class ImageAlbumModel;
class ImageAlbumFilterModel;
class ImageDelegate;
class ImageDelegateOverlay;
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

    Album *currentAlbum() const;

    ImageInfo currentInfo() const;
    KUrl currentUrl() const;

    QList<ImageInfo> selectedImageInfos() const;
    QList<ImageInfo> selectedImageInfosCurrentFirst() const;
    KUrl::List selectedUrls() const;
    int numberOfSelectedIndexes() const;

    QList<ImageInfo> imageInfos() const;
    KUrl::List urls() const;

    /** Selects the index as current and scrolls to it */
    void toFirstIndex();
    void toLastIndex();
    void toNextIndex();
    void toPreviousIndex();
    void toIndex(const KUrl& url);
    void toIndex(const QModelIndex& index);
    /** Returns the n-th info after the given one.
     *  Specifically, return the previous info for nth = -1
     *  and the next info for n = 1.
     *  Returns a null info if either startingPoint or the nth info are
     *  not contained in the model */
    ImageInfo nextInOrder(const ImageInfo &startingPoint, int nth);
    ImageInfo previousInfo(const ImageInfo &info) { return nextInOrder(info, -1); }
    ImageInfo nextInfo(const ImageInfo &info) { return nextInOrder(info, 1); }

    void invertSelection();

    ThumbnailSize thumbnailSize() const;
    void setThumbnailSize(const ThumbnailSize& size);

    /** If the model is categorized by an album, returns the album of the category
     *  that contains the position.
     *  If this is not applicable, return the current album. May return 0. */
    Album *albumAt(const QPoint& pos);

    /// Add and remove an overlay. It will as well be removed automatically when destroyed.
    void addOverlay(ImageDelegateOverlay *overlay);
    void removeOverlay(ImageDelegateOverlay *overlay);

    void setToolTipEnabled(bool enabled);
    bool isToolTipEnabled() const;

    void addSelectionOverlay();

public Q_SLOTS:

    void openAlbum(Album *album);

    void setThumbnailSize(int size);
    /** Scroll the view to the given item when it becomes available */
    void scrollToWhenAvailable(qlonglong imageId);
    /** Set as current item the item identified by its file url */
    void setCurrentUrl(const KUrl& url);

Q_SIGNALS:

    void currentChanged(const ImageInfo& info);
    /// Emitted when any selection change occurs. Any of the signals below will be emitted before.
    void selectionChanged();
    /// Emitted when new items are selected. The parameter includes only the newly selected infos,
    /// there may be other already selected infos.
    void selected(const QList<ImageInfo>& newSelectedInfos);
    /// Emitted when items are deselected. There may be other selected infos left.
    /// This signal is not emitted when the model is reset; then only selectionCleared is emitted.
    void deselected(const QList<ImageInfo>& nowDeselectedInfos);
    /// Emitted when the selection is completely cleared.
    void selectionCleared();

    void zoomOutStep();
    void zoomInStep();

    /** For overlays: Like the respective parent class signals, but with additional info.
     *  Do not change the mouse events.
     */
    void clicked(const QMouseEvent *e, const QModelIndex& index);
    void entered(const QMouseEvent *e, const QModelIndex& index);
    /**  Remember you may want to check if the event is accepted or ignored.
     *   This signal is emitted after being handled by this widget.
     *   You can accept it if ignored. */
    void keyPressed(QKeyEvent *e);

protected Q_SLOTS:

    virtual void slotThemeChanged();
    virtual void slotSetupChanged();

    void slotImageInfosAdded();

    void slotActivated(const QModelIndex& index);
    void slotClicked(const QModelIndex& index);
    void slotEntered(const QModelIndex& index);
    void layoutAboutToBeChanged();
    void layoutWasChanged();

protected:

    /// Reimplement these in a subclass
    virtual void activated(const ImageInfo& info);
    virtual void showContextMenu(QContextMenuEvent *event, const ImageInfo& info);
    virtual void showContextMenu(QContextMenuEvent *event);
    virtual void copy();
    virtual void paste();

    /** Returns an index that is representative for the category at position pos */
    QModelIndex indexForCategoryAt(const QPoint& pos) const;

    // reimplemented from parent class
    void contextMenuEvent(QContextMenuEvent* event);
    void currentChanged(const QModelIndex& index, const QModelIndex& previous);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);
    void keyPressEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    void reset();
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void selectionChanged(const QItemSelection &, const QItemSelection &);
    void startDrag(Qt::DropActions supportedActions);
    bool viewportEvent(QEvent *event);
    void wheelEvent(QWheelEvent* event);

private Q_SLOTS:

    void slotGridSizeChanged(const QSize &);
    void slotFileChanged(const QString& filePath);

private:

    void updateDelegateSizes();
    void scrollToStoredItem();
    void userInteraction();
    void ensureSelectionAfterChanges();

private:

    ImageCategorizedViewPriv* const d;
};

} // namespace Digikam

#endif /* IMAGECATEGORIZEDVIEW_H */
