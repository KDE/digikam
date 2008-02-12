/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display GPS info on a world map
 * 
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QPainter>
#include <QString>
#include <QPixmap>
#include <QLabel>
#include <QFrame>

// KDE includes.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "worldmapwidget.h"
#include "worldmapwidget.moc"

namespace Digikam
{

class WorldMapWidgetPriv
{

public:

    WorldMapWidgetPriv()
    {
        latitude  = 0;
        longitude = 0;
    }

    int             xPos;
    int             yPos;
    int             xMousePos;
    int             yMousePos;

    double          latitude;
    double          longitude;
};

K_GLOBAL_STATIC(QPixmap, worldMap)

WorldMapWidget::WorldMapWidget(int w, int h, QWidget *parent)
              : Q3ScrollView(parent)
{
    d = new WorldMapWidgetPriv;

    setAttribute(Qt::WA_DeleteOnClose);
    setVScrollBarMode(Q3ScrollView::AlwaysOff);
    setHScrollBarMode(Q3ScrollView::AlwaysOff);
    viewport()->setMouseTracking(true);
    setMinimumWidth(w);
    setMinimumHeight(h);
    resizeContents(worldMapPixmap().width(), worldMapPixmap().height());
}

WorldMapWidget::~WorldMapWidget()
{
    delete d;
}

QPixmap &WorldMapWidget::worldMapPixmap()
{
    if (worldMap->isNull())
    {
        QString mapPath = KStandardDirs::locate("data", "digikam/data/worldmap.jpg");
        *worldMap = QPixmap(mapPath);
    }
    return *worldMap;
}

double WorldMapWidget::getLatitude(void)
{
    return d->latitude;
}

double WorldMapWidget::getLongitude(void)
{
    return d->longitude;
}

void WorldMapWidget::setGPSPosition(double lat, double lng)
{
    d->latitude  = lat;
    d->longitude = lng;

    double latMid  = contentsHeight() / 2.0;
    double longMid = contentsWidth()  / 2.0;

    double latOffset  = ( d->latitude  * latMid )  / 90.0;
    double longOffset = ( d->longitude * longMid ) / 180.0;

    d->xPos = (int)(longMid + longOffset);
    d->yPos = (int)(latMid  - latOffset);

    viewport()->repaint();
    center(d->xPos, d->yPos);
}

void WorldMapWidget::drawContents(QPainter *p, int x, int y, int w, int h)
{
    if (isEnabled())
    {
        p->drawPixmap(x, y, worldMapPixmap(), x, y, w, h);
        p->setPen(QPen(Qt::white, 0, Qt::SolidLine));
        p->drawLine(d->xPos, 0, d->xPos, contentsHeight());
        p->drawLine(0, d->yPos, contentsWidth(), d->yPos);
        p->setPen(QPen(Qt::red, 0, Qt::DotLine));
        p->drawLine(d->xPos, 0, d->xPos, contentsHeight());
        p->drawLine(0, d->yPos, contentsWidth(), d->yPos);
        p->setPen( Qt::red );
        p->setBrush( Qt::red );
        p->drawEllipse( d->xPos-2, d->yPos-2, 4, 4 );
    }
    else
    {
        p->fillRect(x, y, w, h, palette().color(QPalette::Disabled, QPalette::Background));
    }
}

void WorldMapWidget::contentsMousePressEvent(QMouseEvent *e)
{
    if (!e) return;

    if (e->button() == Qt::LeftButton)
    {
       d->xMousePos = e->x();
       d->yMousePos = e->y();
       setCursor( Qt::SizeAllCursor );
    }
}

void WorldMapWidget::contentsMouseMoveEvent(QMouseEvent *e)
{
    if (!e) return;

    if (e->buttons() & Qt::LeftButton)
    {
       uint newxpos = e->x();
       uint newypos = e->y();

       scrollBy (-(newxpos - d->xMousePos), -(newypos - d->yMousePos));
       viewport()->repaint();

       d->xMousePos = newxpos - (newxpos-d->xMousePos);
       d->yMousePos = newypos - (newypos-d->yMousePos);
       return;
    }

    setCursor( Qt::PointingHandCursor );
}

void WorldMapWidget::contentsMouseReleaseEvent(QMouseEvent*)
{
    unsetCursor(); 
}

}  // namespace Digikam
