/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tool utils.
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BATCHTOOLUTILS_H
#define BATCHTOOLUTILS_H

// Qt includes

#include <QString>
#include <QVariant>
#include <QMap>
#include <QList>

// KDE includes

#include <kurl.h>

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

    BatchToolSet()
    {
    };

    QString                   name;
    BatchTool::BatchToolGroup group;
    BatchToolSettings         settings;
};

// -------------------------------------------------------------------------------------------------------------

/** An indexed map of batch tools with settings.
 */
typedef QMap<int, BatchToolSet> BatchToolMap;

// -------------------------------------------------------------------------------------------------------------

/** Container to assign Batch tools and settings to an item by Url.
    Url is used only with ActionThread class.
 */
class AssignedBatchTools
{
public:

    AssignedBatchTools();
    ~AssignedBatchTools();

    QString targetSuffix(bool* const extSet = 0) const;

public:

    QString      m_destFileName;
    KUrl         m_itemUrl;
    BatchToolMap m_toolsMap;
};

}  // namespace Digikam

#endif /* BATCHTOOLUTILS_H */
