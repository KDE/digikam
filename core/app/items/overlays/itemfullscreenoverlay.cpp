/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-30
 * Description : fullscreen overlay
 *
 * Copyright (C)      2015 by Luca Carlon <carlon dot luca at gmail dot com>
 * Copyright (C) 2015-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemfullscreenoverlay.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "itemcategorizedview.h"
#include "iteminfo.h"
#include "itemmodel.h"
#include "itemviewhoverbutton.h"

namespace Digikam
{

ItemFullScreenOverlayButton::ItemFullScreenOverlayButton(QAbstractItemView* const parentView)
    : ItemViewHoverButton(parentView)
{
    setup();
}

QSize ItemFullScreenOverlayButton::sizeHint() const
{
    return QSize(32, 32);
}

QIcon ItemFullScreenOverlayButton::icon()
{
    return QIcon::fromTheme(QLatin1String("media-playback-start"));
}

void ItemFullScreenOverlayButton::updateToolTip()
{
    setToolTip(i18nc("@info:tooltip", "Show Fullscreen"));
}

// --------------------------------------------------------------------

ItemFullScreenOverlay::ItemFullScreenOverlay(QObject* const parent)
    : HoverButtonDelegateOverlay(parent)
{
}

ItemFullScreenOverlay* ItemFullScreenOverlay::instance(QObject* const parent)
{
    return new ItemFullScreenOverlay(parent);
}

void ItemFullScreenOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked()));
    }

    // if !active, button() is deleted
}

ItemViewHoverButton* ItemFullScreenOverlay::createButton()
{
    return new ItemFullScreenOverlayButton(view());
}

void ItemFullScreenOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int size   = qBound(16, rect.width() / 8 - 2, 48);
    const int gap    = 5;
    const int x      = rect.right() - 3*gap - size*4 + 2;
    const int y      = rect.top() + gap;
    button()->resize(size, size);
    button()->move(QPoint(x, y));
}

void ItemFullScreenOverlay::slotClicked()
{
    QModelIndex index = button()->index();

    if (index.isValid())
        emit signalFullscreen(affectedIndexes(index));
}

bool ItemFullScreenOverlay::checkIndex(const QModelIndex& index) const
{
    ItemInfo info = ItemModel::retrieveItemInfo(index);

    return (info.category() == DatabaseItem::Image ||
            info.category() == DatabaseItem::Video);
}

void ItemFullScreenOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(button()->index());
}

void ItemFullScreenOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

} // namespace Digikam
