/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager items list.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef QUEUE_LIST_H
#define QUEUE_LIST_H

// Qt includes.

#include <QTreeWidget>
#include <QWidget>
#include <QPixmap>
#include <QIcon>

// Local includes.

#include "batchtool.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "loadingdescription.h"
#include "queuesettingsview.h"

namespace Digikam
{

class QueueListViewItemPriv;
class QueueListPriv;

class QueueListViewItem : public QTreeWidgetItem
{

public:

    QueueListViewItem(QTreeWidget *view, const ImageInfo& info);
    ~QueueListViewItem();

    void setInfo(const ImageInfo& info);
    ImageInfo info() const;

    void assignTool(int index, const BatchToolSet& set);
    void unassignTool(int index);

    void setThumb(const QPixmap& pix);

    void setProgressIcon(const QIcon& icon);

    void setDone(bool b);
    bool isDone();

private:

    QueueListViewItemPriv* const d;
};

// -------------------------------------------------------------------------

class QueueListViewPriv;

class QueueListView : public QTreeWidget
{
    Q_OBJECT

public:

    QueueListView(QWidget *parent);
    ~QueueListView();

    int  itemsCount();
    void removeItemByInfo(const ImageInfo& info);

    ImageInfoList pendingItemsList();
    int           pendingItemsCount();
    int           pendingTasksCount();

    QueueListViewItem* findItemByUrl(const KUrl& url);

    void setSettings(const QueueSettings& settings);
    QueueSettings settings();

    void setAssignedTools(const AssignedBatchTools& tools);
    AssignedBatchTools assignedTools();

signals:

    void signalQueueContentsChanged();

public slots:

    void slotClearList();
    void slotRemoveSelectedItems();
    void slotRemoveItemsDone();
    void slotAddItems(const ImageInfoList&, const ImageInfo&);
    void slotAssignedToolsChanged(const AssignedBatchTools&);

private slots:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    bool findItemByInfo(const ImageInfo& info);
    void removeItems(int removeType);

    QMimeData* mimeData(const QList<QTreeWidgetItem*> items) const;
    void startDrag(Qt::DropActions supportedActions);
    Qt::DropActions supportedDropActions() const;
    void dragMoveEvent(QDragMoveEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);

private:

    QueueListViewPriv* const d;
};

}  // namespace Digikam

#endif // QUEUE_LIST_H
