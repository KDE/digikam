/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-06-16
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju
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

namespace Digikam
{

AlbumSelectDialog::AlbumSelectDialog(QWidget* parent, PAlbum* albumToSelect,
                                     const QString& header,
                                     const QString& newAlbumString,
                                     bool allowRootSelection )
    : KDialogBase(parent, 0, true, i18n("Select Album"),
                  Help|User1|Ok|Cancel)
{
    setButtonText(User1, i18n("&New Album"));
    setHelp("targetalbumdialog.anchor", "digikam");
    enableButtonOK(false);

    m_allowRootSelection = allowRootSelection;
    m_newAlbumString     = newAlbumString;
    
    QFrame *page     = makeMainWidget();
    QVBoxLayout* lay = new QVBoxLayout(page, 5, 5);

    if (!header.isEmpty())
    {
        QLabel* head = new QLabel(header, page);
        lay->addWidget(head);
        QFrame* hline = new QFrame(page);
        hline->setFrameStyle(QFrame::Sunken|QFrame::HLine);
        lay->addWidget(hline);
    }

    m_folderView = new FolderView( page );
    m_folderView->addColumn( i18n("Albums") );
    m_folderView->setColumnWidthMode( 0, QListView::Maximum );
    m_folderView->setResizeMode( QListView::AllColumns );
    m_folderView->setRootIsDecorated(true);
    lay->addWidget(m_folderView);    

    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
    QPixmap icon = iconLoader->loadIcon("folder", KIcon::NoGroup,
                                        32, KIcon::DefaultState, 0, true);

    AlbumList aList = AlbumManager::instance()->allPAlbums();
    for (AlbumList::const_iterator it = aList.begin(); it != aList.end(); ++it)
    {
        PAlbum* album = (PAlbum*)(*it);

        FolderItem* viewItem = 0;
        
        if (album->isRoot())
        {
            viewItem = new FolderItem(m_folderView, album->title());
            viewItem->setOpen(true);
        }
        else
        {
            FolderItem* parentItem =
                (FolderItem*)(album->parent()->extraData(m_folderView));
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
            album->setExtraData(m_folderView, viewItem);
            m_albumMap.insert(viewItem, album);

            if (album == albumToSelect)
            {
                viewItem->setOpen(true);
                m_folderView->setSelected(viewItem, true);
                m_folderView->ensureItemVisible(viewItem);
            }
        }
    }

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotAlbumAdded(Album*)));
    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            SLOT(slotAlbumDeleted(Album*)));
    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            SLOT(slotAlbumsCleared()));
    connect(m_folderView, SIGNAL(selectionChanged()),
            SLOT(slotSelectionChanged()));
    connect(m_folderView, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
            SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));

    slotSelectionChanged();
}

void AlbumSelectDialog::slotAlbumAdded(Album* album)
{
    if (!album || album->type() != Album::PHYSICAL)
        return;

    FolderItem* parentItem =
        (FolderItem*)(album->parent()->extraData(m_folderView));
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
    album->setExtraData(m_folderView, viewItem);
    m_albumMap.insert(viewItem, (PAlbum*)album);
}

void AlbumSelectDialog::slotAlbumDeleted(Album* album)
{
    if (!album || album->type() != Album::PHYSICAL)
        return;

    FolderItem* viewItem =
        (FolderItem*)(album->extraData(m_folderView));

    if (viewItem)
    {
        delete viewItem;
        album->removeExtraData(m_folderView);
        m_albumMap.remove(viewItem);
    }
}

void AlbumSelectDialog::slotAlbumsCleared()
{
    m_folderView->clear();    
}

void AlbumSelectDialog::slotSelectionChanged()
{
    QListViewItem* selItem = 0;
    QListViewItemIterator it(m_folderView);
    while (it.current())
    {
        if (it.current()->isSelected())
        {
            selItem = it.current();
            break;
        }
        ++it;
    }

    if (!selItem || (selItem == m_folderView->firstChild()) &&
        !m_allowRootSelection)
    {
        enableButtonOK(false);
        return;
    }

    enableButtonOK(true);
}

void AlbumSelectDialog::slotContextMenu(QListViewItem *, const QPoint &, int)
{
    QPopupMenu popmenu(m_folderView);
    KAction *action = new KAction(i18n( "Create New Album" ),
                                  "albumfoldernew", 0, this,
                                  SLOT( slotUser1() ),
                                  &popmenu);
    action->plug(&popmenu);
    popmenu.exec(QCursor::pos());
}

void AlbumSelectDialog::slotUser1()
{
    QListViewItem* item = m_folderView->currentItem();
    if (!item)
        item = m_folderView->firstChild();

    if (!item)
        return;

    PAlbum* album = m_albumMap[(FolderItem*)item];
    if (!album)
        return;

    bool ok;
    QString newAlbumName = KInputDialog::getText(i18n("New Album Name"),
                                                 i18n("Creating new album in '%1'\n"
                                                      "Enter album name:")
                                                 .arg(album->prettyURL()),
                                                 m_newAlbumString, &ok, this);
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

    FolderItem* newItem = (FolderItem*)newAlbum->extraData(m_folderView);
    if (newItem)
    {
        m_folderView->ensureItemVisible(newItem);
        m_folderView->setSelected(newItem, true);
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

    FolderItem* item = (FolderItem*) dlg.m_folderView->currentItem();
    if (!item || (item == dlg.m_folderView->firstChild()) &&
        !allowRootSelection)
    {
        return 0;
    }
    
    return dlg.m_albumMap[item];
}

}  // namespace Digikam

#include "albumselectdialog.moc"
