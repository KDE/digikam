/* ============================================================
 * File  : imageattributeswatch.cpp
 * Author: Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2007-03-23
 * Description : Watch database image attributes
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

namespace Digikam
{

DatabaseAttributesWatch *DatabaseAttributesWatch::m_instance = 0;

DatabaseAttributesWatch::~DatabaseAttributesWatch()
{
    m_instance = 0;
}

void DatabaseAttributesWatch::cleanUp()
{
    delete m_instance;
    m_instance = 0;
}

void DatabaseAttributesWatch::shutDown()
{
    if (m_instance)
        m_instance->disconnect(0, 0, 0);
}

DatabaseAttributesWatch *DatabaseAttributesWatch::instance()
{
    if (!m_instance)
        m_instance = new DatabaseAttributesWatch;
    return m_instance;
}

void DatabaseAttributesWatch::imageTagsChanged(Q_LLONG imageId)
{
    emit signalImageTagsChanged(imageId);
}

void DatabaseAttributesWatch::imagesChanged(int albumId)
{
    emit signalImagesChanged(albumId);
}

void DatabaseAttributesWatch::imageRatingChanged(Q_LLONG imageId)
{
    emit signalImageRatingChanged(imageId);
}

void DatabaseAttributesWatch::imageDateChanged(Q_LLONG imageId)
{
    emit signalImageDateChanged(imageId);
}

void DatabaseAttributesWatch::imageCaptionChanged(Q_LLONG imageId)
{
    emit signalImageCaptionChanged(imageId);
}

} // namespace Digikam

#include "databaseattributeswatch.moc"
