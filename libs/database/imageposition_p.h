/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-10-26
 * Description : Access image position stored in database.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2008 by Patrick Spendrin <ps_ml at gmx dot de>
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

// Qt includes

#include <QString>
#include <QSharedData>

// Local includes

#include "databasefields.h"

namespace Digikam
{

class ImagePositionPriv : public QSharedData
{

public:

    ImagePositionPriv()
    {
        imageId         = -1;
        latitudeNumber  = 0;
        longitudeNumber = 0;
        altitude        = 0;
        orientation     = 0;
        tilt            = 0;
        roll            = 0;
        accuracy        = 0;
        empty           = true;
        dirtyFields     = DatabaseFields::ImagePositionsNone;
    }

    void resetData()
    {
        description     = QString();
        latitude        = QString();
        longitude       = QString();
        latitudeNumber  = 0;
        longitudeNumber = 0;
        altitude        = 0;
        orientation     = 0;
        tilt            = 0;
        roll            = 0;
        empty           = true;
        dirtyFields     = DatabaseFields::ImagePositionsNone;
    }

    bool                           empty;

    double                         latitudeNumber;
    double                         longitudeNumber;
    double                         altitude;
    double                         orientation;
    double                         tilt;
    double                         roll;
    double                         accuracy;

    qlonglong                      imageId;

    QString                        description;
    QString                        latitude;
    QString                        longitude;


    DatabaseFields::ImagePositions dirtyFields;
};

} // namespace Digikam
