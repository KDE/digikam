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

#include "coredbaccess.h"
#include "coredb.h"
#include "coredbfields.h"
#include "dmetadata.h"
#include "metadatainfo.h"

namespace Digikam
{

class ImagePositionPriv : public QSharedData
{

public:

    ImagePositionPriv() :
        empty(true),
        //Note: Do not initialize the QVariants here, they are expected to be null
        imageId(-1),
        dirtyFields(DatabaseFields::ImagePositionsNone)
    {
    }

    void resetData()
    {
        description.clear();
        latitude.clear();
        longitude.clear();
        latitudeNumber  = QVariant();
        longitudeNumber = QVariant();
        altitude        = QVariant();
        orientation     = QVariant();
        tilt            = QVariant();
        roll            = QVariant();
        empty           = true;
        dirtyFields     = DatabaseFields::ImagePositionsNone;
    }

    bool                           empty;

    QVariant                       latitudeNumber;
    QVariant                       longitudeNumber;
    QVariant                       altitude;
    QVariant                       orientation;
    QVariant                       tilt;
    QVariant                       roll;
    QVariant                       accuracy;

    qlonglong                      imageId;

    QString                        description;
    QString                        latitude;
    QString                        longitude;

    DatabaseFields::ImagePositions dirtyFields;

    void init(CoreDbAccess& access, qlonglong imageId);
};

void ImagePositionPriv::init(CoreDbAccess& access, qlonglong id)
{
    imageId = id;

    QVariantList values = access.db()->getImagePosition(imageId);

    if (values.size() == 10)
    {
        empty           = false;
        latitude        = values.at(0).toString();
        latitudeNumber  = values.at(1);
        longitude       = values.at(2).toString();
        longitudeNumber = values.at(3);
        altitude        = values.at(4);
        orientation     = values.at(5);
        tilt            = values.at(6);
        roll            = values.at(7);
        accuracy        = values.at(8);
        description     = values.at(9).toString();
    }
}

ImagePosition::ImagePosition()
{
}

ImagePosition::ImagePosition(qlonglong imageId)
{
    d = new ImagePositionPriv;
    CoreDbAccess access;
    d->init(access, imageId);
}

ImagePosition::ImagePosition(CoreDbAccess& access, qlonglong imageId)
{
    d = new ImagePositionPriv;
    d->init(access, imageId);
}

ImagePosition::ImagePosition(const ImagePosition& other)
{
    d = other.d;
}

ImagePosition::~ImagePosition()
{
    apply();
}

ImagePosition& ImagePosition::operator=(const ImagePosition& other)
{
    d = other.d;
    return *this;
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
    {
        return QString();
    }

    return d->latitude;
}

QString ImagePosition::longitude() const
{
    if (!d)
    {
        return QString();
    }

    return d->longitude;
}

double ImagePosition::latitudeNumber() const
{
    if (!d)
    {
        return 0;
    }

    return d->latitudeNumber.toDouble();
}

double ImagePosition::longitudeNumber() const
{
    if (!d)
    {
        return 0;
    }

    return d->longitudeNumber.toDouble();
}

QString ImagePosition::latitudeFormatted() const
{
    if (!d)
    {
        return QString();
    }

    return DMetadata::valueToString(d->latitude, MetadataInfo::Latitude);
}

QString ImagePosition::longitudeFormatted() const
{
    if (!d)
    {
        return QString();
    }

    return DMetadata::valueToString(d->longitude, MetadataInfo::Longitude);
}

bool ImagePosition::latitudeUserPresentableNumbers(int* degrees, int* minutes, double* seconds, char* directionReference)
{
    if (!d)
    {
        return false;
    }

    return DMetadata::convertToUserPresentableNumbers(d->latitude, degrees, minutes, seconds, directionReference);
}

bool ImagePosition::longitudeUserPresentableNumbers(int* degrees, int* minutes, double* seconds, char* directionReference)
{
    if (!d)
    {
        return false;
    }

    return DMetadata::convertToUserPresentableNumbers(d->longitude, degrees, minutes, seconds, directionReference);
}

double ImagePosition::altitude() const
{
    if (!d)
    {
        return 0;
    }

    return d->altitude.toDouble();
}

QString ImagePosition::altitudeFormatted() const
{
    if (!d)
    {
        return QString();
    }

    return DMetadata::valueToString(d->altitude, MetadataInfo::Altitude);
}

double ImagePosition::orientation() const
{
    if (!d)
    {
        return 0;
    }

    return d->orientation.toDouble();
}

double ImagePosition::tilt() const
{
    if (!d)
    {
        return 0;
    }

    return d->tilt.toDouble();
}

double ImagePosition::roll() const
{
    if (!d)
    {
        return 0;
    }

    return d->roll.toDouble();
}

double ImagePosition::accuracy() const
{
    if (!d)
    {
        return 0;
    }

    return d->accuracy.toDouble();
}

QString ImagePosition::description() const
{
    if (!d)
    {
        return QString();
    }

    return d->description;
}

bool ImagePosition::hasCoordinates() const
{
    return d && !d->latitudeNumber.isNull() && !d->longitudeNumber.isNull();
}

bool ImagePosition::hasAltitude() const
{
    return d && !d->altitude.isNull();
}

bool ImagePosition::hasOrientation() const
{
    return d && !d->orientation.isNull();
}

bool ImagePosition::hasTilt() const
{
    return d && !d->tilt.isNull();
}

bool ImagePosition::hasRoll() const
{
    return d && !d->roll.isNull();
}

bool ImagePosition::hasAccuracy() const
{
    return d && !d->accuracy.isNull();
}

bool ImagePosition::setLatitude(const QString& latitude)
{
    if (!d)
    {
        return false;
    }

    double number;

    if (!DMetadata::convertFromGPSCoordinateString(latitude, &number))
    {
        return false;
    }

    d->latitude = latitude;
    d->latitudeNumber = number;
    d->dirtyFields |= DatabaseFields::Latitude | DatabaseFields::LatitudeNumber;
    return true;
}

bool ImagePosition::setLongitude(const QString& longitude)
{
    if (!d)
    {
        return false;
    }

    double number;

    if (!DMetadata::convertFromGPSCoordinateString(longitude, &number))
    {
        return false;
    }

    d->longitude = longitude;
    d->longitudeNumber = number;
    d->dirtyFields |= DatabaseFields::Longitude | DatabaseFields::LongitudeNumber;
    return true;
}

bool ImagePosition::setLatitude(double latitudeNumber)
{
    if (!d)
    {
        return false;
    }

    QString string = DMetadata::convertToGPSCoordinateString(true, latitudeNumber);

    if (string.isNull())
    {
        return false;
    }

    d->latitude = string;
    d->latitudeNumber = latitudeNumber;
    d->dirtyFields |= DatabaseFields::Latitude | DatabaseFields::LatitudeNumber;
    return true;
}

bool ImagePosition::setLongitude(double longitudeNumber)
{
    if (!d)
    {
        return false;
    }

    QString string = DMetadata::convertToGPSCoordinateString(false, longitudeNumber);

    if (string.isNull())
    {
        return false;
    }

    d->longitude = string;
    d->longitudeNumber = longitudeNumber;
    d->dirtyFields |= DatabaseFields::Longitude | DatabaseFields::LongitudeNumber;
    return true;
}

void ImagePosition::setAltitude(double altitude)
{
    if (!d)
    {
        return;
    }

    d->altitude = altitude;
    d->dirtyFields |= DatabaseFields::Altitude;
}

void ImagePosition::setOrientation(double orientation)
{
    if (!d)
    {
        return;
    }

    d->orientation = orientation;
    d->dirtyFields |= DatabaseFields::PositionOrientation;
}

void ImagePosition::setTilt(double tilt)
{
    if (!d)
    {
        return;
    }

    d->tilt = tilt;
    d->dirtyFields |= DatabaseFields::PositionTilt;
}

void ImagePosition::setRoll(double roll)
{
    if (!d)
    {
        return;
    }

    d->roll = roll;
    d->dirtyFields |= DatabaseFields::PositionRoll;
}

void ImagePosition::setAccuracy(double accuracy)
{
    if (!d)
    {
        return;
    }

    d->accuracy = accuracy;
    d->dirtyFields |= DatabaseFields::PositionAccuracy;
}

void ImagePosition::setDescription(const QString& description)
{
    if (!d)
    {
        return;
    }

    d->description = description;
    d->dirtyFields |= DatabaseFields::PositionDescription;
}

void ImagePosition::apply()
{
    if (!d)
    {
        return;
    }

    if (d->dirtyFields == DatabaseFields::ImagePositionsNone)
    {
        return;
    }

    QVariantList values;

    if (d->dirtyFields & DatabaseFields::Latitude)
    {
        values << d->latitude;
    }

    if (d->dirtyFields & DatabaseFields::LatitudeNumber)
    {
        values << d->latitudeNumber;
    }

    if (d->dirtyFields & DatabaseFields::Longitude)
    {
        values << d->longitude;
    }

    if (d->dirtyFields & DatabaseFields::LongitudeNumber)
    {
        values << d->longitudeNumber;
    }

    if (d->dirtyFields & DatabaseFields::Altitude)
    {
        values << d->altitude;
    }

    if (d->dirtyFields & DatabaseFields::PositionOrientation)
    {
        values << d->orientation;
    }

    if (d->dirtyFields & DatabaseFields::PositionTilt)
    {
        values << d->tilt;
    }

    if (d->dirtyFields & DatabaseFields::PositionRoll)
    {
        values << d->roll;
    }

    if (d->dirtyFields & DatabaseFields::PositionAccuracy)
    {
        values << d->accuracy;
    }

    if (d->dirtyFields & DatabaseFields::PositionDescription)
    {
        values << d->description;
    }

    if (d->empty)
    {
        CoreDbAccess().db()->addImagePosition(d->imageId, values, d->dirtyFields);
        d->empty = false;
    }
    else
    {
        CoreDbAccess().db()->changeImagePosition(d->imageId, values, d->dirtyFields);
    }

    d->dirtyFields = DatabaseFields::ImagePositionsNone;
}

void ImagePosition::remove()
{
    CoreDbAccess().db()->removeImagePosition(d->imageId);
    d->resetData();
}

void ImagePosition::removeAltitude()
{
    CoreDbAccess().db()->removeImagePositionAltitude(d->imageId);
    d->dirtyFields &= ~DatabaseFields::Altitude;
    d->altitude = QVariant();
}

} // namespace Digikam
