/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-19
 * Description : a base class for a database keys collection
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "dbkeyscollection.h"

namespace Digikam
{

DbKeysCollection::DbKeysCollection(const QString& name)
{
    this->name = name;
}

DbKeysCollection::~DbKeysCollection()
{
}

QString DbKeysCollection::getValue(const QString& key, ParseSettings& settings)
{
    return getDbValue(key, settings);
}

QString DbKeysCollection::collectionName() const
{
    return name;
}

void DbKeysCollection::addId(const QString& id, const QString& description)
{
    if (id.isEmpty() || description.isEmpty())
    {
        return;
    }

    idsMap.insert(id, description);
}

DbKeyIdsMap DbKeysCollection::ids() const
{
    return idsMap;
}

} // namespace Digikam
