/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-27
 * Description : Interface for info stored about a face tag in the database
 *
 * Copyright (C) 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#include "facetagsiface.h"

// Qt includes

#include <QStringList>

// Local includes

#include "digikam_debug.h"
#include "coredbconstants.h"
#include "tagscache.h"

namespace Digikam
{

FaceTagsIface::FaceTagsIface()
    : m_type(InvalidFace),
      m_imageId(0),
      m_tagId(0)
{
}

FaceTagsIface::FaceTagsIface(Type type, qlonglong imageId, int tagId, const TagRegion& region)
    : m_type(type),
      m_imageId(imageId),
      m_tagId(tagId),
      m_region(region)
{
}

FaceTagsIface::FaceTagsIface(const QString& attribute, qlonglong imageId, int tagId, const TagRegion& region)
    : m_imageId(imageId),
      m_tagId(tagId),
      m_region(region)
{
    m_type = typeForAttribute(attribute, tagId);
}

bool FaceTagsIface::isNull() const
{
    return m_type == InvalidFace;
}

FaceTagsIface::Type FaceTagsIface::type() const
{
    return m_type;
}

qlonglong FaceTagsIface::imageId() const
{
    return m_imageId;
}

int FaceTagsIface::tagId() const
{
    return m_tagId;
}

TagRegion FaceTagsIface::region() const
{
    return m_region;
}

void FaceTagsIface::setType(Type type)
{
    m_type = type;
}

void FaceTagsIface::setTagId(int tagId)
{
    m_tagId = tagId;
}

void FaceTagsIface::setRegion(const TagRegion& region)
{
    m_region = region;
}

bool FaceTagsIface::operator==(const FaceTagsIface& other) const
{
    return m_tagId   == other.m_tagId   &&
           m_imageId == other.m_imageId &&
           m_type    == other.m_type    &&
           m_region  == other.m_region;
}

QStringList FaceTagsIface::attributesForFlags(TypeFlags flags)
{
    QStringList attributes;

    for (int i = FaceTagsIface::TypeFirst; i <= FaceTagsIface::TypeLast; i <<= 1)
    {
        if (flags & FaceTagsIface::Type(i))
        {
            QString attribute = attributeForType(FaceTagsIface::Type(i));

            if (!attributes.contains(attribute))
            {
                attributes << attribute;
            }
        }
    }

    return attributes;
}

QString FaceTagsIface::attributeForType(Type type)
{
    if (type == FaceTagsIface::UnknownName || type == FaceTagsIface::UnconfirmedName)
    {
        return ImageTagPropertyName::autodetectedFace();
    }

    if (type == FaceTagsIface::ConfirmedName)
    {
        return ImageTagPropertyName::tagRegion();
    }

    if (type == FaceTagsIface::FaceForTraining)
    {
        return ImageTagPropertyName::faceToTrain();
    }

    return QString();
}

FaceTagsIface::Type FaceTagsIface::typeForAttribute(const QString& attribute, int tagId)
{
    if (attribute == ImageTagPropertyName::autodetectedFace())
    {
        if (tagId && TagsCache::instance()->hasProperty(tagId, TagPropertyName::unknownPerson()))
        {
            return FaceTagsIface::UnknownName;
        }
        else
        {
            return FaceTagsIface::UnconfirmedName;
        }
    }
    else if (attribute == ImageTagPropertyName::tagRegion())
    {
        return FaceTagsIface::ConfirmedName;
    }
    else if (attribute == ImageTagPropertyName::faceToTrain())
    {
        return FaceTagsIface::FaceForTraining;
    }

    return FaceTagsIface::InvalidFace;
}

FaceTagsIface FaceTagsIface::fromVariant(const QVariant& var)
{
    if (var.type() == QVariant::List)
    {
        QList<QVariant> list = var.toList();

        if (list.size() == 4)
        {
            return FaceTagsIface((Type)list.at(0).toInt(),
                                list.at(1).toLongLong(),
                                list.at(2).toInt(),
                                TagRegion::fromVariant(list.at(3)));
        }
    }

    return FaceTagsIface();
}

QVariant FaceTagsIface::toVariant() const
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

FaceTagsIface FaceTagsIface::fromListing(qlonglong imageId, const QList<QVariant>& extraValues)
{
    if (extraValues.size() < 3)
    {
        return FaceTagsIface();
    }

    // See imagelister.cpp: value - property - tagId
    int tagId         = extraValues.at(2).toInt();
    QString attribute = extraValues.at(1).toString();
    QString value     = extraValues.at(0).toString();

    //qCDebug(DIGIKAM_DATABASE_LOG) << tagId << attribute << value;

    return FaceTagsIface(attribute,
                        imageId, tagId,
                        TagRegion(value));
}

QString FaceTagsIface::getAutodetectedPersonString() const
{
    if (isUnconfirmedType())
    {
        return QString::number(tagId()) + QLatin1Char(',') + ImageTagPropertyName::autodetectedFace() + QLatin1Char(',') + (TagRegion(region().toRect())).toXml();
    }
    else
    {
        return QString();
    }
}

QDebug operator<<(QDebug dbg, const FaceTagsIface& f)
{
    dbg.nospace() << "FaceTagsIface(" << f.type()
                  << ", image "       << f.imageId()
                  << ", tag "         << f.tagId()
                  << ", region"       << f.region();
    return dbg;
}

}  // Namespace Digikam
