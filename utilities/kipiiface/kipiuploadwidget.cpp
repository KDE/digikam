/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : a widget to select an image collection
 *               to upload new items using digiKam album folder views
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

#include "kipiuploadwidget.h"
#include "kipiuploadwidget.moc"

// Qt includes.

#include <QHeaderView>
#include <QTreeWidget>
#include <QHBoxLayout>

// KDE includes.

#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>

// Local includes.

#include "constants.h"
#include "album.h"
#include "albummanager.h"
#include "albumthumbnailloader.h"
#include "treefolderitem.h"
#include "kipiinterface.h"
#include "kipiimagecollection.h"

namespace Digikam
{

class KipiUploadWidgetPriv
{
public:

    KipiUploadWidgetPriv()
    {
        albumsView = 0;
        iface      = 0;
        searchBar  = 0;
    }

    QTreeWidget   *albumsView;

    KipiInterface *iface;

    SearchTextBar *searchBar;
};

KipiUploadWidget::KipiUploadWidget(KipiInterface* iface, QWidget *parent)
                : KIPI::UploadWidget(parent),
                  d(new KipiUploadWidgetPriv)
{
    d->iface      = iface;
    d->albumsView = new QTreeWidget(this);
    d->albumsView->setDragEnabled(false);
    d->albumsView->setDropIndicatorShown(false);
    d->albumsView->setAcceptDrops(false);
    d->albumsView->header()->hide();

    d->searchBar      = new SearchTextBar(this, "KipiUploadWidgetSearchBar");

    QVBoxLayout *vlay = new QVBoxLayout(this);
    vlay->addWidget(d->albumsView, 10);
    vlay->addWidget(d->searchBar);
    vlay->setMargin(0);
    vlay->setSpacing(KDialog::spacingHint());

    // ------------------------------------------------------------------------------------

    populateTreeView(AlbumManager::instance()->allPAlbums(), d->albumsView);

    // ------------------------------------------------------------------------------------

    connect(d->albumsView, SIGNAL(itemSelectionChanged()),
            this, SIGNAL(selectionChanged()));

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            this, SLOT(slotSearchTextChanged(const SearchTextSettings&)));
}

KipiUploadWidget::~KipiUploadWidget()
{
    delete d;
}

void KipiUploadWidget::populateTreeView(const AlbumList& aList, QTreeWidget *view)
{
    for (AlbumList::const_iterator it = aList.begin(); it != aList.end(); ++it)
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

            if (album == AlbumManager::instance()->currentAlbum())
            {
                item->setExpanded(true);
                view->setCurrentItem(item);
                view->scrollToItem(item);
            }
        }
    }
}

KIPI::ImageCollection KipiUploadWidget::selectedImageCollection() const
{
    QString ext = d->iface->fileExtensions();
    KIPI::ImageCollection collection;

    TreeAlbumItem* item = dynamic_cast<TreeAlbumItem*>(d->albumsView->currentItem());
    if (item)
        collection = new KipiImageCollection(KipiImageCollection::AllItems, item->album(), ext);

    return collection;
}

void KipiUploadWidget::slotSearchTextChanged(const SearchTextSettings& settings)
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

}  // namespace Digikam
