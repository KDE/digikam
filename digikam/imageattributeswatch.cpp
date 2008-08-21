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

// Local includes.

#include "imageattributeswatch.h"
#include "imageattributeswatch.moc"

namespace Digikam
{

ImageAttributesWatch *ImageAttributesWatch::m_instance = 0;

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

