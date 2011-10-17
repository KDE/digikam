/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an abstract option class
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

#include "option.moc"

// Qt includes

#include <QRegExp>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{

class OptionPriv
{
public:

    OptionPriv()
    {}

    ParseResults parsedResults;
};

Option::Option(const QString& name, const QString& description)
    : Parseable(name), d(new OptionPriv)
{
    setDescription(description);
}

Option::Option(const QString& name, const QString& description, const QString& icon)
    : Parseable(name, icon), d(new OptionPriv)
{
    setDescription(description);
}

Option::~Option()
{
    delete d;
}


ParseResults Option::parse(ParseSettings& settings)
{
    d->parsedResults.clear();
    const QRegExp& reg         = regExp();
    const QString& parseString = settings.parseString;

    int pos = 0;

    while (pos > -1)
    {
        pos = reg.indexIn(parseString, pos);

        if (pos > -1)
        {
            QString result = parseOperation(settings);

            ParseResults::ResultsKey   k(pos, reg.cap(0).count());
            ParseResults::ResultsValue v(reg.cap(0), result);
            d->parsedResults.addEntry(k, v);
            pos += reg.matchedLength();
        }
    }

    return d->parsedResults;
}

} // namespace Digikam
