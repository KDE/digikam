/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Schema update
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

#ifndef SCHEMAUPDATER_H
#define SCHEMAUPDATER_H

// Qt includes.

#include <QString>

namespace Digikam
{

class DatabaseAccess;
class InitializationObserver;

class SchemaUpdater
{
public:

    SchemaUpdater(DatabaseAccess *access);

    static int schemaVersion();
    static int filterSettingsVersion();
    bool update();
    void setObserver(InitializationObserver *observer);

private:

    bool startUpdates();
    bool makeUpdates();
    void defaultFilterSettings(QStringList &defaultImageFilter,
                               QStringList &defaultVideoFilter,
                               QStringList &defaultAudioFilter);
    bool createFilterSettings();
    bool updateFilterSettings();
    bool createDatabase();
    bool createTablesV5();
    bool createIndicesV5();
    bool createTriggersV5();
    bool copyV3toV4(const QString &digikam3DBPath, const QString &currentDBPath);
    bool updateV4toV5();
    bool updateV2toV4(const QString &sqlite2DBPath);
    bool createTablesV3();

private:

    DatabaseAccess         *m_access;

    int                     m_currentVersion;
    InitializationObserver *m_observer;
    bool                    m_setError;

    void preAlpha010Update1();
    void preAlpha010Update2();
    void preAlpha010Update3();
    void beta010Update1();
    void beta010Update2();

};

}  // namespace Digikam

#endif // SCHEMAUPDATER_H
