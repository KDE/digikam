/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : Qt item view mouse hover button
 *
 * Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>
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

#include "imageratingoverlay.h"
#include "imageratingoverlay.moc"

// Qt includes

#include <QMouseEvent>

// KDE includes

#include <klocale.h>

// Local includes

#include "imagedelegate.h"
#include "imagemodel.h"
#include "imagecategorizedview.h"
#include "ratingbox.h"

namespace Digikam
{

ImageRatingOverlay::ImageRatingOverlay(QObject *parent)
            : AbstractWidgetDelegateOverlay(parent)
{
}

RatingBox *ImageRatingOverlay::ratingBox() const
{
    return static_cast<RatingBox*>(m_widget);
}

QWidget *ImageRatingOverlay::createWidget()
{
    return new RatingBox(parentWidget());
}

void ImageRatingOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        connect(ratingBox(), SIGNAL(signalRatingChanged(int)),
                this, SLOT(slotRatingChanged(int)));

        connect(view()->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                this, SLOT(slotDataChanged(const QModelIndex &, const QModelIndex &)));
    }
}

void ImageRatingOverlay::mouseMoved(QMouseEvent *e, const QRect& visualRect, const QModelIndex &index)
{
    if (index != m_index)
        return;

    QRect rect = delegate()->ratingRect();
    rect.translate(visualRect.topLeft());

    if (rect.contains(e->pos()))
    {
        delegate()->setRatingEdited(m_index);
        view()->update(m_index);
        m_widget->show();
    }
    else
    {
        hide();
    }
}

void ImageRatingOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
        updatePosition();
}

void ImageRatingOverlay::hide()
{
    delegate()->setRatingEdited(QModelIndex());
    AbstractWidgetDelegateOverlay::hide();
}

void ImageRatingOverlay::updatePosition()
{
    if (!m_index.isValid())
        return;

    QRect rect = delegate()->ratingRect();
    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

void ImageRatingOverlay::updateRating()
{
    if (!m_index.isValid())
        return;
    ImageInfo info = ImageModel::retrieveImageInfo(m_index);
    ratingBox()->setRating(info.rating());
}

void ImageRatingOverlay::slotRatingChanged(int rating)
{
    if (m_widget && m_widget->isVisible() && m_index.isValid())
        emit ratingEdited(m_index, rating);
}

void ImageRatingOverlay::slotEntered(const QModelIndex& index)
{
    // do _not_ call base class, which shows the widget

    m_index = index;

    updatePosition();
    updateRating();
}

void ImageRatingOverlay::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (m_widget && m_widget->isVisible() && QItemSelectionRange(topLeft, bottomRight).contains(m_index))
        updateRating();
}

} // namespace Digikam

