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

#include "versionstreeview.h"

// Qt includes

#include <QPaintEvent>

// Local includes

#include "digikam_debug.h"
#include "dimagehistory.h"
#include "imagehistorygraphmodel.h"
#include "versionsdelegate.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "imagelistmodel.h"
#include "ditemdelegate.h"
#include "itemviewtooltip.h"
#include "tooltipfiller.h"

namespace Digikam
{

class VersionsTreeView::ToolTip : public ItemViewToolTip
{
public:

    enum Mode
    {
        InvalidMode,
        ImageMode,
        FilterActionMode
    };

public:

    explicit ToolTip(QAbstractItemView* const view)
        : ItemViewToolTip(view),
          m_mode(InvalidMode)
    {
    }

    void show(const QStyleOptionViewItem& option, const QModelIndex& index, Mode mode)
    {
        m_mode = mode;
        ItemViewToolTip::show(option, index);
        m_mode = InvalidMode;
    }

protected:

    virtual QString tipContents()
    {
        switch (m_mode)
        {
            default:
            case InvalidMode:
                return QString();
            case ImageMode:
            {
                ImageInfo info = ImageModel::retrieveImageInfo(currentIndex());
                return ToolTipFiller::imageInfoTipContents(info);
            }
            case FilterActionMode:
            {
                FilterAction action = currentIndex().data(ImageHistoryGraphModel::FilterActionRole).value<FilterAction>();
                return ToolTipFiller::filterActionTipContents(action);
            }
        }
    }

protected:

    Mode m_mode;
};

// --------------------------------------------------------------------------------------------------------------------------

VersionsTreeView::VersionsTreeView(QWidget* const parent)
    : QTreeView(parent),
      m_delegate(0),
      m_dragDropHandler(0),
      m_showToolTip(false),
      m_toolTip(0)
{
    m_delegate = new VersionsDelegate(this);
    setItemDelegate(m_delegate);
    m_delegate->setViewOnAllOverlays(this);
    setMouseTracking(true);
}

// All overlay management code in a sophisticated form can be studied in ImageCategorizedView
VersionsTreeView::~VersionsTreeView()
{
    m_delegate->removeAllOverlays();
}

VersionsDelegate* VersionsTreeView::delegate() const
{
    return m_delegate;
}

void VersionsTreeView::addOverlay(ImageDelegateOverlay* overlay)
{
    m_delegate->installOverlay(overlay);
    overlay->setView(this);
    overlay->setActive(true);
}

void VersionsTreeView::removeOverlay(ImageDelegateOverlay* overlay)
{
    m_delegate->removeOverlay(overlay);
    overlay->setView(0);
}

void VersionsTreeView::setToolTipEnabled(bool on)
{
    if (on == m_showToolTip)
        return;

    m_showToolTip = on;

    if (m_showToolTip && !m_toolTip)
    {
        m_toolTip = new ToolTip(this);
    }
}

void VersionsTreeView::paintEvent(QPaintEvent* e)
{
    static_cast<VersionsDelegate*>(itemDelegate())->beginPainting();
    QTreeView::paintEvent(e);
    static_cast<VersionsDelegate*>(itemDelegate())->finishPainting();
}

QModelIndex VersionsTreeView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    // TODO: Need to find a solution to skip non-vertex items in CombinedTreeMode. Not easy.
    return QTreeView::moveCursor(cursorAction, modifiers);
}

void VersionsTreeView::mouseMoveEvent(QMouseEvent* event)
{
    QTreeView::mouseMoveEvent(event);

    QModelIndex index = indexAt(event->pos());
    QRect indexVisualRect;

    if (index.isValid())
    {
        indexVisualRect = visualRect(index);
    }

    m_delegate->mouseMoved(event, indexVisualRect, index);
}

void VersionsTreeView::setDragDropHandler(AbstractItemDragDropHandler* handler)
{
    m_dragDropHandler = handler;
}

AbstractItemDragDropHandler* VersionsTreeView::dragDropHandler() const
{
    return m_dragDropHandler;
}

QModelIndex VersionsTreeView::mapIndexForDragDrop(const QModelIndex& index) const
{
    return index; // no filter model involved
}

QPixmap VersionsTreeView::pixmapForDrag(const QList<QModelIndex>& indexes) const
{
    QStyleOptionViewItem option = viewOptions();
    option.rect                 = viewport()->rect();
    QPixmap pix;

    if (indexes.count() == 1)
    {
        pix = indexes.first().data(Qt::DecorationRole).value<QPixmap>();
    }

    return DItemDelegate::makeDragPixmap(option, indexes, pix);
}

bool VersionsTreeView::viewportEvent(QEvent* event)
{
    switch (event->type())
    {
        case QEvent::ToolTip:
        {
            if (!m_showToolTip)
            {
                break;
            }

            QHelpEvent* he          = static_cast<QHelpEvent*>(event);
            const QModelIndex index = indexAt(he->pos());

            if (!index.isValid())
            {
                break;
            }

            ToolTip::Mode mode;

            if (index.data(ImageHistoryGraphModel::IsImageItemRole).toBool())
            {
                mode = ToolTip::ImageMode;
            }
            else if (index.data(ImageHistoryGraphModel::IsFilterActionItemRole).toBool())
            {
                mode = ToolTip::FilterActionMode;
            }
            else
            {
                break;
            }

            QStyleOptionViewItem option = viewOptions();
            option.rect                 = visualRect(index);
            option.state               |= (index == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);

            m_toolTip->show(option, index, mode);

            return true;
        }
        default:
            break;
    }

    return QTreeView::viewportEvent(event);
}

} // namespace Digikam
