/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-01
 * Description : Recording changes on the database
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "databasechangesets.h"

namespace Digikam
{

ImageChangeset::ImageChangeset()
{
}

ImageChangeset::ImageChangeset(QList<qlonglong> ids, DatabaseFields::Set changes)
    : m_ids(ids), m_changes(changes)
{
}

ImageChangeset::ImageChangeset(qlonglong id, DatabaseFields::Set changes)
    : m_changes(changes)
{
    m_ids << id;
}

QList<qlonglong> ImageChangeset::ids() const
{
    return m_ids;
}

bool ImageChangeset::containsImage(qlonglong id) const
{
    return m_ids.contains(id);
}

DatabaseFields::Set ImageChangeset::changes() const
{
    return m_changes;
}



ImageTagChangeset::ImageTagChangeset()
    : m_operation(Unknown)
{
}

ImageTagChangeset::ImageTagChangeset(QList<qlonglong> ids, QList<int> tags, Operation op)
    : m_ids(ids), m_tags(tags), m_operation(op)
{
}

ImageTagChangeset::ImageTagChangeset(qlonglong id, QList<int> tags, Operation op)
    : m_tags(tags), m_operation(op)
{
    m_ids << id;
}

ImageTagChangeset::ImageTagChangeset(qlonglong id, int tag, Operation op)
    : m_operation(op)
{
    m_ids << id;
    m_tags << tag;
}

QList<qlonglong> ImageTagChangeset::ids() const
{
    return m_ids;
}

bool ImageTagChangeset::containsImage(qlonglong id) const
{
    return m_ids.contains(id);
}

QList<int> ImageTagChangeset::tags() const
{
    return m_tags;
}

bool ImageTagChangeset::containsTag(int id)
{
    return (m_operation & RemovedAll) || m_tags.contains(id);
}

ImageTagChangeset::Operation ImageTagChangeset::operation() const
{
    return m_operation;
}



}



