/* ============================================================
 * File  : datefolderview.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-27
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include <kdeversion.h>
#if KDE_IS_VERSION(3,2,0)
#include <kcalendarsystem.h>
#endif

#include <qdatetime.h>

#include "album.h"
#include "albummanager.h"
#include "datefolderview.h"

class DateFolderViewPriv
{
public:

};

class DateFolderItem : public QListViewItem
{
public:

    DateFolderItem(QListView* parent, const QString& name)
        : QListViewItem(parent, name), m_album(0)
    {
    }

    DateFolderItem(QListViewItem* parent, const QString& name, DAlbum* album)
        : QListViewItem(parent, name), m_album(album)
    {
    }

    int compare(QListViewItem* i, int , bool ) const
    {
        if (!i)
            return 0;
        
        DateFolderItem* dItem = dynamic_cast<DateFolderItem*>(i);
        if (!dItem || !dItem->m_album)
        {
            return text(0).localeAwareCompare(i->text(0));
        } 

        if (m_album->getDate() == dItem->m_album->getDate())
            return 0;
        else if (m_album->getDate() > dItem->m_album->getDate())
            return 1;
        else
            return -1;
    }
    
    DAlbum* m_album;
};
    


DateFolderView::DateFolderView(QWidget* parent)
    : QListView(parent)
{
    d = new DateFolderViewPriv;

    addColumn(i18n("My Dates"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(true);

    connect(AlbumManager::instance(), SIGNAL(signalDAlbumAdded(DAlbum*)),
            SLOT(slotDAlbumAdded(DAlbum*)));

    connect(this, SIGNAL(selectionChanged()),
            SLOT(slotSelectionChanged()));
}

DateFolderView::~DateFolderView()
{
    delete d;
}

void DateFolderView::slotDAlbumAdded(DAlbum* album)
{
    if (!album)
        return;

    QDate date = album->getDate();

    QString yr = QString::number(date.year());
    
#if KDE_IS_VERSION(3,2,0)
    QString mo = KGlobal::locale()->calendar()->monthName(date, false);
#else
    QString mo = KGlobal::locale()->monthName(date, false);
#endif
    
    QListViewItem* parent = findItem(yr, 0);
    if (!parent)
    {
        parent = new DateFolderItem(this, yr);
        parent->setPixmap(0, SmallIcon("date"));
    }

    DateFolderItem* item = new DateFolderItem(parent, mo, album);
    item->setPixmap(0, SmallIcon("date"));
}

void DateFolderView::slotSelectionChanged()
{
    QListViewItem* selItem = 0;
    
    QListViewItemIterator it( this );
    while (it.current())
    {
        if (it.current()->isSelected())
        {
            selItem = it.current();
            break;
        }
        ++it;
    }

    DateFolderItem* dateItem = dynamic_cast<DateFolderItem*>(selItem);
    
    if (!dateItem)
    {
        AlbumManager::instance()->setCurrentAlbum(0);
    }
    else
    {
        AlbumManager::instance()->setCurrentAlbum(dateItem->m_album);
    }
}

#include "datefolderview.moc"
