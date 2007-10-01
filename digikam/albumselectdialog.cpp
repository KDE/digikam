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
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QCursor>
#include <QDateTime>
#include <QGridLayout>
#include <QPixmap>

// KDE includes.

#include <kmenu.h>
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
#include "collectionmanager.h"
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
    }

    bool                        allowRootSelection;

    QString                     newAlbumString;

    QMap<FolderItem*, PAlbum*>  albumMap;

    FolderView                 *folderView;
};

AlbumSelectDialog::AlbumSelectDialog(QWidget* parent, PAlbum* albumToSelect,
                                     const QString& header,
                                     const QString& newAlbumString,
                                     bool allowRootSelection )
             : KDialog(parent)
{
    setCaption(i18n("Select Album"));
    setButtons(Help|User1|Ok|Cancel);
    setButtonText(User1,i18n("&New Album"));
    setDefaultButton(Ok);
    setHelp("targetalbumdialog.anchor", "digikam");
    enableButtonOk(false);

    d = new AlbumSelectDialogPrivate;
    d->allowRootSelection = allowRootSelection;
    d->newAlbumString     = newAlbumString;

    QWidget *page = new QWidget(this);
    setMainWidget(page);   

    QGridLayout* grid = new QGridLayout(page);

    QLabel *logo = new QLabel(page);
    KIconLoader* iconLoader = KIconLoader::global();
    logo->setPixmap(iconLoader->loadIcon("digikam", KIconLoader::NoGroup, 128));  

    QLabel *message = new QLabel(page);
    message->setWordWrap(true);
    if (!header.isEmpty())
        message->setText(header);

    d->folderView = new FolderView(page);
    d->folderView->addColumn(i18n("digiKam Albums"));
    d->folderView->setColumnWidthMode( 0, Q3ListView::Maximum );
    d->folderView->setResizeMode( Q3ListView::AllColumns );
    d->folderView->setRootIsDecorated(true);

    grid->addWidget(logo, 0, 0, 1, 1);
    grid->addWidget(message, 1, 0, 1, 1);
    grid->addWidget(d->folderView, 0, 1, 3, 1);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(2, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    QPixmap icon = iconLoader->loadIcon("folder", KIconLoader::NoGroup,
                                        AlbumSettings::instance()->getDefaultTreeIconSize());

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

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(d->folderView, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->folderView, SIGNAL(contextMenuRequested(Q3ListViewItem*, const QPoint&, int)),
            this, SLOT(slotContextMenu(Q3ListViewItem*, const QPoint&, int)));

    // -------------------------------------------------------------

    connect(this,SIGNAL(user1Clicked()),this,SLOT(slotUser1()));

    resize(500, 500);
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

    KIconLoader* iconLoader = KIconLoader::global();
    QPixmap icon = iconLoader->loadIcon("folder", KIconLoader::NoGroup,
                                        AlbumSettings::instance()->getDefaultTreeIconSize());
    
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
    Q3ListViewItem* selItem = 0;
    Q3ListViewItemIterator it(d->folderView);
    
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
        enableButtonOk(false);
        return;
    }

    enableButtonOk(true);
}

void AlbumSelectDialog::slotContextMenu(Q3ListViewItem *, const QPoint &, int)
{
    KMenu popmenu(d->folderView);
    
    KAction *action = new KAction(KIcon("albumfolder-new"), i18n("Create New Album"), this);
    connect(action, SIGNAL(triggered(bool) ), 
            this, SLOT(slotUser1()));

    popmenu.addAction(action);
    popmenu.exec(QCursor::pos());
}

void AlbumSelectDialog::slotUser1()
{
    Q3ListViewItem* item = d->folderView->currentItem();
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
                                                      "Enter album name:",
                                                      album->prettyUrl()),
                                                 d->newAlbumString, &ok, this);
    if (!ok)
        return;

    // if we create an album under root, need to supply the album root path.
    QString albumRootPath;
    if (album->isRoot())
    {
        // TODO: Let user choose an album root
        albumRootPath = CollectionManager::instance()->oneAlbumRootPath();
    }

    QString errMsg;
    PAlbum* newAlbum = AlbumManager::instance()->createPAlbum(album, albumRootPath, newAlbumName,
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

    if (dlg.exec() != KDialog::Accepted)
        return 0;

    FolderItem* item = (FolderItem*) dlg.d->folderView->currentItem();
    if (!item || (item == dlg.d->folderView->firstChild()) &&
        !allowRootSelection)
    {
        return 0;
    }
    
    return dlg.d->albumMap[item];
}

}  // namespace Digikam
