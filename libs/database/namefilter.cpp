/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-02
 * Description : Building complex database SQL queries from search descriptions
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#include "namefilter.h"
//Added by qt3to4:
#include <Q3ValueList>

namespace Digikam
{

NameFilter::NameFilter(const QString &filter)
{
    if ( filter.isEmpty() )
        return;

    QChar sep( ';' );
    int i = filter.find( sep, 0 );
    if ( i == -1 && filter.find( ' ', 0 ) != -1 )
        sep = QChar( ' ' );

    QStringList list = QStringList::split( sep, filter );
    QStringList::Iterator it = list.begin();
    while ( it != list.end() ) {
        m_filterList << QRegExp( (*it).trimmed(), false, true );
        ++it;
    }
}

bool NameFilter::matches(const QString &name)
{
    Q3ValueList<QRegExp>::ConstIterator rit = m_filterList.begin();
    while ( rit != m_filterList.end() ) {
        if ( (*rit).exactMatch(name) )
            return true;
        ++rit;
    }
    return false;
}


}




