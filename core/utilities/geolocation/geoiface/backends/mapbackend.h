/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Base-class for backends for geolocation interface
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2014      by Justus Schwartz <justus at gmx dot li>
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

#ifndef MAP_BACKEND_H
#define MAP_BACKEND_H

// Qt includes

#include <QModelIndex>

// Local includes

#include "geoifacecommon.h"

class QMenu;
class QWidget;

class KConfigGroup;

namespace Marble
{
    class GeoDataLatLonBox;
}

namespace Digikam
{

class MapBackend : public QObject
{

Q_OBJECT

public:

    MapBackend(const QExplicitlySharedDataPointer<GeoIfaceSharedData>& sharedData, QObject* const parent);
    virtual ~MapBackend();

    virtual QString backendName() const = 0;
    virtual QString backendHumanName() const = 0;
    virtual QWidget* mapWidget() = 0;
    virtual void releaseWidget(GeoIfaceInternalWidgetInfo* const info) = 0;
    virtual void mapWidgetDocked(const bool state) = 0;

    virtual GeoCoordinates getCenter() const = 0;
    virtual void setCenter(const GeoCoordinates& coordinate) = 0;

    virtual bool isReady() const = 0;

    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;

    virtual void saveSettingsToGroup(KConfigGroup* const group) = 0;
    virtual void readSettingsFromGroup(const KConfigGroup* const group) = 0;

    virtual void addActionsToConfigurationMenu(QMenu* const configurationMenu) = 0;

    virtual void updateMarkers() = 0;
    virtual void updateClusters() = 0;

    virtual bool screenCoordinates(const GeoCoordinates& coordinates, QPoint* const point) = 0;
    virtual bool geoCoordinates(const QPoint& point, GeoCoordinates* const coordinates) const = 0;
    virtual QSize mapSize() const = 0;

    virtual void setZoom(const QString& newZoom) = 0;
    virtual QString getZoom() const = 0;

    virtual int getMarkerModelLevel() = 0;
    virtual GeoCoordinates::PairList getNormalizedBounds() = 0;

//     virtual void updateDragDropMarker(const QPoint& pos, const GeoIfaceDragData* const dragData) = 0;
//     virtual void updateDragDropMarkerPosition(const QPoint& pos) = 0;

    virtual void updateActionAvailability() = 0;

    virtual void regionSelectionChanged() = 0;
    virtual void mouseModeChanged() = 0;

    const QExplicitlySharedDataPointer<GeoIfaceSharedData> s;

    virtual void centerOn(const Marble::GeoDataLatLonBox& box, const bool useSaneZoomLevel = true) = 0;
    virtual void setActive(const bool state) = 0;

public Q_SLOTS:

    virtual void slotClustersNeedUpdating() = 0;
    virtual void slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap);
    virtual void slotTrackManagerChanged();

Q_SIGNALS:

    void signalBackendReadyChanged(const QString& backendName);
    void signalClustersMoved(const QIntList& clusterIndices, const QPair<int, QModelIndex>& snapTarget);
    void signalClustersClicked(const QIntList& clusterIndices);
    void signalMarkersMoved(const QIntList& markerIndices);
    void signalZoomChanged(const QString& newZoom);
    void signalSelectionHasBeenMade(const Digikam::GeoCoordinates::Pair& coordinates);
};

} // namespace Digikam

#endif // MAP_BACKEND_H
