/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-30
 * Description : Similarity DB schema update
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2017 by Swati  Lodha   <swatilodha27 at gmail dot com>
 * Copyright (C)      2018 by Mario Frank    <mario dot frank at uni minus potsdam dot de>
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

#ifndef SIMILARITY_DB_SCHEMA_UPDATER_H
#define SIMILARITY_DB_SCHEMA_UPDATER_H

namespace Digikam
{

class SimilarityDbAccess;
class InitializationObserver;

class SimilarityDbSchemaUpdater
{
public:

    static int schemaVersion();

public:

    explicit SimilarityDbSchemaUpdater(SimilarityDbAccess* const access);
    ~SimilarityDbSchemaUpdater();

    bool update();
    void setObserver(InitializationObserver* const observer);

private:

    bool startUpdates();
    bool makeUpdates();
    bool createDatabase();
    bool createTables();
    bool createIndices();
    bool createTriggers();
    bool updateV1ToV2();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // SIMILARITY_DB_SCHEMA_UPDATER_H
