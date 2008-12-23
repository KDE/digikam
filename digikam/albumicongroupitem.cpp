/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-25
 * Description : implementation to render album icons group item.
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

#include "albumicongroupitem.h"

// Qt includes.

#include <QPainter>
#include <QPixmap>

// KDE includes.

#include <kcalendarsystem.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstringhandler.h>

// Local includes.

#include "album.h"
#include "albumiconview.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "themeengine.h"

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
    PAlbum* his  = AlbumManager::instance()->findPAlbum(agroup->m_albumID);

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
            return KStringHandler::naturalCompare(mine->albumPath(), his->albumPath());
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

void AlbumIconGroupItem::paintBanner(QPainter *p)
{
    AlbumManager* man = AlbumManager::instance();
    PAlbum* album     = man->findPAlbum(m_albumID);

    QString dateAndComments;
    QString prettyUrl;

    if (album)
    {
        QDate  date  = album->date();

        KLocale tmpLocale(*KGlobal::locale());

        tmpLocale.setDateFormat("%B"); // long form of the month
        QString month = tmpLocale.formatDate(date);

        tmpLocale.setDateFormat("%Y"); // long form of the year
        QString year = tmpLocale.formatDate(date);

        dateAndComments = i18ncp("%1 long month name, %2 year",
                                 "Created on %1 %2 - 1 Item", "Created on %1 %2 - %3 Items",
                                 month, year,
                                 count());

        if (!album->caption().isEmpty())
        {
            QString caption = album->caption();
            dateAndComments += " - " + caption.replace('\n', ' ');
        }

        prettyUrl = album->prettyUrl();
    }

    QRect r(0, 0, rect().width(), rect().height());

    p->drawPixmap(0, 0, m_view->bannerPixmap());

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

    p->setPen(ThemeEngine::instance()->textSelColor());
    p->setFont(fn);

    QRect tr;
    p->drawText(5, 5, r.width(), r.height(),
               Qt::AlignLeft | Qt::AlignTop,
               prettyUrl.left(-1), &tr);

    r.setY(tr.height() + 2);

    if (usePointSize)
        fn.setPointSize(m_view->font().pointSize());
    else
        fn.setPixelSize(m_view->font().pixelSize());

    fn.setBold(false);
    p->setFont(fn);

    p->drawText(5, r.y(), r.width(), r.height(),
               Qt::AlignLeft | Qt::AlignVCenter, dateAndComments);
}

}  // namespace Digikam
