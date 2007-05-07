/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-20
 * Description : search results item.
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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
#include <qpixmap.h>

// Local includes.

#include "searchresultsitem.h"

namespace Digikam
{

QPixmap* SearchResultsItem::m_basePixmap = 0;

SearchResultsItem::SearchResultsItem(QIconView* view, const QString& path)
    : QIconViewItem(view), m_path(path)
{
    if (!m_basePixmap)
    {
        m_basePixmap = new QPixmap(128, 128);
        m_basePixmap->fill(view->colorGroup().base());

        QPainter p(m_basePixmap);
        p.setPen(Qt::lightGray);
        p.drawRect(0, 0, 128, 128);
        p.end();
    }

    setPixmap(*m_basePixmap);
    calcRect();
    m_marked = true;
}

SearchResultsItem::~SearchResultsItem()
{
    
}

void SearchResultsItem::calcRect(const QString&)
{
    QRect r(0,0,0,0);
    setTextRect(r);
    setPixmapRect(r);
    setItemRect(QRect(x(),y(),130,130));
}

void SearchResultsItem::paintItem(QPainter* p, const QColorGroup&)
{
    QRect r(rect());
    p->drawPixmap(r.x() + (width()-pixmap()->width())/2 ,
                  r.y() + (height()-pixmap()->height())/2,
                  *pixmap());
}

void SearchResultsItem::paintFocus(QPainter* p, const QColorGroup&)
{
    QRect r(rect());
    p->save();
    p->setPen(QPen(Qt::darkGray, 0, Qt::DotLine));
    p->drawRect(rect());
    p->restore();
}

}  // namespace Digikam
