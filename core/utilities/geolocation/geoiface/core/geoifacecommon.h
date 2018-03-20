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
 * Copyright (C)      2014 by Justus Schwartz <justus at gmx dot li>
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

#ifndef GEO_IFACE_COMMON_H
#define GEO_IFACE_COMMON_H

// Qt includes

#include <QPoint>
#include <QPointer>
#include <QSharedData>
#include <QSize>
#include <QWidget>
#include <QPixmap>

// Local includes

#include "geoifacetypes.h"
#include "tileindex.h"
#include "groupstatecomputer.h"
#include "digikam_export.h"

namespace Digikam
{
class MapWidget;
class AbstractMarkerTiler;
class TileGrouper;
class TrackManager;
class MapBackend;
class GeoModelHelper;

/**
 * @brief Class to hold information about map widgets stored in the GeoIfaceGlobalObject
 *
 * @todo The list of these info structures has to be cleaned up periodically
 */
class DIGIKAM_EXPORT GeoIfaceInternalWidgetInfo
{
public:

    typedef void (*DeleteFunction)(GeoIfaceInternalWidgetInfo* const info);

    enum InternalWidgetState
    {
        InternalWidgetReleased    = 1,
        InternalWidgetUndocked    = 2,
        InternalWidgetStillDocked = 4
    };

    Q_DECLARE_FLAGS(InternalWidgetStates, InternalWidgetState)

    GeoIfaceInternalWidgetInfo()
        : state(),
          widget(),
          backendData(),
          backendName(),
          currentOwner(0),
          deleteFunction(0)
    {
    }

public:

    InternalWidgetStates state;
    QPointer<QWidget>    widget;
    QVariant             backendData;
    QString              backendName;
    QPointer<QObject>    currentOwner;
    DeleteFunction       deleteFunction;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(GeoIfaceInternalWidgetInfo::InternalWidgetStates)

// ----------------------------------------------------------------------------------------------

/**
 * @brief Global object for geolocation interface to hold items common to all
 * geolocation interface Widget instances
 */
class DIGIKAM_EXPORT GeoIfaceGlobalObject : public QObject
{
    Q_OBJECT

public:

    static GeoIfaceGlobalObject* instance();

    /// @name Shared pixmaps
    //@{
    QPixmap getMarkerPixmap(const QString& pixmapId);
    QPixmap getStandardMarkerPixmap();
    QUrl    locateDataFile(const QString& filename);
    //@}

    /// @name Shared internal map widgets
    //@{
    void removeMyInternalWidgetFromPool(const MapBackend* const mapBackend);
    bool getInternalWidgetFromPool(const MapBackend* const mapBackend, GeoIfaceInternalWidgetInfo* const targetInfo);
    void addMyInternalWidgetToPool(const GeoIfaceInternalWidgetInfo& info);
    void updatePooledWidgetState(const QWidget* const widget, const GeoIfaceInternalWidgetInfo::InternalWidgetState newState);
    void clearWidgetPool();
    //@}

private:

    GeoIfaceGlobalObject();
    ~GeoIfaceGlobalObject();

    class Private;
    Private* const d;

    Q_DISABLE_COPY(GeoIfaceGlobalObject)

    friend class GeoIfaceGlobalObjectCreator;
};

// ----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT GeoIfaceCluster
{

public:

    enum PixmapType
    {
        PixmapMarker,
        PixmapCircle,
        PixmapImage
    } pixmapType;

    typedef QList<GeoIfaceCluster> List;

public:

    GeoIfaceCluster()
        : pixmapType(PixmapMarker),
          tileIndicesList(),
          markerCount(0),
          markerSelectedCount(0),
          coordinates(),
          pixelPos(),
          groupState(SelectedNone),
          representativeMarkers(),
          pixmapSize(),
          pixmapOffset()
    {
    }

    QList<TileIndex>    tileIndicesList;
    int                 markerCount;
    int                 markerSelectedCount;
    GeoCoordinates      coordinates;
    QPoint              pixelPos;
    GeoGroupState          groupState;
    QMap<int, QVariant> representativeMarkers;

    QSize               pixmapSize;

    //! anchor point of the image, measured from bottom-left
    QPoint              pixmapOffset;
};

// ----------------------------------------------------------------------------------------------

/// @todo Move these somewhere else
const int GeoIfaceMinMarkerGroupingRadius    = 1;
const int GeoIfaceMinThumbnailGroupingRadius = 15;
const int GeoIfaceMinThumbnailSize           = GeoIfaceMinThumbnailGroupingRadius * 2;

// ----------------------------------------------------------------------------------------------

/**
 * @brief Helper function, returns the square of the distance between two points
 * @todo Move this function somewhere else
 *
 * @param a Point a
 * @param b Point b
 * @return Square of the distance between a and b
 */
DIGIKAM_EXPORT inline int QPointSquareDistance(const QPoint& a, const QPoint& b)
{
    return (a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y());
}

// ----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT GeoIfaceSharedData : public QSharedData
{
public:

    GeoIfaceSharedData()
        : QSharedData(),
          worldMapWidget(0),
          tileGrouper(0),
          markerModel(0),
          clusterList(),
          trackManager(0),
          showThumbnails(true),
          thumbnailSize(GeoIfaceMinThumbnailSize),
          thumbnailGroupingRadius(GeoIfaceMinThumbnailGroupingRadius),
          markerGroupingRadius(GeoIfaceMinMarkerGroupingRadius),
          previewSingleItems(true),
          previewGroupedItems(true),
          showNumbersOnItems(true),
          sortKey(0),
          modificationsAllowed(true),
          selectionRectangle(),
          haveMovingCluster(false),
          currentMouseMode(0),
          availableMouseModes(0),
          visibleMouseModes(0),
          activeState(false)
    {
    }

    /// @todo De-inline?
    bool hasRegionSelection() const
    {
        return selectionRectangle.first.hasCoordinates();
    }

public:

    /// @name Objects
    //@{
    MapWidget*                worldMapWidget;
    TileGrouper*              tileGrouper;
    AbstractMarkerTiler*      markerModel;
    GeoIfaceCluster::List     clusterList;
    QList<GeoModelHelper*>    ungroupedModels;
    TrackManager*             trackManager;
    //@}

    /// @name Display options
    //@{
    bool                      showThumbnails;
    int                       thumbnailSize;
    int                       thumbnailGroupingRadius;
    int                       markerGroupingRadius;
    bool                      previewSingleItems;
    bool                      previewGroupedItems;
    bool                      showNumbersOnItems;
    int                       sortKey;
    bool                      modificationsAllowed;
    //@}

    /// @name Current map state
    //@{
    GeoCoordinates::Pair      selectionRectangle;
    bool                      haveMovingCluster;
    GeoMouseModes             currentMouseMode;
    GeoMouseModes             availableMouseModes;
    GeoMouseModes             visibleMouseModes;
    bool                      activeState;
    //@}
};

// ----------------------------------------------------------------------------------------------

// helper functions:

DIGIKAM_EXPORT bool GeoIfaceHelperParseLatLonString(const QString& latLonString,
                                                    GeoCoordinates* const coordinates);

DIGIKAM_EXPORT bool GeoIfaceHelperParseXYStringToPoint(const QString& xyString,
                                                       QPoint* const point);

DIGIKAM_EXPORT bool GeoIfaceHelperParseBoundsString(const QString& boundsString,
                                                    QPair<GeoCoordinates, GeoCoordinates>* const boundsCoordinates);

DIGIKAM_EXPORT GeoCoordinates::PairList GeoIfaceHelperNormalizeBounds(const GeoCoordinates::Pair& boundsPair);

DIGIKAM_EXPORT void GeoIface_assert(const char* const condition,
                                    const char* const filename,
                                    const int lineNumber);

} // namespace Digikam

#define GEOIFACE_ASSERT(cond) ((!(cond)) ? Digikam::GeoIface_assert(#cond,__FILE__,__LINE__) : qt_noop())

#endif // GEO_IFACE_COMMON_H
