/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-23
 * Description : position information keys
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef POSITIONKEYS_H
#define POSITIONKEYS_H

// Local includes

#include "dbkeyscollection.h"
#include "parsesettings.h"

namespace Digikam
{

class PositionKeys : public DbKeysCollection
{

public:

    PositionKeys();
    virtual ~PositionKeys() {};

protected:

    virtual QString getDbValue(const QString& key, ParseSettings& settings);

private:

    PositionKeys(const PositionKeys&);
    PositionKeys& operator=(const PositionKeys&);
};

} // namespace Digikam

#endif /* POSITIONKEYS_H */
