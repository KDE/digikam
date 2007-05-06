/* ============================================================
 * File  : imageattributeswatch.cpp
 * Author: Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2006-05-04
 * Description : Watch image attributes
 * 
 * Copyright 2006 by Marcel Wiesweg
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

namespace Digikam
{

ImageAttributesWatch *ImageAttributesWatch::m_instance = 0;

ImageAttributesWatch::ImageAttributesWatch()
{
    DatabaseAttributesWatch *dbwatch = DatabaseAccess::attributesWatch();

    connect(dbwatch, SIGNAL(signalImageFieldChanged(Q_LLONG, Digikam::DatabaseAttributesWatch::ImageDataField)),
            this, SLOT(slotImageFieldChanged(Q_LLONG, Digikam::DatabaseAttributesWatch::ImageDataField)));
}

ImageAttributesWatch::~ImageAttributesWatch()
{
    m_instance = 0;
}

void ImageAttributesWatch::cleanUp()
{
    delete m_instance;
    m_instance = 0;
}

void ImageAttributesWatch::shutDown()
{
    if (m_instance)
        m_instance->disconnect(0, 0, 0);
}

ImageAttributesWatch *ImageAttributesWatch::instance()
{
    if (!m_instance)
        m_instance = new ImageAttributesWatch;
    return m_instance;
}

void ImageAttributesWatch::slotImageFieldChanged(Q_LLONG imageId, DatabaseAttributesWatch::ImageDataField field)
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
void ImageAttributesWatch::imageTagsChanged(Q_LLONG imageId)
{
    emit signalImageTagsChanged(imageId);
}

void ImageAttributesWatch::imagesChanged(int albumId)
{
    emit signalImagesChanged(albumId);
}

void ImageAttributesWatch::imageRatingChanged(Q_LLONG imageId)
{
    emit signalImageRatingChanged(imageId);
}

void ImageAttributesWatch::imageDateChanged(Q_LLONG imageId)
{
    emit signalImageDateChanged(imageId);
}

void ImageAttributesWatch::imageCaptionChanged(Q_LLONG imageId)
{
    emit signalImageCaptionChanged(imageId);
}
*/

void ImageAttributesWatch::fileMetadataChanged(const KURL &url)
{
    emit signalFileMetadataChanged(url);
}

} // namespace Digikam

#include "imageattributeswatch.moc"
