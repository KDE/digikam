/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-26
 * Description : images versions QTreeView
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef VERSIONS_TREEVIEW_H
#define VERSIONS_TREEVIEW_H

// Qt includes

#include <QTreeView>

// Local includes

#include "digikam_export.h"
#include "dragdropimplementations.h"

namespace Digikam
{

class VersionsDelegate;
class ImageDelegateOverlay;

class VersionsTreeView : public QTreeView, public DragDropViewImplementation
{
    Q_OBJECT

public:

    explicit VersionsTreeView(QWidget* const parent = 0);
    ~VersionsTreeView();

    void setToolTipEnabled(bool on);

    void addOverlay(ImageDelegateOverlay* overlay);
    void removeOverlay(ImageDelegateOverlay* overlay);

    VersionsDelegate* delegate() const;

protected:

    virtual void paintEvent(QPaintEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual bool viewportEvent(QEvent* event);
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

    DECLARE_VIEW_DRAG_DROP_METHODS(QTreeView)
    virtual QModelIndex mapIndexForDragDrop(const QModelIndex& index)    const;
    virtual QPixmap     pixmapForDrag(const QList<QModelIndex>& indexes) const;
    virtual AbstractItemDragDropHandler* dragDropHandler()               const;
    virtual void setDragDropHandler(AbstractItemDragDropHandler* handler);

protected:

    class ToolTip;

    VersionsDelegate*            m_delegate;
    AbstractItemDragDropHandler* m_dragDropHandler;
    bool                         m_showToolTip;
    ToolTip*                     m_toolTip;
};

} // namespace Digikam

#endif // VERSIONS_TREEVIEW_H
