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

#ifndef TRACK_MANAGER_H
#define TRACK_MANAGER_H

// Qt includes

#include <QColor>
#include <QDateTime>
#include <QUrl>

// local includes

#include "geoifacetypes.h"
#include "geocoordinates.h"
#include "digikam_export.h"

class TestGPXParsing;

namespace Digikam
{

class DIGIKAM_EXPORT TrackManager : public QObject
{
    Q_OBJECT

public:

    class  TrackPoint
    {
    public:

        TrackPoint()
          : dateTime(),
            coordinates(),
            nSatellites(-1),
            hDop(-1),
            pDop(-1),
            fixType(-1),
            speed(-1)
        {
        }

        static bool EarlierThan(const TrackPoint& a, const TrackPoint& b);

    public:

        QDateTime                 dateTime;
        GeoCoordinates            coordinates;
        int                       nSatellites;
        qreal                     hDop;
        qreal                     pDop;
        int                       fixType;
        qreal                     speed;

        typedef QList<TrackPoint> List;
    };

    // -------------------------------------

    // We assume here that we will never load more than uint32_max tracks.
    typedef quint32 Id;

    class Track
    {
    public:

        enum Flags
        {
            FlagVisible = 1,
            FlagDefault = FlagVisible
        };

    public:

        Track()
          : url(),
            points(),
            id(0),
            color(Qt::red),
            flags(FlagDefault)
        {
            qRegisterMetaType<TrackChanges>("TrackChanges");
        }

        QUrl                 url;
        QList<TrackPoint>    points;
        /// 0 means no track id assigned yet
        Id                   id;
        QColor               color;
        Flags                flags;

        typedef QList<Track> List;
    };

    enum ChangeFlag
    {
        ChangeTrackPoints = 1,
        ChangeMetadata    = 2,

        ChangeRemoved     = 4,
        ChangeAdd         = ChangeTrackPoints | ChangeMetadata
    };

    typedef QPair<Id, ChangeFlag> TrackChanges;

public:

    explicit TrackManager(QObject* const parent = 0);
    virtual ~TrackManager();

    void loadTrackFiles(const QList<QUrl>& urls);
    QList<QPair<QUrl, QString> > readLoadErrors();
    void clear();

    const Track& getTrack(const int index) const;
    Track::List getTrackList() const;
    int trackCount() const;

    quint64 getNextFreeTrackId();
    Track   getTrackById(const quint64 trackId) const;
    QColor  getNextFreeTrackColor();

    void setVisibility(const bool value);
    bool getVisibility() const;

Q_SIGNALS:

    void signalTrackFilesReadyAt(const int startIndex, const int endIndex);
    void signalAllTrackFilesReady();
    void signalTracksChanged(const QList<TrackManager::TrackChanges> trackChanges);
    void signalVisibilityChanged(const bool newValue);

private Q_SLOTS:

    void slotTrackFilesReadyAt(int beginIndex, int endIndex);
    void slotTrackFilesFinished();

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam

#endif // TRACK_MANAGER_H
