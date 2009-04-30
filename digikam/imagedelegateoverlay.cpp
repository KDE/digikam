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

// KDE includes

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

void ImageDelegateOverlay::mouseMoved(QMouseEvent *, const QRect &, const QModelIndex &)
{
}

void ImageDelegateOverlay::paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &)
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

HoverWidgetDelegateOverlay::HoverWidgetDelegateOverlay(QObject *parent)
            : ImageDelegateOverlay(parent),
              m_button(0)
{
}

void HoverWidgetDelegateOverlay::setActive(bool active)
{
    if (active)
    {
        if (m_button)
            delete m_button;
        m_button = createButton();
        m_button->initIcon();

        connect(m_view->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                this, SLOT(slotRowsRemoved(const QModelIndex&, int, int)));

        connect(m_view->model(), SIGNAL(reset()),
                this, SLOT(slotReset()));

        connect(m_view, SIGNAL(entered(const QModelIndex &)),
                this, SLOT(slotEntered(const QModelIndex &)));

        connect(m_view, SIGNAL(viewportEntered()),
                this, SLOT(slotViewportEntered()));
    }
    else
    {
        delete m_button;
        m_button = 0;
        disconnect(m_view->model(), 0, this, 0);
        disconnect(m_view, 0, this, 0);
    }
}

void HoverWidgetDelegateOverlay::visualChange()
{
    if (m_button && m_button->isVisible())
        updateButton(m_button->index());
}

void HoverWidgetDelegateOverlay::slotReset()
{
    m_button->reset();
}

void HoverWidgetDelegateOverlay::slotEntered(const QModelIndex &index)
{
    m_button->hide();
    if (index.isValid()) {
        m_button->setIndex(index);
        updateButton(index);
        m_button->show();
    } else {
        m_button->setIndex(index);
    }
}

void HoverWidgetDelegateOverlay::slotViewportEntered()
{
    m_button->hide();
}

void HoverWidgetDelegateOverlay::slotRowsRemoved(const QModelIndex &, int, int)
{
    m_button->hide();
}


} // namespace Digikam


