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

// C Ansi includes.

extern "C"
{
#include <unistd.h>
}

// Qt includes.

#include <qcstring.h>
#include <qstring.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qstringlist.h>

// KDE includes.

#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kinstance.h>
#include <kglobal.h>

// Local includes.

#include "ddebug.h"
#include "undocache.h"

namespace Digikam
{

class UndoCachePriv
{
public:

    QString     cachePrefix;
    QStringList cacheFilenames;
};

UndoCache::UndoCache()
{
    d = new UndoCachePriv;

    QString cacheDir;
    cacheDir = locateLocal("cache", 
                           KGlobal::instance()->aboutData()->programName() + '/');

    d->cachePrefix = QString("%1undocache-%2")
                             .arg(cacheDir)
                             .arg(getpid());
}

UndoCache::~UndoCache()
{
    clear();
    delete d;
}

/**
 * delete all cache files
 */
void UndoCache::clear()
{
    for (QStringList::iterator it = d->cacheFilenames.begin();
         it != d->cacheFilenames.end(); ++it)
    {
        ::unlink(QFile::encodeName(*it));
    }

    d->cacheFilenames.clear();
}

/**
 * write the data into a cache file
 */
bool UndoCache::putData(int level, int w, int h, int bytesDepth, uchar* data)
{
    QString cacheFile = QString("%1-%2.bin")
                        .arg(d->cachePrefix)
                        .arg(level);

    QFile file(cacheFile);
    
    if (file.exists() || !file.open(IO_WriteOnly))
        return false;

    QDataStream ds(&file);
    ds << w;
    ds << h;
    ds << bytesDepth;

    QByteArray ba(w*h*bytesDepth);
    memcpy (ba.data(), data, w*h*bytesDepth);
    ds << ba;

    file.close();

    d->cacheFilenames.append(cacheFile);

    return true;
}

/**
 * get the data from a cache file
 */
uchar* UndoCache::getData(int level, int& w, int& h, int& bytesDepth, bool del)
{
    QString cacheFile = QString("%1-%2.bin")
                        .arg(d->cachePrefix)
                        .arg(level);

    QFile file(cacheFile);
    if (!file.open(IO_ReadOnly))
        return 0;

    QDataStream ds(&file);
    ds >> w;
    ds >> h;
    ds >> bytesDepth;

    uchar *data = new uchar[w*h*bytesDepth];
    if (!data)
        return 0;

    QByteArray ba(w*h*bytesDepth);
    ds >> ba;
    memcpy (data, ba.data(), w*h*bytesDepth);
    
    file.close();

    if(del)
    {
        ::unlink(QFile::encodeName(cacheFile));
        d->cacheFilenames.remove(cacheFile);
    }

    return data;
}

/**
 * delete a cache file
 */
void UndoCache::erase(int level)
{
    QString cacheFile = QString("%1-%2.bin")
                        .arg(d->cachePrefix)
                        .arg(level);

    if(d->cacheFilenames.find(cacheFile) == d->cacheFilenames.end())
        return;
    
    ::unlink(QFile::encodeName(cacheFile));
}

}  // namespace Digikam
