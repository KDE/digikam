/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-14
 * Description : Common internal data structures for geolocation interface
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "geoifacecommon.h"

// Qt includes

#include <QStandardPaths>
#include <QUrl>

// Local includes

#include "mapbackend.h"
#include "digikam_debug.h"

namespace Digikam
{

class GeoIfaceGlobalObjectCreator
{
public:

    GeoIfaceGlobalObject object;
};

Q_GLOBAL_STATIC(GeoIfaceGlobalObjectCreator, geoifaceGlobalObjectCreator)

// -------------------------------------------------------------------------

class GeoIfaceGlobalObject::Private
{
public:

    Private()
        : internalMapWidgetsPool(),
          markerPixmaps()
    {
    }

    void loadMarkerPixmaps()
    {
        QStringList markerColors;
        markerColors
            << QLatin1String( "00ff00" )
            << QLatin1String( "00ffff" )
            << QLatin1String( "ff0000" )
            << QLatin1String( "ff7f00" )
            << QLatin1String( "ffff00" );

        QStringList stateNames;
        stateNames
            << QLatin1String( "" )
            << QLatin1String( "-selected" )
            << QLatin1String( "-someselected" );

        for (QStringList::const_iterator it = markerColors.constBegin() ; it != markerColors.constEnd() ; ++it)
        {
            for (QStringList::const_iterator sit = stateNames.constBegin() ; sit != stateNames.constEnd() ; ++sit)
            {
                const QString pixmapName  = *it + *sit;
                const QUrl markerUrl      = GeoIfaceGlobalObject::instance()->locateDataFile(QString::fromLatin1( "marker-%1.png").arg(pixmapName));
                markerPixmaps[pixmapName] = QPixmap(markerUrl.toLocalFile());
            }
        }

        const QUrl markerIconUrl                            = GeoIfaceGlobalObject::instance()->locateDataFile(QLatin1String( "marker-icon-16x16.png" ));
        markerPixmaps[QLatin1String( "marker-icon-16x16" )] = QPixmap(markerIconUrl.toLocalFile());
    }

public:

    QList<GeoIfaceInternalWidgetInfo> internalMapWidgetsPool;

    // marker pixmaps:
    QMap<QString, QPixmap>            markerPixmaps;
};

GeoIfaceGlobalObject::GeoIfaceGlobalObject()
    : QObject(),
      d(new Private())
{
}

GeoIfaceGlobalObject::~GeoIfaceGlobalObject()
{
    delete d;
}

GeoIfaceGlobalObject* GeoIfaceGlobalObject::instance()
{
    return &(geoifaceGlobalObjectCreator->object);
}

QPixmap GeoIfaceGlobalObject::getMarkerPixmap(const QString& pixmapId)
{
    if (d->markerPixmaps.isEmpty())
    {
        d->loadMarkerPixmaps();
    }

    return d->markerPixmaps.value(pixmapId);
}

QPixmap GeoIfaceGlobalObject::getStandardMarkerPixmap()
{
    return getMarkerPixmap(QLatin1String("00ff00"));
}

QUrl GeoIfaceGlobalObject::locateDataFile(const QString& filename)
{
    const QUrl dataFile = QUrl::fromLocalFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/geoiface/") + filename));
    qCDebug(DIGIKAM_GEOIFACE_LOG) << "located data: " << dataFile;
    return dataFile;
}

/**
 * @brief Parse a 'lat,lon' string a returned by the JavaScript parts
 * @return true if the string could be parsed successfully
 */
bool GeoIfaceHelperParseLatLonString(const QString& latLonString, GeoCoordinates* const coordinates)
{
    // parse a 'lat,lon' string:
    const QStringList coordinateStrings = latLonString.trimmed().split(QLatin1Char(','));
    bool valid                          = ( coordinateStrings.size() == 2 );

    if (valid)
    {
        double ptLongitude      = 0.0;
        const double ptLatitude = coordinateStrings.at(0).toDouble(&valid);

        if (valid)
        {
            ptLongitude = coordinateStrings.at(1).toDouble(&valid);
        }

        if (valid)
        {
            if (coordinates)
            {
                *coordinates = GeoCoordinates(ptLatitude, ptLongitude);
            }

            return true;
        }
    }

    return false;
}

/**
 * @brief Parse a '(X.xxx,Y.yyy)' string as returned by the JavaScript parts
 */
bool GeoIfaceHelperParseXYStringToPoint(const QString& xyString, QPoint* const point)
{
    // a point is returned as (X.xxx, Y.yyy)

    const QString myXYString = xyString.trimmed();
    bool          valid      = myXYString.startsWith(QLatin1Char('(')) && myXYString.endsWith(QLatin1Char(')'));
    QStringList   pointStrings;

    if (valid)
    {
        pointStrings = myXYString.mid(1, myXYString.length()-2).split(QLatin1Char(','));
        valid        = ( pointStrings.size() == 2 );
    }

    if (valid)
    {
        int ptX = 0;
        int ptY = 0;

        // We do not actually care about the float part, only about the integer part
        // but we have to parse floats since this is what the data is.
        ptX = pointStrings.at(0).toFloat(&valid);

        if (valid)
        {
            ptY = pointStrings.at(1).toFloat(&valid);
        }

        if (valid)
        {
            if (point)
            {
                // This will round to 0.
                *point = QPoint(ptX, ptY);
            }

            return true;
        }
    }

    return false;
}

/**
 * @brief Parses a '((lat1, lon1), (lat2, lon2))' bounds string as returned by the JavaScript parts
 */
bool GeoIfaceHelperParseBoundsString(const QString& boundsString,
                                     QPair<GeoCoordinates, GeoCoordinates>* const boundsCoordinates)
{
    // bounds are returned as ((lat1, lon1), (lat2, lon2))

    const QString myBoundsString = boundsString.trimmed();

    // check for minimum length
    bool valid                   =  myBoundsString.size() >= 13;
    valid                       &= myBoundsString.startsWith(QLatin1Char('(')) && myBoundsString.endsWith(QLatin1Char(')'));

    if (valid)
    {
        // remove outer parentheses:
        const QString string1 = myBoundsString.mid(1, myBoundsString.length()-2).trimmed();

        // split the string at the middle comma:
        const int dumpComma   = string1.indexOf(QLatin1String(","), 0);
        const int splitComma  = string1.indexOf(QLatin1String(","), dumpComma+1);
        valid                 = (dumpComma >= 0) && (splitComma >= 0);

        if (valid)
        {
            const QString coord1String  = string1.mid(0, splitComma).trimmed();
            const QString coord2String  = string1.mid(splitComma + 1).trimmed();
            valid                      &= coord1String.startsWith(QLatin1Char('(')) && coord1String.endsWith(QLatin1Char(')'));
            valid                      &= coord2String.startsWith(QLatin1Char('(')) && coord2String.endsWith(QLatin1Char(')'));

            GeoCoordinates coord1, coord2;

            if (valid)
            {
                valid = GeoIfaceHelperParseLatLonString(coord1String.mid(1, coord1String.length()-2), &coord1);
            }

            if (valid)
            {
                valid = GeoIfaceHelperParseLatLonString(coord2String.mid(1, coord2String.length()-2), &coord2);
            }

            if (valid && boundsCoordinates)
            {
                *boundsCoordinates = QPair<GeoCoordinates, GeoCoordinates>(coord1, coord2);
            }

            return valid;
        }
    }

    return false;
}

/**
 * @brief Split bounds crossing the dateline into parts which do not cross the dateline
 */
GeoCoordinates::PairList GeoIfaceHelperNormalizeBounds(const GeoCoordinates::Pair& boundsPair)
{
    GeoCoordinates::PairList boundsList;

    const qreal bWest  = boundsPair.first.lon();
    const qreal bEast  = boundsPair.second.lon();
    const qreal bNorth = boundsPair.second.lat();
    const qreal bSouth = boundsPair.first.lat();
//     qCDebug(DIGIKAM_GEOIFACE_LOG) << bWest << bEast << bNorth << bSouth;

    if (bEast<bWest)
    {
        boundsList << GeoCoordinates::makePair(bSouth, -180, bNorth, bEast);
        boundsList << GeoCoordinates::makePair(bSouth, bWest, bNorth, 180);
    }
    else
    {
        boundsList << GeoCoordinates::makePair(bSouth, bWest, bNorth, bEast);
    }

//     qCDebug(DIGIKAM_GEOIFACE_LOG) << boundsList;
    return boundsList;
}

void GeoIfaceGlobalObject::removeMyInternalWidgetFromPool(const MapBackend* const mapBackend)
{
    for (int i = 0 ; i < d->internalMapWidgetsPool.count() ; ++i)
    {
        if (d->internalMapWidgetsPool.at(i).currentOwner == static_cast<const QObject* const>(mapBackend))
        {
            d->internalMapWidgetsPool.takeAt(i);
            break;
        }
    }
}

bool GeoIfaceGlobalObject::getInternalWidgetFromPool(const MapBackend* const mapBackend,
                                                     GeoIfaceInternalWidgetInfo* const targetInfo)
{
    const QString requestingBackendName = mapBackend->backendName();

    // try to find an available widget:
    int bestDockedWidget                = -1;
    int bestUndockedWidget              = -1;
    int bestReleasedWidget              = -1;

    for (int i = 0 ; i < d->internalMapWidgetsPool.count() ; ++i)
    {
        const GeoIfaceInternalWidgetInfo& info = d->internalMapWidgetsPool.at(i);

        if (info.backendName!=requestingBackendName)
        {
            continue;
        }

        if ((info.state.testFlag(GeoIfaceInternalWidgetInfo::InternalWidgetReleased)&&(bestReleasedWidget<0)))
        {
            bestReleasedWidget = i;
            break;
        }

        if ((info.state.testFlag(GeoIfaceInternalWidgetInfo::InternalWidgetUndocked)&&(bestUndockedWidget<0)))
        {
            bestUndockedWidget = i;
        }

        if ((info.state.testFlag(GeoIfaceInternalWidgetInfo::InternalWidgetStillDocked)&&(bestDockedWidget<0)))
        {
            bestDockedWidget = i;
        }
    }

    int widgetToUse = bestReleasedWidget;

    if ((widgetToUse < 0) && (bestUndockedWidget >= 0))
    {
        widgetToUse = bestUndockedWidget;
    }
    else
    {
        widgetToUse = bestDockedWidget;
    }

    if (widgetToUse < 0)
    {
        return false;
    }

    *targetInfo = d->internalMapWidgetsPool.takeAt(widgetToUse);

    if (targetInfo->currentOwner)
    {
        qobject_cast<MapBackend*>(targetInfo->currentOwner.data())->releaseWidget(targetInfo);
    }

    return true;
}

void GeoIfaceGlobalObject::addMyInternalWidgetToPool(const GeoIfaceInternalWidgetInfo& info)
{
    d->internalMapWidgetsPool.append(info);
}

void GeoIfaceGlobalObject::updatePooledWidgetState(const QWidget* const widget,
                                                   const GeoIfaceInternalWidgetInfo::InternalWidgetState newState)
{
    for (int i = 0 ; i < d->internalMapWidgetsPool.count() ; ++i)
    {
        if (d->internalMapWidgetsPool.at(i).widget == widget)
        {
            GeoIfaceInternalWidgetInfo& info = d->internalMapWidgetsPool[i];
            info.state                       = newState;

            if (newState == GeoIfaceInternalWidgetInfo::InternalWidgetReleased)
            {
                info.currentOwner = 0;
            }

            break;
        }
    }
}

void GeoIfaceGlobalObject::clearWidgetPool()
{
    while (!d->internalMapWidgetsPool.isEmpty())
    {
        GeoIfaceInternalWidgetInfo info = d->internalMapWidgetsPool.takeLast();
        qCDebug(DIGIKAM_GEOIFACE_LOG) << info.backendName << info.deleteFunction;

        if (info.deleteFunction)
        {
            info.deleteFunction(&info);
        }
    }
}

// ---------------------------------------------------

void GeoIface_assert(const char* const condition, const char* const filename, const int lineNumber)
{
    const QString debugString = QString::fromLatin1("ASSERT: %1 - %2:%3")
        .arg(QLatin1String( condition ))
        .arg(QLatin1String( filename ))
        .arg(lineNumber);

    qCDebug(DIGIKAM_GEOIFACE_LOG) << debugString;
}

} // namespace Digikam
