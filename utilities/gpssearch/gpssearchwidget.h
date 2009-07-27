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

#ifndef GPSSEARCHWIDGET_H
#define GPSSEARCHWIDGET_H

// Qt includes

#include <QList>

// Local includes

#include "worldmapwidget.h"

namespace Digikam
{

class GPSSearchWidgetPriv;

class GPSSearchWidget : public WorldMapWidget
{
    Q_OBJECT

public:

    GPSSearchWidget(QWidget *parent=0);
    ~GPSSearchWidget();

    bool asSelection() const;

    QList<double> selectionCoordinates() const;
    void setSelectionCoordinates(const QList<double>&);

Q_SIGNALS:

    void signalNewSelectionFromMap();
    void signalSetNewMapSelection(const QList<double>&);

private Q_SLOTS:

    void slotNewSelectionFromMap(const QList<double>&);

private:

    GPSSearchWidgetPriv* const d;
};

}  // namespace Digikam

#endif // GPSSEARCHWIDGET_H
