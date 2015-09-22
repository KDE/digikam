/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-09-18
 * @brief  A tool to calibrate the tiling levels used in libkgeomap
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
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

#ifndef CALIBRATOR_H
#define CALIBRATOR_H

// Qt includes

#include <QMainWindow>

// local includes

#include "modelhelper.h"

class QStandardItemModel;

class CalibratorModelHelper : public KGeoMap::ModelHelper
{
    Q_OBJECT

public:

    explicit CalibratorModelHelper(QStandardItemModel* const model, QObject* const parent = 0);
    ~CalibratorModelHelper();

    void setItemCoordinates(const QModelIndex& index, const KGeoMap::GeoCoordinates& coordinates);

    // these are necessary for grouped and ungrouped models
    virtual QAbstractItemModel*  model()          const;
    virtual QItemSelectionModel* selectionModel() const;
    virtual Flags                modelFlags()     const;
    virtual bool itemCoordinates(const QModelIndex& index, KGeoMap::GeoCoordinates* const coordinates) const;

private:

    class Private;
    Private* const d;

    Q_DISABLE_COPY(CalibratorModelHelper)
};

// -----------------------------------------------------------------------------------------------------------

class Calibrator : public QMainWindow
{
    Q_OBJECT

public:

    Calibrator();
    ~Calibrator();

private:

    void addMarkerAt(const KGeoMap::GeoCoordinates& coordinates);

private Q_SLOTS:

    void updateGroupingMode();
    void updateMarkers();
    void updateZoomView();
    void slotAddMapWidget();
    void slotRemoveMapWidget();
    void slotActivateMapActionTriggered(bool state);

private:

    class Private;
    Private* const d;

    Q_DISABLE_COPY(Calibrator)
};

#endif /* CALIBRATOR_H */
