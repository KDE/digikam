/* ============================================================
 * File  : imageattributeswatch.cpp
 * Author: Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2007-03-23
 * Description : Watch database image attributes
 * 
 * Copyright 2006 by Marcel Wiesweg
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

