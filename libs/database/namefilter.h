/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-02
 * Description : Building complex database SQL queries from search descriptions
 *
 * Copyright (C) 2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QString>
#include <QList>
#include <QRegExp>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT NameFilter
{
public:

    /**
     * Creates a name filter object with the given filter string.
     * The string is a list of text parts of which one needs to match
     * (file suffixes),
     * separated by ';' characters.
     */
    explicit NameFilter(const QString& filter);

    /**
     * Returns if the specified name matches this filter
     */
    bool matches(const QString& name);

protected:

    QList<QRegExp> m_filterList;
};

}  // namespace Digikam

#endif // NAMEFILTER_H
