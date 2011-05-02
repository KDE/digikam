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

#ifndef IMAGEPANELOVERLAY_H
#define IMAGEPANELOVERLAY_H

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "imagedelegateoverlay.h"

namespace Digikam
{

class ImagePanelOverlayButton : public ItemViewHoverButton
{
public:

    ImagePanelOverlayButton(QAbstractItemView* parentView);
    virtual QSize sizeHint() const;

protected:

    virtual QPixmap icon();
    virtual void updateToolTip();
};

// --------------------------------------------------------------------

class ImagePanelOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    ImagePanelOverlay(QObject* parent);
    virtual void setActive(bool active);

protected:

    virtual ItemViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);

protected Q_SLOTS:

    void slotClicked(bool checked);
    void slotSelectionChanged(const QItemSelection&, const QItemSelection&);
};

} // namespace Digikam

#endif /* IMAGEPANELOVERLAY_H */
