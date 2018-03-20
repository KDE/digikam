/*
   Copyright 2010 Pino Toscano <pino at kde dot org>
   Copyright 2011 Marcel Wiesweg <marcel dot wiesweg at gmx dot de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kmemoryinfo.h"

// Qt includes

#include <QDateTime>
#include <QSharedData>
#include <QDebug>

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
        platform  = QLatin1String("Unknown");
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

Q_GLOBAL_STATIC(KMemoryInfoDataSharedNull, kmemoryInfoDataSharedNull)

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
        qDebug() << "KMemoryInfo: TotalRam: " << d->totalRam;

        if (d->totalRam == -1)
            return -1;

        value += d->totalRam;
    }
    else if (details & AvailableRam)
    {
        qDebug() << "KMemoryInfo: AvailableRam: " << d->freeRam << " (cache: " << d->cacheRam << ")";

        if (d->freeRam == -1 || d->cacheRam == -1)
            return -1;

        value += d->freeRam + d->cacheRam;
    }

    if (details & TotalSwap)
    {
        qDebug() << "KMemoryInfo: TotalSwap: " << d->totalSwap;

        if (d->totalSwap == -1)
            return -1;

        value += d->totalSwap;
    }
    else if (details & AvailableSwap)
    {
        qDebug() << "KMemoryInfo: AvailableSwap: " << d->freeSwap;

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
    qDebug() << "KMemoryInfo: Platform identified : " << d->platform;
    d->lastUpdate = QDateTime::currentDateTime();
    return res;
}

} // namespace Digikam

#include "kmemoryinfo_backend.cpp"
