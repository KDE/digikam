/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tools Manager.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "batchtoolsmanager.h"
#include "batchtoolsmanager.moc"

// KDE includes

#include <kdebug.h>

// Local includes

#include "convert2jpeg.h"
#include "convert2png.h"
#include "convert2tiff.h"
#include "convert2jp2.h"
#include "autocorrection.h"
#include "resize.h"
#include "rotate.h"
#include "restoration.h"
#include "flip.h"
#include "watermark.h"
#include "metadata.h"
#include "sharpen.h"

namespace Digikam
{

class BatchToolsManagerPriv
{

public:

    BatchToolsManagerPriv(){}

    BatchToolsList toolsList;
};

BatchToolsManager::BatchToolsManager(QObject* parent)
                 : QObject(parent), d(new BatchToolsManagerPriv)
{
    // Register base tools.
    registerTool(new Convert2JPEG(this));
    registerTool(new Convert2PNG(this));
    registerTool(new Convert2TIFF(this));
    registerTool(new Convert2JP2(this));
    registerTool(new AutoCorrection(this));
    registerTool(new Rotate(this));
    registerTool(new Flip(this));
    registerTool(new Resize(this));
    registerTool(new Restoration(this));
    registerTool(new WaterMark(this));
//    registerTool(new Metadata(this));
    registerTool(new Sharpen(this));
}

BatchToolsManager::~BatchToolsManager()
{
    delete d;
}

BatchToolsList BatchToolsManager::toolsList() const
{
    return d->toolsList;
}

void BatchToolsManager::registerTool(BatchTool* tool)
{
    d->toolsList.append(tool);
}

void BatchToolsManager::unregisterTool(BatchTool* tool)
{
    d->toolsList.removeAll(tool);
}

BatchTool* BatchToolsManager::findTool(const QString& name, BatchTool::BatchToolGroup group) const
{
    for (BatchToolsList::const_iterator it = d->toolsList.constBegin(); it != d->toolsList.constEnd(); ++it)
    {
        if ((*it)->objectName() == name && (*it)->toolGroup() == group)
            return *it;
    }

    return 0;
}

}  // namespace Digikam
