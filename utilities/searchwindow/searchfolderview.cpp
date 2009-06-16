/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : Searches folder view
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "searchfolderview.h"
#include "searchfolderview.moc"

// Qt includes

#include <QFont>
#include <QPainter>
#include <QStyle>
#include <QCursor>

// KDE includes

#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "contextmenuhelper.h"
#include "folderitem.h"

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

// -------------------------------------------------------------------------

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
    saveViewState();
}

void SearchFolderView::slotTextSearchFilterChanged(const SearchTextSettings& settings)
{
    QString search       = settings.text;
    bool atleastOneMatch = false;

    AlbumList sList = AlbumManager::instance()->allSAlbums();
    for (AlbumList::const_iterator it = sList.constBegin(); it != sList.constEnd(); ++it)
    {
        SAlbum* salbum = (SAlbum*)(*it);
        if (!salbum->isNormalSearch())
            continue;

        SearchFolderItem* viewItem = (SearchFolderItem*) salbum->extraData(this);

        // Use viewItem text rather than album title for Last/Current Search switch
        if (viewItem->text(0).contains(search, settings.caseSensitive))
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
    return QString("_Current_Search_View_Search_");
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
    item->setPixmap(0, SmallIcon("edit-find", AlbumSettings::instance()->getTreeViewIconSize()));
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
    SearchFolderItem* sItem = dynamic_cast<SearchFolderItem*>(item);

    // temporary actions  -------------------------------------

    QAction *newAction = new QAction(SmallIcon("document-new"),
                                     i18nc("Create new search", "New..."), this);
    QAction *edtAction = new QAction(SmallIcon("edit-find"),
                                     i18nc("Edit selected search", "Edit..."), this);
    QAction *delAction = new QAction(SmallIcon("edit-delete"),
                                     i18nc("Delete selected search", "Delete"), this);

    if (!item)
    {
        delAction->setEnabled(false);
        edtAction->setEnabled(false);
    }
    if (item == m_currentSearchViewSearchItem)
        delAction->setEnabled(false);

    // --------------------------------------------------------

    KMenu popmenu(this);
    popmenu.addTitle(SmallIcon("digikam"),  i18n("My Searches"));
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction(newAction);
    popmenu.addSeparator();
    cmhelper.addAction(edtAction);
    cmhelper.addAction(delAction);

    // special action handling --------------------------------

    QAction* choice = cmhelper.exec(QCursor::pos());
    if (choice)
    {
        if (choice == newAction)
        {
            emit newSearch();
        }
        else if (choice == edtAction)
        {
            emit editSearch(sItem->album());
        }
        else if (choice == delAction)
        {
            searchDelete(sItem->album());
        }
    }
}

void SearchFolderView::slotDoubleClicked(Q3ListViewItem* item, const QPoint&, int)
{
    if (!item)
        return;

    SearchFolderItem* sItem = dynamic_cast<SearchFolderItem*>(item);
    emit editSearch(sItem->album());
}

void SearchFolderView::selectItem(int id)
{
    SAlbum *album = AlbumManager::instance()->findSAlbum(id);
    if (!album)
        return;

    SearchFolderItem *item = (SearchFolderItem*)album->extraData(this);
    if (item)
    {
        setSelected(item, true);
        ensureItemVisible(item);
    }
}

}  // namespace Digikam
