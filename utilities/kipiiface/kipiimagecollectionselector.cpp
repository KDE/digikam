/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : a widget to select image collections using
 *               digiKam album folder views
 *
 * Copyright (C) 2008-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "kipiimagecollectionselector.h"

// Qt includes

#include <QHeaderView>
#include <QTreeWidgetItemIterator>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QTabWidget>

// KDE includes


#include <klocalizedstring.h>

// Libkdcraw includes

#include <KDCRAW/RWidgetUtils>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "applicationsettings.h"
#include "albumtreeview.h"
#include "kipiimagecollection.h"
#include "kipiinterface.h"
#include "albumlabelstreeview.h"

using namespace KDcrawIface;

namespace Digikam
{

class KipiImageCollectionSelector::Private
{
public:

    Private() :
        tab(0),
        albumModel(0),
        albumTreeView(0),
        tagModel(0),
        tagTreeView(0),
        searchModel(0),
        searchTreeView(0),
        iface(0),
        albumSearchBar(0),
        tagSearchBar(0),
        searchSearchBar(0),
        labelsTree(0),
        labelsSearchHandler(0)
    {
    }

    void prepareTreeView(AbstractCheckableAlbumTreeView* const treeView)
    {

        treeView->checkableModel()->setShowCount(false);
        treeView->checkableModel()->setCheckable(true);
        treeView->setRootIsDecorated(true);
        treeView->setSortingEnabled(true);
        treeView->setSelectAlbumOnClick(false);
        treeView->setExpandOnSingleClick(false);
        treeView->setEnableContextMenu(false);
        treeView->setDragEnabled(false);

    }

    void fillCollectionsFromCheckedModel(QList<KIPI::ImageCollection>& collectionList,
                                         AbstractCheckableAlbumModel* const model,
                                         const QString& ext)
    {
        foreach(Album* const album, model->checkedAlbums())
        {
            if (!album)
            {
                continue;
            }

            KipiImageCollection* const col = new KipiImageCollection(KipiImageCollection::AllItems, album, ext);
            collectionList.append(col);
        }
    }

    void fillCollectionsFromCheckedLabels(QList<KIPI::ImageCollection>& collectionList,
                                                   const QString& ext)
    {
        Album* const album = labelsSearchHandler->albumForSelectedItems();

        if (!album)
        {
            return;
        }

        KipiImageCollection* const col = new KipiImageCollection(KipiImageCollection::AllItems, album, ext,labelsSearchHandler->imagesUrls());
        collectionList.append(col);
    }

public:

    QTabWidget*               tab;

    AlbumModel*               albumModel;
    AlbumTreeView*            albumTreeView;

    TagModel*                 tagModel;
    TagTreeView*              tagTreeView;

    SearchModel*              searchModel;
    SearchTreeView*           searchTreeView;

    KipiInterface*            iface;

    SearchTextBar*            albumSearchBar;
    SearchTextBar*            tagSearchBar;
    SearchTextBar*            searchSearchBar;

    AlbumLabelsTreeView*      labelsTree;
    AlbumLabelsSearchHandler* labelsSearchHandler;
};

KipiImageCollectionSelector::KipiImageCollectionSelector(KipiInterface* const iface, QWidget* const parent)
    : KIPI::ImageCollectionSelector(parent),
      d(new Private)
{
    KSharedConfigPtr config  = KSharedConfig::openConfig();
    KConfigGroup configGroup = config->group("KipiImageCollectionSelector");
    d->iface                 = iface;
    d->tab                   = new QTabWidget(this);

    RVBox* const albumBox = new RVBox(d->tab);
    d->albumModel         = new AlbumModel(AbstractAlbumModel::IgnoreRootAlbum, albumBox);
    d->albumTreeView      = new AlbumTreeView(albumBox);
    d->albumTreeView->setAlbumModel(d->albumModel);
    d->albumTreeView->setEntryPrefix("AlbumTreeView");
    d->albumTreeView->setConfigGroup(configGroup);
    d->prepareTreeView(d->albumTreeView);

    d->albumSearchBar = new SearchTextBar(albumBox, "KipiImageCollectionSelectorAlbumSearchBar");
    d->albumSearchBar->setEntryPrefix("AlbumSearchBar");
    d->albumSearchBar->setConfigGroup(configGroup);
    d->albumSearchBar->setModel(d->albumModel, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->albumSearchBar->setFilterModel(d->albumTreeView->albumFilterModel());

    albumBox->setMargin(0);
    albumBox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    albumBox->setStretchFactor(d->albumTreeView, 100);
    albumBox->setStretchFactor(d->albumSearchBar, 1);

    // -------------------------------------------------------------------------------

    RVBox* const tagBox = new RVBox(d->tab);
    d->tagModel         = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, tagBox);
    d->tagTreeView      = new TagTreeView(tagBox);
    d->tagTreeView->setAlbumModel(d->tagModel);
    d->tagTreeView->setEntryPrefix("TagTreeView");
    d->tagTreeView->setConfigGroup(configGroup);
    d->prepareTreeView(d->tagTreeView);

    d->tagSearchBar = new SearchTextBar(tagBox, "KipiImageCollectionSelectorTagSearchBar");
    d->tagSearchBar->setEntryPrefix("TagSearchBar");
    d->tagSearchBar->setConfigGroup(configGroup);
    d->tagSearchBar->setModel(d->tagTreeView->filteredModel(),
                              AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagSearchBar->setFilterModel(d->tagTreeView->albumFilterModel());

    tagBox->setMargin(0);
    tagBox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    tagBox->setStretchFactor(d->tagTreeView, 100);
    tagBox->setStretchFactor(d->tagSearchBar, 1);

    // -------------------------------------------------------------------------------

    RVBox* const searchBox  = new RVBox(d->tab);
    d->searchModel          = new SearchModel(searchBox);
    d->searchTreeView       = new SearchTreeView(searchBox);
    d->searchTreeView->setAlbumModel(d->searchModel);
    d->searchTreeView->setEntryPrefix("SearchTreeView");
    d->searchTreeView->setConfigGroup(configGroup);
    d->searchTreeView->filteredModel()->listAllSearches();
    d->prepareTreeView(d->searchTreeView);

    d->searchSearchBar = new SearchTextBar(searchBox, "KipiImageCollectionSelectorSearchSearchBar");
    d->searchSearchBar->setEntryPrefix("SearchSearchBar");
    d->searchSearchBar->setConfigGroup(configGroup);
    d->searchSearchBar->setModel(d->searchModel, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchSearchBar->setFilterModel(d->searchTreeView->albumFilterModel());

    searchBox->setMargin(0);
    searchBox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    searchBox->setStretchFactor(d->searchTreeView, 100);
    searchBox->setStretchFactor(d->searchSearchBar, 1);

    // -------------------------------------------------------------------------------

    RVBox* const labelsBox = new RVBox(d->tab);
    d->labelsTree          = new AlbumLabelsTreeView(labelsBox,true);
    d->labelsSearchHandler = new AlbumLabelsSearchHandler(d->labelsTree);

    labelsBox->setMargin(0);
    labelsBox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    labelsBox->setStretchFactor(d->labelsTree, 100);

    // -------------------------------------------------------------------------------

    d->tab->addTab(albumBox,  i18n("Albums"));
    d->tab->addTab(tagBox,    i18n("Tags"));
    d->tab->addTab(searchBox, i18n("Searches"));
    d->tab->addTab(labelsBox, i18n("Labels"));

    QHBoxLayout* const hlay = new QHBoxLayout(this);
    hlay->addWidget(d->tab);
    hlay->setMargin(0);
    hlay->setSpacing(0);

    // ------------------------------------------------------------------------------------

    connect(d->albumModel, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SIGNAL(selectionChanged()));

    connect(d->tagModel, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SIGNAL(selectionChanged()));

    connect(d->searchModel, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SIGNAL(selectionChanged()));

    connect(d->labelsSearchHandler, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SIGNAL(selectionChanged()));

    // ------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_GENERAL_LOG) << "Restoring view state";

    d->albumTreeView->loadState();
    d->albumSearchBar->loadState();
    d->tagTreeView->loadState();
    d->tagSearchBar->loadState();
    d->searchTreeView->loadState();
    d->searchSearchBar->loadState();
}

KipiImageCollectionSelector::~KipiImageCollectionSelector()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Saving view state";

    d->albumTreeView->saveState();
    d->albumSearchBar->saveState();
    d->tagTreeView->saveState();
    d->tagSearchBar->saveState();
    d->searchTreeView->saveState();
    d->searchSearchBar->saveState();

    delete d;
}

QList<KIPI::ImageCollection> KipiImageCollectionSelector::selectedImageCollections() const
{
    QString ext = d->iface->hostSetting("FileExtensions").toString();
    QList<KIPI::ImageCollection> list;

    d->fillCollectionsFromCheckedModel(list,  d->albumModel,  ext);
    d->fillCollectionsFromCheckedModel(list,  d->tagModel,    ext);
    d->fillCollectionsFromCheckedModel(list,  d->searchModel, ext);
    d->fillCollectionsFromCheckedLabels(list, ext);

    qCDebug(DIGIKAM_GENERAL_LOG) << list.count() << " collection items selected";

    return list;
}

void KipiImageCollectionSelector::enableVirtualCollections(bool flag)
{
    if (flag == false)
    {
        d->tab->setTabEnabled(1, false);
        d->tab->setTabEnabled(2, false);
    }
    else
    {
        d->tab->setTabEnabled(1, true);
        d->tab->setTabEnabled(2, true);
    }
}

}  // namespace Digikam
