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

void ImageRatingOverlay::mouseMoved(QMouseEvent *e, const QRect& visualRect, const QModelIndex&)
{
    QRect rect = delegate()->ratingRect();
    rect.translate(visualRect.topLeft());

    if (rect.contains(e->pos()))
        m_widget->show();
    else
        m_widget->hide();
}

void ImageRatingOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
        updateBox(m_index);
}

void ImageRatingOverlay::updateBox(const QModelIndex& index)
{
    QRect rect = delegate()->ratingRect();
    QRect visualRect = m_view->visualRect(index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

void ImageRatingOverlay::slotEntered(const QModelIndex& index)
{
    // do _not_ call base class

    m_index = index;

    if (m_index.isValid())
    {
        ImageInfo info = ImageModel::retrieveImageInfo(m_index);
        ratingBox()->setRating(info.rating());
        updateBox(m_index);
    }
}

} // namespace Digikam

