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
#include <QTreeWidget>
#include <QTreeWidgetItem>
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
#include "kipiinterface.h"
#include "kipiimagecollection.h"
#include "kipiimagecollectionselector.h"
#include "kipiimagecollectionselector.moc"

namespace Digikam
{

class KipiImageCollectionSelectorItem : public QTreeWidgetItem
{
public:

    KipiImageCollectionSelectorItem(QTreeWidget* parent, Album* tag);
    KipiImageCollectionSelectorItem(QTreeWidgetItem* parent, Album* tag);

    Album* album() const;

private:

    Album *m_album;
};

KipiImageCollectionSelectorItem::KipiImageCollectionSelectorItem(QTreeWidget* parent, Album* album)
                               : QTreeWidgetItem(parent, QStringList() << album->title())
{
    m_album = album;
    m_album->setExtraData(treeWidget(), this);
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled); 
}

KipiImageCollectionSelectorItem::KipiImageCollectionSelectorItem(QTreeWidgetItem* parent, Album* album)
                               : QTreeWidgetItem(parent, QStringList() << album->title())
{
    m_album = album;
    m_album->setExtraData(treeWidget(), this);
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled); 
}

Album* KipiImageCollectionSelectorItem::album() const
{
    return m_album;
}

KipiImageCollectionSelector::KipiImageCollectionSelector(KipiInterface *iface, QWidget *parent)
                           : KIPI::ImageCollectionSelector(parent)
{
    m_iface      = iface;
    m_tab        = new KTabWidget(this);

    m_albumsView = new QTreeWidget(m_tab);
    m_albumsView->setColumnCount(1);
    m_albumsView->setRootIsDecorated(true);
    m_albumsView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_albumsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_albumsView->setAllColumnsShowFocus(true);
    m_albumsView->setDragEnabled(false);
    m_albumsView->setDropIndicatorShown(false);
    m_albumsView->setAcceptDrops(false);
    m_albumsView->header()->hide();

    m_tagsView = new QTreeWidget(m_tab);
    m_tagsView->setColumnCount(1);
    m_tagsView->setRootIsDecorated(true);
    m_tagsView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tagsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tagsView->setAllColumnsShowFocus(true);
    m_tagsView->setDragEnabled(false);
    m_tagsView->setDropIndicatorShown(false);
    m_tagsView->setAcceptDrops(false);
    m_tagsView->header()->hide();

    m_tab->addTab(m_albumsView, i18n("My Albums"));
    m_tab->addTab(m_tagsView, i18n("My Tags"));

    QHBoxLayout *hlay = new QHBoxLayout(this);
    hlay->addWidget(m_tab);
    hlay->setMargin(0);
    hlay->setSpacing(0);

    // ------------------------------------------------------------------------------------

    populateTreeView(AlbumManager::instance()->allPAlbums(), m_albumsView); 
    populateTreeView(AlbumManager::instance()->allTAlbums(), m_tagsView); 

    // ------------------------------------------------------------------------------------

    connect(m_albumsView, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SIGNAL(selectionChanged()));

    connect(m_tagsView, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SIGNAL(selectionChanged()));
}

KipiImageCollectionSelector::~KipiImageCollectionSelector() 
{
}

void KipiImageCollectionSelector::populateTreeView(const AlbumList& aList, QTreeWidget *view)
{
    for (AlbumList::const_iterator it = aList.begin(); it != aList.end(); ++it)
    {
        Album* album = *it;

        KipiImageCollectionSelectorItem* item = 0;

        if (album->isRoot())
        {
            item = new KipiImageCollectionSelectorItem(view, album);
            item->setExpanded(true);
        }
        else
        {
            KipiImageCollectionSelectorItem* pitem = (KipiImageCollectionSelectorItem*)(album->parent()->extraData(view));
            if (!pitem)
            {
                DWarning() << "Failed to find parent for Album " << album->title() << endl;
                continue;
            }

            item = new KipiImageCollectionSelectorItem(pitem, album);
        }

        if (item)
        {
            item->setCheckState(0, Qt::Unchecked);
            PAlbum* palbum = dynamic_cast<PAlbum*>(album);
            if (palbum)
                item->setIcon(0, AlbumThumbnailLoader::instance()->getStandardAlbumIcon(palbum));
            else
            {
                TAlbum* talbum = dynamic_cast<TAlbum*>(album);
                if (talbum)
                    item->setIcon(0, AlbumThumbnailLoader::instance()->getStandardTagIcon(talbum));
            }

            album->setExtraData(view, item);

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
    QString ext = m_iface->fileExtensions();
    QList<KIPI::ImageCollection> list; 

    QTreeWidgetItemIterator it(m_albumsView, QTreeWidgetItemIterator::Checked);
    while (*it)
    {
        KipiImageCollectionSelectorItem* item = dynamic_cast<KipiImageCollectionSelectorItem*>(*it);
        if (item)
        {
            KipiImageCollection *col = new KipiImageCollection(KipiImageCollection::AllItems, item->album(), ext);
            list.append(col);
         }
         ++it;
    }

    QTreeWidgetItemIterator it2(m_tagsView, QTreeWidgetItemIterator::Checked);
    while (*it2)
    {
        KipiImageCollectionSelectorItem* item = dynamic_cast<KipiImageCollectionSelectorItem*>(*it2);
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
