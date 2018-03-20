/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-26-02
 * Description : a widget to select a physical album
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
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

// Qt includes

#include <QIcon>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QPushButton>
#include <QAction>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummodificationhelper.h"
#include "albumtreeview.h"
#include "albumthumbnailloader.h"
#include "collectionmanager.h"
#include "contextmenuhelper.h"

namespace Digikam
{

class AlbumSelectTreeView::Private
{
public:

    Private() :
        albumModificationHelper(0),
        newAlbumAction(0)
    {
    }

    AlbumModificationHelper* albumModificationHelper;
    QAction*                 newAlbumAction;
};

AlbumSelectTreeView::AlbumSelectTreeView(AlbumModel* const model,
                                         AlbumModificationHelper* const albumModificationHelper,
                                         QWidget* const parent)
    : AlbumTreeView(parent),
      d(new Private)
{
    setAlbumModel(model);
    d->albumModificationHelper = albumModificationHelper;
    d->newAlbumAction          = new QAction(QIcon::fromTheme(QLatin1String("folder-new")),
                                             i18n("Create New Album"), this);
}

AlbumSelectTreeView::~AlbumSelectTreeView()
{
    delete d;
}

void AlbumSelectTreeView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    cmh.addAction(d->newAlbumAction);
    d->newAlbumAction->setEnabled(album);
}

void AlbumSelectTreeView::handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album)
{
    Album* const a       = album;
    PAlbum* const palbum = dynamic_cast<PAlbum*>(a);

    if (palbum && action == d->newAlbumAction)
    {
        d->albumModificationHelper->slotAlbumNew(palbum);
    }
}

void AlbumSelectTreeView::slotNewAlbum()
{
    PAlbum* const palbum = currentAlbum();

    if (palbum)
    {
        PAlbum* const createdAlbum = d->albumModificationHelper->slotAlbumNew(palbum);

        if (createdAlbum)
        {
            setCurrentAlbums(QList<Album*>() << createdAlbum, false);
        }
    }
}

// --------------------------------------------------------------------------------------------------------

class AlbumSelectWidget::Private
{
public:

    Private() :
        albumModel(0),
        albumTreeView(0),
        albumModificationHelper(0),
        searchBar(0),
        newAlbumBtn(0)
    {
    }

    AlbumModel*              albumModel;
    AlbumSelectTreeView*     albumTreeView;

    AlbumModificationHelper* albumModificationHelper;

    SearchTextBar*           searchBar;

    QPushButton*             newAlbumBtn;
};

AlbumSelectWidget::AlbumSelectWidget(QWidget* const parent, PAlbum* const albumToSelect)
    : QWidget(parent),
      d(new Private)
{
    setObjectName(QLatin1String("AlbumSelectWidget"));

    d->albumModificationHelper = new AlbumModificationHelper(this, this);

    // TODO let this class implement StateSavingObject
    KConfigGroup group = KSharedConfig::openConfig()->group(objectName());

    QGridLayout* const grid = new QGridLayout(this);
    d->albumModel           = new AlbumModel(AbstractAlbumModel::IgnoreRootAlbum, this);
    d->albumTreeView        = new AlbumSelectTreeView(d->albumModel, d->albumModificationHelper, this);
    d->albumTreeView->setDragEnabled(false);
    d->albumTreeView->setDropIndicatorShown(false);
    d->albumTreeView->setAcceptDrops(false);
    d->albumTreeView->setSelectAlbumOnClick(false);
    d->albumTreeView->setSelectOnContextMenu(false);
    d->albumTreeView->setEnableContextMenu(true);
    d->albumTreeView->setSortingEnabled(true);
    d->albumTreeView->setConfigGroup(group);
    d->albumTreeView->setEntryPrefix(QLatin1String("AlbumTreeView"));

    d->searchBar   = new SearchTextBar(this, QLatin1String("AlbumSelectWidgetSearchBar"));
    d->searchBar->setModel(d->albumModel, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchBar->setFilterModel(d->albumTreeView->albumFilterModel());
    d->searchBar->setConfigGroup(group);
    d->albumTreeView->setEntryPrefix(QLatin1String("AlbumTreeView"));

    d->newAlbumBtn = new QPushButton(i18n("&New Album"), this);
    d->newAlbumBtn->setToolTip(i18n("Create new album"));
    d->newAlbumBtn->setIcon(QIcon::fromTheme(QLatin1String("folder-new")));

    grid->addWidget(d->albumTreeView, 0, 0, 1, 2);
    grid->addWidget(d->searchBar,     1, 0, 1, 1);
    grid->addWidget(d->newAlbumBtn,   1, 1, 1, 1);
    grid->setRowStretch(0, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // ------------------------------------------------------------------------------------

    PAlbum* select = albumToSelect;

    if (!select)
    {
        select = AlbumManager::instance()->currentPAlbum();
    }

    d->albumTreeView->setCurrentAlbums(QList<Album*>() << select, false);

    // ------------------------------------------------------------------------------------

    connect(d->albumTreeView, SIGNAL(currentAlbumChanged(Album*)),
            this, SIGNAL(itemSelectionChanged()));

    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    connect(d->newAlbumBtn, SIGNAL(clicked()),
            d->albumTreeView, SLOT(slotNewAlbum()));

    d->albumTreeView->loadState();
    d->searchBar->loadState();
}

AlbumSelectWidget::~AlbumSelectWidget()
{
    d->albumTreeView->saveState();
    d->searchBar->saveState();
    delete d;
}

PAlbum* AlbumSelectWidget::currentAlbum() const
{
    return d->albumTreeView->currentAlbum();
}

void AlbumSelectWidget::setCurrentAlbum(PAlbum* const albumToSelect)
{
    d->albumTreeView->setCurrentAlbums(QList<Album*>() << albumToSelect);
}

QUrl AlbumSelectWidget::currentAlbumUrl() const
{
    PAlbum* const palbum = d->albumTreeView->currentAlbum();

    if (palbum)
    {
        return palbum->fileUrl();
    }

    return QUrl();
}

void AlbumSelectWidget::setCurrentAlbumUrl(const QUrl& albumUrl)
{
    PAlbum* const urlAlbum = AlbumManager::instance()->findPAlbum(albumUrl);

    if (urlAlbum)
    {
        d->albumTreeView->setCurrentAlbums(QList<Album*>() << urlAlbum);
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot find an album for " << albumUrl;
    }
}

void AlbumSelectWidget::slotAlbumRenamed(Album* album)
{
    if (!album || album->type() != Album::PHYSICAL)
    {
        return;
    }

    QModelIndex index = d->albumModel->indexForAlbum(album);

    if (index.isValid())
    {
        emit itemSelectionChanged();
    }
}

} // namespace Digikam
