/* ============================================================
 * File  : albumhistory.h
 * Author: Joern Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2004-11-17
 * Description : 
 * 
 * Copyright 2004 by Joern Ahrens <joern.ahrens@kdemail.net>
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

/** @file albumhistory.h */

// Qt includes.

#include <qobject.h>
#include <qvaluelist.h>
#include <qstringlist.h>

namespace Digikam
{

class Album;
class HistoryItem;

/**
 * Manages the history of the last visited albums.
 *
 * The user is able to navigate through the albums, he has
 * opened during a session.
 */
class AlbumHistory : public QObject
{
    Q_OBJECT

public:

    AlbumHistory();
    ~AlbumHistory();

    void            addAlbum(Album *album, QWidget *widget = 0);
    void            deleteAlbum(Album *album);
    void            clearHistory();
    void            back(Album **album, QWidget **widget, unsigned int steps=1);
    void            forward(Album **album, QWidget **widget, unsigned int steps=1);
    void            getCurrentAlbum(Album **album, QWidget **widget) const;
    
    void            getBackwardHistory(QStringList &list) const;
    void            getForwardHistory(QStringList &list) const;

    bool            isForwardEmpty() const;
    bool            isBackwardEmpty() const;
    
private:
    
    HistoryItem*    getCurrentAlbum() const;
    void            forward(unsigned int steps=1);
    
    typedef QValueList<HistoryItem*> AlbumStack;
    
    AlbumStack      *m_backwardStack;
    AlbumStack      *m_forwardStack;
    bool             m_moving;
};

}  // namespace Digikam

#endif /* ALBUMHISTORY_H */
