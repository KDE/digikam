/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-31
 * Description : rotate icon view item at mouse hover
 *
 * Copyright (C) 2009-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagerotationoverlay.moc"

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "imagecategorizedview.h"
#include "imageinfo.h"
#include "imagemodel.h"
#include "itemviewhoverbutton.h"

namespace Digikam
{

ImageRotateOverlayButton::ImageRotateOverlayButton(ImageRotateOverlayDirection dir, QAbstractItemView* const parentView)
    : ItemViewHoverButton(parentView),
      m_direction(dir)
{
}

QSize ImageRotateOverlayButton::sizeHint() const
{
    return QSize(16, 16);
}

QPixmap ImageRotateOverlayButton::icon()
{
    if (m_direction == ImageRotateOverlayLeft)
    {
        return KIconLoader::global()->loadIcon("object-rotate-left", KIconLoader::NoGroup, KIconLoader::SizeSmall);
    }
    else
    {
        return KIconLoader::global()->loadIcon("object-rotate-right", KIconLoader::NoGroup, KIconLoader::SizeSmall);
    }
}

void ImageRotateOverlayButton::updateToolTip()
{
    if (m_direction == ImageRotateOverlayLeft)
    {
        setToolTip(i18nc("@info:tooltip", "Rotate Left"));
    }
    else
    {
        setToolTip(i18nc("@info:tooltip", "Rotate Right"));
    }
}

// --------------------------------------------------------------------

ImageRotateOverlay::ImageRotateOverlay(ImageRotateOverlayDirection dir, QObject* const parent)
    : HoverButtonDelegateOverlay(parent),
      m_direction(dir)
{
}

void ImageRotateOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked()));
    }

    // if !active, button() is deleted
}

ItemViewHoverButton* ImageRotateOverlay::createButton()
{
    return new ImageRotateOverlayButton(m_direction, view());
}

void ImageRotateOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int gap    = 5;
    const int x      = rect.right() - 2*gap - (isLeft() ? KIconLoader::SizeSmall*3 + 2 : KIconLoader::SizeSmall*2 +2);
    const int y      = rect.top() + gap;
    button()->move(QPoint(x, y));
}

void ImageRotateOverlay::slotClicked()
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        /*QItemSelectionModel* selModel = m_view->selectionModel();
        selModel->reset();
        selModel->select(index, QItemSelectionModel::Select);
        selModel->setCurrentIndex(index, QItemSelectionModel::Current);*/
        emit signalRotate(affectedIndexes(index));
    }
}

bool ImageRotateOverlay::checkIndex(const QModelIndex& index) const
{
    ImageInfo info = ImageModel::retrieveImageInfo(index);
    return (info.category() == DatabaseItem::Image);
}

void ImageRotateOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(button()->index());
}

void ImageRotateOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

} // namespace Digikam
