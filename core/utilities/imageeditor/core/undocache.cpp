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
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimgloader.h"

namespace Digikam
{

class UndoCache::Private
{
public:

    explicit Private()
    {
        cacheError = false;
    }

    QString cacheFile(int level) const
    {
        return QString::fromUtf8("%1-%2.bin").arg(cachePrefix).arg(level);
    }

    QString   cacheDir;
    QString   cachePrefix;
    QSet<int> cachedLevels;

    bool      cacheError;
};

UndoCache::UndoCache()
    : d(new Private)
{
    d->cacheDir    = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1Char('/');

    d->cachePrefix = QString::fromUtf8("%1undocache-%2")
                     .arg(d->cacheDir)
                     .arg(QCoreApplication::applicationPid());

    // remove any remnants
    QDir dir(d->cacheDir);

    foreach(const QFileInfo& info, dir.entryInfoList(QStringList() << QLatin1String("undocache-*")))
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
    if (d->cacheError)
    {
        return false;
    }

    QStorageInfo info(d->cacheDir);

    qint64 fspace = (info.bytesAvailable() / 1024 / 1024);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Free space available in Editor cache [" << d->cacheDir << "] in Mbytes:" << fspace;

    if (fspace < 2048) // Check if free space is over 2 GiB to put data in cache.
    {
        if (!qApp->activeWindow()) // Special case for the Jenkins build server.
        {
            return false;
        }

        QApplication::restoreOverrideCursor();

        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("The free disk space in the path \"%1\" for the undo "
                                   "cache file is < 2 GiB! Undo cache is now disabled!",
                                   QDir::toNativeSeparators(d->cacheDir)));
        d->cacheError = true;

        return false;
    }

    QFile file(d->cacheFile(level));

    if (file.exists() || !file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QDataStream ds(&file);
    ds << img.width();
    ds << img.height();
    ds << img.numBytes();
    ds << img.hasAlpha();
    ds << img.sixteenBit();

    file.write((const char*)img.bits(), img.numBytes());

    if (file.error() != QFileDevice::NoError)
    {
        file.close();
        file.remove();
        return false;
    }

    d->cachedLevels << level;

    file.close();
    return true;
}

DImg UndoCache::getData(int level) const
{
    uint w          = 0;
    uint h          = 0;
    uint numBytes   = 0;
    bool hasAlpha   = false;
    bool sixteenBit = false;

    QFile file(d->cacheFile(level));

    if (!file.open(QIODevice::ReadOnly))
    {
        return DImg();
    }

    QDataStream ds(&file);
    ds >> w;
    ds >> h;
    ds >> numBytes;
    ds >> hasAlpha;
    ds >> sixteenBit;

    qint64 size  = w * h * (sixteenBit ? 8 : 4);

    if (ds.status() != QDataStream::Ok ||
        ds.atEnd() || numBytes != size || size == 0)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "The undo cache file is corrupt";

        file.close();
        return DImg();
    }

    char* data = (char*)DImgLoader::new_failureTolerant(size);

    if (!data)
    {
        file.close();
        return DImg();
    }

    qint64 readSize = file.read(data, size);

    if (file.error() != QFileDevice::NoError || readSize != size)
    {
        delete [] data;

        file.close();
        return DImg();
    }

    DImg img(w, h, sixteenBit, hasAlpha, (uchar*)data, false);

    file.close();
    return img;
}

} // namespace Digikam
