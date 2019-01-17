/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - Xmp helpers.
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011      by Leif Huhn <leif at dkstat dot com>
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

#include "dmetadata.h"

// Qt includes

#include <QLocale>

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

QVariant DMetadata::fromXmpList(const char* const xmpTagName) const
{
    QVariant var = getXmpTagVariant(xmpTagName);

    if (var.isNull())
    {
        return QVariant(QVariant::StringList);
    }

    return var;
}

QVariant DMetadata::fromXmpLangAlt(const char* const xmpTagName) const
{
    QVariant var = getXmpTagVariant(xmpTagName);

    if (var.isNull())
    {
        return QVariant(QVariant::Map);
    }

    return var;
}

bool DMetadata::addToXmpTagStringBag(const char* const xmpTagName, const QStringList& entriesToAdd) const
{
    QStringList oldEntries = getXmpTagStringBag(xmpTagName, false);
    QStringList newEntries = entriesToAdd;

    // Create a list of keywords including old one which already exists.
    for (QStringList::const_iterator it = oldEntries.constBegin(); it != oldEntries.constEnd(); ++it )
    {
        if (!newEntries.contains(*it))
        {
            newEntries.append(*it);
        }
    }

    if (setXmpTagStringBag(xmpTagName, newEntries))
    {
        return true;
    }

    return false;
}

bool DMetadata::removeFromXmpTagStringBag(const char* const xmpTagName, const QStringList& entriesToRemove) const
{
    QStringList currentEntries = getXmpTagStringBag(xmpTagName, false);
    QStringList newEntries;

    // Create a list of current keywords except those that shall be removed
    for (QStringList::const_iterator it = currentEntries.constBegin(); it != currentEntries.constEnd(); ++it )
    {
        if (!entriesToRemove.contains(*it))
        {
            newEntries.append(*it);
        }
    }

    if (setXmpTagStringBag(xmpTagName, newEntries))
    {
        return true;
    }

    return false;
}

QStringList DMetadata::getXmpKeywords() const
{
    return (getXmpTagStringBag("Xmp.dc.subject", false));
}

bool DMetadata::setXmpKeywords(const QStringList& newKeywords) const
{
    return setXmpTagStringBag("Xmp.dc.subject", newKeywords);
}

bool DMetadata::removeXmpKeywords(const QStringList& keywordsToRemove)
{
    return removeFromXmpTagStringBag("Xmp.dc.subject", keywordsToRemove);
}

QStringList DMetadata::getXmpSubCategories() const
{
    return (getXmpTagStringBag("Xmp.photoshop.SupplementalCategories", false));
}

bool DMetadata::setXmpSubCategories(const QStringList& newSubCategories) const
{
    return addToXmpTagStringBag("Xmp.photoshop.SupplementalCategories", newSubCategories);
}

bool DMetadata::removeXmpSubCategories(const QStringList& subCategoriesToRemove)
{
    return removeFromXmpTagStringBag("Xmp.photoshop.SupplementalCategories", subCategoriesToRemove);
}

QStringList DMetadata::getXmpSubjects() const
{
    return (getXmpTagStringBag("Xmp.iptc.SubjectCode", false));
}

bool DMetadata::setXmpSubjects(const QStringList& newSubjects) const
{
    return addToXmpTagStringBag("Xmp.iptc.SubjectCode", newSubjects);
}

bool DMetadata::removeXmpSubjects(const QStringList& subjectsToRemove)
{
    return removeFromXmpTagStringBag("Xmp.iptc.SubjectCode", subjectsToRemove);
}

bool DMetadata::removeXmpTags(const QStringList& tagFilters)
{
    MetaDataMap m = getXmpTagsDataList(tagFilters);

    if (m.isEmpty())
        return false;

    for (MetaDataMap::iterator it = m.begin() ; it != m.end() ; ++it)
    {
        removeXmpTag(it.key().toLatin1().constData());
    }

    return true;
}

} // namespace Digikam
