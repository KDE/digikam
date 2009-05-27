/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-29
 * Description : Qt item view for images - delegate additions
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imagedelegateoverlay.h"
#include "imagedelegateoverlay.moc"

// Qt includes

#include <QEvent>
#include <QMouseEvent>

// KDE includes

#include <kdebug.h>

// Local includes

#include "imagecategorizedview.h"
#include "imagedelegate.h"
#include "itemviewhoverbutton.h"


namespace Digikam
{

ImageDelegateOverlay::ImageDelegateOverlay(QObject *parent)
                    : QObject(parent)
{
}

ImageDelegateOverlay::~ImageDelegateOverlay()
{
    if (m_delegate)
        m_delegate->removeOverlay(this);
}

void ImageDelegateOverlay::setActive(bool)
{
}

void ImageDelegateOverlay::visualChange()
{
}

void ImageDelegateOverlay::mouseMoved(QMouseEvent *, const QRect&, const QModelIndex&)
{
}

void ImageDelegateOverlay::paint(QPainter *, const QStyleOptionViewItem&, const QModelIndex&)
{
}

void ImageDelegateOverlay::setView(ImageCategorizedView *view)
{
    if (!view && m_view)
    {
        disconnect(m_view, 0, this, 0);
        disconnect(this, 0, m_view, 0);
    }

    m_view = view;

    if (m_view)
    {
        connect(this, SIGNAL(update(const QModelIndex &)),
                m_view, SLOT(update(const QModelIndex &)));
    }
}

ImageCategorizedView *ImageDelegateOverlay::view() const
{
    return m_view;
}

void ImageDelegateOverlay::setDelegate(ImageDelegate *delegate)
{
    if (!delegate && m_delegate)
    {
        disconnect(m_delegate, 0, this, 0);
        disconnect(this, 0, m_delegate, 0);
    }

    m_delegate = delegate;

    if (m_delegate)
    {
        connect(m_delegate, SIGNAL(visualChange()),
                this, SLOT(visualChange()));
    }
}

ImageDelegate *ImageDelegateOverlay::delegate() const
{
    return m_delegate;
}

// -----------------------------

AbstractWidgetDelegateOverlay::AbstractWidgetDelegateOverlay(QObject *parent)
                          : ImageDelegateOverlay(parent),
                            m_widget(0),
                            m_mouseButtonPressedOnWidget(false)
{
}

void AbstractWidgetDelegateOverlay::setActive(bool active)
{
    if (active)
    {
        if (m_widget)
            delete m_widget;
        m_widget = createWidget();

        m_widget->setFocusPolicy(Qt::NoFocus);
        m_widget->hide();

        m_view->viewport()->installEventFilter(this);
        m_widget->installEventFilter(this);

        connect(m_view->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                this, SLOT(slotRowsRemoved(const QModelIndex&, int, int)));

        connect(m_view->model(), SIGNAL(modelReset()),
                this, SLOT(slotReset()));

        connect(m_view, SIGNAL(entered(const QModelIndex &)),
                this, SLOT(slotEntered(const QModelIndex &)));

        connect(m_view, SIGNAL(viewportEntered()),
                this, SLOT(slotViewportEntered()));
    }
    else
    {
        delete m_widget;
        m_widget = 0;
        m_view->viewport()->removeEventFilter(this);
        disconnect(m_view->model(), 0, this, 0);
        disconnect(m_view, 0, this, 0);
    }
}

QWidget *AbstractWidgetDelegateOverlay::parentWidget() const
{
    return m_view->viewport();
}

void AbstractWidgetDelegateOverlay::slotReset()
{
}

void AbstractWidgetDelegateOverlay::slotEntered(const QModelIndex& index)
{
    m_widget->hide();
    if (index.isValid())
        m_widget->show();
}

void AbstractWidgetDelegateOverlay::slotViewportEntered()
{
    m_widget->hide();
}

void AbstractWidgetDelegateOverlay::slotRowsRemoved(const QModelIndex &, int, int)
{
    m_widget->hide();
}

bool AbstractWidgetDelegateOverlay::eventFilter(QObject* obj, QEvent* event)
{
    // events on view's viewport
    if (obj == m_widget->parent()) {
        switch (event->type()) {
            case QEvent::Leave:
                m_widget->hide();
                break;

            case QEvent::MouseMove:
                if (m_mouseButtonPressedOnWidget) {
                    // Don't forward mouse move events to the viewport,
                    // otherwise a rubberband selection will be shown when
                    // clicking on the selection toggle and moving the mouse
                    // above the viewport.
                    return true;
                }
                break;
            case QEvent::MouseButtonRelease:
                m_mouseButtonPressedOnWidget = false;
                break;
            default:
                break;
        }
    }
    else if (obj == m_widget)
    {
        switch (event->type()) {
            case QEvent::MouseButtonPress:
                if (static_cast<QMouseEvent*>(event)->buttons() & Qt::LeftButton)
                    m_mouseButtonPressedOnWidget = true;
                break;
            case QEvent::MouseButtonRelease:
                m_mouseButtonPressedOnWidget = false;
                break;
            default:
                break;
        }
    }

    return ImageDelegateOverlay::eventFilter(obj, event);
}

// -----------------------------

HoverButtonDelegateOverlay::HoverButtonDelegateOverlay(QObject *parent)
                          : AbstractWidgetDelegateOverlay(parent)
{
}

ItemViewHoverButton *HoverButtonDelegateOverlay::button() const
{
    return static_cast<ItemViewHoverButton*>(m_widget);
}

void HoverButtonDelegateOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);
    if (active)
        button()->initIcon();
}

QWidget *HoverButtonDelegateOverlay::createWidget()
{
    return createButton();
}

void HoverButtonDelegateOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
        updateButton(button()->index());
}

void HoverButtonDelegateOverlay::slotReset()
{
    AbstractWidgetDelegateOverlay::slotReset();

    button()->reset();
}

void HoverButtonDelegateOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);

    if (index.isValid())
    {
        button()->setIndex(index);
        updateButton(index);
    }
    else
    {
        button()->setIndex(index);
    }
}

} // namespace Digikam
