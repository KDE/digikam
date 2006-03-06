/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-20
 * Description : a widget to display GPS info on a world map
 * 
 * Copyright 2006 by Gilles Caulier
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

#include <qimage.h>
#include <qpainter.h>
#include <qstring.h>

// KDE includes.

#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>

// Local includes.

#include "worldmapwidget.h"

namespace Digikam
{

WorldMapWidget::WorldMapWidget( QWidget *parent, const char *name )
              : QWidget( parent, name ), m_latitude( 0 ), m_longitude( 0 )
{
    KGlobal::dirs()->addResourceType("worldmap", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("worldmap", "worldmap.png");
    QImage map(directory + "worldmap.png");
    m_worldMap = QPixmap(map.scale(300, 150));
    
    setBackgroundMode( Qt::NoBackground );
    setFixedSize( m_worldMap.size() );
    update();
}

WorldMapWidget::~WorldMapWidget()
{
}

double WorldMapWidget::getLatitude(void)
{
    return m_latitude;
}

double WorldMapWidget::getLongitude(void)
{
    return m_longitude;
}

void WorldMapWidget::setGPSPosition(double lat, double lng)
{
    m_latitude  = lat;
    m_longitude = lng;
    repaint(false);
}

void WorldMapWidget::paintEvent( QPaintEvent* )
{
    QPixmap pm( m_worldMap );

    if (isEnabled())
    {
        QPainter p;
        p.begin( &pm, this );
    
        double latMid  = height() / 2.0;
        double longMid = width() / 2.0;
        
        double latOffset  = ( m_latitude * latMid ) / 90.0;
        double longOffset = ( m_longitude * longMid ) / 180.0;
    
        int xPos = (int)(longMid + longOffset);
        int yPos = (int)(latMid - latOffset);
    
        p.setPen(QPen(Qt::white, 0, Qt::SolidLine));
        p.drawLine(xPos, 0, xPos, height());
        p.drawLine(0, yPos, width(), yPos);
        p.setPen(QPen(Qt::red, 0, Qt::DotLine));
        p.drawLine(xPos, 0, xPos, height());
        p.drawLine(0, yPos, width(), yPos);
        p.setPen( Qt::red );
        p.setBrush( Qt::red );
        p.drawEllipse( xPos-2, yPos-2, 4, 4 );
                
        p.end();
    }

    bitBlt( this, 0, 0, &pm );
}

}  // namespace Digikam

#include "worldmapwidget.moc"
