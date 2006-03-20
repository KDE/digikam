/* ============================================================
 * File  : superimposewidget.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-01-04
 * Description : 
 * 
 * Copyright 2005 Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
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

#include <cstdio>

// Qt includes.

#include <qpainter.h>

// KDE include.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "superimposewidget.h"
#include "superimpose.h"

namespace DigikamSuperImposeImagesPlugin
{

SuperImposeWidget::SuperImposeWidget(int w, int h, QWidget *parent)
                 : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_pixmap   = new QPixmap(w, h);
    m_editMode = MOVE;

    Digikam::ImageIface iface(0, 0);
    m_w          = iface.originalWidth();
    m_h          = iface.originalHeight();

    setBackgroundMode(Qt::NoBackground);
    setMinimumSize(w, h);
    setMouseTracking(true);

    resetEdit();
}

SuperImposeWidget::~SuperImposeWidget()
{
    if(m_pixmap) 
       delete m_pixmap;
}

Digikam::DImg SuperImposeWidget::makeSuperImpose(void)
{
    Digikam::ImageIface iface(0, 0);
    SuperImpose superimpose(iface.getOriginalImg(), &m_template, m_currentSelection, false);
    return superimpose.getTargetImage();
}

void SuperImposeWidget::resetEdit(void)
{
    m_zoomFactor = 100;
    m_currentSelection = QRect(m_w/2 - m_rect.width()/2, m_h/2 - m_rect.height()/2, 
                               m_rect.width(), m_rect.height());
    makePixmap();
    repaint(false);
}

void SuperImposeWidget::makePixmap(void)
{
    Digikam::ImageIface iface(0, 0);
    SuperImpose superimpose(iface.getOriginalImg(), &m_templateScaled, m_currentSelection, true);
    Digikam::DImg image = superimpose.getTargetImage();

    m_pixmap->fill(colorGroup().background());
    QPainter p(m_pixmap);
    QPixmap imagePix = image.convertToPixmap();
    p.drawPixmap(m_rect.x(), m_rect.y(), imagePix, 0, 0, m_rect.width(), m_rect.height());
    p.end();
}

void SuperImposeWidget::resizeEvent(QResizeEvent * e)
{
    blockSignals(true);
    delete m_pixmap;
    int w = e->size().width();
    int h = e->size().height();
    m_pixmap = new QPixmap(w, h);

    if (!m_template.isNull())
    {
        int templateWidth  = m_template.width();
        int templateHeight = m_template.height();

        if (templateWidth < templateHeight)
        {
            int neww = (int) ((float)height() / (float)templateHeight * (float)templateWidth);
            m_rect = QRect(width()/2-neww/2, 0, neww, height());
        }
        else
        {
            int newh = (int) ((float)width() / (float)templateWidth * (float)templateHeight);
            m_rect = QRect(0, height()/2-newh/2, width(), newh);
        }

        m_templateScaled = m_template.smoothScale(m_rect.width(), m_rect.height());
        makePixmap();
    }
    else
    {
        m_rect = QRect();
        m_pixmap->fill(colorGroup().background());
    }

    blockSignals(false);
}

void SuperImposeWidget::paintEvent( QPaintEvent * )
{
    bitBlt(this, 0, 0, m_pixmap);
}

void SuperImposeWidget::slotEditModeChanged(int mode)
{
    m_editMode = mode;
}

void SuperImposeWidget::slotSetCurrentTemplate(const KURL& url)
{
    m_template.load(url.path());

    if (m_template.isNull())
    {
        m_rect = QRect();
        return;
    }

    int templateWidth  = m_template.width();
    int templateHeight = m_template.height();

    if (templateWidth < templateHeight)
    {
        int neww = (int) ((float)height() / (float)templateHeight * (float)templateWidth);
        m_rect = QRect(width()/2-neww/2, 0, neww, height());
    }
    else
    {
        int newh = (int) ((float)width() / (float)templateWidth * (float)templateHeight);
        m_rect = QRect(0, height()/2-newh/2, width(), newh);
    }

    m_templateScaled = m_template.smoothScale(m_rect.width(), m_rect.height());

    m_currentSelection = QRect(m_w/2 - m_rect.width()/2, m_h/2 - m_rect.height()/2, m_rect.width(), m_rect.height());
    int z = m_zoomFactor;
    m_zoomFactor = 100;
    zoomSelection(z-100);
}

void SuperImposeWidget::moveSelection(int dx, int dy)
{
    float wf = (float)m_currentSelection.width() / (float)m_rect.width();
    float hf = (float)m_currentSelection.height() / (float)m_rect.height();

    m_currentSelection.moveBy( -(int)(wf*(float)dx), -(int)(hf*(float)dy) );
}

void SuperImposeWidget::zoomSelection(int deltaZoomFactor)
{
    m_zoomFactor = m_zoomFactor + deltaZoomFactor;
    int wf = (int)((float)m_rect.width()  * (100-(float)m_zoomFactor) / 100);
    int hf = (int)((float)m_rect.height() * (100-(float)m_zoomFactor) / 100);

    if (deltaZoomFactor > 0)  // Zoom in.
    {
        m_currentSelection.setLeft(m_currentSelection.left() + (wf /2));
        m_currentSelection.setTop(m_currentSelection.top() + (hf /2));
        m_currentSelection.setWidth(m_currentSelection.width() - wf);
        m_currentSelection.setHeight(m_currentSelection.height() - hf);
    }
    else                      // Zoom out.
    {
        m_currentSelection.setLeft(m_currentSelection.left() - (wf /2));
        m_currentSelection.setTop(m_currentSelection.top() - (hf /2));
        m_currentSelection.setWidth(m_currentSelection.width() + wf);
        m_currentSelection.setHeight(m_currentSelection.height() + hf);
    }

    makePixmap();
    repaint(false);
}

void SuperImposeWidget::mousePressEvent ( QMouseEvent * e )
{
    if ( isEnabled() && e->button() == Qt::LeftButton &&
         rect().contains( e->x(), e->y() ) )
    {
        switch (m_editMode)
        {
            case ZOOMIN:
                if (m_zoomFactor < 100)
                {
                    moveSelection(width()/2 - e->x(), height()/2 - e->y());
                    zoomSelection(+5);
                }
                break;

            case ZOOMOUT:
                if (m_zoomFactor > 1)
                {
                    moveSelection(width()/2 - e->x(), height()/2 - e->y());
                    zoomSelection(-5);
                }
                break;

            case MOVE:
                m_xpos = e->x();
                m_ypos = e->y();
                setCursor ( KCursor::sizeAllCursor() );
        }
    }
}

void SuperImposeWidget::mouseReleaseEvent ( QMouseEvent * )
{
    unsetCursor();
}

void SuperImposeWidget::mouseMoveEvent ( QMouseEvent * e )
{
    if ( isEnabled() && rect().contains( e->x(), e->y() ) )
    {
        if ( e->state() == Qt::LeftButton )
        {
            switch (m_editMode)
            {
                case ZOOMIN:
                case ZOOMOUT:
                    break;

                case MOVE:
                    uint newxpos = e->x();
                    uint newypos = e->y();

                    moveSelection(newxpos - m_xpos, newypos - m_ypos);
                    makePixmap();
                    repaint(false);

                    m_xpos = newxpos;
                    m_ypos = newypos;
                    setCursor( KCursor::handCursor() );
                    break;
            }
        }
        else
        {
            switch (m_editMode)
            {
                case ZOOMIN:
                case ZOOMOUT:
                    setCursor( KCursor::crossCursor() );
                    break;

                case MOVE:
                    setCursor ( KCursor::sizeAllCursor() );
            }
        }
    }
}

}  // NameSpace DigikamSuperImposeImagesPlugin


#include "superimposewidget.moc"
