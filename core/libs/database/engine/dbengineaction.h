/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-27
 * Description : Database engine action
 *
 * Copyright (C) 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef _DATABASE_ENGINE_ACTION_H_
#define _DATABASE_ENGINE_ACTION_H_

// Qt includes

#include <QList>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DbEngineActionElement
{
public:

    DbEngineActionElement()
        : order(0)
    {
    }

    QString mode;
    int     order;
    QString statement;
};

// ---------------------------------------------------------------

class DIGIKAM_EXPORT DbEngineAction
{
public:

    QString                      name;
    QString                      mode;
    QList<DbEngineActionElement> dbActionElements;
};

} // namespace Digikam

#endif // _DATABASE_ENGINE_ACTION_H_
