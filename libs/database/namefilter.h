/* ============================================================
 * Authors: Marcel Wiesweg
 * Date   : 2007-04-02
 * Description : database interface - file name filter
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

#ifndef NAMEFILTER_H
#define NAMEFILTER_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qregexp.h>

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT NameFilter
{
public:

    /**
     * Creates a name filter object with the given filter string.
     * The string is a list of text parts of which one needs to match
     * (file suffixes),
     * separated by ';' characters.
     */
    NameFilter(const QString &filter);

    /**
     * Returns if the specified name matches this filter
     */
    bool matches(const QString &name);

protected:

    QValueList<QRegExp> m_filterList;

};



}

#endif


