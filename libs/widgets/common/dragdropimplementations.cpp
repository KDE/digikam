/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-26
 * Description : Qt Model for Images - drag and drop handling
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dragdropimplementations.h"

// Qt includes

#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QDrag>
#include <QDropEvent>
#include <QMimeData>

namespace Digikam
{

// ------------ Model sample implementation -------------

DragDropModelImplementation::DragDropModelImplementation()
    : m_dragDropHandler(0)
{
}

Qt::ItemFlags DragDropModelImplementation::dragDropFlags(const QModelIndex& index) const
{
    Q_UNUSED(index);

    if (!m_dragDropHandler)
    {
        return 0;
    }

    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::ItemFlags DragDropModelImplementation::dragDropFlagsV2(const QModelIndex& index) const
{
    Qt::ItemFlags flags;

    if (isDragEnabled(index))
    {
        flags |= Qt::ItemIsDragEnabled;
    }

    if (isDropEnabled(index))
    {
        flags |= Qt::ItemIsDropEnabled;
    }

    return flags;
}

bool DragDropModelImplementation::isDragEnabled(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return true;
}

bool DragDropModelImplementation::isDropEnabled(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return true;
}

Qt::DropActions DragDropModelImplementation::supportedDropActions() const
{
    return Qt::CopyAction|Qt::MoveAction;
}

QStringList DragDropModelImplementation::mimeTypes() const
{
    if (m_dragDropHandler)
    {
        return m_dragDropHandler->mimeTypes();
    }

    return QStringList();
}

bool DragDropModelImplementation::dropMimeData(const QMimeData*, Qt::DropAction, int, int, const QModelIndex&)
{
    // we require custom solutions
    return false;
}

QMimeData* DragDropModelImplementation::mimeData(const QModelIndexList& indexes) const
{
    if (!m_dragDropHandler)
    {
        return 0;
    }

    return m_dragDropHandler->createMimeData(indexes);
}

void DragDropModelImplementation::setDragDropHandler(AbstractItemDragDropHandler* handler)
{
    m_dragDropHandler = handler;
}

AbstractItemDragDropHandler* DragDropModelImplementation::dragDropHandler() const
{
    return m_dragDropHandler;
}

// ------------ View sample implementation -------------

void DragDropViewImplementation::cut()
{
    QMimeData* const data = asView()->model()->mimeData(asView()->selectionModel()->selectedIndexes());

    if (data)
    {
        encodeIsCutSelection(data, true);
        qApp->clipboard()->setMimeData(data);
    }
}

void DragDropViewImplementation::copy()
{
    QMimeData* const data = asView()->model()->mimeData(asView()->selectionModel()->selectedIndexes());

    if (data)
    {
        encodeIsCutSelection(data, false);
        qApp->clipboard()->setMimeData(data);
    }
}

void DragDropViewImplementation::paste()
{
    const QMimeData* const data = qApp->clipboard()->mimeData(QClipboard::Clipboard);

    if (!data)
    {
        return;
    }

    // We need to have a real (context menu action) or fake (Ctrl+V shortcut) mouse position
    QPoint eventPos = asView()->mapFromGlobal(QCursor::pos());

    if (!asView()->rect().contains(eventPos))
    {
        eventPos = QPoint(0, 0);
    }

    bool cutAction = decodeIsCutSelection(data);
    QDropEvent event(eventPos,
                     cutAction ? Qt::MoveAction : Qt::CopyAction,
                     data, Qt::NoButton,
                     cutAction ? Qt::ShiftModifier : Qt::ControlModifier);
    QModelIndex index = asView()->indexAt(event.pos());

    if (!dragDropHandler()->accepts(&event, index))
    {
        return;
    }

    dragDropHandler()->dropEvent(asView(), &event, index);
}

void DragDropViewImplementation::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = asView()->selectionModel()->selectedIndexes();

    if (indexes.count() > 0)
    {
        QMimeData* const data = asView()->model()->mimeData(indexes);

        if (!data)
        {
            return;
        }

        QPixmap pixmap    = pixmapForDrag(indexes);
        QDrag* const drag = new QDrag(asView());
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->exec(supportedActions, Qt::CopyAction);
    }
}

void DragDropViewImplementation::dragEnterEvent(QDragEnterEvent* e)
{
    AbstractItemDragDropHandler* const handler = dragDropHandler();

    if (handler && handler->acceptsMimeData(e->mimeData()))
    {
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void DragDropViewImplementation::dragMoveEvent(QDragMoveEvent* e)
{
    // Note: Must call parent view first. This is done by the DECLARE... macro.

    AbstractItemDragDropHandler* const handler = dragDropHandler();

    if (handler)
    {
        QModelIndex index     = asView()->indexAt(e->pos());
        Qt::DropAction action = handler->accepts(e, mapIndexForDragDrop(index));

        if (action == Qt::IgnoreAction)
        {
            e->ignore();
        }
        else
        {
            e->setDropAction(action);
            e->accept();
        }
    }
}

void DragDropViewImplementation::dropEvent(QDropEvent* e)
{
    // Note: Must call parent view first. This is done by the DECLARE... macro.

    AbstractItemDragDropHandler* const handler = dragDropHandler();

    if (handler)
    {
        QModelIndex index = asView()->indexAt(e->pos());

        if (handler->dropEvent(asView(), e, mapIndexForDragDrop(index)))
        {
            e->accept();
        }
    }
}

static const QString mimeTypeCutSelection(QLatin1String("application/x-kde-cutselection"));

void DragDropViewImplementation::encodeIsCutSelection(QMimeData* mime, bool cut)
{
    const QByteArray cutSelection = cut ? "1" : "0";
    mime->setData(mimeTypeCutSelection, cutSelection);
}

bool DragDropViewImplementation::decodeIsCutSelection(const QMimeData* mime)
{
    QByteArray a = mime->data(mimeTypeCutSelection);

    if (a.isEmpty())
    {
        return false;
    }

    return (a.at(0) == '1'); // true if 1
}

} // namespace Digikam
