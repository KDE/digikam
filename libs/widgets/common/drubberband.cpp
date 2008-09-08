/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2008-09-07
 * Description : Rubber band for Q3ScrollView
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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


#include "drubberband.h"
#include "ddebug.h"

namespace Digikam
{

DRubberBand::DRubberBand(Q3ScrollView *scrollView)
    : QRubberBand(QRubberBand::Rectangle, scrollView->viewport()),
      m_scrollView(scrollView), m_active(false)
{
    hide();
}

QRect DRubberBand::rubberBandAreaOnContents()
{
    QRect rubber = QRect(m_firstPoint, m_secondPoint);
    return rubber.normalized();
}

bool DRubberBand::isActive() const
{
    return m_active;
}

void DRubberBand::setActive(bool active)
{
    m_active = active;
    if (m_active)
        show();
    else
        hide();
}

void DRubberBand::setFirstPointOnViewport(const QPoint &p)
{
    m_firstPoint = p;
    m_active = true;
}

void DRubberBand::setFirstPointOnContents(const QPoint &p)
{
    setFirstPointOnViewport(m_scrollView->contentsToViewport(p));
}

void DRubberBand::setSecondPointOnViewport(const QPoint &p)
{
    m_secondPoint = p;

    updateForContentsPosition(m_scrollView->contentsX(), m_scrollView->contentsY());

    if (m_active)
        show();
}

void DRubberBand::setSecondPointOnContents(const QPoint &p)
{
    setSecondPointOnViewport(m_scrollView->contentsToViewport(p));
}

void DRubberBand::updateForContentsPosition(int contentsX, int contentsY)
{
    QRect rubber(m_firstPoint, m_secondPoint);
    rubber = rubber.normalized();
    rubber.translate( - contentsX, - contentsY);

    move(rubber.x(), rubber.y());
    resize(rubber.width(), rubber.height());
}

}

