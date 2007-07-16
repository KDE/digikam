/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-05-04
 * Description : Watch image attributes
 * 
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "databaseaccess.h"
#include "databaseattributeswatch.h"
#include "imageattributeswatch.h"
#include "imageattributeswatch.moc"

namespace Digikam
{

ImageAttributesWatch *ImageAttributesWatch::m_componentData = 0;

ImageAttributesWatch::ImageAttributesWatch()
{
    DatabaseAttributesWatch *dbwatch = DatabaseAccess::attributesWatch();

    connect(dbwatch, SIGNAL(imageFieldChanged(qlonglong, int)),
            this, SLOT(slotImageFieldChanged(qlonglong, int)));
}

ImageAttributesWatch::~ImageAttributesWatch()
{
    m_componentData = 0;
}

void ImageAttributesWatch::cleanUp()
{
    delete m_componentData;
    m_componentData = 0;
}

void ImageAttributesWatch::shutDown()
{
    if (m_componentData)
        m_componentData->disconnect(0, 0, 0);
}

ImageAttributesWatch *ImageAttributesWatch::componentData()
{
    if (!m_componentData)
        m_componentData = new ImageAttributesWatch;
    return m_componentData;
}

void ImageAttributesWatch::slotImageFieldChanged(qlonglong imageId, int field)
{
    // Translate signals
    // TODO: compress?
    // TODO!!: we have databaseaccess lock here as well. Make connection queued in one place (above!)
    switch (field)
    {
        case DatabaseAttributesWatch::ImageComment:
            emit signalImageCaptionChanged(imageId);
            break;
        case DatabaseAttributesWatch::ImageDate:
            emit signalImageDateChanged(imageId);
            break;
        case DatabaseAttributesWatch::ImageRating:
            emit signalImageRatingChanged(imageId);
            break;
        case DatabaseAttributesWatch::ImageTags:
            emit signalImageTagsChanged(imageId);
            break;
    }
}

/*
void ImageAttributesWatch::imageTagsChanged(qint64 imageId)
{
    emit signalImageTagsChanged(imageId);
}

void ImageAttributesWatch::imagesChanged(int albumId)
{
    emit signalImagesChanged(albumId);
}

void ImageAttributesWatch::imageRatingChanged(qint64 imageId)
{
    emit signalImageRatingChanged(imageId);
}

void ImageAttributesWatch::imageDateChanged(qint64 imageId)
{
    emit signalImageDateChanged(imageId);
}

void ImageAttributesWatch::imageCaptionChanged(qint64 imageId)
{
    emit signalImageCaptionChanged(imageId);
}
*/

void ImageAttributesWatch::fileMetadataChanged(const KUrl &url)
{
    emit signalFileMetadataChanged(url);
}

} // namespace Digikam
