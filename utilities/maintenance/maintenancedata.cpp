/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-02-20
 * Description : Synchronized container for maintenance data.
 *
 * Copyright (C) 2017-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
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

#include "maintenancedata.h"

// Qt includes

#include <QMutex>

// Local includes

#include "identity.h"

namespace Digikam
{

class MaintenanceData::Private
{
public:

    Private()
    {
    }

    QList<qlonglong>              imageIdList;
    QList<int>                    thumbnailIdList;
    QList<QString>                imagePathList;
    QList<ImageInfo>              imageInfoList;
    QList<Identity>               identitiesList;
    QList<qlonglong>              similarityImageIdList;

    QMutex                        lock;
};

MaintenanceData::MaintenanceData()
    : d(new Private)
{
}

MaintenanceData::~MaintenanceData()
{
    delete d;
}

void MaintenanceData::setImageIds(const QList<qlonglong>& ids)
{
    d->imageIdList = ids;
}

void MaintenanceData::setThumbnailIds(const QList<int>& ids)
{
    d->thumbnailIdList = ids;
}

void MaintenanceData::setImagePaths(const QList<QString>& paths)
{
    d->imagePathList = paths;
}

void MaintenanceData::setImageInfos(const QList<ImageInfo>& infos)
{
    d->imageInfoList = infos;
}

void MaintenanceData::setSimilarityImageIds(const QList<qlonglong>& ids)
{
    d->similarityImageIdList = ids;
}

void MaintenanceData::setIdentities(const QList<Identity>& identities)
{
    d->identitiesList = identities;
}

qlonglong MaintenanceData::getImageId() const
{
    d->lock.lock();
    qlonglong id = -1;

    if (!d->imageIdList.isEmpty())
    {
        id = d->imageIdList.takeFirst();
    }

    d->lock.unlock();
    return id;
}

int MaintenanceData::getThumbnailId() const
{
    d->lock.lock();
    int id = -1;

    if (!d->thumbnailIdList.isEmpty())
    {
        id = d->thumbnailIdList.takeFirst();
    }

    d->lock.unlock();
    return id;
}

QString MaintenanceData::getImagePath() const
{
    d->lock.lock();
    QString path;

    if (!d->imagePathList.isEmpty())
    {
        path = d->imagePathList.takeFirst();
    }

    d->lock.unlock();
    return path;
}

ImageInfo MaintenanceData::getImageInfo() const
{
    d->lock.lock();
    ImageInfo info;

    if (!d->imageInfoList.isEmpty())
    {
        info = d->imageInfoList.takeFirst();
    }

    d->lock.unlock();
    return info;
}

Identity MaintenanceData::getIdentity() const
{
    d->lock.lock();
    Identity identity;

    if (!d->identitiesList.isEmpty())
    {
        identity = d->identitiesList.takeFirst();
    }

    d->lock.unlock();
    return identity;
}

qlonglong MaintenanceData::getSimilarityImageId() const
{
    d->lock.lock();
    qlonglong id = -1;

    if (!d->similarityImageIdList.isEmpty())
    {
        id = d->similarityImageIdList.takeFirst();
    }

    d->lock.unlock();
    return id;
}

} // namespace Digikam
