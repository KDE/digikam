/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-05
 * Description : Access to the properties of a tag in the database
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

#include "tagproperties.h"

// Qt includes

#include <QSharedData>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "tagscache.h"

namespace Digikam
{

class TagPropertiesPriv : public QSharedData
{
public:

    TagPropertiesPriv()
    {
        tagId = -1;
    }

    int tagId;
    QMap<QString, QString> properties;
};

class TagPropertiesPrivSharedNull : public QSharedDataPointer<TagPropertiesPriv>
{
    public:
    TagPropertiesPrivSharedNull() : QSharedDataPointer<TagPropertiesPriv>(new TagPropertiesPriv) {}
};
K_GLOBAL_STATIC(TagPropertiesPrivSharedNull, tagPropertiesPrivSharedNull)

TagProperties::TagProperties()
            : d(*tagPropertiesPrivSharedNull)
{
}

TagProperties::TagProperties(int tagId)
            : d(new TagPropertiesPriv)
{
    d->tagId = tagId;
    QList<TagProperty> properties = DatabaseAccess().db()->getTagProperties(tagId);
    foreach (const TagProperty &p, properties)
        d->properties.insertMulti(p.property, p.value);
}

TagProperties::~TagProperties()
{
}

TagProperties::TagProperties(const TagProperties& other)
{
    d = other.d;
}

TagProperties &TagProperties::operator=(const TagProperties& other)
{
    d = other.d;
    return *this;
}

bool TagProperties::isNull() const
{
    return d == *tagPropertiesPrivSharedNull;
}

TagProperties TagProperties::getOrCreate(const QString& tagPath)
{
    int tagId = TagsCache::instance()->getOrCreateTag(tagPath);
    return TagProperties(tagId);
}

int TagProperties::tagId() const
{
    return d->tagId;
}

bool TagProperties::hasProperty(const QString& key) const
{
    return d->properties.contains(key);
}

QString TagProperties::value(const QString& key) const
{
    return d->properties.value(key);
}

QStringList TagProperties::propertyKeys() const
{
    return d->properties.keys();
}

QMap<QString, QString> TagProperties::properties() const
{
    return d->properties;
}

void TagProperties::setProperty(const QString& key, const QString& value)
{
    // for single entries in db, this can of course be optimized using a single UPDATE WHERE
    removeProperties(key);
    d->properties.insert(key, value);
    DatabaseAccess().db()->addTagProperty(d->tagId, key, value);
}

void TagProperties::addProperty(const QString& key, const QString& value)
{
    d->properties.insertMulti(key, value);
    DatabaseAccess().db()->addTagProperty(d->tagId, key, value);
}

void TagProperties::removeProperties(const QString& key)
{
    if (d->properties.contains(key))
    {
        DatabaseAccess().db()->removeTagProperties(d->tagId, key);
        d->properties.remove(key);
    }
}

}


