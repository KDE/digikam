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
#include <QDebug>

// Local includes

#include "galleryconfig.h"
#include "album.h"

namespace Digikam
{

/**
 * This class stores all the export settings. It is initialized by the
 * Wizard and read by the Generator.
 */
class GalleryInfo : public GalleryConfig
{
public:

    explicit GalleryInfo();
    ~GalleryInfo();

    QString fullFormatString() const;

    QString thumbnailFormatString() const;

    QString getThemeParameterValue(const QString& theme, const QString& parameter,
                                   const QString& defaultValue) const;

    void setThemeParameterValue(const QString& theme, const QString& parameter,
                                const QString& value);

public:

    AlbumList mCollectionList;

private:

    /**
     * KConfigXT enums are mapped to ints.
     * This method returns the string associated to the enum value.
     */
    QString getEnumString(const QString& itemName) const;
};

//! qDebug() stream operator. Writes property @a t to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const GalleryInfo& t);

} // namespace Digikam

#endif // GALLERY_INFO_H
