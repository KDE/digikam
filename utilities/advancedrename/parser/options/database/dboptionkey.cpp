/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-19
 * Description : a db option key
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "dboptionkey.h"

namespace Digikam
{

DbOptionKey::DbOptionKey()
{
}

DbOptionKey::~DbOptionKey()
{
}

QString DbOptionKey::getValue(const QString& key, ParseSettings& settings)
{
    return getDbValue(key, settings);
}

void DbOptionKey::addKey(const QString& key, const QString& description)
{
    if (key.isEmpty() || description.isEmpty())
    {
        return;
    }
    m_keywords.insert(key, description);
}

DbKeyIdsMap DbOptionKey::ids() const
{
    return m_keywords;
}

} // namespace Digikam
