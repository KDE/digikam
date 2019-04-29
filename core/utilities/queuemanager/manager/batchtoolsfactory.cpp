/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tools Factory.
 *
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "batchtoolsfactory.h"

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dpluginloader.h"
#include "dpluginbqm.h"

namespace Digikam
{

class Q_DECL_HIDDEN BatchToolsFactory::Private
{

public:

    explicit Private()
    {
    }

    BatchToolsList toolsList;
};

// --------------------------------------------------------------------------------

class Q_DECL_HIDDEN BatchToolsFactoryCreator
{
public:

    BatchToolsFactory object;
};

Q_GLOBAL_STATIC(BatchToolsFactoryCreator, batchToolsManagerCreator)

// --------------------------------------------------------------------------------

BatchToolsFactory* BatchToolsFactory::instance()
{
    return &batchToolsManagerCreator->object;
}

BatchToolsFactory::BatchToolsFactory()
    : d(new Private)
{
    DPluginLoader* const dpl = DPluginLoader::instance();

    foreach (DPlugin* const p, dpl->allPlugins())
    {
        DPluginBqm* const bqm = dynamic_cast<DPluginBqm*>(p);

        if (bqm)
        {
            bqm->setup(this);

            qCDebug(DIGIKAM_GENERAL_LOG) << "BQM plugin named" << bqm->name()
                                         << "registered to" << this;

            foreach (BatchTool* const t, bqm->tools(this))
            {
                registerTool(t);
            }
        }
    }
}

BatchToolsFactory::~BatchToolsFactory()
{
    delete d;
}

BatchToolsList BatchToolsFactory::toolsList() const
{
    return d->toolsList;
}

void BatchToolsFactory::registerTool(BatchTool* const tool)
{
    if (!tool)
    {
        return;
    }

    d->toolsList.append(tool);
}

BatchTool* BatchToolsFactory::findTool(const QString& name, BatchTool::BatchToolGroup group) const
{
    foreach (BatchTool* const tool, d->toolsList)
    {
        if (tool->objectName() == name && tool->toolGroup() == group)
        {
            return tool;
        }
    }

    return nullptr;
}

} // namespace Digikam
