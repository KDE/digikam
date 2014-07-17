/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : selection icon view item at mouse hover
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

#include "facerejectionoverlay.moc"

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "imagecategorizedview.h"
#include "itemviewhoverbutton.h"
#include "imageinfo.h"
#include "digikamimagefacedelegate.h"
#include "imagemodel.h"

namespace Digikam
{

FaceRejectionOverlayButton::FaceRejectionOverlayButton(QAbstractItemView* parentView)
    : ItemViewHoverButton(parentView)
{
}

QSize FaceRejectionOverlayButton::sizeHint() const
{
    return QSize(16, 16);
}

QPixmap FaceRejectionOverlayButton::icon()
{
    return KIconLoader::global()->loadIcon("dialog-close",
                                           KIconLoader::NoGroup,
                                           KIconLoader::SizeSmall);
}

void FaceRejectionOverlayButton::updateToolTip()
{
    setToolTip(i18nc("@info:tooltip", "If this is not a face, click to reject it."));
}

// --------------------------------------------------------------------

FaceRejectionOverlay::FaceRejectionOverlay(QObject* parent)
    : HoverButtonDelegateOverlay(parent)
{
}

void FaceRejectionOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked()));
    }
    else
    {
        // button is deleted
    }
}

ItemViewHoverButton* FaceRejectionOverlay::createButton()
{
    return new FaceRejectionOverlayButton(view());
}

void FaceRejectionOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int gap    = 5;
    const int x      = rect.right() - button()->sizeHint().width() - gap;
    const int y      = rect.top() + gap;
    button()->move(QPoint(x, y));
}

void FaceRejectionOverlay::slotClicked()
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        emit rejectFaces(affectedIndexes(index));
    }
}

bool FaceRejectionOverlay::checkIndex(const QModelIndex& index) const
{
    return !index.data(ImageModel::ExtraDataRole).isNull();
}

void FaceRejectionOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(button()->index());
}

void FaceRejectionOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

} // namespace Digikam
