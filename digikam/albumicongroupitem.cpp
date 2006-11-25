/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date   : 2005-04-25
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju
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
#include <kglobal.h>
#include <kcalendarsystem.h>

// Local includes.

#include "albummanager.h"
#include "album.h"
#include "themeengine.h"
#include "albumsettings.h"
#include "albumiconview.h"
#include "albumicongroupitem.h"

namespace Digikam
{

AlbumIconGroupItem::AlbumIconGroupItem(AlbumIconView* view, int albumID)
                  : IconGroupItem(view), m_albumID(albumID), m_view(view)
{    
}

AlbumIconGroupItem::~AlbumIconGroupItem()
{    
}

int AlbumIconGroupItem::compare(IconGroupItem* group)
{
    AlbumIconGroupItem* agroup = (AlbumIconGroupItem*)group;
    
    PAlbum* mine = AlbumManager::instance()->findPAlbum(m_albumID);
    PAlbum* his = AlbumManager::instance()->findPAlbum(agroup->m_albumID);

    if (!mine || !his)
        return 0;

    const AlbumSettings *settings = m_view->settings();
    
    switch (settings->getImageSortOrder())
    {
        case(AlbumSettings::ByIName):
        case(AlbumSettings::ByISize):
        case(AlbumSettings::ByIPath):
        case(AlbumSettings::ByIRating):
        {
            return mine->url().localeAwareCompare(his->url());
        }
        case(AlbumSettings::ByIDate):
        {
            if (mine->date() < his->date())
                return -1;
            else if (mine->date() > his->date())
                return 1;
            else
                return 0;
        }
    }

    return 0;
}

void AlbumIconGroupItem::paintBanner()
{
    AlbumManager* man = AlbumManager::instance();
    PAlbum* album     = man->findPAlbum(m_albumID);

    QString dateAndComments;
    QString prettyURL;
    
    if (album)
    {
        QDate  date  = album->date();
        
        dateAndComments = i18n("%1 %2 - 1 Item", "%1 %2 - %n Items", count())
                          .arg(KGlobal::locale()->calendar()->monthName(date, false))
                          .arg(KGlobal::locale()->calendar()->year(date));
        
        if (!album->caption().isEmpty())
        {
            QString caption = album->caption();
            dateAndComments += " - " + caption.replace("\n", " ");
        }

        prettyURL = album->prettyURL();
    }        
    
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
               Qt::AlignLeft | Qt::AlignTop, prettyURL,
               -1, &tr);

    r.setY(tr.height() + 2);

    if (usePointSize)
        fn.setPointSize(m_view->font().pointSize());
    else
        fn.setPixelSize(m_view->font().pixelSize());

    fn.setBold(false);
    p.setFont(fn);

    p.drawText(5, r.y(), r.width(), r.height(),
               Qt::AlignLeft | Qt::AlignVCenter, dateAndComments);
    
    p.end();

    r = rect();
    r = QRect(iconView()->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));
    
    bitBlt(iconView()->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width(), r.height());
}

}  // namespace Digikam
