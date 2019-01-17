/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

#ifndef DIGIKAM_ITEM_FULL_SCREEN_OVERLAY_H
#define DIGIKAM_ITEM_FULL_SCREEN_OVERLAY_H

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "itemdelegateoverlay.h"

namespace Digikam
{

class ItemFullScreenOverlayButton : public ItemViewHoverButton
{
public:

    explicit ItemFullScreenOverlayButton(QAbstractItemView* const parentView);
    virtual QSize sizeHint() const;

protected:

    virtual QIcon icon();
    virtual void updateToolTip();
};

// --------------------------------------------------------------------

class ItemFullScreenOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    explicit ItemFullScreenOverlay(QObject* const parent);
    virtual void setActive(bool active);

    static ItemFullScreenOverlay* instance(QObject* const parent);

Q_SIGNALS:

    void signalFullscreen(const QList<QModelIndex>& indexes);

protected:

    virtual ItemViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;
    virtual void widgetEnterEvent();
    virtual void widgetLeaveEvent();

private Q_SLOTS:

    void slotClicked();
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_FULL_SCREEN_OVERLAY_H
