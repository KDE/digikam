/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-11
 * Description : Table view
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2017 by Simon Frei <freisim93 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

// Qt includes

#include <QWidget>

// Local includes

#include "applicationsettings.h"
#include "dcategorizedsortfilterproxymodel.h"
#include "digikam_export.h"
#include "imageviewutilities.h"
#include "imageinfo.h"
#include "statesavingobject.h"

class QMenu;
class QContextMenuEvent;
class QItemDelegate;
class QItemSelectionModel;
class QTreeView;

namespace Digikam
{

class Album;
class ImageFilterModel;
class ThumbnailSize;
class TableViewShared;

class TableView : public QWidget, public StateSavingObject
{
    Q_OBJECT

public:

    explicit TableView(QItemSelectionModel* const selectionModel,
                       DCategorizedSortFilterProxyModel* const imageFilterModel,
                       QWidget* const parent);

    virtual ~TableView();

    void setThumbnailSize(const ThumbnailSize& size);
    ThumbnailSize getThumbnailSize()                                     const;
    ImageInfo currentInfo()                                              const;
    Album* currentAlbum()                                                const;
    ImageInfoList allInfo(bool grouping = false)                         const;
    QList<QUrl> allUrls(bool grouping = false)                           const;
    int numberOfSelectedItems()                                          const;
    ImageInfo nextInfo()                                                 const;
    ImageInfo previousInfo()                                             const;
    ImageInfo deepRowImageInfo(const int rowNumber, const bool relative) const;

    void selectAll();
    void clearSelection();
    void invertSelection();

    QModelIndexList  selectedIndexesCurrentFirst()                               const;
    ImageInfoList    selectedImageInfos(bool grouping = false)                   const;
    ImageInfoList    selectedImageInfos(ApplicationSettings::OperationType type) const;
    ImageInfoList    selectedImageInfosCurrentFirst(bool grouping = false)       const;
    QList<qlonglong> selectedImageIdsCurrentFirst(bool grouping = false)         const;
    QList<QUrl>      selectedUrls(bool grouping = false)                         const;

    bool             needGroupResolving(ApplicationSettings::OperationType type,
                                        bool all = false) const;

protected:

    void doLoadState();
    void doSaveState();

    virtual bool eventFilter(QObject* watched, QEvent* event);
    QList<QAction*> getExtraGroupingActions();

    // Adds group members when appropriate
    ImageInfoList resolveGrouping(const QList<QModelIndex>& indexes) const;
    ImageInfoList resolveGrouping(const ImageInfoList& infos) const;

public Q_SLOTS:

    void slotGoToRow(const int rowNumber, const bool relativeMove);
    void slotSetCurrentWhenAvailable(const qlonglong id);
    void slotAwayFromSelection();
    void slotDeleteSelected(const ImageViewUtilities::DeleteMode deleteMode = ImageViewUtilities::DeleteUseTrash);
    void slotDeleteSelectedWithoutConfirmation(const ImageViewUtilities::DeleteMode deleteMode = ImageViewUtilities::DeleteUseTrash);
    void slotSetActive(const bool isActive);
    void slotPaste();
    void rename();

protected Q_SLOTS:

    void slotItemActivated(const QModelIndex& tableViewIndex);
    void slotGroupingModeActionTriggered();

Q_SIGNALS:

    void signalPreviewRequested(const ImageInfo& info);
    void signalZoomInStep();
    void signalZoomOutStep();
    void signalPopupTagsView();
    void signalItemsChanged();
    void signalInsertSelectedToExistingQueue(int queue);
    void signalShowContextMenu(QContextMenuEvent* event,
                               const QList<QAction*>& actions);
    void signalShowContextMenuOnInfo(QContextMenuEvent* event, const ImageInfo& info,
                                     const QList<QAction*>& actions,
                                     ImageFilterModel* filterModel = 0);

private:

    class Private;
    const QScopedPointer<Private> d;
    const QScopedPointer<TableViewShared> s;
};

} // namespace Digikam

#endif // TABLEVIEW_H
