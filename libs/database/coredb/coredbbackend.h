/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Core database abstract backend.
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef COREDATABASEBACKEND_H
#define COREDATABASEBACKEND_H

// Local includes

#include "digikam_export.h"
#include "dbenginebackend.h"
#include "coredbchangesets.h"

namespace Digikam
{

class CoreDbSchemaUpdater;
class CoreDbWatch;
class CoreDbBackendPrivate;

class DIGIKAM_DATABASE_EXPORT CoreDbBackend : public BdEngineBackend
{
    Q_OBJECT

public:

    explicit CoreDbBackend(DbEngineLocking* const locking, const QString& backendName = QLatin1String("digikamDatabase-"));
    ~CoreDbBackend();

    /** Sets the global database watch
     */
    void setCoreDbWatch(CoreDbWatch* watch);

    /**
     * Initialize the database schema to the current version,
     * carry out upgrades if necessary.
     * Shall only be called from the thread that called open().
     */
    bool initSchema(CoreDbSchemaUpdater* updater);

    /**
     * Notify all listeners of the changeset
     */
    void recordChangeset(const ImageChangeset& changeset);
    void recordChangeset(const ImageTagChangeset& changeset);
    void recordChangeset(const CollectionImageChangeset& changeset);
    void recordChangeset(const AlbumChangeset& changeset);
    void recordChangeset(const TagChangeset& changeset);
    void recordChangeset(const AlbumRootChangeset& changeset);
    void recordChangeset(const SearchChangeset& changeset);

private:

    Q_DECLARE_PRIVATE(CoreDbBackend)
};

} // namespace Digikam

#endif // COREDATABASEBACKEND_H
