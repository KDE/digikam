/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Google-Maps-backend for geolocation interface
 *
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_BACKEND_GOOGLE_MAPS_H
#define DIGIKAM_BACKEND_GOOGLE_MAPS_H

// Local includes

#include "mapbackend.h"
#include "trackmanager.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT BackendGoogleMaps : public MapBackend
{
    Q_OBJECT

public:

    explicit BackendGoogleMaps(const QExplicitlySharedDataPointer<GeoIfaceSharedData>& sharedData,
                               QObject* const parent = nullptr);
    virtual ~BackendGoogleMaps();

    virtual QString backendName() const override;
    virtual QString backendHumanName() const override;
    virtual QWidget* mapWidget() override;
    virtual void releaseWidget(GeoIfaceInternalWidgetInfo* const info) override;
    virtual void mapWidgetDocked(const bool state) override;

    virtual GeoCoordinates getCenter() const override;
    virtual void setCenter(const GeoCoordinates& coordinate) override;

    virtual bool isReady() const override;

    virtual void zoomIn() override;
    virtual void zoomOut() override;

    virtual void saveSettingsToGroup(KConfigGroup* const group) override;
    virtual void readSettingsFromGroup(const KConfigGroup* const group) override;

    virtual void addActionsToConfigurationMenu(QMenu* const configurationMenu) override;

    virtual void updateMarkers() override;
    virtual void updateClusters() override;

    virtual bool screenCoordinates(const GeoCoordinates& coordinates, QPoint* const point) override;
    virtual bool geoCoordinates(const QPoint& point, GeoCoordinates* const coordinates) const override;
    virtual QSize mapSize() const override;

    virtual void setZoom(const QString& newZoom) override;
    virtual QString getZoom() const override;

    virtual int getMarkerModelLevel() override;
    virtual GeoCoordinates::PairList getNormalizedBounds() override;

//     virtual void updateDragDropMarker(const QPoint& pos, const GeoIfaceDragData* const dragData);
//     virtual void updateDragDropMarkerPosition(const QPoint& pos);

    virtual void updateActionAvailability() override;

    QString getMapType() const;
    void setMapType(const QString& newMapType);
    void setShowMapTypeControl(const bool state);
    void setShowScaleControl(const bool state);
    void setShowNavigationControl(const bool state);

    virtual void regionSelectionChanged() override;
    virtual void mouseModeChanged() override;

    virtual void centerOn(const Marble::GeoDataLatLonBox& latLonBox, const bool useSaneZoomLevel) override;
    virtual void setActive(const bool state) override;

public Q_SLOTS:

    virtual void slotClustersNeedUpdating() override;
    virtual void slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap) override;
    void slotUngroupedModelChanged(const int mindex);

protected:

    bool eventFilter(QObject* object, QEvent* event) override;
    void createActions();
    void setClusterPixmap(const int clusterId, const QPoint& centerPoint, const QPixmap& clusterPixmap);
    void setMarkerPixmap(const int modelId, const int markerId, const QPoint& centerPoint, const QPixmap& markerPixmap);
    void setMarkerPixmap(const int modelId, const int markerId, const QPoint& centerPoint, const QSize& iconSize, const QUrl& iconUrl);
    void storeTrackChanges(const TrackManager::TrackChanges trackChanges);

private Q_SLOTS:

    void slotHTMLInitialized();
    void slotSetCenterTimer();
    void slotMapTypeActionTriggered(QAction* action);
    void slotHTMLEvents(const QStringList& eventStrings);
    void slotFloatSettingsTriggered(QAction* action);
    void slotSelectionHasBeenMade(const Digikam::GeoCoordinates::Pair& searchCoordinates);
    void slotTrackManagerChanged() override;
    void slotTracksChanged(const QList<TrackManager::TrackChanges> trackChanges);
    void slotTrackVisibilityChanged(const bool newState);

private:

    void updateZoomMinMaxCache();
    static void deleteInfoFunction(GeoIfaceInternalWidgetInfo* const info);
    void addPointsToTrack(const quint64 trackId, TrackManager::TrackPoint::List const& track, const int firstPoint, const int nPoints);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_BACKEND_GOOGLE_MAPS_H
