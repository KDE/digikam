/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Track file loading and managing
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
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

#include "trackmanager.h"

// Qt includes

#include <QtConcurrentMap>
#include <QFuture>
#include <QFutureWatcher>

// local includes

#include "trackreader.h"

namespace Digikam
{

// TrackManager::TrackPoint ---------------------------------------------------

bool TrackManager::TrackPoint::EarlierThan(const TrackPoint& a, const TrackPoint& b)
{
    return (a.dateTime < b.dateTime);
}

// TrackManager ---------------------------------------------------------------

class Q_DECL_HIDDEN TrackManager::Private
{
public:

    Private()
      : trackLoadFutureWatcher(0),
        trackLoadFuture(),
        trackList(),
        loadErrorFiles(),
        nextTrackId(1),
        nextTrackColor(0),
        visibility(true)
    {
    }

    QFutureWatcher<TrackReader::TrackReadResult>* trackLoadFutureWatcher;
    QFuture<TrackReader::TrackReadResult>         trackLoadFuture;
    TrackManager::Track::List                     trackPendingList;
    TrackManager::Track::List                     trackList;
    QList<QPair<QUrl, QString> >                  loadErrorFiles;

    Id                                            nextTrackId;
    int                                           nextTrackColor;
    bool                                          visibility;
};

TrackManager::TrackManager(QObject* const parent)
    : QObject(parent),
      d(new Private)
{
}

TrackManager::~TrackManager()
{
}

bool TrackManager::getVisibility() const
{
    return d->visibility;
}

void TrackManager::setVisibility(const bool value)
{
    if (d->visibility == value)
    {
        return;
    }

    d->visibility = value;
    emit(signalVisibilityChanged(value));
}

void TrackManager::clear()
{
    /// @TODO send a signal
    d->trackList.clear();
}

const TrackManager::Track& TrackManager::getTrack(const int index) const
{
    return d->trackList.at(index);
}

void TrackManager::loadTrackFiles(const QList<QUrl>& urls)
{
    d->trackLoadFutureWatcher = new QFutureWatcher<TrackReader::TrackReadResult>(this);

    connect(d->trackLoadFutureWatcher, SIGNAL(resultsReadyAt(int,int)),
            this, SLOT(slotTrackFilesReadyAt(int,int)));

    connect(d->trackLoadFutureWatcher, SIGNAL(finished()),
            this, SLOT(slotTrackFilesFinished()));

    d->trackLoadFuture = QtConcurrent::mapped(urls, TrackReader::loadTrackFile);
    d->trackLoadFutureWatcher->setFuture(d->trackLoadFuture);

    // results are reported to slotTrackFilesReadyAt
}

void TrackManager::slotTrackFilesReadyAt(int beginIndex, int endIndex)
{
    //const int nFilesBefore = d->trackList.count();

    // note that endIndex is exclusive!
    for (int i = beginIndex; i < endIndex; ++i)
    {
        const TrackReader::TrackReadResult nextFile = d->trackLoadFuture.resultAt(i);

        if (nextFile.isValid)
        {
            Track nextTrack = nextFile.track;
            nextTrack.id    = getNextFreeTrackId();
            nextTrack.color = getNextFreeTrackColor();
            d->trackPendingList << nextTrack;
        }
        else
        {
            d->loadErrorFiles << QPair<QUrl, QString>(nextFile.track.url, nextFile.loadError);
        }
    }
}

void TrackManager::slotTrackFilesFinished()
{
    d->trackLoadFutureWatcher->deleteLater();

    d->trackList << d->trackPendingList;
    QList<TrackChanges> trackChanges;

    Q_FOREACH(const Track& track, d->trackPendingList)
    {
        trackChanges << TrackChanges(track.id, ChangeAdd);
    }

    d->trackPendingList.clear();

    emit(signalAllTrackFilesReady());
    emit(signalTracksChanged(trackChanges));
}

TrackManager::Track::List TrackManager::getTrackList() const
{
    return d->trackList;
}

int TrackManager::trackCount() const
{
    return d->trackList.count();
}

QList<QPair<QUrl, QString> > TrackManager::readLoadErrors()
{
    const QList<QPair<QUrl, QString> > result = d->loadErrorFiles;
    d->loadErrorFiles.clear();

    return result;
}

quint64 TrackManager::getNextFreeTrackId()
{
    const quint64 nextId = d->nextTrackId;
    ++(d->nextTrackId);

    return nextId;
}

TrackManager::Track TrackManager::getTrackById(const quint64 trackId) const
{
    Q_FOREACH(const Track& track, d->trackList)
    {
        if (track.id == trackId)
        {
            return track;
        }
    }

    return TrackManager::Track();
}

QColor TrackManager::getNextFreeTrackColor()
{
    QList<QColor> colorList;
    colorList << Qt::red << Qt::blue << Qt::green << Qt::magenta;

    const QColor nextColor = colorList.at(d->nextTrackColor);
    ++(d->nextTrackColor);

    if (d->nextTrackColor >= colorList.count())
    {
        d->nextTrackColor = 0;
    }

    return nextColor;
}

} // namespace Digikam
