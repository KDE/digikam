/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-02
 * Description : a directory name parser class
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

#ifndef DIRECTORYNAMEPARSER_H
#define DIRECTORYNAMEPARSER_H

// Local includes

#include "parser.h"

class QString;

namespace Digikam
{
namespace ManualRename
{

class DirectoryNameParser : public Parser
{
    Q_OBJECT

public:

    DirectoryNameParser();
    ~DirectoryNameParser() {};

    void parse(QString& parseString, const ParseInformation& info);
};

} // namespace ManualRename
} // namespace Digikam

#endif /* DIRECTORYNAMEPARSER_H */
