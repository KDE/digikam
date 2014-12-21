/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-11
 * Description : Qt item model for database entries - private shared header
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

#ifndef IMAGEFILTERMODELPRIV_H
#define IMAGEFILTERMODELPRIV_H

// Qt includes

#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>

// Local includes

#include "imageinfo.h"
#include "imagefiltermodel.h"

#include "digikam_export.h"
// Yes, we need the EXPORT macro in a private header because
// this private header is shared across binary objects.
// This does NOT make this classes here any more public!

namespace Digikam
{

const int PrepareChunkSize = 101;
const int FilterChunkSize  = 2001;

class ImageFilterModelTodoPackage
{
public:

    ImageFilterModelTodoPackage()
        : version(0), isForReAdd(false)
    {
    }

    ImageFilterModelTodoPackage(const QVector<ImageInfo>& infos, const QVector<QVariant>& extraValues, int version, bool isForReAdd)
        : infos(infos), extraValues(extraValues), version(version), isForReAdd(isForReAdd)
    {
    }

    QVector<ImageInfo>     infos;
    QVector<QVariant>      extraValues;
    unsigned int           version;
    bool                   isForReAdd;
    QHash<qlonglong, bool> filterResults;
};

// ------------------------------------------------------------------------------------------------

class ImageFilterModelPreparer;
class ImageFilterModelFilterer;

class DIGIKAM_DATABASE_EXPORT ImageFilterModel::ImageFilterModelPrivate : public QObject
{
    Q_OBJECT

public:

    ImageFilterModelPrivate();
    ~ImageFilterModelPrivate();

    void init(ImageFilterModel* q);
    void setupWorkers();
    void infosToProcess(const QList<ImageInfo>& infos);
    void infosToProcess(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues, bool forReAdd = true);

public:

    ImageFilterModel*                   q;

    ImageModel*                         imageModel;

    ImageFilterSettings                 filter;
    ImageSortSettings                   sorter;
    VersionImageFilterSettings          versionFilter;
    GroupImageFilterSettings            groupFilter;

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
    ImageFilterSettings                 filterCopy;
    VersionImageFilterSettings          versionFilterCopy;
    GroupImageFilterSettings            groupFilterCopy;
    ImageFilterModelPreparer*           preparer;
    ImageFilterModelFilterer*           filterer;

    QHash<qlonglong, bool>              filterResults;
    bool                                hasOneMatch;
    bool                                hasOneMatchForText;

    QList<ImageFilterModelPrepareHook*> prepareHooks;

/*
    QHash<int, QSet<qlonglong> >        categoryCountHashInt;
    QHash<QString, QSet<qlonglong> >    categoryCountHashString;

public:

        void cacheCategoryCount(int id, qlonglong imageid) const
        { const_cast<ImageFilterModelPrivate*>(this)->categoryCountHashInt[id].insert(imageid); }
        void cacheCategoryCount(const QString& id, qlonglong imageid) const
        { const_cast<ImageFilterModelPrivate*>(this)->categoryCountHashString[id].insert(imageid); }
*/

public Q_SLOTS:

    void preprocessInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues);
    void processAddedInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues);
    void packageFinished(const ImageFilterModelTodoPackage& package);
    void packageDiscarded(const ImageFilterModelTodoPackage& package);

Q_SIGNALS:

    void packageToPrepare(const ImageFilterModelTodoPackage& package);
    void packageToFilter(const ImageFilterModelTodoPackage& package);
    void reAddImageInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues);
    void reAddingFinished();
};

} // namespace Digikam

#endif // IMAGEFILTERMODELPRIV_H
