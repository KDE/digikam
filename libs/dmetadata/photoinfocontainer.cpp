/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-21
 * Description : main photograph information container
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "photoinfocontainer.h"

// Qt includes

#include <QtCore/QDataStream>

namespace Digikam
{

QDataStream& operator<<(QDataStream& ds, const PhotoInfoContainer& info)
{
    ds << info.make;
    ds << info.model;
    ds << info.lens;
    ds << info.exposureTime;
    ds << info.exposureMode;
    ds << info.exposureProgram;
    ds << info.aperture;
    ds << info.focalLength;
    ds << info.focalLength35mm;
    ds << info.sensitivity;
    ds << info.flash;
    ds << info.whiteBalance;
    ds << info.dateTime;

    return ds;
}

QDataStream& operator>>(QDataStream& ds, PhotoInfoContainer& info)
{
    ds >> info.make;
    ds >> info.model;
    ds >> info.lens;
    ds >> info.exposureTime;
    ds >> info.exposureMode;
    ds >> info.exposureProgram;
    ds >> info.aperture;
    ds >> info.focalLength;
    ds >> info.focalLength35mm;
    ds >> info.sensitivity;
    ds >> info.flash;
    ds >> info.whiteBalance;
    ds >> info.dateTime;

    return ds;
}

} // namespace Digikam
