/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-27
 * Description : structure for info stored about a face in the database
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASEFACE_H
#define DATABASEFACE_H

// Qt includes

#include <QFlags>
#include <QVariant>

// Local includes

#include "tagregion.h"
#include "digikam_export.h"

class QDebug;

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT DatabaseFace
{
public:

    enum Type
    {
        InvalidFace      = 0,
        UnknownName      = 1 << 0,
        UnconfirmedName  = 1 << 1,
        ConfirmedName    = 1 << 2,
        FaceForTraining  = 1 << 3,

        UnconfirmedTypes = UnknownName | UnconfirmedName,
        NormalFaces      = UnknownName | UnconfirmedName | ConfirmedName,
        AllTypes         = UnknownName | UnconfirmedName | ConfirmedName | FaceForTraining,
        TypeFirst        = UnknownName,
        TypeLast         = FaceForTraining
    };
    Q_DECLARE_FLAGS(TypeFlags, Type)

public:

    DatabaseFace();
    DatabaseFace(Type type, qlonglong imageId, int tagId, const TagRegion& region);
    DatabaseFace(const QString& attribute, qlonglong imageId, int tagId, const TagRegion& region);

    bool      isNull() const;

    Type      type()    const;
    qlonglong imageId() const;
    int       tagId()   const;
    TagRegion region()  const;

    bool      isUnknownName() const
    {
        return type() == UnknownName;
    }

    bool      isUnconfirmedName() const
    {
        return type() == UnconfirmedName;
    }

    bool      isUnconfirmedType() const
    {
        return type() & UnconfirmedTypes;
    }

    bool      isConfirmedName() const
    {
        return type() == ConfirmedName;
    }

    bool      isForTraining() const
    {
        return type() == FaceForTraining;
    }

    void setType(Type type);
    void setTagId(int tagId);
    void setRegion(const TagRegion& region);

    bool operator==(const DatabaseFace& other) const;

    /// Returns a list of all image tag properties for which flags are set
    static QStringList attributesForFlags(TypeFlags flags);
    /// Return the corresponding image tag property for the given type
    static QString     attributeForType(Type type);
    /**
     * Return the Type for the given attribute. To distinguish between UnknownName
     * and UnconfirmedName, the tagId must be given.
     */
    static Type        typeForAttribute(const QString& attribute, int tagId = 0);

    /**
     * Writes the contents of this face - in a compact way - in the QVariant.
     * Only native QVariant types are used, that is, the QVariant will not have a custom type,
     * thus it can be compared by value by operator==.
     */
    static DatabaseFace fromVariant(const QVariant& var);
    QVariant toVariant() const;

    /**
     * Create a DatabaseFace from the extraValues returned from ImageLister.
     */
    static DatabaseFace fromListing(qlonglong imageid, const QList<QVariant>& values);

protected:

    Type      m_type;
    qlonglong m_imageId;
    int       m_tagId;
    TagRegion m_region;
};

DIGIKAM_DATABASE_EXPORT QDebug operator<<(QDebug dbg, const DatabaseFace& f);

}  // Namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DatabaseFace::TypeFlags)

#endif // FACEIFACE_H
