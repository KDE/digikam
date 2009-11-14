/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-30
 * Description : a widget to search image over a map.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// #include "gpssearchwidget.h"
#include "gpssearchwidget.moc"

// KDE includes

#include <klocale.h>
#include <kdebug.h>

// Marble includes
#ifdef HAVE_MARBLEWIDGET
#include <marble/GeoDataLinearRing.h>
#include <marble/GeoPainter.h>
#endif // HAVE_MARBLEWIDGET

// Local includes.

#include "config-digikam.h"

namespace Digikam
{

class GPSSearchWidgetPriv
{
public:

    GPSSearchWidgetPriv(){}

    QList<double> selection;
};

GPSSearchWidget::GPSSearchWidget(QWidget *parent)
               : WorldMapWidget(256, 256, parent), d(new GPSSearchWidgetPriv)
{
    // NOTE: see B.K.O #153070
    // Marble will include selection area over map canvas with KDE 4.2
    // using CTRL + right mouse button.
    // A new signal named regionSelected() will be emitted when user select an aera.


#ifdef HAVE_MARBLEWIDGET
    connect(marbleWidget(), SIGNAL(regionSelected(const QList<double>&)),
            this, SLOT(slotNewSelectionFromMap(const QList<double>&)));
    setCustomPaintFunction(markerClusterHolderCustomPaint, this);
#endif // HAVE_MARBLEWIDGET

    // allow the user to select and filter items on the map:
    slotSetAllowItemFiltering(true);
    slotSetAllowItemSelection(true);
    slotSetFocusOnAddedItems(false);

    // TODO: connect marble with new selection to perform on the map when the user selects a virtual album.
}

GPSSearchWidget::~GPSSearchWidget()
{
    delete d;
}

bool GPSSearchWidget::hasSelection() const
{
    return !d->selection.isEmpty();
}

QList<double> GPSSearchWidget::selectionCoordinates() const
{
    return d->selection;
}

void GPSSearchWidget::setSelectionCoordinates(const QList<double>& sel)
{
    d->selection = sel;
    kDebug() << "Set new selection area: West, North, East, South: " << d->selection;

    // Set selection area in marble widget.
    emit signalSetNewMapSelection(d->selection);
}

void GPSSearchWidget::slotNewSelectionFromMap(const QList<double>& sel)
{
    d->selection = sel;
    emit signalNewSelectionFromMap();
}

#ifdef HAVE_MARBLEWIDGET
/**
 * @brief Callback that draws the selection for the current search on the map
 * @param geoPainter Painter on which to draw on
 * @param isBefore Whether this call is before the clusters are drawn
 * @param yourdata Pointer to GPSSearchWidget
 */
void GPSSearchWidget::markerClusterHolderCustomPaint(Marble::GeoPainter* const geoPainter, const bool isBefore, void* const yourdata)
{
    if (isBefore)
    {
      GPSSearchWidget* const me = reinterpret_cast<GPSSearchWidget*>(yourdata);
      if (me->d->selection.isEmpty())
        return;

      // prepare drawing of a polygon (because drawRect does not take four geo-corners...)
      // West, North, East, South
      const qreal lonWest = me->d->selection.at(0);
      const qreal latNorth = me->d->selection.at(1);
      const qreal lonEast = me->d->selection.at(2);
      const qreal latSouth = me->d->selection.at(3);

      // TODO: once support for Marble<0.8 is dropped, mark these variables as const
      Marble::GeoDataCoordinates coordTopLeft(lonWest, latNorth, 0, Marble::GeoDataCoordinates::Degree);
      Marble::GeoDataCoordinates coordTopRight(lonEast, latNorth, 0, Marble::GeoDataCoordinates::Degree);
      Marble::GeoDataCoordinates coordBottomLeft(lonWest, latSouth, 0, Marble::GeoDataCoordinates::Degree);
      Marble::GeoDataCoordinates coordBottomRight(lonEast, latSouth, 0, Marble::GeoDataCoordinates::Degree);
      Marble::GeoDataLinearRing polyRing;
#if MARBLE_VERSION < 0x000800
      polyRing.append(&coordTopLeft);
      polyRing.append(&coordTopRight);
      polyRing.append(&coordBottomRight);
      polyRing.append(&coordBottomLeft);
#else // MARBLE_VERSION < 0x000800
      polyRing << coordTopLeft << coordTopRight << coordBottomRight << coordBottomLeft;
#endif // MARBLE_VERSION < 0x000800

      geoPainter->save();

      // paint the selection:
      QPen selectionPen;
      selectionPen.setColor(Qt::blue);
      selectionPen.setStyle(Qt::SolidLine);
      selectionPen.setWidth(1);
      geoPainter->setPen(selectionPen);
      geoPainter->setBrush(Qt::NoBrush);
      geoPainter->drawPolygon(polyRing);

      geoPainter->restore();
    }
}
#endif // HAVE_MARBLEWIDGET

}  // namespace Digikam
