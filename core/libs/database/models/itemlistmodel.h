/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-06
 * Description : An item model based on a static list
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_ITEM_LIST_MODEL_H
#define DIGIKAM_ITEM_LIST_MODEL_H

// Local includes

#include "itemthumbnailmodel.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageChangeset;
class CollectionImageChangeset;

class DIGIKAM_DATABASE_EXPORT ItemListModel : public ItemThumbnailModel
{
    Q_OBJECT

public:

    explicit ItemListModel(QObject* parent = 0);
    ~ItemListModel();

    // NOTE: necessary methods to add and remove ItemInfos to the model are inherited from ItemModel

Q_SIGNALS:

    /**
     * Emitted when images are removed from the model because they are removed in the database
     */
    void imageInfosRemoved(const QList<ItemInfo>& infos);

protected Q_SLOTS:

    void slotCollectionImageChange(const CollectionImageChangeset& changeset);
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_LIST_MODEL_H
