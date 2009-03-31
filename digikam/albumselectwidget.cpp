/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-26-02
 * Description : a widget to select a physical album
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumselectwidget.h"
#include "albumselectwidget.moc"

// Qt includes.

#include <QHeaderView>
#include <QTreeWidget>
#include <QGridLayout>

// KDE includes.

#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

// Local includes.

#include "constants.h"
#include "album.h"
#include "albummanager.h"
#include "albumthumbnailloader.h"
#include "treefolderitem.h"
#include "collectionmanager.h"
#include "kipiinterface.h"
#include "kipiimagecollection.h"

namespace Digikam
{

class AlbumSelectWidgetPriv
{
public:

    AlbumSelectWidgetPriv()
    {
        albumsView = 0;
        searchBar  = 0;
        newAlbumBtn= 0;
    }

    QTreeWidget   *albumsView;

    SearchTextBar *searchBar;

    KPushButton   *newAlbumBtn;
};

AlbumSelectWidget::AlbumSelectWidget(QWidget *parent, PAlbum* albumToSelect)
                 : QWidget(parent),
                   d(new AlbumSelectWidgetPriv)
{
    QGridLayout *grid = new QGridLayout(this);
    d->albumsView     = new QTreeWidget(this);
    d->albumsView->setDragEnabled(false);
    d->albumsView->setDropIndicatorShown(false);
    d->albumsView->setAcceptDrops(false);
    d->albumsView->header()->hide();
    d->albumsView->setContextMenuPolicy(Qt::CustomContextMenu);
    d->albumsView->setSelectionMode(QAbstractItemView::SingleSelection);

    d->searchBar      = new SearchTextBar(this, "AlbumSelectWidgetSearchBar");
    d->newAlbumBtn    = new KPushButton(KGuiItem(i18n("&New Album"), "albumfolder-new",
                                                 i18n("Create new album")), this);

    grid->addWidget(d->albumsView,  0, 0, 1, 2);
    grid->addWidget(d->searchBar,   1, 0, 1, 1);
    grid->addWidget(d->newAlbumBtn, 1, 1, 1, 1);
    grid->setRowStretch(0, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // ------------------------------------------------------------------------------------

    if (!albumToSelect) albumToSelect = dynamic_cast<PAlbum*>(AlbumManager::instance()->currentAlbum());
    populateTreeView(AlbumManager::instance()->allPAlbums(), d->albumsView, albumToSelect);

    // ------------------------------------------------------------------------------------

    connect(AlbumManager::instance(), SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(d->albumsView, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotContextMenu()));

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            this, SLOT(slotSearchTextChanged(const SearchTextSettings&)));

    connect(d->newAlbumBtn, SIGNAL(clicked()),
            this, SLOT(slotNewAlbum()));
}

AlbumSelectWidget::~AlbumSelectWidget()
{
    delete d;
}

QTreeWidget* AlbumSelectWidget::albumView() const
{
    return d->albumsView;
}

void AlbumSelectWidget::populateTreeView(const AlbumList& aList, QTreeWidget *view, PAlbum* albumToSelect)
{
    for (AlbumList::const_iterator it = aList.constBegin(); it != aList.constEnd(); ++it)
    {
        Album *album        = *it;
        TreeAlbumItem *item = 0;

        if (album->isRoot())
        {
            item = new TreeAlbumItem(view, album);
            item->setExpanded(true);
        }
        else
        {
            TreeAlbumItem* pitem = (TreeAlbumItem*)(album->parent()->extraData(view));
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
            else
            {
                TAlbum* talbum = dynamic_cast<TAlbum*>(album);
                if (talbum)
                    item->setIcon(0, AlbumThumbnailLoader::instance()->getStandardTagIcon(talbum));
            }

            if (album == albumToSelect)
            {
                item->setExpanded(true);
                view->setCurrentItem(item);
                view->scrollToItem(item);
            }
        }
    }
}

PAlbum* AlbumSelectWidget::currentAlbum() const
{
    TreeAlbumItem* item = dynamic_cast<TreeAlbumItem*>(d->albumsView->currentItem());
    if (item)
    {
        PAlbum* palbum = (PAlbum*)(item->album());
        return palbum;
    }
    return 0;
}

void AlbumSelectWidget::setCurrentAlbum(PAlbum *albumToSelect)
{
    QTreeWidgetItemIterator it(d->albumsView);
    while (*it)
    {
        TreeAlbumItem* item = dynamic_cast<TreeAlbumItem*>(*it);
        if (item)
        {
            PAlbum* palbum = (PAlbum*)(item->album());
            if (palbum == albumToSelect)
            {
                d->albumsView->setCurrentItem(item);
                d->albumsView->scrollToItem(item);
                item->setSelected(true);
                return;
            }
        }
        ++it;
    }
}

KUrl AlbumSelectWidget::currentAlbumUrl() const
{
    PAlbum* palbum = currentAlbum();
    if (palbum) return palbum->fileUrl();

    return KUrl();
}

void AlbumSelectWidget::setCurrentAlbumUrl(const KUrl& albumUrl)
{
    QTreeWidgetItemIterator it(d->albumsView);
    while (*it)
    {
        TreeAlbumItem* item = dynamic_cast<TreeAlbumItem*>(*it);
        if (item)
        {
            PAlbum* palbum = (PAlbum*)(item->album());
            if (palbum->fileUrl() == albumUrl)
            {
                d->albumsView->setCurrentItem(item);
                d->albumsView->scrollToItem(item);
                item->setSelected(true);
                return;
            }
        }
        ++it;
    }
}

void AlbumSelectWidget::slotSearchTextChanged(const SearchTextSettings& settings)
{
    QString search       = settings.text;
    bool atleastOneMatch = false;

    AlbumList pList = AlbumManager::instance()->allPAlbums();
    for (AlbumList::iterator it = pList.begin(); it != pList.end(); ++it)
    {
        PAlbum* palbum = (PAlbum*)(*it);

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

        TreeAlbumCheckListItem* viewItem = (TreeAlbumCheckListItem*) palbum->extraData(d->albumsView);

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

void AlbumSelectWidget::slotNewAlbum()
{
    QTreeWidgetItem* item = d->albumsView->currentItem();
    if (!item)
        item = d->albumsView->topLevelItem(0);

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
                                                 QString(), &ok, this);
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

    TreeAlbumItem* newItem = (TreeAlbumItem*)newAlbum->extraData(d->albumsView);
    if (newItem)
    {
        d->albumsView->scrollToItem(newItem);
        d->albumsView->setCurrentItem(newItem);
        newItem->setSelected(true);
    }
}

void AlbumSelectWidget::slotAlbumAdded(Album* album)
{
    if (!album || album->type() != Album::PHYSICAL)
        return;

    TreeAlbumItem* parentItem = 0;

    if (album->parent())
        parentItem = static_cast<TreeAlbumItem*>(album->parent()->extraData(d->albumsView));

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

void AlbumSelectWidget::slotAlbumDeleted(Album* album)
{
    if (!album || album->type() != Album::PHYSICAL)
        return;

    TreeAlbumItem *item = (TreeAlbumItem*)(album->extraData(d->albumsView));
    if (item)
        delete item;
}

void AlbumSelectWidget::slotAlbumsCleared()
{
    for(QTreeWidgetItemIterator it(d->albumsView); *it; ++it)
    {
        Album *album = static_cast<TreeAlbumItem*>(*it)->album();
        if (album)
            album->removeExtraData(d->albumsView);
    }
    d->albumsView->clear();
}

void AlbumSelectWidget::slotContextMenu()
{
    KMenu popmenu(d->albumsView);

    KAction *action = new KAction(KIcon("albumfolder-new"), i18n("Create New Album"), this);
    connect(action, SIGNAL(triggered(bool) ),
            this, SLOT(slotNewAlbum()));

    popmenu.addAction(action);
    popmenu.exec(QCursor::pos());
}

}  // namespace Digikam
