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

#include "toolslistview.h"

// Qt includes

#include <QDrag>
#include <QHeaderView>
#include <QMap>
#include <QMimeData>
#include <QPainter>
#include <QPixmap>
#include <QAction>
#include <QMenu>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

ToolListViewGroup::ToolListViewGroup(QTreeWidget* const parent, BatchTool::BatchToolGroup group)
    : QTreeWidgetItem(parent)
{
    setFlags(Qt::ItemIsEnabled);

    setExpanded(true);
    setDisabled(false);

    m_group = group;

    switch (m_group)
    {
        case BatchTool::ColorTool:
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18n("Color"));
            break;

        case BatchTool::EnhanceTool:
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18n("Enhance"));
            break;

        case BatchTool::TransformTool:
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18n("Transform"));
            break;

        case BatchTool::DecorateTool:
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18n("Decorate"));
            break;

        case BatchTool::FiltersTool:
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18n("Filters"));
            break;

        case BatchTool::ConvertTool:
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18n("Convert"));
            break;

        case BatchTool::MetadataTool:
            setIcon(0, QIcon::fromTheme(QLatin1String("digikam")));
            setText(0, i18n("Metadata"));
            break;

        default:      // User customized tools.
            setIcon(0, QIcon::fromTheme(QLatin1String("user-properties")));
            setText(0, i18n("Custom Tools"));
            break;
    }
}

ToolListViewGroup::~ToolListViewGroup()
{
}

BatchTool::BatchToolGroup ToolListViewGroup::toolGroup() const
{
    return m_group;
}

// ---------------------------------------------------------------------------

ToolListViewItem::ToolListViewItem(ToolListViewGroup* const parent, BatchTool* const tool)
    : QTreeWidgetItem(parent)
{
    setDisabled(false);
    setSelected(false);

    m_tool = tool;

    if (m_tool)
    {
        setIcon(0, QIcon::fromTheme(m_tool->toolIconName()));
        setText(0, m_tool->toolTitle());
        setText(1, m_tool->toolDescription());
    }
}

ToolListViewItem::~ToolListViewItem()
{
}

BatchTool* ToolListViewItem::tool() const
{
    return m_tool;
}

// ---------------------------------------------------------------------------

ToolsListView::ToolsListView(QWidget* const parent)
    : QTreeWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setIconSize(QSize(22, 22));
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSortingEnabled(false);
    setAllColumnsShowFocus(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(2);
    setHeaderHidden(true);
    setDragEnabled(true);
    header()->setSectionResizeMode(QHeaderView::Stretch);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotContextMenu()));
}

ToolsListView::~ToolsListView()
{
}

BatchToolsList ToolsListView::toolsList()
{
    BatchToolsList list;

    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        ToolListViewItem* const item = dynamic_cast<ToolListViewItem*>(*it);

        if (item)
        {
            list.append(item->tool());
        }

        ++it;
    }

    return list;
}

void ToolsListView::addTool(BatchTool* const tool)
{
    if (!tool)
    {
        return;
    }

    ToolListViewGroup* const parent = findToolGroup(tool->toolGroup());

    if (parent)
    {
        new ToolListViewItem(parent, tool);
    }
}

bool ToolsListView::removeTool(BatchTool* const tool)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        ToolListViewItem* const item = dynamic_cast<ToolListViewItem*>(*it);

        if (item && item->tool() == tool)
        {
            delete item;
            return true;
        }

        ++it;
    }

    return false;
}

ToolListViewGroup* ToolsListView::findToolGroup(BatchTool::BatchToolGroup group)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        ToolListViewGroup* const item = dynamic_cast<ToolListViewGroup*>(*it);

        if (item && item->toolGroup() == group)
        {
            return item;
        }

        ++it;
    }

    return 0;
}

bool ToolsListView::findTool(BatchTool* tool)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        ToolListViewItem* const item = dynamic_cast<ToolListViewItem*>(*it);

        if (item && item->tool() == tool)
        {
            return true;
        }

        ++it;
    }

    return false;
}

void ToolsListView::startDrag(Qt::DropActions /*supportedActions*/)
{
    QList<QTreeWidgetItem*> items = selectedItems();

    if (items.isEmpty())
    {
        return;
    }

    QPixmap icon(QIcon::fromTheme(QLatin1String("system-run")).pixmap(48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w + 4, h + 4);
    QString text(QString::number(items.count()));

    QPainter p(&pix);
    p.fillRect(0, 0, pix.width() - 1, pix.height() - 1, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
    p.drawPixmap(2, 2, icon);
    QRect r = p.boundingRect(2, 2, w, h, Qt::AlignLeft | Qt::AlignTop, text);
    r.setWidth(qMax(r.width(), r.height()));
    r.setHeight(qMax(r.width(), r.height()));
    p.fillRect(r, QColor(0, 80, 0));
    p.setPen(Qt::white);
    QFont f(font());
    f.setBold(true);
    p.setFont(f);
    p.drawText(r, Qt::AlignCenter, text);
    p.end();

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData(items));
    drag->setPixmap(pix);
    drag->exec();
}

QStringList ToolsListView::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("digikam/batchtoolslist");
    return types;
}

void ToolsListView::mouseDoubleClickEvent(QMouseEvent*)
{
    if (viewport()->isEnabled())
    {
        slotAssignTools();
    }
}

void ToolsListView::slotAssignTools()
{
    QList<QTreeWidgetItem*> items = selectedItems();

    if (items.isEmpty())
    {
        return;
    }

    QMap<int, QString> map = itemsToMap(items);
    emit signalAssignTools(map);
}

QMimeData* ToolsListView::mimeData(const QList<QTreeWidgetItem*> items) const
{
    QMimeData* const mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QMap<int, QString> map    = itemsToMap(items);
    stream << map;

    mimeData->setData(QLatin1String("digikam/batchtoolslist"), encodedData);
    return mimeData;
}

QMap<int, QString> ToolsListView::itemsToMap(const QList<QTreeWidgetItem*> items) const
{
    QMap<int, QString> map;

    foreach(QTreeWidgetItem* const itm, items)
    {
        ToolListViewItem* const tlwi = dynamic_cast<ToolListViewItem*>(itm);

        if (tlwi)
        {
            map.insertMulti((int)(tlwi->tool()->toolGroup()), tlwi->tool()->objectName());
        }
    }
    return map;
}

void ToolsListView::slotContextMenu()
{
    QMenu popmenu(this);
    QAction* const action = new QAction(QIcon::fromTheme(QLatin1String("list-add")), i18n("Assign tools"), this);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotAssignTools()));

    popmenu.addAction(action);
    popmenu.exec(QCursor::pos());
}

}  // namespace Digikam
