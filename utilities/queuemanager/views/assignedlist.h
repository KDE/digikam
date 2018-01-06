/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-27
 * Description : batch tools list assigned to an queued item.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ASSIGNED_LIST_H
#define ASSIGNED_LIST_H

// Qt includes

#include <QTreeWidget>
#include <QWidget>
#include <QIcon>

// Local includes

#include "batchtool.h"
#include "batchtoolutils.h"

namespace Digikam
{

class QueueSettings;

class AssignedListViewItem : public QTreeWidgetItem
{

public:

    AssignedListViewItem(QTreeWidget* const parent);
    AssignedListViewItem(QTreeWidget* const parent, QTreeWidgetItem* const preceding);
    virtual ~AssignedListViewItem();

    void setIndex(int index);

    void setToolSet(const BatchToolSet& set);
    BatchToolSet toolSet();

private:

    BatchToolSet m_set;
};

// -------------------------------------------------------------------------

class AssignedListView : public QTreeWidget
{
    Q_OBJECT

public:

    explicit AssignedListView(QWidget* const parent);
    ~AssignedListView();

    int                assignedCount();
    AssignedBatchTools assignedList();

    AssignedListViewItem* insertTool(AssignedListViewItem* const preceding, const BatchToolSet& set);
    AssignedListViewItem* moveTool(AssignedListViewItem* const preceding, const BatchToolSet& set);
    AssignedListViewItem* addTool(const BatchToolSet& set);

    bool removeTool(const BatchToolSet& set);

    void setBusy(bool b);

Q_SIGNALS:

    void signalToolSelected(const BatchToolSet&);
    void signalAssignedToolsChanged(const AssignedBatchTools&);

public Q_SLOTS:

    void slotMoveCurrentToolUp();
    void slotMoveCurrentToolDown();
    void slotRemoveCurrentTool();
    void slotClearToolsList();
    void slotQueueSelected(int, const QueueSettings&, const AssignedBatchTools&);
    void slotSettingsChanged(const BatchToolSet&);
    void slotAssignTools(const QMap<int, QString>&);

protected:

    void keyPressEvent(QKeyEvent*);

private Q_SLOTS:

    void slotSelectionChanged();
    void slotContextMenu();

private:

    AssignedListViewItem* findTool(const BatchToolSet& set);
    void assignTools(const QMap<int, QString>& map, AssignedListViewItem* const preceding);
    void refreshIndex();

    Qt::DropActions supportedDropActions()                        const;
    QStringList     mimeTypes()                                   const;
    QMimeData*      mimeData(const QList<QTreeWidgetItem*> items) const;

    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dropEvent(QDropEvent*);
};

}  // namespace Digikam

#endif // ASSIGNED_LIST_H
