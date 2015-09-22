/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  OpenStreetMap-backend for Digikam
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef BACKEND_MAP_OSM_H
#define BACKEND_MAP_OSM_H

// local includes

#include "mapbackend.h"

namespace GeoIface
{

class BackendOSM : public MapBackend
{
    Q_OBJECT

public:

    explicit BackendOSM(const QExplicitlySharedDataPointer<DigikamSharedData>& sharedData, QObject* const parent = 0);
    virtual ~BackendOSM();

    virtual QString backendName()      const;
    virtual QString backendHumanName() const;
    virtual QWidget* mapWidget()       const;

    virtual GeoCoordinates getCenter() const;
    virtual void setCenter(const GeoCoordinates& coordinate);

    virtual bool isReady() const;

    virtual void zoomIn();
    virtual void zoomOut();

    virtual void saveSettingsToGroup(KConfigGroup* const group);
    virtual void readSettingsFromGroup(const KConfigGroup* const group);

    virtual void addActionsToConfigurationMenu(QMenu* const configurationMenu);

    virtual void updateMarkers();
    virtual void updateClusters();

    virtual bool screenCoordinates(const GeoCoordinates& coordinates, QPoint* const point);
    virtual bool GeoCoordinates(const QPoint& point, GeoCoordinates* const coordinates) const;
    virtual QSize mapSize() const;

    virtual void setZoom(const QString& newZoom);
    virtual QString getZoom() const;

    virtual int getMarkerModelLevel();
    virtual GeoCoordinates::PairList getNormalizedBounds();

public Q_SLOTS:

    virtual void slotClustersNeedUpdating();

private Q_SLOTS:

    void slotHTMLInitialized();
    void updateActionsEnabled();
    void slotHTMLEvents(const QStringList& eventStrings);

private:

    void loadInitialHTML();

private:

    class Private;
    Private* const d;
};

} /* namespace GeoIface */

#endif /* BACKEND_MAP_OSM_H */
