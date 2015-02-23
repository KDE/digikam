/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location abstraction
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "collectionlocation.h"

namespace Digikam
{

CollectionLocation::CollectionLocation()
    : m_id(-1),
      m_status(LocationNull),
      m_type(TypeVolumeHardWired)
{
}

int CollectionLocation::id() const
{
    return m_id;
}

CollectionLocation::Status CollectionLocation::status() const
{
    return m_status;
}

CollectionLocation::Type CollectionLocation::type() const
{
    return m_type;
}

QString CollectionLocation::albumRootPath() const
{
    return m_path;
}

QString CollectionLocation::label() const
{
    return m_label;
}

}  // namespace Digikam
