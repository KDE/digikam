/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Storage container for database connection parameters.
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
/*
#ifndef DATABASEPARAMETERS_DEBUG
#define DATABASEPARAMETERS_DEBUG
#endif
*/

#include "databaseparameters.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QIODevice>
#include <QTextStream>
#include <QtGlobal>
#include <QtDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>

// KDE includes

#include <kstandarddirs.h>
#include <kcodecs.h>
#include <kdebug.h>

namespace Digikam
{

DatabaseParameters::DatabaseParameters()
                  : port(-1)
{
}

DatabaseParameters::DatabaseParameters(const QString& type,
                                       const QString& databaseName,
                                       const QString& connectOptions,
                                       const QString& hostName,
                                       int port,
                                       const QString& userName,
                                       const QString& password)
                  : databaseType(type), databaseName(databaseName),
                    connectOptions(connectOptions), hostName(hostName),
                    port(port), userName(userName),
                    password(password)
{
}

DatabaseParameters::DatabaseParameters(const KUrl& url)
                  : port(-1)
{
    databaseType   = url.queryItem("databaseType");
    databaseName   = url.queryItem("databaseName");
    connectOptions = url.queryItem("connectOptions");
    hostName       = url.queryItem("hostName");
    QString queryPort = url.queryItem("port");
    if (!queryPort.isNull())
        port = queryPort.toInt();
    userName       = url.queryItem("userName");
    password       = url.queryItem("password");
}

bool DatabaseParameters::operator==(const DatabaseParameters& other) const
{
    return databaseType   == other.databaseType &&
           databaseName   == other.databaseName &&
           connectOptions == other.connectOptions &&
           hostName       == other.hostName &&
           port           == other.port &&
           userName       == other.userName &&
           password       == other.password;
}

bool DatabaseParameters::operator!=(const DatabaseParameters& other) const
{
    return !operator==(other);
}

bool DatabaseParameters::isValid() const
{
    if (isSQLite())
        return !databaseName.isEmpty();
    return false;
}

bool DatabaseParameters::isSQLite() const
{
    return databaseType == "QSQLITE";
}

QString DatabaseParameters::SQLiteDatabaseFile() const
{
    if (isSQLite())
        return databaseName;
    return QString();
}

QByteArray DatabaseParameters::hash() const
{
    KMD5 md5;
    md5.update(databaseType.toUtf8());
    md5.update(databaseName.toUtf8());
    md5.update(connectOptions.toUtf8());
    md5.update(hostName.toUtf8());
    md5.update((const char *)&port, sizeof(int));
    md5.update(userName.toUtf8());
    md5.update(password.toUtf8());
    return md5.hexDigest();
}

DatabaseParameters DatabaseParameters::parametersFromConfig(const QString databaseType, const QString databaseName,
                                                            const QString databaseHostName, int databasePort,
                                                            const QString databaseUserName, const QString databaseUserPassword,
                                                            const QString databaseConnectOptions)
{
	DatabaseParameters parameters;

    // only the database name is needed
	parameters.readConfig();

    parameters.databaseType     = databaseType;
    parameters.databaseName     = databaseName;
    parameters.hostName         = databaseHostName;
    parameters.userName         = databaseUserName;
    parameters.password         = databaseUserPassword;
    parameters.port             = databasePort;
    parameters.connectOptions   = databaseConnectOptions;

    return parameters;
}

DatabaseParameters DatabaseParameters::parametersFromConfig(const QString databaseType)
{
    DatabaseParameters parameters;

    // only the database name is needed
    parameters.readConfig();

    parameters.databaseType     = databaseType;
    parameters.databaseName     = parameters.databaseConfigs[databaseType].databaseName;
    parameters.hostName         = parameters.databaseConfigs[databaseType].hostName;
    parameters.userName         = parameters.databaseConfigs[databaseType].userName;
    parameters.password         = parameters.databaseConfigs[databaseType].password;
    parameters.port             = parameters.databaseConfigs[databaseType].port.toInt();

    const QString miscDir     = KStandardDirs::locateLocal("data", "digikam/db_misc");
    QString connectOptions = parameters.databaseConfigs[databaseType].connectOptions;
    connectOptions.replace(QString("$$DBMISCPATH$$"), miscDir);

    parameters.connectOptions   = connectOptions;

    kDebug(50003)<<"ConnectOptions "<< parameters.connectOptions;
    return parameters;
}


DatabaseParameters DatabaseParameters::parametersForSQLite(const QString& databaseFile)
{
    // only the database name is needed
    return DatabaseParameters("QSQLITE", databaseFile);
}

DatabaseParameters DatabaseParameters::parametersForSQLiteDefaultFile(const QString& directory)
{
    QString filePath = directory + '/' + "digikam4.db";
    filePath = QDir::cleanPath(filePath);
    return parametersForSQLite(filePath);
}

void DatabaseParameters::insertInUrl(KUrl& url) const
{
    removeFromUrl(url);

    url.addQueryItem("databaseType", databaseType);
    url.addQueryItem("databaseName", databaseName);
    if (!connectOptions.isNull())
        url.addQueryItem("connectOptions", connectOptions);
    if (!hostName.isNull())
        url.addQueryItem("hostName", hostName);
    if (port != -1)
        url.addQueryItem("port", QString::number(port));
    if (!userName.isNull())
        url.addQueryItem("userName", userName);
    if (!password.isNull())
        url.addQueryItem("password", password);
}

void DatabaseParameters::removeFromUrl(KUrl& url)
{
    url.removeQueryItem("databaseType");
    url.removeQueryItem("databaseName");
    url.removeQueryItem("connectOptions");
    url.removeQueryItem("hostName");
    url.removeQueryItem("port");
    url.removeQueryItem("userName");
    url.removeQueryItem("password");
}

void DatabaseParameters::readConfig()
{
    QString filepath = KStandardDirs::locate("data", "digikam/database/dbconfig.xml");
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        #ifdef DATABASEPARAMETERS_DEBUG
        kDebug(50003) << "Couldn't open file: " << file.fileName().toAscii();
        #endif
        return;
    }

    QDomDocument doc("DBConfig");
    if (!doc.setContent(&file)){
        file.close();
        return;
    }
    file.close();

    QDomElement element = doc.namedItem("databaseconfig").toElement();
    if (element.isNull()){
        #ifdef DATABASEPARAMETERS_DEBUG
        kDebug(50003) << "Missing element <databaseconfig>.";
        #endif
        return;
    }

    QDomElement defaultDB =  element.namedItem("defaultDB").toElement();
    if (defaultDB.isNull())
    {
        #ifdef DATABASEPARAMETERS_DEBUG
        kDebug(50003) << "Missing element <defaultDB>.";
        #endif
        return;
    }
    defaultDatabase = defaultDB.text();

    #ifdef DATABASEPARAMETERS_DEBUG
    kDebug(50003) << "Default DB Node contains: " << defaultDatabase;
    #endif

    QDomElement databaseElement =  element.firstChildElement("database");
    for( ; !databaseElement.isNull();  databaseElement=databaseElement.nextSiblingElement("database"))
    {
        DatabaseConfigElement l_DBCfgElement = readDatabase(databaseElement);
        databaseConfigs.insert(l_DBCfgElement.databaseID, l_DBCfgElement);
    }

    #ifdef DATABASEPARAMETERS_DEBUG
    kDebug(50003) << "Found entries: " << databaseConfigs.size();
    foreach (const DatabaseConfigElement& configElement, databaseConfigs )
    {
        kDebug(50003) << "DatabaseID: " << configElement.databaseID;
        kDebug(50003) << "HostName: " << configElement.hostName;
        kDebug(50003) << "DatabaseName: " << configElement.databaseName;
        kDebug(50003) << "UserName: " << configElement.userName;
        kDebug(50003) << "Password: " << configElement.password;
        kDebug(50003) << "Port: " << configElement.port;
        kDebug(50003) << "ConnectOptions: " << configElement.connectOptions;
        kDebug(50003) << "Database Server CMD: " << configElement.dbServerCmd;
        kDebug(50003) << "Database Init CMD: " << configElement.dbInitCmd;


        kDebug(50003) << "Statements:";

        foreach (const QString actionKey, configElement.sqlStatements.keys()){
            QList<databaseActionElement> l_DBActionElement = configElement.sqlStatements[actionKey].dBActionElements;
            kDebug(50003) << "DBAction [" << actionKey << "] has [" << l_DBActionElement.size() << "] actions";
            foreach (const databaseActionElement statement, l_DBActionElement){
                kDebug(50003) << "\tMode ["<< statement.mode <<"] Value ["<< statement.statement <<"]";
            }
        }
    }
    #endif
}

DatabaseConfigElement DatabaseParameters::readDatabase(QDomElement &databaseElement)
{
    DatabaseConfigElement configElement;
    configElement.databaseID="Unidentified";

    if (!databaseElement.hasAttribute("name"))
    {
        kDebug(50003) << "Missing statement attribute <name>.";
    }
    configElement.databaseID = databaseElement.attribute("name");

    QDomElement element =  databaseElement.namedItem("databaseName").toElement();
    if (element.isNull())
    {
        kDebug(50003) << "Missing element <databaseName>.";
    }
    configElement.databaseName = element.text();

    element =  databaseElement.namedItem("userName").toElement();
    if (element.isNull())
    {
        kDebug(50003) << "Missing element <userName>.";
    }
    configElement.userName = element.text();

    element =  databaseElement.namedItem("password").toElement();
    if (element.isNull())
    {
        kDebug(50003) << "Missing element <password>.";
    }
    configElement.password = element.text();

    element =  databaseElement.namedItem("hostName").toElement();
    if (element.isNull())
    {
        kDebug(50003) << "Missing element <hostName>.";
    }
    configElement.hostName = element.text();

    element =  databaseElement.namedItem("port").toElement();
    if (element.isNull())
    {
        kDebug(50003) << "Missing element <port>.";
    }
    configElement.port = element.text();

    element =  databaseElement.namedItem("connectoptions").toElement();
    if (element.isNull())
    {
        kDebug(50003) << "Missing element <connectoptions>.";
    }
    configElement.connectOptions = element.text();

    element =  databaseElement.namedItem("dbservercmd").toElement();
    if (element.isNull())
    {
        kDebug(50003) << "Missing element <dbservercmd>.";
    }
    configElement.dbServerCmd = element.text();

    element =  databaseElement.namedItem("dbinitcmd").toElement();
    if (element.isNull())
    {
        kDebug(50003) << "Missing element <dbinitcmd>.";
    }
    configElement.dbInitCmd = element.text();

    element =  databaseElement.namedItem("dbactions").toElement();
    if (element.isNull())
    {
        kDebug(50003) << "Missing element <dbactions>.";
    }
    readDBActions(element, configElement);

    return configElement;
}

void DatabaseParameters::readDBActions(QDomElement& sqlStatementElements, DatabaseConfigElement& configElement)
{
    QDomElement dbActionElement =  sqlStatementElements.firstChildElement("dbaction");
    for( ; !dbActionElement.isNull();  dbActionElement=dbActionElement.nextSiblingElement("dbaction"))
    {
        if (!dbActionElement.hasAttribute("name"))
        {
            kDebug(50003) << "Missing statement attribute <name>.";
        }
        DatabaseAction action;
        action.name = dbActionElement.attribute("name");
        //kDebug(50003) << "Getting attribute " << dbActionElement.attribute("name");

        if (dbActionElement.hasAttribute("mode"))
        {
            action.mode = dbActionElement.attribute("mode");
        }
        else
        {
            kDebug(50003) << "Missing statement attribute <mode>. Setting to default \"transaction\".";
            action.mode = QString("transaction");
        }


        QDomElement databaseElement = dbActionElement.firstChildElement("statement");
        for( ; !databaseElement.isNull();  databaseElement=databaseElement.nextSiblingElement("statement"))
        {
            if (!databaseElement.hasAttribute("mode"))
            {
                kDebug(50003) << "Missing statement attribute <mode>.";
            }

            DatabaseActionElement actionElement;
            actionElement.mode      = databaseElement.attribute("mode");
            actionElement.statement = databaseElement.text();

            action.dbActionElements.append(actionElement);
        }
        configElement.sqlStatements.insert(action.name, action);
    }
}

}  // namespace Digikam
