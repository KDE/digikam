/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : world map widget library
 *
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

// Qt includes

#include <QWidget>
#include <QStringList>

// local includes

#include "geoiface_types.h"
#include "geocoordinates.h"
#include "groupstate.h"
#include "digikam_export.h"

class QAction;
class KConfigGroup;
class QDragEnterEvent;
class QDropEvent;
class QMenu;

namespace GeoIface
{

class AbstractMarkerTiler;
class DragDropHandler;
class GeoIfaceSharedData;
class ModelHelper;
class TrackManager;

class DIGIKAM_EXPORT MapWidget : public QWidget
{
  Q_OBJECT

public:

    explicit MapWidget(QWidget* const parent = 0);
    ~MapWidget();

    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);

    /// @name Data
    //@{
    void addUngroupedModel(ModelHelper* const modelHelper);
    void removeUngroupedModel(ModelHelper* const modelHelper);
    void setGroupedModel(AbstractMarkerTiler* const markerModel);
    void setDragDropHandler(DragDropHandler* const dragDropHandler);
    void setTrackManager(TrackManager* const trackManager);
    //@}

    /// @name UI setup
    //@{
    QAction* getControlAction(const QString& actionName);
    QWidget* getControlWidget();
    void addWidgetToControlWidget(QWidget* const newWidget);
    void setSortOptionsMenu(QMenu* const sortMenu);
    void setMouseMode(const MouseModes mouseMode);
    void setAvailableMouseModes(const MouseModes mouseModes);
    void setVisibleMouseModes(const MouseModes mouseModes);
    void setAllowModifications(const bool state);
    void setActive(const bool state);
    bool getActiveState();
    bool getStickyModeState() const;
    void setStickyModeState(const bool state);
    void setVisibleExtraActions(const ExtraActions actions);
    void setEnabledExtraActions(const ExtraActions actions);
    //@}

    /// @name Map related functions
    //@{
    QStringList availableBackends() const;
    bool setBackend(const QString& backendName);

    GeoCoordinates getCenter() const;
    void setCenter(const GeoCoordinates& coordinate);

    void setZoom(const QString& newZoom);
    QString getZoom();

    void adjustBoundariesToGroupedMarkers(const bool useSaneZoomLevel = true);
    void refreshMap();
    //@}

    /// @name Appearance
    //@{
    void setSortKey(const int sortKey);
    void setThumnailSize(const int newThumbnailSize);
    void setThumbnailGroupingRadius(const int newGroupingRadius);
    void setMarkerGroupingRadius(const int newGroupingRadius);
    int  getThumbnailSize() const;
    int  getUndecoratedThumbnailSize() const;
    void setShowThumbnails(const bool state);
    //@}

    /// @name Region selection
    //@{
    void setRegionSelection(const GeoCoordinates::Pair& region);
    GeoCoordinates::Pair getRegionSelection();
    void clearRegionSelection();
    //@}

    /**
     * @name Internal
     * Functions that are only used internally and should be hidden from the public interface
     */
    //@{
    void updateMarkers();
    void updateClusters();
    void markClustersAsDirty();

    void getColorInfos(const int clusterIndex, QColor* fillColor, QColor* strokeColor,
                       Qt::PenStyle* strokeStyle, QString* labelText, QColor* labelColor,
                       const GroupState* const overrideSelection = 0,
                       const int* const overrideCount = 0) const;

    void getColorInfos(const GroupState groupState,
                       const int nMarkers,
                       QColor* fillColor, QColor* strokeColor,
                       Qt::PenStyle* strokeStyle, QString* labelText, QColor* labelColor) const;

    QString convertZoomToBackendZoom(const QString& someZoom, const QString& targetBackend) const;
    QPixmap getDecoratedPixmapForCluster(const int clusterId, const GroupState* const selectedStateOverride, const int* const countOverride, QPoint* const centerPoint);
    QVariant getClusterRepresentativeMarker(const int clusterIndex, const int sortKey);
    //@}

public Q_SLOTS:

    /// @name Appearance
    //@{
    void slotZoomIn();
    void slotZoomOut();
    void slotDecreaseThumbnailSize();
    void slotIncreaseThumbnailSize();
    //@}

    /// @name Internal?
    //@{
    void slotUpdateActionsEnabled();
    void slotClustersNeedUpdating();
    void stopThumbnailTimer();
    void slotStickyModeChanged();
    //@}

Q_SIGNALS:

    void signalUngroupedModelChanged(const int index);
    void signalRegionSelectionChanged();
    void signalRemoveCurrentFilter();
    void signalStickyModeChanged();
    void signalMouseModeChanged(const GeoIface::MouseModes& currentMouseMode);

public:

    /** Return a string version of LibMarbleWidget release in format "major.minor.patch"
     */
    static QString MarbleWidgetVersion();

protected:

    bool currentBackendReady() const;
    void applyCacheToBackend();
    void saveBackendToCache();
    void rebuildConfigurationMenu();
    void dropEvent(QDropEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void createActions();
    void createActionsForBackendSelection();
    void setShowPlaceholderWidget(const bool state);
    void setMapWidgetInFrame(QWidget* const widgetForFrame);
    void removeMapWidgetFromFrame();

protected Q_SLOTS:

    void slotBackendReadyChanged(const QString& backendName);
    void slotChangeBackend(QAction* action);
    void slotBackendZoomChanged(const QString& newZoom);
    void slotClustersMoved(const QIntList& clusterIndices, const QPair<int, QModelIndex>& snapTarget);
    void slotClustersClicked(const QIntList& clusterIndices);
    void slotShowThumbnailsChanged();
    void slotRequestLazyReclustering();
    void slotLazyReclusteringRequestCallBack();
    void slotItemDisplaySettingsChanged();
    void slotUngroupedModelChanged();
    void slotNewSelectionFromMap(const GeoIface::GeoCoordinates::Pair& sel);

    /// @name Mouse modes
    //@{
    void slotMouseModeChanged(QAction* triggeredAction);
    void slotRemoveCurrentRegionSelection();
    //@}

private:

    const QExplicitlySharedDataPointer<GeoIfaceSharedData> s;

    class Private;
    Private* const d;

    Q_DISABLE_COPY(MapWidget)
};

} // namespace GeoIface

#endif // MAPWIDGET_H
