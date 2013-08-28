/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Schema update
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef SCHEMAUPDATER_H
#define SCHEMAUPDATER_H

// Qt includes

#include <QString>
#include <QVariant>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "databasebackend.h"
#include "databaseparameters.h"

namespace Digikam
{

class DatabaseAccess;
class InitializationObserver;

class DIGIKAM_DATABASE_EXPORT SchemaUpdater
{
public:

    static int  schemaVersion();
    static int  filterSettingsVersion();
    static int  uniqueHashVersion();
    static bool isUniqueHashUpToDate();

public:

    SchemaUpdater(AlbumDB* const albumDB, DatabaseBackend* const backend, DatabaseParameters parameters);
    ~SchemaUpdater();

    bool  update();
    bool  updateUniqueHash();
    void  setObserver(InitializationObserver* const observer);
    const QString getLastErrorMessage();
    void  setDatabaseAccess(DatabaseAccess* const access);

private:

    bool startUpdates();
    bool makeUpdates();
    bool beginWrapSchemaUpdateStep();
    bool endWrapSchemaUpdateStep(bool stepOperationSuccess, const QString& errorMsg);
    void defaultFilterSettings(QStringList& defaultImageFilter,
                               QStringList& defaultVideoFilter,
                               QStringList& defaultAudioFilter);
    bool createFilterSettings();
    bool updateFilterSettings();
    bool createDatabase();
    bool createTables();
    bool createIndices();
    bool createTriggers();
    bool copyV3toV4(const QString& digikam3DBPath, const QString& currentDBPath);
    bool performUpdateToVersion(const QString& actionName, int newVersion, int newRequiredVersion);
    bool updateToVersion(int targetVersion);
    bool updateV4toV7();
    bool updateV2toV4(const QString& sqlite2DBPath);
    void setLegacySettingEntries();
    void readVersionSettings();
    void setVersionSettings();

private:

    bool createTablesV3();
    void preAlpha010Update1();
    void preAlpha010Update2();
    void preAlpha010Update3();
    void beta010Update1();
    void beta010Update2();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SCHEMAUPDATER_H
