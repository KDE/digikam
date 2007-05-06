/* ============================================================
 * Authors: Marcel Wiesweg
 * Date   : 2007-05-01
 * Description : ImageInfo common data
 * 
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local inclused

#include "imageinfodata.h"
#include "imageinfocache.h"
#include "imageinfocache.moc"

namespace Digikam
{

ImageInfoCache::ImageInfoCache()
{
    DatabaseAttributesWatch *dbwatch = DatabaseAccess::attributesWatch();

    connect(dbwatch, SIGNAL(signalImageFieldChanged(Q_LLONG, Digikam::DatabaseAttributesWatch::ImageDataField)),
            this, SLOT(slotImageFieldChanged(Q_LLONG, Digikam::DatabaseAttributesWatch::ImageDataField)));
}

ImageInfoCache::~ImageInfoCache()
{
}

ImageInfoData *ImageInfoCache::infoForId(Q_LLONG id)
{
    QMap<Q_LLONG, ImageInfoData *>::iterator it = m_map.find(id);
    if (it == m_map.end())
    {
        ImageInfoData *data = new ImageInfoData();
        data->id = id;
        m_map[id] = data;
        return data;
    }
    return *it;
}

bool ImageInfoCache::hasInfoForId(Q_LLONG id) const
{
    return m_map.contains(id);
}

void ImageInfoCache::dropInfo(ImageInfoData *infodata)
{
    // check again ref count, now in mutex-protected context
    if (infodata->count > 1)
        return;

    QMap<Q_LLONG, ImageInfoData *>::iterator it = m_map.find(infodata->id);
    if (it != m_map.end() && (*it) == infodata)
    {
        m_map.remove(it);
        delete infodata;
    }
}

void ImageInfoCache::slotImageFieldChanged(Q_LLONG imageId, DatabaseAttributesWatch::ImageDataField field)
{
    // we have databaseaccess lock here as well!
    QMap<Q_LLONG, ImageInfoData *>::iterator it = m_map.find(imageId);
    if (it != m_map.end())
    {
        // invalidate the relevant field. It will be lazy-loaded at first access.
        switch (field)
        {
            case DatabaseAttributesWatch::ImageComment:
                (*it)->commentValid = false;
                break;
            case DatabaseAttributesWatch::ImageDate:
                (*it)->dateTime = QDateTime();
                break;
            case DatabaseAttributesWatch::ImageRating:
                (*it)->rating = -1;
                break;
            case DatabaseAttributesWatch::ImageTags:
                break;
        }
    }
}



}




