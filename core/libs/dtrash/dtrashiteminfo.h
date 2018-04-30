/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-08
 * Description : DTrash item info container
 *
 * Copyright (C) 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#ifndef DIGIKAM_DTRASH_ITEM_INFO_H
#define DIGIKAM_DTRASH_ITEM_INFO_H

// Qt includes

#include <QList>
#include <QDateTime>

namespace Digikam
{

class DTrashItemInfo
{

public:

    explicit DTrashItemInfo();
    bool isNull() const;

public:

    QString   trashPath;
    QString   jsonFilePath;
    QString   collectionPath;
    QString   collectionRelativePath;
    QDateTime deletionTimestamp;
    qlonglong imageId;
};

typedef QList<DTrashItemInfo> DTrashItemInfoList;

//! qDebug() stream operator. Writes property @a info to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const DTrashItemInfo& info);

} // namespace Digikam

#endif // DIGIKAM_DTRASH_ITEM_INFO_H
