/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-04
 * Description : Access to extended properties of an item in the database
 *
 * Copyright (C) 2009      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemextendedproperties.h"

// Local includes

#include "coredb.h"
#include "coredbaccess.h"
#include "itemscanner.h"
#include "similaritydbaccess.h"
#include "similaritydb.h"

namespace Digikam
{

ItemExtendedProperties::ItemExtendedProperties(qlonglong imageid)
    : m_id(imageid)
{
}

ItemExtendedProperties::ItemExtendedProperties()
    : m_id(0)
{
}

QString ItemExtendedProperties::intellectualGenre()
{
    return readProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreIntellectualGenre));
}

void ItemExtendedProperties::setIntellectualGenre(const QString& intellectualGenre)
{
    setProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreIntellectualGenre), intellectualGenre);
}

void ItemExtendedProperties::removeIntellectualGenre()
{
    setIntellectualGenre(QString());
}

QString ItemExtendedProperties::jobId()
{
    return readProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreJobID));
}

void ItemExtendedProperties::setJobId(const QString& jobId)
{
    setProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreJobID), jobId);
}

void ItemExtendedProperties::removeJobId()
{
    setJobId(QString());
}

double ItemExtendedProperties::similarityTo(const qlonglong imageId)
{
    // TODO: extend for additional algorithms
    double similarity = SimilarityDbAccess().db()->getImageSimilarity(m_id, imageId);
    return (similarity > 0) ? similarity : 0.0;
}

void ItemExtendedProperties::setSimilarityTo(const qlonglong imageId, const double value)
{
    // TODO: extend for additional algorithms
    SimilarityDbAccess().db()->setImageSimilarity(m_id, imageId, value);
}

void ItemExtendedProperties::removeSimilarityTo(const qlonglong imageId)
{
    // TODO: extend for additional algorithms
    SimilarityDbAccess().db()->removeImageSimilarity(m_id, imageId);
}

QStringList ItemExtendedProperties::scene()
{
    return readFakeListProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreScene));
}

void ItemExtendedProperties::setScene(const QStringList& scene)
{
    setFakeListProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreScene), scene);
}

void ItemExtendedProperties::removeScene()
{
    setScene(QStringList());
}

QStringList ItemExtendedProperties::subjectCode()
{
    return readFakeListProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreSubjectCode));
}

void ItemExtendedProperties::setSubjectCode(const QStringList& subjectCode)
{
    setFakeListProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreSubjectCode), subjectCode);
}

void ItemExtendedProperties::removeSubjectCode()
{
    setSubjectCode(QStringList());
}

IptcCoreLocationInfo ItemExtendedProperties::location()
{
    IptcCoreLocationInfo location;
    location.country       = readProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCountry));
    location.countryCode   = readProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCountryCode));
    location.city          = readProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCity));
    location.location      = readProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreLocation));
    location.provinceState = readProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreProvinceState));
    return location;
}

void ItemExtendedProperties::setLocation(const IptcCoreLocationInfo& location)
{
    setProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCountry), location.country);
    setProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCountryCode), location.countryCode);
    setProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreCity), location.city);
    setProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreLocation), location.location);
    setProperty(ItemScanner::iptcCorePropertyName(MetadataInfo::IptcCoreProvinceState), location.provinceState);
}

void ItemExtendedProperties::removeLocation()
{
    setLocation(IptcCoreLocationInfo());
}

QString ItemExtendedProperties::readProperty(const QString& property)
{
    return CoreDbAccess().db()->getImageProperty(m_id, property);
}

void ItemExtendedProperties::setProperty(const QString& property, const QString& value)
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

QStringList ItemExtendedProperties::readFakeListProperty(const QString& property)
{
    QString value = CoreDbAccess().db()->getImageProperty(m_id, property);
    return value.split(QLatin1Char(';'), QString::SkipEmptyParts);
}

void ItemExtendedProperties::setFakeListProperty(const QString& property, const QStringList& value)
{
    if (value.isEmpty())
    {
        removeProperty(property);
    }
    else
    {
        CoreDbAccess().db()->setImageProperty(m_id, property, value.join(QLatin1Char(';')));
    }
}

void ItemExtendedProperties::removeProperty(const QString& property)
{
    CoreDbAccess().db()->removeImageProperty(m_id, property);
}

} // namespace Digikam
