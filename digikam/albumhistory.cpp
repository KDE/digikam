/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : albums history manager.
 *
 * Copyright (C) 2004 by Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumhistory.h"
#include "albumhistory.moc"

// Qt includes

#include <QString>
#include <QWidget>

// Local includes

#include "album.h"

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
        album  = 0;
        widget = 0;
    };

    HistoryItem(Album *a, QWidget *w)
    {
        album  = a;
        widget = w;
    };

    bool operator==(const HistoryItem& item)
    {
        return (album == item.album) && (widget == item.widget);
    }

    Album*   album;
    QWidget* widget;
};

AlbumHistory::AlbumHistory()
{
    m_backwardStack = new AlbumStack;
    m_forwardStack  = new AlbumStack;
    m_moving        = false;
}

AlbumHistory::~AlbumHistory()
{
    clearHistory();

    delete m_backwardStack;
    delete m_forwardStack;
}

void AlbumHistory::clearHistory()
{
    AlbumStack::const_iterator iter = m_backwardStack->constBegin();

    for(; iter != m_backwardStack->constEnd(); ++iter)
        delete *iter;

    m_backwardStack->clear();

    iter = m_forwardStack->constBegin();

    for(; iter != m_forwardStack->constEnd(); ++iter)
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

    // Same album as before in the history
    if(!m_backwardStack->isEmpty() && m_backwardStack->last()->album == album)
    {
        m_backwardStack->last()->widget = widget;
        return;
    }

    HistoryItem *item = new HistoryItem(album, widget);

    m_backwardStack->push_back(item);

    // The forward stack has to be cleared, if backward stack was changed
    if(!m_forwardStack->isEmpty())
    {
        AlbumStack::const_iterator iter = m_forwardStack->constBegin();

        for(; iter != m_forwardStack->constEnd(); ++iter)
            delete *iter;

        m_forwardStack->clear();
    }
}

void AlbumHistory::deleteAlbum(Album *album)
{
    if(!album || m_backwardStack->isEmpty())
        return;

    //  Search all HistoryItems, with album and delete them
    AlbumStack::iterator iter = m_backwardStack->begin();
    while(iter != m_backwardStack->end())
    {
        if((*iter)->album == album)
        {
            delete *iter;
            iter = m_backwardStack->erase(iter);
        }
        else
        {
            ++iter;
        }
    }
    iter = m_forwardStack->begin();
    while(iter != m_forwardStack->end())
    {
        if((*iter)->album == album)
        {
            delete *iter;
            iter = m_forwardStack->erase(iter);
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
    // that neighboring albums are different
    AlbumStack::iterator lhs = m_backwardStack->begin();
    AlbumStack::iterator rhs = lhs;
    ++rhs;
    while(rhs != m_backwardStack->end())
    {
        if(*lhs == *rhs)
        {
            rhs = m_backwardStack->erase(rhs);
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
            rhs = m_forwardStack->erase(rhs);
        }
        else
        {
            if(lhs == (m_backwardStack->isEmpty() ? m_backwardStack->end() : --m_backwardStack->end()))
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

void AlbumHistory::getBackwardHistory(QStringList& list) const
{
    if(m_backwardStack->isEmpty())
        return;

    Album* album = 0;

    AlbumStack::const_iterator iter = m_backwardStack->constBegin();
    for(; iter != (m_backwardStack->isEmpty() ? m_backwardStack->constEnd() : --m_backwardStack->constEnd()); ++iter)
    {
        album = (*iter)->album;
        if (album)
        {
            list.push_front(album->title());
        }
    }
}

void AlbumHistory::getForwardHistory(QStringList& list) const
{
    if(m_forwardStack->isEmpty())
        return;

    Album* album = 0;

    AlbumStack::const_iterator iter;
    for(iter = m_forwardStack->constBegin(); iter != m_forwardStack->constEnd(); ++iter)
    {
        album = (*iter)->album;
        if (album)
        {
            list.append(album->title());
        }
    }
}

void AlbumHistory::back(Album **album, QWidget **widget, unsigned int steps)
{
    *album  = 0;
    *widget = 0;

    if(m_backwardStack->count() <= 1 || (int)steps > m_backwardStack->count())
        return; // Only the current album available

    while(steps)
    {
        m_forwardStack->push_front(m_backwardStack->last());
        m_backwardStack->erase((m_backwardStack->isEmpty() ? m_backwardStack->end() : --m_backwardStack->end()));
        --steps;
    }
    m_moving = true;

    HistoryItem *item = getCurrentAlbum();
    if(item)
    {
        *album  = item->album;
        *widget = item->widget;
    }
}

void AlbumHistory::forward(Album **album, QWidget **widget, unsigned int steps)
{
    *album  = 0;
    *widget = 0;

    if(m_forwardStack->isEmpty() || (int)steps > m_forwardStack->count())
        return;

    forward(steps);

    HistoryItem *item = getCurrentAlbum();
    if(item)
    {
        *album  = item->album;
        *widget = item->widget;
    }
}

void AlbumHistory::forward(unsigned int steps)
{
    if(m_forwardStack->isEmpty() || (int)steps > m_forwardStack->count())
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
    *album  = 0;
    *widget = 0;

    if(m_backwardStack->isEmpty())
        return;

    HistoryItem *item = m_backwardStack->last();
    if(item)
    {
        *album  = item->album;
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
