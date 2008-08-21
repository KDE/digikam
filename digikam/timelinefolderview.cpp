/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-14
 * Description : Searches dates folder view used by timeline
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qfont.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qcursor.h>

// KDE includes.

#include <kpopupmenu.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "folderitem.h"
#include "timelinefolderview.h"
#include "timelinefolderview.moc"

namespace Digikam
{

class TimeLineFolderItem : public FolderItem
{

public:

    TimeLineFolderItem(QListView* parent, SAlbum* album)
        : FolderItem(parent, album->title()),
          m_album(album)
    {
        m_album->setExtraData(parent, this);
    }

    ~TimeLineFolderItem()
    {
        m_album->removeExtraData(listView());
    }

    int compare(QListViewItem* i, int , bool ) const
    {
        if (!i)
            return 0;

        return text(0).localeAwareCompare(i->text(0));
    }

    int id() const
    {
        return m_album ? m_album->id() : 0;
    }

    SAlbum* album() const
    {
        return m_album;
    }

private:

    SAlbum *m_album;
};

TimeLineFolderView::TimeLineFolderView(QWidget* parent)
                  : FolderView(parent, "TimeLineFolderView")
{
    m_currentTimeLineSearchName = QString("_Current_Time_Line_Search_");
    addColumn(i18n("My Date Searches"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(false);

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(clear()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
        this, SLOT(slotAlbumRenamed(Album*)));

    connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            this, SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));

    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

TimeLineFolderView::~TimeLineFolderView()
{
}

QString TimeLineFolderView::currentTimeLineSearchName() const
{
    return m_currentTimeLineSearchName;
}

void TimeLineFolderView::slotTextSearchFilterChanged(const QString& filter)
{
    QString search = filter.lower();

    bool atleastOneMatch = false;

    AlbumList sList = AlbumManager::instance()->allSAlbums();
    for (AlbumList::iterator it = sList.begin(); it != sList.end(); ++it)
    {
        SAlbum* salbum               = (SAlbum*)(*it);
        TimeLineFolderItem* viewItem = (TimeLineFolderItem*) salbum->extraData(this);

        // Check if a special url query exist to identify a SAlbum dedicaced to Date Search
        // used with TimeLine.
        KURL url     = salbum->kurl();
        QString type = url.queryItem("type");

        if (salbum->title().lower().contains(search) &&
            type == QString("datesearch") && 
            salbum->title() != currentTimeLineSearchName())
        {
            atleastOneMatch = true;

            if (viewItem)
                viewItem->setVisible(true);
        }
        else
        {
            if (viewItem)
            {
                viewItem->setVisible(false);
            }
        }
    }

    emit signalTextSearchFilterMatch(atleastOneMatch);
}

void TimeLineFolderView::searchDelete(SAlbum* album)
{
    if (!album)
        return;

    // Make sure that a complicated search is not deleted accidentally
    int result = KMessageBox::warningYesNo(this, i18n("Are you sure you want to "
                                                      "delete the selected Date Search "
                                                      "\"%1\"?")
                                           .arg(album->title()),
                                           i18n("Delete Date Search?"),
                                           i18n("Delete"),
                                           KStdGuiItem::cancel());

    if (result != KMessageBox::Yes)
        return;

    AlbumManager::instance()->deleteSAlbum(album);
}

void TimeLineFolderView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum *salbum  = dynamic_cast<SAlbum*>(a);
    if (!salbum) return;

    // Check if a special url query exist to identify a SAlbum dedicaced to Date Search
    KURL url = salbum->kurl();
    QMap<QString, QString> queries = url.queryItems();
    if (queries.isEmpty()) return;

    QString type = url.queryItem("type");
    if (type != QString("datesearch")) return;

    // We will ignore the internal Dates Search Album used to perform selection from timeline.
    QString name = url.queryItem("name");
    if (name == currentTimeLineSearchName()) return;

    TimeLineFolderItem* item = new TimeLineFolderItem(this, salbum);
    item->setPixmap(0, SmallIcon("find", AlbumSettings::instance()->getDefaultTreeIconSize()));
}

void TimeLineFolderView::slotAlbumDeleted(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum* album = (SAlbum*)a;

    TimeLineFolderItem* item = (TimeLineFolderItem*) album->extraData(this);
    if (item)
        delete item;
}

void TimeLineFolderView::slotAlbumRenamed(Album* album)
{
    if (!album)
        return;

    SAlbum* salbum = dynamic_cast<SAlbum*>(album);
    if (!salbum)
        return;

    TimeLineFolderItem* item = (TimeLineFolderItem*)(salbum->extraData(this));
    if (item)
        item->setText(0, item->album()->title());
}

void TimeLineFolderView::slotSelectionChanged()
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

    if (!selItem)
    {
        emit signalAlbumSelected(0);
        return;
    }

    TimeLineFolderItem* searchItem = dynamic_cast<TimeLineFolderItem*>(selItem);

    if (!searchItem || !searchItem->album())
        emit signalAlbumSelected(0);
    else
        emit signalAlbumSelected(searchItem->album());
}

void TimeLineFolderView::slotContextMenu(QListViewItem* item, const QPoint&, int)
{
    if (!item) return;

    TimeLineFolderItem* sItem = dynamic_cast<TimeLineFolderItem*>(item);

    KPopupMenu popmenu(this);
    popmenu.insertTitle(SmallIcon("digikam"), i18n("My Date Searches"));
    popmenu.insertItem(SmallIcon("pencil"), i18n("Rename..."), 10);
    popmenu.insertItem(SmallIcon("editdelete"), i18n("Delete"), 11);

    switch (popmenu.exec(QCursor::pos()))
    {
        case 10:
        {
            emit signalRenameAlbum(sItem->album());
            break;
        }
        case 11:
        {
            searchDelete(sItem->album());
            break;
        }
        default:
            break;
    }
}

void TimeLineFolderView::selectItem(int id)
{
    SAlbum *album = AlbumManager::instance()->findSAlbum(id);
    if(!album)
        return;

    TimeLineFolderItem *item = (TimeLineFolderItem*)album->extraData(this);
    if(item)
    {
        setSelected(item, true);
        ensureItemVisible(item);
    }
}

}  // namespace Digikam
