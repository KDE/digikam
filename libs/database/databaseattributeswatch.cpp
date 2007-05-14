/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-23
 * Description : Keeping image properties in sync.
 *
 * Copyright 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "databaseattributeswatch.h"
#include "databaseattributeswatch.moc"

namespace Digikam
{

DatabaseAttributesWatch::~DatabaseAttributesWatch()
{
}

/*
void DatabaseAttributesWatch::cleanUp()
{
}

void DatabaseAttributesWatch::shutDown()
{
    if (m_instance)
        m_instance->disconnect(0, 0, 0);
}
*/

void DatabaseAttributesWatch::sendImageFieldChanged(Q_LLONG imageId, ImageDataField field)
{
    emit imageFieldChanged(imageId, field);
}


} // namespace Digikam

