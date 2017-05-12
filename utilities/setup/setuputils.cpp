/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : utilities for digiKam setup
 *
 * Copyright (C) 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Album
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Album Public License for more details.
 *
 * ============================================================ */

#include "setuputils.h"

// Qt includes

#include <QStringList>

namespace Digikam
{

QStringList cleanUserFilterString(QString filterString, const bool caseSensitive)
{
    if (!caseSensitive)
    {
        filterString = filterString.toLower();
    }
    filterString.replace(QLatin1Char(';'), QLatin1Char(' '));
    filterString.remove(QLatin1String("*"));
    filterString.replace(QLatin1String(" ."), QLatin1String(" "));
    filterString.replace(QLatin1String(" -."), QLatin1String(" -"));

    QStringList filterList;
    foreach(const QString& filter, filterString.split(QLatin1Char(' '), QString::SkipEmptyParts))
    {
        filterList << filter.trimmed();
    }

    return filterList;
}

}  // namespace Digikam
