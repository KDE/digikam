/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-20
 * Description : Data set for image lister
 *
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <qstring.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qsize.h>

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImageListerRecord
{
public:
    ImageListerRecord()
    : imageID(-1), albumID(-1), size(0)
    {
    }

    Q_LLONG    imageID;
    int        albumID;
    QString    name;
    QString    albumName;
    QString    albumRoot;
    QDateTime  dateTime;
    size_t     size;
    QSize      dims;
};

QDataStream &operator<<(QDataStream &os, const ImageListerRecord &record);
QDataStream &operator>>(QDataStream &ds, ImageListerRecord &record);

}


#endif

