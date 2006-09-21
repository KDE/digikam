/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2005-06-16
 * Description : a dialog to select a target album to downoad
 *               pictures from camera
 * 
 * Copyright 2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

// KDE includes.

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kaction.h>
#include <kinputdialog.h>
#include <kmessagebox.h>

// Local includes.

#include "folderview.h"
#include "folderitem.h"
#include "album.h"
#include "albummanager.h"
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
    
    QGridLayout* grid = new QGridLayout(plainPage(), 2, 1, 0, spacingHint());

    QLabel *logo = new QLabel(plainPage());
    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    logo->setPixmap(iconLoader->loadIcon("digikam", KIcon::NoGroup, 128, KIcon::DefaultState, 0, true));    

    QLabel *message = new QLabel(plainPage());
    if (!header.isEmpty())
        message->setText(header);

    d->folderView = new FolderView(plainPage());
    d->folderView->addColumn(i18n("digiKam Albums"));
    d->folderView->setColumnWidthMode( 0, QListView::Maximum );
    d->folderView->setResizeMode( QListView::AllColumns );
    d->folderView->setRootIsDecorated(true);

    grid->addMultiCellWidget(logo, 0, 0, 0, 0);
    grid->addMultiCellWidget(message, 1, 1, 0, 0);
    grid->addMultiCellWidget(d->folderView, 0, 2, 1, 1);
    grid->setRowStretch(2, 10);

    QPixmap icon = iconLoader->loadIcon("folder", KIcon::NoGroup,
                                        32, KIcon::DefaultState, 0, true);

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
                kdWarning() << "Failed to find parent for Album "
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

    connect(d->folderView, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            this, SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));

    // -------------------------------------------------------------

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
        kdWarning() << "Failed to find parent for Album "
                    << album->title() << endl;
        return;
    }

    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
    QPixmap icon = iconLoader->loadIcon("folder", KIcon::NoGroup,
                                        32, KIcon::DefaultState, 0, true);
    
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
                                  "albumfoldernew", 0, this,
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

}  // namespace Digikam

