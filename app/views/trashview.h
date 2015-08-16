/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-07
 * Description : Trash view
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef TRASHVIEW_H
#define TRASHVIEW_H

// Qt includes

#include <QWidget>

// Local includes

#include "dtrashitemmodel.h"
#include "thumbnailsize.h"

namespace Digikam
{

class TrashView : public QWidget
{
    Q_OBJECT

public:

    explicit TrashView(QWidget* parent = 0);
    ~TrashView();

    DTrashItemModel* model() const;

    ThumbnailSize getThumbnailSize() const;
    void setThumbnailSize(ThumbnailSize thumbSize);

    QUrl lastSelectedItemUrl() const;
    void selectLastSelected();

private Q_SLOTS:

    void slotSelectionChanged();
    void slotRestoreSelectedItems();
    void slotDeleteSelectedItems();
    void slotRemoveItemsFromModel();
    void slotRemoveAllItemsFromModel();
    void slotDeleteAllItems();
    void slotDataChanged();
    void slotChangeLastSelectedItem(const QModelIndex& curr, const QModelIndex& prev);

Q_SIGNALS:

    void selectionChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // TRASHVIEW_H
