/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : a widget to select albums using a tab of folder views.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "albumselecttabs.h"

// Qt includes

#include <QHeaderView>
#include <QTreeWidgetItemIterator>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QString>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "applicationsettings.h"
#include "albumtreeview.h"
#include "albumlabelstreeview.h"
#include "abstractalbummodel.h"
#include "searchtextbar.h"
#include "albummanager.h"

namespace Digikam
{

class AlbumSelectTabs::Private
{
public:

    Private() :
        albumModel(0),
        albumTreeView(0),
        tagModel(0),
        tagTreeView(0),
        searchModel(0),
        searchTreeView(0),
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
        treeView->setRestoreCheckState(true);
    }

public:

    AlbumModel*               albumModel;
    AlbumTreeView*            albumTreeView;

    TagModel*                 tagModel;
    TagTreeView*              tagTreeView;

    SearchModel*              searchModel;
    SearchTreeView*           searchTreeView;

    SearchTextBar*            albumSearchBar;
    SearchTextBar*            tagSearchBar;
    SearchTextBar*            searchSearchBar;

    AlbumLabelsTreeView*      labelsTree;
    AlbumLabelsSearchHandler* labelsSearchHandler;
};

AlbumSelectTabs::AlbumSelectTabs(const QString& name, QWidget* const parent)
    : QTabWidget(parent),
      d(new Private)
{
    KSharedConfigPtr config  = KSharedConfig::openConfig();
    KConfigGroup configGroup = config->group(QLatin1String("AlbumSelectTabs") +
                               QString::fromLatin1("_%1").arg(name));

    DVBox* const albumBox = new DVBox(this);
    d->albumModel         = new AlbumModel(AbstractAlbumModel::IgnoreRootAlbum, albumBox);
    d->albumTreeView      = new AlbumTreeView(albumBox);
    d->albumTreeView->setAlbumModel(d->albumModel);
    d->albumTreeView->setEntryPrefix(QLatin1String("AlbumTreeView"));
    d->albumTreeView->setConfigGroup(configGroup);
    d->prepareTreeView(d->albumTreeView);

    d->albumSearchBar = new SearchTextBar(albumBox, QLatin1String("AlbumSelectTabsAlbumSearchBar"));
    d->albumSearchBar->setEntryPrefix(QLatin1String("AlbumSearchBar"));
    d->albumSearchBar->setConfigGroup(configGroup);
    d->albumSearchBar->setModel(d->albumModel, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->albumSearchBar->setFilterModel(d->albumTreeView->albumFilterModel());

    albumBox->setContentsMargins(QMargins());
    albumBox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    albumBox->setStretchFactor(d->albumTreeView, 100);
    albumBox->setStretchFactor(d->albumSearchBar, 1);

    // -------------------------------------------------------------------------------

    DVBox* const tagBox = new DVBox(this);
    d->tagModel         = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, tagBox);
    d->tagTreeView      = new TagTreeView(tagBox);
    d->tagTreeView->setAlbumModel(d->tagModel);
    d->tagTreeView->setEntryPrefix(QLatin1String("TagTreeView"));
    d->tagTreeView->setConfigGroup(configGroup);
    d->prepareTreeView(d->tagTreeView);

    d->tagSearchBar = new SearchTextBar(tagBox, QLatin1String("AlbumSelectTabsTagSearchBar"));
    d->tagSearchBar->setEntryPrefix(QLatin1String("TagSearchBar"));
    d->tagSearchBar->setConfigGroup(configGroup);
    d->tagSearchBar->setModel(d->tagTreeView->filteredModel(),
                              AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagSearchBar->setFilterModel(d->tagTreeView->albumFilterModel());

    tagBox->setContentsMargins(QMargins());
    tagBox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    tagBox->setStretchFactor(d->tagTreeView, 100);
    tagBox->setStretchFactor(d->tagSearchBar, 1);

    // -------------------------------------------------------------------------------

    DVBox* const searchBox  = new DVBox(this);
    d->searchModel          = new SearchModel(searchBox);
    d->searchTreeView       = new SearchTreeView(searchBox);
    d->searchTreeView->setAlbumModel(d->searchModel);
    d->searchTreeView->setEntryPrefix(QLatin1String("SearchTreeView"));
    d->searchTreeView->setConfigGroup(configGroup);
    d->searchTreeView->filteredModel()->listNormalSearches();
    d->searchTreeView->filteredModel()->setListTemporarySearches(false);
    d->prepareTreeView(d->searchTreeView);

    d->searchSearchBar = new SearchTextBar(searchBox, QLatin1String("AlbumSelectTabsSearchSearchBar"));
    d->searchSearchBar->setEntryPrefix(QLatin1String("SearchSearchBar"));
    d->searchSearchBar->setConfigGroup(configGroup);
    d->searchSearchBar->setModel(d->searchModel, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchSearchBar->setFilterModel(d->searchTreeView->albumFilterModel());

    searchBox->setContentsMargins(QMargins());
    searchBox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    searchBox->setStretchFactor(d->searchTreeView, 100);
    searchBox->setStretchFactor(d->searchSearchBar, 1);

    // -------------------------------------------------------------------------------

    DVBox* const labelsBox = new DVBox(this);
    d->labelsTree          = new AlbumLabelsTreeView(labelsBox, true);
    d->labelsTree->setEntryPrefix(QLatin1String("LabelsTreeView"));
    d->labelsTree->setConfigGroup(configGroup);
    d->labelsSearchHandler = new AlbumLabelsSearchHandler(d->labelsTree);

    labelsBox->setContentsMargins(QMargins());
    labelsBox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    labelsBox->setStretchFactor(d->labelsTree, 100);

    // -------------------------------------------------------------------------------

    addTab(albumBox,  i18n("Albums"));
    addTab(tagBox,    i18n("Tags"));
    addTab(searchBox, i18n("Searches"));
    addTab(labelsBox, i18n("Labels"));

    // ------------------------------------------------------------------------------------

    connect(d->albumModel, SIGNAL(checkStateChanged(Album*, Qt::CheckState)),
            this, SIGNAL(signalAlbumSelectionChanged()));

    connect(d->tagModel, SIGNAL(checkStateChanged(Album*, Qt::CheckState)),
            this, SIGNAL(signalAlbumSelectionChanged()));

    connect(d->searchModel, SIGNAL(checkStateChanged(Album*, Qt::CheckState)),
            this, SIGNAL(signalAlbumSelectionChanged()));

    connect(d->labelsSearchHandler, SIGNAL(checkStateChanged(Album*, Qt::CheckState)),
            this, SIGNAL(signalAlbumSelectionChanged()));

    // ------------------------------------------------------------------------------------

    d->albumTreeView->loadState();
    d->albumSearchBar->loadState();
    d->tagTreeView->loadState();
    d->tagSearchBar->loadState();
    d->searchTreeView->loadState();
    d->searchSearchBar->loadState();
    d->labelsTree->doLoadState();
}

AlbumSelectTabs::~AlbumSelectTabs()
{
    d->albumTreeView->saveState();
    d->albumSearchBar->saveState();
    d->tagTreeView->saveState();
    d->tagSearchBar->saveState();
    d->searchTreeView->saveState();
    d->searchSearchBar->saveState();
    d->labelsTree->doSaveState();

    delete d;
}

AlbumList AlbumSelectTabs::selectedAlbums() const
{
    AlbumList list;

    list << d->albumModel->checkedAlbums();
    list << d->tagModel->checkedAlbums();
    list << d->searchTreeView->albumModel()->checkedAlbums();
    list << d->labelsSearchHandler->albumForSelectedItems();

    // Remove all null albums.
    list.removeAll(0);

    qCDebug(DIGIKAM_GENERAL_LOG) << list.count() << " albums selected";

    return list;
}

void AlbumSelectTabs::enableVirtualAlbums(bool flag)
{
    setTabEnabled(1, flag);
    setTabEnabled(2, flag);
    setTabEnabled(3, flag);
}

QList<AbstractCheckableAlbumModel*> AlbumSelectTabs::albumModels() const
{
    QList<AbstractCheckableAlbumModel*> list;
    list << d->albumModel;
    list << d->tagModel;
    list << d->searchModel;

    return list;
}

AlbumLabelsSearchHandler* AlbumSelectTabs::albumLabelsHandler() const
{
    return d->labelsSearchHandler;
}

}  // namespace Digikam
