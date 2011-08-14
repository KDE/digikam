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

#include "databaseface.h"

// Qt includes

#include <QStringList>

// KDE includes

#include <kdebug.h>

// Local includes

#include "databaseconstants.h"
#include "tagregion.h"
#include "tagscache.h"

namespace Digikam
{

DatabaseFace::DatabaseFace()
    : m_type(InvalidFace), m_imageId(0), m_tagId(0)
{
}

DatabaseFace::DatabaseFace(Type type, qlonglong imageId, int tagId, const TagRegion& region)
    : m_type(type), m_imageId(imageId), m_tagId(tagId), m_region(region)
{
}

DatabaseFace::DatabaseFace(const QString& attribute, qlonglong imageId, int tagId, const TagRegion& region)
    : m_imageId(imageId), m_tagId(tagId), m_region(region)
{
    m_type = typeForAttribute(attribute, tagId);
}

bool DatabaseFace::isNull() const
{
    return m_type == InvalidFace;
}

DatabaseFace::Type DatabaseFace::type() const
{
    return m_type;
}

qlonglong DatabaseFace::imageId() const
{
    return m_imageId;
}

int DatabaseFace::tagId() const
{
    return m_tagId;
}

TagRegion DatabaseFace::region() const
{
    return m_region;
}

void DatabaseFace::setType(Type type)
{
    m_type = type;
}

void DatabaseFace::setTagId(int tagId)
{
    m_tagId = tagId;
}

void DatabaseFace::setRegion(const TagRegion& region)
{
    m_region = region;
}

bool DatabaseFace::operator==(const DatabaseFace& other) const
{
    return m_tagId   == other.m_tagId   &&
           m_imageId == other.m_imageId &&
           m_type    == other.m_type    &&
           m_region  == other.m_region;
}

QStringList DatabaseFace::attributesForFlags(TypeFlags flags)
{
    QStringList attributes;

    for (int i = DatabaseFace::TypeFirst; i <= DatabaseFace::TypeLast; i <<= 1)
    {
        if (flags & DatabaseFace::Type(i))
        {
            QString attribute = attributeForType(DatabaseFace::Type(i));

            if (!attributes.contains(attribute))
            {
                attributes << attribute;
            }
        }
    }

    return attributes;
}

QString DatabaseFace::attributeForType(Type type)
{
    if (type == DatabaseFace::UnknownName || type == DatabaseFace::UnconfirmedName)
    {
        return ImageTagPropertyName::autodetectedFace();
    }

    if (type == DatabaseFace::ConfirmedName)
    {
        return ImageTagPropertyName::tagRegion();
    }

    if (type == DatabaseFace::FaceForTraining)
    {
        return ImageTagPropertyName::faceToTrain();
    }

    return QString();
}

DatabaseFace::Type DatabaseFace::typeForAttribute(const QString& attribute, int tagId)
{
    if (attribute == ImageTagPropertyName::autodetectedFace())
    {
        if (tagId && TagsCache::instance()->hasProperty(tagId, TagPropertyName::unknownPerson()))
        {
            return DatabaseFace::UnknownName;
        }
        else
        {
            return DatabaseFace::UnconfirmedName;
        }
    }
    else if (attribute == ImageTagPropertyName::tagRegion())
    {
        return DatabaseFace::ConfirmedName;
    }
    else if (attribute == ImageTagPropertyName::faceToTrain())
    {
        return DatabaseFace::FaceForTraining;
    }

    return DatabaseFace::InvalidFace;
}

DatabaseFace DatabaseFace::fromVariant(const QVariant& var)
{
    if (var.type() == QVariant::List)
    {
        QList<QVariant> list = var.toList();

        if (list.size() == 4)
        {
            return DatabaseFace((Type)list.at(0).toInt(),
                                list.at(1).toLongLong(),
                                list.at(2).toInt(),
                                TagRegion::fromVariant(list.at(3)));
        }
    }

    return DatabaseFace();
}

QVariant DatabaseFace::toVariant() const
{
    // this is still not perfect, with QList<QVariant> being inefficient
    // we must keep to native types, to make operator== work.
    QList<QVariant> list;
    list << (int)m_type;
    list << m_imageId;
    list << m_tagId;
    list << m_region.toVariant();
    return list;
}

DatabaseFace DatabaseFace::fromListing(qlonglong imageId, const QList<QVariant>& extraValues)
{
    if (extraValues.size() < 3)
    {
        return DatabaseFace();
    }

    // See imagelister.cpp: value - property - tagId
    int tagId         = extraValues.at(2).toInt();
    QString attribute = extraValues.at(1).toString();
    QString value     = extraValues.at(0).toString();
    //kDebug() << tagId << attribute << value;

    return DatabaseFace(attribute,
                        imageId, tagId,
                        TagRegion(value));
}

QDebug operator<<(QDebug dbg, const DatabaseFace& f)
{
    dbg.nospace() << "DatabaseFace(" << f.type()
                  << ", image " << f.imageId()
                  << ", tag " << f.tagId()
                  << ", region" << f.region();
    return dbg;
}

}  // Namespace Digikam
