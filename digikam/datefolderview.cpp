/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-27
 * Description : a folder view for date albums.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "datefolderview.h"
#include "datefolderview.moc"

// Qt includes.

#include <QDateTime>
#include <QFont>
#include <QPainter>
#include <QStyle>
#include <QFileInfo>

// KDE includes.

#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kcalendarsystem.h>
#include <kconfiggroup.h>

// Local includes.

#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "folderview.h"
#include "monthwidget.h"

namespace Digikam
{

class DateFolderItem : public FolderItem
{

public:

    DateFolderItem(Q3ListView* parent, DAlbum* album);
    DateFolderItem(Q3ListViewItem* parent, DAlbum* album);

    ~DateFolderItem();

    void refresh();

    int     compare(Q3ListViewItem *i, int, bool) const;
    QString date() const;
    QString name() const;

    DAlbum* album() const;

    int count() const;
    void setCount(int v);

private:

    int      m_count;

    QString  m_name;

    DAlbum  *m_album;
};

DateFolderItem::DateFolderItem(Q3ListView* parent, DAlbum* album)
              : FolderItem(parent, QString(), true)
{
    m_count = 0;
    m_album = album;
    m_name  = QString::number(album->date().year());
    setText(0, m_name);
}

DateFolderItem::DateFolderItem(Q3ListViewItem* parent, DAlbum* album)
              : FolderItem(parent, QString())
{
    m_count = 0;
    m_album = album;
    m_name  = KGlobal::locale()->calendar()->monthName(m_album->date(), KCalendarSystem::LongName);
    setText(0, m_name);
}

DateFolderItem::~DateFolderItem()
{
}

void DateFolderItem::refresh()
{
    if (AlbumSettings::instance()->getShowFolderTreeViewItemsCount())
        setText(0, QString("%1 (%2)").arg(m_name).arg(m_count));
    else
        setText(0, m_name);
}

int DateFolderItem::compare(Q3ListViewItem* i, int , bool ) const
{
    if (!i)
        return 0;

    DateFolderItem* dItem = dynamic_cast<DateFolderItem*>(i);
    if (m_album->date() == dItem->m_album->date())
        return 0;
    else if (m_album->date() > dItem->m_album->date())
        return 1;
    else
        return -1;
}

QString DateFolderItem::date() const
{
    return m_album->date().toString();
}

QString DateFolderItem::name() const
{
    return m_name;
}

DAlbum* DateFolderItem::album() const
{
    return m_album;
}

int DateFolderItem::count() const
{
    return m_count;
}

void DateFolderItem::setCount(int v)
{
    m_count = v;
    refresh();
}

// -----------------------------------------------------------------

class DateFolderViewPriv
{
public:

    DateFolderViewPriv()
    {
        active    = false;
        listview  = 0;
        monthview = 0;
    }

    bool         active;

    QString      selected;

    FolderView  *listview;
    MonthWidget *monthview;
};

DateFolderView::DateFolderView(QWidget* parent)
              : KVBox(parent), d(new DateFolderViewPriv)
{
    setObjectName("DateFolderView");

    d->listview  = new FolderView(this, "DateListView");
    d->monthview = new MonthWidget(this);

    d->listview->addColumn(i18n("My Calendar"));
    d->listview->setResizeMode(Q3ListView::LastColumn);
    d->listview->setRootIsDecorated(true);

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAllDAlbumsLoaded()),
            this, SLOT(slotAllDAlbumsLoaded()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            d->listview, SLOT(clear()));

    connect(AlbumManager::instance(), SIGNAL(signalDAlbumsDirty(const QMap<YearMonth, int>&)),
            this, SLOT(slotRefresh(const QMap<YearMonth, int>&)));

    connect(d->listview, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

DateFolderView::~DateFolderView()
{
    saveViewState();
    delete d;
}

void DateFolderView::setActive(bool val)
{
    if (d->active == val)
        return;

    d->active = val;
    if (d->active)
    {
        slotSelectionChanged();
    }
    else
    {
        d->monthview->setActive(false);
    }
}

void DateFolderView::slotAllDAlbumsLoaded()
{
    disconnect(AlbumManager::instance(), SIGNAL(signalAllDAlbumsLoaded()),
               this, SLOT(slotAllDAlbumsLoaded()));
    loadViewState();
}

void DateFolderView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::DATE)
        return;

    DAlbum* album = (DAlbum*)a;
    QDate date    = album->date();

    if (album->range() == DAlbum::Year)
    {
        DateFolderItem* item = new DateFolderItem(d->listview, album);
        item->setPixmap(0, SmallIcon("view-calendar-list", AlbumSettings::instance()->getTreeViewIconSize()));
        album->setExtraData(this, item);
        return;
    }

    QString yr             = QString::number(date.year());
    Q3ListViewItem* parent = findRootItemByYear(yr);

    if (parent)
    {
        DateFolderItem* item = new DateFolderItem(parent, album);
        item->setPixmap(0, SmallIcon("view-calendar-month", AlbumSettings::instance()->getTreeViewIconSize()));
        album->setExtraData(this, item);
    }
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

    d->monthview->setActive(false);

    Q3ListViewItem* selItem = 0;
    Q3ListViewItemIterator it( d->listview );
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
        return;
    }

    DateFolderItem* dateItem = dynamic_cast<DateFolderItem*>(selItem);
    if (!dateItem)
    {
        AlbumManager::instance()->setCurrentAlbum(0);
        return;
    }

    AlbumManager::instance()->setCurrentAlbum(dateItem->album());

    if (dateItem->album()->range() == DAlbum::Month)
    {

        QDate date = dateItem->album()->date();
        d->monthview->setActive(true);
        d->monthview->setYearMonth(date.year(), date.month());
    }
}

void DateFolderView::loadViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(objectName());

    QString selected;
    if(group.hasKey("Last Selected Date"))
    {
        selected = group.readEntry("Last Selected Date", QString());
    }

    QStringList openFolders;
    if(group.hasKey("Open Date Folders"))
    {
        openFolders = group.readEntry("Open Date Folders", QStringList());
    }

    DateFolderItem *item;
    QString id;
    Q3ListViewItemIterator it(d->listview);
    for( ; it.current(); ++it)
    {
        item = dynamic_cast<DateFolderItem*>(it.current());
        id = item->date();
        if(openFolders.contains(id))
            d->listview->setOpen(item, true);
        else
            d->listview->setOpen(item, false);

        if(id == selected)
            d->listview->setSelected(item, true);
    }
}

void DateFolderView::saveViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(objectName());

    DateFolderItem *item = dynamic_cast<DateFolderItem*>(d->listview->selectedItem());
    if(item)
        group.writeEntry("Last Selected Date", item->date());

    QStringList openFolders;
    Q3ListViewItemIterator it(d->listview);
    item = dynamic_cast<DateFolderItem*>(d->listview->firstChild());
    while(item)
    {
        // Storing the years only, a month cannot be open
        if(item && d->listview->isOpen(item))
            openFolders.push_back(item->date());
        item = dynamic_cast<DateFolderItem*>(item->nextSibling());
    }
    group.writeEntry("Open Date Folders", openFolders);
}

void DateFolderView::gotoDate(const QDate& dt)
{
    DateFolderItem *item = 0;
    QDate           id;

    QDate date = QDate(dt.year(), dt.month(), 1);

    // Find that date in the side-bar list.
    Q3ListViewItemIterator it(d->listview);
    for( ; it.current(); ++it)
    {
        item = dynamic_cast<DateFolderItem*>(it.current());
        if (item->album())
        {
            id = item->album()->date();
            if(id == date)
            {
                d->listview->setSelected(item, true);
                d->listview->ensureItemVisible(item);
            }
        }
    }
}

void DateFolderView::setSelected(Q3ListViewItem *item)
{
    if(!item)
        return;

    d->listview->setSelected(item, true);
    d->listview->ensureItemVisible(item);
}

Q3ListViewItem *DateFolderView::findRootItemByYear(const QString& year)
{
    Q3ListViewItemIterator it(d->listview);

    while (it.current())
    {
        DateFolderItem* item = dynamic_cast<DateFolderItem*>(*it);
        if (item)
        {
            if (item->album()->range() == DAlbum::Year && item->name() == year)
                return item;
        }
        ++it;
    }
    return 0;
}

void DateFolderView::refresh()
{
    Q3ListViewItemIterator it(d->listview);

    while (it.current())
    {
        DateFolderItem* item = dynamic_cast<DateFolderItem*>(*it);
        if (item)
            item->refresh();
        ++it;
    }
}

void DateFolderView::slotRefresh(const QMap<YearMonth, int>& yearMonthMap)
{
    Q3ListViewItemIterator it(d->listview);

    while (it.current())
    {
        DateFolderItem* item = dynamic_cast<DateFolderItem*>(*it);
        if (item)
        {
            QDate date = item->album()->date();

            if (item->album()->range() == DAlbum::Month)
            {
                QMap<YearMonth, int>::const_iterator it2 = yearMonthMap.find(YearMonth(date.year(), date.month()));
                if ( it2 != yearMonthMap.end() )
                    item->setCount(it2.value());
            }
            else
            {
                int count = 0;
                for ( QMap<YearMonth, int>::const_iterator it2 = yearMonthMap.begin();
                      it2 != yearMonthMap.end(); ++it2 )
                {
                    if (it2.key().first == date.year())
                        count += it2.value();
                }
                item->setCount(count);
            }
        }
        ++it;
    }
}

}  // namespace Digikam
