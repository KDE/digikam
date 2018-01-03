/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-09-24
 * Description : an databse wrapper for GPSImageItem
 *
 * Copyright (C) 2015-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGE_GPS_H
#define IMAGE_GPS_H

// Qt includes

#include <QList>

// Local includes

#include "digikam_export.h"
#include "imageinfo.h"
#include "gpsimageitem.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ImageGPS : public GPSImageItem
{

public:

    ImageGPS(const ImageInfo& info);
    virtual ~ImageGPS();

    QString saveChanges();
    bool loadImageData();

    static QList<GPSImageItem*> infosToItems(const ImageInfoList& infos);

private:

    ImageInfo m_info;
};

} /* namespace Digikam */

#endif /* IMAGE_GPS_H */
