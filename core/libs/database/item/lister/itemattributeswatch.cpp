/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

#include "itemattributeswatch.h"

// Local includes

#include "coredbaccess.h"
#include "coredbwatch.h"

namespace Digikam
{

ItemAttributesWatch* ItemAttributesWatch::m_instance = nullptr;

ItemAttributesWatch::ItemAttributesWatch()
{
    CoreDbWatch* const dbwatch = CoreDbAccess::databaseWatch();

    connect(dbwatch, SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotImageChange(ImageChangeset)));

    connect(dbwatch, SIGNAL(imageTagChange(ImageTagChangeset)),
            this, SLOT(slotImageTagChange(ImageTagChangeset)));
}

ItemAttributesWatch::~ItemAttributesWatch()
{
    m_instance = nullptr;
}

void ItemAttributesWatch::cleanUp()
{
    delete m_instance;
    m_instance = nullptr;
}

void ItemAttributesWatch::shutDown()
{
    if (m_instance)
    {
        m_instance->disconnect(nullptr, nullptr, nullptr);
    }
}

ItemAttributesWatch* ItemAttributesWatch::instance()
{
    if (!m_instance)
    {
        m_instance = new ItemAttributesWatch;
    }

    return m_instance;
}

void ItemAttributesWatch::slotImageChange(const ImageChangeset& changeset)
{
    DatabaseFields::Set set = changeset.changes();

    if ((set & DatabaseFields::ItemCommentsAll) ||
        (set & DatabaseFields::CreationDate)     ||
        (set & DatabaseFields::ModificationDate) ||
        (set & DatabaseFields::Rating))
    {
        foreach (const qlonglong& imageId, changeset.ids())
        {
            if (set & DatabaseFields::ItemCommentsAll)
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

void ItemAttributesWatch::slotImageTagChange(const ImageTagChangeset& changeset)
{
    foreach (const qlonglong& imageId, changeset.ids())
    {
        emit signalImageTagsChanged(imageId);
    }
}

void ItemAttributesWatch::fileMetadataChanged(const QUrl& url)
{
    emit signalFileMetadataChanged(url);
}

/*

void ItemAttributesWatch::slotImageFieldChanged(qlonglong imageId, int field)
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

void ItemAttributesWatch::imageTagsChanged(qint64 imageId)
{
    emit signalImageTagsChanged(imageId);
}

void ItemAttributesWatch::imagesChanged(int albumId)
{
    emit signalImagesChanged(albumId);
}

void ItemAttributesWatch::imageRatingChanged(qint64 imageId)
{
    emit signalImageRatingChanged(imageId);
}

void ItemAttributesWatch::imageDateChanged(qint64 imageId)
{
    emit signalImageDateChanged(imageId);
}

void ItemAttributesWatch::imageCaptionChanged(qint64 imageId)
{
    emit signalImageCaptionChanged(imageId);
}

*/

} // namespace Digikam
