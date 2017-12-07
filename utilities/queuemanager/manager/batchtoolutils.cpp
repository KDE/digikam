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

#include "batchtoolutils.h"

// Qt includes

#include <QFileInfo>

// Local includes

#include "batchtool.h"
#include "batchtoolsmanager.h"

namespace Digikam
{

BatchToolSet::BatchToolSet()
{
    index   = -1;
    version = 0;
    group   = BatchTool::BaseTool;
}

BatchToolSet::~BatchToolSet()
{
}

bool BatchToolSet::operator==(const BatchToolSet& set) const
{
    return ( (index   == set.index)   &&
             (version == set.version) &&
             (name    == set.name )   &&
             (group   == set.group)
           );
}

QDebug operator<<(QDebug dbg, const BatchToolSet& s)
{
    dbg.nospace() << "BatchToolSet::";
    dbg.nospace() << "index: "    << s.index   << ", ";
    dbg.nospace() << "version: "  << s.version << ", ";
    dbg.nospace() << "name: "     << s.name    << ", ";
    dbg.nospace() << "group: "    << s.group   << ", ";
    dbg.nospace() << "settings: " << s.settings;
    return dbg.space();
}

// ---------------------------------------------------------------------------------------------

AssignedBatchTools::AssignedBatchTools()
{
}

AssignedBatchTools::~AssignedBatchTools()
{
}

QString AssignedBatchTools::targetSuffix(bool* const extSet) const
{
    QString suffix;

    foreach(const BatchToolSet& set, m_toolsList)
    {
        BatchTool* const tool = BatchToolsManager::instance()->findTool(set.name, set.group);
        if (tool)
        {
            QString s = tool->outputSuffix();

            if (!s.isEmpty())
            {
                suffix = s;

                if (extSet != 0)
                {
                    *extSet = true;
                }
            }
        }
    }

    if (suffix.isEmpty())
    {
        if (extSet != 0)
        {
            *extSet = false;
        }

        return (QFileInfo(m_itemUrl.fileName()).suffix());
    }

    return suffix;
}

}  // namespace Digikam
