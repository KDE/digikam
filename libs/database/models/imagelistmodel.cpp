/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-06
 * Description : An image model based on a static list
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

#include "imagelistmodel.h"

// Local includes

#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredbchangesets.h"
#include "coredbwatch.h"
#include "imageinfo.h"
#include "imageinfolist.h"

namespace Digikam
{

ImageListModel::ImageListModel(QObject* parent)
    : ImageThumbnailModel(parent)
{
    connect(CoreDbAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)));
}

ImageListModel::~ImageListModel()
{
}

void ImageListModel::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{
    if (isEmpty())
    {
        return;
    }

    switch (changeset.operation())
    {
        case CollectionImageChangeset::Added:
            break;
        case CollectionImageChangeset::Removed:
        case CollectionImageChangeset::RemovedAll:
            removeImageInfos(ImageInfoList(changeset.ids()));
            break;

        default:
            break;
    }
}

} // namespace Digikam
