/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager items list.
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

#define ICONSIZE 32

#include "assignedlist.h"

// Qt includes

#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QPainter>
#include <QUrl>
#include <QMimeData>
#include <QAction>
#include <QMenu>

// KDE includes

#include <kactioncollection.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "queuemgrwindow.h"
#include "queuesettingsview.h"
#include "toolslistview.h"
#include "batchtoolsmanager.h"

namespace Digikam
{

AssignedListViewItem::AssignedListViewItem(QTreeWidget* const parent)
    : QTreeWidgetItem(parent)
{
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | flags());
}

AssignedListViewItem::AssignedListViewItem(QTreeWidget* const parent, QTreeWidgetItem* const preceding)
    : QTreeWidgetItem(parent, preceding)
{
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | flags());
}

AssignedListViewItem::~AssignedListViewItem()
{
}

void AssignedListViewItem::setToolSet(const BatchToolSet& set)
{
    m_set = set;
    setIndex(m_set.index);

    BatchTool* const tool = BatchToolsManager::instance()->findTool(m_set.name, m_set.group);

    if (tool)
    {
        setIcon(1, QIcon::fromTheme(tool->toolIconName()));
        setText(1, tool->toolTitle());
    }
}

BatchToolSet AssignedListViewItem::toolSet()
{
    return m_set;
}

void AssignedListViewItem::setIndex(int index)
{
    m_set.index = index;
    setText(0, QString::fromUtf8("%1").arg(m_set.index + 1));
}

// ---------------------------------------------------------------------------

AssignedListView::AssignedListView(QWidget* const parent)
    : QTreeWidget(parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setWhatsThis(i18n("This is the list of batch tools assigned."));
    setIconSize(QSize(ICONSIZE, ICONSIZE));

    setDragEnabled(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setSortingEnabled(false);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(2);
    setHeaderHidden(true);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(this, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotContextMenu()));
}

AssignedListView::~AssignedListView()
{
}

void AssignedListView::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Delete)
    {
        slotRemoveCurrentTool();
    }
    else
    {
        QTreeWidget::keyPressEvent(e);
    }
}

void AssignedListView::setBusy(bool b)
{
    viewport()->setEnabled(!b);
}

AssignedBatchTools AssignedListView::assignedList()
{
    BatchSetList            list;
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(*it);

        if (item)
        {
            list.append(item->toolSet());
        }

        ++it;
    }

    AssignedBatchTools tools;
    tools.m_toolsList = list;
    return tools;
}

int AssignedListView::assignedCount()
{
    return assignedList().m_toolsList.count();
}

void AssignedListView::slotRemoveCurrentTool()
{
    AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(currentItem());

    if (item)
    {
        delete item;
        refreshIndex();
        emit signalAssignedToolsChanged(assignedList());
    }

    if (assignedCount() == 0)
    {
        emit signalToolSelected(BatchToolSet());
    }
}

void AssignedListView::slotClearToolsList()
{
    clear();
    emit signalAssignedToolsChanged(assignedList());
    emit signalToolSelected(BatchToolSet());
}

void AssignedListView::slotMoveCurrentToolUp()
{
    AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(currentItem());

    if (item)
    {
        AssignedListViewItem* const iabove = dynamic_cast<AssignedListViewItem*>(itemAbove(item));

        if (iabove)
        {
            moveTool(item, iabove->toolSet());
            setCurrentItem(item);
        }
    }
}

void AssignedListView::slotMoveCurrentToolDown()
{
    AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(currentItem());

    if (item)
    {
        AssignedListViewItem* const ibelow = dynamic_cast<AssignedListViewItem*>(itemBelow(item));

        if (ibelow)
        {
            AssignedListViewItem* const nitem = moveTool(ibelow, item->toolSet());
            setCurrentItem(nitem);
        }
    }
}

AssignedListViewItem* AssignedListView::moveTool(AssignedListViewItem* const preceding, const BatchToolSet& set)
{
    BatchTool* const tool = BatchToolsManager::instance()->findTool(set.name, set.group);

    if (!tool)
    {
        return 0;
    }

    removeTool(set);
    AssignedListViewItem* const item = insertTool(preceding, set);
    refreshIndex();

    emit signalAssignedToolsChanged(assignedList());

    return item;
}

AssignedListViewItem* AssignedListView::insertTool(AssignedListViewItem* const preceding, const BatchToolSet& set)
{
    AssignedListViewItem* item = 0;

    if (preceding)
    {
        item = new AssignedListViewItem(this, preceding);
    }
    else
    {
        item = new AssignedListViewItem(this);
    }

    item->setToolSet(set);
    refreshIndex();

    emit signalAssignedToolsChanged(assignedList());

    return item;
}

AssignedListViewItem* AssignedListView::addTool(const BatchToolSet& set)
{
    return insertTool(0, set);
}

bool AssignedListView::removeTool(const BatchToolSet& set)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(*it);

        if (item && item->toolSet() == set)
        {
            delete item;
            refreshIndex();
            return true;
        }

        ++it;
    }

    return false;
}

AssignedListViewItem* AssignedListView::findTool(const BatchToolSet& set)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(*it);

        if (item && item->toolSet() == set)
        {
            return item;
        }

        ++it;
    }

    return 0;
}

Qt::DropActions AssignedListView::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList AssignedListView::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("digikam/assignedbatchtool");
    return types;
}

QMimeData* AssignedListView::mimeData(const QList<QTreeWidgetItem*> items) const
{
    QMimeData* const mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << items.count();

    foreach(QTreeWidgetItem* const itm, items)
    {
        AssignedListViewItem* const alwi = dynamic_cast<AssignedListViewItem*>(itm);

        if (alwi)
        {
            stream << (int)(alwi->toolSet().group);
            stream << alwi->toolSet().name;
            stream << alwi->toolSet().index;
            stream << alwi->toolSet().version;
            stream << alwi->toolSet().settings;
        }
    }

    mimeData->setData(QLatin1String("digikam/assignedbatchtool"), encodedData);
    return mimeData;
}

void AssignedListView::dragEnterEvent(QDragEnterEvent* e)
{
    QTreeWidget::dragEnterEvent(e);
    e->accept();
}

void AssignedListView::dragMoveEvent(QDragMoveEvent* e)
{
    if (e->mimeData()->formats().contains(QLatin1String("digikam/batchtoolslist")) ||
        e->mimeData()->formats().contains(QLatin1String("digikam/assignedbatchtool")))
    {
        QTreeWidget::dragMoveEvent(e);
        e->accept();
        return;
    }

    e->ignore();
}

void AssignedListView::dropEvent(QDropEvent* e)
{
    if (e->mimeData()->formats().contains(QLatin1String("digikam/batchtoolslist")))
    {
        QByteArray ba = e->mimeData()->data(QLatin1String("digikam/batchtoolslist"));

        if (ba.size())
        {
            QDataStream ds(ba);
            QMap<int, QString> map;
            ds >> map;

            AssignedListViewItem* const preceding = dynamic_cast<AssignedListViewItem*>(itemAt(e->pos()));
            assignTools(map, preceding);
        }

        e->acceptProposedAction();
    }
    else if (e->mimeData()->formats().contains(QLatin1String("digikam/assignedbatchtool")))
    {
        QByteArray ba = e->mimeData()->data(QLatin1String("digikam/assignedbatchtool"));

        if (ba.size())
        {
            int count;
            QDataStream ds(ba);
            ds >> count;

            for (int i = 0 ; i < count ; ++i)
            {
                int               group, index, version;
                QString           name;
                BatchToolSettings settings;

                ds >> group;
                ds >> name;
                ds >> index;
                ds >> version;
                ds >> settings;

                AssignedListViewItem* const preceding = dynamic_cast<AssignedListViewItem*>(itemAt(e->pos()));

                BatchToolSet set;
                set.name                         = name;
                set.group                        = (BatchTool::BatchToolGroup)group;
                set.index                        = index;
                set.version                      = version;
                set.settings                     = settings;
                AssignedListViewItem* const item = moveTool(preceding, set);
                setCurrentItem(item);
            }
        }

        e->acceptProposedAction();
    }
    else
    {
        e->ignore();
    }
}

void AssignedListView::slotSelectionChanged()
{
    QList<QTreeWidgetItem*> list = selectedItems();

    if (list.isEmpty())
    {
        return;
    }

    AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(list.first());

    if (item)
    {
        BatchToolSet set = item->toolSet();
        emit signalToolSelected(set);
    }
    else
    {
        emit signalToolSelected(BatchToolSet());
    }
}

void AssignedListView::slotQueueSelected(int, const QueueSettings&, const AssignedBatchTools& tools)
{
    // Clear assigned tools list and tool settings view.
    clear();
    emit signalToolSelected(BatchToolSet());

    if (!tools.m_toolsList.isEmpty())
    {
        blockSignals(true);

        foreach (const BatchToolSet& set, tools.m_toolsList)
        {
            addTool(set);
        }

        blockSignals(false);
    }
}

void AssignedListView::slotSettingsChanged(const BatchToolSet& set)
{
    AssignedListViewItem* const item = findTool(set);

    if (item)
    {
        item->setToolSet(set);
        emit signalAssignedToolsChanged(assignedList());
    }
}

void AssignedListView::slotAssignTools(const QMap<int, QString>& map)
{
    if (map.isEmpty())
    {
        return;
    }

    assignTools(map, 0);
}

void AssignedListView::assignTools(const QMap<int, QString>& map, AssignedListViewItem* const preceding)
{
    // We pop all items in reverse order to have same order than selection from Batch Tools list.
    QMapIterator<int, QString> it(map);
    it.toBack();

    while (it.hasPrevious())
    {
        it.previous();
        BatchTool::BatchToolGroup group  = (BatchTool::BatchToolGroup)(it.key());
        QString name                     = it.value();
        BatchTool* const tool            = BatchToolsManager::instance()->findTool(name, group);
        BatchToolSet set;
        set.name                         = tool->objectName();
        set.group                        = tool->toolGroup();
        set.version                      = tool->toolVersion();
        set.settings                     = tool->defaultSettings();
        AssignedListViewItem* const item = insertTool(preceding, set);
        setCurrentItem(item);
    }
}

void AssignedListView::slotContextMenu()
{
    if (!viewport()->isEnabled())
    {
        return;
    }

    KActionCollection* const acol = QueueMgrWindow::queueManagerWindow()->actionCollection();
    QMenu popmenu(this);
    popmenu.addAction(acol->action(QLatin1String("queuemgr_toolup")));
    popmenu.addAction(acol->action(QLatin1String("queuemgr_tooldown")));
    popmenu.addAction(acol->action(QLatin1String("queuemgr_toolremove")));
    popmenu.addSeparator();
    popmenu.addAction(acol->action(QLatin1String("queuemgr_savequeue")));
    popmenu.addAction(acol->action(QLatin1String("queuemgr_toolsclear")));
    popmenu.exec(QCursor::pos());
}

void AssignedListView::refreshIndex()
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(*it);

        if (item)
        {
            item->setIndex(indexOfTopLevelItem(item));
        }

        ++it;
    }
}

}  // namespace Digikam
