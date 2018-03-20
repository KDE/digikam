/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Available batch tools list.
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

#ifndef TOOLS_LIST_VIEW_H
#define TOOLS_LIST_VIEW_H

// Qt includes

#include <QTreeWidget>
#include <QWidget>
#include <QPixmap>
#include <QMap>
#include <QString>
#include <QList>

// Local includes

#include "batchtool.h"
#include "batchtoolsmanager.h"

namespace Digikam
{

class ToolListViewGroup : public QTreeWidgetItem
{

public:

    ToolListViewGroup(QTreeWidget* const parent, BatchTool::BatchToolGroup group);
    virtual ~ToolListViewGroup();

    BatchTool::BatchToolGroup toolGroup() const;

private:

    BatchTool::BatchToolGroup  m_group;
};

// -------------------------------------------------------------------------

class ToolListViewItem : public QTreeWidgetItem
{

public:


    ToolListViewItem(ToolListViewGroup* const parent, BatchTool* const tool);
    virtual ~ToolListViewItem();

    BatchTool* tool() const;

private:

    BatchTool* m_tool;
};

// -------------------------------------------------------------------------

class ToolsListView : public QTreeWidget
{
    Q_OBJECT

public:

    explicit ToolsListView(QWidget* const parent);
    virtual ~ToolsListView();

    BatchToolsList toolsList();

    void addTool(BatchTool* const tool);
    bool removeTool(BatchTool* const tool);

Q_SIGNALS:

    void signalAssignTools(const QMap<int, QString>&);

private Q_SLOTS:

    void slotContextMenu();
    void slotAssignTools();

private:

    bool findTool(BatchTool* const tool);
    ToolListViewGroup* findToolGroup(BatchTool::BatchToolGroup group);

    void startDrag(Qt::DropActions supportedActions);
    QStringList mimeTypes() const;
    QMimeData* mimeData(const QList<QTreeWidgetItem*> items) const;

    void mouseDoubleClickEvent(QMouseEvent*);
    QMap<int, QString> itemsToMap(const QList<QTreeWidgetItem*> items) const;
};

}  // namespace Digikam

#endif // TOOLS_LIST_VIEW_H
