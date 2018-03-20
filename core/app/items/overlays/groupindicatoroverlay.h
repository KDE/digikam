/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-28
 * Description : overlay for extra functionality of the group indicator
 *
 * Copyright (C) 2011      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef GROUPINDICATOROVERLAY_H
#define GROUPINDICATOROVERLAY_H

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "imagedelegateoverlay.h"
#include "itemviewimagedelegate.h"

namespace Digikam
{

class GroupIndicatorOverlayWidget : public QAbstractButton
{
    Q_OBJECT

public:

    explicit GroupIndicatorOverlayWidget(QWidget* const parent = 0);
    virtual void contextMenuEvent(QContextMenuEvent* event);

protected:

    virtual void paintEvent(QPaintEvent*);

Q_SIGNALS:

    void contextMenu(QContextMenuEvent* event);
};

// ----------------------------------------------------------------------------------

class GroupIndicatorOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewImageDelegate)

public:

    explicit GroupIndicatorOverlay(QObject* const parent);

    GroupIndicatorOverlayWidget* buttonWidget() const;

Q_SIGNALS:

    void toggleGroupOpen(const QModelIndex& index);
    void showButtonContextMenu(const QModelIndex& index, QContextMenuEvent* event);

protected Q_SLOTS:

    void slotButtonClicked();
    void slotButtonContextMenu(QContextMenuEvent* event);

protected:

    void updatePosition();
    void updateRating();

    virtual QWidget* createWidget();
    virtual void setActive(bool);
    virtual void visualChange();
    virtual void slotEntered(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;

protected:

    QPersistentModelIndex m_index;
};

} // namespace Digikam

#endif /* GROUPINDICATOROVERLAY_H */
