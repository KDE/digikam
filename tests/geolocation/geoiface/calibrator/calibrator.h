/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-18
 * Description : A tool to calibrate the tiling levels
 *               used in geolocation interface
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef CALIBRATOR_H
#define CALIBRATOR_H

// Qt includes

#include <QMainWindow>

// Local includes

#include "geomodelhelper.h"

class QStandardItemModel;

using namespace Digikam;

class CalibratorModelHelper : public GeoModelHelper
{
    Q_OBJECT

public:

    explicit CalibratorModelHelper(QStandardItemModel* const model, QObject* const parent = 0);
    ~CalibratorModelHelper();

    void setItemCoordinates(const QModelIndex& index, const GeoCoordinates& coordinates);

    // these are necessary for grouped and ungrouped models
    virtual QAbstractItemModel*  model()          const;
    virtual QItemSelectionModel* selectionModel() const;
    virtual PropertyFlags        modelFlags()     const;
    virtual bool itemCoordinates(const QModelIndex& index, GeoCoordinates* const coordinates) const;

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

    explicit Calibrator();
    ~Calibrator();

private:

    void addMarkerAt(const GeoCoordinates& coordinates);

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

#endif // CALIBRATOR_H
