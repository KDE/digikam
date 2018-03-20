/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-21
 * Description : An item to hold information about an image.
 *
 * Copyright (C) 2010,2014 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "imagegps.h"

// Local includes

#include "imageposition.h"
#include "coredb.h"
#include "tagscache.h"

namespace Digikam
{

ImageGPS::ImageGPS(const ImageInfo& info)
    : GPSImageItem(info.fileUrl()),
      m_info(info)
{
}

ImageGPS::~ImageGPS()
{
}

bool ImageGPS::loadImageData()
{
    // In first, we try to get GPS info from database.
    ImagePosition pos = m_info.imagePosition();

    if (!pos.isEmpty() && pos.hasCoordinates())
    {
        m_gpsData.setLatLon(pos.latitudeNumber(), pos.longitudeNumber());

        if (pos.hasAltitude())
        {
            m_gpsData.setAltitude(pos.altitude());
        }

        m_dateTime   = m_info.dateTime();

        // mark us as not-dirty, because the data was just loaded:
        m_dirty      = false;
        m_savedState = m_gpsData;

        emitDataChanged();

        return true;
    }

    // If item do not have any GPS data in databse, we will try to load it from file using standar implementation from GPSImageItem.

    return GPSImageItem::loadImageData();
}

QString ImageGPS::saveChanges()
{
    SaveProperties p = saveProperties();

    // Save info to database.

    ImagePosition pos = m_info.imagePosition();

    if (p.shouldWriteCoordinates)
    {
        pos.setLatitude(p.latitude);
        pos.setLongitude(p.longitude);

        if (p.shouldWriteAltitude)
        {
            pos.setAltitude(p.altitude);
        }
    }

    if (p.shouldRemoveCoordinates)
    {
        pos.remove();
    }
    else if (p.shouldRemoveAltitude)
    {
        pos.removeAltitude();
    }

    pos.apply();

    if (!m_tagList.isEmpty())
    {
        QMap<QString, QVariant> attributes;
        QStringList tagsPath;

        for (int i = 0; i < m_tagList.count(); ++i)
        {

            QString singleTagPath;
            QList<TagData> currentTagPath = m_tagList[i];

            for (int j = 0; j < currentTagPath.count(); ++j)
            {
                singleTagPath.append(QLatin1String("/") + currentTagPath[j].tagName);

                if (j == 0)
                {
                    singleTagPath.remove(0, 1);
                }
            }

            tagsPath.append(singleTagPath);
        }

        QList<int> tagIds = TagsCache::instance()->getOrCreateTags(tagsPath);
        CoreDbAccess().db()->addTagsToItems(QList<qlonglong>() << m_info.id(), tagIds);
    }

    // Save info to file.

    return GPSImageItem::saveChanges();
}

QList<GPSImageItem*> ImageGPS::infosToItems(const ImageInfoList& infos)
{
    QList<GPSImageItem*> items;

    foreach(const ImageInfo& inf, infos)
    {
        items << new ImageGPS(inf);
    }

    return items;
}

} /* namespace Digikam */
