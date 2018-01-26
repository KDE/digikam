/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-04
 * Description : Access to extended properties of an image in the database
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017-2018 by Mario Frank    <mario dot frank at uni minus potsdam dot de>
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

#include "imageextendedproperties.h"

// Local includes

#include "coredb.h"
#include "coredbaccess.h"
#include "imagescanner.h"
#include "similaritydbaccess.h"
#include "similaritydb.h"

namespace Digikam
{

ImageExtendedProperties::ImageExtendedProperties(qlonglong imageid)
    : m_id(imageid)
{
}

ImageExtendedProperties::ImageExtendedProperties()
    : m_id(0)
{
}

QString ImageExtendedProperties::intellectualGenre()
{
    return readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreIntellectualGenre));
}

void ImageExtendedProperties::setIntellectualGenre(const QString& intellectualGenre)
{
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreIntellectualGenre), intellectualGenre);
}

void ImageExtendedProperties::removeIntellectualGenre()
{
    setIntellectualGenre(QString());
}

QString ImageExtendedProperties::jobId()
{
    return readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreJobID));
}

void ImageExtendedProperties::setJobId(const QString& jobId)
{
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreJobID), jobId);
}

void ImageExtendedProperties::removeJobId()
{
    setJobId(QString());
}

double ImageExtendedProperties::similarityTo(const qlonglong imageId)
{
    // TODO: extend for additional algorithms
    double similarity = SimilarityDbAccess().db()->getImageSimilarity(m_id, imageId);
    return (similarity > 0) ? similarity : 0.0;
}

void ImageExtendedProperties::setSimilarityTo(const qlonglong imageId, const double value)
{
    // TODO: extend for additional algorithms
    SimilarityDbAccess().db()->setImageSimilarity(m_id, imageId, value);
}

void ImageExtendedProperties::removeSimilarityTo(const qlonglong imageId)
{
    // TODO: extend for additional algorithms
    SimilarityDbAccess().db()->removeImageSimilarity(m_id, imageId);
}

QStringList ImageExtendedProperties::scene()
{
    return readFakeListProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreScene));
}

void ImageExtendedProperties::setScene(const QStringList& scene)
{
    setFakeListProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreScene), scene);
}

void ImageExtendedProperties::removeScene()
{
    setScene(QStringList());
}

QStringList ImageExtendedProperties::subjectCode()
{
    return readFakeListProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreSubjectCode));
}

void ImageExtendedProperties::setSubjectCode(const QStringList& subjectCode)
{
    setFakeListProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreSubjectCode), subjectCode);
}

void ImageExtendedProperties::removeSubjectCode()
{
    setSubjectCode(QStringList());
}

IptcCoreLocationInfo ImageExtendedProperties::location()
{
    IptcCoreLocationInfo location;
    location.country       = readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCountry));
    location.countryCode   = readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCountryCode));
    location.city          = readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCity));
    location.location      = readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreLocation));
    location.provinceState = readProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreProvinceState));
    return location;
}

void ImageExtendedProperties::setLocation(const IptcCoreLocationInfo& location)
{
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCountry), location.country);
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCountryCode), location.countryCode);
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCity), location.city);
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreLocation), location.location);
    setProperty(ImageScanner::iptcCorePropertyName(MetadataInfo::IptcCoreProvinceState), location.provinceState);
}

void ImageExtendedProperties::removeLocation()
{
    setLocation(IptcCoreLocationInfo());
}

QString ImageExtendedProperties::readProperty(const QString& property)
{
    return CoreDbAccess().db()->getImageProperty(m_id, property);
}

void ImageExtendedProperties::setProperty(const QString& property, const QString& value)
{
    if (value.isNull()) // there is a NOT NULL restriction on the table.
    {
        removeProperty(property);
    }
    else
    {
        CoreDbAccess().db()->setImageProperty(m_id, property, value);
    }
}

QStringList ImageExtendedProperties::readFakeListProperty(const QString& property)
{
    QString value = CoreDbAccess().db()->getImageProperty(m_id, property);
    return value.split(QLatin1Char(';'), QString::SkipEmptyParts);
}

void ImageExtendedProperties::setFakeListProperty(const QString& property, const QStringList& value)
{
    if (value.isEmpty())
    {
        removeProperty(property);
    }
    else
    {
        CoreDbAccess().db()->setImageProperty(m_id, property, value.join(QLatin1String(";")));
    }
}

void ImageExtendedProperties::removeProperty(const QString& property)
{
    CoreDbAccess().db()->removeImageProperty(m_id, property);
}

} // namespace Digikam
