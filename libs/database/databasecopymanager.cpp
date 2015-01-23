/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
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

#include "databasecopymanager.moc"

// QT includes

#include <QSqlError>
#include <QSqlDriver>
#include <QSqlRecord>

// KDE Includes

#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "databasecorebackend.h"
#include "databaseparameters.h"
#include "albumdb.h"
#include "schemaupdater.h"

namespace Digikam
{

DatabaseCopyManager::DatabaseCopyManager()
{
    m_isStopProcessing = false;
}

DatabaseCopyManager::~DatabaseCopyManager()
{
}

void DatabaseCopyManager::stopProcessing()
{
    m_isStopProcessing = true;
}

void DatabaseCopyManager::copyDatabases(DatabaseParameters fromDBParameters, DatabaseParameters toDBParameters)
{
    m_isStopProcessing = false;
    DatabaseLocking fromLocking;
    DatabaseBackend fromDBbackend(&fromLocking, "MigrationFromDatabase");

    if (!fromDBbackend.open(fromDBParameters))
    {
        emit finished(DatabaseCopyManager::failed, i18n("Error while opening the source database."));
        return;
    }

    DatabaseLocking toLocking;
    DatabaseBackend toDBbackend(&toLocking, "MigrationToDatabase");

    if (!toDBbackend.open(toDBParameters))
    {
        emit finished(DatabaseCopyManager::failed, i18n("Error while opening the target database."));
        fromDBbackend.close();
        return;
    }

    // order may be important, array class must _not_ be sorted
    const QStringList tables = QStringList()
        << "AlbumRoots"
        << "Albums"
        << "Images"
        << "ImageHaarMatrix"
        << "ImageInformation"
        << "ImageMetadata"
        << "ImageTagProperties"
        << "TagProperties"
        << "ImagePositions"
        << "ImageComments"
        << "ImageCopyright"
        << "Tags"
        << "ImageTags"
        << "ImageProperties"
        << "ImageHistory"
        << "ImageRelations"
        << "Searches"
        << "DownloadHistory"
        << "VideoMetadata"
        /*
        << "Settings"
        */
    ;
    const int tablesSize = tables.size();

    QMap<QString, QVariant> bindingMap;

    // Delete all tables
    for (int i=0; m_isStopProcessing || i < tablesSize; ++i)
    {
        if (toDBbackend.execDirectSql(QString("DROP TABLE IF EXISTS %1;").arg(tables[i])) != DatabaseCoreBackend::NoErrors)
        {
            emit finished(DatabaseCopyManager::failed, i18n("Error while scrubbing the target database."));
            fromDBbackend.close();
            toDBbackend.close();
            return;
        }
    }
    if (toDBbackend.execDirectSql(QString("DROP TABLE IF EXISTS Settings;")) != DatabaseCoreBackend::NoErrors)
    {
        emit finished(DatabaseCopyManager::failed, i18n("Error while scrubbing the target database."));
        fromDBbackend.close();
        toDBbackend.close();
        return;
    }

    // then create the schema
    AlbumDB       albumDB(&toDBbackend);
    SchemaUpdater updater(&albumDB, &toDBbackend, toDBParameters);

    emit stepStarted(i18n("Create Schema..."));

    if (!updater.update())
    {
        emit finished(DatabaseCopyManager::failed, i18n("Error while creating the database schema."));
        fromDBbackend.close();
        toDBbackend.close();
        return;
    }

    /*
     * loop copying the tables, stop if an error is met
     */
    for (int i=0; m_isStopProcessing || i < tablesSize; ++i)
    {
        emit stepStarted(i18n(QString("Copy %1...").arg(tables[i]).toLatin1()));

        // now perform the copy action
        if ( m_isStopProcessing ||
             !copyTable(fromDBbackend, QString("Migrate_Read_%1").arg(tables[i]),
                        toDBbackend, QString("Migrate_Write_%1").arg(tables[i]))
           )
        {
            handleClosing(m_isStopProcessing, fromDBbackend, toDBbackend);
            return;
        }
    }

/*
    if (isStopThread || !copyTable(fromDBbackend, QString("Migrate_Read_Settings"), toDBbackend, QString("Migrate_Write_Settings")))
    {
        handleClosing(isStopThread, fromDBbackend, toDBbackend);
        return;
    }
*/
    fromDBbackend.close();
    toDBbackend.close();

    emit finished(DatabaseCopyManager::success, QString());
}

bool DatabaseCopyManager::copyTable(DatabaseBackend& fromDBbackend, const QString& fromActionName, 
                                    DatabaseBackend& toDBbackend, const QString& toActionName)
{
    kDebug(50003) << "Trying to copy contents from DB with ActionName: [" << fromActionName
                  << "] to DB with ActionName [" << toActionName << "]";

    QMap<QString, QVariant> bindingMap;

    // now perform the copy action
    QList<QString> columnNames;
    QSqlQuery      result        = fromDBbackend.execDBActionQuery(fromDBbackend.getDBAction(fromActionName), bindingMap) ;
    int            resultSize    = -1;
    bool           isForwardOnly = result.isForwardOnly();

    if (result.driver()->hasFeature(QSqlDriver::QuerySize))
    {
        resultSize=result.size();
    }
    else
    {
        kDebug(50003) << "Driver doesn't support query size. We try to go to the last row and back to the current.";
        result.last();
        /*
         * Now get the current row. If this is not possible, a value lower than 0 will be returned.
         * To not confuse the log reading user, we reset this value to 0.
         */
        resultSize = result.at()<0 ? 0 : result.at();
        /*
        * avoid a misleading error message, query is redone if isForwardOnly
        */
        if ( ! isForwardOnly)
        {
            result.first();
        }
    }

    kDebug(50003) << "Result size: ["<< resultSize << "]";

    /*
     * If the sql query is forward only - perform the query again.
     * This is not atomic, so it can be tend to different results between
     * real database entries copied and shown at the progressbar.
     */
    if (isForwardOnly)
    {
        result.finish();
        result = fromDBbackend.execDBActionQuery(fromDBbackend.getDBAction(fromActionName), bindingMap) ;
    }

    int columnCount = result.record().count();

    for (int i=0; i<columnCount; ++i)
    {
        //            kDebug(50003) << "Column: ["<< result.record().fieldName(i) << "]";
        columnNames.append(result.record().fieldName(i));
    }

    int resultCounter = 0;

    while (result.next())
    {
        kDebug(50003) << "Query isOnValidRow [" << result.isValid() << "] isActive [" << result.isActive()
                      << "] result size: [" << result.size() << "]";

        if (m_isStopProcessing == true)
        {
            return false;
        }

        // Send a signal to the GUI to entertain the user
        emit smallStepStarted(++resultCounter, resultSize);

        // read the values from the fromDB into a hash
        QMap<QString, QVariant> tempBindingMap;
        int i = 0;

        foreach(QString columnName, columnNames) // krazy:exclude=foreach
        {
            kDebug(50003) << "Column: ["<< columnName << "] value ["<<result.value(i)<<"]";
            tempBindingMap.insert(columnName.insert(0, ':'), result.value(i));
            ++i;
        }

        // insert the previous requested values to the toDB
        DatabaseAction action = toDBbackend.getDBAction(toActionName);
        DatabaseCoreBackend::QueryState queryStateResult = toDBbackend.execDBAction(action, tempBindingMap);

        if (queryStateResult != DatabaseCoreBackend::NoErrors &&
            toDBbackend.lastSQLError().isValid()              &&
            toDBbackend.lastSQLError().number() != 0)
        {
            kDebug(50003) << "Error while converting table data. Details: " << toDBbackend.lastSQLError();
            QString errorMsg = i18n("Error while converting the database. \n Details: %1", toDBbackend.lastSQLError().databaseText());
            emit finished(DatabaseCopyManager::failed, errorMsg);
            return false;
        }
    }

    return true;
}

void DatabaseCopyManager::handleClosing(bool isStopThread, DatabaseBackend& fromDBbackend, DatabaseBackend& toDBbackend)
{
    if (isStopThread)
    {
        emit finished(DatabaseCopyManager::canceled, "");
    }

    fromDBbackend.close();
    toDBbackend.close();
}

} // namespace Digikam
