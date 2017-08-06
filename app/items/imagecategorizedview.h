/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-22
 * Description : Qt item view for images
 *
 * Copyright (C) 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2017      by Simon Frei <freisim93 at gmail dot com>
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

// Local includes

#include "applicationsettings.h"
#include "imageinfo.h"
#include "itemviewcategorized.h"
#include "thumbnailsize.h"
#include "iccsettingscontainer.h"

namespace Digikam
{

class Album;
class ImageAlbumModel;
class ImageAlbumFilterModel;
class ImageModel;
class ImageFilterModel;
class ImageSortFilterModel;
class ImageDelegate;
class ImageDelegateOverlay;
class ImageThumbnailModel;

class ImageCategorizedView : public ItemViewCategorized
{
    Q_OBJECT

public:

    explicit ImageCategorizedView(QWidget* const parent = 0);
    ~ImageCategorizedView();

    void setModels(ImageModel* model, ImageSortFilterModel* filterModel);

    ImageModel*            imageModel()            const;
    ImageSortFilterModel*  imageSortFilterModel()  const;

    QItemSelectionModel*   getSelectionModel()     const;

    /// Returns any ImageFilterMode in chain. May not be sourceModel()
    ImageFilterModel*      imageFilterModel()      const;

    /// Returns 0 if the ImageModel is not an ImageThumbnailModel
    ImageThumbnailModel*   imageThumbnailModel()   const;

    /// Returns 0 if the ImageModel is not an ImageAlbumModel
    ImageAlbumModel*       imageAlbumModel()       const;
    ImageAlbumFilterModel* imageAlbumFilterModel() const;

    ImageDelegate*         delegate()              const;

    Album*                 currentAlbum()          const;
    ImageInfo              currentInfo()           const;
    QUrl                   currentUrl()            const;

    ImageInfo              imageInfo(const QModelIndex& index) const;
    ImageInfoList          imageInfos(const QList<QModelIndex>& indexes,
                                      bool grouping = false) const;
    ImageInfoList          imageInfos(const QList<QModelIndex>& indexes,
                                      ApplicationSettings::OperationType type) const;

    ImageInfoList          selectedImageInfos(bool grouping = false) const;
    ImageInfoList          selectedImageInfos(ApplicationSettings::OperationType type) const;
    ImageInfoList          selectedImageInfosCurrentFirst(bool grouping = false) const;
    QList<QUrl>            selectedUrls(bool grouping = false) const;
    QList<QUrl>            selectedUrls(ApplicationSettings::OperationType type) const;

    ImageInfoList          allImageInfos(bool grouping = false) const;
    QList<QUrl>            allUrls(bool grouping = false) const;

    bool                   needGroupResolving(ApplicationSettings::OperationType type,
                                              bool all = false) const;

    /** Selects the index as current and scrolls to it.
     */
    void toIndex(const QUrl& url);

    /** Returns the n-th info after the given one.
     *  Specifically, return the previous info for nth = -1
     *  and the next info for n = 1.
     *  Returns a null info if either startingPoint or the nth info are
     *  not contained in the model.
     */
    ImageInfo nextInOrder(const ImageInfo& startingPoint, int nth);

    ImageInfo previousInfo(const ImageInfo& info)
    {
        return nextInOrder(info, -1);
    }

    ImageInfo nextInfo(const ImageInfo& info)
    {
        return nextInOrder(info, 1);
    }

    QModelIndex indexForInfo(const ImageInfo& info) const;

    ThumbnailSize thumbnailSize() const;

    virtual void setThumbnailSize(const ThumbnailSize& size);

    /** If the model is categorized by an album, returns the album of the category
     *  that contains the position.
     *  If this is not applicable, return the current album. May return 0.
     */
    Album* albumAt(const QPoint& pos) const;

    /// Add and remove an overlay. It will as well be removed automatically when destroyed.
    /// Unless you pass a different delegate, the current delegate will be used.
    void addOverlay(ImageDelegateOverlay* overlay, ImageDelegate* delegate = 0);

    void removeOverlay(ImageDelegateOverlay* overlay);

    void addSelectionOverlay(ImageDelegate* delegate = 0);

public Q_SLOTS:

    void openAlbum(QList<Album*> album);

    void setThumbnailSize(int size);

    /** Scroll the view to the given item when it becomes available.
     */
    void setCurrentWhenAvailable(qlonglong imageId);

    /** Set as current item when it becomes available, the item identified by its file url.
     */
    void setCurrentUrlWhenAvailable(const QUrl& url);

    /** Set as current item the item identified by its file url.
     */
    void setCurrentUrl(const QUrl& url);

    /** Set as current item the item identified by the imageinfo.
     */
    void setCurrentInfo(const ImageInfo& info);

    /** Set selected items identified by their file urls.
     */
    void setSelectedUrls(const QList<QUrl>& urlList);

    /** Set selected items.
     */
    void setSelectedImageInfos(const QList<ImageInfo>& infos);

    /** Does something to gain attention for info, but not changing current selection.
     */
    void hintAt(const ImageInfo& info);

Q_SIGNALS:

    void currentChanged(const ImageInfo& info);

    /// Emitted when new items are selected. The parameter includes only the newly selected infos,
    /// there may be other already selected infos.
    void selected(const QList<ImageInfo>& newSelectedInfos);

    /// Emitted when items are deselected. There may be other selected infos left.
    /// This signal is not emitted when the model is reset; then only selectionCleared is emitted.
    void deselected(const QList<ImageInfo>& nowDeselectedInfos);

    /// Emitted when the given image is activated. Info is never null.
    void imageActivated(const ImageInfo& info);

    /// Emitted when a new model is set
    void modelChanged();

protected Q_SLOTS:

    void slotImageInfosAdded();
    void slotCurrentUrlTimer();

protected:

    /// install default ImageAlbumModel and filter model, ready for use
    void installDefaultModels();

    // reimplemented from parent class

    QSortFilterProxyModel*       filterModel()     const;
    AbstractItemDragDropHandler* dragDropHandler() const;
    QModelIndex                  nextIndexHint(const QModelIndex& indexToAnchor, const QItemSelectionRange& removed) const;

    void setItemDelegate(ImageDelegate* delegate);
    void indexActivated(const QModelIndex& index, Qt::KeyboardModifiers modifiers);
    void currentChanged(const QModelIndex& index, const QModelIndex& previous);
    void paintEvent(QPaintEvent* e);
    void selectionChanged(const QItemSelection&, const QItemSelection&);
    void updateGeometries();

    /// Reimplement these in a subclass
    virtual void activated(const ImageInfo& info, Qt::KeyboardModifiers modifiers);
    virtual void showContextMenuOnInfo(QContextMenuEvent* event, const ImageInfo& info);
    virtual void showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index);

    // Adds group members when appropriate
    ImageInfoList resolveGrouping(const QModelIndexList& indexes) const;
    ImageInfoList resolveGrouping(const ImageInfoList& infos) const;
    bool          needGroupResolving(ApplicationSettings::OperationType type,
                                     const QList<QModelIndex>& indexes) const;
    bool          needGroupResolving(ApplicationSettings::OperationType type,
                                     const ImageInfoList& infos) const;

private Q_SLOTS:

    void slotIccSettingsChanged(const ICCSettingsContainer&, const ICCSettingsContainer&);
    void slotFileChanged(const QString& filePath);
    void slotDelayedEnter();

private:

    void scrollToStoredItem();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* IMAGECATEGORIZEDVIEW_H */
