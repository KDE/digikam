/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef GALLERY_INFO_H
#define GALLERY_INFO_H

// Qt includes

#include <QList>
#include <QUrl>

// Local includes

#include "album.h"
#include "theme.h"
#include "htmlgalleryconfig.h"

namespace Digikam
{

/**
 * This class stores all the export settings. It is initialized by the
 * Wizard and read by the Generator.
 */
class GalleryInfo : public HTMLGalleryConfig
{
public:

    QString fullFormatString() const
    {
        return getEnumString(QLatin1String("fullFormat"));
    }

    QString thumbnailFormatString() const
    {
        return getEnumString(QLatin1String("thumbnailFormat"));
    }

    QString getThemeParameterValue(const QString& theme, const QString& parameter, const QString& defaultValue) const;

    void setThemeParameterValue(const QString& theme, const QString& parameter, const QString& value);

public:

    AlbumList mCollectionList;

private:

    /**
     * KConfigXT enums are mapped to ints.
     * This method returns the string associated to the enum value.
     */
    QString getEnumString(const QString& itemName) const
    {
        // findItem is not marked const :-(
        GalleryInfo* const that               = const_cast<GalleryInfo*>(this);
        KConfigSkeletonItem* const tmp        = that->findItem(itemName);
        KConfigSkeleton::ItemEnum* const item = dynamic_cast<KConfigSkeleton::ItemEnum*>(tmp);

        Q_ASSERT(item);

        if (!item)
            return QString();

        int value                                                   = item->value();
        QList<KConfigSkeleton::ItemEnum::Choice> lst                = item->choices();
        QList<KConfigSkeleton::ItemEnum::Choice>::ConstIterator it  = lst.constBegin();
        QList<KConfigSkeleton::ItemEnum::Choice>::ConstIterator end = lst.constEnd();

        for (int pos = 0 ; it != end ; ++it, pos++)
        {
            if (pos == value)
            {
                return (*it).name;
            }
        }

        return QString();
    }
};

} // namespace Digikam

#endif // GALLERY_INFO_H
