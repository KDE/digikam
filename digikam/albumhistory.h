/* ============================================================
 * File  : albumhistory.h
 * Author: Jörn Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2004-11-17
 * Description : 
 * 
 * Copyright 2004 by Jörn Ahrens
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

#ifndef ALBUMHISTORY_H
#define ALBUMHISTORY_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qstringlist.h>

class Album;

class AlbumHistory : public QObject
{
    Q_OBJECT

public:
    AlbumHistory();
    ~AlbumHistory();

    void            addAlbum(Album *album);
    Album*          deleteAlbum(Album *album);
    Album*          back(unsigned int steps=1);
    Album*          forward(unsigned int steps=1);
    Album*          getCurrentAlbum();
    
    void            getBackwardHistory(QStringList &list) const;
    void            getForwardHistory(QStringList &list) const;

    bool            isForwardEmpty();
    bool            isBackwardEmpty();
    
private:
    typedef QValueList<Album*> AlbumStack;
    
    AlbumStack      *m_backwardStack;
    AlbumStack      *m_forwardStack;
    bool            m_moving;
};



#endif /* ALBUMHISTORY_H */
