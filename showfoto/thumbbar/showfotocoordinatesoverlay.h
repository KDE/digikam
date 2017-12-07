/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-05-28
 * Description : overlay for GPS location indicator
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SHOWFOTO_COORDINATES_OVERLAY_H
#define SHOWFOTO_COORDINATES_OVERLAY_H

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "imagedelegateoverlay.h"
#include "itemviewshowfotodelegate.h"

using namespace Digikam;

namespace ShowFoto
{

class ShowfotoCoordinatesOverlayWidget : public QAbstractButton
{
    Q_OBJECT

public:

    explicit ShowfotoCoordinatesOverlayWidget(QWidget* const parent = 0);

protected:

    virtual void paintEvent(QPaintEvent*);
};

// ----------------------------------------------------------------------

class ShowfotoCoordinatesOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewShowfotoDelegate)

public:

    explicit ShowfotoCoordinatesOverlay(QObject* const parent);
    ShowfotoCoordinatesOverlayWidget* buttonWidget() const;

protected:

    void updatePosition();

    virtual QWidget* createWidget();
    virtual void setActive(bool active);
    virtual void visualChange();
    virtual bool checkIndex(const QModelIndex& index) const;
    virtual void slotEntered(const QModelIndex& index);

protected:

    QPersistentModelIndex m_index;
};

} // namespace ShowFoto

#endif // SHOWFOTO_COORDINATES_OVERLAY_H
