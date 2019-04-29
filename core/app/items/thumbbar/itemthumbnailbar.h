/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-06
 * Description : Thumbnail bar for items
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_ITEM_THUMBNAIL_BAR_H
#define DIGIKAM_ITEM_THUMBNAIL_BAR_H

// Local includes

#include "itemcategorizedview.h"

namespace Digikam
{

class ItemThumbnailBar : public ItemCategorizedView
{
    Q_OBJECT

public:

    explicit ItemThumbnailBar(QWidget* const parent = nullptr);
    ~ItemThumbnailBar();

    /// Sets the policy always for the one scroll bar which is relevant, depending on orientation
    void setScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setFlow(QListView::Flow newFlow);

    void installOverlays();

    /**
     * This installs a duplicate filter model, if the ItemModel may contain duplicates.
     * Otherwise, just use setModels().
     */
    void setModelsFiltered(ItemModel* model, ImageSortFilterModel* filterModel);

    QModelIndex nextIndex(const QModelIndex& index)     const;
    QModelIndex previousIndex(const QModelIndex& index) const;
    QModelIndex firstIndex() const;
    QModelIndex lastIndex()  const;

public Q_SLOTS:

    void assignRating(const QList<QModelIndex>& index, int rating);
    void slotDockLocationChanged(Qt::DockWidgetArea area);

protected:

    virtual void slotSetupChanged();
    virtual bool event(QEvent*);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_THUMBNAIL_BAR_H
