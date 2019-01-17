/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-01
 * Description : Access item position stored in database.
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

#include "itemposition.h"

// Local includes

#include "coredbaccess.h"
#include "coredb.h"
#include "coredbfields.h"
#include "dmetadata.h"
#include "metadatainfo.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemPositionPriv : public QSharedData
{

public:

    ItemPositionPriv() :
        empty(true),
        //Note: Do not initialize the QVariants here, they are expected to be null
        imageId(-1),
        dirtyFields(DatabaseFields::ItemPositionsNone)
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
        dirtyFields     = DatabaseFields::ItemPositionsNone;
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

    DatabaseFields::ItemPositions dirtyFields;

    void init(CoreDbAccess& access, qlonglong imageId);
};

void ItemPositionPriv::init(CoreDbAccess& access, qlonglong id)
{
    imageId = id;

    QVariantList values = access.db()->getItemPosition(imageId);

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

// ------------------------------------------------------

ItemPosition::ItemPosition()
{
}

ItemPosition::ItemPosition(qlonglong imageId)
{
    d = new ItemPositionPriv;
    CoreDbAccess access;
    d->init(access, imageId);
}

ItemPosition::ItemPosition(CoreDbAccess& access, qlonglong imageId)
{
    d = new ItemPositionPriv;
    d->init(access, imageId);
}

ItemPosition::ItemPosition(const ItemPosition& other)
{
    d = other.d;
}

ItemPosition::~ItemPosition()
{
    apply();
}

ItemPosition& ItemPosition::operator=(const ItemPosition& other)
{
    d = other.d;
    return *this;
}

bool ItemPosition::isNull() const
{
    return !d;
}

bool ItemPosition::isEmpty() const
{
    return !d || d->empty;
}

QString ItemPosition::latitude() const
{
    if (!d)
    {
        return QString();
    }

    return d->latitude;
}

QString ItemPosition::longitude() const
{
    if (!d)
    {
        return QString();
    }

    return d->longitude;
}

double ItemPosition::latitudeNumber() const
{
    if (!d)
    {
        return 0;
    }

    return d->latitudeNumber.toDouble();
}

double ItemPosition::longitudeNumber() const
{
    if (!d)
    {
        return 0;
    }

    return d->longitudeNumber.toDouble();
}

QString ItemPosition::latitudeFormatted() const
{
    if (!d)
    {
        return QString();
    }

    return DMetadata::valueToString(d->latitude, MetadataInfo::Latitude);
}

QString ItemPosition::longitudeFormatted() const
{
    if (!d)
    {
        return QString();
    }

    return DMetadata::valueToString(d->longitude, MetadataInfo::Longitude);
}

bool ItemPosition::latitudeUserPresentableNumbers(int* degrees, int* minutes, double* seconds, char* directionReference)
{
    if (!d)
    {
        return false;
    }

    return DMetadata::convertToUserPresentableNumbers(d->latitude, degrees, minutes, seconds, directionReference);
}

bool ItemPosition::longitudeUserPresentableNumbers(int* degrees, int* minutes, double* seconds, char* directionReference)
{
    if (!d)
    {
        return false;
    }

    return DMetadata::convertToUserPresentableNumbers(d->longitude, degrees, minutes, seconds, directionReference);
}

double ItemPosition::altitude() const
{
    if (!d)
    {
        return 0;
    }

    return d->altitude.toDouble();
}

QString ItemPosition::altitudeFormatted() const
{
    if (!d)
    {
        return QString();
    }

    return DMetadata::valueToString(d->altitude, MetadataInfo::Altitude);
}

double ItemPosition::orientation() const
{
    if (!d)
    {
        return 0;
    }

    return d->orientation.toDouble();
}

double ItemPosition::tilt() const
{
    if (!d)
    {
        return 0;
    }

    return d->tilt.toDouble();
}

double ItemPosition::roll() const
{
    if (!d)
    {
        return 0;
    }

    return d->roll.toDouble();
}

double ItemPosition::accuracy() const
{
    if (!d)
    {
        return 0;
    }

    return d->accuracy.toDouble();
}

QString ItemPosition::description() const
{
    if (!d)
    {
        return QString();
    }

    return d->description;
}

bool ItemPosition::hasCoordinates() const
{
    return d && !d->latitudeNumber.isNull() && !d->longitudeNumber.isNull();
}

bool ItemPosition::hasAltitude() const
{
    return d && !d->altitude.isNull();
}

bool ItemPosition::hasOrientation() const
{
    return d && !d->orientation.isNull();
}

bool ItemPosition::hasTilt() const
{
    return d && !d->tilt.isNull();
}

bool ItemPosition::hasRoll() const
{
    return d && !d->roll.isNull();
}

bool ItemPosition::hasAccuracy() const
{
    return d && !d->accuracy.isNull();
}

bool ItemPosition::setLatitude(const QString& latitude)
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

bool ItemPosition::setLongitude(const QString& longitude)
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

bool ItemPosition::setLatitude(double latitudeNumber)
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

bool ItemPosition::setLongitude(double longitudeNumber)
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

void ItemPosition::setAltitude(double altitude)
{
    if (!d)
    {
        return;
    }

    d->altitude = altitude;
    d->dirtyFields |= DatabaseFields::Altitude;
}

void ItemPosition::setOrientation(double orientation)
{
    if (!d)
    {
        return;
    }

    d->orientation = orientation;
    d->dirtyFields |= DatabaseFields::PositionOrientation;
}

void ItemPosition::setTilt(double tilt)
{
    if (!d)
    {
        return;
    }

    d->tilt = tilt;
    d->dirtyFields |= DatabaseFields::PositionTilt;
}

void ItemPosition::setRoll(double roll)
{
    if (!d)
    {
        return;
    }

    d->roll = roll;
    d->dirtyFields |= DatabaseFields::PositionRoll;
}

void ItemPosition::setAccuracy(double accuracy)
{
    if (!d)
    {
        return;
    }

    d->accuracy = accuracy;
    d->dirtyFields |= DatabaseFields::PositionAccuracy;
}

void ItemPosition::setDescription(const QString& description)
{
    if (!d)
    {
        return;
    }

    d->description = description;
    d->dirtyFields |= DatabaseFields::PositionDescription;
}

void ItemPosition::apply()
{
    if (!d)
    {
        return;
    }

    if (d->dirtyFields == DatabaseFields::ItemPositionsNone)
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
        CoreDbAccess().db()->addItemPosition(d->imageId, values, d->dirtyFields);
        d->empty = false;
    }
    else
    {
        CoreDbAccess().db()->changeItemPosition(d->imageId, values, d->dirtyFields);
    }

    d->dirtyFields = DatabaseFields::ItemPositionsNone;
}

void ItemPosition::remove()
{
    CoreDbAccess().db()->removeItemPosition(d->imageId);
    d->resetData();
}

void ItemPosition::removeAltitude()
{
    CoreDbAccess().db()->removeItemPositionAltitude(d->imageId);
    d->dirtyFields &= ~DatabaseFields::Altitude;
    d->altitude = QVariant();
}

} // namespace Digikam
