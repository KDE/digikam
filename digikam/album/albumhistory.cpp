/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : albums history manager.
 *
 * Copyright (C) 2004 by Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumhistory.moc"

// Qt includes

#include <QString>
#include <QWidget>

// Local includes

#include "album.h"
#include "imageinfo.h"
#include "albummanager.h"

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

    HistoryItem(Album* a, QWidget* w)
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

// ---------------------------------------------------------------------

class HistoryPosition
{
public:

    HistoryPosition()
    {

    };

    HistoryPosition(const ImageInfo& c, const QList<ImageInfo>& s)
    {
        current = c;
        select  = s;
    };

    bool operator==(const HistoryPosition& item)
    {
        return (current == item.current) && (select == item.select);
    }

    ImageInfo  current;
    QList<ImageInfo> select;
};

// ---------------------------------------------------------------------

class AlbumHistory::AlbumHistoryPriv
{
public:

    AlbumHistoryPriv() :
        moving(false),
        blockSelection(false),
        backwardStack(0),
        forwardStack(0)
    {
    }

    typedef QList<HistoryItem*>   AlbumStack;

    bool                          moving;
    bool                          blockSelection;

    AlbumStack*                   backwardStack;
    AlbumStack*                   forwardStack;
    QMap<Album*, HistoryPosition> historyPos;
};

AlbumHistory::AlbumHistory()
    : d(new AlbumHistoryPriv)
{
    d->backwardStack = new AlbumHistoryPriv::AlbumStack;
    d->forwardStack  = new AlbumHistoryPriv::AlbumStack;
}

AlbumHistory::~AlbumHistory()
{
    clearHistory();

    delete d->backwardStack;
    delete d->forwardStack;
    delete d;
}

void AlbumHistory::clearHistory()
{
    AlbumHistoryPriv::AlbumStack::const_iterator iter = d->backwardStack->constBegin();

    for (; iter != d->backwardStack->constEnd(); ++iter)
    {
        delete *iter;
    }

    d->backwardStack->clear();

    iter = d->forwardStack->constBegin();

    for (; iter != d->forwardStack->constEnd(); ++iter)
    {
        delete *iter;
    }

    d->forwardStack->clear();

    d->moving = false;
}

void AlbumHistory::addAlbum(Album* album, QWidget* widget)
{
    if (!album || !widget || d->moving)
    {
        d->moving = false;
        return;
    }

    // Same album as before in the history
    if (!d->backwardStack->isEmpty() && d->backwardStack->last()->album == album)
    {
        d->backwardStack->last()->widget = widget;
        return;
    }

    HistoryItem* item = new HistoryItem(album, widget);

    d->backwardStack->push_back(item);

    // The forward stack has to be cleared, if backward stack was changed
    if (!d->forwardStack->isEmpty())
    {
        AlbumHistoryPriv::AlbumStack::const_iterator iter = d->forwardStack->constBegin();

        for (; iter != d->forwardStack->constEnd(); ++iter)
        {
            delete *iter;
        }

        d->forwardStack->clear();
    }
}

void AlbumHistory::deleteAlbum(Album* album)
{
    if (!album || d->backwardStack->isEmpty())
    {
        return;
    }

    //  Search all HistoryItems, with album and delete them
    AlbumHistoryPriv::AlbumStack::iterator iter = d->backwardStack->begin();

    while (iter != d->backwardStack->end())
    {
        if ((*iter)->album == album)
        {
            delete *iter;
            iter = d->backwardStack->erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    iter = d->forwardStack->begin();

    while (iter != d->forwardStack->end())
    {
        if ((*iter)->album == album)
        {
            delete *iter;
            iter = d->forwardStack->erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    if (d->backwardStack->isEmpty() && d->forwardStack->isEmpty())
    {
        return;
    }

    // If backwardStack is empty, then there is no current album.
    // So make the first album of the forwardStack the current one.
    if (d->backwardStack->isEmpty())
    {
        forward();
    }

    // After the album is deleted from the history it has to be ensured,
    // that neighboring albums are different
    AlbumHistoryPriv::AlbumStack::iterator lhs = d->backwardStack->begin();
    AlbumHistoryPriv::AlbumStack::iterator rhs = lhs;
    ++rhs;

    while (rhs != d->backwardStack->end())
    {
        if (*lhs == *rhs)
        {
            rhs = d->backwardStack->erase(rhs);
        }
        else
        {
            ++lhs;
            rhs = lhs;
            ++rhs;
        }
    }

    rhs = d->forwardStack->begin();

    while (rhs != d->forwardStack->end())
    {
        if (*lhs == *rhs)
        {
            rhs = d->forwardStack->erase(rhs);
        }
        else
        {
            if (lhs == (d->backwardStack->isEmpty() ? d->backwardStack->end() : --d->backwardStack->end()))
            {
                lhs = d->forwardStack->begin();
            }
            else
            {
                ++lhs;
                rhs = lhs;
            }

            ++rhs;
        }
    }

    if (d->backwardStack->isEmpty() && !d->forwardStack->isEmpty())
    {
        forward();
    }
}

void AlbumHistory::getBackwardHistory(QStringList& list) const
{
    if (d->backwardStack->isEmpty())
    {
        return;
    }

    Album* album = 0;

    AlbumHistoryPriv::AlbumStack::const_iterator iter = d->backwardStack->constBegin();

    for (; iter != (d->backwardStack->isEmpty() ? d->backwardStack->constEnd() : --d->backwardStack->constEnd()); ++iter)
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
    if (d->forwardStack->isEmpty())
    {
        return;
    }

    Album* album = 0;

    AlbumHistoryPriv::AlbumStack::const_iterator iter;

    for (iter = d->forwardStack->constBegin(); iter != d->forwardStack->constEnd(); ++iter)
    {
        album = (*iter)->album;

        if (album)
        {
            list.append(album->title());
        }
    }
}

void AlbumHistory::back(Album** album, QWidget** widget, unsigned int steps)
{
    *album  = 0;
    *widget = 0;

    if (d->backwardStack->count() <= 1 || (int)steps > d->backwardStack->count())
    {
        return;    // Only the current album available
    }

    while (steps)
    {
        d->forwardStack->push_front(d->backwardStack->last());
        d->backwardStack->erase((d->backwardStack->isEmpty() ? d->backwardStack->end() : --d->backwardStack->end()));
        --steps;
    }

    d->moving = true;

    HistoryItem* item = getCurrentAlbum();

    if (item)
    {
        *album  = item->album;
        *widget = item->widget;
    }
}

void AlbumHistory::forward(Album** album, QWidget** widget, unsigned int steps)
{
    *album  = 0;
    *widget = 0;

    if (d->forwardStack->isEmpty() || (int)steps > d->forwardStack->count())
    {
        return;
    }

    forward(steps);

    HistoryItem* item = getCurrentAlbum();

    if (item)
    {
        *album  = item->album;
        *widget = item->widget;
    }
}

void AlbumHistory::forward(unsigned int steps)
{
    if (d->forwardStack->isEmpty() || (int)steps > d->forwardStack->count())
    {
        return;
    }

    while (steps)
    {
        d->backwardStack->push_back(d->forwardStack->first());
        d->forwardStack->erase(d->forwardStack->begin());
        --steps;
    }

    d->moving = true;
}

HistoryItem* AlbumHistory::getCurrentAlbum() const
{
    if (d->backwardStack->isEmpty())
    {
        return 0;
    }

    return d->backwardStack->last();
}

void AlbumHistory::getCurrentAlbum(Album** album, QWidget** widget) const
{
    *album  = 0;
    *widget = 0;

    if (d->backwardStack->isEmpty())
    {
        return;
    }

    HistoryItem* item = d->backwardStack->last();

    if (item)
    {
        *album  = item->album;
        *widget = item->widget;
    }
}

bool AlbumHistory::isForwardEmpty() const
{
    return d->forwardStack->isEmpty();
}

bool AlbumHistory::isBackwardEmpty() const
{
    // the last album of the backwardStack is the currently shown
    // album, and therfore not really a previous album
    return (d->backwardStack->count() <= 1) ? true : false;
}

void AlbumHistory::slotAlbumSelected()
{
    Album* currentAlbum = AlbumManager::instance()->currentAlbum();

    if (d->historyPos.contains(currentAlbum))
    {
        if (currentAlbum->type() == Album::PHYSICAL || currentAlbum->type() == Album::TAG)
        {
            d->blockSelection = true;
            emit signalSetCurrent(d->historyPos[currentAlbum].current.id());
        }
    }
}

void AlbumHistory::slotAlbumCurrentChanged()
{
    Album* currentAlbum = AlbumManager::instance()->currentAlbum();

    if (d->historyPos.contains(currentAlbum))
    {
        if (currentAlbum->type() == Album::PHYSICAL || currentAlbum->type() == Album::TAG)
        {
            if (d->historyPos[currentAlbum].select.size())
            {
                emit signalSetSelectedInfos(d->historyPos[currentAlbum].select);
            }
        }
    }

    d->blockSelection = false;
}

void AlbumHistory::slotCurrentChange(const ImageInfo& info)
{
    Album* currentAlbum                = AlbumManager::instance()->currentAlbum();
    d->historyPos[currentAlbum].current = info;
}

void AlbumHistory::slotImageSelected(const ImageInfoList& selectedImages)
{
    if (d->blockSelection)
    {
        return;
    }

    Album* currentAlbum = AlbumManager::instance()->currentAlbum();

    if (d->historyPos.contains(currentAlbum))
    {
        d->historyPos[currentAlbum].select = selectedImages;
    }
}

void AlbumHistory::slotClearSelectPAlbum(const ImageInfo& imageInfo)
{
    Album* album = dynamic_cast<Album*>(AlbumManager::instance()->findPAlbum(imageInfo.albumId()));

    if (d->historyPos.contains(album))
    {
        d->historyPos[album].select.clear();
    }
}

void AlbumHistory::slotClearSelectTAlbum(int id)
{
    Album* album = dynamic_cast<Album*>(AlbumManager::instance()->findTAlbum(id));

    if (d->historyPos.contains(album))
    {
        d->historyPos[album].select.clear();
    }
}

void AlbumHistory::slotAlbumDeleted(Album* album)
{
    if (d->historyPos.contains(album))
    {
        d->historyPos.remove(album);
    }
}

}  // namespace Digikam
