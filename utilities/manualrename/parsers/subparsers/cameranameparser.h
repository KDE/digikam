/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a camera name parser class
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

#ifndef CAMERANAMEPARSER_H
#define CAMERANAMEPARSER_H

// Qt includes

#include <QString>

// Local includes

#include "parser.h"

namespace Digikam
{
namespace ManualRename
{

class CameraNameParser : public Parser
{
    Q_OBJECT

public:

    CameraNameParser();
    ~CameraNameParser() {};

protected:

    virtual void parseOperation(QString& parseString, const ParseInformation& info);
};

} // namespace ManualRename
} // namespace Digikam

#endif /* CAMERANAMEPARSER_H */
