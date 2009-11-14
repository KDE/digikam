/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : a widget to select image collections using
 *               digiKam album folder views
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// #include "kipiimagecollectionselector.h"
#include "kipiimagecollectionselector.moc"

// Qt includes

#include <QHeaderView>
#include <QTreeWidgetItemIterator>
#include <QHBoxLayout>

// KDE includes

#include <kdialog.h>
#include <klocale.h>
#include <ktabwidget.h>
#include <kvbox.h>
#include <kdebug.h>

// LibKIPI includes

#include <libkipi/version.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "albumthumbnailloader.h"
#include "treefolderitem.h"
#include "searchfolderview.h"
#include "timelinefolderview.h"
#include "fuzzysearchfolderview.h"
#include "gpssearchfolderview.h"
#include "kipiinterface.h"
#include "kipiimagecollection.h"

namespace Digikam
{

class KipiImageCollectionSelectorPriv
{
public:

    KipiImageCollectionSelectorPriv()
    {
        tab               = 0;
        albumsView        = 0;
        tagsView          = 0;
        searchesView      = 0;
        iface             = 0;
        albumsSearchBar   = 0;
        tagsSearchBar     = 0;
        searchesSearchBar = 0;
    }

    QTreeWidget*   albumsView;
    QTreeWidget*   tagsView;
    QTreeWidget*   searchesView;

    KTabWidget*    tab;

    KipiInterface* iface;

    SearchTextBar* albumsSearchBar;
    SearchTextBar* tagsSearchBar;
    SearchTextBar* searchesSearchBar;
};

KipiImageCollectionSelector::KipiImageCollectionSelector(KipiInterface *iface, QWidget *parent)
                           : KIPI::ImageCollectionSelector(parent),
                             d(new KipiImageCollectionSelectorPriv)
{
    d->iface = iface;
    d->tab   = new KTabWidget(this);

    KVBox *vbox1  = new KVBox(d->tab);
    d->albumsView = new QTreeWidget(vbox1);
    d->albumsView->setDragEnabled(false);
    d->albumsView->setDropIndicatorShown(false);
    d->albumsView->setAcceptDrops(false);
    d->albumsView->header()->hide();

    d->albumsSearchBar = new SearchTextBar(vbox1, "KipiImageCollectionSelectorAlbumSearchBar");

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

    d->tagsSearchBar = new SearchTextBar(vbox2, "KipiImageCollectionSelectorTagSearchBar");

    vbox2->setMargin(0);
    vbox2->setSpacing(KDialog::spacingHint());
    vbox2->setStretchFactor(d->tagsView, 10);

    // -------------------------------------------------------------------------------

    KVBox *vbox3    = new KVBox(d->tab);
    d->searchesView = new QTreeWidget(vbox3);
    d->searchesView->setDragEnabled(false);
    d->searchesView->setDropIndicatorShown(false);
    d->searchesView->setAcceptDrops(false);
    d->searchesView->header()->hide();

    d->searchesSearchBar = new SearchTextBar(vbox3, "KipiImageCollectionSelectorTagSearchBar");

    vbox3->setMargin(0);
    vbox3->setSpacing(KDialog::spacingHint());
    vbox3->setStretchFactor(d->searchesView, 10);

    // -------------------------------------------------------------------------------

    d->tab->addTab(vbox1, i18n("My Albums"));
    d->tab->addTab(vbox2, i18n("My Tags"));
    d->tab->addTab(vbox3, i18n("My Searches"));

    QHBoxLayout *hlay = new QHBoxLayout(this);
    hlay->addWidget(d->tab);
    hlay->setMargin(0);
    hlay->setSpacing(0);

    // ------------------------------------------------------------------------------------

    populateTreeView(AlbumManager::instance()->allPAlbums(), d->albumsView);
    populateTreeView(AlbumManager::instance()->allTAlbums(), d->tagsView);
    populateTreeView(AlbumManager::instance()->allSAlbums(), d->searchesView);

    // ------------------------------------------------------------------------------------

    connect(d->albumsView, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SIGNAL(selectionChanged()));

    connect(d->tagsView, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SIGNAL(selectionChanged()));

    connect(d->searchesView, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SIGNAL(selectionChanged()));

    connect(d->albumsSearchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            this, SLOT(slotAlbumsSearchTextChanged(const SearchTextSettings&)));

    connect(d->tagsSearchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            this, SLOT(slotTagsSearchTextChanged(const SearchTextSettings&)));

    connect(d->searchesSearchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            this, SLOT(slotSearchesSearchTextChanged(const SearchTextSettings&)));
}

KipiImageCollectionSelector::~KipiImageCollectionSelector()
{
    delete d;
}

void KipiImageCollectionSelector::populateTreeView(const AlbumList& aList, QTreeWidget *view)
{
    for (AlbumList::const_iterator it = aList.constBegin(); it != aList.constEnd(); ++it)
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
                else
                {
                    SAlbum* salbum = dynamic_cast<SAlbum*>(album);
                    if (salbum)
                        ritem->setIcon(0, KIcon("edit-find"));
                }
            }
        }
        else
        {
            TreeAlbumCheckListItem* pitem = 0;
            if (album->parent())
                pitem = static_cast<TreeAlbumCheckListItem*>(album->parent()->extraData(view));

            if (!pitem)
            {
                kWarning() << "Failed to find parent for Album " << album->title();
                continue;
            }

            SAlbum* salbum = dynamic_cast<SAlbum*>(album);
            if (salbum &&
                (salbum->title() == SearchFolderView::currentSearchViewSearchName()       ||
                 salbum->title() == TimeLineFolderView::currentTimeLineSearchName()       ||
                 salbum->title() == FuzzySearchFolderView::currentFuzzySketchSearchName() ||
                 salbum->title() == FuzzySearchFolderView::currentFuzzyImageSearchName()  ||
                 salbum->title() == GPSSearchFolderView::currentGPSSearchName()           ||
                 salbum->isDuplicatesSearch()))
                continue;

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
                else
                {
                    SAlbum* salbum = dynamic_cast<SAlbum*>(album);
                    if (salbum && !salbum->isDuplicatesSearch())
                    {
                        if (salbum->isTimelineSearch())
                            item->setIcon(0, KIcon("clock"));
                        else if (salbum->isHaarSearch())
                            item->setIcon(0, KIcon("tools-wizard"));
                        else if (salbum->isMapSearch())
                            item->setIcon(0, KIcon("applications-internet"));
                        else
                            item->setIcon(0, KIcon("edit-find"));
                    }
                }
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
#if KIPI_VERSION >= 0x000300
    QString ext = d->iface->hostSetting("FileExtensions").toString();
#else
    QString ext = d->iface->fileExtensions();
#endif
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

    QTreeWidgetItemIterator it3(d->searchesView, QTreeWidgetItemIterator::Checked);
    while (*it3)
    {
        TreeAlbumCheckListItem* item = dynamic_cast<TreeAlbumCheckListItem*>(*it3);
        if (item)
        {
            KipiImageCollection *col = new KipiImageCollection(KipiImageCollection::AllItems, item->album(), ext);
            list.append(col);
         }
         ++it3;
    }

    kDebug() << list.count() << " collection items selected";

    return list;
}

void KipiImageCollectionSelector::slotAlbumsSearchTextChanged(const SearchTextSettings& settings)
{
    QString search       = settings.text;
    bool atleastOneMatch = false;

    AlbumList pList = AlbumManager::instance()->allPAlbums();
    for (AlbumList::const_iterator it = pList.constBegin(); it != pList.constEnd(); ++it)
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

    d->albumsSearchBar->slotSearchResult(atleastOneMatch);
}

void KipiImageCollectionSelector::slotTagsSearchTextChanged(const SearchTextSettings& settings)
{
    QString search       = settings.text;
    bool atleastOneMatch = false;

    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
    {
        TAlbum* talbum  = (TAlbum*)(*it);

        // don't touch the root Album
        if (talbum->isRoot())
            continue;

        bool match = talbum->title().contains(search, settings.caseSensitive);
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = talbum->parent();
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
            AlbumIterator it(talbum);
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

    d->tagsSearchBar->slotSearchResult(atleastOneMatch);
}

void KipiImageCollectionSelector::slotSearchesSearchTextChanged(const SearchTextSettings& settings)
{
    QString search       = settings.text;
    bool atleastOneMatch = false;

    AlbumList tList = AlbumManager::instance()->allSAlbums();
    for (AlbumList::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
    {
        SAlbum* salbum  = (SAlbum*)(*it);

        // don't touch the root Album
        if (salbum->isRoot())
            continue;

        bool match = salbum->title().contains(search, settings.caseSensitive);
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = salbum->parent();
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
            AlbumIterator it(salbum);
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

        TreeAlbumCheckListItem* viewItem = (TreeAlbumCheckListItem*) salbum->extraData(d->searchesView);

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

    d->searchesSearchBar->slotSearchResult(atleastOneMatch);
}

}  // namespace Digikam
