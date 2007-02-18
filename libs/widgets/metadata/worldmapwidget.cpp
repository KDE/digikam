/* ============================================================
 * Author: Gilles Caulier
 * Date  : 2006-02-20
 * Description : a widget to display GPS info on a world map
 * 
 * Copyright 2006 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qpainter.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qlabel.h>

// KDE includes.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <klocale.h>
#include <kstaticdeleter.h>

// Local includes.

#include "ddebug.h"
#include "worldmapwidget.h"

namespace Digikam
{

class WorldMapWidgetPriv
{

public:

    WorldMapWidgetPriv()
    {
        latitude  = 0;
        longitude = 0;
        latLonPos = 0;
    }
    
    int      xPos;
    int      yPos;
    int      xMousePos;
    int      yMousePos;
    
    double   latitude;
    double   longitude;

    QLabel  *latLonPos;

    static QPixmap *worldMap;
};

static KStaticDeleter<QPixmap> pixmapDeleter;

QPixmap *WorldMapWidgetPriv::worldMap         = 0;

WorldMapWidget::WorldMapWidget(int w, int h, QWidget *parent)
              : QScrollView(parent, 0, Qt::WDestructiveClose)
{
    d = new WorldMapWidgetPriv;

    setVScrollBarMode(QScrollView::AlwaysOff);
    setHScrollBarMode(QScrollView::AlwaysOff);
    viewport()->setMouseTracking(true);
    viewport()->setPaletteBackgroundColor(colorGroup().background());
    setMinimumWidth(w);
    setMaximumHeight(h);
    resizeContents(worldMapPixmap().width(), worldMapPixmap().height());
    
    d->latLonPos = new QLabel(viewport());
    d->latLonPos->setMaximumHeight(fontMetrics().height());
    d->latLonPos->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->latLonPos->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    addChild(d->latLonPos);
}

WorldMapWidget::~WorldMapWidget()
{
    delete d;
}

QPixmap &WorldMapWidget::worldMapPixmap()
{
    if (!d->worldMap)
    {
        KGlobal::dirs()->addResourceType("worldmap", KGlobal::dirs()->kde_default("data") + "digikam/data");
        QString directory = KGlobal::dirs()->findResourceDir("worldmap", "worldmap.jpg");
        pixmapDeleter.setObject(d->worldMap, new QPixmap(directory + "worldmap.jpg"));
    }
    return *d->worldMap;
}

double WorldMapWidget::getLatitude(void)
{
    return d->latitude;
}

double WorldMapWidget::getLongitude(void)
{
    return d->longitude;
}

void WorldMapWidget::setEnabled(bool b)
{
    if (!b)
        d->latLonPos->hide();
    else 
        d->latLonPos->show();        
    
    QScrollView::setEnabled(b);
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
    
    repaintContents(false); 
    center(d->xPos, d->yPos);
    
    QString la, lo;
    d->latLonPos->setText(QString("(%1, %2)").arg(la.setNum(d->latitude,  'f', 2)) 
                                             .arg(lo.setNum(d->longitude, 'f', 2)));

    moveChild(d->latLonPos, contentsX()+10, contentsY()+10);
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
        p->fillRect(x, y, w, h, palette().disabled().background());
    }
}

void WorldMapWidget::contentsMousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
    {
       d->xMousePos = e->x();
       d->yMousePos = e->y();
       setCursor( KCursor::sizeAllCursor() );    
    }
}

void WorldMapWidget::contentsMouseReleaseEvent ( QMouseEvent *  )
{
    unsetCursor(); 
}

void WorldMapWidget::contentsMouseMoveEvent( QMouseEvent * e )
{
    if ( e->state() == Qt::LeftButton )
    {
       uint newxpos = e->x();
       uint newypos = e->y();
       
       scrollBy (-(newxpos - d->xMousePos), -(newypos - d->yMousePos));
       repaintContents(false);    
       
       d->xMousePos = newxpos - (newxpos-d->xMousePos);
       d->yMousePos = newypos - (newypos-d->yMousePos);
       return;
    }

    setCursor( KCursor::handCursor() );    
}

}  // namespace Digikam

#include "worldmapwidget.moc"
