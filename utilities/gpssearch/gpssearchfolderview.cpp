/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-30
 * Description : GPS search folder view
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "gpssearchfolderview.h"
#include "gpssearchfolderview.moc"

// Qt includes.

#include <QFont>
#include <QPainter>
#include <QStyle>
#include <QCursor>

// KDE includes.

#include <kmenu.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "folderitem.h"

namespace Digikam
{

class GPSSearchFolderItem : public FolderItem
{

public:

    GPSSearchFolderItem(Q3ListView* parent, SAlbum* album)
        : FolderItem(parent, album->title()),
          m_album(album)
    {
        m_album->setExtraData(parent, this);
    }

    ~GPSSearchFolderItem()
    {
        m_album->removeExtraData(listView());
    }

    int compare(Q3ListViewItem* i, int , bool ) const
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

GPSSearchFolderView::GPSSearchFolderView(QWidget* parent)
                     : FolderView(parent, "GPSSearchFolderView")
{
    addColumn(i18n("My Map Searches"));
    setResizeMode(Q3ListView::LastColumn);
    setRootIsDecorated(false);

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(clear()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
        this, SLOT(slotAlbumRenamed(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumCurrentChanged(Album*)),
        this, SLOT(slotAlbumCurrentChanged(Album*)));

    connect(this, SIGNAL(contextMenuRequested(Q3ListViewItem*, const QPoint&, int)),
            this, SLOT(slotContextMenu(Q3ListViewItem*, const QPoint&, int)));

    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

GPSSearchFolderView::~GPSSearchFolderView()
{
    saveViewState();
}

QString GPSSearchFolderView::currentGPSSearchName()
{
    return QString("_Current_Map_Search_");
}

void GPSSearchFolderView::slotTextSearchFilterChanged(const SearchTextSettings& settings)
{
    QString search       = settings.text;
    bool atleastOneMatch = false;

    AlbumList sList = AlbumManager::instance()->allSAlbums();
    for (AlbumList::iterator it = sList.begin(); it != sList.end(); ++it)
    {
        SAlbum* salbum                = (SAlbum*)(*it);
        GPSSearchFolderItem* viewItem = (GPSSearchFolderItem*) salbum->extraData(this);

        if (salbum->title().contains(search, settings.caseSensitive) &&
            salbum->isHaarSearch() &&
            salbum->title() != currentGPSSearchName())
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

void GPSSearchFolderView::searchDelete(SAlbum* album)
{
    if (!album)
        return;

    if (album->title() == currentGPSSearchName())
        return;

    // Make sure that a complicated search is not deleted accidentally
    int result = KMessageBox::warningYesNo(this, i18n("Are you sure you want to "
                                                      "delete the selected Map Search "
                                                      "\"%1\"?", album->title()),
                                           i18n("Delete Map Search?"),
                                           KGuiItem(i18n("Delete")),
                                           KStandardGuiItem::cancel());

    if (result != KMessageBox::Yes)
        return;

    AlbumManager::instance()->deleteSAlbum(album);
}

void GPSSearchFolderView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum *salbum  = dynamic_cast<SAlbum*>(a);
    if (!salbum) return;

    if (!salbum->isMapSearch())
        return;

    GPSSearchFolderItem* item = new GPSSearchFolderItem(this, salbum);
    item->setPixmap(0, SmallIcon("applications-internet", AlbumSettings::instance()->getTreeViewIconSize()));

    if (salbum->title() == currentGPSSearchName())
        item->setText(0, i18n("Current Map Search"));
}

void GPSSearchFolderView::slotAlbumCurrentChanged(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum *salbum  = dynamic_cast<SAlbum*>(a);
    if (!salbum) return;

    if (!salbum->isMapSearch())
        return;

    GPSSearchFolderItem* item = (GPSSearchFolderItem*) salbum->extraData(this);
    if (item)
        setCurrentItem(item);
}

void GPSSearchFolderView::slotAlbumDeleted(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum* album = (SAlbum*)a;

    GPSSearchFolderItem* item = (GPSSearchFolderItem*) album->extraData(this);
    if (item)
        delete item;
}

void GPSSearchFolderView::slotAlbumRenamed(Album* album)
{
    if (!album)
        return;

    SAlbum* salbum = dynamic_cast<SAlbum*>(album);
    if (!salbum)
        return;

    GPSSearchFolderItem* item = (GPSSearchFolderItem*)(salbum->extraData(this));
    if (item)
        item->setText(0, item->album()->title());
}

void GPSSearchFolderView::slotSelectionChanged()
{
    Q3ListViewItem* selItem = 0;

    Q3ListViewItemIterator it( this );
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

    GPSSearchFolderItem* searchItem = dynamic_cast<GPSSearchFolderItem*>(selItem);

    if (!searchItem || !searchItem->album())
        emit signalAlbumSelected(0);
    else
        emit signalAlbumSelected(searchItem->album());
}

void GPSSearchFolderView::slotContextMenu(Q3ListViewItem* item, const QPoint&, int)
{
    if (!item) return;

    GPSSearchFolderItem* sItem = dynamic_cast<GPSSearchFolderItem*>(item);

    KMenu popmenu(this);
    popmenu.addTitle(SmallIcon("digikam"), i18n("My Map Searches"));
    QAction *renSearch = popmenu.addAction(SmallIcon("edit-rename"), i18n("Rename..."));
    QAction *delSearch = popmenu.addAction(SmallIcon("edit-delete"), i18n("Delete"));
    QAction *choice    = popmenu.exec(QCursor::pos());
    if (choice)
    {
        if (choice == renSearch)
        {
            emit signalRenameAlbum(sItem->album());
        }
        else if (choice == delSearch)
        {
            searchDelete(sItem->album());
        }
    }
}

void GPSSearchFolderView::selectItem(int id)
{
    SAlbum *album = AlbumManager::instance()->findSAlbum(id);
    if(!album)
        return;

    GPSSearchFolderItem *item = (GPSSearchFolderItem*)album->extraData(this);
    if(item)
    {
        setSelected(item, true);
        ensureItemVisible(item);
    }
}

}  // namespace Digikam
