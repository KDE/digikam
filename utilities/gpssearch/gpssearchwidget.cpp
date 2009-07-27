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

#include "gpssearchwidget.h"
#include "gpssearchwidget.moc"

// KDE includes

#include <klocale.h>
#include <kdebug.h>

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
#endif // HAVE_MARBLEWIDGET

    // TODO: connect marble with new selection to perform on the map when user select virtual album.
}

GPSSearchWidget::~GPSSearchWidget()
{
    delete d;
}

bool GPSSearchWidget::asSelection() const
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
    kDebug(50003) << "Set new selection area: " << d->selection;

    // Set selection area in marble widget.
    emit signalSetNewMapSelection(d->selection);
}

void GPSSearchWidget::slotNewSelectionFromMap(const QList<double>& sel)
{
    d->selection = sel;
    emit signalNewSelectionFromMap();
}

}  // namespace Digikam
