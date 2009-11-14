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

DatabaseParameters DatabaseParameters::parametersFromConfig(const QString &databaseType, const QString &databaseName,
                                                            const QString &databaseHostName, int databasePort,
                                                            const QString &databaseUserName, const QString &databaseUserPassword,
                                                            const QString &databaseConnectOptions)
{
	DatabaseParameters parameters;

    // only the database name is needed
	parameters.readConfig();

	/*
	// now set default database entries according the default values
	parameters.databaseType 	= parameters.m_DatabaseConfigs[parameters.m_DefaultDatabase].m_DatabaseID;
	parameters.databaseName 	= parameters.m_DatabaseConfigs[parameters.m_DefaultDatabase].m_DatabaseName;
	parameters.hostName     	= parameters.m_DatabaseConfigs[parameters.m_DefaultDatabase].m_HostName;
	parameters.userName     	= parameters.m_DatabaseConfigs[parameters.m_DefaultDatabase].m_UserName;
	parameters.password     	= parameters.m_DatabaseConfigs[parameters.m_DefaultDatabase].m_Password;
	parameters.port         	= parameters.m_DatabaseConfigs[parameters.m_DefaultDatabase].m_Port.toInt();
	parameters.connectOptions 	= parameters.m_DatabaseConfigs[parameters.m_DefaultDatabase].m_ConnectOptions;
	*/

	parameters.databaseType     = databaseType;
    parameters.databaseName     = databaseName;
    parameters.hostName         = databaseHostName;
    parameters.userName         = databaseUserName;
    parameters.password         = databaseUserPassword;
    parameters.port             = databasePort;
    parameters.connectOptions   = databaseConnectOptions;

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

void DatabaseParameters::readConfig(){    
	    QString filepath = KStandardDirs::locate("data", "digikam/database/dbconfig.xml");
        QFile file(filepath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
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
        if (defaultDB.isNull()){
#ifdef DATABASEPARAMETERS_DEBUG
            kDebug(50003) << "Missing element <defaultDB>.";
#endif
            return;
        }         
         m_DefaultDatabase = defaultDB.text();
#ifdef DATABASEPARAMETERS_DEBUG
         kDebug(50003) << "Default DB Node contains: " << m_DefaultDatabase;
#endif
         QDomElement databaseElement =  element.firstChildElement("database");
         for( ; !databaseElement.isNull();  databaseElement=databaseElement.nextSiblingElement("database")){
            databaseconfigelement l_DBCfgElement = readDatabase(databaseElement);
            m_DatabaseConfigs.insert(l_DBCfgElement.m_DatabaseID, l_DBCfgElement);
         }
#ifdef DATABASEPARAMETERS_DEBUG
               kDebug(50003) << "Found entries: " << m_DatabaseConfigs.size();
               foreach (const databaseconfigelement& l_Element, m_DatabaseConfigs ){
                   kDebug(50003) << "DatabaseID: " << l_Element.m_DatabaseID;
                   kDebug(50003) << "HostName: " << l_Element.m_HostName;
                   kDebug(50003) << "DatabaseName: " << l_Element.m_DatabaseName;
                   kDebug(50003) << "UserName: " << l_Element.m_UserName;
                   kDebug(50003) << "Password: " << l_Element.m_Password;
                   kDebug(50003) << "Port: " << l_Element.m_Port;
                   kDebug(50003) << "ConnectOptions: " << l_Element.m_ConnectOptions;

                   kDebug(50003) << "Statements:";

                   foreach (const QString actionKey, l_Element.m_SQLStatements.keys()){
                       QList<databaseActionElement> l_DBActionElement = l_Element.m_SQLStatements[actionKey].m_DBActionElements;
                       kDebug(50003) << "DBAction [" << actionKey << "] has [" << l_DBActionElement.size() << "] actions";
                    foreach (const databaseActionElement statement, l_DBActionElement){
                           kDebug(50003) << "\tMode ["<< statement.m_Mode <<"] Value ["<< statement.m_Statement <<"]";
                       }
                   }
           }
#endif
     }

databaseconfigelement DatabaseParameters::readDatabase(QDomElement &databaseElement){
	databaseconfigelement l_Element;
	l_Element.m_DatabaseID="Unidentified";

        if (!databaseElement.hasAttribute("name")){
            kDebug(50003) << "Missing statement attribute <name>.";
        }
        l_Element.m_DatabaseID = databaseElement.attribute("name");

        QDomElement element =  databaseElement.namedItem("databaseName").toElement();
        if (element.isNull()){
            kDebug(50003) << "Missing element <databaseName>.";
        }
        l_Element.m_DatabaseName = element.text();

        element =  databaseElement.namedItem("userName").toElement();
        if (element.isNull()){
            kDebug(50003) << "Missing element <userName>.";
        }
        l_Element.m_UserName = element.text();

        element =  databaseElement.namedItem("password").toElement();
        if (element.isNull()){
            kDebug(50003) << "Missing element <password>.";
        }
        l_Element.m_Password = element.text();

        element =  databaseElement.namedItem("hostName").toElement();
        if (element.isNull()){
            kDebug(50003) << "Missing element <hostName>.";
        }
        l_Element.m_HostName = element.text();

        element =  databaseElement.namedItem("port").toElement();
        if (element.isNull()){
            kDebug(50003) << "Missing element <port>.";
        }
        l_Element.m_Port = element.text();

        element =  databaseElement.namedItem("connectoptions").toElement();
        if (element.isNull()){
            kDebug(50003) << "Missing element <connectoptions>.";
        }
        l_Element.m_ConnectOptions = element.text();

        element =  databaseElement.namedItem("dbactions").toElement();
        if (element.isNull()){
            kDebug(50003) << "Missing element <dbactions>.";
        }
        readDBActions(element, l_Element);

	return l_Element;
}

void DatabaseParameters::readDBActions(QDomElement& sqlStatementElements, databaseconfigelement& configElement){
         QDomElement dbActionElement =  sqlStatementElements.firstChildElement("dbaction");
         for( ; !dbActionElement.isNull();  dbActionElement=dbActionElement.nextSiblingElement("dbaction")){
                if (!dbActionElement.hasAttribute("name")){
                    kDebug(50003) << "Missing statement attribute <name>.";
                }
                databaseAction l_Action;
                l_Action.m_Name = dbActionElement.attribute("name");
                //kDebug(50003) << "Getting attribute " << dbActionElement.attribute("name");
		
		if (dbActionElement.hasAttribute("mode")){
		  l_Action.m_Mode = dbActionElement.attribute("mode");  
		}else{
		  kDebug(50003) << "Missing statement attribute <mode>. Setting to default \"transaction\".";
		  l_Action.m_Mode = QString("transaction"); 
		}
		

                QDomElement databaseElement =  dbActionElement.firstChildElement("statement");
                for( ; !databaseElement.isNull();  databaseElement=databaseElement.nextSiblingElement("statement")){

                    if (!databaseElement.hasAttribute("mode")){
                        kDebug(50003) << "Missing statement attribute <mode>.";
                    }

                    databaseActionElement l_ActionElement;
                    l_ActionElement.m_Mode      = databaseElement.attribute("mode");
                    l_ActionElement.m_Statement = databaseElement.text();

                    l_Action.m_DBActionElements.append(l_ActionElement);
                }
                configElement.m_SQLStatements.insert(l_Action.m_Name, l_Action);
            }
}

}  // namespace Digikam
