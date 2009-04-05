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
#include <QDomDocument>
#include <QList>

// KDE includes

#include <kurl.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class GPSInfo
{
public:

    GPSInfo()
    {
        latitude  = 0.0;
        longitude = 0.0;
        altitude  = 0.0;
    };

    double    latitude;
    double    longitude;
    double    altitude;

    QDateTime dateTime;

    KUrl      url;
};

typedef QList<GPSInfo> GPSInfoList;

// ------------------------------------------------------------------------------

class WorldMapWidgetPriv;

class WorldMapWidget : public QFrame
{
Q_OBJECT

public:

    WorldMapWidget(int w, int h, QWidget *parent);
    virtual ~WorldMapWidget();

    void   setGPSPositions(const GPSInfoList& list);

    double getLatitude();
    double getLongitude();

private:

    QDomElement addKmlElement(QDomDocument &kmlDocument, QDomElement &target, const QString& tag);
    QDomElement addKmlTextElement(QDomDocument &kmlDocument, QDomElement &target, const QString& tag, const QString& text);

private:

    WorldMapWidgetPriv* const d;
};

}  // namespace Digikam

#endif /* WORLDMAPWIDGET_H */
