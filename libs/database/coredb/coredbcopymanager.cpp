/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : Core database copy manager for migration operations.
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
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

#include "coredbcopymanager.h"

// Qt includes

#include <QSqlError>
#include <QSqlDriver>
#include <QSqlRecord>

// KDE Includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dbenginebackend.h"
#include "dbengineparameters.h"
#include "coredb.h"
#include "coredbschemaupdater.h"

namespace Digikam
{

CoreDbCopyManager::CoreDbCopyManager()
{
    m_isStopProcessing = false;
}

CoreDbCopyManager::~CoreDbCopyManager()
{
}

void CoreDbCopyManager::stopProcessing()
{
    m_isStopProcessing = true;
}

void CoreDbCopyManager::copyDatabases(const DbEngineParameters& fromDBParameters, DbEngineParameters& toDBParameters)
{
    m_isStopProcessing = false;
    DbEngineLocking fromLocking;
    CoreDbBackend fromDBbackend(&fromLocking, QLatin1String("MigrationFromDatabase"));

    if (!fromDBbackend.open(fromDBParameters))
    {
        emit finished(CoreDbCopyManager::failed, i18n("Error while opening the source database."));
        return;
    }

    DbEngineLocking toLocking;
    CoreDbBackend toDBbackend(&toLocking, QLatin1String("MigrationToDatabase"));

    if (!toDBbackend.open(toDBParameters))
    {
        emit finished(CoreDbCopyManager::failed, i18n("Error while opening the target database."));
        fromDBbackend.close();
        return;
    }

    // Order may be important, array class must _not_ be sorted

    const QStringList tables = QStringList()
        << QLatin1String("AlbumRoots")
        << QLatin1String("Albums")
        << QLatin1String("Images")
        // Virtual table used to allow population of Albums.icon after Images migration
        << QLatin1String("AlbumsExtra")
        << QLatin1String("ImageInformation")
        << QLatin1String("ImageMetadata")
        << QLatin1String("ImagePositions")
        << QLatin1String("ImageComments")
        << QLatin1String("ImageCopyright")
        << QLatin1String("Tags")
        << QLatin1String("TagProperties")
        << QLatin1String("ImageTagProperties")
        << QLatin1String("ImageTags")
        << QLatin1String("ImageProperties")
        << QLatin1String("ImageHistory")
        << QLatin1String("ImageRelations")
        << QLatin1String("Searches")
        << QLatin1String("DownloadHistory")
        << QLatin1String("VideoMetadata")
        << QLatin1String("Settings")
    ;
    const int tablesSize = tables.size();

    QMap<QString, QVariant> bindingMap;

    // Run any database specific preparatory cleanup prior to dropping tables.

    DbEngineAction action                        = toDBbackend.getDBAction(QString::fromUtf8("Migrate_Cleanup_Prepare"));
    BdEngineBackend::QueryState queryStateResult = toDBbackend.execDBAction(action);
    // Accept SQL error because the foreign key may not exist.
    if (queryStateResult == BdEngineBackend::ConnectionError)
    {
        emit finished(CoreDbCopyManager::failed, i18n("Error while preparing the target database."));
    }

    // Delete all tables

    for (int i = (tablesSize - 1); m_isStopProcessing || i >= 0; --i)
    {
        if ( m_isStopProcessing ||
             toDBbackend.execDirectSql(QString::fromUtf8("DROP TABLE IF EXISTS %1;").arg(tables[i])) != BdEngineBackend::NoErrors)
        {
            emit finished(CoreDbCopyManager::failed, i18n("Error while scrubbing the target database."));
            fromDBbackend.close();
            toDBbackend.close();
            return;
        }
    }

    // Then create the schema

    CoreDB       albumDB(&toDBbackend);
    CoreDbSchemaUpdater updater(&albumDB, &toDBbackend, toDBParameters);

    emit stepStarted(i18n("Create Schema..."));

    if (!updater.update())
    {
        emit finished(CoreDbCopyManager::failed, i18n("Error while creating the database schema."));
        fromDBbackend.close();
        toDBbackend.close();
        return;
    }

    // loop copying the tables, stop if an error is met

    for (int i = 0; m_isStopProcessing || i < tablesSize; ++i)
    {
        if (i < tablesSize)
        {
            emit stepStarted(i18n(QString::fromUtf8("Copy %1...").arg(tables[i]).toLatin1().constData()));
        }

        // Now perform the copy action

        if ( m_isStopProcessing ||
             !copyTable(fromDBbackend, QString::fromUtf8("Migrate_Read_%1").arg(tables[i]),
                        toDBbackend, QString::fromUtf8("Migrate_Write_%1").arg(tables[i]))
           )
        {
            handleClosing(m_isStopProcessing, fromDBbackend, toDBbackend);
            return;
        }
    }

    fromDBbackend.close();
    toDBbackend.close();

    emit smallStepStarted(1, 1);
    emit finished(CoreDbCopyManager::success, QString());
}

bool CoreDbCopyManager::copyTable(CoreDbBackend& fromDBbackend, const QString& fromActionName,
                                  CoreDbBackend& toDBbackend, const QString& toActionName)
{
    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: trying to copy contents from DB with ActionName: [" << fromActionName
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
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: driver doesn't support query size. "
                                       "We try to go to the last row and back to the current.";

        result.last();

        // Now get the current row. If this is not possible, a value lower than 0 will be returned.
        // To not confuse the log reading user, we reset this value to 0.

        resultSize = (result.at() < 0) ? 0 : result.at();

        // Avoid a misleading error message, query is redone if isForwardOnly

        if ( ! isForwardOnly)
        {
            result.first();
        }
    }

    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: result size: ["<< resultSize << "]";

    // If the sql query is forward only - perform the query again.
    // This is not atomic, so it can be tend to different results between
    // real database entries copied and shown at the progressbar.

    if (isForwardOnly)
    {
        result.finish();
        result = fromDBbackend.execDBActionQuery(fromDBbackend.getDBAction(fromActionName), bindingMap) ;
    }

    int columnCount = result.record().count();

    for (int i = 0; i < columnCount; ++i)
    {
        //qCDebug(DIGIKAM_COREDB_LOG) << "Column: ["<< result.record().fieldName(i) << "]";
        columnNames.append(result.record().fieldName(i));
    }

    int resultCounter = 0;

    while (result.next())
    {
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: query isOnValidRow [" << result.isValid()
                                    << "] isActive [" << result.isActive()
                                    << "] result size: [" << result.size() << "]";

        if (m_isStopProcessing == true)
        {
            return false;
        }

        // Send a signal to the GUI to entertain the user

        emit smallStepStarted(resultCounter++, resultSize);

        // Read the values from the fromDB into a hash

        QMap<QString, QVariant> tempBindingMap;
        int i = 0;

        foreach(QString columnName, columnNames) // krazy:exclude=foreach
        {
            qCDebug(DIGIKAM_COREDB_LOG) << "Core database: column: ["
                                        << columnName << "] value ["
                                        << result.value(i) << "]";
            tempBindingMap.insert(columnName.insert(0, QLatin1Char(':')), result.value(i));
            ++i;
        }

        // Insert the previous requested values to the DB

        DbEngineAction action                        = toDBbackend.getDBAction(toActionName);
        BdEngineBackend::QueryState queryStateResult = toDBbackend.execDBAction(action, tempBindingMap);

        if (queryStateResult != BdEngineBackend::NoErrors &&
            toDBbackend.lastSQLError().isValid()          &&
            toDBbackend.lastSQLError().number() != 0)
        {
            qCDebug(DIGIKAM_COREDB_LOG) << "Core database: error while converting table data. Details: " << toDBbackend.lastSQLError();
            QString errorMsg = i18n("Error while converting the database.\n Details: %1", toDBbackend.lastSQLError().databaseText());
            emit finished(CoreDbCopyManager::failed, errorMsg);
            return false;
        }
    }

    return true;
}

void CoreDbCopyManager::handleClosing(bool isStopThread, CoreDbBackend& fromDBbackend, CoreDbBackend& toDBbackend)
{
    if (isStopThread)
    {
        emit finished(CoreDbCopyManager::canceled, QLatin1String(""));
    }

    fromDBbackend.close();
    toDBbackend.close();
}

} // namespace Digikam
