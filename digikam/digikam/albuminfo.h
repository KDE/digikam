/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-21
 * Copyright 2005 by Renchi Raju
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
 * ============================================================ */

#ifndef ALBUMINFO_H
#define ALBUMINFO_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qdatetime.h>

/*
 * A container class for transporting info from AlbumDB
 * to AlbumManager
 */
class AlbumInfo
{
public:

    typedef QValueList<AlbumInfo> List;
    
    enum Type
    {
        PHYSICAL=0,
        TAG,
        DATE,
        SEARCH
    };

    // common for all album types
    Type     type;
    int      id;
    QString  icon;

    // only for TAG albums
    int      pid;
    QString  name;

    // only for PHYSICAL albums
    QString  url;
    QString  caption;
    QString  collection;
    QDate    date;

    AlbumInfo() {}
        
    
    AlbumInfo(const AlbumInfo& info)
    {
        type       = info.type;
        id         = info.id;
        icon       = info.icon;
                   
        pid        = info.pid;
        name       = info.name;

        url        = info.url;
        caption    = info.caption;
        collection = info.collection;
        date       = info.date;
    }

    AlbumInfo& operator=(const AlbumInfo& info)
    {
        if (this != &info)
        {
            type       = info.type;
            id         = info.id;
            icon       = info.icon;
                       
            pid        = info.pid;
            name       = info.name;

            url        = info.url;
            caption    = info.caption;
            collection = info.collection;
            date       = info.date;
        }
        return *this;
    }
    
    // need for sorting
    bool operator<(const AlbumInfo& info)
    {
        switch (type)
        {
        case(PHYSICAL):
            return url < info.url;
        case(TAG):
            return id  < info.id;
        default:
            break;
        }
        
        return true;
    }
};

#endif /* ALBUMINFO_H */
