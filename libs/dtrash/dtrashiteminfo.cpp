/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-08
 * Description : DTrash item info container
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "dtrashiteminfo.h"

// Qt includes

#include <QDebug>

namespace Digikam {

DTrashItemInfo::DTrashItemInfo()
{
}

bool DTrashItemInfo::isNull() const
{
    return trashPath.isEmpty()              &&
           jsonFilePath.isEmpty()           &&
           collectionPath.isEmpty()               &&
           collectionRelativePath.isEmpty() &&
           deletionTimestamp.isNull();
}

QDebug operator<<(QDebug dbg, const DTrashItemInfo& info)
{
    dbg.nospace() << "DTrashItemInfo::trashPath: "
                  << info.trashPath << ", ";
    dbg.nospace() << "DTrashItemInfo::jsonFilePath: "
                  << info.jsonFilePath << ", ";
    dbg.nospace() << "DTrashItemInfo::CollectionPath: "
                  << info.collectionPath<< ", ";
    dbg.nospace() << "DTrashItemInfo::RelativePath: "
                  << info.collectionRelativePath << ", ";
    dbg.nospace() << "DTrashItemInfo::DeletionTimestamp: "
                  << info.deletionTimestamp.toString();
    return dbg.space();
}

} // namespace Digikam
