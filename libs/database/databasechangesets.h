/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-01
 * Description : Recording changes on the database
 *
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASECHANGESETS_H
#define DATABASECHANGESETS_H

// Qt includes

#include <QList>

// Local includes

#include "digikam_export.h"
#include "databasefields.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImageChangeset
{
public:

    /**
     * An ImageChangeset covers adding or changing any properties of an image.
     * It is described by a list of affected image ids, and a set of affected database fields.
     * There is no guarantee that information in the database has actually been changed.
     */

    ImageChangeset();
    ImageChangeset(QList<qlonglong> ids, DatabaseFields::Set changes);
    ImageChangeset(qlonglong id, DatabaseFields::Set changes);

    QList<qlonglong> ids() const;
    bool containsImage(qlonglong id) const;
    DatabaseFields::Set changes() const;

private:

    QList<qlonglong>    m_ids;
    DatabaseFields::Set m_changes;
};

class DIGIKAM_EXPORT ImageTagChangeset
{
public:

    /**
     * An ImageTagChangeset covers adding and removing the association of a tag with an image.
     * It is described by a list of affected image ids, a list of affected tags,
     * and an operation.
     * There is no guarantee that information in the database has actually been changed.
     * Special case:
     * If all tags have been removed from an item, operation is Removed | RemovedAll,
     * and the tags list is empty. containsTag() will always return true in this case.
     */

    enum Operations
    {
        Unknown    = 1 << 0,
        Added      = 1 << 1,
        Removed    = 1 << 2,
        RemovedAll = 1 << 3,

        RemovedAllOperation = Removed | RemovedAll
    };
    Q_DECLARE_FLAGS(Operation, Operations)

    ImageTagChangeset();
    ImageTagChangeset(QList<qlonglong> ids, QList<int> tags, Operation operation = Unknown);
    ImageTagChangeset(qlonglong id, QList<int> tags, Operation operation = Unknown);
    ImageTagChangeset(qlonglong id, int tag, Operation operation = Unknown);

    QList<qlonglong> ids() const;
    bool containsImage(qlonglong id) const;
    QList<int> tags() const;
    bool containsTag(int id);
    Operation operation() const;

    bool tagsWereAdded() const
    { return operation() & Added; }
    bool tagsWereRemoved() const
    { return operation() & Removed; }

private:

    QList<qlonglong>    m_ids;
    QList<int>          m_tags;
    Operation           m_operation;
};



}

#endif //DATABASECHANGESETS_H

