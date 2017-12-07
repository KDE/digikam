/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-01
 * Description : Access image position stored in database.
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Patrick Spendrin <ps_ml at gmx dot de>
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

#ifndef IMAGEPOSITION_H
#define IMAGEPOSITION_H

// Qt includes

#include <QString>
#include <QSharedDataPointer>
#include <QSharedData>

// Local includes

#include "digikam_export.h"
#include "coredbfields.h"

namespace Digikam
{

class CoreDbAccess;
class ImagePositionPriv;

class DIGIKAM_DATABASE_EXPORT ImagePosition
{

public:

    /**
     * Creates a null ImagePosition object
     */
    ImagePosition();
    /**
     * Creates an ImagePosition object for the given image.
     * The information is read from the database.
     */
    explicit ImagePosition(qlonglong imageId);
    ImagePosition(CoreDbAccess& access, qlonglong imageId);

    ImagePosition(const ImagePosition& other);
    ~ImagePosition();

    ImagePosition& operator=(const ImagePosition& other);

    bool isNull() const;
    /**
     * An object is empty if no entry exists in the ImagePosition
     * table for the referenced image, or if the object is null.
     * An empty object is empty even if values have been set;
     * it becomes not empty after calling apply().
     */
    bool isEmpty() const;

    /** Returns latitude/longitude in the format as described by
     *  the XMP specification as "GPSCoordinate":
     *  A Text value in the form ?DDD,MM,SSk? or ?DDD,MM.mmk?.
     *  This provides lossless storage.
     */
    QString latitude() const;
    QString longitude() const;

    /**
     * Returns latitude/longitude as a double in degrees.
     * North and East have a positive sign, South and West negative.
     * This provides high precision, with the usual floating point
     * concerns, and possible problems finding the exact text form when
     * converting _back_ to fractions.
     */
    double latitudeNumber() const;
    double longitudeNumber() const;

    /**
     * Returns the latitude/longitude in a user-presentable version,
     * in the form "30°45'55.123'' East"
     */
    QString latitudeFormatted() const;
    QString longitudeFormatted() const;

    /**
     * Returns latitude/longitude as user-presentable numbers.
     * This means that degrees and minutes are integer, the seconds fractional.
     * Direction reference is 'N'/'S', 'E'/'W' resp.
     * This is for the purpose of presenting to the user, there are no guarantees on precision.
     * Returns true if the values have been changed.
     */
    bool latitudeUserPresentableNumbers(int* degrees, int* minutes, double* seconds, char* directionReference);
    bool longitudeUserPresentableNumbers(int* degrees, int* minutes, double* seconds, char* directionReference);

    /** The altitude in meters
     */
    double altitude() const;

    /**
     * Returns the altitude formatted in a user-presentable way in the form "43.45m"
     */
    QString altitudeFormatted() const;
    double orientation() const;
    double tilt() const;
    double roll() const;
    double accuracy() const;
    QString description() const;

    bool hasCoordinates() const;
    bool hasAltitude() const;
    bool hasOrientation() const;
    bool hasTilt() const;
    bool hasRoll() const;
    bool hasAccuracy() const;

    /**
     * Sets the latitude/longitude from the GPSCoordinate string as described by XMP.
     * Returns true if the format is accepted.
     */
    bool setLatitude(const QString& latitude);
    bool setLongitude(const QString& longitude);

    /**
     * Sets the latitude/longitude from a double floating point number,
     * as described for latitudeNumber() above.
     * Returns true if the value is valid and accepted.
     */
    bool setLatitude(double latitudeNumber);
    bool setLongitude(double longitudeNumber);

    /**
     * Set the altitude in meters
     */
    void setAltitude(double altitude);
    void setOrientation(double orientation);
    void setTilt(double tilt);
    void setRoll(double roll);
    void setAccuracy(double accuracy);
    void setDescription(const QString& description);

    /**
     * Apply all changes made to this object.
     * (Also called from desctructor)
     */
    void apply();

    /**
     * Removes the whole data set for the referenced image
     * from the database.
     * This object and any ImagePosition object created later
     * will be empty.
     */
    void remove();

    /**
     * Removes the altitude for the referenced image
     * from the database.
     */
    void removeAltitude();

private:

    QSharedDataPointer<ImagePositionPriv> d;
};

} // namespace Digikam

#endif // IMAGEPOSITION_H
