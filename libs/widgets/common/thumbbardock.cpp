/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-15-08
 * Description : A floatable/dockable widget for thumbnail bars (ThumbBarView
 *               and its descendants), providing i drag handle similar to the
 *               one on toolbars and a standard KToggleAction to show/hide the
 *               thumbnail bar. It inherits QDockWidget and can be used in
 *               the dock area's of a QMainWindow.
 *
 * Copyright (C) 2009 by Pieter Edelman <p dot edelman at gmx dot net>
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

#include "thumbbardock.moc"

namespace Digikam
{

class DragHandle::DragHandlePriv
{

public:

    DragHandlePriv() :
        parent(0),
        currentArea(Qt::LeftDockWidgetArea)
    {
    }

    QDockWidget*       parent;
    Qt::DockWidgetArea currentArea;
};

DragHandle::DragHandle(QDockWidget* parent)
    : QWidget(), d(new DragHandlePriv)
{
    d->parent = parent;

    setToolTip(i18n("Drag to reposition"));
    setCursor(Qt::PointingHandCursor);

    // When the dock location changes, check if the orientation has changed.
    connect(d->parent, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(dockLocationChanged(Qt::DockWidgetArea)));
}

DragHandle::~DragHandle()
{
    delete d;
}

void DragHandle::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QStyle* style = d->parent->style();

    // The QStyleOptionToolBar contains every parameter needed to draw the
    // handle.
    QStyleOptionToolBar opt;
    opt.initFrom(d->parent);
    opt.features = QStyleOptionToolBar::Movable;

    // If the thumbnail bar is layed out horizontally, the state should be set
    // to horizontal to draw the handle in the proper orientation.
    if (d->currentArea == Qt::LeftDockWidgetArea || d->currentArea == Qt::RightDockWidgetArea)
    {
        opt.rect = QRect(opt.rect.x(), opt.rect.y(),
                         d->parent->width(),
                         style->pixelMetric(QStyle::PM_ToolBarHandleExtent));
    }
    else
    {
        opt.state |= QStyle::State_Horizontal;
        opt.rect  = QRect(opt.rect.x(), opt.rect.y(),
                          style->pixelMetric(QStyle::PM_ToolBarHandleExtent),
                          d->parent->height());
    }

    // Draw the toolbar handle.
    style->drawPrimitive(QStyle::PE_IndicatorToolBarHandle, &opt, &p, this);
}

void DragHandle::dockLocationChanged(Qt::DockWidgetArea area)
{
    d->currentArea = area;

    // When the dock widget that contains this handle changes to a different
    // orientation, the DockWidgetVerticalTitleBar feature needs to be adjusted:
    // present when the thumbbar orientation is horizontal, absent when it is
    // vertical(!)
    if (d->currentArea == Qt::LeftDockWidgetArea || d->currentArea == Qt::RightDockWidgetArea)
    {
        d->parent->setFeatures(d->parent->features() & ~QDockWidget::DockWidgetVerticalTitleBar);
    }
    else
    {
        d->parent->setFeatures(d->parent->features() | QDockWidget::DockWidgetVerticalTitleBar);
    }
}

QSize DragHandle::sizeHint() const
{
    // Size is the sum of the margin, frame width and the handle itself.
    QStyle* style   = d->parent->style();
    int handleWidth = style->pixelMetric(QStyle::PM_ToolBarHandleExtent);
    int margin      = style->pixelMetric(QStyle::PM_ToolBarItemMargin) +
                      style->pixelMetric(QStyle::PM_ToolBarFrameWidth);

    if (d->currentArea == Qt::LeftDockWidgetArea || d->currentArea == Qt::RightDockWidgetArea)
    {
        return QSize(d->parent->width(), handleWidth + 2*margin);
    }
    else
    {
        return QSize(handleWidth + 2*margin, d->parent->height());
    }
}

QSize DragHandle::minimumSizeHint() const
{
    return QSize(0, 0);
}

// ----------------------------------------------------------------------------

ThumbBarDock::ThumbBarDock(QWidget* parent, Qt::WindowFlags flags)
    : QDockWidget(parent, flags), m_visible(SHOULD_BE_SHOWN)
{
    // Use a DragHandle as title bar widget.
    setTitleBarWidget(new DragHandle(this));

    // Detect changes in dock location.
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));
}

ThumbBarDock::~ThumbBarDock()
{
}

void ThumbBarDock::reInitialize()
{
    // Measure orientation of the widget and adjust the child thumbbar to this
    // orientation and size.
    QMainWindow* parent   = qobject_cast<QMainWindow*>(parentWidget());
    emit dockLocationChanged(parent->dockWidgetArea(this));
    //ThumbBarView *child = qobject_cast<ThumbBarView *>(widget());
    widget()->resize(size());
    update();
}

KToggleAction* ThumbBarDock::getToggleAction(QObject* parent, const QString& caption) const
{
    KToggleAction* action = new KToggleAction(KIcon("view-choose"), caption, parent);

    // Default shortcut is Ctrl+T.
    action->setShortcut(KShortcut(Qt::CTRL+Qt::Key_T));

    // Connect the triggered signal, which is only emitted after a user action
    // and not programmatically, to the show/hide method.
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(showThumbBar(bool)));

    // Connect the show/hide signal to the state of the toggle action.
    connect(this, SIGNAL(visibilityChanged(bool)),
            action, SLOT(setChecked(bool)));

    return action;
}

void ThumbBarDock::restoreVisibility()
{
    // Set the visibility to what it should be or to what it was. Reset
    // SHOULD_BE_ values to their WAS_ values, to implement correct behavior
    // on subsequent calls.
    if (m_visible == SHOULD_BE_SHOWN)
    {
        m_visible = WAS_SHOWN;
    }
    else if (m_visible == SHOULD_BE_HIDDEN)
    {
        m_visible = WAS_HIDDEN;
    }

    setVisible(m_visible == WAS_SHOWN);
}

bool ThumbBarDock::shouldBeVisible() const
{
    if ((m_visible == WAS_SHOWN) || (m_visible == SHOULD_BE_SHOWN))
    {
        return true;
    }

    return false;
}

void ThumbBarDock::setShouldBeVisible(bool status)
{
    if (status)
    {
        m_visible = SHOULD_BE_SHOWN;
    }
    else
    {
        m_visible = SHOULD_BE_HIDDEN;
    }
}

void ThumbBarDock::slotDockLocationChanged(Qt::DockWidgetArea area)
{
    // Change orientation of child thumbbar when location has changed.
    ThumbBarView* child = qobject_cast<ThumbBarView*>(widget());

    if (!child)
    {
        return;
    }

    if ((area == Qt::LeftDockWidgetArea) || (area == Qt::RightDockWidgetArea))
    {
        child->setOrientation(Qt::Vertical);
    }
    else
    {
        child->setOrientation(Qt::Horizontal);
    }
}

void ThumbBarDock::showThumbBar(bool status)
{
    if (status)
    {
        m_visible = WAS_SHOWN;
    }
    else
    {
        m_visible = WAS_HIDDEN;
    }

    setVisible(status);
}

} // namespace Digikam
