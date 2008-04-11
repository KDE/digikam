/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : a widget to select image collections using 
 *               digiKam album folder views
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QHeaderView>
#include <QTreeWidgetItemIterator>
#include <QHBoxLayout>

// KDE includes.

#include <ktabwidget.h>
#include <kdialog.h>
#include <klocale.h>
#include <kvbox.h>

// Local includes.

#include "constants.h"
#include "ddebug.h"
#include "album.h"
#include "albummanager.h"
#include "albumthumbnailloader.h"
#include "searchtextbar.h"
#include "treefolderitem.h"
#include "kipiinterface.h"
#include "kipiimagecollection.h"
#include "kipiimagecollectionselector.h"
#include "kipiimagecollectionselector.moc"

namespace Digikam
{

class KipiImageCollectionSelectorPriv
{
public:

    KipiImageCollectionSelectorPriv()
    {
        tab            = 0;
        albumsView     = 0;
        tagsView       = 0;
        iface          = 0;
        albumSearchBar = 0;
        tagSearchBar   = 0;
    }

    QTreeWidget   *albumsView;
    QTreeWidget   *tagsView;

    KTabWidget    *tab;

    KipiInterface *iface; 

    SearchTextBar *albumSearchBar;
    SearchTextBar *tagSearchBar;
};

KipiImageCollectionSelector::KipiImageCollectionSelector(KipiInterface *iface, QWidget *parent)
                           : KIPI::ImageCollectionSelector(parent)
{
    d = new KipiImageCollectionSelectorPriv();
    d->iface = iface;
    d->tab   = new KTabWidget(this);

    KVBox *vbox1  = new KVBox(d->tab);
    d->albumsView = new QTreeWidget(vbox1);
    d->albumsView->setDragEnabled(false);
    d->albumsView->setDropIndicatorShown(false);
    d->albumsView->setAcceptDrops(false);
    d->albumsView->header()->hide();

    d->albumSearchBar = new SearchTextBar(vbox1, "KipiImageCollectionSelectorAlbumSearchBar");

    vbox1->setMargin(0);
    vbox1->setSpacing(KDialog::spacingHint());
    vbox1->setStretchFactor(d->albumsView, 10);

    // -------------------------------------------------------------------------------

    KVBox *vbox2 = new KVBox(d->tab);
    d->tagsView  = new QTreeWidget(vbox2);
    d->tagsView->setDragEnabled(false);
    d->tagsView->setDropIndicatorShown(false);
    d->tagsView->setAcceptDrops(false);
    d->tagsView->header()->hide();

    d->tagSearchBar = new SearchTextBar(vbox2, "KipiImageCollectionSelectorTagSearchBar");

    vbox2->setMargin(0);
    vbox2->setSpacing(KDialog::spacingHint());
    vbox2->setStretchFactor(d->tagsView, 10);

    // -------------------------------------------------------------------------------

    d->tab->addTab(vbox1, i18n("My Albums"));
    d->tab->addTab(vbox2, i18n("My Tags"));

    QHBoxLayout *hlay = new QHBoxLayout(this);
    hlay->addWidget(d->tab);
    hlay->setMargin(0);
    hlay->setSpacing(0);

    // ------------------------------------------------------------------------------------

    populateTreeView(AlbumManager::instance()->allPAlbums(), d->albumsView); 
    populateTreeView(AlbumManager::instance()->allTAlbums(), d->tagsView); 

    // ------------------------------------------------------------------------------------

    connect(d->albumsView, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SIGNAL(selectionChanged()));

    connect(d->tagsView, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SIGNAL(selectionChanged()));

    connect(d->albumSearchBar, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotAlbumSearchTextChanged(const QString&)));

    connect(d->tagSearchBar, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTagSearchTextChanged(const QString&)));
}

KipiImageCollectionSelector::~KipiImageCollectionSelector() 
{
    delete d;
}

void KipiImageCollectionSelector::populateTreeView(const AlbumList& aList, QTreeWidget *view)
{
    for (AlbumList::const_iterator it = aList.begin(); it != aList.end(); ++it)
    {
        Album *album                 = *it;
        TreeAlbumCheckListItem *item = 0;

        if (album->isRoot())
        {
            TreeAlbumItem *ritem = new TreeAlbumItem(view, album);
            ritem->setExpanded(true);
            PAlbum* palbum = dynamic_cast<PAlbum*>(album);
            if (palbum)
                ritem->setIcon(0, AlbumThumbnailLoader::instance()->getStandardAlbumIcon(palbum));
            else
            {
                TAlbum* talbum = dynamic_cast<TAlbum*>(album);
                if (talbum)
                    ritem->setIcon(0, AlbumThumbnailLoader::instance()->getStandardTagIcon(talbum));
            }
        }
        else
        {
            TreeAlbumCheckListItem* pitem = (TreeAlbumCheckListItem*)(album->parent()->extraData(view));
            if (!pitem)
            {
                DWarning() << "Failed to find parent for Album " << album->title() << endl;
                continue;
            }

            item = new TreeAlbumCheckListItem(pitem, album);
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

            if (album == AlbumManager::instance()->currentAlbum())
            {
                item->setCheckState(0, Qt::Checked);
                item->setExpanded(true);
                view->setCurrentItem(item);
                view->scrollToItem(item);
            }
        }
    }
}

QList<KIPI::ImageCollection> KipiImageCollectionSelector::selectedImageCollections() const
{
    QString ext = d->iface->fileExtensions();
    QList<KIPI::ImageCollection> list; 

    QTreeWidgetItemIterator it(d->albumsView, QTreeWidgetItemIterator::Checked);
    while (*it)
    {
        TreeAlbumCheckListItem* item = dynamic_cast<TreeAlbumCheckListItem*>(*it);
        if (item)
        {
            KipiImageCollection *col = new KipiImageCollection(KipiImageCollection::AllItems, item->album(), ext);
            list.append(col);
         }
         ++it;
    }

    QTreeWidgetItemIterator it2(d->tagsView, QTreeWidgetItemIterator::Checked);
    while (*it2)
    {
        TreeAlbumCheckListItem* item = dynamic_cast<TreeAlbumCheckListItem*>(*it2);
        if (item)
        {
            KipiImageCollection *col = new KipiImageCollection(KipiImageCollection::AllItems, item->album(), ext);
            list.append(col);
         }
         ++it2;
    }

    DDebug() << list.count() << " collection items selected" << endl;

    return list;
}

void KipiImageCollectionSelector::slotAlbumSearchTextChanged(const QString& filter)
{
    QString search = filter.toLower();

    bool atleastOneMatch = false;

    AlbumList pList = AlbumManager::instance()->allPAlbums();
    for (AlbumList::iterator it = pList.begin(); it != pList.end(); ++it)
    {
        PAlbum* palbum  = (PAlbum*)(*it);

        // don't touch the root Album
        if (palbum->isRoot())
            continue;

        bool match = palbum->title().toLower().contains(search);
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = palbum->parent();
            while (parent && !parent->isRoot())
            {
                if (parent->title().toLower().contains(search))
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
                if ((*it)->title().toLower().contains(search))
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

    d->albumSearchBar->slotSearchResult(atleastOneMatch);
}

void KipiImageCollectionSelector::slotTagSearchTextChanged(const QString& filter)
{
    QString search = filter.toLower();

    bool atleastOneMatch = false;

    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum* talbum  = (TAlbum*)(*it);

        // don't touch the root Album
        if (talbum->isRoot())
            continue;

        bool match = talbum->title().toLower().contains(search);
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = talbum->parent();
            while (parent && !parent->isRoot())
            {
                if (parent->title().toLower().contains(search))
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
            AlbumIterator it(talbum);
            while (it.current())
            {
                if ((*it)->title().toLower().contains(search))
                {
                    match = true;
                    break;
                }
                ++it;
            }
        }

        TreeAlbumCheckListItem* viewItem = (TreeAlbumCheckListItem*) talbum->extraData(d->tagsView);

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

    d->tagSearchBar->slotSearchResult(atleastOneMatch);
}

}  // namespace Digikam
