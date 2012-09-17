/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-05
 * Description : undo cache manager for image editor
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005      by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "undocache.h"

// Qt includes

#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>

// KDE includes

#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kglobal.h>

namespace Digikam
{

class UndoCache::Private
{
public:

    Private()
    {
    }

    QString cacheFile(int level) const
    {
        return QString("%1-%2.bin").arg(cachePrefix).arg(level);
    }

    QString   cachePrefix;
    QSet<int> cachedLevels;
};

UndoCache::UndoCache()
    : d(new Private)
{
    QString cacheDir = KStandardDirs::locateLocal("cache", KGlobal::mainComponent().aboutData()->programName() + '/');

    d->cachePrefix   = QString("%1undocache-%2")
                       .arg(cacheDir)
                       .arg(QCoreApplication::applicationPid());

    // remove any remnants
    QDir dir(cacheDir);

    foreach(const QFileInfo& info, dir.entryInfoList(QStringList() << (d->cachePrefix + '*')))
    {
        QFile(info.filePath()).remove();
    }
}

UndoCache::~UndoCache()
{
    clear();
    delete d;
}

void UndoCache::clear()
{
    foreach(int level, d->cachedLevels)
    {
        QFile(d->cacheFile(level)).remove();
    }

    d->cachedLevels.clear();
}

void UndoCache::clearFrom(int fromLevel)
{
    foreach(int level, d->cachedLevels)
    {
        if (level >= fromLevel)
        {
            QFile(d->cacheFile(level)).remove();
            d->cachedLevels.remove(level);
        }
    }
}

bool UndoCache::putData(int level, const DImg& img) const
{
    QFile file(d->cacheFile(level));

    if (file.exists() || !file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QDataStream ds(&file);
    ds << img.width();
    ds << img.height();
    ds << img.sixteenBit();
    ds << img.hasAlpha();

    QByteArray ba((const char*)img.bits(), img.numBytes());
    ds << ba;

    file.close();

    d->cachedLevels << level;

    return true;
}

DImg UndoCache::getData(int level) const
{
    int  w          = 0;
    int  h          = 0;
    bool sixteenBit = false;
    bool hasAlpha   = false;

    QFile file(d->cacheFile(level));

    if (!file.open(QIODevice::ReadOnly))
    {
        return DImg();
    }

    QDataStream ds(&file);
    ds >> w;
    ds >> h;
    ds >> sixteenBit;
    ds >> hasAlpha;

    QByteArray ba;
    ds >> ba;

    DImg img(w, h, sixteenBit, hasAlpha, (uchar*)ba.data(), true);

    file.close();

    return img;
}

}  // namespace Digikam
