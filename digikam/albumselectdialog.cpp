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

#include "albumselectdialog.h"
#include "albumselectdialog.moc"

// Qt includes.

#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QCursor>
#include <QGridLayout>
#include <QPixmap>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>

// KDE includes.

#include <kdebug.h>
#include <kmenu.h>
#include <klocale.h>
#include <kapplication.h>
#include <kaction.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// Local includes.

#include "treefolderitem.h"
#include "album.h"
#include "albummanager.h"
#include "albumthumbnailloader.h"
#include "collectionmanager.h"

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

    bool           allowRootSelection;

    QString        newAlbumString;

    QTreeWidget   *folderView;

    SearchTextBar *searchBar;
};

AlbumSelectDialog::AlbumSelectDialog(QWidget* parent, PAlbum* albumToSelect,
                                     const QString& header,
                                     const QString& newAlbumString,
                                     bool allowRootSelection)
                 : KDialog(parent), d(new AlbumSelectDialogPrivate)
{
    d->allowRootSelection = allowRootSelection;
    d->newAlbumString     = newAlbumString;

    setCaption(i18n("Select Album"));
    setButtons(Help|User1|Ok|Cancel);
    setButtonText(User1, i18n("&New Album"));
    setButtonIcon(User1, KIcon("albumfolder-new"));
    setDefaultButton(Ok);
    setHelp("targetalbumdialog.anchor", "digikam");
    enableButtonOk(false);

    // -------------------------------------------------------------

    QWidget *page = new QWidget(this);
    setMainWidget(page);

    QGridLayout* grid       = new QGridLayout(page);
    QLabel *logo            = new QLabel(page);
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                            .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel *message = new QLabel(page);
    message->setWordWrap(true);
    if (!header.isEmpty())
        message->setText(header);

    d->folderView = new QTreeWidget(page);
    d->folderView->setHeaderLabels(QStringList() << i18n("My Albums"));
    d->folderView->setContextMenuPolicy(Qt::CustomContextMenu);

    d->searchBar = new SearchTextBar(page, "AlbumSelectDialogSearchBar");

    grid->addWidget(logo,          0, 0, 1, 1);
    grid->addWidget(message,       1, 0, 1, 1);
    grid->addWidget(d->folderView, 0, 1, 3, 1);
    grid->addWidget(d->searchBar,  3, 1, 1, 1);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(2, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    populateTreeView(albumToSelect);

    // -------------------------------------------------------------

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(d->folderView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->folderView, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotContextMenu()));

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotUser1()));

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            this, SLOT(slotSearchTextChanged(const SearchTextSettings&)));

    // -------------------------------------------------------------

    resize(500, 500);
    slotSelectionChanged();
}

AlbumSelectDialog::~AlbumSelectDialog()
{
    delete d;
}

void AlbumSelectDialog::populateTreeView(PAlbum *albumToSelect)
{
    AlbumList aList = AlbumManager::instance()->allPAlbums();

    for (AlbumList::const_iterator it = aList.constBegin(); it != aList.constEnd(); ++it)
    {
        Album *album        = *it;
        TreeAlbumItem *item = 0;

        if (album->isRoot())
        {
            item = new TreeAlbumItem(d->folderView, album);
            item->setExpanded(true);
        }
        else
        {
            TreeAlbumItem* pitem = (TreeAlbumItem*)(album->parent()->extraData(d->folderView));
            if (!pitem)
            {
                kWarning(50003) << "Failed to find parent for Album " << album->title() << endl;
                continue;
            }

            item = new TreeAlbumItem(pitem, album);
        }

        if (item)
        {
            PAlbum* palbum = dynamic_cast<PAlbum*>(album);
            if (palbum)
                item->setIcon(0, AlbumThumbnailLoader::instance()->getStandardAlbumIcon(palbum));

            if (album == albumToSelect)
            {
                item->setExpanded(true);
                d->folderView->setCurrentItem(item);
                d->folderView->scrollToItem(item);
            }
        }
    }
}

void AlbumSelectDialog::slotAlbumAdded(Album* album)
{
    if (!album || album->type() != Album::PHYSICAL)
        return;

    TreeAlbumItem* parentItem = (TreeAlbumItem*)(album->parent()->extraData(d->folderView));

    if (!parentItem)
    {
        kWarning(50003) << "Failed to find parent for Album "
                        << album->title() << endl;
        return;
    }

    TreeAlbumItem* item = new TreeAlbumItem(parentItem, album);
    PAlbum* palbum      = dynamic_cast<PAlbum*>(album);
    if (palbum)
        item->setIcon(0, AlbumThumbnailLoader::instance()->getStandardAlbumIcon(palbum));
}

void AlbumSelectDialog::slotAlbumDeleted(Album* album)
{
    if (!album || album->type() != Album::PHYSICAL)
        return;

    TreeAlbumItem *item = (TreeAlbumItem*)(album->extraData(d->folderView));
    if (item)
        delete item;
}

void AlbumSelectDialog::slotAlbumsCleared()
{
    for(QTreeWidgetItemIterator it(d->folderView); *it; ++it)
    {
        Album *album = static_cast<TreeAlbumItem*>(*it)->album();
        if (album)
            album->removeExtraData(d->folderView);
    }
    d->folderView->clear();
}

void AlbumSelectDialog::slotSelectionChanged()
{
    QTreeWidgetItem* selItem = d->folderView->currentItem();

    if ((!selItem || (selItem == d->folderView->topLevelItem(0))) &&
        !d->allowRootSelection)
    {
        enableButtonOk(false);
        return;
    }

    enableButtonOk(true);
}

void AlbumSelectDialog::slotContextMenu()
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
    QTreeWidgetItem* item = d->folderView->currentItem();
    if (!item)
        item = d->folderView->topLevelItem(0);

    if (!item)
        return;

    TreeAlbumItem* viewItem = dynamic_cast<TreeAlbumItem*>(item);
    if (!viewItem)
        return;

    PAlbum* album = dynamic_cast<PAlbum*>(viewItem->album());
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

    PAlbum *newAlbum;
    QString errMsg;
    if (album->isRoot())
    {
        // if we create an album under root, need to supply the album root path.
        // TODO: Let user choose an album root
        newAlbum = AlbumManager::instance()->createPAlbum(CollectionManager::instance()->oneAlbumRootPath(),
                                                          newAlbumName, QString(), QDate::currentDate(), QString(), errMsg);
    }
    else
    {
        newAlbum = AlbumManager::instance()->createPAlbum(album, newAlbumName, QString(),
                                                          QDate::currentDate(), QString(), errMsg);
    }

    if (!newAlbum)
    {
        KMessageBox::error(this, errMsg);
        return;
    }

    TreeAlbumItem* newItem = (TreeAlbumItem*)newAlbum->extraData(d->folderView);
    if (newItem)
    {
        d->folderView->scrollToItem(newItem);
        newItem->setSelected(true);
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

    TreeAlbumItem* item = (TreeAlbumItem*) dlg.d->folderView->currentItem();
    if ((!item || (item == dlg.d->folderView->topLevelItem(0))) &&
        !allowRootSelection)
    {
        return 0;
    }

    return (dynamic_cast<PAlbum*>(item->album()));
}

void AlbumSelectDialog::slotSearchTextChanged(const SearchTextSettings& settings)
{
    QString search       = settings.text;
    bool atleastOneMatch = false;

    AlbumList pList = AlbumManager::instance()->allPAlbums();
    for (AlbumList::iterator it = pList.begin(); it != pList.end(); ++it)
    {
        PAlbum* palbum  = (PAlbum*)(*it);

        // don't touch the root Album
        if (palbum->isRoot())
            continue;

        bool match = palbum->title().contains(search, settings.caseSensitive);
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = palbum->parent();
            while (parent && !parent->isRoot())
            {
                if (parent->title().contains(search, settings.caseSensitive))
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
                if ((*it)->title().contains(search, settings.caseSensitive))
                {
                    match = true;
                    break;
                }
                ++it;
            }
        }

        TreeAlbumCheckListItem* viewItem = (TreeAlbumCheckListItem*) palbum->extraData(d->folderView);

        if (match)
        {
            atleastOneMatch = true;

            if (viewItem)
                viewItem->setHidden(false);
        }
        else
        {
            if (viewItem)
            {
                viewItem->setHidden(true);
            }
        }
    }

    d->searchBar->slotSearchResult(atleastOneMatch);
}

}  // namespace Digikam
