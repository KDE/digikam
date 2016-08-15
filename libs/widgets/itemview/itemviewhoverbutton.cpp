/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : Qt item view mouse hover button
 *
 * Copyright (C) 2008      by Peter Penz <peter dot penz at gmx dot at>
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

#include "itemviewhoverbutton.h"

// Qt includes

#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QTimer>
#include <QTimeLine>
#include <QIcon>

namespace Digikam
{

ItemViewHoverButton::ItemViewHoverButton(QAbstractItemView* const view)
    : QAbstractButton(view->viewport()),
      m_isHovered(false),
      m_fadingValue(0),
      m_icon(),
      m_fadingTimeLine(0)
{
    m_fadingTimeLine = new QTimeLine(600, this);
    m_fadingTimeLine->setFrameRange(0, 255);

    setCheckable(true);
    setChecked(false);

    connect(m_fadingTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFadingValue(int)));

    connect(this, SIGNAL(toggled(bool)),
            this, SLOT(refreshIcon()));
}

void ItemViewHoverButton::initIcon()
{
    // virtual, cannot call from constructor
    refreshIcon();
    resize(sizeHint());
}

void ItemViewHoverButton::updateToolTip()
{
}

void ItemViewHoverButton::reset()
{
    m_index = QModelIndex();
    hide();
}

void ItemViewHoverButton::setIndex(const QModelIndex& index)
{
    m_index = index;

    if (index.isValid())
    {
        startFading();
    }
}

QModelIndex ItemViewHoverButton::index() const
{
    return m_index;
}

void ItemViewHoverButton::setVisible(bool visible)
{
    QAbstractButton::setVisible(visible);

    stopFading();

    if (visible)
    {
        startFading();
    }
}

void ItemViewHoverButton::enterEvent(QEvent* event)
{
    QAbstractButton::enterEvent(event);

    // if the mouse cursor is above the button, display
    // it immediately without fading timer
    m_isHovered   = true;
    m_fadingTimeLine->stop();
    m_fadingValue = 255;
    updateToolTip();
    update();
}

void ItemViewHoverButton::leaveEvent(QEvent* event)
{
    QAbstractButton::leaveEvent(event);
    m_isHovered = false;
    update();
}

void ItemViewHoverButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRect(event->rect());
    painter.setRenderHint(QPainter::Antialiasing);

    // draw an alpha blended circle as background
    const QPalette& palette       = parentWidget()->palette();

    const QBrush& backgroundBrush = palette.brush(QPalette::Normal, QPalette::Window);
    QColor background             = backgroundBrush.color();
    background.setAlpha(m_fadingValue / 2);
    painter.setBrush(background);

    const QBrush& foregroundBrush = palette.brush(QPalette::Normal, QPalette::WindowText);
    QColor foreground             = foregroundBrush.color();
    foreground.setAlpha(m_fadingValue / 4);
    painter.setPen(foreground);

    painter.drawEllipse(0, 0, width(), height());

    // draw the icon overlay
    QPixmap icon = m_icon.pixmap(width() - 2, height() - 2);

    if (m_isHovered)
    {
        QPixmap hovered = icon;
        QPainter p(&hovered);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(hovered.rect(), QColor(0, 0, 0, 127));
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.drawPixmap(0, 0, icon);
        p.end();
        painter.drawPixmap(1, 1, hovered);
    }
    else
    {
        if (m_fadingValue < 255)
        {
            QPainter p(&icon);
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            p.fillRect(icon.rect(), QColor(0, 0, 0, m_fadingValue));
            p.end();
            painter.drawPixmap(1, 1, icon);
        }
        else
        {
            // no fading is required
            painter.drawPixmap(1, 1, icon);
        }
    }
}

void ItemViewHoverButton::setFadingValue(int value)
{
    m_fadingValue = value;

    if (m_fadingValue >= 255)
    {
        m_fadingTimeLine->stop();
    }

    update();
}

/*
void ItemViewHoverButton::setIconOverlay()
{
    const char* icon = isChecked() ? "list-remove" : "list-add";
    m_icon = QIcon::fromTheme(icon).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
}
*/

void ItemViewHoverButton::refreshIcon()
{
    m_icon = icon();
    update();
}

void ItemViewHoverButton::startFading()
{
    if (m_fadingTimeLine->state() != QTimeLine::Running)
    {
        m_fadingTimeLine->start();
    }

    m_fadingValue = 0;
}

void ItemViewHoverButton::stopFading()
{
    m_fadingTimeLine->stop();
    m_fadingValue = 0;
}

} // namespace Digikam
