/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-01-16
 * @brief  test for the model holding markers
 *
 * @author Copyright (C) 2010, 2011 by Michael G. Hansen
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

#ifndef TEST_ITEMMARKERTILER_H
#define TEST_ITEMMARKERTILER_H

// Qt includes

#include <QtTest/QtTest>

// local includes

#include "itemmarkertiler.h"
#include "modelhelper.h"

class MarkerModelHelper : public KGeoMap::ModelHelper
{
Q_OBJECT

public:

    MarkerModelHelper(QAbstractItemModel* const itemModel, QItemSelectionModel* const itemSelectionModel);
    ~MarkerModelHelper();

    virtual QAbstractItemModel*  model()          const;
    virtual QItemSelectionModel* selectionModel() const;
    virtual bool itemCoordinates(const QModelIndex& index, KGeoMap::GeoCoordinates* const coordinates) const;

private Q_SLOTS:

    void slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

private:

    QAbstractItemModel* const  m_itemModel;
    QItemSelectionModel* const m_itemSelectionModel;
};

// --------------------------------------------------------------------------------

class TestItemMarkerTiler : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testNoOp();
    void testIndices();
    void testAddMarkers1();
    void testRemoveMarkers1();
    void testRemoveMarkers2();
    void testMoveMarkers1();
    void testMoveMarkers2();
    void testIteratorWholeWorldNoBackingModel();
    void testIteratorWholeWorld();
    void testIteratorPartial1();
    void testIteratorPartial2();
    void testPreExistingMarkers();
    void testSelectionState1();
    void benchmarkIteratorWholeWorld();
};

#endif /* TEST_ITEMMARKERTILER_H */
