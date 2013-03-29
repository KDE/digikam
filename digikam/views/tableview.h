/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-11
 * Description : Table view
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
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

// KDE includes

#include "kcategorizedsortfilterproxymodel.h"
#include "kdialog.h"

// local includes

/// @todo clean up includes and use forward-declarations where possible
#include "digikam_export.h"
#include "imageinfo.h"
#include "statesavingobject.h"

class KMenu;
class QContextMenuEvent;
class QItemDelegate;
class QItemSelectionModel;
class QTreeView;

namespace Digikam
{

class Album;
class ThumbnailSize;
class TableViewShared;

class TableView : public QWidget, public StateSavingObject
{
    Q_OBJECT

public:

    explicit TableView(
            QItemSelectionModel* const selectionModel,
            KCategorizedSortFilterProxyModel* const imageFilterModel,
            QWidget* const parent
        );
    virtual ~TableView();

    void setThumbnailSize(const ThumbnailSize& size);
    ThumbnailSize getThumbnailSize() const;
    QList<qlonglong> selectedImageIdsCurrentFirst() const;
    QList<ImageInfo> selectedImageInfos() const;
    ImageInfo currentInfo();

protected:

    void doLoadState();
    void doSaveState();

    virtual bool eventFilter(QObject* watched, QEvent* event);
    void showTreeViewContextMenuOnItem(QContextMenuEvent* const event, const QModelIndex& indexAtMenu);
    void showTreeViewContextMenuOnEmptyArea(QContextMenuEvent* const event);
    Album* currentAlbum();

protected Q_SLOTS:

    void slotItemActivated(const QModelIndex& tableViewIndex);
    void slotAssignTagToSelected(const int tagID);
    void slotRemoveTagFromSelected(const int tagID);
    void slotAssignPickLabelToSelected(const int pickLabelID);
    void slotAssignColorLabelToSelected(const int colorLabelID);
    void slotAssignRatingToSelected(const int rating);
    void slotInsertSelectedToExistingQueue(const int queueId);
    void slotSetAsAlbumThumbnail(const ImageInfo& info);
    void slotPaste();

Q_SIGNALS:

    void signalPreviewRequested(const ImageInfo& info);
    void signalZoomInStep();
    void signalZoomOutStep();
    void signalPopupTagsView();
    void signalGotoTagAndImageRequested(const int tagId);
    void signalGotoAlbumAndImageRequested(const ImageInfo& info);
    void signalGotoDateAndImageRequested(const ImageInfo& info);

private:

    class Private;
    const QScopedPointer<Private> d;
    const QScopedPointer<TableViewShared> s;
};

} /* namespace Digikam */

#endif // TABLEVIEW_H
