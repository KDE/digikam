/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Data set for image lister
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGELISTERRECORD_H
#define IMAGELISTERRECORD_H

// Qt includes

#include <QString>
#include <QDataStream>
#include <QDateTime>
#include <QSize>

// Local includes

#include "digikam_export.h"
#include "coredbalbuminfo.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ImageListerRecord
{

public:

    enum
    {
        MagicValue = 0xD315783F
    };

    enum BinaryFormat
    {
        // keep values constant
        TraditionalFormat = 1,
        ExtraValueFormat  = 2
    };

    BinaryFormat binaryFormat;

    explicit ImageListerRecord(BinaryFormat format = TraditionalFormat)
        : binaryFormat(format)
    {
        imageID                          = -1;
        albumID                          = -1;
        albumRootID                      = -1;
        rating                           = -1;
        fileSize                         = -1;
        currentSimilarity                = 0.0;
        category                         = DatabaseItem::UndefinedCategory;
        currentFuzzySearchReferenceImage = -1;
    }

    int                    albumID;
    int                    albumRootID;
    int                    rating;
    int                    fileSize;

    qlonglong              imageID;
    qlonglong              currentFuzzySearchReferenceImage;

    double                 currentSimilarity;

    QString                format;
    QString                name;

    QDateTime              creationDate;
    QDateTime              modificationDate;

    QSize                  imageSize;

    DatabaseItem::Category category;

    QList<QVariant>        extraValues;

    /** Initializes the beginning of a data packet. For later check with checkStream().
     */
    static void initializeStream(ImageListerRecord::BinaryFormat format, QDataStream& ds);

    /**
     * Checks that the data accessible through the stream complies with
     * the given protocol.
     * Note: No check is possible for the TraditionalFormat, always returns true.
     */
    static bool checkStream(ImageListerRecord::BinaryFormat format, QDataStream& data);

    bool operator==(const ImageListerRecord& record) const
    {
        return this->imageID == record.imageID;
    }
};

DIGIKAM_DATABASE_EXPORT QDataStream& operator<<(QDataStream& os, const ImageListerRecord& record);
DIGIKAM_DATABASE_EXPORT QDataStream& operator>>(QDataStream& ds, ImageListerRecord& record);

}  // namespace Digikam

#endif // IMAGELISTERRECORD_H
