/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-24
 * Description : Core database privileges checker
 *
 * Copyright (C) 2010 by Holger Foerster <hamsi2k at freenet dot de>
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

#ifndef COREDATABASECHECKER_H
#define COREDATABASECHECKER_H

// Local includes

#include "dbengineparameters.h"
#include "coredbbackend.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT CoreDbPrivilegesChecker
{

public:

    explicit CoreDbPrivilegesChecker(const DbEngineParameters& parameters);
    ~CoreDbPrivilegesChecker();

    bool checkPrivileges(QStringList& insufficientRights);
    bool checkPriv(CoreDbBackend& dbBackend, const QString& dbActionName);

private:

    DbEngineParameters m_parameters;
};

}  // namespace Digikam

#endif // COREDATABASECHECKER_H
