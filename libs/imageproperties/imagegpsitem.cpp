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

//#include "imagegpsitem.moc"

// local include

#include "imagegpsitem.h"

namespace Digikam
{

ImageGPSItem::ImageGPSItem(const GPSInfo& gpsInfo)
{
    m_gpsInfo = gpsInfo;
}

ImageGPSItem::~ImageGPSItem()
{
}


QVariant ImageGPSItem::data(int role) const
{
    if (role == RoleGPSInfo)
    {
        return QVariant::fromValue(m_gpsInfo);
    }

    return QStandardItem::data(role);

}

void ImageGPSItem::setData(const QVariant& value, int role)
{
    if (role == RoleGPSInfo)
    {
        if (value.canConvert<GPSInfo>())
        {
            m_gpsInfo = value.value<GPSInfo>();
        }

        return;
    }

    QStandardItem::setData(value, role);
}

int ImageGPSItem::type() const
{
    return RoleGPSInfo;
}

} //namespace Digikam
