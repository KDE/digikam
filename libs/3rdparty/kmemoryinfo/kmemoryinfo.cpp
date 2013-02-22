/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-14
 * Description : Memory info wrapper class
 *
 * Copyright (C) 2010 by Pino Toscano <pino at kde dot org>
 * Copyright (C) 2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "kmemoryinfo.h"

// Qt includes

#include <QtCore/QDateTime>
#include <QtCore/QSharedData>

// KDE includes

#include <kglobal.h>
#include <kdebug.h>

static int fillMemoryInfo(Digikam::KMemoryInfo::KMemoryInfoData* const data);

namespace Digikam
{

class KMemoryInfo::KMemoryInfoData : public QSharedData
{
public:

    KMemoryInfoData()
    {
        reset();
    }

    void reset()
    {
        valid     = -1;
        totalRam  = -1;
        freeRam   = -1;
        usedRam   = -1;
        cacheRam  = -1;
        totalSwap = -1;
        usedSwap  = -1;
        freeSwap  = -1;
        platform  = QString("Unknown");
    }

    QDateTime lastUpdate;
    int       valid;
    qint64    totalRam;
    qint64    freeRam;
    qint64    usedRam;
    qint64    cacheRam;
    qint64    totalSwap;
    qint64    usedSwap;
    qint64    freeSwap;
    QString   platform;
};

// ------------------------------------------------------------------------------------------

class KMemoryInfoDataSharedNull : public QSharedDataPointer<KMemoryInfo::KMemoryInfoData>
{
public:

    KMemoryInfoDataSharedNull()
        : QSharedDataPointer<KMemoryInfo::KMemoryInfoData>(new KMemoryInfo::KMemoryInfoData)
    {
    }
};

K_GLOBAL_STATIC(KMemoryInfoDataSharedNull, kmemoryInfoDataSharedNull)

// ------------------------------------------------------------------------------------------

KMemoryInfo::KMemoryInfo()
    : d(*kmemoryInfoDataSharedNull)
{
}

KMemoryInfo::KMemoryInfo(const KMemoryInfo& other)
    : d(other.d)
{
}

KMemoryInfo::~KMemoryInfo()
{
}

KMemoryInfo& KMemoryInfo::operator=(const KMemoryInfo& other)
{
    d = other.d;
    return *this;
}

int KMemoryInfo::isValid() const
{
    return d->valid;
}

KMemoryInfo KMemoryInfo::currentInfo()
{
    KMemoryInfo info;
    info.update();
    return info;
}

qint64 KMemoryInfo::bytes(KMemoryInfo::MemoryDetails details) const
{
    qint64 value = 0;

    if (details & TotalRam)
    {
        kDebug() << "TotalRam: " << d->totalRam;

        if (d->totalRam == -1)
            return -1;

        value += d->totalRam;
    }
    else if (details & AvailableRam)
    {
        kDebug() << "AvailableRam: " << d->freeRam << " (cache: " << d->cacheRam << ")";

        if (d->freeRam == -1 || d->cacheRam == -1)
            return -1;

        value += d->freeRam + d->cacheRam;
    }

    if (details & TotalSwap)
    {
        kDebug() << "TotalSwap: " << d->totalSwap;

        if (d->totalSwap == -1)
            return -1;

        value += d->totalSwap;
    }
    else if (details & AvailableSwap)
    {
        kDebug() << "AvailableSwap: " << d->freeSwap;

        if (d->freeSwap == -1)
            return -1;

        value += d->freeSwap;
    }

    return value;
}

double KMemoryInfo::kilobytes(MemoryDetails detail) const
{
    qint64 b = bytes(detail);

    if (b == -1)
        return -1;

    return double(b) / 1024.0;
}

double KMemoryInfo::megabytes(MemoryDetails detail) const
{
    qint64 b = bytes(detail);

    if (b == -1)
        return -1;

    return double(b) / 1024.0 / 1024.0;
}

QDateTime KMemoryInfo::lastUpdate() const
{
    return d->lastUpdate;
}

int KMemoryInfo::update()
{
    d->reset();
    const int res = fillMemoryInfo(d);
    kDebug() << "Platform identified : " << d->platform;
    d->lastUpdate = QDateTime::currentDateTime();
    return res;
}

} // namespace Digikam

#include "kmemoryinfo_backend.cpp"
