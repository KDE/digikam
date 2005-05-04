/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-18
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

#ifndef GPITEMINFO_H
#define GPITEMINFO_H

#include <qvaluelist.h>
#include <qcstring.h>

extern "C"
{
#include <time.h>
}

class GPItemInfo
{
public:
    
    QString name;
    QString folder;
    time_t  mtime;
    QString mime;
    long    size;
    int     width;
    int     height;
    int     downloaded;
    int     readPermissions;
    int     writePermissions;
};

class QDataStream;
QDataStream& operator<<( QDataStream &, const GPItemInfo & );
QDataStream& operator>>( QDataStream &, GPItemInfo & );

typedef QValueList<GPItemInfo> GPItemInfoList;

#endif /* GPITEMINFO_H */
