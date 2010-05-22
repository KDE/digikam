/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : image format key
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

#ifndef FORMATKEY_H
#define FORMATKEY_H

// local includes

#include "dboptionkey.h"
#include "parsesettings.h"

namespace Digikam
{
class FormatKey : public DbOptionKey
{
public:
    FormatKey();
    virtual ~FormatKey() {};

protected:

    virtual QString getDbValue(ParseSettings& settings);
};

} // namespace Digikam

#endif /* FORMATKEY_H */
