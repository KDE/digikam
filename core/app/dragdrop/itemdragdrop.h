/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Items - drag and drop handling
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

#ifndef DIGIKAM_ITEM_DRAG_DROP_H
#define DIGIKAM_ITEM_DRAG_DROP_H

// Qt includes

#include <QUrl>

// Local includes

#include "iteminfo.h"
#include "abstractitemdragdrophandler.h"
#include "itemalbummodel.h"

namespace Digikam
{

class ItemDragDropHandler : public AbstractItemDragDropHandler
{
    Q_OBJECT

public:

    explicit ItemDragDropHandler(ItemModel* const model);

    ItemModel*      model()      const;
    ItemAlbumModel* albumModel() const;

    /**
     * Enables a mode in which dropping will never start an operation
     * which copies or moves files on disk.
     * Only the signals are emitted.
     */
    void setReadOnlyDrop(bool readOnly);

    virtual bool dropEvent(QAbstractItemView* view, const QDropEvent* e, const QModelIndex& droppedOn);
    virtual Qt::DropAction accepts(const QDropEvent* e, const QModelIndex& dropIndex);
    virtual QStringList mimeTypes() const;
    virtual QMimeData* createMimeData(const QList<QModelIndex> &);

Q_SIGNALS:

    void itemInfosDropped(const QList<ItemInfo>& infos);
    void urlsDropped(const QList<QUrl>& urls);
    void assignTags(const QList<ItemInfo>& list, const QList<int>& tagIDs);
    void addToGroup(const ItemInfo& pick, const QList<ItemInfo>& infos);
    void dragDropSort(const ItemInfo& pick, const QList<ItemInfo>& infos);

protected:

    bool m_readOnly;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_DRAG_DROP_H
