/* ============================================================
 * File  : gpfileiteminfo.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-22
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef GPFILEITEMINFO_H
#define GPFILEITEMINFO_H

#include <qstring.h>
#include <qvaluelist.h>

class GPFileItemInfo
{
    
public:

    GPFileItemInfo();
    ~GPFileItemInfo();

    GPFileItemInfo(const GPFileItemInfo& info);
    GPFileItemInfo& operator=(const GPFileItemInfo& info);
    

    // ---------------------------------------------------------
    
    QString name;
    QString folder;

    // ---------------------------------------------------------

    bool    fileInfoAvailable;

    QString mime;
    QString time;
    int     size;
    int     width;
    int     height;
    int     readPermissions;
    int     writePermissions;
    int     downloaded;

    // ---------------------------------------------------------

    bool    previewInfoAvailable;

    QString previewMime;
    int     previewSize;
    int     previewWidth;
    int     previewHeight;
    int     previewDownloaded;

    // ---------------------------------------------------------

    bool    audioInfoAvailable;

    QString audioMime;
    int     audioSize;
    int     audioDownloaded;

    // ---------------------------------------------------------

    void   *viewItem;
    
};

// Container for GPFileItemInfo
typedef QValueList<GPFileItemInfo> GPFileItemInfoList;


#endif 
