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

#include <qcstring.h>
#include <qstring.h>
#include <qfile.h>

extern "C"
{
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <gdbm.h>
}

#include "undocache.h"

class UndoCachePriv
{
public:

    GDBM_FILE db;
    QString   dbPath;
};

void UndoCache_gdbm_fatal_func(char* val)
{
    kdWarning() << "UndoCache: GDBM fatal error occured: "
                << val << endl;    
}

UndoCache::UndoCache()
{
    d = new UndoCachePriv;

    d->dbPath = locateLocal("cache", "digikam_imageviewer_undo.db");
    QCString encPath = QFile::encodeName(d->dbPath);

    const char* path = encPath; 
    d->db = gdbm_open((char*)path, 0, GDBM_WRCREAT|GDBM_SYNC,
                        0666, (void (*)()) UndoCache_gdbm_fatal_func);
    if (!d->db)
    {
        kdWarning() << "Failed to open ImageViewer Undo DB file: "
                    << d->dbPath << endl;
    }
}

UndoCache::~UndoCache()
{
    if (d->db)
    {
        gdbm_close(d->db);
        unlink(QFile::encodeName(d->dbPath));
    }
    delete d;
}

void UndoCache::clear()
{
    if (d->db)
    {
        datum key, nextkey;

        key = gdbm_firstkey(d->db);
        while (key.dptr)
        {
            nextkey = gdbm_nextkey(d->db, key);
            gdbm_delete(d->db, key);
            free(key.dptr);
            key = nextkey;
        }
    }
}

bool UndoCache::pushLevel(int level, int w, int h, uint* data)
{
    if (d->db)
    {
        datum key, content;

        QString levelName;
        levelName.setNum(level);
        
        key.dsize = 1;
        key.dptr  = (char*)levelName.ascii();

        char* buf = new char[w*h*sizeof(uint) + 2*sizeof(int)];
        char* p = buf;

        memcpy(p, &w, sizeof(int));
        p += sizeof(int);
        memcpy(p, &h, sizeof(int));
        p += sizeof(int);
        memcpy(p, data, w*h*sizeof(uint));

        content.dsize = w*h*sizeof(uint) + 2*sizeof(int);
        content.dptr  = buf;
        
        gdbm_store(d->db, key, content, GDBM_REPLACE);

        delete [] buf;
        
        return true;
    }
    else
        return false;
}

bool UndoCache::popLevel(int level, int& w, int& h, uint*& data)
{
    if (d->db)
    {
        datum key, content;

        QString levelName;
        levelName.setNum(level);

        key.dsize = 1;
        key.dptr  = (char*)levelName.ascii();

        content = gdbm_fetch(d->db, key);
        if (!content.dptr)
            return false;

        char* p = content.dptr;
        memcpy(&w, p, sizeof(int));
        p += sizeof(int);
        memcpy(&h, p, sizeof(int));
        p += sizeof(int);

        data = new uint[w*h];
        memcpy(data, p, w*h*sizeof(uint));

        gdbm_delete(d->db, key);
        free(content.dptr);

        return true;
    }
    else
        return false;
}
