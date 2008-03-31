/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : Searches folder view 
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

// Qt includes.

#include <QFont>
#include <QPainter>
#include <QStyle>
#include <QCursor>

// KDe includes.

#include <kmenu.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "ddebug.h"
#include "searchquickdialog.h"
#include "searchadvanceddialog.h"
#include "folderitem.h"
#include "searchfolderview.h"
#include "searchfolderview.moc"

namespace Digikam
{

class SearchFolderItem : public FolderItem
{

public:

    SearchFolderItem(Q3ListView* parent, SAlbum* album)
        : FolderItem(parent, album->title()),
          m_album(album)
    {
        m_album->setExtraData(parent, this);
    }

    ~SearchFolderItem()
    {
        m_album->removeExtraData(listView());
    }

    int compare(Q3ListViewItem* i, int , bool ) const
    {
        if (!i)
            return 0;

        SearchFolderItem *item = static_cast<SearchFolderItem*>(i);

        if (m_album->title() == SearchFolderView::currentSearchViewSearchName())
            return -1;

        if (item->m_album->title() == SearchFolderView::currentSearchViewSearchName())
            return 1;

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

    void setCurrentSearchViewSearchTitle()
    {
        if (isSelected())
            setText(0, i18n("Current Search"));
        else
            setText(0, i18n("Last Search"));
    }

private:

    SAlbum *m_album;
};

SearchFolderView::SearchFolderView(QWidget* parent)
                : FolderView(parent, "SearchFolderView")
{
    addColumn(i18n("My Searches"));
    setResizeMode(Q3ListView::LastColumn);
    setRootIsDecorated(false);

    m_lastAddedItem = 0;
    m_currentSearchViewSearchItem = 0;

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(clear()));

    connect(this, SIGNAL(contextMenuRequested(Q3ListViewItem*, const QPoint&, int)),
            this, SLOT(slotContextMenu(Q3ListViewItem*, const QPoint&, int)));

    connect(this, SIGNAL(doubleClicked(Q3ListViewItem*, const QPoint&, int)),
            this, SLOT(slotDoubleClicked(Q3ListViewItem*, const QPoint&, int)));

    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

SearchFolderView::~SearchFolderView()
{
}

void SearchFolderView::slotTextSearchFilterChanged(const QString& filter)
{
    QString search = filter.toLower();

    bool atleastOneMatch = false;

    AlbumList sList = AlbumManager::instance()->allSAlbums();
    for (AlbumList::iterator it = sList.begin(); it != sList.end(); ++it)
    {
        SAlbum* salbum             = (SAlbum*)(*it);
        if (!salbum->isNormalSearch())
            continue;

        SearchFolderItem* viewItem = (SearchFolderItem*) salbum->extraData(this);

        // Check if a special url query exist to identify a SAlbum dedicaced to Date Search
        // used with TimeLine.
        KUrl url     = salbum->kurl();

        if (salbum->title().toLower().contains(search))
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

void SearchFolderView::slotSelectSearch(SAlbum *salbum)
{
    if (!salbum)
    {
        clearSelection();
        return;
    }

    SearchFolderItem* viewItem = (SearchFolderItem*) salbum->extraData(this);
    if (viewItem)
    {
/*        if (viewItem->isSelected())
            slotSelectionChanged();
        else
            setSelected(viewItem, true);*/
        clearSelection();
        setSelected(viewItem, true);
    }
}

/*
void SearchFolderView::quickSearchNew()
{
    KUrl url;
    SearchQuickDialog dlg(this, url);

    if (!dlg.exec())
        return;

    // Check if there is not already an album with that namespace
    // and return if user aborts the dialog.
    if ( ! checkName( url ) )
        return;

    SAlbum* album = AlbumManager::instance()->createSAlbum(url.queryItem("name"), DatabaseSearch::LegacyUrlSearch, url.url());

    if (album)
    {
        SearchFolderItem* searchItem = (SearchFolderItem*)(album->extraData(this));
        if (searchItem)
        {
            clearSelection();
            setSelected(searchItem, true);
            slotSelectionChanged();
        }
    }
}

void SearchFolderView::extendedSearchNew()
{
    KUrl url;
    SearchAdvancedDialog dlg(this, url);

    if (!dlg.exec())
        return;

    // Check if there is not already an album with that name
    // and return if user aborts the dialog.
    if ( ! checkName( url ) )
        return;

    SAlbum* album = AlbumManager::instance()->createSAlbum(url.queryItem("name"), DatabaseSearch::LegacyUrlSearch, url.url());

    if (album)
    {
        SearchFolderItem* searchItem = (SearchFolderItem*)(album->extraData(this));
        if (searchItem)
        {
            clearSelection();
            setSelected(searchItem, true);
            slotSelectionChanged();
        }
    }
}

bool SearchFolderView::checkName( KUrl& url )
{
    QString albumTitle     = url.queryItem("name");
    AlbumManager* aManager = AlbumManager::instance();
    AlbumList aList        = aManager->allSAlbums();
    bool checked           = checkAlbum( albumTitle );

    while ( !checked) 
    {
        QString label = i18n( "Search name already exists."
                              "\nPlease enter a new name:" );
        bool ok;
        QString newTitle = KInputDialog::getText( i18n("Name exists"), label,
                                                  albumTitle, &ok, this );
        if (!ok)
            return false;

        albumTitle=newTitle;
        checked = checkAlbum( albumTitle );
    }

    url.removeQueryItem( "name" );
    url.addQueryItem( "name", albumTitle );
    return true;
}

bool SearchFolderView::checkAlbum( const QString& name ) const
{

    AlbumManager* aManager = AlbumManager::instance();
    AlbumList aList        = aManager->allSAlbums();

    for ( AlbumList::Iterator it = aList.begin();
          it != aList.end(); ++it )
    {
        SAlbum *album = (SAlbum*)(*it);
        if ( album->title() == name )
            return false;
    }
    return true;
}

void SearchFolderView::quickSearchEdit(SAlbum* album)
{
    if (!album)
        return;

    KUrl url = album->kurl();
    SearchQuickDialog dlg(this, url);

    if (dlg.exec() != KDialog::Accepted)
        return;

    AlbumManager::instance()->updateSAlbum(album, url.url());

    ((SearchFolderItem*)album->extraData(this))->setText(0, album->title());

    clearSelection();
    setSelected((SearchFolderItem*)(album->extraData(this)), true);
}

void SearchFolderView::extendedSearchEdit(SAlbum* album)
{
    if (!album)
        return;

    KUrl url = album->kurl();
    SearchAdvancedDialog dlg(this, url);

    if (dlg.exec() != KDialog::Accepted)
        return;

    AlbumManager::instance()->updateSAlbum(album, url.url());

    ((SearchFolderItem*)album->extraData(this))->setText(0, album->title());

    clearSelection();
    setSelected((SearchFolderItem*)(album->extraData(this)), true);
}
*/

void SearchFolderView::searchDelete(SAlbum* album)
{
    if (!album)
        return;

    // Make sure that a complicated search is not deleted accidentally
    int result = KMessageBox::warningYesNo(this, i18n("Are you sure you want to "
                                                      "delete the selected search "
                                                      "\"%1\"?", album->title()),
                                           i18n("Delete Search?"),
                                           KGuiItem(i18n("Delete")),
                                           KStandardGuiItem::cancel());

    if (result != KMessageBox::Yes)
        return;

    AlbumManager::instance()->deleteSAlbum(album);
}

QString SearchFolderView::currentSearchViewSearchName()
{
    return "_Current_Search_View_Search_";
}

void SearchFolderView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum* album = (SAlbum*)a;

    // Check if this is an SAlbum dedicated to the Date Search View
    // used with TimeLine. In this case, SAlbum is not displayed here, but in TimeLineFolderView.
    if (!album->isNormalSearch())
        return;

    SearchFolderItem* item = new SearchFolderItem(this, album);
    item->setPixmap(0, SmallIcon("find", AlbumSettings::instance()->getDefaultTreeIconSize()));
    m_lastAddedItem = item;

    if (album->title() == currentSearchViewSearchName())
    {
        m_currentSearchViewSearchItem = item;
        item->setCurrentSearchViewSearchTitle();
    }
}

void SearchFolderView::slotAlbumDeleted(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum* album = (SAlbum*)a;

    SearchFolderItem* item = (SearchFolderItem*) album->extraData(this);
    if (item)
        delete item;
}

void SearchFolderView::slotSelectionChanged()
{
    if (!active())
        return;

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
        AlbumManager::instance()->setCurrentAlbum(0);
        return;
    }

    SearchFolderItem* searchItem = dynamic_cast<SearchFolderItem*>(selItem);

    if (!searchItem || !searchItem->album())
    {
        AlbumManager::instance()->setCurrentAlbum(0);
        emit selectedSearchChanged(0);
    }
    else
    {
        AlbumManager::instance()->setCurrentAlbum(searchItem->album());
        emit selectedSearchChanged(searchItem->album());
    }

    if (m_currentSearchViewSearchItem)
        m_currentSearchViewSearchItem->setCurrentSearchViewSearchTitle();
}

void SearchFolderView::slotContextMenu(Q3ListViewItem* item, const QPoint&, int)
{
    /*
    if (!item)
    {
        KMenu popmenu(this);
        popmenu.addTitle(SmallIcon("digikam"),  i18n("My Searches"));
        QAction *smpSearch = popmenu.addAction(SmallIcon("filefind"), i18n("New Simple Search..."));
        QAction *advSearch = popmenu.addAction(SmallIcon("edit-find"), i18n("New Advanced Search..."));
        QAction *choice    = popmenu.exec(QCursor::pos());
        if (choice)
        {
            if (choice == smpSearch)
            {
                quickSearchNew();
            }
            else if (choice == advSearch)
            {
                extendedSearchNew();
            }
        }
    }
    else
    */
    if (item)
    {
        SearchFolderItem* sItem = dynamic_cast<SearchFolderItem*>(item);
        // QAction *edtadvSearch   = 0;

        KMenu popmenu(this);
        popmenu.addTitle(SmallIcon("digikam"),  i18n("My Searches"));
        QAction *edtSearch = popmenu.addAction(SmallIcon("filefind"), i18n("Edit Search..."));

        /*
        if ( sItem->album()->isKeywordSearch() )
        {
            edtadvSearch = popmenu.addAction(SmallIcon("find"), i18n("Edit as Advanced Search..."));
            popmenu.insertSeparator(edtadvSearch);
        }
        else
        {
            popmenu.insertSeparator(edtsmpSearch);
        }
        */

        QAction *delSearch = popmenu.addAction(SmallIcon("edit-delete"), i18n("Delete Search"));
        if (item == m_currentSearchViewSearchItem)
            delSearch->setEnabled(false);
        QAction *choice    = popmenu.exec(QCursor::pos());
        if (choice)
        {
            /*
            if (choice == edtsmpSearch)
            {
                if (sItem->album()->isKeywordSearch())
                    quickSearchEdit(sItem->album());
                else
                    extendedSearchEdit(sItem->album());
            }
            else if (choice == edtadvSearch)
            {
                extendedSearchEdit(sItem->album());
            }
            */
            if (choice == edtSearch)
            {
                emit editSearch(sItem->album());
            }
            else if (choice == delSearch)
            {
                searchDelete(sItem->album());
            }
        }
    }
}

void SearchFolderView::slotDoubleClicked(Q3ListViewItem* item, const QPoint&, int)
{
    if (!item)
        return;

    SearchFolderItem* sItem = dynamic_cast<SearchFolderItem*>(item);
    emit editSearch(sItem->album());

    /*
    if (sItem->album()->isKeywordSearch())
        quickSearchEdit(sItem->album());
    else
        extendedSearchEdit(sItem->album());
    */
}

void SearchFolderView::selectItem(int id)
{
    SAlbum *album = AlbumManager::instance()->findSAlbum(id);
    if(!album)
        return;

    SearchFolderItem *item = (SearchFolderItem*)album->extraData(this);
    if(item)
    {
        setSelected(item, true);
        ensureItemVisible(item);
    }
}

}  // namespace Digikam
