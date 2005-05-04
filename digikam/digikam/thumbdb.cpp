/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-21
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qdir.h>
#include <qfile.h>
#include <qcstring.h>
#include <qdatastream.h>

#include <kmdcodec.h>
#include <kdebug.h>

extern "C"
{
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>	
#include <gdbm.h>
}

#include "thumbdb.h"

void digikam_gdbm_fatal_func(char* val)
{
    kdWarning() << "GDBM fatal error occured: "
                << val << endl;
}

class ThumbDBPriv
{
public:

    GDBM_FILE db;
};


ThumbDB* ThumbDB::instance()
{
    if (!m_instance)
        return new ThumbDB();
    return m_instance;
}

ThumbDB* ThumbDB::m_instance = 0;

ThumbDB::ThumbDB()
{
    m_instance = this;

    d = new ThumbDBPriv;
    
    QString dbPath(QDir::homeDirPath() +
                   "/.thumbnails/digikam-thumbnails.db");
    QCString encPath = QFile::encodeName(dbPath);
    
    const char* path = encPath; 
    d->db = gdbm_open((char*)path, 0, GDBM_WRCREAT|GDBM_FAST,
                      0666, (void (*)()) digikam_gdbm_fatal_func);
     if (!d->db)
         kdWarning() << "Failed to open Thumbnail DB file: " << dbPath << endl;
}

ThumbDB::~ThumbDB()
{
    m_instance = 0;
    if (d->db)
        gdbm_close(d->db);
}

void ThumbDB::putThumb(const QString& path, const QImage& image)
{
    if (!d->db)
        return;
    
    QImage thumb(image.scale(48,48,QImage::ScaleMin));
    
    QCString keyStr(getKey(path));

    datum key, content;

    memset(&key, 0, sizeof(key));
    key.dsize = keyStr.length();
    key.dptr  = const_cast<char*>((const char*)keyStr);

    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << thumb;

    memset(&content, 0, sizeof(content));
    content.dsize = ba.size();
    content.dptr  = ba.data();

    gdbm_store(d->db, key, content, GDBM_REPLACE);

}

void ThumbDB::getThumb(const QString& path, QPixmap& pix, int w, int h)
{
    if (!d->db)
        return;

    QCString keyStr(getKey(path));    

    datum key, content;

    memset(&key, 0, sizeof(key));
    key.dsize = keyStr.length();
    key.dptr  = (char*) ((const char*)keyStr);

    memset(&content, 0, sizeof(content));
    content = gdbm_fetch(d->db, key);

    if (!content.dptr)
    {
        return;
    }

    QByteArray  ba;
    ba.setRawData( content.dptr, content.dsize );
    QDataStream ds( ba, IO_ReadOnly );
    QImage thumb;
    ds >> thumb;
    ba.resetRawData( content.dptr, content.dsize );
    
    free(content.dptr);

    if (thumb.isNull())
    {
        gdbm_delete(d->db, key);
        return;
    }
    
    thumb = thumb.scale(w,h,QImage::ScaleMin);
    pix = QPixmap(thumb);

    w = pix.width();
    h = pix.height();
    if (w >= 10 && h >= 10)
    {
        QPainter p(&pix);
        p.setPen(QPen(QColor(0,0,0),1));
        p.drawRect(0,0,w,h);
        p.setPen(QPen(QColor(255,255,255),1));
        p.drawRect(1,1,w-2,h-2);
        p.end();
    }
}

bool ThumbDB::hasThumb(const QString& path)
{
    if (!d->db)
        return false;
    
    QCString keyStr(getKey(path));    

    datum key;

    memset(&key, 0, sizeof(key));
    key.dsize = keyStr.length();
    key.dptr  = (char*) ((const char*)keyStr);

    if (gdbm_exists(d->db, key))
    {
        return true;
    }
    
    return false;
}

QCString ThumbDB::getKey(const QString& path)
{
    QString uri = "file://" + QDir::cleanDirPath(path);
    
    KMD5 md5( QFile::encodeName( uri ) );
    return md5.hexDigest();
}

