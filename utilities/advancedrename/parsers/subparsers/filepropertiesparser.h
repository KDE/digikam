/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a file properties parser class
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

#ifndef FILEPROPERTIESPARSER_H
#define FILEPROPERTIESPARSER_H

// Qt includes

#include <QString>

// Local includes

#include "subparser.h"

namespace Digikam
{

class FilePropertiesParser : public SubParser
{
    Q_OBJECT

public:

    FilePropertiesParser();
    ~FilePropertiesParser() {};

protected:

    virtual void parseOperation(const QString& parseString, const ParseInformation& info, ParseResults& results);
};

} // namespace Digikam

#endif /* FILEPROPERTIESPARSER_H */
