/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-05
 * Description : undo cache manager for image editor
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005 by Joern Ahrens <joern.ahrens@kdemail.net>
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

class UndoCache::UndoCachePriv
{
public:

    QString     cachePrefix;
    QSet<int>   cachedLevels;

    QString cacheFile(int level) const
    {
        return QString("%1-%2.bin")
                        .arg(cachePrefix)
                        .arg(level);
    }
};

UndoCache::UndoCache()
    : d(new UndoCachePriv)
{
    QString cacheDir = KStandardDirs::locateLocal("cache",
                                                  KGlobal::mainComponent().aboutData()->programName() + '/');

    d->cachePrefix = QString("%1undocache-%2")
                     .arg(cacheDir)
                     .arg(QCoreApplication::applicationPid());

    // remove any remnants
    QDir dir(cacheDir);
    foreach (const QFileInfo& info, dir.entryInfoList(QStringList() << (d->cachePrefix + "*")))
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
    foreach (int level, d->cachedLevels)
    {
        QFile(d->cacheFile(level)).remove();
    }
    d->cachedLevels.clear();
}

void UndoCache::clearFrom(int fromLevel)
{
    foreach (int level, d->cachedLevels)
    {
        if (level >= fromLevel)
        {
            QFile(d->cacheFile(level)).remove();
            d->cachedLevels.remove(level);
        }
    }
}

bool UndoCache::putData(int level, int w, int h, bool sixteenBit, bool hasAlpha, uchar* data)
{
    QFile file(d->cacheFile(level));

    if (file.exists() || !file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QDataStream ds(&file);
    ds << w;
    ds << h;
    ds << sixteenBit;
    ds << hasAlpha;

    QByteArray ba((char*)data, w*h*(sixteenBit ? 8 : 4));
    ds << ba;

    file.close();

    d->cachedLevels << level;

    return true;
}

uchar* UndoCache::getData(int level, int& w, int& h, bool& sixteenBit, bool& hasAlpha) const
{
    w                 = 0;
    h                 = 0;
    sixteenBit        = false;
    hasAlpha          = false;

    QFile file(d->cacheFile(level));

    if (!file.open(QIODevice::ReadOnly))
    {
        return 0;
    }

    QDataStream ds(&file);
    ds >> w;
    ds >> h;
    ds >> sixteenBit;
    ds >> hasAlpha;

    uchar* data = new uchar[w*h*(sixteenBit ? 8 : 4)];

    if (!data)
    {
        return 0;
    }

    QByteArray ba;
    ds >> ba;
    memcpy (data, ba.data(), ba.size());

    file.close();

    return data;
}

}  // namespace Digikam
