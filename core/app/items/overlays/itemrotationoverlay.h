/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-31
 * Description : rotate icon view item at mouse hover
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ITEM_ROTATION_OVERLAY_H
#define DIGIKAM_ITEM_ROTATION_OVERLAY_H

// Qt includes

#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "itemdelegateoverlay.h"

namespace Digikam
{

enum ItemRotateOverlayDirection
{
    ItemRotateOverlayLeft,
    ItemRotateOverlayRight
};

class ItemRotateOverlayButton : public ItemViewHoverButton
{
public:

    ItemRotateOverlayButton(ItemRotateOverlayDirection dir, QAbstractItemView* const parentView);
    virtual QSize sizeHint() const;

protected:

    virtual QIcon icon();
    virtual void updateToolTip();

protected:

    ItemRotateOverlayDirection const m_direction;
};

// --------------------------------------------------------------------

class ItemRotateOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    ItemRotateOverlay(ItemRotateOverlayDirection dir, QObject* const parent);
    virtual void setActive(bool active);

    ItemRotateOverlayDirection direction() const { return m_direction; }
    bool isLeft() const  { return m_direction  == ItemRotateOverlayLeft; }
    bool isRight() const { return m_direction == ItemRotateOverlayRight; }

    static ItemRotateOverlay* left(QObject* const parent)  { return new ItemRotateOverlay(ItemRotateOverlayLeft, parent);  }
    static ItemRotateOverlay* right(QObject* const parent) { return new ItemRotateOverlay(ItemRotateOverlayRight, parent); }

Q_SIGNALS:

    void signalRotate(const QList<QModelIndex>& indexes);

protected:

    virtual ItemViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;
    virtual void widgetEnterEvent();
    virtual void widgetLeaveEvent();

private Q_SLOTS:

    void slotClicked();

private:

    ItemRotateOverlayDirection const m_direction;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_ROTATION_OVERLAY_H
