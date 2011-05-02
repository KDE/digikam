/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-02
 * Description : LT thumbbar item panel indicator
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepaneloverlay.moc"

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "imagecategorizedview.h"
#include "itemviewhoverbutton.h"

namespace Digikam
{

ImagePanelOverlayButton::ImagePanelOverlayButton(QAbstractItemView* parentView)
    : ItemViewHoverButton(parentView)
{
}

QSize ImagePanelOverlayButton::sizeHint() const
{
    return QSize(16, 16);
}

QPixmap ImagePanelOverlayButton::icon()
{
    return KIconLoader::global()->loadIcon(isChecked() ? "arrow-left" : "arrow-right",
                                           KIconLoader::NoGroup,
                                           KIconLoader::SizeSmall);
}

void ImagePanelOverlayButton::updateToolTip()
{
    setToolTip(isChecked() ? i18nc("@info:tooltip", "Set on left panel") :
                             i18nc("@info:tooltip", "Set on right panel"));
}

// --------------------------------------------------------------------

ImagePanelOverlay::ImagePanelOverlay(QObject* parent)
    : HoverButtonDelegateOverlay(parent)
{
}

void ImagePanelOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked(bool)));

        connect(m_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                this, SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection&)));
    }
    else
    {
        // button is deleted

        if (m_view)
        {
            disconnect(m_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                       this, SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection&)));
        }
    }
}

ItemViewHoverButton* ImagePanelOverlay::createButton()
{
    return new ImagePanelOverlayButton(view());
}

void ImagePanelOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int gap    = 5;
    const int x      = rect.left() + gap;
    const int y      = rect.top()  + gap;
    button()->move(QPoint(x, y));

    QItemSelectionModel* selModel = m_view->selectionModel();
    button()->setChecked(selModel->isSelected(index));
}

void ImagePanelOverlay::slotClicked(bool checked)
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        QItemSelectionModel* selModel = m_view->selectionModel();

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

void ImagePanelOverlay::slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
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
