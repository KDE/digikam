/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-21
 * Description : a class to hold GPS information about an item.
 *
 * Copyright (C) 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef DIGIKAM_ITEM_GPS_H
#define DIGIKAM_ITEM_GPS_H

// Qt includes

#include <QList>

// Local includes

#include "digikam_export.h"
#include "iteminfo.h"
#include "gpsitemcontainer.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ItemGPS : public GPSItemContainer
{

public:

    explicit ItemGPS(const ItemInfo& info);
    virtual ~ItemGPS();

    QString saveChanges();
    bool loadImageData();

    static QList<GPSItemContainer*> infosToItems(const ItemInfoList& infos);

private:

    ItemInfo m_info;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_GPS_H
