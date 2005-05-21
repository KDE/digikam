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
#include <qpopupmenu.h>
#include <qcursor.h>

#include "album.h"
#include "albummanager.h"
#include "searchquickdialog.h"
#include "searchfolderview.h"

class SearchFolderItem : public KListViewItem
{
public:

    SearchFolderItem(KListView* parent, SAlbum* album)
        : KListViewItem(parent, album->getName()),
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
    connect(this,
            SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));

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
    AlbumManager::instance()->createSAlbum(url, true, renamedAlbum);

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

void SearchFolderView::extendedSearchNew()
{
/* TODO:
   add extended search
*/    
}

void SearchFolderView::quickSearchEdit(SAlbum* album)
{
    if (!album)
        return;

    KURL url = album->getKURL();
    SearchQuickDialog dlg(this, url);

    if (dlg.exec() != KDialogBase::Accepted)
        return;


    AlbumManager::instance()->updateSAlbum(album, url);

    ((SearchFolderItem*)album->getViewItem())->setText(0, album->getName());

    clearSelection();
    setSelected((SearchFolderItem*)(album->getViewItem()), true);
}

void SearchFolderView::extendedSearchEdit(SAlbum* album)
{
    if (!album)
        return;

/* TODO:
   add extended search
*/
}

void SearchFolderView::searchDelete(SAlbum* album)
{
    if (!album)
        return;

    AlbumManager::instance()->deleteSAlbum(album);
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

void SearchFolderView::slotContextMenu(QListViewItem* item, const QPoint&, int)
{
    if (!item)
    {
        QPopupMenu popmenu(this);
        popmenu.insertItem(SmallIcon("find"), i18n("New Simple Search..."), 10);
        popmenu.insertItem(SmallIcon("find"), i18n("New Extended Search..."), 11);

        switch (popmenu.exec(QCursor::pos()))
        {
        case 10:
        {
            quickSearchNew();
            break;
        }
        case 11:
        {
            extendedSearchNew();
            break;
        }
        default:
            break;
        }
    }
    else
    {
        QPopupMenu popmenu(this);
        popmenu.insertItem(SmallIcon("find"), i18n("Edit Search..."), 10);
        popmenu.insertItem(SmallIcon("editdelete"), i18n("Delete Search"), 11);

        SearchFolderItem* sItem = dynamic_cast<SearchFolderItem*>(item);
        
        switch (popmenu.exec(QCursor::pos()))
        {
        case 10:
        {
            if (sItem->m_album->isSimple())
                quickSearchEdit(sItem->m_album);
            else
                extendedSearchEdit(sItem->m_album);
            break;
        }
        case 11:
        {
            searchDelete(sItem->m_album);
            break;
        }
        default:
            break;
        }
    }
}

#include "searchfolderview.moc"
