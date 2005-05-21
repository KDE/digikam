/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-21
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

#include <qfont.h>
#include <qpainter.h>
#include <qstyle.h>

#include "album.h"
#include "albummanager.h"
#include "searchquickdialog.h"
#include "searchfolderview.h"

class SearchFolderItem : public KListViewItem
{
public:

    SearchFolderItem(KListView* parent, SAlbum* album)
        : KListViewItem(parent, album->getKURL().queryItem("name")),
          m_album(album)
    {
        m_album->setViewItem(this);
    }

    ~SearchFolderItem()
    {
        m_album->setViewItem(0);
    }

    int compare(QListViewItem* i, int , bool ) const
    {
        if (!i)
            return 0;
        
        if (text(0) == i18n("Last Search"))
            return -1;

        return text(0).localeAwareCompare(i->text(0));
    }
    
    SAlbum* m_album;
};

SearchFolderView::SearchFolderView(QWidget* parent)
    : KListView(parent)
{
    addColumn(i18n("My Searches"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(false);

    m_active = false;
    m_lastAddedItem = 0;

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotAlbumAdded(Album*)));
    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            SLOT(slotAlbumDeleted(Album*)));
    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(clear()));

    connect(this, SIGNAL(selectionChanged()),
            SLOT(slotSelectionChanged()));
}

SearchFolderView::~SearchFolderView()
{
    
}

void SearchFolderView::quickSearchNew()
{
    KURL url;
    SearchQuickDialog dlg(this, url);

    if (dlg.exec() != KDialogBase::Accepted)
        return;

    SAlbum* renamedAlbum = 0;
    AlbumManager::instance()->createSAlbum(url, renamedAlbum);

    if (renamedAlbum)
    {
        SearchFolderItem* searchItem =
            (SearchFolderItem*)(renamedAlbum->getViewItem());
        if (searchItem)
        {
            clearSelection();
            setSelected(searchItem, true);
            slotSelectionChanged();
        }
    }
    else if (m_lastAddedItem)
    {
        clearSelection();
        setSelected(m_lastAddedItem, true);
        m_lastAddedItem = 0;
    }
}

void SearchFolderView::setActive(bool val)
{
    m_active = val;
    if (m_active)
    {
        slotSelectionChanged();
    }
}

void SearchFolderView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum* album = (SAlbum*)a;

    SearchFolderItem* item = new SearchFolderItem(this, album);
    item->setPixmap(0, SmallIcon("find", 22));
    m_lastAddedItem = item;
}

void SearchFolderView::slotAlbumDeleted(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum* album = (SAlbum*)a;

    SearchFolderItem* item = (SearchFolderItem*) album->getViewItem();
    if (item)
    {
        delete item;
    }
}

void SearchFolderView::slotSelectionChanged()
{
    if (!m_active)
        return;
    
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
        AlbumManager::instance()->setCurrentAlbum(0);
        return;
    }

    SearchFolderItem* searchItem = dynamic_cast<SearchFolderItem*>(selItem);
    
    if (!searchItem || !searchItem->m_album)
    {
        AlbumManager::instance()->setCurrentAlbum(0);
    }
    else
    {
        AlbumManager::instance()->setCurrentAlbum(searchItem->m_album);
    }
}

#include "searchfolderview.moc"
