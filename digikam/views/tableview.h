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

#include <QTreeView>
#include <QWidget>
#include <QItemDelegate>

// KDE includes

#include "kcategorizedsortfilterproxymodel.h"
#include "kdialog.h"

// local includes

/// @todo clean up includes and use forward-declarations where possible
#include "statesavingobject.h"
#include "digikam_export.h"
#include "imagealbummodel.h"
#include "thumbnailloadthread.h"
#include "imagefiltermodel.h"
#include "tableview_columnfactory.h"
#include "tableview_shared.h"

class KMenu;
class QContextMenuEvent;

namespace Digikam
{

class ThumbnailSize;

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

protected:

    void doLoadState();
    void doSaveState();

    virtual bool eventFilter(QObject* watched, QEvent* event);
    void showTreeViewContextMenu(QContextMenuEvent* const event);
    QList<ImageInfo> selectedImageInfos() const;

protected Q_SLOTS:

    void slotItemActivated(const QModelIndex& sortedIndex);
    void slotAssignTagToSelected(const int tagID);
    void slotRemoveTagFromSelected(const int tagID);
    void slotAssignPickLabelToSelected(const int pickLabelID);
    void slotAssignColorLabelToSelected(const int colorLabelID);
    void slotAssignRatingToSelected(const int rating);


Q_SIGNALS:

    void signalPreviewRequested(const ImageInfo& info);
    void signalZoomInStep();
    void signalZoomOutStep();

private:

    class Private;
    const QScopedPointer<Private> d;
    const QScopedPointer<TableViewShared> s;
};

} /* namespace Digikam */

#endif // TABLEVIEW_H
