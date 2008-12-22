/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-16
 * Description : a dialog to select a target album to download
 *               pictures from camera
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

#include <qlabel.h>
#include <qframe.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qdatetime.h>
#include <qmap.h>

// KDE includes.

#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kaction.h>
#include <kinputdialog.h>
#include <kmessagebox.h>

// Local includes.

#include "ddebug.h"
#include "folderview.h"
#include "folderitem.h"
#include "album.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "searchtextbar.h"
#include "albumselectdialog.h"
#include "albumselectdialog.moc"

namespace Digikam
{

class AlbumSelectDialogPrivate
{

public:

    AlbumSelectDialogPrivate()
    {
        allowRootSelection = false;
        folderView         = 0;
        searchBar          = 0;
    }

    bool                        allowRootSelection;

    QString                     newAlbumString;

    QMap<FolderItem*, PAlbum*>  albumMap;

    FolderView                 *folderView;

    SearchTextBar              *searchBar;
};

AlbumSelectDialog::AlbumSelectDialog(QWidget* parent, PAlbum* albumToSelect,
                                     const QString& header,
                                     const QString& newAlbumString,
                                     bool allowRootSelection )
             : KDialogBase(Plain, i18n("Select Album"),
                           Help|User1|Ok|Cancel, Ok,
                           parent, 0, true, true,
                           i18n("&New Album"))
{
    d = new AlbumSelectDialogPrivate;
    setHelp("targetalbumdialog.anchor", "digikam");
    enableButtonOK(false);

    d->allowRootSelection = allowRootSelection;
    d->newAlbumString     = newAlbumString;

    // -------------------------------------------------------------

    QGridLayout* grid = new QGridLayout(plainPage(), 2, 1, 0, spacingHint());

    QLabel *logo = new QLabel(plainPage());
    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    logo->setPixmap(iconLoader->loadIcon("digikam", KIcon::NoGroup, 128, KIcon::DefaultState, 0, true));

    QLabel *message = new QLabel(plainPage());
    if (!header.isEmpty())
        message->setText(header);

    d->folderView = new FolderView(plainPage());
    d->folderView->addColumn(i18n("My Albums"));
    d->folderView->setColumnWidthMode( 0, QListView::Maximum );
    d->folderView->setResizeMode( QListView::AllColumns );
    d->folderView->setRootIsDecorated(true);

    d->searchBar = new SearchTextBar(plainPage(), "AlbumSelectDialogSearchBar");

    // -------------------------------------------------------------

    QPixmap icon = iconLoader->loadIcon("folder", KIcon::NoGroup,
                                        AlbumSettings::instance()->getDefaultTreeIconSize(), KIcon::DefaultState, 0, true);

    AlbumList aList = AlbumManager::instance()->allPAlbums();

    for (AlbumList::const_iterator it = aList.begin(); it != aList.end(); ++it)
    {
        PAlbum* album = (PAlbum*)(*it);

        FolderItem* viewItem = 0;

        if (album->isRoot())
        {
            viewItem = new FolderItem(d->folderView, album->title());
            viewItem->setOpen(true);
        }
        else
        {
            FolderItem* parentItem = (FolderItem*)(album->parent()->extraData(d->folderView));

            if (!parentItem)
            {
                DWarning() << "Failed to find parent for Album "
                            << album->title() << endl;
                continue;
            }

            viewItem = new FolderItem(parentItem, album->title());
        }

        if (viewItem)
        {
            viewItem->setPixmap(0, icon);
            album->setExtraData(d->folderView, viewItem);
            d->albumMap.insert(viewItem, album);

            if (album == albumToSelect)
            {
                viewItem->setOpen(true);
                d->folderView->setSelected(viewItem, true);
                d->folderView->ensureItemVisible(viewItem);
            }
        }
    }

    // -------------------------------------------------------------

    grid->addMultiCellWidget(logo,          0, 0, 0, 0);
    grid->addMultiCellWidget(message,       1, 1, 0, 0);
    grid->addMultiCellWidget(d->folderView, 0, 2, 1, 1);
    grid->addMultiCellWidget(d->searchBar,  3, 3, 1, 1);
    grid->setRowStretch(2, 10);

    // -------------------------------------------------------------

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(d->folderView, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->folderView, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            this, SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));

    connect(d->searchBar, SIGNAL(signalTextChanged(const QString&)),
            this, SLOT(slotSearchTextChanged(const QString&)));

    // -------------------------------------------------------------

    resize(650, 650);
    slotSelectionChanged();
}

AlbumSelectDialog::~AlbumSelectDialog()
{
    delete d;
}

void AlbumSelectDialog::slotAlbumAdded(Album* album)
{
    if (!album || album->type() != Album::PHYSICAL)
        return;

    FolderItem* parentItem = (FolderItem*)(album->parent()->extraData(d->folderView));

    if (!parentItem)
    {
        DWarning() << "Failed to find parent for Album "
                   << album->title() << endl;
        return;
    }

    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
    QPixmap icon            = iconLoader->loadIcon("folder", KIcon::NoGroup,
                                                   AlbumSettings::instance()->getDefaultTreeIconSize(),
                                                   KIcon::DefaultState, 0, true);

    FolderItem* viewItem = new FolderItem(parentItem, album->title());
    viewItem->setPixmap(0, icon);
    album->setExtraData(d->folderView, viewItem);
    d->albumMap.insert(viewItem, (PAlbum*)album);
}

void AlbumSelectDialog::slotAlbumDeleted(Album* album)
{
    if (!album || album->type() != Album::PHYSICAL)
        return;

    FolderItem* viewItem = (FolderItem*)(album->extraData(d->folderView));

    if (viewItem)
    {
        delete viewItem;
        album->removeExtraData(d->folderView);
        d->albumMap.remove(viewItem);
    }
}

void AlbumSelectDialog::slotAlbumsCleared()
{
    d->folderView->clear();
}

void AlbumSelectDialog::slotSelectionChanged()
{
    QListViewItem* selItem = 0;
    QListViewItemIterator it(d->folderView);

    while (it.current())
    {
        if (it.current()->isSelected())
        {
            selItem = it.current();
            break;
        }
        ++it;
    }

    if (!selItem || (selItem == d->folderView->firstChild()) &&
        !d->allowRootSelection)
    {
        enableButtonOK(false);
        return;
    }

    enableButtonOK(true);
}

void AlbumSelectDialog::slotContextMenu(QListViewItem *, const QPoint &, int)
{
    QPopupMenu popmenu(d->folderView);
    KAction *action = new KAction(i18n( "Create New Album" ),
                                  "albumfolder-new", 0, this,
                                  SLOT( slotUser1() ),
                                  &popmenu);
    action->plug(&popmenu);
    popmenu.exec(QCursor::pos());
}

void AlbumSelectDialog::slotUser1()
{
    QListViewItem* item = d->folderView->currentItem();
    if (!item)
        item = d->folderView->firstChild();

    if (!item)
        return;

    PAlbum* album = d->albumMap[(FolderItem*)item];
    if (!album)
        return;

    bool ok;
    QString newAlbumName = KInputDialog::getText(i18n("New Album Name"),
                                                 i18n("Creating new album in '%1'\n"
                                                      "Enter album name:")
                                                 .arg(album->prettyURL()),
                                                 d->newAlbumString, &ok, this);
    if (!ok)
        return;

    QString errMsg;
    PAlbum* newAlbum = AlbumManager::instance()->createPAlbum(album, newAlbumName,
                                                              QString(), QDate::currentDate(),
                                                              QString(), errMsg);
    if (!newAlbum)
    {
        KMessageBox::error(this, errMsg);
        return;
    }

    FolderItem* newItem = (FolderItem*)newAlbum->extraData(d->folderView);
    if (newItem)
    {
        d->folderView->ensureItemVisible(newItem);
        d->folderView->setSelected(newItem, true);
    }
}

PAlbum* AlbumSelectDialog::selectAlbum(QWidget* parent,
                                       PAlbum* albumToSelect,
                                       const QString& header,
                                       const QString& newAlbumString,
                                       bool allowRootSelection )
{
    AlbumSelectDialog dlg(parent, albumToSelect,
                          header, newAlbumString,
                          allowRootSelection);

    if (dlg.exec() != KDialogBase::Accepted)
        return 0;

    FolderItem* item = (FolderItem*) dlg.d->folderView->currentItem();
    if (!item || (item == dlg.d->folderView->firstChild()) &&
        !allowRootSelection)
    {
        return 0;
    }

    return dlg.d->albumMap[item];
}

void AlbumSelectDialog::slotSearchTextChanged(const QString& filter)
{
    QString search = filter.lower();

    bool atleastOneMatch = false;

    AlbumList pList = AlbumManager::instance()->allPAlbums();
    for (AlbumList::iterator it = pList.begin(); it != pList.end(); ++it)
    {
        PAlbum* palbum  = (PAlbum*)(*it);

        // don't touch the root Album
        if (palbum->isRoot())
            continue;

        bool match = palbum->title().lower().contains(search);
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = palbum->parent();
            while (parent && !parent->isRoot())
            {
                if (parent->title().lower().contains(search))
                {
                    match = true;
                    break;
                }

                parent = parent->parent();
            }
        }

        if (!match)
        {
            // check if any of the children match the search
            AlbumIterator it(palbum);
            while (it.current())
            {
                if ((*it)->title().lower().contains(search))
                {
                    match = true;
                    break;
                }
                ++it;
            }
        }

        FolderItem* viewItem = (FolderItem*) palbum->extraData(d->folderView);

        if (match)
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

    d->searchBar->slotSearchResult(atleastOneMatch);
}

}  // namespace Digikam
