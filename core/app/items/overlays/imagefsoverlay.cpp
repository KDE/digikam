/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-30
 * Description : fullscreen overlay
 *
 * Copyright (C)      2015 by Luca Carlon <carlon dot luca at gmail dot com>
 * Copyright (C) 2015-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagefsoverlay.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imagecategorizedview.h"
#include "imageinfo.h"
#include "imagemodel.h"
#include "itemviewhoverbutton.h"

namespace Digikam
{

ImageFsOverlayButton::ImageFsOverlayButton(QAbstractItemView* const parentView)
    : ItemViewHoverButton(parentView)
{
}

QSize ImageFsOverlayButton::sizeHint() const
{
    return QSize(32, 32);
}

QIcon ImageFsOverlayButton::icon()
{
    return QIcon::fromTheme(QLatin1String("media-playback-start"));
}

void ImageFsOverlayButton::updateToolTip()
{
    setToolTip(i18nc("@info:tooltip", "Show Fullscreen"));
}

// --------------------------------------------------------------------

ImageFsOverlay::ImageFsOverlay(QObject* const parent)
    : HoverButtonDelegateOverlay(parent)
{
}

ImageFsOverlay* ImageFsOverlay::instance(QObject* const parent)
{
    return new ImageFsOverlay(parent);
}

void ImageFsOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked()));
    }

    // if !active, button() is deleted
}

ItemViewHoverButton* ImageFsOverlay::createButton()
{
    return new ImageFsOverlayButton(view());
}

void ImageFsOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int size   = qBound(16, rect.width() / 8 - 2, 48);
    const int gap    = 5;
    const int x      = rect.right() - 3*gap - size*4 + 2;
    const int y      = rect.top() + gap;
    button()->resize(size, size);
    button()->move(QPoint(x, y));
}

void ImageFsOverlay::slotClicked()
{
    QModelIndex index = button()->index();

    if (index.isValid())
        emit signalFullscreen(affectedIndexes(index));
}

bool ImageFsOverlay::checkIndex(const QModelIndex& index) const
{
    ImageInfo info = ImageModel::retrieveImageInfo(index);

    return (info.category() == DatabaseItem::Image ||
            info.category() == DatabaseItem::Video);
}

void ImageFsOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(button()->index());
}

void ImageFsOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

} // namespace Digikam
