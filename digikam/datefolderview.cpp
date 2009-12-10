/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-27
 * Description : a folder view for date albums.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "datefolderview.moc"

// Qt includes

#include <QDateTime>
#include <QFont>
#include <QPainter>
#include <QStyle>
#include <QFileInfo>

// KDE includes


#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kcalendarsystem.h>
#include <kconfiggroup.h>
#include <kdebug.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "albumtreeview.h"
#include "folderview.h"
#include "monthwidget.h"

namespace Digikam
{

//class DateFolderItem : public FolderItem
//{
//
//public:
//
//    DateFolderItem(Q3ListView* parent, DAlbum* album);
//    DateFolderItem(Q3ListViewItem* parent, DAlbum* album);
//
//    ~DateFolderItem();
//
//    void refresh();
//
//    int     compare(Q3ListViewItem *i, int, bool) const;
//    QString date() const;
//    QString name() const;
//
//    DAlbum* album() const;
//
//    int count() const;
//    void setCount(int v);
//
//private:
//
//    int      m_count;
//
//    QString  m_name;
//
//    DAlbum  *m_album;
//};
//
//DateFolderItem::DateFolderItem(Q3ListView* parent, DAlbum* album)
//              : FolderItem(parent, QString(), true)
//{
//    m_count = 0;
//    m_album = album;
//    m_name  = QString::number(album->date().year());
//    setText(0, m_name);
//}
//
//DateFolderItem::DateFolderItem(Q3ListViewItem* parent, DAlbum* album)
//              : FolderItem(parent, QString())
//{
//    m_count = 0;
//    m_album = album;
//    m_name  = KGlobal::locale()->calendar()->monthName(m_album->date(), KCalendarSystem::LongName);
//    setText(0, m_name);
//}
//
//DateFolderItem::~DateFolderItem()
//{
//}
//
//void DateFolderItem::refresh()
//{
//    if (AlbumSettings::instance()->getShowFolderTreeViewItemsCount())
//        setText(0, QString("%1 (%2)").arg(m_name).arg(m_count));
//    else
//        setText(0, m_name);
//}
//
//int DateFolderItem::compare(Q3ListViewItem* i, int , bool ) const
//{
//    if (!i)
//        return 0;
//
//    DateFolderItem* dItem = dynamic_cast<DateFolderItem*>(i);
//    if (m_album->date() == dItem->m_album->date())
//        return 0;
//    else if (m_album->date() > dItem->m_album->date())
//        return 1;
//    else
//        return -1;
//}
//
//QString DateFolderItem::date() const
//{
//    return m_album->date().toString();
//}
//
//QString DateFolderItem::name() const
//{
//    return m_name;
//}
//
//DAlbum* DateFolderItem::album() const
//{
//    return m_album;
//}
//
//int DateFolderItem::count() const
//{
//    return m_count;
//}
//
//void DateFolderItem::setCount(int v)
//{
//    m_count = v;
//    refresh();
//}

// -----------------------------------------------------------------

class DateFolderViewPriv
{
public:

    DateFolderViewPriv() :
        active(false),
        dateTreeView(0),
        monthview(0)
    {
    }

    bool         active;

    QString      selected;

    DateAlbumTreeView *dateTreeView;
    MonthWidget *monthview;
};

DateFolderView::DateFolderView(QWidget* parent, DateAlbumModel *dateAlbumModel)
              : KVBox(parent), d(new DateFolderViewPriv)
{
    setObjectName("DateFolderView");

    d->dateTreeView  = new DateAlbumTreeView(this, dateAlbumModel);
    d->dateTreeView->setSelectAlbumOnClick(true);
    d->monthview = new MonthWidget(this);

    connect(d->dateTreeView, SIGNAL(currentAlbumChanged(Album*)),
            this, SLOT(slotSelectionChanged(Album*)));

}

DateFolderView::~DateFolderView()
{
    saveViewState();
    delete d;
}

void DateFolderView::setImageModel(ImageFilterModel *model)
{
    d->monthview->setImageModel(model);
}

void DateFolderView::setActive(bool val)
{
    if (d->active == val)
        return;

    d->active = val;
    if (d->active)
    {
        slotSelectionChanged(d->dateTreeView->currentAlbum());
    }
    else
    {
        d->monthview->setActive(false);
    }
}

void DateFolderView::slotSelectionChanged(Album *selectedAlbum)
{
    if (!d->active)
    {
        kDebug() << "Not active, returning without action";
        return;
    }

    d->monthview->setActive(false);

    DAlbum *dalbum = dynamic_cast<DAlbum *> (selectedAlbum);
    if (!dalbum)
    {
        return;
    }

    if (dalbum->range() == DAlbum::Month)
    {

        QDate date = dalbum->date();
        d->monthview->setActive(true);
        d->monthview->setYearMonth(date.year(), date.month());
    }
}

void DateFolderView::loadViewState()
{
    // TODO update, call tree view method
}

void DateFolderView::saveViewState()
{
    // TODO update, call tree view method
}

void DateFolderView::gotoDate(const QDate& dt)
{
    // TODO update, how to do this???
//    DateFolderItem *item = 0;
//    QDate           id;
//
//    QDate date = QDate(dt.year(), dt.month(), 1);
//
//    // Find that date in the side-bar list.
//    Q3ListViewItemIterator it(d->listview);
//    for( ; it.current(); ++it)
//    {
//        item = dynamic_cast<DateFolderItem*>(it.current());
//        if (item->album())
//        {
//            id = item->album()->date();
//            if(id == date)
//            {
//                d->listview->setSelected(item, true);
//                d->listview->ensureItemVisible(item);
//            }
//        }
//    }
}

//Q3ListViewItem *DateFolderView::findRootItemByYear(const QString& year)
//{
//    Q3ListViewItemIterator it(d->listview);
//
//    while (it.current())
//    {
//        DateFolderItem* item = dynamic_cast<DateFolderItem*>(*it);
//        if (item)
//        {
//            if (item->album()->range() == DAlbum::Year && item->name() == year)
//                return item;
//        }
//        ++it;
//    }
//    return 0;
//}

}  // namespace Digikam
