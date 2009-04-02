/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-27
 * Description : Fuzzy search folder view
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

#include "fuzzysearchfolderview.h"
#include "fuzzysearchfolderview.moc"

// Qt includes.

#include <QFont>
#include <QPainter>
#include <QStyle>
#include <QCursor>

// KDE includes.

#include <kdebug.h>
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

class FuzzySearchFolderItem : public FolderItem
{

public:

    FuzzySearchFolderItem(Q3ListView* parent, SAlbum* album)
        : FolderItem(parent, album->title()),
          m_album(album)
    {
        m_album->setExtraData(parent, this);
    }

    ~FuzzySearchFolderItem()
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

FuzzySearchFolderView::FuzzySearchFolderView(QWidget* parent)
                     : FolderView(parent, "FuzzySearchFolderView")
{
    addColumn(i18n("My Fuzzy Searches"));
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

FuzzySearchFolderView::~FuzzySearchFolderView()
{
    saveViewState();
}

QString FuzzySearchFolderView::currentFuzzySketchSearchName()
{
    return QString("_Current_Fuzzy_Sketch_Search_");
}

QString FuzzySearchFolderView::currentFuzzyImageSearchName()
{
    return QString("_Current_Fuzzy_Image_Search_");
}

void FuzzySearchFolderView::slotTextSearchFilterChanged(const SearchTextSettings& settings)
{
    QString search       = settings.text;
    bool atleastOneMatch = false;

    AlbumList sList = AlbumManager::instance()->allSAlbums();
    for (AlbumList::const_iterator it = sList.constBegin(); it != sList.constEnd(); ++it)
    {
        SAlbum* salbum                  = (SAlbum*)(*it);
        FuzzySearchFolderItem* viewItem = (FuzzySearchFolderItem*) salbum->extraData(this);

        if (viewItem &&
            viewItem->text(0).contains(search, settings.caseSensitive))
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

void FuzzySearchFolderView::searchDelete(SAlbum* album)
{
    if (!album)
        return;

    if (album->title() == currentFuzzySketchSearchName() ||
        album->title() == currentFuzzyImageSearchName())
        return;

    // Make sure that a complicated search is not deleted accidentally
    int result = KMessageBox::warningYesNo(this, i18n("Are you sure you want to "
                                                      "delete the selected Fuzzy Search "
                                                      "\"%1\"?", album->title()),
                                           i18n("Delete Fuzzy Search?"),
                                           KGuiItem(i18n("Delete")),
                                           KStandardGuiItem::cancel());

    if (result != KMessageBox::Yes)
        return;

    AlbumManager::instance()->deleteSAlbum(album);
}

void FuzzySearchFolderView::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum *salbum  = dynamic_cast<SAlbum*>(a);
    if (!salbum) return;

    if (!salbum->isHaarSearch())
        return;

    FuzzySearchFolderItem* item = new FuzzySearchFolderItem(this, salbum);
    item->setPixmap(0, SmallIcon("tools-wizard", AlbumSettings::instance()->getTreeViewIconSize()));

    if (salbum->title() == currentFuzzySketchSearchName())
        item->setText(0, i18n("Current Fuzzy Sketch Search"));

    if (salbum->title() == currentFuzzyImageSearchName())
        item->setText(0, i18n("Current Fuzzy Image Search"));
}

void FuzzySearchFolderView::slotAlbumCurrentChanged(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum *salbum  = dynamic_cast<SAlbum*>(a);
    if (!salbum) return;

    if (!salbum->isHaarSearch())
        return;

    FuzzySearchFolderItem* item = (FuzzySearchFolderItem*) salbum->extraData(this);
    if (item)
        setCurrentItem(item);
}

void FuzzySearchFolderView::slotAlbumDeleted(Album* a)
{
    if (!a || a->type() != Album::SEARCH)
        return;

    SAlbum* album = (SAlbum*)a;

    FuzzySearchFolderItem* item = (FuzzySearchFolderItem*) album->extraData(this);
    if (item)
        delete item;
}

void FuzzySearchFolderView::slotAlbumRenamed(Album* album)
{
    if (!album)
        return;

    SAlbum* salbum = dynamic_cast<SAlbum*>(album);
    if (!salbum)
        return;

    FuzzySearchFolderItem* item = (FuzzySearchFolderItem*)(salbum->extraData(this));
    if (item)
        item->setText(0, item->album()->title());
}

void FuzzySearchFolderView::slotSelectionChanged()
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

    FuzzySearchFolderItem* searchItem = dynamic_cast<FuzzySearchFolderItem*>(selItem);

    if (!searchItem || !searchItem->album())
        emit signalAlbumSelected(0);
    else
        emit signalAlbumSelected(searchItem->album());
}

void FuzzySearchFolderView::slotContextMenu(Q3ListViewItem* item, const QPoint&, int)
{
    if (!item) return;

    FuzzySearchFolderItem* sItem = dynamic_cast<FuzzySearchFolderItem*>(item);

    KMenu popmenu(this);
    popmenu.addTitle(SmallIcon("digikam"), i18n("My Fuzzy Searches"));
    QAction *renSearch = popmenu.addAction(SmallIcon("edit-rename"), i18n("Rename..."));
    QAction *delSearch = popmenu.addAction(SmallIcon("edit-delete"), i18n("Delete"));

    if (sItem->album()->title() == currentFuzzySketchSearchName() ||
        sItem->album()->title() == currentFuzzyImageSearchName())
    {
        renSearch->setEnabled(false);
        delSearch->setEnabled(false);
    }

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

void FuzzySearchFolderView::selectItem(int id)
{
    SAlbum *album = AlbumManager::instance()->findSAlbum(id);
    if(!album)
        return;

    FuzzySearchFolderItem *item = (FuzzySearchFolderItem*)album->extraData(this);
    if(item)
    {
        setSelected(item, true);
        ensureItemVisible(item);
    }
}

}  // namespace Digikam
