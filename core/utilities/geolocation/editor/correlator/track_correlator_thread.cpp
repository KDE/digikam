/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Thread for correlator for tracks and images
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

#include "track_correlator_thread.h"

// Local includes

#include "track_correlator.h"

namespace Digikam
{

bool TrackCorrelationLessThan(const TrackCorrelator::Correlation& a, const TrackCorrelator::Correlation& b)
{
    return (a.dateTime < b.dateTime);
}

TrackCorrelatorThread::TrackCorrelatorThread(QObject* const parent)
    : QThread(parent),
      doCancel(false),
      canceled(false)
{
}

TrackCorrelatorThread::~TrackCorrelatorThread()
{
}

void TrackCorrelatorThread::run()
{
    // sort the items to correlate by time:
    std::sort(itemsToCorrelate.begin(), itemsToCorrelate.end(), TrackCorrelationLessThan);

    // now perform the correlation
    // we search all loaded gpx data files in parallel for the points with the best match
    const int nFiles = fileList.count();
    QList<int> currentIndices;

    for (int i = 0 ; i < nFiles ; i++)
        currentIndices << 0;

    for (TrackCorrelator::Correlation::List::iterator it = itemsToCorrelate.begin() ; it != itemsToCorrelate.end() ; ++it)
    {
        if (doCancel)
        {
            canceled = true;
            return;
        }

        // GPS device are sync in time by satelite using GMT time.
        QDateTime itemDateTime = it->dateTime.addSecs(options.secondsOffset*(-1));

        // find the last point before our item:
        QDateTime       lastSmallerTime;
        QPair<int, int> lastIndexPair;
        QDateTime       firstBiggerTime;
        QPair<int, int> firstIndexPair;

        for (int f = 0 ; f < nFiles ; f++)
        {
            if (doCancel)
            {
                canceled = true;
                return;
            }

            const TrackManager::Track& currentFile = fileList.at(f);
            int index                              = currentIndices.at(f);

            for (; index < currentFile.points.count(); ++index)
            {
                if (doCancel)
                {
                    canceled = true;
                    return;
                }

                const QDateTime& indexTime = currentFile.points.at(index).dateTime;

                if (indexTime < itemDateTime)
                {
                    bool timeIsBetter = false;

                    if (lastSmallerTime.isValid())
                    {
                        timeIsBetter = (indexTime>lastSmallerTime);
                    }
                    else
                    {
                        timeIsBetter = true;
                    }

                    if (timeIsBetter)
                    {
                        lastSmallerTime = indexTime;
                        lastIndexPair   = QPair<int, int>(f, index);
                    }
                }
                else
                {
                    // is it the first time after our item?
                    bool timeIsBetter = false;

                    if (firstBiggerTime.isValid())
                    {
                        timeIsBetter = (indexTime<firstBiggerTime);
                    }
                    else
                    {
                        timeIsBetter = true;
                    }

                    if (timeIsBetter)
                    {
                        firstBiggerTime = indexTime;
                        firstIndexPair  = QPair<int, int>(f, index);
                    }

                    break;
                }
            }

            // Remember the last index which we searched in this file
            // to save time when looking for matching times for the next
            // item.
            // However, we have to decrease the index by one:
            // The current index should correspond to a time after the item,
            // and index-1 should correspond to a time before the item. The next
            // item may be before the time of the current index, and by decreasing
            // the stored index by 1 we ensure that we start our search at an index
            // corresponding to a time before the next item. Remember that the
            // items are sorted by time!
            currentIndices[f] = (index>1)?(index-1):0;
        }

        TrackCorrelator::Correlation correlatedData = *it;

        if (!options.interpolate)
        {
            // do we have a timestamp within maxGap?
            bool canUseTimeBefore = lastSmallerTime.isValid();
            int dtimeBefore       = 0;

            if (canUseTimeBefore)
            {
                dtimeBefore      = qAbs(lastSmallerTime.secsTo(itemDateTime));
                canUseTimeBefore = dtimeBefore <= options.maxGapTime;
            }

            bool canUseTimeAfter = firstBiggerTime.isValid();
            int dtimeAfter       = 0;

            if (canUseTimeAfter)
            {
                dtimeAfter      = qAbs(firstBiggerTime.secsTo(itemDateTime));
                canUseTimeAfter = dtimeAfter <= options.maxGapTime;
            }

            if (canUseTimeAfter || canUseTimeBefore)
            {
                QPair<int, int> indexToUse(-1, -1);

                if (canUseTimeAfter&&canUseTimeBefore)
                {
                    indexToUse = (dtimeBefore < dtimeAfter) ? lastIndexPair:firstIndexPair;
                }
                else if (canUseTimeAfter)
                {
                    indexToUse = firstIndexPair;
                }
                else if (canUseTimeBefore)
                {
                    indexToUse = lastIndexPair;
                }

                if (indexToUse.first>=0)
                {
                    const TrackManager::TrackPoint& dataPoint = fileList.at(indexToUse.first).points.at(indexToUse.second);
                    correlatedData.coordinates                          = dataPoint.coordinates;
                    correlatedData.flags                                = static_cast<TrackCorrelator::CorrelationFlags>(correlatedData.flags|TrackCorrelator::CorrelationFlagCoordinates);
                    correlatedData.nSatellites                          = dataPoint.nSatellites;
                    correlatedData.hDop                                 = dataPoint.hDop;
                    correlatedData.pDop                                 = dataPoint.pDop;
                    correlatedData.fixType                              = dataPoint.fixType;
                    correlatedData.speed                                = dataPoint.speed;
                }
            }
        }
        else
        {
            bool canInterpolate = lastSmallerTime.isValid() && firstBiggerTime.isValid();

            if (canInterpolate)
            {
                canInterpolate = qAbs(lastSmallerTime.secsTo(itemDateTime)) <= options.interpolationDstTime;
            }

            if (canInterpolate)
            {
                canInterpolate = qAbs(firstBiggerTime.secsTo(itemDateTime)) <= options.interpolationDstTime;
            }

            if (canInterpolate)
            {
                const TrackManager::TrackPoint& dataPointBefore = fileList.at(lastIndexPair.first).points.at(lastIndexPair.second);
                const TrackManager::TrackPoint& dataPointAfter  = fileList.at(firstIndexPair.first).points.at(firstIndexPair.second);

                const uint tBefore = dataPointBefore.dateTime.toTime_t();
                const uint tAfter  = dataPointAfter.dateTime.toTime_t();
                const uint tCor    = itemDateTime.toTime_t();

                if (tCor-tBefore != 0)
                {
                    GeoCoordinates resultCoordinates;
                    const double latBefore  = dataPointBefore.coordinates.lat();
                    const double lonBefore  = dataPointBefore.coordinates.lon();
                    const double latAfter   = dataPointAfter.coordinates.lat();
                    const double lonAfter   = dataPointAfter.coordinates.lon();
                    const qreal interFactor = qreal(tCor-tBefore) / qreal(tAfter-tBefore);

                    resultCoordinates.setLatLon(latBefore + (latAfter - latBefore) * interFactor,
                                                lonBefore + (lonAfter - lonBefore) * interFactor);

                    const bool hasAlt = dataPointBefore.coordinates.hasAltitude() && dataPointAfter.coordinates.hasAltitude();

                    if (hasAlt)
                    {
                        const double altBefore = dataPointBefore.coordinates.alt();
                        const double altAfter  = dataPointAfter.coordinates.alt();
                        resultCoordinates.setAlt(altBefore + (altAfter - altBefore) * interFactor);
                    }

                    correlatedData.coordinates = resultCoordinates;
                    correlatedData.flags       = static_cast<TrackCorrelator::CorrelationFlags>(correlatedData.flags | TrackCorrelator::CorrelationFlagCoordinates);
                }

            }
        }

        if (correlatedData.flags&TrackCorrelator::CorrelationFlagCoordinates)
        {
            TrackCorrelator::Correlation::List readyItems;
            readyItems << correlatedData;
            emit(signalItemsCorrelated(readyItems));
        }
    }
}

} // namespace Digikam
