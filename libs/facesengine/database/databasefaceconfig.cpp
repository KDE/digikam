/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-27
 * Description : Database face configuration
 *
 * Copyright (C) 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
 * Copyright (C) 2010-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "databasefaceconfig.h"

// Qt includes

#include <QtGlobal>
#include <QStandardPaths>

// Local includes

#include "digikam_dbconfig.h"
#include "databaseconfigloader.h"

using namespace Digikam;

namespace FacesEngine
{

Q_GLOBAL_STATIC_WITH_ARGS(DatabaseConfigLoader,
                          loader,
                          (QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/facesengine/dbfaceconfig.xml")),
                           dbfaceconfig_xml_version)
                         )

DatabaseConfig DatabaseFaceConfig::element(const QString& databaseType)
{
    // Unprotected read-only access? Usually accessed under DatabaseAccess protection anyway
    return loader()->databaseConfigs.value(databaseType);
}

bool DatabaseFaceConfig::checkReadyForUse()
{
    return loader()->isValid;
}

QString DatabaseFaceConfig::errorMessage()
{
    return loader()->errorMessage;
}

} // namespace FacesEngine
