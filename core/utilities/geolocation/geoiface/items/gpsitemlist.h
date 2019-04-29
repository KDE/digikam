/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : A view to display a list of items with GPS info.
 *
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef DIGIKAM_GPS_ITEM_LIST_H
#define DIGIKAM_GPS_ITEM_LIST_H

// Qt includes

#include <QTreeView>

// Local includes

#include "gpsitemmodel.h"
#include "gpsitemsortproxymodel.h"
#include "digikam_export.h"

class QWheelEvent;
class KConfigGroup;

namespace Digikam
{

class ItemListDragDropHandler;

class DIGIKAM_EXPORT GPSItemList : public QTreeView
{
    Q_OBJECT

public:

    explicit GPSItemList(QWidget* const parent = nullptr);
    ~GPSItemList();

    void setModelAndSelectionModel(GPSItemModel* const model, QItemSelectionModel* const selectionModel);
    GPSItemModel* getModel() const;
    QItemSelectionModel* getSelectionModel() const;
    void setDragDropHandler(ItemListDragDropHandler* const dragDropHandler);
    void setThumbnailSize(const int size);
    GPSItemSortProxyModel* getSortProxyModel() const;

    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);
    void setEditEnabled(const bool state);
    void setDragEnabled(const bool state);

Q_SIGNALS:

    void signalImageActivated(const QModelIndex& index);

public Q_SLOTS:

    void slotIncreaseThumbnailSize();
    void slotDecreaseThumbnailSize();
    void slotUpdateActionsEnabled();

private Q_SLOTS:

    void slotThumbnailFromModel(const QPersistentModelIndex& index, const QPixmap& pixmap);
    void slotInternalTreeViewImageActivated(const QModelIndex& index);
    void slotColumnVisibilityActionTriggered(QAction* action);

protected:

    virtual bool eventFilter(QObject* watched, QEvent* event);
    virtual void startDrag(Qt::DropActions supportedActions);
    virtual void wheelEvent(QWheelEvent* we);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_GPS_ITEM_LIST_H
