/* ============================================================
 * File  : undocache.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-02-05
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kinstance.h>
#include <kglobal.h>

#include <qcstring.h>
#include <qstring.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qstringlist.h>

extern "C"
{
#include <unistd.h>
}

#include "undocache.h"

class UndoCachePriv
{
public:

    QString     cachePrefix;
    QStringList cacheFilenames;
};

UndoCache::UndoCache()
{
    d = new UndoCachePriv;

    QString tmp = QString("%1-%2-undocache")
                  .arg(KGlobal::instance()->aboutData()->programName())
                  .arg(getpid());
    
    d->cachePrefix = locateLocal("cache", tmp);
}

UndoCache::~UndoCache()
{
    clear();
    delete d;
}

void UndoCache::clear()
{
    for (QStringList::iterator it = d->cacheFilenames.begin();
         it != d->cacheFilenames.end(); ++it)
    {
        ::unlink(QFile::encodeName(*it));
    }

    d->cacheFilenames.clear();
}

bool UndoCache::pushLevel(int level, int w, int h, uint* data)
{
    QString cacheFile = QString("%1-%2.bin")
                        .arg(d->cachePrefix)
                        .arg(level);

    QFile file(cacheFile);
    
    if (!file.open(IO_WriteOnly))
        return false;

    QDataStream ds(&file);
    ds << w;
    ds << h;

    QByteArray ba;
    ba.setRawData((const char*)data, w*h*sizeof(uint));
    ds << ba;
    ba.resetRawData((const char*)data, w*h*sizeof(uint));

    file.close();
    return true;
}

bool UndoCache::popLevel(int level, int& w, int& h, uint*& data)
{
    QString cacheFile = QString("%1-%2.bin")
                        .arg(d->cachePrefix)
                        .arg(level);

    QFile file(cacheFile);
    if (!file.open(IO_ReadOnly))
        return false;

    QDataStream ds(&file);
    ds >> w;
    ds >> h;

    data = new uint[w*h];

    QByteArray ba;
    ba.setRawData((const char*)data, w*h*sizeof(uint));
    ds >> ba;
    ba.resetRawData((const char*)data, w*h*sizeof(uint));

    file.close();

    ::unlink(QFile::encodeName(cacheFile));
    return true;
}
