/* ============================================================
 * File  : albumhistory.cpp
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

#include <kdebug.h>

#include <qstring.h>
#include <qstringlist.h>

#include "album.h"
#include "albumhistory.h"

AlbumHistory::AlbumHistory()
{
    m_backwardStack = new QValueList<Album*>;
    m_forwardStack = new QValueList<Album*>;
    m_moving = false;
}

AlbumHistory::~AlbumHistory()
{
    delete m_backwardStack;
    delete m_forwardStack;
}

void AlbumHistory::addAlbum(Album *album)
{
    if(!album || m_moving)
    {
        m_moving = false;
        return;
    }
    
    if(!m_backwardStack->isEmpty() &&
       m_backwardStack->last() == album)
    {
        return;
    }
    
    m_backwardStack->push_back(album);
    if(!m_forwardStack->isEmpty())
        m_forwardStack->clear();
}

Album* AlbumHistory::deleteAlbum(Album *album)
{
    if(!album || m_backwardStack->isEmpty())
        return 0;
    
    m_backwardStack->remove(album);
    m_forwardStack->remove(album);
    
    if(m_backwardStack->isEmpty() && m_forwardStack->isEmpty())
        return 0;

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
    
    return getCurrentAlbum();   
}

void AlbumHistory::getBackwardHistory(QStringList &list) const
{
    if(m_backwardStack->isEmpty())
        return;
    
    AlbumStack::const_iterator iter = m_backwardStack->begin();
    for(; iter != m_backwardStack->fromLast(); ++iter)
    {
        list.push_front((*iter)->getTitle());
    }
}
        
void AlbumHistory::getForwardHistory(QStringList &list) const
{
    if(m_forwardStack->isEmpty())
        return;
    
    AlbumStack::const_iterator iter;
    for(iter = m_forwardStack->begin(); iter != m_forwardStack->end(); ++iter)
    {
        list.append((*iter)->getTitle());
    }    
}

Album* AlbumHistory::back(unsigned int steps)
{
    if(m_backwardStack->count() <= 1 || steps > m_backwardStack->count())
        return 0; // Only the current album available

    while(steps)
    {
        m_forwardStack->push_front(m_backwardStack->last());
        m_backwardStack->erase(m_backwardStack->fromLast());
        --steps;
    }
    m_moving = true;    
    
    return getCurrentAlbum();
}

Album* AlbumHistory::forward(unsigned int steps)
{
    if(m_forwardStack->isEmpty() || steps > m_forwardStack->count())
        return 0;
    
    while(steps)
    {
        m_backwardStack->push_back(m_forwardStack->first());
        m_forwardStack->erase(m_forwardStack->begin());
        --steps;
    }
    m_moving = true;
   
    return getCurrentAlbum();
}

Album* AlbumHistory::getCurrentAlbum()
{
    if(m_backwardStack->isEmpty())
        return 0;
    
    return m_backwardStack->last();
}  

bool AlbumHistory::isForwardEmpty()
{
    return m_forwardStack->isEmpty();
}

bool AlbumHistory::isBackwardEmpty()
{
    // the last album of the backwardStack is the currently shown
    // album, and therfore not really a previous album
    return (m_backwardStack->count() <= 1) ? true : false;
}

#include "albumhistory.moc"
