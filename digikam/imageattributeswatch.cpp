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

#include "databaseattributeswatch.h"
#include "imageattributeswatch.h"

namespace Digikam
{

ImageAttributesWatch *ImageAttributesWatch::m_instance = 0;

ImageAttributesWatch::ImageAttributesWatch()
{
    // temporary solution
    DatabaseAttributesWatch *dbwatch = DatabaseAttributesWatch::instance();

    connect(dbwatch, SIGNAL(signalImageTagsChanged(Q_LLONG)),
            this, SIGNAL(signalImageTagsChanged(Q_LLONG)));

    connect(dbwatch, SIGNAL(signalImagesChanged(int)),
            this, SIGNAL(signalImagesChanged(int)));

    connect(dbwatch, SIGNAL(signalImageRatingChanged(Q_LLONG)),
            this, SIGNAL(signalImageRatingChanged(Q_LLONG)));

    connect(dbwatch, SIGNAL(signalImageDateChanged(Q_LLONG)),
            this, SIGNAL(signalImageDateChanged(Q_LLONG)));

    connect(dbwatch, SIGNAL(signalImageCaptionChanged(Q_LLONG)),
            this, SIGNAL(signalImageCaptionChanged(Q_LLONG)));
}

ImageAttributesWatch::~ImageAttributesWatch()
{
    m_instance = 0;
}

void ImageAttributesWatch::cleanUp()
{
    DatabaseAttributesWatch::cleanUp();
    delete m_instance;
    m_instance = 0;
}

void ImageAttributesWatch::shutDown()
{
    ImageAttributesWatch::shutDown();
    if (m_instance)
        m_instance->disconnect(0, 0, 0);
}

ImageAttributesWatch *ImageAttributesWatch::instance()
{
    if (!m_instance)
        m_instance = new ImageAttributesWatch;
    return m_instance;
}

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

void ImageAttributesWatch::fileMetadataChanged(const KURL &url)
{
    emit signalFileMetadataChanged(url);
}

} // namespace Digikam

#include "imageattributeswatch.moc"
