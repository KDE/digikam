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

#include "imagedelegateoverlay.moc"

// Qt includes

#include <QEvent>
#include <QMouseEvent>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "itemviewimagedelegate.h"
#include "itemviewhoverbutton.h"

namespace Digikam
{


ImageDelegateOverlay::ImageDelegateOverlay(QObject* parent)
    : QObject(parent), m_view(0), m_delegate(0)
{
}

ImageDelegateOverlay::~ImageDelegateOverlay()
{
}

void ImageDelegateOverlay::setActive(bool)
{
}

void ImageDelegateOverlay::visualChange()
{
}

void ImageDelegateOverlay::mouseMoved(QMouseEvent*, const QRect&, const QModelIndex&)
{
}

void ImageDelegateOverlay::paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&)
{
}

void ImageDelegateOverlay::setView(QAbstractItemView* view)
{
    if (m_view)
    {
        disconnect(this, SIGNAL(update(const QModelIndex&)),
                   m_view, SLOT(update(const QModelIndex&)));
    }

    m_view = view;

    if (m_view)
    {
        connect(this, SIGNAL(update(const QModelIndex&)),
                m_view, SLOT(update(const QModelIndex&)));
    }
}

QAbstractItemView* ImageDelegateOverlay::view() const
{
    return m_view;
}

void ImageDelegateOverlay::setDelegate(QAbstractItemDelegate* delegate)
{
    if (m_delegate)
    {
        disconnect(m_delegate, SIGNAL(visualChange()),
                   this, SLOT(visualChange()));
    }

    m_delegate = delegate;

    if (m_delegate)
    {
        connect(m_delegate, SIGNAL(visualChange()),
                this, SLOT(visualChange()));
    }
}

QAbstractItemDelegate* ImageDelegateOverlay::delegate() const
{
    return m_delegate;
}

bool ImageDelegateOverlay::affectsMultiple(const QModelIndex& index) const
{
    // note how selectionModel->selectedIndexes().contains() can scale badly
    QItemSelectionModel *selectionModel = view()->selectionModel();
    if (!selectionModel->hasSelection())
    {
        return false;
    }
    if (!selectionModel->isSelected(index))
    {
        return false;
    }
    QItemSelection selection = selectionModel->selection();
    if (selection.size() > 1)
    {
        return true;
    }
    return selection.indexes().size() > 1;
}

QList<QModelIndex> ImageDelegateOverlay::affectedIndexes(const QModelIndex& index) const
{
    if (!affectsMultiple(index))
    {
        return QList<QModelIndex>() << index;
    }
    else
    {
        return view()->selectionModel()->selectedIndexes();
    }
}

int ImageDelegateOverlay::numberOfAffectedIndexes(const QModelIndex& index) const
{
    if (!affectsMultiple(index))
    {
        return 1;
    }

    // scales better than selectedIndexes().count()
    int count = 0;
    foreach (const QItemSelectionRange& range, view()->selectionModel()->selection())
    {
        count += range.height();
    }
    return count;
}

// -----------------------------

AbstractWidgetDelegateOverlay::AbstractWidgetDelegateOverlay(QObject* parent)
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
        {
            delete m_widget;
            m_widget = 0;
        }

        m_widget = createWidget();

        m_widget->setFocusPolicy(Qt::NoFocus);
        m_widget->hide(); // hide per default

        m_view->viewport()->installEventFilter(this);
        m_widget->installEventFilter(this);

        if (view()->model())
        {
            connect(m_view->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                    this, SLOT(slotRowsRemoved(const QModelIndex&, int, int)));

            connect(m_view->model(), SIGNAL(layoutChanged()),
                    this, SLOT(slotLayoutChanged()));

            connect(m_view->model(), SIGNAL(modelReset()),
                    this, SLOT(slotReset()));
        }

        connect(m_view, SIGNAL(entered(const QModelIndex&)),
                this, SLOT(slotEntered(const QModelIndex&)));

        connect(m_view, SIGNAL(viewportEntered()),
                this, SLOT(slotViewportEntered()));
    }
    else
    {
        delete m_widget;
        m_widget = 0;

        if (m_view)
        {
            m_view->viewport()->removeEventFilter(this);

            if (view()->model())
            {
                disconnect(m_view->model(), 0, this, 0);
            }

            disconnect(m_view, SIGNAL(entered(const QModelIndex&)),
                       this, SLOT(slotEntered(const QModelIndex&)));

            disconnect(m_view, SIGNAL(viewportEntered()),
                       this, SLOT(slotViewportEntered()));
        }
    }
}

void AbstractWidgetDelegateOverlay::hide()
{
    if (m_widget)
    {
        m_widget->hide();
    }
}

QWidget* AbstractWidgetDelegateOverlay::parentWidget() const
{
    return m_view->viewport();
}

void AbstractWidgetDelegateOverlay::slotReset()
{
    hide();
}

void AbstractWidgetDelegateOverlay::slotEntered(const QModelIndex& index)
{
    hide();

    if (index.isValid() && checkIndex(index))
    {
        m_widget->show();
    }
}

bool AbstractWidgetDelegateOverlay::checkIndex(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return true;
}

void AbstractWidgetDelegateOverlay::slotViewportEntered()
{
    hide();
}

void AbstractWidgetDelegateOverlay::slotRowsRemoved(const QModelIndex&, int, int)
{
    hide();
}

void AbstractWidgetDelegateOverlay::slotLayoutChanged()
{
    hide();
}

void AbstractWidgetDelegateOverlay::viewportLeaveEvent(QObject*, QEvent*)
{
    hide();
}

void AbstractWidgetDelegateOverlay::widgetEnterEvent()
{
}

void AbstractWidgetDelegateOverlay::widgetLeaveEvent()
{
}

void AbstractWidgetDelegateOverlay::widgetEnterNotifyMultiple(const QModelIndex& index)
{
    if (index.isValid() && affectsMultiple(index))
    {
        emit requestNotification(index, notifyMultipleMessage(index, numberOfAffectedIndexes(index)));
    }
}

void AbstractWidgetDelegateOverlay::widgetLeaveNotifyMultiple()
{
    emit hideNotification();
}

QString AbstractWidgetDelegateOverlay::notifyMultipleMessage(const QModelIndex&, int number)
{
    return i18nc("@info", "<i>Applying operation on <br/><b>%1</b> selected pictures</i>", number);
}

bool AbstractWidgetDelegateOverlay::eventFilter(QObject* obj, QEvent* event)
{
    if (m_widget && obj == m_widget->parent())   // events on view's viewport
    {
        switch (event->type())
        {
            case QEvent::Leave:
                viewportLeaveEvent(obj, event);
                break;

            case QEvent::MouseMove:

                if (m_mouseButtonPressedOnWidget)
                {
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
        switch (event->type())
        {
            case QEvent::MouseButtonPress:

                if (static_cast<QMouseEvent*>(event)->buttons() & Qt::LeftButton)
                {
                    m_mouseButtonPressedOnWidget = true;
                }

                break;
            case QEvent::MouseButtonRelease:
                m_mouseButtonPressedOnWidget = false;
                break;
            case QEvent::Enter:
                widgetEnterEvent();
                break;
            case QEvent::Leave:
                widgetLeaveEvent();
                break;
            default:
                break;
        }
    }

    return ImageDelegateOverlay::eventFilter(obj, event);
}

// -----------------------------

HoverButtonDelegateOverlay::HoverButtonDelegateOverlay(QObject* parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

ItemViewHoverButton* HoverButtonDelegateOverlay::button() const
{
    return static_cast<ItemViewHoverButton*>(m_widget);
}

void HoverButtonDelegateOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        button()->initIcon();
    }
}

QWidget* HoverButtonDelegateOverlay::createWidget()
{
    return createButton();
}

void HoverButtonDelegateOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updateButton(button()->index());
    }
}

void HoverButtonDelegateOverlay::slotReset()
{
    AbstractWidgetDelegateOverlay::slotReset();

    button()->reset();
}

void HoverButtonDelegateOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);

    if (index.isValid() && checkIndex(index))
    {
        button()->setIndex(index);
        updateButton(index);
    }
    else
    {
        button()->setIndex(index);
    }
}

// -----------------------------

ImageDelegateOverlayContainer::~ImageDelegateOverlayContainer()
{
}

void ImageDelegateOverlayContainer::installOverlay(ImageDelegateOverlay* overlay)
{
    if (!overlay->acceptsDelegate(asDelegate()))
    {
        kError() << "Cannot accept delegate" << asDelegate() << "for installing" << overlay;
        return;
    }

    overlay->setDelegate(asDelegate());
    m_overlays << overlay;
    // let the view call setActive

    QObject::connect(overlay, SIGNAL(destroyed(QObject*)),
                     asDelegate(), SLOT(overlayDestroyed(QObject*)));

    QObject::connect(overlay, SIGNAL(requestNotification(const QModelIndex&, const QString&)),
                     asDelegate(), SIGNAL(requestNotification(const QModelIndex&, const QString&)));

    QObject::connect(overlay, SIGNAL(hideNotification()),
                     asDelegate(), SIGNAL(hideNotification()));
}

QList<ImageDelegateOverlay*> ImageDelegateOverlayContainer::overlays() const
{
    return m_overlays;
}

void ImageDelegateOverlayContainer::removeOverlay(ImageDelegateOverlay* overlay)
{
    overlay->setActive(false);
    overlay->setDelegate(0);
    m_overlays.removeAll(overlay);
    QObject::disconnect(overlay, 0, asDelegate(), 0);
}

void ImageDelegateOverlayContainer::setAllOverlaysActive(bool active)
{
    foreach (ImageDelegateOverlay* overlay, m_overlays)
    {
        overlay->setActive(active);
    }
}

void ImageDelegateOverlayContainer::setViewOnAllOverlays(QAbstractItemView* view)
{
    foreach (ImageDelegateOverlay* overlay, m_overlays)
    {
        overlay->setView(view);
    }
}

void ImageDelegateOverlayContainer::removeAllOverlays()
{
    foreach (ImageDelegateOverlay* overlay, m_overlays)
    {
        overlay->setActive(false);
        overlay->setDelegate(0);
        overlay->setView(0);
    }
    m_overlays.clear();
}

void ImageDelegateOverlayContainer::overlayDestroyed(QObject* o)
{
    ImageDelegateOverlay* overlay = qobject_cast<ImageDelegateOverlay*>(o);
    if (overlay)
    {
        removeOverlay(overlay);
    }
}

void ImageDelegateOverlayContainer::mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index)
{
    foreach (ImageDelegateOverlay* overlay, m_overlays)
    {
        overlay->mouseMoved(e, visualRect, index);
    }
}

void ImageDelegateOverlayContainer::drawOverlays(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    foreach (ImageDelegateOverlay* overlay, m_overlays)
    {
        overlay->paint(p, option, index);
    }
}


} // namespace Digikam
