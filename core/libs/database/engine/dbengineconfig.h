/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-27
 * Description : Database engine configuration
 *
 * Copyright (C) 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DB_ENGINE_CONFIG_H
#define DIGIKAM_DB_ENGINE_CONFIG_H

// Local includes

#include "digikam_export.h"
#include "dbengineconfigsettings.h"

namespace Digikam
{

class DIGIKAM_EXPORT DbEngineConfig
{
public:

    static bool                   checkReadyForUse();
    static QString                errorMessage();
    static DbEngineConfigSettings element(const QString& databaseType);
};

} // namespace Digikam

#endif // DIGIKAM_DB_ENGINE_CONFIG_H
