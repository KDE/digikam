/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-22
 * Description : a generic widget to display a panel to choose
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

// C++ include.

#include <cmath>

// Qt includes.

#include <QPainter>
#include <QPixmap>
#include <QPen>
#include <QTimer>
#include <QTimerEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QHideEvent>

// KDE includes.

#include <kcursor.h>

// Local includes.

#include "ddebug.h"
#include "paniconwidget.h"
#include "paniconwidget.moc"

namespace Digikam
{

class PanIconWidgetPriv
{

public:

    PanIconWidgetPriv()
    {
        moveSelection = false;
    }

    bool   moveSelection;

    int    xpos;
    int    ypos;

    QRect  regionSelection;         // Original size image selection.

    QImage image;
};

PanIconWidget::PanIconWidget(QWidget *parent, Qt::WidgetAttribute attribute)
             : QWidget(parent)
{
    d = new PanIconWidgetPriv;
    m_flicker    = false;
    m_timerID    = 0;
    m_pixmap     = 0;
    m_zoomFactor = 1.0;

    setMouseTracking(true);
    setAttribute(attribute);
}

PanIconWidget::~PanIconWidget()
{
    if (m_timerID) killTimer(m_timerID);

    if (m_pixmap) delete m_pixmap;

    delete d;
}

void PanIconWidget::setImage(int previewWidth, int previewHeight, const QImage& image)
{
    QSize sz(image.width(), image.height());
    sz.scale(previewWidth, previewHeight, Qt::KeepAspectRatio);
    m_pixmap          = new QPixmap(previewWidth, previewHeight);
    m_width           = sz.width();
    m_height          = sz.height();
    d->image          = image.scaled(sz.width(), sz.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_orgWidth        = image.width();
    m_orgHeight       = image.height();
    m_zoomedOrgWidth  = image.width();
    m_zoomedOrgHeight = image.height();
    setFixedSize(m_width, m_height);
 
    m_rect = QRect(width()/2-m_width/2, height()/2-m_height/2, m_width, m_height);
    updatePixmap();
    m_timerID = startTimer(800);
}

void PanIconWidget::setImage(int previewWidth, int previewHeight, const DImg& image)
{
    DImg img(image);
    setImage(previewWidth, previewHeight, img.copyQImage());
}

void PanIconWidget::slotZoomFactorChanged(double factor)
{
    if (m_zoomFactor == factor) return;
    m_zoomFactor      = factor;
    m_zoomedOrgWidth  = (int)(m_orgWidth  * factor);
    m_zoomedOrgHeight = (int)(m_orgHeight * factor);
    updatePixmap();
    repaint();
}

void PanIconWidget::setRegionSelection(const QRect& regionSelection)
{
    d->regionSelection = regionSelection;
    m_localRegionSelection.setX( m_rect.x() + (int)((float)d->regionSelection.x() *
                                 ((float)m_width / (float)m_zoomedOrgWidth)) );

    m_localRegionSelection.setY( m_rect.y() + (int)((float)d->regionSelection.y() *
                                 ((float)m_height / (float)m_zoomedOrgHeight)) );

    m_localRegionSelection.setWidth( (int)((float)d->regionSelection.width() *
                                     ((float)m_width / (float)m_zoomedOrgWidth)) );

    m_localRegionSelection.setHeight( (int)((float)d->regionSelection.height() *
                                      ((float)m_height / (float)m_zoomedOrgHeight)) );

    updatePixmap();
    repaint();
}

QRect PanIconWidget::getRegionSelection()
{
    return (d->regionSelection);
}

void PanIconWidget::setCursorToLocalRegionSelectionCenter()
{
    QCursor::setPos(mapToGlobal(m_localRegionSelection.center()));
}

void PanIconWidget::setCenterSelection()
{
    setRegionSelection(QRect( 
             (int)(((float)m_zoomedOrgWidth  / 2.0) - ((float)d->regionSelection.width()  / 2.0)),
             (int)(((float)m_zoomedOrgHeight / 2.0) - ((float)d->regionSelection.height() / 2.0)),
             d->regionSelection.width(),
             d->regionSelection.height()));
}

void PanIconWidget::regionSelectionMoved(bool targetDone)
{
    if (targetDone)
    {
       updatePixmap();
       repaint();
    }

    int x = (int)lround( ((float)m_localRegionSelection.x() - (float)m_rect.x() ) *
                         ((float)m_zoomedOrgWidth / (float)m_width) );

    int y = (int)lround( ((float)m_localRegionSelection.y() - (float)m_rect.y() ) *
                         ((float)m_zoomedOrgHeight / (float)m_height) );

    int w = (int)lround( (float)m_localRegionSelection.width() *
                         ((float)m_zoomedOrgWidth / (float)m_width) );

    int h = (int)lround( (float)m_localRegionSelection.height() *
                         ((float)m_zoomedOrgHeight / (float)m_height) );

    d->regionSelection.setX(x);
    d->regionSelection.setY(y);
    d->regionSelection.setWidth(w);
    d->regionSelection.setHeight(h);

    emit signalSelectionMoved( d->regionSelection, targetDone );
}

void PanIconWidget::updatePixmap()
{
    // Drawing background and image.
    m_pixmap->fill(palette().color(QPalette::Background));
    QPainter p(m_pixmap);
    p.drawPixmap(m_rect.x(), m_rect.y(), QPixmap::fromImage(d->image));

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

    p.end();
}

void PanIconWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, *m_pixmap);
    p.end();
}

void PanIconWidget::setMouseFocus()
{
    raise();
    d->xpos          = m_localRegionSelection.center().x();
    d->ypos          = m_localRegionSelection.center().y();
    d->moveSelection = true;
    setCursor( Qt::SizeAllCursor );           
    emit signalSelectionTakeFocus();
}

void PanIconWidget::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);

    if ( d->moveSelection )
    {    
        d->moveSelection = false;
        setCursor( Qt::ArrowCursor );           
        emit signalHiden();  
    }
}

void PanIconWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( (e->button() == Qt::LeftButton || e->button() == Qt::MidButton) &&
         m_localRegionSelection.contains( e->x(), e->y() ) )
    {
        d->xpos          = e->x();
        d->ypos          = e->y();
        d->moveSelection = true;
        setCursor( Qt::SizeAllCursor );           
        emit signalSelectionTakeFocus();
    }
}

void PanIconWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( d->moveSelection && 
         (e->buttons() == Qt::LeftButton || e->buttons() == Qt::MidButton) )
    {
        int newxpos = e->x();
        int newypos = e->y();

        m_localRegionSelection.translate(newxpos - d->xpos, newypos - d->ypos);

        d->xpos = newxpos;
        d->ypos = newypos;

        // Perform normalization of selection area.

        if (m_localRegionSelection.left() < m_rect.left())
            m_localRegionSelection.moveLeft(m_rect.left());

        if (m_localRegionSelection.top() < m_rect.top())
            m_localRegionSelection.moveTop(m_rect.top());

        if (m_localRegionSelection.right() > m_rect.right())
            m_localRegionSelection.moveRight(m_rect.right());

        if (m_localRegionSelection.bottom() > m_rect.bottom())
            m_localRegionSelection.moveBottom(m_rect.bottom());

        updatePixmap();
        repaint();
        regionSelectionMoved(false);
        return;
    }
    else 
    {
        if ( m_localRegionSelection.contains( e->x(), e->y() ) )
            setCursor( Qt::PointingHandCursor );
        else
            setCursor( Qt::ArrowCursor );
    }
}

void PanIconWidget::mouseReleaseEvent ( QMouseEvent * )
{
    if ( d->moveSelection )
    {
        d->moveSelection = false;
        setCursor( Qt::ArrowCursor );
        regionSelectionMoved(true);
    }
}

void PanIconWidget::timerEvent(QTimerEvent * e)
{
    if (e->timerId() == m_timerID)
    {
        m_flicker = !m_flicker;
        updatePixmap();
        repaint();
    }
    else
        QWidget::timerEvent(e);
}

}  // NameSpace Digikam
