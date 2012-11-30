/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager items list.
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "assignedlist.moc"

// Qt includes

#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QPainter>
#include <QUrl>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>

// Local includes

#include "queuemgrwindow.h"
#include "queuesettingsview.h"
#include "toolslistview.h"

namespace Digikam
{

AssignedListViewItem::AssignedListViewItem(QTreeWidget* const parent, const BatchToolSet& set)
    : QTreeWidgetItem(parent)
{
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | flags());
    setToolSet(set);
}

AssignedListViewItem::AssignedListViewItem(QTreeWidget* const parent, QTreeWidgetItem* const preceding, const BatchToolSet& set)
    : QTreeWidgetItem(parent, preceding)
{
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | flags());
    setToolSet(set);
}

AssignedListViewItem::~AssignedListViewItem()
{
}

void AssignedListViewItem::setToolSet(const BatchToolSet& set)
{
    m_set = set;

    if (m_set.tool)
    {
        setIcon(0, SmallIcon(m_set.tool->toolIconName()));
        setText(0, m_set.tool->toolTitle());
    }
}

BatchToolSet AssignedListViewItem::toolSet() const
{
    return m_set;
}

// ---------------------------------------------------------------------------

AssignedListView::AssignedListView(QWidget* const parent)
    : QTreeWidget(parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setWhatsThis(i18n("This is the list of batch tools assigned."));

    setDragEnabled(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setSortingEnabled(false);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(1);
    setHeaderHidden(true);
    header()->setResizeMode(QHeaderView::Stretch);

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
    BatchToolMap map;
    int index = 1;

    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(*it);

        if (item)
        {
            map.insert(index, item->toolSet());
        }

        ++index;
        ++it;
    }

    AssignedBatchTools tools;
    tools.m_toolsMap = map;
    return tools;
}

int AssignedListView::assignedCount()
{
    return assignedList().m_toolsMap.count();
}

void AssignedListView::slotRemoveCurrentTool()
{
    AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(currentItem());

    if (item)
    {
        delete item;
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
    if (!set.tool)
    {
        return 0;
    }

    removeTool(set);
    AssignedListViewItem* const item = insertTool(preceding, set);

    emit signalAssignedToolsChanged(assignedList());

    return item;
}

AssignedListViewItem* AssignedListView::insertTool(AssignedListViewItem* const preceding, const BatchToolSet& set)
{
    if (!set.tool)
    {
        return 0;
    }

    if (findTool(set))
    {
        return 0;
    }

    AssignedListViewItem* item = 0;

    if (preceding)
    {
        item = new AssignedListViewItem(this, preceding, set);
    }
    else
    {
        item = new AssignedListViewItem(this, set);
    }

    emit signalAssignedToolsChanged(assignedList());

    return item;
}

AssignedListViewItem* AssignedListView::addTool(int /*index*/, const BatchToolSet& set)
{
    if (!set.tool)
    {
        return 0;
    }

    if (findTool(set))
    {
        return 0;
    }

    AssignedListViewItem* const item = new AssignedListViewItem(this, set);

    emit signalAssignedToolsChanged(assignedList());

    return item;
}

bool AssignedListView::removeTool(const BatchToolSet& set)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(*it);

        if (item->toolSet().tool == set.tool)
        {
            delete item;
            return true;
        }

        ++it;
    }

    return false;
}

AssignedListViewItem* AssignedListView::findTool(int index)
{
    int count = 1;
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(*it);

        if (count == index)
        {
            return item;
        }

        ++count;
        ++it;
    }

    return 0;
}

AssignedListViewItem* AssignedListView::findTool(const BatchToolSet& set)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        AssignedListViewItem* const item = dynamic_cast<AssignedListViewItem*>(*it);

        if (item->toolSet().tool == set.tool)
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
    types << "digikam/assignedbatchtool";
    return types;
}

QMimeData* AssignedListView::mimeData(const QList<QTreeWidgetItem*> items) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << items.count();
    foreach(QTreeWidgetItem* const itm, items)
    {
        AssignedListViewItem* const alwi = dynamic_cast<AssignedListViewItem*>(itm);

        if (alwi)
        {
            stream << (int)(alwi->toolSet().tool->toolGroup());
            stream << alwi->toolSet().tool->objectName();
            stream << alwi->toolSet().settings;
        }
    }

    mimeData->setData("digikam/assignedbatchtool", encodedData);
    return mimeData;
}

void AssignedListView::dragEnterEvent(QDragEnterEvent* e)
{
    QTreeWidget::dragEnterEvent(e);
    e->accept();
}

void AssignedListView::dragMoveEvent(QDragMoveEvent* e)
{
    if (e->mimeData()->formats().contains("digikam/batchtoolslist") ||
        e->mimeData()->formats().contains("digikam/assignedbatchtool"))
    {
        QTreeWidget::dragMoveEvent(e);
        e->accept();
        return;
    }

    e->ignore();
}

void AssignedListView::dropEvent(QDropEvent* e)
{
    if (e->mimeData()->formats().contains("digikam/batchtoolslist"))
    {
        QByteArray ba = e->mimeData()->data("digikam/batchtoolslist");

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
    else if (e->mimeData()->formats().contains("digikam/assignedbatchtool"))
    {
        QByteArray ba = e->mimeData()->data("digikam/assignedbatchtool");

        if (ba.size())
        {
            int count;
            QDataStream ds(ba);
            ds >> count;

            for (int i = 0 ; i < count ; ++i)
            {
                int               group;
                QString           name;
                BatchToolSettings settings;

                ds >> group;
                ds >> name;
                ds >> settings;

                BatchTool* const tool = QueueMgrWindow::queueManagerWindow()->batchToolsManager()
                                            ->findTool(name, (BatchTool::BatchToolGroup)group);
                tool->ensureIsInitialized();

                AssignedListViewItem* const preceding = dynamic_cast<AssignedListViewItem*>(itemAt(e->pos()));

                BatchToolSet set;
                set.tool                   = tool;
                set.settings               = settings;
                AssignedListViewItem* item = moveTool(preceding, set);
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

    AssignedListViewItem* item = dynamic_cast<AssignedListViewItem*>(list.first());

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

    if (!tools.m_toolsMap.isEmpty())
    {
        blockSignals(true);

        for (BatchToolMap::const_iterator it = tools.m_toolsMap.constBegin() ; it != tools.m_toolsMap.constEnd() ; ++it)
        {
            addTool(it.key(), it.value());
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
        BatchTool::BatchToolGroup group = (BatchTool::BatchToolGroup)(it.key());
        QString name                    = it.value();
        BatchTool* const tool           = QueueMgrWindow::queueManagerWindow()->batchToolsManager()->findTool(name, group);
        tool->ensureIsInitialized();
        BatchToolSet set;
        set.tool                        = tool;
        set.settings                    = tool->defaultSettings();
        AssignedListViewItem* item      = insertTool(preceding, set);
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
    KMenu popmenu(this);
    popmenu.addAction(acol->action("queuemgr_toolup"));
    popmenu.addAction(acol->action("queuemgr_tooldown"));
    popmenu.addAction(acol->action("queuemgr_toolremove"));
    popmenu.addSeparator();
    popmenu.addAction(acol->action("queuemgr_toolsclear"));
    popmenu.exec(QCursor::pos());
}

}  // namespace Digikam
