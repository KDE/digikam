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
#include <klocale.h>

// Local includes.

#include "constants.h"
#include "ddebug.h"
#include "album.h"
#include "albumthumbnailloader.h"
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
        tab        = 0;
        albumsView = 0;
        tagsView   = 0;
        iface      = 0;
    }

    QTreeWidget   *albumsView;
    QTreeWidget   *tagsView;

    KTabWidget    *tab;

    KipiInterface *iface; 
};

KipiImageCollectionSelector::KipiImageCollectionSelector(KipiInterface *iface, QWidget *parent)
                           : KIPI::ImageCollectionSelector(parent)
{
    d = new KipiImageCollectionSelectorPriv();
    d->iface = iface;
    d->tab   = new KTabWidget(this);

    d->albumsView = new QTreeWidget(d->tab);
    d->albumsView->setDragEnabled(false);
    d->albumsView->setDropIndicatorShown(false);
    d->albumsView->setAcceptDrops(false);
    d->albumsView->header()->hide();

    d->tagsView = new QTreeWidget(d->tab);
    d->tagsView->setDragEnabled(false);
    d->tagsView->setDropIndicatorShown(false);
    d->tagsView->setAcceptDrops(false);
    d->tagsView->header()->hide();

    d->tab->addTab(d->albumsView, i18n("My Albums"));
    d->tab->addTab(d->tagsView, i18n("My Tags"));

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

}  // namespace Digikam
