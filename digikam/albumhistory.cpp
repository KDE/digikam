/* ============================================================
 * File  : albumhistory.cpp
 * Author: Joern Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2004-11-17
 * Description : 
 * 
 * Copyright 2004 by Joern Ahrens
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

// Qt includes.

#include <qstring.h>
#include <qstringlist.h>
#include <qwidget.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albumhistory.h"

namespace Digikam
{

/**
 * Stores an album along with the sidebar view, where the album
 * is selected
 */
class HistoryItem
{
public:
    
    HistoryItem()
    {
        album = 0;
        widget = 0;
    };
    
    HistoryItem(Album *a, QWidget *w)
    {
        album = a;
        widget = w;
    };
    
    bool operator==(const HistoryItem& item)
    {
        return (album == item.album) && (widget == item.widget);
    }
    
    Album   *album;
    QWidget *widget;
};

AlbumHistory::AlbumHistory()
{
    m_backwardStack = new AlbumStack;
    m_forwardStack = new AlbumStack;
    m_moving = false;
}

AlbumHistory::~AlbumHistory()
{
    clearHistory();
    
    delete m_backwardStack;
    delete m_forwardStack;
}

void AlbumHistory::clearHistory()
{
    AlbumStack::iterator iter = m_backwardStack->begin();
    AlbumStack::iterator end = m_backwardStack->end();
    for(; iter != end; ++iter)
        delete *iter;
    m_backwardStack->clear();

    iter = m_forwardStack->begin();
    end = m_forwardStack->end();
    for(; iter != end; ++iter)
        delete *iter;
    m_forwardStack->clear();

    m_moving = false;
}

void AlbumHistory::addAlbum(Album *album, QWidget *widget)
{
    if(!album || !widget || m_moving)
    {
        m_moving = false;
        return;
    }
    
    HistoryItem *item = new HistoryItem(album, widget);
    
    // Same album as before in the history
    if(!m_backwardStack->isEmpty() &&
       *m_backwardStack->last() == *item)
    {
        delete item;
        return;
    }
    
    m_backwardStack->push_back(item);

    // The forward stack has to be cleared, if backward stack was changed
    if(!m_forwardStack->isEmpty())
    {
        AlbumStack::iterator iter = m_forwardStack->begin();
        for(; iter != m_forwardStack->end(); ++iter)
        {
            delete *iter;
        }
        m_forwardStack->clear();
    }
}

void AlbumHistory::deleteAlbum(Album *album)
{
    if(!album || m_backwardStack->isEmpty())
        return;
    
    //  Search all HistoryItems, with album and delete them
    AlbumStack::iterator iter = m_backwardStack->begin();
    AlbumStack::iterator end = m_backwardStack->end();
    while(iter != end)
    {
        if((*iter)->album == album)
        {
            delete *iter;
            iter = m_backwardStack->remove(iter);            
        }
        else
        {
            ++iter;
        }
    }
    iter = m_forwardStack->begin();
    end = m_forwardStack->end();
    while(iter != end)
    {
        if((*iter)->album == album)
        {
            delete *iter;
            iter = m_forwardStack->remove(iter);
        }
        else
        {
            ++iter;
        }
    }
    
    if(m_backwardStack->isEmpty() && m_forwardStack->isEmpty())
        return;

    // If backwardStack is empty, then there is no current album.
    // So make the first album of the forwardStack the current one.
    if(m_backwardStack->isEmpty())
        forward();

    // After the album is deleted from the history it has to be ensured, 
    // that neigboring albums are different
    AlbumStack::iterator lhs = m_backwardStack->begin();
    AlbumStack::iterator rhs = lhs; 
    ++rhs;
    while(rhs != m_backwardStack->end())
    {
        if(*lhs == *rhs)
        {
            rhs = m_backwardStack->remove(rhs);
        }
        else
        {
            ++lhs;
            rhs = lhs;
            ++rhs;
        }
    }
    
    rhs = m_forwardStack->begin();
    while(rhs != m_forwardStack->end())
    {
        if(*lhs == *rhs)
        {
            rhs = m_forwardStack->remove(rhs);
        }
        else
        {
            if(lhs == m_backwardStack->fromLast())
            {
                lhs = m_forwardStack->begin();
            }
            else
            {
                ++lhs;
                rhs = lhs;
            }
            ++rhs;
        }                
    }
    
    if(m_backwardStack->isEmpty() && !m_forwardStack->isEmpty())
        forward();
}

void AlbumHistory::getBackwardHistory(QStringList &list) const
{
    if(m_backwardStack->isEmpty())
        return;
    
    AlbumStack::const_iterator iter = m_backwardStack->begin();
    for(; iter != m_backwardStack->fromLast(); ++iter)
    {
        list.push_front((*iter)->album->title());
    }
}
        
void AlbumHistory::getForwardHistory(QStringList &list) const
{
    if(m_forwardStack->isEmpty())
        return;
    
    AlbumStack::const_iterator iter;
    for(iter = m_forwardStack->begin(); iter != m_forwardStack->end(); ++iter)
    {
        list.append((*iter)->album->title());
    }    
}

void AlbumHistory::back(Album **album, QWidget **widget, unsigned int steps)
{
    *album = 0;
    *widget = 0;
    
    if(m_backwardStack->count() <= 1 || steps > m_backwardStack->count())
        return; // Only the current album available

    while(steps)
    {
        m_forwardStack->push_front(m_backwardStack->last());
        m_backwardStack->erase(m_backwardStack->fromLast());
        --steps;
    }
    m_moving = true;    
    
    HistoryItem *item = getCurrentAlbum();
    if(item)
    {
        *album = item->album;
        *widget = item->widget;
    }
}

void AlbumHistory::forward(Album **album, QWidget **widget, unsigned int steps)
{
    *album = 0;
    *widget = 0;

    if(m_forwardStack->isEmpty() || steps > m_forwardStack->count())
        return;
    
    forward(steps);
    
    HistoryItem *item = getCurrentAlbum();
    if(item)
    {
        *album = item->album;
        *widget = item->widget;
    }
}

void AlbumHistory::forward(unsigned int steps)
{
    if(m_forwardStack->isEmpty() || steps > m_forwardStack->count())
        return;
    
    while(steps)
    {
        m_backwardStack->push_back(m_forwardStack->first());
        m_forwardStack->erase(m_forwardStack->begin());
        --steps;
    }
    m_moving = true;
}

HistoryItem* AlbumHistory::getCurrentAlbum() const
{
    if(m_backwardStack->isEmpty())
        return 0;
    
    return m_backwardStack->last();
}  

void AlbumHistory::getCurrentAlbum(Album **album, QWidget **widget) const
{
    *album = 0;
    *widget = 0;
    
    if(m_backwardStack->isEmpty())
        return;
    
    HistoryItem *item = m_backwardStack->last();
    if(item)
    {
        *album = item->album;
        *widget = item->widget;
    }
}

bool AlbumHistory::isForwardEmpty() const
{
    return m_forwardStack->isEmpty();
}

bool AlbumHistory::isBackwardEmpty() const
{
    // the last album of the backwardStack is the currently shown
    // album, and therfore not really a previous album
    return (m_backwardStack->count() <= 1) ? true : false;
}

}  // namespace Digikam

#include "albumhistory.moc"
