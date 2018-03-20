/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : selection icon view item at mouse hover
 *
 * Copyright (C) 2008      by Peter Penz <peter.penz@gmx.at>
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

#include "imageselectionoverlay.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imagecategorizedview.h"
#include "itemviewhoverbutton.h"

namespace Digikam
{

ImageSelectionOverlayButton::ImageSelectionOverlayButton(QAbstractItemView* parentView)
    : ItemViewHoverButton(parentView)
{
}

QSize ImageSelectionOverlayButton::sizeHint() const
{
    return QSize(32, 32);
}

QIcon ImageSelectionOverlayButton::icon()
{
    return QIcon::fromTheme(isChecked() ? QLatin1String("list-remove") : QLatin1String("list-add"));
}

void ImageSelectionOverlayButton::updateToolTip()
{
    setToolTip(isChecked() ? i18nc("@info:tooltip", "Deselect Item") :
               i18nc("@info:tooltip", "Select Item"));
}

// --------------------------------------------------------------------

ImageSelectionOverlay::ImageSelectionOverlay(QObject* parent)
    : HoverButtonDelegateOverlay(parent)
{
}

void ImageSelectionOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked(bool)));

        connect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)));
    }
    else
    {
        // button is deleted

        if (m_view)
        {
            disconnect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                       this, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)));
        }
    }
}

ItemViewHoverButton* ImageSelectionOverlay::createButton()
{
    return new ImageSelectionOverlayButton(view());
}

void ImageSelectionOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int size   = qBound(16, rect.width() / 8 - 2, 48);
    const int gap    = 5;
    const int x      = rect.left() + gap;
    const int y      = rect.top() + gap;
    button()->resize(size, size);
    button()->move(QPoint(x, y));

    QItemSelectionModel* const selModel = m_view->selectionModel();
    button()->setChecked(selModel->isSelected(index));
}

void ImageSelectionOverlay::slotClicked(bool checked)
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        QItemSelectionModel* const selModel = m_view->selectionModel();

        if (checked)
        {
            selModel->select(index, QItemSelectionModel::Select);
        }
        else
        {
            selModel->select(index, QItemSelectionModel::Deselect);
        }

        selModel->setCurrentIndex(index, QItemSelectionModel::Current);
    }
}

void ImageSelectionOverlay::slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        if (selected.contains(index))
        {
            button()->setChecked(true);
        }
        else if (deselected.contains(index))
        {
            button()->setChecked(false);
        }
    }
}

} // namespace Digikam
