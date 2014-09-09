/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Arnd Baecker <arnd dot baecker at web dot de>
 * Copyright (C) 2014      by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
 * Copyright (C) 2014      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

// Local includes

#include "applicationsettings.h"
#include "applicationsettings_p.h"

namespace Digikam
{

QString ApplicationSettings::getDatabaseFilePath() const
{
    return d->databaseParams.getDatabaseNameOrDir();
}

void ApplicationSettings::setDatabaseFilePath(const QString& path)
{
    d->databaseParams.setDatabasePath(path);
    d->databaseParams.setThumbsDatabasePath(path);
}

DatabaseParameters ApplicationSettings::getDatabaseParameters() const
{
    return d->databaseParams;
}

void ApplicationSettings::setDatabaseParameters(const DatabaseParameters& params)
{
    d->databaseParams = params;
}

QString ApplicationSettings::getDatabaseType() const
{
    return d->databaseParams.databaseType;
}

void ApplicationSettings::setDatabaseType(const QString& databaseType)
{
    d->databaseParams.databaseType = databaseType;
}

QString ApplicationSettings::getDatabaseConnectoptions() const
{
    return d->databaseParams.connectOptions;
}

QString ApplicationSettings::getDatabaseName() const
{
    return d->databaseParams.databaseName;
}

QString ApplicationSettings::getDatabaseNameThumbnails() const
{
    return d->databaseParams.databaseNameThumbnails;
}

QString ApplicationSettings::getDatabaseHostName() const
{
    return d->databaseParams.hostName;
}

QString ApplicationSettings::getDatabasePassword() const
{
    return d->databaseParams.password;
}

int ApplicationSettings::getDatabasePort() const
{
    return d->databaseParams.port;
}

QString ApplicationSettings::getDatabaseUserName() const
{
    return d->databaseParams.userName;
}

bool ApplicationSettings::getInternalDatabaseServer() const
{
    return d->databaseParams.internalServer;
}

void ApplicationSettings::setDatabaseConnectoptions(const QString& connectoptions)
{
    d->databaseParams.connectOptions = connectoptions;
}

void ApplicationSettings::setDatabaseName(const QString& databaseName)
{
    d->databaseParams.databaseName = databaseName;
}

void ApplicationSettings::setDatabaseNameThumbnails(const QString& databaseNameThumbnails)
{
    d->databaseParams.databaseNameThumbnails = databaseNameThumbnails;
}

void ApplicationSettings::setDatabaseHostName(const QString& hostName)
{
    d->databaseParams.hostName = hostName;
}

void ApplicationSettings::setDatabasePassword(const QString& password)
{
    d->databaseParams.password = password;
}

void ApplicationSettings::setDatabasePort(int port)
{
    d->databaseParams.port = port;
}

void ApplicationSettings::setDatabaseUserName(const QString& userName)
{
    d->databaseParams.userName = userName;
}

void ApplicationSettings::setInternalDatabaseServer(const bool useInternalDBServer)
{
    d->databaseParams.internalServer = useInternalDBServer;
}

void ApplicationSettings::setSyncBalooToDigikam(bool val)
{
    d->syncToDigikam = val;
    emit balooSettingsChanged();
}

bool ApplicationSettings::getSyncBalooToDigikam() const
{
    return d->syncToDigikam;
}

void ApplicationSettings::setSyncDigikamToBaloo(bool val)
{
    d->syncToBaloo = val;
    emit balooSettingsChanged();
}

bool ApplicationSettings::getSyncDigikamToBaloo() const
{
    return d->syncToBaloo;
}

}  // namespace Digikam
