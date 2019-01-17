/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Data set for item lister
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemlisterrecord.h"

namespace Digikam
{

ItemListerRecord::ItemListerRecord(ItemListerRecord::BinaryFormat format)
{
    imageID                          = -1;
    albumID                          = -1;
    albumRootID                      = -1;
    rating                           = -1;
    fileSize                         = -1;
    currentSimilarity                = 0.0;
    category                         = DatabaseItem::UndefinedCategory;
    currentFuzzySearchReferenceImage = -1;
    binaryFormat                     = format;
}

bool ItemListerRecord::operator==(const ItemListerRecord& record) const
{
    return (imageID == record.imageID);
}

} // namespace Digikam
