/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a
 *               template to an image.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "superimposewidget.moc"

// C++ includes

#include <cstdio>

// Qt includes

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPixmap>
#include <QMouseEvent>

// KDE includes

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kglobal.h>

// Local includes

#include "superimpose.h"

namespace DigikamDecorateImagePlugin
{

SuperImposeWidget::SuperImposeWidget(int w, int h, QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_bgColor  = palette().color(QPalette::Background);
    m_pixmap   = new QPixmap(w, h);
    m_editMode = MOVE;

    ImageIface iface(0, 0);
    m_w = iface.originalWidth();
    m_h = iface.originalHeight();

    setMinimumSize(w, h);
    setMouseTracking(true);

    resetEdit();
}

SuperImposeWidget::~SuperImposeWidget()
{
    delete m_pixmap;
}

DImg SuperImposeWidget::makeSuperImpose()
{
    ImageIface iface(0, 0);
    SuperImpose superimpose(iface.getOriginalImg(), &m_template, m_currentSelection);
    return superimpose.getTargetImage();
}

void SuperImposeWidget::resetEdit()
{
    m_zoomFactor       = 1.0;
    m_currentSelection = QRect(m_w/2 - m_rect.width()/2, m_h/2 - m_rect.height()/2,
                               m_rect.width(), m_rect.height());
    makePixmap();
    repaint();
}

void SuperImposeWidget::makePixmap()
{
    ImageIface iface(0, 0);
    SuperImpose superimpose(iface.getOriginalImg(), &m_templateScaled, m_currentSelection);
    DImg image = superimpose.getTargetImage();

    m_pixmap->fill(m_bgColor);
    QPainter p(m_pixmap);
    QPixmap imagePix = image.convertToPixmap();
    p.drawPixmap(m_rect.x(), m_rect.y(), imagePix, 0, 0, m_rect.width(), m_rect.height());
    p.end();
}

void SuperImposeWidget::resizeEvent(QResizeEvent* e)
{
    blockSignals(true);
    delete m_pixmap;
    int w    = e->size().width();
    int h    = e->size().height();
    m_pixmap = new QPixmap(w, h);

    if (!m_template.isNull())
    {
        int templateWidth  = m_template.width();
        int templateHeight = m_template.height();

        if (templateWidth < templateHeight)
        {
            int neww = (int) ((float)height() / (float)templateHeight * (float)templateWidth);
            m_rect   = QRect(width()/2-neww/2, 0, neww, height());
        }
        else
        {
            int newh = (int) ((float)width() / (float)templateWidth * (float)templateHeight);
            m_rect   = QRect(0, height()/2-newh/2, width(), newh);
        }

        m_templateScaled = m_template.smoothScale(m_rect.width(), m_rect.height());
        makePixmap();
    }
    else
    {
        m_rect = QRect();
        m_pixmap->fill(palette().color(QPalette::Background));
    }

    blockSignals(false);
}

void SuperImposeWidget::setBackgroundColor(const QColor& bg)
{
    m_bgColor = bg;
    makePixmap();
    repaint();
}

void SuperImposeWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, *m_pixmap);
    p.end();
}

void SuperImposeWidget::slotEditModeChanged(int mode)
{
    m_editMode = mode;
}

void SuperImposeWidget::slotSetCurrentTemplate(const KUrl& url)
{
    m_template.load(url.toLocalFile());

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
        m_rect   = QRect(width()/2-neww/2, 0, neww, height());
    }
    else
    {
        int newh = (int) ((float)width() / (float)templateWidth * (float)templateHeight);
        m_rect   = QRect(0, height()/2-newh/2, width(), newh);
    }

    m_templateScaled  = m_template.smoothScale(m_rect.width(), m_rect.height());
    m_currentSelection = QRect(m_w/2 - m_rect.width()/2, m_h/2 - m_rect.height()/2,
                               m_rect.width(), m_rect.height());
    zoomSelection(0);
}

void SuperImposeWidget::moveSelection(int dx, int dy)
{
    QRect selection = m_currentSelection;
    float wf = (float)selection.width() / (float)m_rect.width();
    float hf = (float)selection.height() / (float)m_rect.height();

    selection.translate( -(int)(wf*(float)dx), -(int)(hf*(float)dy) );

    if (selection.left() < 0)
    {
        selection.moveLeft(0);
    }

    if (selection.top() < 0)
    {
        selection.moveTop(0);
    }

    if (selection.bottom() > m_h)
    {
        selection.moveBottom(m_h);
    }

    if (selection.right() > m_w)
    {
        selection.moveRight(m_w);
    }

    m_currentSelection = selection;
}

bool SuperImposeWidget::zoomSelection(float deltaZoomFactor)
{
    float newZoom = m_zoomFactor + deltaZoomFactor;

    if (newZoom < 0.0)
    {
        return false;
    }

    QRect selection = m_currentSelection;
    int wf          = (int)((float)m_rect.width()  / newZoom);
    int hf          = (int)((float)m_rect.height() / newZoom);
    int deltaX      = (m_currentSelection.width()  - wf) / 2;
    int deltaY      = (m_currentSelection.height() - hf) / 2;

    selection.setLeft(m_currentSelection.left() + deltaX);
    selection.setTop(m_currentSelection.top() + deltaY);
    selection.setWidth(wf);
    selection.setHeight(hf);

    // check that selection is still inside original image
    QRect orgImageRect(0, 0, m_w, m_h);

    if (!orgImageRect.contains(selection))
    {
        // try to adjust
        if (selection.left() < 0)
        {
            selection.moveLeft(0);
        }

        if (selection.top() < 0)
        {
            selection.moveTop(0);
        }

        if (selection.bottom() > m_h)
        {
            selection.moveBottom(m_h);
        }

        if (selection.right() > m_w)
        {
            selection.moveRight(m_w);
        }

        // was it successful?
        if (selection.contains(orgImageRect))
        {
            return false;
        }

    }

    m_zoomFactor       = newZoom;
    m_currentSelection = selection;

    makePixmap();
    repaint();

    return true;
}

void SuperImposeWidget::mousePressEvent(QMouseEvent* e)
{
    if ( isEnabled() && e->button() == Qt::LeftButton &&
         rect().contains( e->x(), e->y() ) )
    {
        switch (m_editMode)
        {
            case ZOOMIN:

                if (zoomSelection(+0.05F))
                {
                    moveSelection(width()/2 - e->x(), height()/2 - e->y());
                }

                break;

            case ZOOMOUT:

                if (zoomSelection(-0.05F))
                {
                    moveSelection(width()/2 - e->x(), height()/2 - e->y());
                }

                break;

            case MOVE:
                m_xpos = e->x();
                m_ypos = e->y();
        }
    }
}

void SuperImposeWidget::mouseReleaseEvent(QMouseEvent*)
{
    setEditModeCursor();
}

void SuperImposeWidget::mouseMoveEvent(QMouseEvent* e)
{
    if ( isEnabled() )
    {
        if ( e->buttons() == Qt::LeftButton )
        {
            switch (m_editMode)
            {
                case ZOOMIN:
                case ZOOMOUT:
                    break;

                case MOVE:
                    int newxpos = e->x();
                    int newypos = e->y();

                    if (newxpos < m_rect.left())
                    {
                        newxpos = m_rect.left();
                    }

                    if (newxpos > m_rect.right())
                    {
                        newxpos = m_rect.right();
                    }

                    if (newxpos < m_rect.top())
                    {
                        newxpos = m_rect.top();
                    }

                    if (newxpos > m_rect.bottom())
                    {
                        newxpos = m_rect.bottom();
                    }

                    moveSelection(newxpos - m_xpos, newypos - m_ypos);
                    makePixmap();
                    repaint();

                    m_xpos = newxpos;
                    m_ypos = newypos;
                    setCursor( Qt::PointingHandCursor );
                    break;
            }
        }
        else if (rect().contains( e->x(), e->y() ))
        {
            setEditModeCursor();
        }
    }
}

void SuperImposeWidget::setEditModeCursor()
{
    switch (m_editMode)
    {
        case ZOOMIN:
        case ZOOMOUT:
            setCursor ( Qt::CrossCursor );
            break;

        case MOVE:
            setCursor ( Qt::SizeAllCursor );
    }
}

}  // namespace DigikamDecorateImagePlugin
