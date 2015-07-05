/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-05
 * Description : Access to the properties of an Image / Tag pair, i.e., a tag associated to an image
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGETAGPAIR_H
#define IMAGETAGPAIR_H

// Qt includes

#include <QString>
#include <QStringList>
#include <QList>
#include <QExplicitlySharedDataPointer>
#include <QMap>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class ImageInfo;
class ImageTagPairPriv;

class DIGIKAM_DATABASE_EXPORT ImageTagPair
{
public:

    /** This class provides a wrapper over the Database methods
     *  to access the properties of tag / image assocation. It is meant to be a
     *  short-lived object, it does not listen to external database changes.
     */

    /// Creates a null pair
    ImageTagPair();

    /** Access the properties of the given image - tag pair */
    ImageTagPair(qlonglong imageId, int tagId);
    ImageTagPair(const ImageInfo& info, int tagId);

    ~ImageTagPair();
    ImageTagPair(const ImageTagPair& other);
    ImageTagPair& operator=(const ImageTagPair& other);

    bool isNull() const;

    /**
     * Return all pairs for the given image for which entries exist.
     * This list of tags may not be identical to the tags assigned to the image.
     */
    static QList<ImageTagPair> availablePairs(qlonglong imageId);
    static QList<ImageTagPair> availablePairs(const ImageInfo& info);

    qlonglong imageId() const;
    int tagId() const;

    /** Returns if the tag is assigned to the image */
    bool isAssigned() const;

    /** Assigns the tag to the image */
    void assignTag();

    /** Removes the tag from the image */
    void unAssignTag();

    /// Returns true if the property is set
    bool hasProperty(const QString& key) const;
    /// Returns true if any of the properties is set
    bool hasAnyProperty(const QStringList& keys) const;
    /// Returns true of the given property and value is set
    bool hasValue(const QString& key, const QString& value) const;
    /// Returns the value of the given property, or a null string if not set
    QString value(const QString& key) const;
    /// Returns value() concatenated for all given keys
    QStringList allValues(const QStringList& keys) const;
    /// Returns a list of values with the given property
    QStringList values(const QString& key) const;
    /// Returns all set property keys
    QStringList propertyKeys() const;
    /// Returns a map of all key->value pairs
    QMap<QString, QString> properties() const;

    /// Set the given property. Replaces all previous occurrences of this property.
    void setProperty(const QString& key, const QString& value);
    /** Adds the given property. Does not change any previous occurrences of this property,
     *  allowing multiple properties with the same key.
     *  (duplicates of same key _and_ value are not added, though) */
    void addProperty(const QString& key, const QString& value);
    /// Remove all occurrences of the property
    void removeProperty(const QString& key, const QString& value);
    /// Remove all occurrences of the property
    void removeProperties(const QString& key);
    /// Removes all properties
    void clearProperties();

private:

    QExplicitlySharedDataPointer<ImageTagPairPriv> d;
};

} // namespace

#endif // IMAGETAGPAIR_H

