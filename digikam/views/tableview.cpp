/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-11
 * Description : Table view
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "tableview.moc"

// Qt includes

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTreeView>
#include <QVBoxLayout>

// KDE includes

#include <kmenu.h>
#include <kaction.h>
#include <klinkitemselectionmodel.h>

// local includes

/// @todo clean up includes
#include "imageposition.h"
#include "imageinfo.h"
#include "imagemodel.h"
#include "importfiltermodel.h"
#include "importimagemodel.h"
#include "databasewatch.h"
#include "databasefields.h"
#include "digikam2kgeomap_database.h"
#include "importui.h"
#include "thumbnailloadthread.h"
#include "tableview_model.h"
#include "tableview_columnfactory.h"
#include "tableview_selection_model_syncer.h"
#include "contextmenuhelper.h"
#include "fileactionmngr.h"
#include "tableview_treeview.h"
#include "tableview_sortfilterproxymodel.h"

namespace Digikam
{

class ImageAlbumModel;
class ImageFilterModel;

class TableView::Private
{
public:
    Private()
      : treeView(0),
        columnProfiles(),
        thumbnailSize()
    {
    }

    TableViewTreeView*      treeView;
    QList<TableViewColumnProfile> columnProfiles;
    ThumbnailSize           thumbnailSize;
};

TableView::TableView(
        QItemSelectionModel* const selectionModel,
        KCategorizedSortFilterProxyModel* const imageFilterModel,
        QWidget* const parent
    )
  : QWidget(parent),
    StateSavingObject(this),
    d(new Private()),
    s(new TableViewShared())
{
    s->tableView = this;
    s->thumbnailLoadThread = new ThumbnailLoadThread(this);
    s->imageFilterModel = dynamic_cast<ImageFilterModel*>(imageFilterModel);
    s->imageModel = dynamic_cast<ImageModel*>(imageFilterModel->sourceModel());
    s->imageFilterSelectionModel = selectionModel;
    s->columnFactory = new TableViewColumnFactory(s.data(), this);

    QVBoxLayout* const vbox1 = new QVBoxLayout();

    s->tableViewModel = new TableViewModel(s.data(), this);
    s->tableViewSelectionModel = new QItemSelectionModel(s->tableViewModel);
    s->sortModel = new TableViewSortFilterProxyModel(s.data(), this);
    s->sortSelectionModel = new KLinkItemSelectionModel(s->sortModel, s->tableViewSelectionModel);
    s->tableViewCurrentToSortedSyncer = new TableViewCurrentToSortedSyncer(s.data(), this);
    s->tableViewSelectionModelSyncer= new TableViewSelectionModelSyncer(s.data(), this);
    d->treeView = new TableViewTreeView(s.data(), this);
    d->treeView->installEventFilter(this);

    connect(d->treeView, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotItemActivated(QModelIndex)));

    connect(d->treeView, SIGNAL(signalZoomInStep()),
            this, SIGNAL(signalZoomInStep()));

    connect(d->treeView, SIGNAL(signalZoomOutStep()),
            this, SIGNAL(signalZoomOutStep()));

    vbox1->addWidget(d->treeView);

    setLayout(vbox1);
}

TableView::~TableView()
{

}

void TableView::doLoadState()
{
    const KConfigGroup group = getConfigGroup();

    TableViewColumnProfile profile;
    const KConfigGroup groupCurrentProfile = group.group("Current Profile");
    profile.loadSettings(groupCurrentProfile);
    s->tableViewModel->loadColumnProfile(profile);

    if (!profile.headerState.isEmpty())
    {
        d->treeView->header()->restoreState(profile.headerState);
    }
}

void TableView::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    TableViewColumnProfile profile = s->tableViewModel->getColumnProfile();
    profile.headerState = d->treeView->header()->saveState();
    KConfigGroup groupCurrentProfile = group.group("Current Profile");
    profile.saveSettings(groupCurrentProfile);
}

void TableView::slotItemActivated(const QModelIndex& sortedIndex)
{
    const QModelIndex& tableViewIndex = s->sortModel->mapToSource(sortedIndex);
    const QModelIndex& imageFilterModelIndex = s->tableViewModel->toImageFilterModelIndex(tableViewIndex);

    if (!imageFilterModelIndex.isValid())
    {
        return;
    }

    const ImageInfo info = s->imageFilterModel->imageInfo(imageFilterModelIndex);

    /// @todo Respect edit/preview setting
    emit signalPreviewRequested(info);
}

bool TableView::eventFilter(QObject* watched, QEvent* event)
{
    // we are looking for context menu events for the table view
    if ((watched==d->treeView)&&(event->type()==QEvent::ContextMenu))
    {
        QContextMenuEvent* const e = static_cast<QContextMenuEvent*>(event);
        e->accept();
        showTreeViewContextMenu(e);

        // event has been filtered by us
        return true;
    }

    return QObject::eventFilter(watched, event);
}

void TableView::showTreeViewContextMenu(QContextMenuEvent* const event)
{
    KMenu menu(this);
    ContextMenuHelper cmHelper(&menu);

    // get a list of currently selected images' ids
    const QModelIndexList selectedIndexes = s->imageFilterSelectionModel->selectedIndexes();
    const QList<qlonglong> selectedImageIds =  s->imageFilterModel->imageIds(selectedIndexes);

    cmHelper.addAssignTagsMenu(selectedImageIds);
    cmHelper.addRemoveTagsMenu(selectedImageIds);
    cmHelper.addSeparator();
    cmHelper.addLabelsAction();

    connect(&cmHelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabelToSelected(int)));
    connect(&cmHelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabelToSelected(int)));
    connect(&cmHelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRatingToSelected(int)));
    connect(&cmHelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(slotAssignTagToSelected(int)));
    connect(&cmHelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTagFromSelected(int)));

    menu.exec(event->globalPos());
}

QList< ImageInfo > TableView::selectedImageInfos() const
{
    const QModelIndexList selectedIndexes = s->imageFilterSelectionModel->selectedIndexes();
    return s->imageFilterModel->imageInfos(selectedIndexes);
}

void TableView::slotAssignColorLabelToSelected(const int colorLabelID)
{
    FileActionMngr::instance()->assignColorLabel(selectedImageInfos(), colorLabelID);
}

void TableView::slotAssignPickLabelToSelected(const int pickLabelID)
{
    FileActionMngr::instance()->assignPickLabel(selectedImageInfos(), pickLabelID);
}

void TableView::slotAssignRatingToSelected(const int rating)
{
    FileActionMngr::instance()->assignRating(selectedImageInfos(), rating);
}

void TableView::slotAssignTagToSelected(const int tagID)
{
    FileActionMngr::instance()->assignTags(selectedImageInfos(), QList<int>() << tagID);
}

void TableView::slotRemoveTagFromSelected(const int tagID)
{
    FileActionMngr::instance()->removeTags(selectedImageInfos(), QList<int>() << tagID);
}

void TableView::setThumbnailSize(const ThumbnailSize& size)
{
    d->thumbnailSize = size;

    const QList<TableViewColumn*> columnObjects = s->tableViewModel->getColumnObjects();
    Q_FOREACH(TableViewColumn* const iColumn, columnObjects)
    {
        iColumn->updateThumbnailSize();
    }
}

ThumbnailSize TableView::getThumbnailSize() const
{
    return d->thumbnailSize;
}

} /* namespace Digikam */
