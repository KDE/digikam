/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-27
 * Description : Database Engine element configuration loader
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

#include "dbengineconfigloader.h"

// Qt includes

#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DbEngineConfigSettingsLoader::DbEngineConfigSettingsLoader(const QString& filepath, int xmlVersion)
{
    isValid = readConfig(filepath, xmlVersion);

    if (!isValid)
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << errorMessage;
    }
}

DbEngineConfigSettings DbEngineConfigSettingsLoader::readDatabase(QDomElement& databaseElement)
{
    DbEngineConfigSettings configElement;
    configElement.databaseID = QLatin1String("Unidentified");

    if (!databaseElement.hasAttribute(QLatin1String("name")))
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Missing statement attribute <name>.";
    }

    configElement.databaseID = databaseElement.attribute(QLatin1String("name"));
    QDomElement element      = databaseElement.namedItem(QLatin1String("databaseName")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Missing element <databaseName>.";
    }

    configElement.databaseName = element.text();
    element                    = databaseElement.namedItem(QLatin1String("userName")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Missing element <userName>.";
    }

    configElement.userName = element.text();
    element                = databaseElement.namedItem(QLatin1String("password")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Missing element <password>.";
    }

    configElement.password = element.text();
    element                = databaseElement.namedItem(QLatin1String("hostName")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Missing element <hostName>.";
    }

    configElement.hostName = element.text();
    element                = databaseElement.namedItem(QLatin1String("port")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Missing element <port>.";
    }

    configElement.port = element.text();
    element            = databaseElement.namedItem(QLatin1String("connectoptions")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Missing element <connectoptions>.";
    }

    configElement.connectOptions = element.text();
    element                      = databaseElement.namedItem(QLatin1String("dbactions")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Missing element <dbactions>.";
    }

    readDBActions(element, configElement);

    return configElement;
}

void DbEngineConfigSettingsLoader::readDBActions(QDomElement& sqlStatementElements, DbEngineConfigSettings& configElement)
{
    QDomElement dbActionElement = sqlStatementElements.firstChildElement(QLatin1String("dbaction"));

    for ( ; !dbActionElement.isNull();  dbActionElement=dbActionElement.nextSiblingElement(QLatin1String("dbaction")))
    {
        if (!dbActionElement.hasAttribute(QLatin1String("name")))
        {
            qCDebug(DIGIKAM_DBENGINE_LOG) << "Missing statement attribute <name>.";
        }

        DbEngineAction action;
        action.name = dbActionElement.attribute(QLatin1String("name"));

        //qCDebug(DIGIKAM_DBENGINE_LOG) << "Getting attribute " << dbActionElement.attribute("name");

        if (dbActionElement.hasAttribute(QLatin1String("mode")))
        {
            action.mode = dbActionElement.attribute(QLatin1String("mode"));
        }

        QDomElement databaseElement = dbActionElement.firstChildElement(QLatin1String("statement"));

        for ( ; !databaseElement.isNull();  databaseElement = databaseElement.nextSiblingElement(QLatin1String("statement")))
        {
            if (!databaseElement.hasAttribute(QLatin1String("mode")))
            {
                qCDebug(DIGIKAM_DBENGINE_LOG) << "Missing statement attribute <mode>.";
            }

            DbEngineActionElement actionElement;
            actionElement.mode      = databaseElement.attribute(QLatin1String("mode"));
            actionElement.statement = databaseElement.text();

            action.dbActionElements.append(actionElement);
        }

        configElement.sqlStatements.insert(action.name, action);
    }
}

bool DbEngineConfigSettingsLoader::readConfig(const QString& filepath, int xmlVersion)
{
    qCDebug(DIGIKAM_DBENGINE_LOG) << "Loading SQL code from config file" << filepath;
    QFile file(filepath);

    if (!file.exists())
    {
        errorMessage = i18n("Could not open the configuration file <b>%1</b>. "
                            "This file is installed with the digikam application "
                            "and is absolutely required to run digikam. "
                            "Please check your installation.", filepath);
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMessage = i18n("Could not open configuration file <b>%1</b>", filepath);
        return false;
    }

    QDomDocument doc(QLatin1String("DBConfig"));

    if (!doc.setContent(&file))
    {
        file.close();
        errorMessage = i18n("The XML in the configuration file <b>%1</b> is invalid and cannot be read.", filepath);
        return false;
    }

    file.close();

    QDomElement element = doc.namedItem(QLatin1String("databaseconfig")).toElement();

    if (element.isNull())
    {
        errorMessage = i18n("The XML in the configuration file <b>%1</b> "
                            "is missing the required element <icode>%2</icode>",
                            filepath, element.tagName());
        return false;
    }

    QDomElement defaultDB =  element.namedItem(QLatin1String("defaultDB")).toElement();

    if (defaultDB.isNull())
    {
        errorMessage = i18n("The XML in the configuration file <b>%1</b> "
                            "is missing the required element <b>%2</b>",
                            filepath, element.tagName());
        return false;
    }

    QDomElement versionElement = element.namedItem(QLatin1String("version")).toElement();
    int version                = 0;

    qCDebug(DIGIKAM_DBENGINE_LOG) << "Checking XML version ID => expected: " << xmlVersion
                                  << " found: " << versionElement.text().toInt();

    if (!versionElement.isNull())
    {
        version = versionElement.text().toInt();
    }

    if (version < xmlVersion)
    {
        errorMessage = i18n("An old version of the configuration file <b>%1</b> "
                            "is found. Please ensure that the version released "
                            "with the running version of digiKam is installed. ",
                            filepath);
        return false;
    }

    //qCDebug(DIGIKAM_DBENGINE_LOG) << "Default DB Node contains: " << defaultDB.text();

    QDomElement databaseElement = element.firstChildElement(QLatin1String("database"));

    for ( ; !databaseElement.isNull(); databaseElement=databaseElement.nextSiblingElement(QLatin1String("database")))
    {
        DbEngineConfigSettings l_DBCfgElement = readDatabase(databaseElement);
        databaseConfigs.insert(l_DBCfgElement.databaseID, l_DBCfgElement);
    }

/*
    qCDebug(DIGIKAM_DBENGINE_LOG) << "Found entries: " << databaseConfigs.size();

    foreach(const DbEngineConfigSettings& configElement, databaseConfigs )
    {
        qCDebug(DIGIKAM_DBENGINE_LOG) << "DatabaseID: "          << configElement.databaseID;
        qCDebug(DIGIKAM_DBENGINE_LOG) << "HostName: "            << configElement.hostName;
        qCDebug(DIGIKAM_DBENGINE_LOG) << "DatabaseName: "        << configElement.databaseName;
        qCDebug(DIGIKAM_DBENGINE_LOG) << "UserName: "            << configElement.userName;
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Password: "            << configElement.password;
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Port: "                << configElement.port;
        qCDebug(DIGIKAM_DBENGINE_LOG) << "ConnectOptions: "      << configElement.connectOptions;
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Database Server CMD: " << configElement.dbServerCmd;
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Database Init CMD: "   << configElement.dbInitCmd;
        qCDebug(DIGIKAM_DBENGINE_LOG) << "Statements:";

        foreach(const QString& actionKey, configElement.sqlStatements.keys())
        {
            QList<databaseActionElement> l_DBActionElement = configElement.sqlStatements[actionKey].dBActionElements;
            qCDebug(DIGIKAM_DBENGINE_LOG) << "DBAction [" << actionKey << "] has [" << l_DBActionElement.size() << "] actions";

            foreach(const databaseActionElement statement, l_DBActionElement)
            {
                qCDebug(DIGIKAM_DBENGINE_LOG) << "\tMode ["<< statement.mode <<"] Value ["<< statement.statement <<"]";
            }
        }

    }
*/
    return true;
}

} // namespace Digikam
