/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Core database Schema updater
 *
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef COREDB_SCHEMA_UPDATER_H
#define COREDB_SCHEMA_UPDATER_H

// Qt includes

#include <QString>
#include <QVariant>

// Local includes

#include "coredb.h"
#include "coredbaccess.h"
#include "coredbbackend.h"
#include "dbengineparameters.h"

namespace Digikam
{

class CoreDbAccess;
class InitializationObserver;

class DIGIKAM_DATABASE_EXPORT CoreDbSchemaUpdater
{
public:

    static int  schemaVersion();
    static int  filterSettingsVersion();
    static int  uniqueHashVersion();
    static bool isUniqueHashUpToDate();

public:

    explicit CoreDbSchemaUpdater(CoreDB* const albumDB, CoreDbBackend* const backend, DbEngineParameters parameters);
    virtual ~CoreDbSchemaUpdater();

    bool  update();
    bool  updateUniqueHash();
    void  setObserver(InitializationObserver* const observer);
    const QString getLastErrorMessage();
    void  setCoreDbAccess(CoreDbAccess* const access);

private:

    bool startUpdates();
    bool makeUpdates();
    bool beginWrapSchemaUpdateStep();
    bool endWrapSchemaUpdateStep(bool stepOperationSuccess, const QString& errorMsg);
    void defaultFilterSettings(QStringList& defaultImageFilter,
                               QStringList& defaultVideoFilter,
                               QStringList& defaultAudioFilter);
    void defaultIgnoreDirectoryFilterSettings(QStringList& defaultIgnoreDirectoryFilter);
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

} // namespace Digikam

#endif // COREDB_SCHEMA_UPDATER_H
