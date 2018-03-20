/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tool utils.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BATCH_TOOL_UTILS_H
#define BATCH_TOOL_UTILS_H

// Qt includes

#include <QString>
#include <QVariant>
#include <QMap>
#include <QList>
#include <QUrl>

// Local includes

#include "batchtool.h"

namespace Digikam
{

/** A list of batch tool instances.
 */
typedef QList<BatchTool*> BatchToolsList;

// -------------------------------------------------------------------------------------------------------------

/** A container of associated batch tool and settings.
 */
class BatchToolSet
{
public:

    explicit BatchToolSet();
    virtual ~BatchToolSet();

    /** Equality operator which check index, version, name, and group data. Settings member is ignored.
     */
    bool operator==(const BatchToolSet& set) const;

    /// Tool identifier data. Index is tool ID from assigned list.
    int                       index;
    int                       version;
    QString                   name;
    BatchTool::BatchToolGroup group;

    /// Settings hosted in this container.
    BatchToolSettings         settings;
};

//! qDebug() stream operator. Writes property @a t to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const BatchToolSet& s);

// -------------------------------------------------------------------------------------------------------------

/** An indexed map of batch tools with settings.
 */
typedef QList<BatchToolSet> BatchSetList;

// -------------------------------------------------------------------------------------------------------------

/** Container to assign Batch tools and settings to an item by Url.
    Url is used only with ActionThread class.
 */
class AssignedBatchTools
{
public:

    explicit AssignedBatchTools();
    ~AssignedBatchTools();

    QString targetSuffix(bool* const extSet = 0) const;

public:

    QString      m_destFileName;
    QUrl         m_itemUrl;
    BatchSetList m_toolsList;
};

} // namespace Digikam

#endif // BATCHTOOLUTILS_H
