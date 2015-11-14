/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-23
 * Description : position information keys
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "positionkeys.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "coredbinfocontainers.h"
#include "imageinfo.h"
#include "imageposition.h"

namespace
{
static const QString KEY_LATITUDE(QLatin1String("Latitude"));
static const QString KEY_LONGITUDE(QLatin1String("Longitude"));
static const QString KEY_LATTITUDENUMBER(QLatin1String("LatitudeNumber"));
static const QString KEY_LONGITUDENUMBER(QLatin1String("LongitudeNumber"));
static const QString KEY_LATITUDEFORMATTED(QLatin1String("LatitudeFormatted"));
static const QString KEY_LONGITUDEFORMATTED(QLatin1String("LongitudeFormatted"));
static const QString KEY_ALTITUDE(QLatin1String("Altitude"));
static const QString KEY_ALTITUDEFORMATTED(QLatin1String("AltitudeFormatted"));
static const QString KEY_ORIENTATION(QLatin1String("Orientation"));
static const QString KEY_ROLL(QLatin1String("Roll"));
static const QString KEY_TILT(QLatin1String("Tilt"));
static const QString KEY_ACCURACY(QLatin1String("Accuracy"));
static const QString KEY_DESCRIPTION(QLatin1String("Description"));
}

namespace Digikam
{

PositionKeys::PositionKeys()
    : DbKeysCollection(i18n("Position Information (GPS)"))
{
    addId(KEY_LATITUDE,           i18n("Latitude in the format as described by the XMP specification"));
    addId(KEY_LONGITUDE,          i18n("Longitude in the format as described by the XMP specification"));
    addId(KEY_LATTITUDENUMBER,    i18n("Latitude as double value"));
    addId(KEY_LONGITUDENUMBER,    i18n("Longitude as double value"));
    addId(KEY_LATITUDEFORMATTED,  i18n("Latitude in a human readable form"));
    addId(KEY_LONGITUDEFORMATTED, i18n("Longitude in a human readable form"));
    addId(KEY_ALTITUDE,           i18n("Altitude in meters"));
    addId(KEY_ALTITUDEFORMATTED,  i18n("Altitude in a human readable form"));
    addId(KEY_ORIENTATION,        i18n("Orientation"));
    addId(KEY_ROLL,               i18n("Roll"));        /** @todo better description! */
    addId(KEY_TILT,               i18n("Tilt"));        /** @todo better description! */
    addId(KEY_ACCURACY,           i18n("Accuracy"));    /** @todo better description! */
    addId(KEY_DESCRIPTION,        i18n("Description")); /** @todo better description! */
}

QString PositionKeys::getDbValue(const QString& key, ParseSettings& settings)
{
    ImageInfo info         = ImageInfo::fromUrl(settings.fileUrl);
    ImagePosition position = info.imagePosition();

    QString result;

    if (key == KEY_LATITUDE)
    {
        result = position.latitude().simplified();
    }
    else if (key == KEY_LONGITUDE)
    {
        result = position.longitude().simplified();
    }
    else if (key == KEY_LATTITUDENUMBER)
    {
        result = QString::number(position.latitudeNumber());
    }
    else if (key == KEY_LONGITUDENUMBER)
    {
        result = QString::number(position.longitudeNumber());
    }
    else if (key == KEY_LATITUDEFORMATTED)
    {
        result = position.latitudeFormatted().simplified();
    }
    else if (key == KEY_LONGITUDEFORMATTED)
    {
        result = position.longitudeFormatted().simplified();
    }
    else if (key == KEY_ALTITUDE)
    {
        result = QString::number(position.altitude());
    }
    else if (key == KEY_ALTITUDEFORMATTED)
    {
        result = position.altitudeFormatted().simplified();
    }
    else if (key == KEY_ORIENTATION)
    {
        result = QString::number(position.orientation());
    }
    else if (key == KEY_ROLL)
    {
        result = QString::number(position.roll());
    }
    else if (key == KEY_TILT)
    {
        result = QString::number(position.tilt());
    }
    else if (key == KEY_ACCURACY)
    {
        result = QString::number(position.accuracy());
    }
    else if (key == KEY_DESCRIPTION)
    {
        result = position.description().simplified();
    }

    return result;
}

} // namespace Digikam
