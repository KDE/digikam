/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-11
 * Description : Qt item model for database entries - private containers
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

#ifndef DIGIKAM_ITEM_FILTER_MODEL_P_H
#define DIGIKAM_ITEM_FILTER_MODEL_P_H

// Qt includes

#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>

// Local includes

#include "iteminfo.h"
#include "itemfiltermodel.h"
#include "digikam_export.h"

// NOTE: we need the EXPORT macro in a private header because
// this private header is shared across binary objects.
// This does NOT make this classes here any more public!

namespace Digikam
{

const int PrepareChunkSize = 101;
const int FilterChunkSize  = 2001;

class ItemFilterModelTodoPackage
{
public:

    ItemFilterModelTodoPackage()
        : version(0),
          isForReAdd(false)
    {
    }

    ItemFilterModelTodoPackage(const QVector<ItemInfo>& infos, const QVector<QVariant>& extraValues, int version, bool isForReAdd)
        : infos(infos),
          extraValues(extraValues),
          version(version),
          isForReAdd(isForReAdd)
    {
    }

    QVector<ItemInfo>     infos;
    QVector<QVariant>      extraValues;
    unsigned int           version;
    bool                   isForReAdd;
    QHash<qlonglong, bool> filterResults;
};

// ------------------------------------------------------------------------------------------------

class ItemFilterModelPreparer;
class ItemFilterModelFilterer;

class DIGIKAM_DATABASE_EXPORT ItemFilterModel::ItemFilterModelPrivate : public QObject
{
    Q_OBJECT

public:

    explicit ItemFilterModelPrivate();
    ~ItemFilterModelPrivate();

    void init(ItemFilterModel* q);
    void setupWorkers();
    void infosToProcess(const QList<ItemInfo>& infos);
    void infosToProcess(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues, bool forReAdd = true);

public:

    ItemFilterModel*                   q;

    ItemModel*                         imageModel;

    ItemFilterSettings                 filter;
    ItemSortSettings                   sorter;
    VersionItemFilterSettings          versionFilter;
    GroupItemFilterSettings            groupFilter;

    volatile unsigned int               version;
    unsigned int                        lastDiscardVersion;
    unsigned int                        lastFilteredVersion;
    int                                 sentOut;
    int                                 sentOutForReAdd;

    QTimer*                             updateFilterTimer;

    bool                                needPrepare;
    bool                                needPrepareComments;
    bool                                needPrepareTags;
    bool                                needPrepareGroups;

    QMutex                              mutex;
    ItemFilterSettings                 filterCopy;
    VersionItemFilterSettings          versionFilterCopy;
    GroupItemFilterSettings            groupFilterCopy;
    ItemFilterModelPreparer*           preparer;
    ItemFilterModelFilterer*           filterer;

    QHash<qlonglong, bool>              filterResults;
    bool                                hasOneMatch;
    bool                                hasOneMatchForText;

    QList<ItemFilterModelPrepareHook*> prepareHooks;

/*
    QHash<int, QSet<qlonglong> >        categoryCountHashInt;
    QHash<QString, QSet<qlonglong> >    categoryCountHashString;

public:

        void cacheCategoryCount(int id, qlonglong imageid) const
        { const_cast<ItemFilterModelPrivate*>(this)->categoryCountHashInt[id].insert(imageid); }
        void cacheCategoryCount(const QString& id, qlonglong imageid) const
        { const_cast<ItemFilterModelPrivate*>(this)->categoryCountHashString[id].insert(imageid); }
*/

public Q_SLOTS:

    void preprocessInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues);
    void processAddedInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues);
    void packageFinished(const ItemFilterModelTodoPackage& package);
    void packageDiscarded(const ItemFilterModelTodoPackage& package);

Q_SIGNALS:

    void packageToPrepare(const ItemFilterModelTodoPackage& package);
    void packageToFilter(const ItemFilterModelTodoPackage& package);
    void reAddItemInfos(const QList<ItemInfo>& infos, const QList<QVariant>& extraValues);
    void reAddingFinished();
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_FILTER_MODEL_P_H
