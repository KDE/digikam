/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Abstract database backend
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "databasebackend.moc"
#include "databasebackend_p.h"

// Qt includes

#include <QApplication>
#include <QCoreApplication>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QThread>

// KDE includes


#include <kglobal.h>

// Local includes

#include "schemaupdater.h"
#include "thumbnailschemaupdater.h"
#include "databasewatch.h"

namespace Digikam
{

DatabaseBackend::DatabaseBackend(DatabaseLocking* locking, const QString& backendName)
    : DatabaseCoreBackend(backendName, locking, *new DatabaseBackendPrivate(this))
{
}

DatabaseBackend::~DatabaseBackend()
{
}

void DatabaseBackend::setDatabaseWatch(DatabaseWatch* watch)
{
    Q_D(DatabaseBackend);
    d->watch = watch;
}

bool DatabaseBackend::initSchema(SchemaUpdater* updater)
{
    Q_D(DatabaseBackend);

    if (d->status == OpenSchemaChecked)
    {
        return true;
    }

    if (d->status == Unavailable)
    {
        return false;
    }

    if (updater->update())
    {
        d->status = OpenSchemaChecked;
        return true;
    }

    return false;
}

void DatabaseBackend::recordChangeset(const ImageChangeset& changeset)
{
    Q_D(DatabaseBackend);
    // if we want to do compression of changesets, think about doing this here
    d->imageChangesetContainer.recordChangeset(changeset);
}

void DatabaseBackend::recordChangeset(const ImageTagChangeset& changeset)
{
    Q_D(DatabaseBackend);
    d->imageTagChangesetContainer.recordChangeset(changeset);
}

void DatabaseBackend::recordChangeset(const CollectionImageChangeset& changeset)
{
    Q_D(DatabaseBackend);
    d->collectionImageChangesetContainer.recordChangeset(changeset);
}

void DatabaseBackend::recordChangeset(const AlbumChangeset& changeset)
{
    Q_D(DatabaseBackend);
    d->albumChangesetContainer.recordChangeset(changeset);
}

void DatabaseBackend::recordChangeset(const TagChangeset& changeset)
{
    Q_D(DatabaseBackend);
    d->tagChangesetContainer.recordChangeset(changeset);
}

void DatabaseBackend::recordChangeset(const AlbumRootChangeset& changeset)
{
    Q_D(DatabaseBackend);
    d->albumRootChangesetContainer.recordChangeset(changeset);
}

void DatabaseBackend::recordChangeset(const SearchChangeset& changeset)
{
    Q_D(DatabaseBackend);
    d->searchChangesetContainer.recordChangeset(changeset);
}

}  // namespace Digikam
