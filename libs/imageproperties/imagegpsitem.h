/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-05
 * Description : Item model containing GPS information for right-side map
 *
 * Copyright (C) 2010 by Voicu Gabriel <ping dot gabi at gmail dot com>
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de> 
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

#ifndef IMAGEGPSITEM_H
#define IMAGEGPSITEM_H

// Qt includes

#include <QStandardItemModel>
#include <QStandardItem>
#include <QDateTime>

// libkmap includes

#include <libkmap/kmap_primitives.h>

// TODO: move this somewhere central, because it is defined multiple times now
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT GPSInfo
{
public:

    GPSInfo()
      : latitude(0.0),
        longitude(0.0),
        altitude(0.0),
        dateTime(),
        rating(0),
        url(),
        id(-1),
        dimensions(),
        hasAltitude(false)
    {
    };

    double    latitude;
    double    longitude;
    double    altitude;

    QDateTime dateTime;
    int       rating;
    KUrl      url;
    qlonglong id;
    QSize     dimensions;
    bool      hasAltitude;
};

typedef QList<GPSInfo> GPSInfoList;

}
Q_DECLARE_METATYPE(Digikam::GPSInfo)

namespace Digikam
{

const int RoleGPSInfo = Qt::UserRole + 1; 

class ImageGPSItem : public QStandardItem
{

public:

    ImageGPSItem(const GPSInfo& gpsInfo);
    ~ImageGPSItem();

    virtual QVariant data(int role) const;
    virtual void setData(const QVariant& value, int role);
    virtual int type() const;

private:

    GPSInfo m_gpsInfo;
};

} // namespace Digikam

#endif // IMAGEGPSITEM_H
