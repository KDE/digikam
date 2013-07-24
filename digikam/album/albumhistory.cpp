/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : albums history manager.
 *
 * Copyright (C) 2004      by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

    HistoryItem(Album* const a, QWidget* const w)
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

    ImageInfo        current;
    QList<ImageInfo> select;
};

// ---------------------------------------------------------------------

class AlbumHistory::Private
{
public:

    Private() :
        moving(false),
        blockSelection(false)
    {
    }

    void forward(unsigned int steps = 1);

public:

    bool                          moving;
    bool                          blockSelection;

    QList<HistoryItem>            backwardStack;
    QList<HistoryItem>            forwardStack;
    QMap<Album*, HistoryPosition> historyPos;
};

void AlbumHistory::Private::forward(unsigned int steps)
{
    if (forwardStack.isEmpty() || (int)steps > forwardStack.count())
    {
        return;
    }

    while (steps)
    {
        backwardStack << forwardStack.takeFirst();
        --steps;
    }

    moving = true;
}

AlbumHistory::AlbumHistory()
    : d(new Private)
{
}

AlbumHistory::~AlbumHistory()
{
    clearHistory();
    delete d;
}

void AlbumHistory::clearHistory()
{
    d->backwardStack.clear();
    d->forwardStack.clear();
    d->historyPos.clear();

    d->moving = false;
}

void AlbumHistory::addAlbum(Album* const album, QWidget* const widget)
{
    if (!album || !widget || d->moving)
    {
        d->moving = false;
        return;
    }

    // Same album as before in the history
    if (!d->backwardStack.isEmpty() && d->backwardStack.last().album == album)
    {
        d->backwardStack.last().widget = widget;
        return;
    }

    d->backwardStack << HistoryItem(album, widget);

    // The forward stack has to be cleared, if backward stack was changed
    d->forwardStack.clear();
}

void AlbumHistory::deleteAlbum(Album* const album)
{
    if (!album || d->backwardStack.isEmpty())
    {
        return;
    }

    //  Search all HistoryItems, with album and delete them
    QList<HistoryItem>::iterator it = d->backwardStack.begin();

    while (it != d->backwardStack.end())
    {
        if (it->album == album)
        {
            it = d->backwardStack.erase(it);
        }
        else
        {
            ++it;
        }
    }

    it = d->forwardStack.begin();

    while (it != d->forwardStack.end())
    {
        if (it->album == album)
        {
            it = d->forwardStack.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (d->backwardStack.isEmpty() && d->forwardStack.isEmpty())
    {
        return;
    }

    // If backwardStack is empty, then there is no current album.
    // So make the first album of the forwardStack the current one.
    if (d->backwardStack.isEmpty())
    {
        d->forward();
    }

    // After the album is deleted from the history it has to be ensured,
    // that neighboring albums are different
    QList<HistoryItem>::iterator lhs = d->backwardStack.begin();
    QList<HistoryItem>::iterator rhs = lhs;
    ++rhs;

    while (rhs != d->backwardStack.end())
    {
        if (*lhs == *rhs)
        {
            rhs = d->backwardStack.erase(rhs);
        }
        else
        {
            ++lhs;
            rhs = lhs;
            ++rhs;
        }
    }

    rhs = d->forwardStack.begin();

    while (rhs != d->forwardStack.end())
    {
        if (*lhs == *rhs)
        {
            rhs = d->forwardStack.erase(rhs);
        }
        else
        {
            if (lhs == (d->backwardStack.isEmpty() ? d->backwardStack.end() : --d->backwardStack.end()))
            {
                lhs = d->forwardStack.begin();
            }
            else
            {
                ++lhs;
                rhs = lhs;
            }

            ++rhs;
        }
    }

    if (d->backwardStack.isEmpty() && !d->forwardStack.isEmpty())
    {
        d->forward();
    }
}

void AlbumHistory::getBackwardHistory(QStringList& list) const
{
    if (d->backwardStack.isEmpty())
    {
        return;
    }

    QList<HistoryItem>::const_iterator it = d->backwardStack.constBegin();

    for (; it != (d->backwardStack.isEmpty() ? d->backwardStack.constEnd() : --d->backwardStack.constEnd()); ++it)
    {
        if (it->album)
        {
            list.push_front(it->album->title());
        }
    }
}

void AlbumHistory::getForwardHistory(QStringList& list) const
{
    if (d->forwardStack.isEmpty())
    {
        return;
    }

    QList<HistoryItem>::const_iterator it;

    for (it = d->forwardStack.constBegin(); it != d->forwardStack.constEnd(); ++it)
    {
        if (it->album)
        {
            list.append(it->album->title());
        }
    }
}

void AlbumHistory::back(Album** const album, QWidget** const widget, unsigned int steps)
{
    *album  = 0;
    *widget = 0;

    if (d->backwardStack.count() <= 1 || (int)steps > d->backwardStack.count())
    {
        return;    // Only the current album available
    }

    while (steps)
    {
        d->forwardStack.prepend(d->backwardStack.takeLast());
        --steps;
    }

    d->moving = true;

    if (d->backwardStack.isEmpty())
    {
        return;
    }

    *album  = d->backwardStack.last().album;
    *widget = d->backwardStack.last().widget;
}

void AlbumHistory::forward(Album** const album, QWidget** const widget, unsigned int steps)
{
    *album  = 0;
    *widget = 0;

    if (d->forwardStack.isEmpty() || (int)steps > d->forwardStack.count())
    {
        return;
    }

    d->forward(steps);

    if (d->backwardStack.isEmpty())
    {
        return;
    }

    *album  = d->backwardStack.last().album;
    *widget = d->backwardStack.last().widget;
}

void AlbumHistory::getCurrentAlbum(Album** const album, QWidget** const widget) const
{
    *album  = 0;
    *widget = 0;

    if (d->backwardStack.isEmpty())
    {
        return;
    }

    *album  = d->backwardStack.last().album;
    *widget = d->backwardStack.last().widget;
}

bool AlbumHistory::isForwardEmpty() const
{
    return d->forwardStack.isEmpty();
}

bool AlbumHistory::isBackwardEmpty() const
{
    // the last album of the backwardStack is the currently shown
    // album, and therfore not really a previous album
    return (d->backwardStack.count() <= 1) ? true : false;
}

void AlbumHistory::slotAlbumSelected()
{
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    Album* currentAlbum = 0;

    if(!albumList.isEmpty())
    {
        currentAlbum = albumList.first();
    }

    if (d->historyPos.contains(currentAlbum))
    {
        d->blockSelection = true;
        emit signalSetCurrent(d->historyPos[currentAlbum].current.id());
    }
}

void AlbumHistory::slotAlbumCurrentChanged()
{
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    Album* currentAlbum = 0;

    if(!albumList.isEmpty())
    {
        currentAlbum = albumList.first();
    }

    if (d->historyPos.contains(currentAlbum))
    {
        if (d->historyPos[currentAlbum].select.size())
        {
            emit signalSetSelectedInfos(d->historyPos[currentAlbum].select);
        }
    }

    d->blockSelection = false;
}

void AlbumHistory::slotCurrentChange(const ImageInfo& info)
{
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    Album* currentAlbum = 0;

    if(!albumList.isEmpty())
    {
        currentAlbum = albumList.first();
    }

    d->historyPos[currentAlbum].current = info;
}

void AlbumHistory::slotImageSelected(const ImageInfoList& selectedImages)
{
    if (d->blockSelection)
    {
        return;
    }

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    Album* currentAlbum = 0;

    if(!albumList.isEmpty())
    {
        currentAlbum = albumList.first();
    }

    if (d->historyPos.contains(currentAlbum))
    {
        d->historyPos[currentAlbum].select = selectedImages;
    }
}

void AlbumHistory::slotClearSelectPAlbum(const ImageInfo& imageInfo)
{
    Album* const album = dynamic_cast<Album*>(AlbumManager::instance()->findPAlbum(imageInfo.albumId()));

    if (d->historyPos.contains(album))
    {
        d->historyPos[album].select.clear();
    }
}

void AlbumHistory::slotClearSelectTAlbum(int id)
{
    Album* const album = dynamic_cast<Album*>(AlbumManager::instance()->findTAlbum(id));

    if (d->historyPos.contains(album))
    {
        d->historyPos[album].select.clear();
    }
}

void AlbumHistory::slotAlbumDeleted(Album* album)
{
    deleteAlbum(album);

    if (d->historyPos.contains(album))
    {
        d->historyPos.remove(album);
    }
}

void AlbumHistory::slotAlbumsCleared()
{
    clearHistory();
}

}  // namespace Digikam
