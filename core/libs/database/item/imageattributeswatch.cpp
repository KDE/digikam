/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-05-04
 * Description : Watch image attributes
 *
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imageattributeswatch.h"

// Local includes

#include "coredbaccess.h"
#include "coredbwatch.h"

namespace Digikam
{

ImageAttributesWatch* ImageAttributesWatch::m_instance = 0;

ImageAttributesWatch::ImageAttributesWatch()
{
    CoreDbWatch* const dbwatch = CoreDbAccess::databaseWatch();

    connect(dbwatch, SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChange(ImageChangeset)));

    connect(dbwatch, SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChange(ImageTagChangeset)));
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
    {
        m_instance->disconnect(0, 0, 0);
    }
}

ImageAttributesWatch* ImageAttributesWatch::instance()
{
    if (!m_instance)
    {
        m_instance = new ImageAttributesWatch;
    }

    return m_instance;
}

void ImageAttributesWatch::slotImageChange(const ImageChangeset& changeset)
{
    DatabaseFields::Set set = changeset.changes();

    if ((set & DatabaseFields::ImageCommentsAll) ||
        (set & DatabaseFields::CreationDate)     ||
        (set & DatabaseFields::ModificationDate) ||
        (set & DatabaseFields::Rating))
    {
        foreach(const qlonglong& imageId, changeset.ids())
        {
            if (set & DatabaseFields::ImageCommentsAll)
            {
                emit signalImageCaptionChanged(imageId);
            }

            if ((set & DatabaseFields::CreationDate) ||
                (set & DatabaseFields::ModificationDate))
            {
                emit signalImageDateChanged(imageId);
            }

            if (set & DatabaseFields::Rating)
            {
                emit signalImageRatingChanged(imageId);
            }
        }
    }
}

void ImageAttributesWatch::slotImageTagChange(const ImageTagChangeset& changeset)
{
    foreach(const qlonglong& imageId, changeset.ids())
    {
        emit signalImageTagsChanged(imageId);
    }
}

void ImageAttributesWatch::fileMetadataChanged(const QUrl& url)
{
    emit signalFileMetadataChanged(url);
}

/*

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

} // namespace Digikam
