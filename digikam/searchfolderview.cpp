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

#include <qfont.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qcursor.h>

// KDE includes.

#include <kpopupmenu.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kmessagebox.h>

#if KDE_IS_VERSION(3,2,0)
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
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

    SearchFolderItem(QListView* parent, SAlbum* album)
        : FolderItem(parent, album->title()),
          m_album(album)
    {
        m_album->setExtraData(parent, this);
    }

    ~SearchFolderItem()
    {
        m_album->removeExtraData(listView());
    }

    int compare(QListViewItem* i, int , bool ) const
    {
        if (!i)
            return 0;

        if (text(0) == i18n("Last Search"))
            return -1;

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

SearchFolderView::SearchFolderView(QWidget* parent)
                : FolderView(parent, "SearchFolderView")
{
    addColumn(i18n("My Searches"));
    setResizeMode(QListView::LastColumn);
    setRootIsDecorated(false);

    m_lastAddedItem = 0;

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(clear()));

    connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            this, SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));

    connect(this, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
            this, SLOT(slotDoubleClicked(QListViewItem*, const QPoint&, int)));

    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

SearchFolderView::~SearchFolderView()
{
}

void SearchFolderView::slotTextSearchFilterChanged(const QString& filter)
{
    QString search = filter.lower();

    bool atleastOneMatch = false;

    AlbumList sList = AlbumManager::instance()->allSAlbums();
    for (AlbumList::iterator it = sList.begin(); it != sList.end(); ++it)
    {
        SAlbum* salbum             = (SAlbum*)(*it);
        SearchFolderItem* viewItem = (SearchFolderItem*) salbum->extraData(this);

        // Check if a special url query exist to identify a SAlbum dedicaced to Date Search
        // used with TimeLine.
        KURL url     = salbum->kurl();
        QString type = url.queryItem("type");

        if (salbum->title().lower().contains(search) &&
            type != QString("datesearch"))
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

void SearchFolderView::quickSearchNew()
{
    KURL url;
    SearchQuickDialog dlg(this, url);

    if (dlg.exec() != KDialogBase::Accepted)
        return;

    // Check if there is not already an album with that namespace
    // and return if user aborts the dialog.
    if ( ! checkName( url ) )
        return;

    SAlbum* album = AlbumManager::instance()->createSAlbum(url, true);

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
    KURL url;
    SearchAdvancedDialog dlg(this, url);

    if (dlg.exec() != KDialogBase::Accepted)
        return;

    // Check if there is not already an album with that name
    // and return if user aborts the dialog.
    if ( ! checkName( url ) )
        return;

    SAlbum* album = AlbumManager::instance()->createSAlbum(url, false);

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

bool SearchFolderView::checkName( KURL& url )
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
#if KDE_IS_VERSION(3,2,0)
        QString newTitle = KInputDialog::getText( i18n("Name exists"), label,
                                                  albumTitle, &ok, this );
#else
        QString newTitle = KLineEditDlg::getText( i18n("Name exists"), label,
                                                  albumTitle, ok, this );
#endif
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

    KURL url = album->kurl();
    SearchQuickDialog dlg(this, url);

    if (dlg.exec() != KDialogBase::Accepted)
        return;

    AlbumManager::instance()->updateSAlbum(album, url);

    ((SearchFolderItem*)album->extraData(this))->setText(0, album->title());

    clearSelection();
    setSelected((SearchFolderItem*)(album->extraData(this)), true);
}

void SearchFolderView::extendedSearchEdit(SAlbum* album)
{
    if (!album)
        return;

    KURL url = album->kurl();
    SearchAdvancedDialog dlg(this, url);

    if (dlg.exec() != KDialogBase::Accepted)
        return;

    AlbumManager::instance()->updateSAlbum(album, url);

    ((SearchFolderItem*)album->extraData(this))->setText(0, album->title());

    clearSelection();
    setSelected((SearchFolderItem*)(album->extraData(this)), true);
}

void SearchFolderView::searchDelete(SAlbum* album)
{
    if (!album)
        return;

    // Make sure that a complicated search is not deleted accidentally
    int result = KMessageBox::warningYesNo(this, i18n("Are you sure you want to "
                                                      "delete the selected search "
                                                      "\"%1\"?")
                                           .arg(album->title()),
                                           i18n("Delete Search?"),
                                           i18n("Delete"),
                                           KStdGuiItem::cancel());

    if (result != KMessageBox::Yes)
        return;

    AlbumManager::instance()->deleteSAlbum(album);
}

void SearchFolderView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum* album = (SAlbum*)a;

    // Check if a special url query exist to identify a SAlbum dedicaced to Date Search
    // used with TimeLine. In this case, SAlbum is not displayed here, but in TimeLineFolderView.
    KURL url     = album->kurl();
    QString type = url.queryItem("type");
    if (type == QString("datesearch")) return;

    SearchFolderItem* item = new SearchFolderItem(this, album);
    item->setPixmap(0, SmallIcon("find", AlbumSettings::instance()->getDefaultTreeIconSize()));
    m_lastAddedItem = item;
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

    if (!searchItem || !searchItem->album())
        AlbumManager::instance()->setCurrentAlbum(0);
    else
        AlbumManager::instance()->setCurrentAlbum(searchItem->album());
}

void SearchFolderView::slotContextMenu(QListViewItem* item, const QPoint&, int)
{
    if (!item)
    {
        KPopupMenu popmenu(this);
        popmenu.insertTitle(SmallIcon("digikam"), i18n("My Searches"));
        popmenu.insertItem(SmallIcon("filefind"), i18n("New Simple Search..."), 10);
        popmenu.insertItem(SmallIcon("find"),     i18n("New Advanced Search..."), 11);

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
        SearchFolderItem* sItem = dynamic_cast<SearchFolderItem*>(item);

        KPopupMenu popmenu(this);
        popmenu.insertTitle(SmallIcon("digikam"), i18n("My Searches"));
        popmenu.insertItem(SmallIcon("filefind"), i18n("Edit Search..."), 10);

        if ( sItem->album()->isSimple() )
            popmenu.insertItem(SmallIcon("find"), i18n("Edit as Advanced Search..."), 11);

        popmenu.insertSeparator(-1);
        popmenu.insertItem(SmallIcon("editdelete"), i18n("Delete Search"), 12);

        switch (popmenu.exec(QCursor::pos()))
        {
            case 10:
            {
                if (sItem->album()->isSimple())
                    quickSearchEdit(sItem->album());
                else
                    extendedSearchEdit(sItem->album());
                break;
            }
            case 11:
            {
                extendedSearchEdit(sItem->album());
                break;
            }
            case 12:
            {
                searchDelete(sItem->album());
                break;
            }
            default:
                break;
        }
    }
}

void SearchFolderView::slotDoubleClicked(QListViewItem* item, const QPoint&, int)
{
    if (!item)
        return;

    SearchFolderItem* sItem = dynamic_cast<SearchFolderItem*>(item);

    if (sItem->album()->isSimple())
        quickSearchEdit(sItem->album());
    else
        extendedSearchEdit(sItem->album());
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
