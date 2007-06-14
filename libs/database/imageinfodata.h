/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-01
 * Description : ImageInfo common data
 * 
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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


#ifndef IMAGEINFODATA_H
#define IMAGEINFODATA_H

// Qt includes

#include <q3shared.h>
#include <qdatetime.h>
#include <qsize.h>

// Local includes

#include "databaseurl.h"

namespace Digikam
{

class ImageInfoData : public Q3Shared
{
public:

    ImageInfoData();

    qlonglong       id;
    int           albumId;
    DatabaseUrl   url;

    QString       comment;
    bool          commentValid;
    int           rating;

    QDateTime     dateTime;
    QDateTime     modDateTime;
    uint          fileSize;
    QSize         imageDimension;

};


}

#endif

