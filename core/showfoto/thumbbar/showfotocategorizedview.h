/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-13
 * Description : Qt categorized item view for showfoto items
 *
 * Copyright (C) 2013 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef SHOWFOTOCATEGORIZEDVIEW_H
#define SHOWFOTOCATEGORIZEDVIEW_H

// Local includes

#include "itemviewcategorized.h"
#include "showfotoimagemodel.h"
#include "showfotofiltermodel.h"
#include "showfotothumbnailmodel.h"
#include "showfotoiteminfo.h"
#include "imagedelegateoverlay.h"

namespace ShowFoto
{

class ShowfotoDelegate;

class ShowfotoCategorizedView : public ItemViewCategorized
{
public:

    Q_OBJECT

public:

    explicit ShowfotoCategorizedView(QWidget* const parent = 0);
    ~ShowfotoCategorizedView();

    void setModels(ShowfotoImageModel* model, ShowfotoSortFilterModel* filterModel);

    ShowfotoImageModel*         showfotoImageModel()                    const;
    ShowfotoSortFilterModel*    showfotoSortFilterModel()               const;

    QItemSelectionModel*        getSelectionModel()                     const;

    /// Returns any ShowfotoFilterModel in chain. May not be sourceModel()
    ShowfotoFilterModel*        showfotoFilterModel()                   const;

    /// Returns 0 if the ShowfotoImageModel is not an ShowfotoThumbnailModel
    ShowfotoThumbnailModel*     showfotoThumbnailModel()                const;

    ShowfotoDelegate*           delegate()                              const;

    ShowfotoItemInfo            currentInfo()                           const;
    QUrl                        currentUrl()                            const;

    QList<ShowfotoItemInfo>     selectedShowfotoItemInfos()             const;
    QList<ShowfotoItemInfo>     selectedShowfotoItemInfosCurrentFirst() const;
    QList<QUrl>                 selectedUrls()                          const;

    QList<ShowfotoItemInfo>     showfotoItemInfos()                     const;
    QList<QUrl>                 urls()                                  const;

    /** Selects the index as current and scrolls to it
     */
    void toIndex(const QUrl& url);

    /** Returns the n-th info after the given one.
     *  Specifically, return the previous info for nth = -1
     *  and the next info for n = 1.
     *  Returns a null info if either startingPoint or the nth info are
     *  not contained in the model
     */
    ShowfotoItemInfo nextInOrder(const ShowfotoItemInfo& startingPoint, int nth);

    ShowfotoItemInfo previousInfo(const ShowfotoItemInfo& info)
    {
        return nextInOrder(info, -1);
    }

    ShowfotoItemInfo nextInfo(const ShowfotoItemInfo& info)
    {
        return nextInOrder(info, 1);
    }

    /// Add and remove an overlay. It will as well be removed automatically when destroyed.
    /// Unless you pass a different delegate, the current delegate will be used.
    void addOverlay(ImageDelegateOverlay* overlay, ShowfotoDelegate* delegate = 0);
    void removeOverlay(ImageDelegateOverlay* overlay);

    //TODO: Implement This
//    void addSelectionOverlay(ShowfotoDelegate* delegate = 0);

    ThumbnailSize thumbnailSize() const;

    virtual void setThumbnailSize(const ThumbnailSize& size);

public Q_SLOTS:

    void setThumbnailSize(int size);

    /** Scroll the view to the given item when it becomes available
     */
    void setCurrentWhenAvailable(qlonglong ShowfotoItemId);

    /** Set as current item the item identified by its file url
     */
    void setCurrentUrl(const QUrl& url);

    /** Set as current item the item identified by the ShowfotoItemInfo
     */
    void setCurrentInfo(const ShowfotoItemInfo& info);

    /** Set selected items identified by their file urls
     */
    void setSelectedUrls(const QList<QUrl>& urlList);

    /** Set selected items
     */
    void setSelectedShowfotoItemInfos(const QList<ShowfotoItemInfo>& infos);

    /** Does something to gain attention for info, but not changing current selection
     */
    void hintAt(const ShowfotoItemInfo& info);

Q_SIGNALS:

    void currentChanged(const ShowfotoItemInfo& info);

    /// Emitted when new items are selected. The parameter includes only the newly selected infos,
    /// there may be other already selected infos.
    void selected(const QList<ShowfotoItemInfo>& newSelectedInfos);

    /// Emitted when items are deselected. There may be other selected infos left.
    /// This signal is not emitted when the model is reset; then only selectionCleared is emitted.
    void deselected(const QList<ShowfotoItemInfo>& nowDeselectedInfos);

    /// Emitted when the given ShowfotoItemInfo is activated. Info is never null.
    void showfotoItemInfoActivated(const ShowfotoItemInfo& info);

    /// Emitted when a new model is set
    void modelChanged();

protected:

    // reimplemented from parent class
    QSortFilterProxyModel*       filterModel()     const;
    AbstractItemDragDropHandler* dragDropHandler() const;
    QModelIndex                  nextIndexHint(const QModelIndex& indexToAnchor, const QItemSelectionRange& removed) const;

    void setItemDelegate(ShowfotoDelegate* delegate);
    void indexActivated(const QModelIndex& index, Qt::KeyboardModifiers modifiers);
    void currentChanged(const QModelIndex& index, const QModelIndex& previous);
    void paintEvent(QPaintEvent* e);
    void selectionChanged(const QItemSelection&, const QItemSelection&);
    void updateGeometries();

    /// Reimplement these in a subclass
    virtual void activated(const ShowfotoItemInfo& info, Qt::KeyboardModifiers modifiers);
    virtual void showContextMenuOnInfo(QContextMenuEvent* event, const ShowfotoItemInfo& info);
    virtual void showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index);

private Q_SLOTS:

    void slotFileChanged(const QString& filePath);
    void slotDelayedEnter();

private:

    void scrollToStoredItem();

private:

    class Private;
    Private* const d;
};

} // namespace Showfoto

#endif // SHOWFOTOCATEGORIZEDVIEW_H
