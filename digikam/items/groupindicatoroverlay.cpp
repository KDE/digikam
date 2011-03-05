/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-28
 * Description : overlay for extra functionality of the group indicator
 *
 * Copyright (C) 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "groupindicatoroverlay.moc"

// Qt includes

#include <QMouseEvent>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kglobalsettings.h>

// Local includes

#include "imagedelegate.h"
#include "imagemodel.h"
#include "imagecategorizedview.h"
#include "ratingwidget.h"

namespace Digikam
{

GroupIndicatorOverlayWidget::GroupIndicatorOverlayWidget(QWidget* parent)
    : QAbstractButton(parent)
{
}

void GroupIndicatorOverlayWidget::contextMenuEvent(QContextMenuEvent* event)
{
    emit contextMenu(event);
}

void GroupIndicatorOverlayWidget::paintEvent(QPaintEvent*)
{
}

GroupIndicatorOverlay::GroupIndicatorOverlay(QObject* parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

GroupIndicatorOverlayWidget* GroupIndicatorOverlay::buttonWidget() const
{
    return static_cast<GroupIndicatorOverlayWidget*>(m_widget);
}

QWidget* GroupIndicatorOverlay::createWidget()
{
    QAbstractButton* button = new GroupIndicatorOverlayWidget(parentWidget());
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

void GroupIndicatorOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        connect(buttonWidget(), SIGNAL(clicked()),
                this, SLOT(slotButtonClicked()));
        connect(buttonWidget(), SIGNAL(contextMenu(QContextMenuEvent*)),
                this, SLOT(slotButtonContextMenu(QContextMenuEvent*)));
    }
    else
    {
        // widget is deleted
    }
}

void GroupIndicatorOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void GroupIndicatorOverlay::updatePosition()
{
    if (!m_index.isValid())
    {
        return;
    }

    QRect rect = static_cast<ImageDelegate*>(delegate())->groupIndicatorRect();

    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

bool GroupIndicatorOverlay::checkIndex(const QModelIndex& index) const
{
    ImageInfo info = ImageModel::retrieveImageInfo(index);
    return info.hasGroupedImages();
}

void GroupIndicatorOverlay::slotButtonClicked()
{
    emit toggleGroupOpen(m_index);
}

void GroupIndicatorOverlay::slotButtonContextMenu(QContextMenuEvent* event)
{
    emit showButtonContextMenu(m_index, event);
}

void GroupIndicatorOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);
    m_index = index;
    updatePosition();
}

} // namespace Digikam
