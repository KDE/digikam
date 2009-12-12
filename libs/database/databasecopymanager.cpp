/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009 by Holger Foerster <Hamsi2k at freenet dot de>
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
#include "databasecopymanager.h"

// KDE Includes
#include "kdebug.h"
#include "klocale.h"

// QT includes
#include <QSqlError>

// Local includes
#include "databasecorebackend.h"
#include "databaseparameters.h"
#include "albumdb.h"
#include "schemaupdater.h"

namespace Digikam
{
    void DatabaseCopyManager::copyDatabases(DatabaseParameters *fromDBParameters, DatabaseParameters *toDBParameters)
    {
        DatabaseLocking toLocking;
        DatabaseBackend toDBbackend(&toLocking, "MigrationToDatabase");
        toDBbackend.open(*toDBParameters);

        DatabaseLocking fromLocking;
        DatabaseBackend fromDBbackend(&fromLocking, "MigrationFromDatabase");
        fromDBbackend.open(*fromDBParameters);


        QMap<QString, QVariant> bindingMap;

        // Delete all tables
        QSqlQuery result = toDBbackend.execDBActionQuery(fromDBbackend.getDBAction("Migrate_Cleanup_DB"), bindingMap) ;

        // then create the schema
        AlbumDB       albumDB(&toDBbackend);
        SchemaUpdater updater(&albumDB, &toDBbackend, *toDBParameters);

        if (!updater.update())
        {
            emit finishedFailure(i18n("Error while converting the database."));
        }

        emit stepStarted(i18n("Copy AlbumRoots..."));
        // now perform the copy action
        if (!copyTable(fromDBbackend, QString("Migrate_Read_AlbumRoots"), toDBbackend, QString("Migrate_Write_AlbumRoots")))
        {
            return;
        }
        emit stepStarted(i18n("Copy Albums..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_Albums"), toDBbackend, QString("Migrate_Write_Albums")))
        {
            return;
        }
        emit stepStarted(i18n("Copy Images..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_Images"), toDBbackend, QString("Migrate_Write_Images")))
        {
            return;
        }
        emit stepStarted(i18n("Copy ImageHaarMatrix..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_ImageHaarMatrix"), toDBbackend, QString("Migrate_Write_ImageHaarMatrix")))
        {
            return;
        }
        emit stepStarted(i18n("Copy ImageInformation..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_ImageInformation"), toDBbackend, QString("Migrate_Write_ImageInformation")))
        {
            return;
        }
        emit stepStarted(i18n("Copy ImageMetadata..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_ImageMetadata"), toDBbackend, QString("Migrate_Write_ImageMetadata")))
        {
            return;
        }
        emit stepStarted(i18n("Copy ImagePositions..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_ImagePositions"), toDBbackend, QString("Migrate_Write_ImagePositions")))
        {
            return;
        }
        emit stepStarted(i18n("Copy ImageComments..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_ImageComments"), toDBbackend, QString("Migrate_Write_ImageComments")))
        {
            return;
        }
        emit stepStarted(i18n("Copy ImageCopyright..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_ImageCopyright"), toDBbackend, QString("Migrate_Write_ImageCopyright")))
        {
            return;
        }
        emit stepStarted(i18n("Copy Tags..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_Tags"), toDBbackend, QString("Migrate_Write_Tags")))
        {
            return;
        }
        emit stepStarted(i18n("Copy ImageTags..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_ImageTags"), toDBbackend, QString("Migrate_Write_ImageTags")))
        {
            return;
        }
        emit stepStarted(i18n("Copy ImageProperties..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_ImageProperties"), toDBbackend, QString("Migrate_Write_ImageProperties")))
        {
            return;
        }
        emit stepStarted(i18n("Copy Searches..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_Searches"), toDBbackend, QString("Migrate_Write_Searches")))
        {
            return;
        }
        emit stepStarted(i18n("Copy DownloadHistory..."));
        if (!copyTable(fromDBbackend, QString("Migrate_Read_DownloadHistory"), toDBbackend, QString("Migrate_Write_DownloadHistory")))
        {
            return;
        }
/*
        if (!copyTable(fromDBbackend, QString("Migrate_Read_Settings"), toDBbackend, QString("Migrate_Write_Settings")))
        {
            return;
        }
*/
        emit finishedSuccessfully();
    }

    bool DatabaseCopyManager::copyTable(DatabaseBackend &fromDBbackend, QString fromActionName, DatabaseBackend &toDBbackend, QString toActionName)
    {
        kDebug(50003) << "Trying to copy contents from DB with ActionName: ["<< fromActionName << "] to DB with ActionName ["<< toActionName <<"]";

        QMap<QString, QVariant> bindingMap;
        // now perform the copy action
        QList<QString> columnNames;
        QSqlQuery result = fromDBbackend.execDBActionQuery(fromDBbackend.getDBAction(fromActionName), bindingMap) ;

        int columnCount = result.record().count();
        for (int i=0; i<columnCount; i++)
        {
            kDebug(50003) << "Column: ["<< result.record().fieldName(i) << "]";
            columnNames.append(result.record().fieldName(i));
        }

        kDebug(50003) << "Result size: ["<< result.size() << "]";
        while (result.next())
        {
            // read the values from the fromDB into a hash
            QMap<QString, QVariant> tempBindingMap;
            int i=0;
            foreach (QString columnName, columnNames)
            {
                kDebug(50003) << "Column: ["<< columnName << "] value ["<<result.value(i)<<"]";
                tempBindingMap.insert(columnName.insert(0, ':'), result.value(i));
                i++;
            }
            // insert the previous requested values to the toDB
            QSqlQuery resultQuery;
            databaseAction action = toDBbackend.getDBAction(toActionName);
            toDBbackend.execDBAction(action, tempBindingMap);

            if (toDBbackend.lastSQLError().isValid() && toDBbackend.lastSQLError().number()!=0)
            {
                QString errorMsg = i18n("Error while converting the database. \n Details: %1", resultQuery.lastError().databaseText());
                emit finishedFailure(errorMsg);
                return false;
            }

        }
        return true;
    }
}
