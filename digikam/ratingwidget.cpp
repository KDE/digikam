/* ============================================================
 * Author: Owen Hirst <n8rider@sbcglobal.net>
 * Date  : 2005-08-15
 * Description :
 * 
 * Copyright 2005 by Owen Hirst
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
#include <qpalette.h>

// KDE includes.

#include <kglobal.h>
#include <kstandarddirs.h>

// Local includes.

#include "themeengine.h"
#include "ratingwidget.h"

namespace Digikam
{

RatingWidget::RatingWidget(QWidget* parent)
    : QWidget(parent)
{
    m_rating = 0;    

    KGlobal::dirs()->addResourceType("digikam_rating",
                                     KGlobal::dirs()->kde_default("data")
                                     + "digikam/data");
    QString ratingPixPath = KGlobal::dirs()->findResourceDir("digikam_rating",
                                                             "rating.png");
    ratingPixPath += "/rating.png";
    m_regPixmap = QPixmap(ratingPixPath);
    m_selPixmap = m_regPixmap;

    {
        QPainter painter(&m_regPixmap);
        painter.fillRect(0, 0, m_regPixmap.width(), m_regPixmap.height(),
                         colorGroup().dark());
        painter.end();
    }

    {
        QPainter painter(&m_selPixmap);
        painter.fillRect(0, 0, m_selPixmap.width(), m_selPixmap.height(),
                         ThemeEngine::instance()->textSpecialRegColor());
        painter.end();
    }
    
    setFixedSize(QSize(m_regPixmap.width()*5, m_regPixmap.height()));
}

RatingWidget::~RatingWidget()
{
    
}

void RatingWidget::setRating(int val)
{
    m_rating = val;
    update();
}

int RatingWidget::rating() const
{
    return m_rating;
}

void RatingWidget::mouseMoveEvent(QMouseEvent* e)
{
    int pos = e->x() / m_regPixmap.width() +1;

    if (m_rating != pos)
    {
        m_rating = pos;
        emit signalRatingChanged(m_rating);
        update();
    }
}

void RatingWidget::mousePressEvent(QMouseEvent* e)
{
    int pos = e->x() / m_regPixmap.width() +1;

    if (m_rating == pos)
    {
        m_rating--;
    }
    else
    {
        m_rating = pos;
    }

    emit signalRatingChanged(m_rating);

    update();
}

void RatingWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    int x = 0;
    for (int i=0; i<m_rating; i++)
    {
        p.drawPixmap(x, 0, m_selPixmap);
        x += m_selPixmap.width();
    }

    for (int i=m_rating; i<5; i++)
    {
        p.drawPixmap(x, 0, m_regPixmap);
        x += m_regPixmap.width();
    }

    p.end();
}

}  // namespace Digikam

#include "ratingwidget.moc"
