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
#include <qlistview.h>
#include <qfont.h>
#include <qpainter.h>
#include <qstyle.h>

#include "album.h"
#include "albummanager.h"
#include "monthwidget.h"
#include "folderitem.h"
#include "folderview.h"
#include "datefolderview.h"

class DateFolderViewPriv
{
public:

    FolderView*  listview;
    MonthWidget* monthview;
    bool         active;
};

class DateFolderItem : public FolderItem
{
public:

    DateFolderItem(QListView* parent, const QString& name)
        : FolderItem(parent, name, true), m_album(0)
    {
    }

    DateFolderItem(QListViewItem* parent, const QString& name, DAlbum* album)
        : FolderItem(parent, name), m_album(album)
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

        if (m_album->date() == dItem->m_album->date())
            return 0;
        else if (m_album->date() > dItem->m_album->date())
            return 1;
        else
            return -1;
    }
    
    DAlbum* m_album;
};
    


DateFolderView::DateFolderView(QWidget* parent)
    : QVBox(parent)
{
    d = new DateFolderViewPriv;
    d->active    = false;
    d->listview  = new FolderView(this);
    d->monthview = new MonthWidget(this);

    d->listview->addColumn(i18n("My Dates"));
    d->listview->setResizeMode(QListView::LastColumn);
    d->listview->setRootIsDecorated(true);

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotAlbumAdded(Album*)));
    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            SLOT(slotAlbumDeleted(Album*)));
    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            d->listview, SLOT(clear()));

    connect(d->listview, SIGNAL(selectionChanged()),
            SLOT(slotSelectionChanged()));
}

DateFolderView::~DateFolderView()
{
    delete d;
}

void DateFolderView::setActive(bool val)
{
    d->active = val;
    if (d->active)
    {
        slotSelectionChanged();
    }
}

void DateFolderView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::DATE)
        return;

    DAlbum* album = (DAlbum*)a;
    
    QDate date = album->date();

    QString yr = QString::number(date.year());
    
#if KDE_IS_VERSION(3,2,0)
    QString mo = KGlobal::locale()->calendar()->monthName(date, false);
#else
    QString mo = KGlobal::locale()->monthName(date, false);
#endif
    
    QListViewItem* parent = d->listview->findItem(yr, 0);
    if (!parent)
    {
        parent = new DateFolderItem(d->listview, yr);
        parent->setPixmap(0, SmallIcon("date", 32));
    }

    DateFolderItem* item = new DateFolderItem(parent, mo, album);
    item->setPixmap(0, SmallIcon("date", 32));

    album->setExtraData(this, item);
}

void DateFolderView::slotAlbumDeleted(Album* a)
{
    if (!a || a->type() != Album::DATE)
        return;

    DAlbum* album = (DAlbum*)a;

    DateFolderItem* item = (DateFolderItem*) album->extraData(this);
    if (item)
    {
        delete item;
        album->removeExtraData(this);
    }
}

void DateFolderView::slotSelectionChanged()
{
    if (!d->active)
        return;
    
    QListViewItem* selItem = 0;
    
    QListViewItemIterator it( d->listview );
    while (it.current())
    {
        if (it.current()->isSelected())
        {
            selItem = it.current();
            break;
        }
        ++it;
    }

    d->monthview->setActive(false);
    
    if (!selItem)
    {
        AlbumManager::instance()->setCurrentAlbum(0);
        return;
    }

    DateFolderItem* dateItem = dynamic_cast<DateFolderItem*>(selItem);
    
    if (!dateItem || !dateItem->m_album)
    {
        AlbumManager::instance()->setCurrentAlbum(0);
        d->monthview->setActive(false);
    }
    else
    {
        AlbumManager::instance()->setCurrentAlbum(dateItem->m_album);

        QDate date = dateItem->m_album->date();        
        d->monthview->setActive(true);
        d->monthview->setYearMonth(date.year(), date.month());
    }
}

#include "datefolderview.moc"
