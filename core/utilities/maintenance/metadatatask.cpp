/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-09
 * Description : Thread actions task for metadata synchronizer.
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadatatask.h"

// Local includes

#include "collectionscanner.h"
#include "metadatahub.h"
#include "digikam_debug.h"
#include "maintenancedata.h"

namespace Digikam
{

class MetadataTask::Private
{
public:

    Private()
        : tagsOnly(false),
          direction(MetadataSynchronizer::WriteFromDatabaseToFile),
          data(0)
    {
    }

    bool                                tagsOnly;

    MetadataSynchronizer::SyncDirection direction;

    MaintenanceData*                    data;
};

// -------------------------------------------------------

MetadataTask::MetadataTask()
    : ActionJob(),
      d(new Private)
{
}

MetadataTask::~MetadataTask()
{
    cancel();
    delete d;
}

void MetadataTask::setTagsOnly(bool value)
{
    d->tagsOnly = value;
}

void MetadataTask::setDirection(MetadataSynchronizer::SyncDirection dir)
{
    d->direction = dir;
}

void MetadataTask::setMaintenanceData(MaintenanceData* const data)
{
    d->data = data;
}

void MetadataTask::run()
{
    // While we have data (using this as check for non-null)
    while (d->data)
    {
        if (m_cancel)
        {
            return;
        }

        ImageInfo item = d->data->getImageInfo();

        // If the item is null, we are done.
        if (item.isNull())
        {
            break;
        }

        if (d->direction == MetadataSynchronizer::WriteFromDatabaseToFile)
        {
            MetadataHub fileHub;

            // read in from database
            fileHub.load(item);

            // write out to file DMetadata
            if (d->tagsOnly)
            {
                fileHub.writeTags(item.filePath());
            }
            else
            {
                fileHub.write(item.filePath(), MetadataHub::WRITE_ALL, true);
            }
        }
        else // MetadataSynchronizer::ReadFromFileToDatabase
        {
            CollectionScanner scanner;
            scanner.scanFile(item, CollectionScanner::Rescan);
        }

        emit signalFinished(QImage());
    }

    emit signalDone();
}

}  // namespace Digikam
