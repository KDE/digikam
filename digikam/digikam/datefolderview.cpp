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
#include "datefolderview.h"

class DateFolderViewPriv
{
public:

    QListView*   listview;
    MonthWidget* monthview;
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

    void paintCell(QPainter* p, const QColorGroup & cg, int column, int width, int align)
    {
        if (m_album)
        {
            QListViewItem::paintCell(p, cg, column, width, align);
            return;
        }

        QFont f(listView()->font());
        f.setBold(true);
        f.setItalic(true);
        p->setFont(f);

        int w = listView()->width();
        const QScrollBar *vBar = listView()->verticalScrollBar();

        if (vBar && vBar->isVisible())
            w -= vBar->width();

        if (isSelected())
        {
            p->setPen(cg.color(QColorGroup::HighlightedText));
            p->fillRect( 0, 0, w, height(), cg.highlight());
        }
        else
        {
            p->setPen(cg.color(QColorGroup::Dark));
            p->fillRect( 0, 0, w, height(), cg.base());
        }

        int x = 2;

        p->drawPixmap(QRect(x, 0, pixmap(0)->width(), pixmap(0)->height()),
                      *pixmap(0));
        x += pixmap(0)->width() + 2;

        QRect br;
        p->drawText(x, 0, w - x, height(), AlignLeft | AlignVCenter,
                    text(0), -1, &br);
        x = br.right() + 5;

        if ((x < w - 6))
        {
            QRect rcSep(x, height()/2, w-6-x, 1);
            listView()->style().drawPrimitive(QStyle::PE_Separator, p, rcSep, cg);
        }
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
    : QVBox(parent)
{
    d = new DateFolderViewPriv;
    d->listview  = new QListView(this);
    d->monthview = new MonthWidget(this);

    d->listview->addColumn(i18n("My Dates"));
    d->listview->setResizeMode(QListView::LastColumn);
    d->listview->setRootIsDecorated(true);

    connect(AlbumManager::instance(), SIGNAL(signalDAlbumAdded(DAlbum*)),
            SLOT(slotDAlbumAdded(DAlbum*)));
    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            d->listview, SLOT(clear()));

    connect(d->listview, SIGNAL(selectionChanged()),
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
    
    QListViewItem* parent = d->listview->findItem(yr, 0);
    if (!parent)
    {
        parent = new DateFolderItem(d->listview, yr);
        parent->setPixmap(0, SmallIcon("date", 22));
    }

    DateFolderItem* item = new DateFolderItem(parent, mo, album);
    item->setPixmap(0, SmallIcon("date", 22));
}

void DateFolderView::slotSelectionChanged()
{
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

    if (!selItem)
    {
        AlbumManager::instance()->setCurrentAlbum(0);
        d->monthview->setActive(false);
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

        QDate date = dateItem->m_album->getDate();        
        d->monthview->setActive(true);
        d->monthview->setYearMonth(date.year(), date.month());
    }
}

#include "datefolderview.moc"
