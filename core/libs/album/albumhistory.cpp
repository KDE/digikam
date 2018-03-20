/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : albums history manager.
 *
 * Copyright (C) 2004      by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Mohamed Anwer <m dot anwer at gmx dot com>
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

// Qt includes

#include <QString>
#include <QWidget>
#include <QHash>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "imageinfo.h"
#include "albummanager.h"
#include "albumlabelstreeview.h"

uint qHash(QList<Digikam::Album*> key)
{
    if(key.isEmpty())
        return 0;

    uint value;
    Digikam::Album* temp = key.first();
    quint64 myint = (unsigned long long)temp;
    value = qHash(myint);

    for(int it = 1; it < key.size(); ++it)
    {
        Digikam::Album* al = key.at(it);
        quint64 myint = (unsigned long long)al;
        value ^= qHash(myint);
    }

    return value;
}

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
        widget = 0;
    };

    HistoryItem(QList<Album*> const a, QWidget* const w)
    {
        albums.append(a);
        widget = w;
    };

    HistoryItem(QList<Album*> const a, QWidget* const w, QHash<AlbumLabelsTreeView::Labels, QList<int> > selectedLabels)
    {
        albums.append(a);
        widget = w;
        labels = selectedLabels;
    };

    bool operator==(const HistoryItem& item)
    {
        if(widget != item.widget)
        {
            return false;
        }

        return albums == item.albums;
    }

    QList<Album*>                                   albums;
    QWidget*                                        widget;
    QHash<AlbumLabelsTreeView::Labels, QList<int> > labels;
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

    bool                                            moving;
    bool                                            blockSelection;

    QList<HistoryItem>                              backwardStack;
    QList<HistoryItem>                              forwardStack;
    QHash<QList<Album*>, HistoryPosition>           historyPos;
    QHash<AlbumLabelsTreeView::Labels, QList<int> > neededLabels;
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

void AlbumHistory::addAlbums(QList<Album*> const albums, QWidget* const widget)
{

    if (albums.isEmpty() || !widget || d->moving)
    {
        d->moving = false;
        return;
    }

    // Same album as before in the history
    if (!d->backwardStack.isEmpty() && d->backwardStack.last().albums == albums)
    {
        d->backwardStack.last().widget = widget;
        return;
    }

    d->backwardStack << HistoryItem(albums, widget);

    // The forward stack has to be cleared, if backward stack was changed
    d->forwardStack.clear();
}

/**
 * @brief AlbumHistory::addAlbums
 *        A special overloaded function for handling AlbumHistory
 *        for the Labels tree-view
 *
 * @author Mohamed Anwer
 */
void AlbumHistory::addAlbums(QList<Album*> const albums, QWidget* const widget, QHash<AlbumLabelsTreeView::Labels, QList<int> > selectedLabels)
{

    if (albums.isEmpty() || !widget || d->moving)
    {
        d->moving = false;
        return;
    }

    if (!d->backwardStack.isEmpty() && d->backwardStack.last().albums.first()->isUsedByLabelsTree())
    {
        d->backwardStack.last().widget = widget;
        d->backwardStack.last().labels = selectedLabels;
        return;
    }

    d->backwardStack << HistoryItem(albums, widget, selectedLabels);

    // The forward stack has to be cleared, if backward stack was changed
    d->forwardStack.clear();
}

void AlbumHistory::deleteAlbum(Album* const album)
{
    if (!album || d->backwardStack.isEmpty())
    {
        return;
    }
    QList<Album*> albums;
    albums << album;

    //  Search all HistoryItems, with album and delete them
    QList<HistoryItem>::iterator it = d->backwardStack.begin();


    while (it != d->backwardStack.end())
    {
        if (it->albums == albums)
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
        if (it->albums == albums)
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
        if (!(it->albums.isEmpty()))
        {
            QString name;

            for (int iter = 0; iter < it->albums.size(); ++iter)
            {
                name.append(it->albums.at(iter)->title());

                if (iter+1 < it->albums.size())
                {
                    name.append(QLatin1String("/"));
                }
            }

            list.push_front(name);
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
        if (!(it->albums.isEmpty()))
        {
            QString name;

            for (int iter = 0; iter < it->albums.size(); ++iter)
            {
                name.append(it->albums.at(iter)->title());

                if (iter+1 < it->albums.size())
                {
                    name.append(QLatin1String("/"));
                }
            }

            list.append(name);
        }
    }
}

void AlbumHistory::back(QList<Album*>& album, QWidget** const widget, unsigned int steps)
{
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

    album.append(d->backwardStack.last().albums);
    *widget         = d->backwardStack.last().widget;
    d->neededLabels = d->backwardStack.last().labels;
}

void AlbumHistory::forward(QList<Album*>& album, QWidget** const widget, unsigned int steps)
{
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

    album.append(d->backwardStack.last().albums);
    *widget         = d->backwardStack.last().widget;
    d->neededLabels = d->backwardStack.last().labels;
}

void AlbumHistory::getCurrentAlbum(Album** const album, QWidget** const widget) const
{
    *album  = 0;
    *widget = 0;

    if (d->backwardStack.isEmpty())
    {
        return;
    }

    if(!(d->backwardStack.last().albums.isEmpty()))
    {
        *album  = d->backwardStack.last().albums.first();
    }

    *widget = d->backwardStack.last().widget;
}

bool AlbumHistory::isForwardEmpty() const
{
    return d->forwardStack.isEmpty();
}

bool AlbumHistory::isBackwardEmpty() const
{
    // the last album of the backwardStack is the currently shown
    // album, and therefore not really a previous album
    return (d->backwardStack.count() <= 1) ? true : false;
}

QHash<AlbumLabelsTreeView::Labels, QList<int> > AlbumHistory::neededLabels()
{
    return d->neededLabels;
}

void AlbumHistory::slotAlbumSelected()
{
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();

    if (d->historyPos.contains(albumList))
    {
        d->blockSelection = true;
        emit signalSetCurrent(d->historyPos[albumList].current.id());
    }
}

void AlbumHistory::slotAlbumCurrentChanged()
{
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();

    if (!(albumList.isEmpty()) && d->historyPos.contains(albumList))
    {
        if (d->historyPos[albumList].select.size())
        {
            emit signalSetSelectedInfos(d->historyPos[albumList].select);
        }
    }

    d->blockSelection = false;
}

void AlbumHistory::slotCurrentChange(const ImageInfo& info)
{
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();

    if (albumList.isEmpty())
        return;

    d->historyPos[albumList].current = info;
}

void AlbumHistory::slotImageSelected(const ImageInfoList& selectedImages)
{
    if (d->blockSelection)
    {
        return;
    }

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();

    if (d->historyPos.contains(albumList))
    {
        d->historyPos[albumList].select = selectedImages;
    }
}

void AlbumHistory::slotClearSelectPAlbum(const ImageInfo& imageInfo)
{
    Album* const album = dynamic_cast<Album*>(AlbumManager::instance()->findPAlbum(imageInfo.albumId()));
    QList<Album*> albums;
    albums << album;

    if (d->historyPos.contains(albums))
    {
        d->historyPos[albums].select.clear();
    }
}

void AlbumHistory::slotClearSelectTAlbum(int id)
{
    Album* const album = dynamic_cast<Album*>(AlbumManager::instance()->findTAlbum(id));
    QList<Album*> albums;
    albums << album;

    if (d->historyPos.contains(albums))
    {
        d->historyPos[albums].select.clear();
    }
}

void AlbumHistory::slotAlbumDeleted(Album* album)
{
    deleteAlbum(album);
    QList<Album*> albums;
    albums << album;

    if (d->historyPos.contains(albums))
    {
        d->historyPos.remove(albums);
    }
}

void AlbumHistory::slotAlbumsCleared()
{
    clearHistory();
}

}  // namespace Digikam
