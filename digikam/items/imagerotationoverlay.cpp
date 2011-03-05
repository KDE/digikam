/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-31
 * Description : rotate icon view item at mouse hover
 *
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

ImageRotateLeftOverlayButton::ImageRotateLeftOverlayButton(QAbstractItemView* parentView)
    : ItemViewHoverButton(parentView)
{
}

QSize ImageRotateLeftOverlayButton::sizeHint() const
{
    return QSize(16, 16);
}

QPixmap ImageRotateLeftOverlayButton::icon()
{
    return KIconLoader::global()->loadIcon("object-rotate-left", KIconLoader::NoGroup, KIconLoader::SizeSmall);
}

void ImageRotateLeftOverlayButton::updateToolTip()
{
    setToolTip(i18nc("@info:tooltip", "Rotate Left"));
}

// --------------------------------------------------------------------

ImageRotateRightOverlayButton::ImageRotateRightOverlayButton(QAbstractItemView* parentView)
    : ItemViewHoverButton(parentView)
{
}

QSize ImageRotateRightOverlayButton::sizeHint() const
{
    return QSize(16, 16);
}

QPixmap ImageRotateRightOverlayButton::icon()
{
    return KIconLoader::global()->loadIcon("object-rotate-right", KIconLoader::NoGroup, KIconLoader::SizeSmall);
}

void ImageRotateRightOverlayButton::updateToolTip()
{
    setToolTip(i18nc("@info:tooltip", "Rotate Right"));
}

// --------------------------------------------------------------------

ImageRotateLeftOverlay::ImageRotateLeftOverlay(QObject* parent)
    : HoverButtonDelegateOverlay(parent)
{
}

void ImageRotateLeftOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked()));
    }

    // if !active, button() is deleted
}

ItemViewHoverButton* ImageRotateLeftOverlay::createButton()
{
    return new ImageRotateLeftOverlayButton(view());
}

void ImageRotateLeftOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int gap    = 5;
    const int x      = rect.right() - 2*gap - 32;
    const int y      = rect.top() + gap;
    button()->move(QPoint(x, y));
}

void ImageRotateLeftOverlay::slotClicked()
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        QItemSelectionModel* selModel = m_view->selectionModel();
        selModel->reset();
        selModel->select(index, QItemSelectionModel::Select);
        selModel->setCurrentIndex(index, QItemSelectionModel::Current);
        emit signalRotateLeft();
    }
}

bool ImageRotateLeftOverlay::checkIndex(const QModelIndex& index) const
{
    ImageInfo info = ImageModel::retrieveImageInfo(index);
    return info.category() == DatabaseItem::Image;
}

// --------------------------------------------------------------------

ImageRotateRightOverlay::ImageRotateRightOverlay(QObject* parent)
    : HoverButtonDelegateOverlay(parent)
{
}

void ImageRotateRightOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked()));
    }

    // if !active, button() is deleted
}

ItemViewHoverButton* ImageRotateRightOverlay::createButton()
{
    return new ImageRotateRightOverlayButton(view());
}

void ImageRotateRightOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int gap    = 5;
    const int x      = rect.right() - gap - 16;
    const int y      = rect.top() + gap;
    button()->move(QPoint(x, y));
}

void ImageRotateRightOverlay::slotClicked()
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        QItemSelectionModel* selModel = m_view->selectionModel();
        selModel->reset();
        selModel->select(index, QItemSelectionModel::Select);
        selModel->setCurrentIndex(index, QItemSelectionModel::Current);
        emit signalRotateRight();
    }
}

bool ImageRotateRightOverlay::checkIndex(const QModelIndex& index) const
{
    ImageInfo info = ImageModel::retrieveImageInfo(index);
    return info.category() == DatabaseItem::Image;
}

} // namespace Digikam
