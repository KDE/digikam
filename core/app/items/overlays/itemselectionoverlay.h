/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : selection icon view item at mouse hover
 *
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

#ifndef DIGIKAM_ITEM_SELECTION_OVERLAY_H
#define DIGIKAM_ITEM_SELECTION_OVERLAY_H

// Qt includes

#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "itemdelegateoverlay.h"

namespace Digikam
{

class ItemSelectionOverlayButton : public ItemViewHoverButton
{
public:

    explicit ItemSelectionOverlayButton(QAbstractItemView* const parentView);
    virtual QSize sizeHint() const;

protected:

    virtual QIcon icon();
    virtual void updateToolTip();
};

// --------------------------------------------------------------------

class ItemSelectionOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    explicit ItemSelectionOverlay(QObject* const parent);
    virtual void setActive(bool active);

protected:

    virtual ItemViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);

protected Q_SLOTS:

    void slotClicked(bool checked);
    void slotSelectionChanged(const QItemSelection&, const QItemSelection&);
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_SELECTION_OVERLAY_H
