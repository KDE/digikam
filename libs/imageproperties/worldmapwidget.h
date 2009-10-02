/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display GPS info on a world map
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WORLDMAPWIDGET_H
#define WORLDMAPWIDGET_H

// Qt includes

#include <QFrame>
#include <QDateTime>
#include <QList>
#include <QToolButton>

// KDE includes

#include <kurl.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"
#include <loadingdescription.h>
#include <config-digikam.h>
#ifdef HAVE_MARBLEWIDGET
#include "markerclusterholder.h"
#endif // HAVE_MARBLEWIDGET

namespace Digikam
{

class DIGIKAM_EXPORT GPSInfo
{
public:

    GPSInfo()
    : latitude(0.0),
      longitude(0.0),
      altitude(0.0),
      dateTime(),
      rating(0),
      url(),
      dimensions()
    {
    };

    double    latitude;
    double    longitude;
    double    altitude;

    QDateTime dateTime;
    int       rating;
    KUrl      url;
    QSize     dimensions;
};

typedef QList<GPSInfo> GPSInfoList;

// ------------------------------------------------------------------------------

class WorldMapWidgetPriv;

class DIGIKAM_EXPORT WorldMapWidget : public QFrame
{
    Q_OBJECT

public:

    enum MapTheme
    {
        AtlasMap = 0,
        OpenStreetMap
    };

public:

    WorldMapWidget(int w, int h, QWidget *parent);
    virtual ~WorldMapWidget();

    void   setGPSPositions(const GPSInfoList& list);
    void   addGPSPositions(const GPSInfoList& list);
    void   clearGPSPositions();

    double getLatitude();
    double getLongitude();

    void   getCenterPosition(double& lat, double& lng);
    void   setCenterPosition(double lat, double lng);

    int    getZoomLevel();
    void   setZoomLevel(int l);

    void     setMapTheme(MapTheme theme);
    MapTheme getMapTheme();

    void readConfig(KConfigGroup& group);
    void writeConfig(KConfigGroup& group);
#ifdef HAVE_MARBLEWIDGET
    void setCustomPaintFunction(const MarkerClusterHolder::CustomPaintFunction customPaintFunction, void* const yourdata);
#endif // HAVE_MARBLEWIDGET
    void setMultiMarkerSettings(const bool showSingleImages, const bool showGroupImages, const bool showHighestRatingFirst, const bool showOldestFirst, const bool showNumbers);
    void getMultiMarkerSettings(bool* const showSingleImages, bool* const showGroupImages, bool* const showHighestRatingFirst, bool* const showOldestFirst, bool* const showNumbers) const;
    
Q_SIGNALS:

    void signalSettingsChanged();
    void signalSelectedItems(const GPSInfoList infoList);
    void signalSoloItems(const GPSInfoList infoList);

public Q_SLOTS:

    void slotZoomIn();
    void slotZoomOut();
    void slotSetSelectedImages(const GPSInfoList &infoList);
    void slotMapMarkerSelectionChanged();
    void slotMapMarkerSoloChanged();
    void slotSetAllowItemSelection(const bool allow);
    void slotSetAllowItemFiltering(const bool allow);
    void slotSetFocusOnAddedItems(const bool doIt);
    void slotSetEnableTooltips(const bool doIt);
    void slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& pix);
#ifdef HAVE_MARBLEWIDGET
    MarkerClusterHolder* getMarkerClusterHolder() const;
#endif // HAVE_MARBLEWIDGET

protected:

    QWidget* marbleWidget() const;

private:

#ifdef HAVE_MARBLEWIDGET
    static MarkerClusterHolder::PixmapOperations getClusterPixmap(const int clusterIndex, MarkerClusterHolder* const mch, const QSize& maxSize, void* const yourdata, QPixmap* const clusterPixmap);
#endif // HAVE_MARBLEWIDGET

private:

    WorldMapWidgetPriv* const d;
};

// ------------------------------------------------------------------------------

class WorldMapThemeBtnPriv;

class DIGIKAM_EXPORT WorldMapThemeBtn : public QToolButton
{
    Q_OBJECT

public:

    WorldMapThemeBtn(WorldMapWidget *map, QWidget *parent);
    virtual ~WorldMapThemeBtn();

private Q_SLOTS:

    void slotUpdateMenu();
    void slotMapThemeChanged(QAction*);

private:

    WorldMapThemeBtnPriv* const d;
    
};

}  // namespace Digikam

#endif /* WORLDMAPWIDGET_H */
