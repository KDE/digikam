/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-05-28
 * Description : overlay for GPS location indicator
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "showfotocoordinatesoverlay.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "showfotodelegate.h"
#include "showfotoimagemodel.h"
#include "showfotocategorizedview.h"

namespace ShowFoto
{

ShowfotoCoordinatesOverlayWidget::ShowfotoCoordinatesOverlayWidget(QWidget* const parent)
    : QAbstractButton(parent)
{
}

void ShowfotoCoordinatesOverlayWidget::paintEvent(QPaintEvent*)
{
}

// -----------------------------------------------------------------------------------------

ShowfotoCoordinatesOverlay::ShowfotoCoordinatesOverlay(QObject* const parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

ShowfotoCoordinatesOverlayWidget* ShowfotoCoordinatesOverlay::buttonWidget() const
{
    return static_cast<ShowfotoCoordinatesOverlayWidget*>(m_widget);
}

QWidget* ShowfotoCoordinatesOverlay::createWidget()
{
    QAbstractButton* const button = new ShowfotoCoordinatesOverlayWidget(parentWidget());
    //button->setCursor(Qt::PointingHandCursor);
    return button;
}

void ShowfotoCoordinatesOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);
}

void ShowfotoCoordinatesOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void ShowfotoCoordinatesOverlay::updatePosition()
{
    if (!m_index.isValid())
    {
        return;
    }

    QRect rect       = static_cast<ShowfotoDelegate*>(delegate())->coordinatesIndicatorRect();
    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

bool ShowfotoCoordinatesOverlay::checkIndex(const QModelIndex& index) const
{
    ShowfotoItemInfo info = ShowfotoImageModel::retrieveShowfotoItemInfo(index);
    QRect rect            = static_cast<ShowfotoDelegate*>(delegate())->coordinatesIndicatorRect();

    if (!rect.isNull() && info.photoInfo.hasCoordinates)
    {
        m_widget->setToolTip(i18nc("@info:tooltip", "This item has geolocation information."));
        return true;
    }

    // If info.hasCoordinates() = false, no need to show a tooltip, because there is no icon over thumbnail.

    return false;
}

void ShowfotoCoordinatesOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);
    m_index = index;
    updatePosition();
}

} // namespace ShowFoto
