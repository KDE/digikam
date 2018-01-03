/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Correlator for tracks and images
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef TRACK_CORRELATOR_H
#define TRACK_CORRELATOR_H

// Local includes

#include "trackmanager.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT TrackCorrelator : public QObject
{
    Q_OBJECT

public:

    enum CorrelationFlags
    {
        CorrelationFlagCoordinates  = 1,
        CorrelationFlagInterpolated = 2,
        CorrelationFlagAltitude     = 3
    };

public:

    class Correlation
    {
    public:

        Correlation()
          : dateTime(),
            userData(),
            nSatellites(-1),
            hDop(-1),
            pDop(-1),
            fixType(-1),
            speed(-1),
            flags(),
            coordinates()
        {
        }

        typedef QList<Correlation>    List;

        QDateTime                     dateTime;
        QVariant                      userData;
        int                           nSatellites;
        qreal                         hDop;
        qreal                         pDop;
        int                           fixType;
        qreal                         speed;
        CorrelationFlags              flags;
        GeoCoordinates                coordinates;
    };

    // -------------------------------------

    class CorrelationOptions
    {
    public:

        CorrelationOptions()
        : interpolate(false),
          interpolationDstTime(0),
          maxGapTime(0),
          secondsOffset(0)
        {
        }

        bool interpolate;
        int  interpolationDstTime;
        int  maxGapTime;
        int  secondsOffset;
    };

public:

    explicit TrackCorrelator(TrackManager* const trackManager, QObject* const parent = 0);
    virtual ~TrackCorrelator();

    void correlate(const Correlation::List& itemsToCorrelate, const CorrelationOptions& options);
    void cancelCorrelation();

Q_SIGNALS:

    void signalItemsCorrelated(const Digikam::TrackCorrelator::Correlation::List& correlatedItems);
    void signalAllItemsCorrelated();
    void signalCorrelationCanceled();

private Q_SLOTS:

    void slotThreadItemsCorrelated(const Digikam::TrackCorrelator::Correlation::List& correlatedItems);
    void slotThreadFinished();

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::TrackCorrelator::Correlation::List)

#endif // TRACK_CORRELATOR_H
