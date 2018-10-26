/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-11
 * Description : Qt item model for database entries - private header
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_ITEM_FILTER_MODEL_THREADS_H
#define DIGIKAM_ITEM_FILTER_MODEL_THREADS_H

// Qt includes

#include <QThread>

// Local includes

#include "digikam_export.h"
#include "workerobject.h"
#include "itemfiltermodel.h"

namespace Digikam
{

class ItemFilterModelTodoPackage;
    
class DIGIKAM_DATABASE_EXPORT ItemFilterModelWorker : public WorkerObject
{
    Q_OBJECT

public:

    explicit ItemFilterModelWorker(ItemFilterModel::ItemFilterModelPrivate* const d);

    bool checkVersion(const ItemFilterModelTodoPackage& package);

public Q_SLOTS:

    virtual void process(ItemFilterModelTodoPackage package) = 0;

Q_SIGNALS:

    void processed(const ItemFilterModelTodoPackage& package);
    void discarded(const ItemFilterModelTodoPackage& package);

protected:

    ItemFilterModel::ItemFilterModelPrivate* d;
};

// -----------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemFilterModelPreparer : public ItemFilterModelWorker
{
    Q_OBJECT

public:

    explicit ItemFilterModelPreparer(ItemFilterModel::ItemFilterModelPrivate* const d)
        : ItemFilterModelWorker(d)
    {
    }

    void process(ItemFilterModelTodoPackage package);
};

// ----------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemFilterModelFilterer : public ItemFilterModelWorker
{
    Q_OBJECT

public:

    explicit ItemFilterModelFilterer(ItemFilterModel::ItemFilterModelPrivate* const d)
        : ItemFilterModelWorker(d)
    {
    }

    void process(ItemFilterModelTodoPackage package);
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_FILTER_MODEL_THREADS_H
