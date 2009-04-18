/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-01
 * Description : Access image position stored in database.
 *
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

#include "imageposition.h"

// Local includes

#include "databaseaccess.h"
#include "albumdb.h"
#include "databasefields.h"
#include "dmetadata.h"
#include "metadatainfo.h"

namespace Digikam
{

ImagePosition::ImagePosition()
{
}

ImagePosition::ImagePosition(qlonglong imageId)
{
    d = new ImagePositionPriv;
    d->imageId = imageId;

    QVariantList values = DatabaseAccess().db()->getImagePosition(imageId);
    if (values.size() == 10)
    {
        d->empty           = false;
        d->latitude        = values[0].toString();
        d->latitudeNumber  = values[1].toDouble();
        d->longitude       = values[2].toString();
        d->longitudeNumber = values[3].toDouble();
        d->altitude        = values[4].toDouble();
        d->orientation     = values[5].toDouble();
        d->tilt            = values[6].toDouble();
        d->roll            = values[7].toDouble();
        d->accuracy        = values[8].toDouble();
        d->description     = values[9].toString();
    }
}

ImagePosition::ImagePosition(const ImagePosition &other)
{
    d = other.d;
}

ImagePosition::~ImagePosition()
{
    apply();
}

bool ImagePosition::isNull() const
{
    return !d;
}

bool ImagePosition::isEmpty() const
{
    return !d || d->empty;
}

QString ImagePosition::latitude() const
{
    if (!d)
        return QString();

    return d->latitude;
}

QString ImagePosition::longitude() const
{
    if (!d)
        return QString();

    return d->longitude;
}

double ImagePosition::latitudeNumber() const
{
    if (!d)
        return 0;

    return d->latitudeNumber;
}

double ImagePosition::longitudeNumber() const
{
    if (!d)
        return 0;

    return d->longitudeNumber;
}

QString ImagePosition::latitudeFormatted() const
{
    if (!d)
        return QString();

    return DMetadata::valueToString(d->latitude, MetadataInfo::Latitude);
}

QString ImagePosition::longitudeFormatted() const
{
    if (!d)
        return QString();

    return DMetadata::valueToString(d->longitude, MetadataInfo::Longitude);
}

bool ImagePosition::latitudeUserPresentableNumbers(int *degrees, int *minutes, double *seconds, char *directionReference)
{
    if (!d)
        return false;

    return DMetadata::convertToUserPresentableNumbers(d->latitude, degrees, minutes, seconds, directionReference);
}

bool ImagePosition::longitudeUserPresentableNumbers(int *degrees, int *minutes, double *seconds, char *directionReference)
{
    if (!d)
        return false;

    return DMetadata::convertToUserPresentableNumbers(d->longitude, degrees, minutes, seconds, directionReference);
}

double ImagePosition::altitude() const
{
    if (!d)
        return 0;

    return d->altitude;
}

QString ImagePosition::altitudeFormatted() const
{
    if (!d)
        return QString();

    return DMetadata::valueToString(d->altitude, MetadataInfo::Altitude);
}

double ImagePosition::orientation() const
{
    if (!d)
        return 0;

    return d->orientation;
}

double ImagePosition::tilt() const
{
    if (!d)
        return 0;

    return d->tilt;
}

double ImagePosition::roll() const
{
    if (!d)
        return 0;

    return d->roll;
}

double ImagePosition::accuracy() const
{
    if (!d)
        return 0;

    return d->accuracy;
}

QString ImagePosition::description() const
{
    if (!d)
        return QString();

    return d->description;
}

bool ImagePosition::setLatitude(const QString &latitude)
{
    if (!d)
        return false;

    double number;
    if (!DMetadata::convertFromGPSCoordinateString(latitude, &number))
        return false;
    d->latitude = latitude;
    d->latitudeNumber = number;
    d->dirtyFields |= DatabaseFields::Latitude | DatabaseFields::LatitudeNumber;
    return true;
}

bool ImagePosition::setLongitude(const QString longitude)
{
    if (!d)
        return false;

    double number;
    if (!DMetadata::convertFromGPSCoordinateString(longitude, &number))
        return false;
    d->longitude = longitude;
    d->longitudeNumber = number;
    d->dirtyFields |= DatabaseFields::Longitude | DatabaseFields::LongitudeNumber;
    return true;
}

bool ImagePosition::setLatitude(double latitudeNumber)
{
    if (!d)
        return false;

    QString string = DMetadata::convertToGPSCoordinateString(true, latitudeNumber);
    if (string.isNull())
        return false;
    d->latitude = string;
    d->latitudeNumber = latitudeNumber;
    d->dirtyFields |= DatabaseFields::Latitude | DatabaseFields::LatitudeNumber;
    return true;
}

bool ImagePosition::setLongitude(double longitudeNumber)
{
    if (!d)
        return false;

    QString string = DMetadata::convertToGPSCoordinateString(false, longitudeNumber);
    if (string.isNull())
        return false;
    d->longitude = string;
    d->longitudeNumber = longitudeNumber;
    d->dirtyFields |= DatabaseFields::Longitude | DatabaseFields::LongitudeNumber;
    return true;
}

void ImagePosition::setAltitude(double altitude)
{
    if (!d)
        return;

    d->altitude = altitude;
    d->dirtyFields |= DatabaseFields::Altitude;
}

void ImagePosition::setOrientation(double orientation)
{
    if (!d)
        return;

    d->orientation = orientation;
    d->dirtyFields |= DatabaseFields::PositionOrientation;
}

void ImagePosition::setTilt(double tilt)
{
    if (!d)
        return;

    d->tilt = tilt;
    d->dirtyFields |= DatabaseFields::PositionTilt;
}

void ImagePosition::setRoll(double roll)
{
    if (!d)
        return;

    d->roll = roll;
    d->dirtyFields |= DatabaseFields::PositionRoll;
}

void ImagePosition::setAccuracy(double accuracy)
{
    if (!d)
        return;

    d->accuracy = accuracy;
    d->dirtyFields |= DatabaseFields::PositionAccuracy;
}

void ImagePosition::setDescription(const QString &description)
{
    if (!d)
        return;

    d->description = description;
    d->dirtyFields |= DatabaseFields::PositionDescription;
}

void ImagePosition::apply()
{
    if (!d)
        return;

    if (d->dirtyFields == DatabaseFields::ImagePositionsNone)
        return;

    QVariantList values;
    if (d->dirtyFields & DatabaseFields::Latitude)
        values << d->latitude;
    if (d->dirtyFields & DatabaseFields::LatitudeNumber)
        values << d->latitudeNumber;
    if (d->dirtyFields & DatabaseFields::Longitude)
        values << d->longitude;
    if (d->dirtyFields & DatabaseFields::LongitudeNumber)
        values << d->longitudeNumber;
    if (d->dirtyFields & DatabaseFields::Altitude)
        values << d->altitude;
    if (d->dirtyFields & DatabaseFields::PositionOrientation)
        values << d->orientation;
    if (d->dirtyFields & DatabaseFields::PositionTilt)
        values << d->tilt;
    if (d->dirtyFields & DatabaseFields::PositionRoll)
        values << d->roll;
    if (d->dirtyFields & DatabaseFields::PositionAccuracy)
        values << d->accuracy;
    if (d->dirtyFields & DatabaseFields::PositionDescription)
        values << d->description;

    if (d->empty)
    {
        DatabaseAccess().db()->addImagePosition(d->imageId, values, d->dirtyFields);
        d->empty = false;
    }
    else
    {
        DatabaseAccess().db()->changeImagePosition(d->imageId, values, d->dirtyFields);
    }
    d->dirtyFields = DatabaseFields::ImagePositionsNone;
}

void ImagePosition::remove()
{
    DatabaseAccess().db()->removeImagePosition(d->imageId);
    d->resetData();
}

} // namespace Digikam
