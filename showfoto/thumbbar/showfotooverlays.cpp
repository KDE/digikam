/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-08
 * Description : Overlays for the showfoto
 *
 * Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "showfotooverlays.h"

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kglobalsettings.h>

// Local includes

#include "showfotocategorizedview.h"
#include "showfotodelegate.h"
#include "showfotoiteminfo.h"
#include "ratingwidget.h"

namespace ShowFoto
{

ShowfotoOverlayWidget::ShowfotoOverlayWidget(QWidget* const parent)
    : QAbstractButton(parent)
{
}

void ShowfotoOverlayWidget::paintEvent(QPaintEvent*)
{
}

// -- Lock Overlay ------------------------------------------------------------------

ShowfotoLockOverlay::ShowfotoLockOverlay(QObject* const parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

ShowfotoOverlayWidget* ShowfotoLockOverlay::buttonWidget() const
{
    return static_cast<ShowfotoOverlayWidget*>(m_widget);
}

QWidget* ShowfotoLockOverlay::createWidget()
{
    QAbstractButton* const button = new ShowfotoOverlayWidget(parentWidget());
    //button->setCursor(Qt::PointingHandCursor);
    return button;
}

void ShowfotoLockOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);
}

void ShowfotoLockOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void ShowfotoLockOverlay::updatePosition()
{
    if (!m_index.isValid())
    {
        return;
    }

    QRect rect       = static_cast<ShowfotoDelegate*>(delegate())->lockIndicatorRect();
    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

bool ShowfotoLockOverlay::checkIndex(const QModelIndex& index) const
{
    ShowfotoItemInfo info = ShowfotoImageModel::retrieveShowfotoItemInfo(index);

    //TODO: create write permission vars if needed

//    if (info.writePermissions == 0)
//    {
//        m_widget->setToolTip(i18nc("@info:tooltip", "This item is locked."));
//        return true;
//    }

//    if (info.writePermissions == 1)
//    {
//        m_widget->setToolTip(i18nc("@info:tooltip", "This item is not locked."));
//        return true;
//    }

    return false;
}

void ShowfotoLockOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);
    m_index = index;
    updatePosition();
}

// -- Rating Overlay ------------------------------------------------------------------

ShowfotoRatingOverlay::ShowfotoRatingOverlay(QObject* parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

RatingWidget* ShowfotoRatingOverlay::ratingWidget() const
{
    return static_cast<RatingWidget*>(m_widget);
}

QWidget* ShowfotoRatingOverlay::createWidget()
{
    const bool animate    = KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects;
    RatingWidget* const w = new RatingWidget(parentWidget());
    w->setFading(animate);
    w->setTracking(false);
    return w;
}

void ShowfotoRatingOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        connect(ratingWidget(), SIGNAL(signalRatingChanged(int)),
                this, SLOT(slotRatingChanged(int)));

        if (view()->model())
        {
            connect(view()->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                    this, SLOT(slotDataChanged(QModelIndex,QModelIndex)));
        }
    }
    else
    {
        // widget is deleted

        if (view() && view()->model())
        {
            disconnect(view()->model(), 0, this, 0);
        }
    }
}

void ShowfotoRatingOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void ShowfotoRatingOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(m_index);
}

void ShowfotoRatingOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

void ShowfotoRatingOverlay::hide()
{
    delegate()->setRatingEdited(QModelIndex());
    AbstractWidgetDelegateOverlay::hide();
}

void ShowfotoRatingOverlay::updatePosition()
{
    if (!m_index.isValid() || !m_widget)
    {
        return;
    }

    QRect rect = delegate()->ratingRect();

    if (rect.width() > ratingWidget()->maximumVisibleWidth())
    {
        int offset = (rect.width() - ratingWidget()->maximumVisibleWidth()) / 2;
        rect.adjust(offset, 0, -offset, 0);
    }

    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

void ShowfotoRatingOverlay::updateRating()
{
    if (!m_index.isValid() || !m_widget)
    {
        return;
    }

    ShowfotoImageModel* const model = m_index.data(ShowfotoImageModel::ShowfotoImageModelPointerRole).value<ShowfotoImageModel*>();

    ratingWidget()->setRating(model->showfotoItemInfoRef(m_index).rating);
}

void ShowfotoRatingOverlay::slotRatingChanged(int rating)
{
    if (m_widget && m_widget->isVisible() && m_index.isValid())
    {
        emit ratingEdited(affectedIndexes(m_index), rating);
    }
}

void ShowfotoRatingOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);

    // see bug 228810, this is a small workaround
    if (m_widget && m_widget->isVisible() && m_index.isValid() && index == m_index)
    {
        ratingWidget()->setVisibleImmediately();
    }

    m_index = index;

    updatePosition();
    updateRating();

    delegate()->setRatingEdited(m_index);
    view()->update(m_index);
}

void ShowfotoRatingOverlay::slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    if (m_widget && m_widget->isVisible() && QItemSelectionRange(topLeft, bottomRight).contains(m_index))
    {
        updateRating();
    }
}

// -- Rotate Overlay ----------------------------------------------------------------

ShowfotoRotateOverlayButton::ShowfotoRotateOverlayButton(ShowfotoRotateOverlayDirection dir, QAbstractItemView* const parentView)
    : ItemViewHoverButton(parentView),
      m_direction(dir)
{
}

QSize ShowfotoRotateOverlayButton::sizeHint() const
{
    return QSize(16, 16);
}

QPixmap ShowfotoRotateOverlayButton::icon()
{
    if (m_direction == ShowfotoRotateOverlayLeft)
    {
        return KIconLoader::global()->loadIcon("object-rotate-left", KIconLoader::NoGroup, KIconLoader::SizeSmall);
    }
    else
    {
        return KIconLoader::global()->loadIcon("object-rotate-right", KIconLoader::NoGroup, KIconLoader::SizeSmall);
    }
}

void ShowfotoRotateOverlayButton::updateToolTip()
{
    if (m_direction == ShowfotoRotateOverlayLeft)
    {
        setToolTip(i18nc("@info:tooltip", "Rotate Left"));
    }
    else
    {
        setToolTip(i18nc("@info:tooltip", "Rotate Right"));
    }
}

// --------------------------------------------------------------------

ShowfotoRotateOverlay::ShowfotoRotateOverlay(ShowfotoRotateOverlayDirection dir, QObject* const parent)
    : HoverButtonDelegateOverlay(parent),
      m_direction(dir)
{
}

void ShowfotoRotateOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked()));
    }
}

ItemViewHoverButton* ShowfotoRotateOverlay::createButton()
{
    return new ShowfotoRotateOverlayButton(m_direction, view());
}

void ShowfotoRotateOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int gap    = 5;
    const int x      = rect.right() - (isLeft() ? (2*gap + 64) : (gap + 51));
    const int y      = rect.top() + gap;
    button()->move(QPoint(x, y));
}

void ShowfotoRotateOverlay::slotClicked()
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        emit signalRotate(affectedIndexes(index));
    }
}

bool ShowfotoRotateOverlay::checkIndex(const QModelIndex& index) const
{
    ShowfotoItemInfo info = ShowfotoImageModel::retrieveShowfotoItemInfo(index);
    return (info.mime.contains("image/"));
}

void ShowfotoRotateOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(button()->index());
}

void ShowfotoRotateOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

} // namespace Digikam
