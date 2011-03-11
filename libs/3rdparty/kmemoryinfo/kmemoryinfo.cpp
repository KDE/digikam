/*
   Copyright 2010 Pino Toscano <pino@kde.org>
   Copyright 2011 Marcel Wiesweg <marcel.wiesweg@gmx.de>

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

#include <QtCore/QDateTime>
#include <QtCore/QSharedData>

// KDE includes

#include <kglobal.h>

static bool fillMemoryInfo(Digikam::KMemoryInfo::KMemoryInfoData* data);

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
        valid          = false;
        totalRam       = -1;
        freeRam        = -1;
        usedRam        = -1;
        cacheRam       = -1;
        totalSwap      = -1;
        usedSwap       = -1;
        freeSwap       = -1;
    }

    QDateTime lastUpdate;
    bool   valid;
    qint64 totalRam;
    qint64 freeRam;
    qint64 usedRam;
    qint64 cacheRam;
    qint64 totalSwap;
    qint64 usedSwap;
    qint64 freeSwap;
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

bool KMemoryInfo::isValid() const
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
        if (d->totalRam == -1)
            return -1;
        value += d->totalRam;
    }
    else if (details & AvailableRam)
    {
        if (d->freeRam == -1 || d->cacheRam == -1)
            return -1;
        value += d->freeRam + d->cacheRam;
    }

    if (details & TotalSwap)
    {
        if (d->totalSwap == -1)
            return -1;
        value += d->totalSwap;
    }
    else if (details & AvailableSwap)
    {
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

bool KMemoryInfo::update()
{
    d->reset();
    const bool res = fillMemoryInfo(d);
    d->lastUpdate  = QDateTime::currentDateTime();
    return res;
}

} // namespace Digikam

#include "kmemoryinfo_backend.cpp"
