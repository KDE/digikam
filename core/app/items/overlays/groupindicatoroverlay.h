/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-28
 * Description : overlay for extra functionality of the group indicator
 *
 * Copyright (C) 2011      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_GROUP_INDICATOR_OVERLAY_H
#define DIGIKAM_GROUP_INDICATOR_OVERLAY_H

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemdelegateoverlay.h"
#include "itemviewdelegate.h"

namespace Digikam
{

class GroupIndicatorOverlayWidget : public QAbstractButton
{
    Q_OBJECT

public:

    explicit GroupIndicatorOverlayWidget(QWidget* const parent = nullptr);
    virtual void contextMenuEvent(QContextMenuEvent* event);

protected:

    virtual void paintEvent(QPaintEvent*) override;

Q_SIGNALS:

    void contextMenu(QContextMenuEvent* event);
};

// ----------------------------------------------------------------------------------

class GroupIndicatorOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewDelegate)

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

    virtual QWidget* createWidget() override;
    virtual void setActive(bool) override;
    virtual void visualChange() override;
    virtual void slotEntered(const QModelIndex& index) override;
    virtual bool checkIndex(const QModelIndex& index) const override;

protected:

    QPersistentModelIndex m_index;
};

} // namespace Digikam

#endif // DIGIKAM_GROUP_INDICATOR_OVERLAY_H
