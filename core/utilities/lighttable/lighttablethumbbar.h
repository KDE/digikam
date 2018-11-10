/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_LIGHT_TABLE_THUMB_BAR_H
#define DIGIKAM_LIGHT_TABLE_THUMB_BAR_H

// Local includes

#include "imagethumbnailbar.h"
#include "iteminfo.h"
#include "itemlistmodel.h"

namespace Digikam
{

class LightTableThumbBar : public ImageThumbnailBar
{
    Q_OBJECT

public:

    explicit LightTableThumbBar(QWidget* const parent);
    ~LightTableThumbBar();

    void clear();

    void setItems(const ItemInfoList& list);
    void setOnLeftPanel(const ItemInfo& info);
    void setOnRightPanel(const ItemInfo& info);
    void setNavigateByPair(bool b);

    void removeItemByInfo(const ItemInfo& info);
    void toggleTag(int tagID);

    bool isOnLeftPanel(const ItemInfo& info)             const;
    bool isOnRightPanel(const ItemInfo& info)            const;
    int  countItems()                                     const;
    QModelIndex findItemByInfo(const ItemInfo& info)     const;
    ItemInfo   findItemByIndex(const QModelIndex& index) const;

Q_SIGNALS:

    void signalSetItemOnLeftPanel(const ItemInfo&);
    void signalSetItemOnRightPanel(const ItemInfo&);
    void signalEditItem(const ItemInfo&);
    void signalRemoveItem(const ItemInfo&);
    void signalDroppedItems(const QList<ItemInfo>&);
    void signalClearAll();
    void signalContentChanged();

public Q_SLOTS:

    void slotAssignPickLabel(int);
    void slotAssignColorLabel(int);
    void slotAssignRating(int);
    void slotRatingChanged(const QUrl&, int);
    void slotColorLabelChanged(const QUrl&, int);
    void slotPickLabelChanged(const QUrl&, int);
    void slotToggleTag(const QUrl&, int);
    void slotDockLocationChanged(Qt::DockWidgetArea area);

private:

    void paintEvent(QPaintEvent*);
    void showContextMenuOnInfo(QContextMenuEvent* e, const ItemInfo& info);
    void assignPickLabel(const ItemInfo& info, int pickId);
    void assignColorLabel(const ItemInfo& info, int colorId);
    void assignRating(const ItemInfo& info, int rating);
    void toggleTag(const ItemInfo& info, int tagID);

private Q_SLOTS:

    void slotSetupChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_LIGHT_TABLE_THUMB_BAR_H
