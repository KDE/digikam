/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-27
 * Description : Database element configuration
 *
 * Copyright (C) 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
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

#include "databasefaceconfigelement.h"

// Qt includes

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QIODevice>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_dbconfig.h"
#include "digikam_debug.h"

namespace FacesEngine
{

class DatabaseFaceConfigElementLoader
{
public:

    DatabaseFaceConfigElementLoader();

    bool                      readConfig();
    DatabaseFaceConfigElement readDatabase(QDomElement& databaseElement);
    void                      readDBActions(QDomElement& sqlStatementElements, DatabaseFaceConfigElement& configElement);

public:

    bool                                     isValid;
    QString                                  errorMessage;
    QMap<QString, DatabaseFaceConfigElement> databaseConfigs;
};

Q_GLOBAL_STATIC(DatabaseFaceConfigElementLoader, loader)

DatabaseFaceConfigElementLoader::DatabaseFaceConfigElementLoader()
{
    isValid = readConfig();

    if (!isValid)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << errorMessage;
    }
}

DatabaseFaceConfigElement DatabaseFaceConfigElementLoader::readDatabase(QDomElement& databaseElement)
{
    DatabaseFaceConfigElement configElement;
    configElement.databaseID = QString::fromLatin1("Unidentified");

    if (!databaseElement.hasAttribute(QString::fromLatin1("name")))
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Missing statement attribute <name>.";
    }

    configElement.databaseID = databaseElement.attribute(QString::fromLatin1("name"));
    QDomElement element      = databaseElement.namedItem(QString::fromLatin1("databaseName")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Missing element <databaseName>.";
    }

    configElement.databaseName = element.text();
    element                    = databaseElement.namedItem(QString::fromLatin1("userName")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Missing element <userName>.";
    }

    configElement.userName = element.text();
    element                = databaseElement.namedItem(QString::fromLatin1("password")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Missing element <password>.";
    }

    configElement.password = element.text();
    element                = databaseElement.namedItem(QString::fromLatin1("hostName")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Missing element <hostName>.";
    }

    configElement.hostName = element.text();
    element                = databaseElement.namedItem(QString::fromLatin1("port")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Missing element <port>.";
    }

    configElement.port = element.text();
    element            = databaseElement.namedItem(QString::fromLatin1("connectoptions")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Missing element <connectoptions>.";
    }

    configElement.connectOptions = element.text();
    element                      = databaseElement.namedItem(QString::fromLatin1("dbservercmd")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Missing element <dbservercmd>.";
    }

    configElement.dbServerCmd = element.text();
    element                   = databaseElement.namedItem(QString::fromLatin1("dbinitcmd")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Missing element <dbinitcmd>.";
    }

    configElement.dbInitCmd = element.text();
    element                 = databaseElement.namedItem(QString::fromLatin1("dbactions")).toElement();

    if (element.isNull())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Missing element <dbactions>.";
    }

    readDBActions(element, configElement);

    return configElement;
}

void DatabaseFaceConfigElementLoader::readDBActions(QDomElement& sqlStatementElements, DatabaseFaceConfigElement& configElement)
{
    QDomElement dbActionElement = sqlStatementElements.firstChildElement(QString::fromLatin1("dbaction"));

    for ( ; !dbActionElement.isNull();  dbActionElement=dbActionElement.nextSiblingElement(QString::fromLatin1("dbaction")))
    {
        if (!dbActionElement.hasAttribute(QString::fromLatin1("name")))
        {
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "Missing statement attribute <name>.";
        }

        DatabaseAction action;
        action.name = dbActionElement.attribute(QString::fromLatin1("name"));
        //qCDebug(DIGIKAM_FACESENGINE_LOG) << "Getting attribute " << dbActionElement.attribute("name");

        if (dbActionElement.hasAttribute(QString::fromLatin1("mode")))
        {
            action.mode = dbActionElement.attribute(QString::fromLatin1("mode"));
        }

        QDomElement databaseElement = dbActionElement.firstChildElement(QString::fromLatin1("statement"));

        for ( ; !databaseElement.isNull();  databaseElement=databaseElement.nextSiblingElement(QString::fromLatin1("statement")))
        {
            if (!databaseElement.hasAttribute(QString::fromLatin1("mode")))
            {
                qCDebug(DIGIKAM_FACESENGINE_LOG) << "Missing statement attribute <mode>.";
            }

            DatabaseActionElement actionElement;
            actionElement.mode      = databaseElement.attribute(QString::fromLatin1("mode"));
            actionElement.statement = databaseElement.text();

            action.dbActionElements.append(actionElement);
        }

        configElement.sqlStatements.insert(action.name, action);
    }
}

bool DatabaseFaceConfigElementLoader::readConfig()
{
    QString filepath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/facesengine/dbfaceconfig.xml"));
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Loading SQL code from config file" << filepath;
    QFile file(filepath);

    if (!file.exists())
    {
        errorMessage = i18n("Could not open the configuration file <b>%1</b>. "
                            "This file is installed with digiKam "
                            "and is absolutely required to run recognition. "
                            "Please check your installation.", filepath);
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMessage = i18n("Could not open configuration file <b>%1</b>", filepath);
        return false;
    }

    QDomDocument doc(QString::fromLatin1("DBConfig"));

    if (!doc.setContent(&file))
    {
        file.close();
        errorMessage = i18n("The XML in the configuration file <b>%1</b> "
                            "is invalid and cannot be read.", filepath);
        return false;
    }

    file.close();

    QDomElement element = doc.namedItem(QString::fromLatin1("databaseconfig")).toElement();

    if (element.isNull())
    {
        errorMessage = i18n("The XML in the configuration file <b>%1</b> "
                            "is missing the required element <b>%2</b>", filepath, element.tagName());
        return false;
    }

    QDomElement versionElement = element.namedItem(QString::fromLatin1("version")).toElement();
    int version                = 0;

    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Checking XML version ID: "
                                     << versionElement.isNull()
                                     << versionElement.text()
                                     << versionElement.text().toInt()
                                     << dbfaceconfig_xml_version;

    if (!versionElement.isNull())
    {
        version = versionElement.text().toInt();
    }

    if (version < dbfaceconfig_xml_version)
    {
        errorMessage = i18n("An old version of the configuration file <b>%1</b> "
                            "is found. Please ensure that the right version of digiKam "
                            "is installed.", filepath);
        return false;
    }

    //qCDebug(DIGIKAM_FACESENGINE_LOG) << "Default DB Node contains: " << defaultDB.text();

    QDomElement databaseElement = element.firstChildElement(QString::fromLatin1("database"));

    for ( ; !databaseElement.isNull(); databaseElement = databaseElement.nextSiblingElement(QString::fromLatin1("database")))
    {
        DatabaseFaceConfigElement l_DBCfgElement = readDatabase(databaseElement);
        databaseConfigs.insert(l_DBCfgElement.databaseID, l_DBCfgElement);
    }

/*
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Found entries: " << databaseConfigs.size();

    foreach(const DatabaseConfigElement& configElement, databaseConfigs )
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "DatabaseID: "          << configElement.databaseID;
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "HostName: "            << configElement.hostName;
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "DatabaseName: "        << configElement.databaseName;
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "UserName: "            << configElement.userName;
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Password: "            << configElement.password;
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Port: "                << configElement.port;
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "ConnectOptions: "      << configElement.connectOptions;
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Database Server CMD: " << configElement.dbServerCmd;
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Database Init CMD: "   << configElement.dbInitCmd;
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Statements:";

        foreach(const QString actionKey, configElement.sqlStatements.keys())
        {
            QList<databaseActionElement> l_DBActionElement = configElement.sqlStatements[actionKey].dBActionElements;
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "DBAction [" << actionKey << "] has [" << l_DBActionElement.size() << "] actions";

            foreach(const databaseActionElement statement, l_DBActionElement)
            {
                qCDebug(DIGIKAM_FACESENGINE_LOG) << "\tMode ["<< statement.mode <<"] Value ["<< statement.statement <<"]";
            }
        }
    }
*/
    return true;
}

// ------------------------------------------------------------------

DatabaseFaceConfigElement DatabaseFaceConfigElement::element(const QString& databaseType)
{
    // Unprotected read-only access? Usually accessed under DatabaseAccess protection anyway
    return loader()->databaseConfigs.value(databaseType);
}

bool DatabaseFaceConfigElement::checkReadyForUse()
{
    return loader()->isValid;
}

QString DatabaseFaceConfigElement::errorMessage()
{
    return loader()->errorMessage;
}

} // namespace FacesEngine
