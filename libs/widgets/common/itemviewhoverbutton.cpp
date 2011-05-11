/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : Qt item view mouse hover button
 *
 * Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>
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

#include "itemviewhoverbutton.moc"

// Qt includes

#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtCore/QRect>
#include <QtCore/QTimer>
#include <QtCore/QTimeLine>

// KDE includes

#include <kglobalsettings.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <klocale.h>

namespace Digikam
{

ItemViewHoverButton::ItemViewHoverButton(QAbstractItemView* view)
    : QAbstractButton(view->viewport()),
      m_isHovered(false),
      m_fadingValue(0),
      m_icon(),
      m_fadingTimeLine(0)
{
    const bool animate = KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects;
    const int duration = animate ? 600 : 1;
    m_fadingTimeLine   = new QTimeLine(duration, this);
    m_fadingTimeLine->setFrameRange(0, 255);

    setCheckable(true);
    setChecked(false);

    connect(m_fadingTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFadingValue(int)));

    connect(this, SIGNAL(toggled(bool)),
            this, SLOT(refreshIcon()));

    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
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
    m_isHovered = true;
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
    if (m_isHovered)
    {
        KIconEffect iconEffect;
        QPixmap activeIcon = iconEffect.apply(m_icon, KIconLoader::Desktop, KIconLoader::ActiveState);
        painter.drawPixmap(0, 0, activeIcon);
    }
    else
    {
        if (m_fadingValue < 255)
        {
            // apply an alpha mask respecting the fading value to the icon
            QPixmap icon = m_icon;
            QPixmap alphaMask(icon.width(), icon.height());
            const QColor color(m_fadingValue, m_fadingValue, m_fadingValue);
            alphaMask.fill(color);
            icon.setAlphaChannel(alphaMask);
            painter.drawPixmap(0, 0, icon);
        }
        else
        {
            // no fading is required
            painter.drawPixmap(0, 0, m_icon);
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
    m_icon = KIconLoader::global()->loadIcon(icon,
                                             KIconLoader::NoGroup,
                                             KIconLoader::SizeSmall);
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
