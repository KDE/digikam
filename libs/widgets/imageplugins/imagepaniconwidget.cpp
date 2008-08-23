/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-22
 * Description : a widget to display a panel to choose
 *               a rectangular image area.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes.

#include <cmath>

// Qt includes.

#include <qpainter.h>
#include <qpixmap.h>
#include <qpen.h>
#include <qtimer.h>

// Local includes.

#include "ddebug.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "imagepaniconwidget.h"
#include "imagepaniconwidget.moc"

namespace Digikam
{

class ImagePanIconWidgetPriv
{
public:

    ImagePanIconWidgetPriv()
    {
        iface        = 0;
        data         = 0;
        separateView = ImageRegionWidget::SeparateViewNone;
    }

    uchar       *data;

    int          separateView;
    
    QPointArray  hightlightPoints;
    
    ImageIface  *iface;
};

ImagePanIconWidget::ImagePanIconWidget(int w, int h, QWidget *parent, WFlags flags)
                  : PanIconWidget(parent, flags)
{
    d = new ImagePanIconWidgetPriv;

    d->iface          = new ImageIface(w, h);
    d->data           = d->iface->getPreviewImage();
    m_width           = d->iface->previewWidth();
    m_height          = d->iface->previewHeight();
    m_orgWidth        = d->iface->originalWidth();
    m_orgHeight       = d->iface->originalHeight();
    m_zoomedOrgWidth  = d->iface->originalWidth();
    m_zoomedOrgHeight = d->iface->originalHeight();
    m_pixmap          = new QPixmap(w, h);
    
    setFixedSize(m_width, m_height);

    m_rect = QRect(width()/2-m_width/2, height()/2-m_height/2, m_width, m_height);
    updatePixmap();
    m_timerID = startTimer(800);
}

ImagePanIconWidget::~ImagePanIconWidget()
{
    delete d->iface;
    delete [] d->data;
    delete d;
}

void ImagePanIconWidget::setHighLightPoints(const QPointArray& pointsList)
{
    d->hightlightPoints = pointsList;
    updatePixmap();
    repaint(false);
}

void ImagePanIconWidget::updatePixmap()
{
    // Drawing background and image.
    m_pixmap->fill(colorGroup().background());
    d->iface->paint(m_pixmap, m_rect.x(), m_rect.y(), m_rect.width(), m_rect.height());
    
    QPainter p(m_pixmap);
   
    // Drawing HighLighted points.
    
    if (!d->hightlightPoints.isEmpty())
    {
       QPoint pt;
       
       for (int i = 0 ; i < d->hightlightPoints.count() ; i++)
       {
          pt = d->hightlightPoints.point(i);
          pt.setX((int)(pt.x() * (float)(m_width)  / (float)d->iface->originalWidth())); 
          pt.setY((int)(pt.y() * (float)(m_height) / (float)d->iface->originalHeight()));
          p.setPen(QPen(Qt::black, 1, Qt::SolidLine));
          p.drawLine(pt.x(), pt.y()-1, pt.x(), pt.y()+1);
          p.drawLine(pt.x()-1, pt.y(), pt.x()+1, pt.y());
          p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
          p.drawPoint(pt.x()-1, pt.y()-1);
          p.drawPoint(pt.x()+1, pt.y()+1);
          p.drawPoint(pt.x()-1, pt.y()+1);
          p.drawPoint(pt.x()+1, pt.y()-1);
       }
    }   
    
    // Drawing selection border

    if (m_flicker) p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
    else p.setPen(QPen(Qt::red, 1, Qt::SolidLine));

    p.drawRect(m_localRegionSelection.x(), 
               m_localRegionSelection.y(),
               m_localRegionSelection.width(), 
               m_localRegionSelection.height());

    if (m_flicker) p.setPen(QPen(Qt::red, 1, Qt::DotLine));
    else p.setPen(QPen(Qt::white, 1, Qt::DotLine));

    p.drawRect(m_localRegionSelection.x(), 
               m_localRegionSelection.y(),
               m_localRegionSelection.width(), 
               m_localRegionSelection.height());
    
    if (d->separateView == ImageRegionWidget::SeparateViewVertical)
    {
        if (m_flicker) p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
        else p.setPen(QPen(Qt::red, 1, Qt::SolidLine));

        p.drawLine(m_localRegionSelection.topLeft().x() + m_localRegionSelection.width()/2,
                   m_localRegionSelection.topLeft().y(),
                   m_localRegionSelection.bottomLeft().x() + m_localRegionSelection.width()/2,
                   m_localRegionSelection.bottomLeft().y());

        if (m_flicker) p.setPen(QPen(Qt::red, 1, Qt::DotLine));
        else p.setPen(QPen(Qt::white, 1, Qt::DotLine));

        p.drawLine(m_localRegionSelection.topLeft().x() + m_localRegionSelection.width()/2,
                   m_localRegionSelection.topLeft().y() + 1,
                   m_localRegionSelection.bottomLeft().x() + m_localRegionSelection.width()/2,
                   m_localRegionSelection.bottomLeft().y() - 1);
    }
    else if (d->separateView == ImageRegionWidget::SeparateViewHorizontal)
    {
        if (m_flicker) p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
        else p.setPen(QPen(Qt::red, 1, Qt::SolidLine));
        
        p.drawLine(m_localRegionSelection.topLeft().x(),
                   m_localRegionSelection.topLeft().y() + m_localRegionSelection.height()/2,
                   m_localRegionSelection.topRight().x(),
                   m_localRegionSelection.topRight().y() + m_localRegionSelection.height()/2);

        if (m_flicker) p.setPen(QPen(Qt::red, 1, Qt::DotLine));
        else p.setPen(QPen(Qt::white, 1, Qt::DotLine));

        p.drawLine(m_localRegionSelection.topLeft().x() + 1,
                   m_localRegionSelection.topLeft().y() + m_localRegionSelection.height()/2,
                   m_localRegionSelection.topRight().x() - 1,
                   m_localRegionSelection.topRight().y() + m_localRegionSelection.height()/2);
    }

    p.end();
}

void ImagePanIconWidget::slotSeparateViewToggled(int t)
{
    d->separateView = t;
    updatePixmap();
    repaint(false);
}

}  // NameSpace Digikam
