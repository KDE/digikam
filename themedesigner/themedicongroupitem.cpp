/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-15
 * Description : themed icon group item 
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
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

#include <qpixmap.h>
#include <qpainter.h>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "themeengine.h"
#include "themediconview.h"
#include "themedicongroupitem.h"

namespace Digikam
{

ThemedIconGroupItem::ThemedIconGroupItem(ThemedIconView* view)
                   : IconGroupItem(view), m_view(view)
{    
}

ThemedIconGroupItem::~ThemedIconGroupItem()
{    
}

void ThemedIconGroupItem::paintBanner()
{
    QRect r(0, 0, rect().width(), rect().height());

    QPixmap pix(m_view->bannerPixmap());
    
    QFont fn(m_view->font());
    fn.setBold(true);
    int fnSize = fn.pointSize();
    bool usePointSize;
    if (fnSize > 0) 
    {
        fn.setPointSize(fnSize+2);
        usePointSize = true;
    }
    else 
    {
        fnSize = fn.pixelSize();
        fn.setPixelSize(fnSize+2);
        usePointSize = false;
    }

    QPainter p(&pix);
    p.setPen(ThemeEngine::instance()->textSelColor());
    p.setFont(fn);

    QRect tr;
    p.drawText(5, 5, r.width(), r.height(),
               Qt::AlignLeft | Qt::AlignTop, i18n("Album Banner"),
               -1, &tr);

    r.setY(tr.height() + 2);

    if (usePointSize)
        fn.setPointSize(m_view->font().pointSize());
    else
        fn.setPixelSize(m_view->font().pixelSize());

    fn.setBold(false);
    p.setFont(fn);

    p.drawText(5, r.y(), r.width(), r.height(),
               Qt::AlignLeft | Qt::AlignVCenter, i18n("July 2007 - 10 Items"));
    
    p.end();

    r = rect();
    r = QRect(iconView()->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));
    
    bitBlt(iconView()->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width(), r.height());
}

}  // NameSpace Digikam
