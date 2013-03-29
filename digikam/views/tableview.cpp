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
#include <QVBoxLayout>

// KDE includes

#include <kaction.h>
#include <kmenu.h>
#include <klinkitemselectionmodel.h>

// local includes

#include "contextmenuhelper.h"
#include "digikam2kgeomap_database.h"
#include "fileactionmngr.h"
#include "album.h"
#include "imageviewutilities.h"
#include "tableview_columnfactory.h"
#include "tableview_model.h"
#include "tableview_selection_model_syncer.h"
#include "tableview_shared.h"
#include "tableview_treeview.h"

namespace Digikam
{

class ImageAlbumModel;
class ImageFilterModel;

class TableView::Private
{
public:
    Private()
      : columnProfiles(),
        thumbnailSize(),
        imageViewUtilities(0)
    {
    }

    QList<TableViewColumnProfile> columnProfiles;
    ThumbnailSize                 thumbnailSize;
    ImageViewUtilities*           imageViewUtilities;
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
    s->tableViewSelectionModelSyncer= new TableViewSelectionModelSyncer(s.data(), this);
    s->treeView = new TableViewTreeView(s.data(), this);
    s->treeView->installEventFilter(this);

    d->imageViewUtilities = new ImageViewUtilities(this);

    connect(s->treeView, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotItemActivated(QModelIndex)));

    connect(s->treeView, SIGNAL(signalZoomInStep()),
            this, SIGNAL(signalZoomInStep()));

    connect(s->treeView, SIGNAL(signalZoomOutStep()),
            this, SIGNAL(signalZoomOutStep()));

    vbox1->addWidget(s->treeView);

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
        s->treeView->header()->restoreState(profile.headerState);
    }
}

void TableView::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    TableViewColumnProfile profile = s->tableViewModel->getColumnProfile();
    profile.headerState = s->treeView->header()->saveState();
    KConfigGroup groupCurrentProfile = group.group("Current Profile");
    profile.saveSettings(groupCurrentProfile);
}

void TableView::slotItemActivated(const QModelIndex& tableViewIndex)
{
    const ImageInfo info = s->tableViewModel->imageInfo(tableViewIndex);

    /// @todo Respect edit/preview setting
    emit signalPreviewRequested(info);
}

bool TableView::eventFilter(QObject* watched, QEvent* event)
{
    // we are looking for context menu events for the table view
    if ((watched==s->treeView)&&(event->type()==QEvent::ContextMenu))
    {
        QContextMenuEvent* const e = static_cast<QContextMenuEvent*>(event);
        e->accept();

        const QModelIndex contextMenuIndex = s->treeView->indexAt(e->pos());
        if (contextMenuIndex.isValid())
        {
            showTreeViewContextMenuOnItem(e, contextMenuIndex);
        }
        else
        {
            showTreeViewContextMenuOnEmptyArea(e);
        }

        // event has been filtered by us
        return true;
    }

    return QObject::eventFilter(watched, event);
}

void TableView::showTreeViewContextMenuOnEmptyArea(QContextMenuEvent* const event)
{
    Album* const album = currentAlbum();

    if (!album ||
        album->isRoot() ||
        (album->type() != Album::PHYSICAL && album->type() != Album::TAG) )
    {
        return;
    }

    KMenu menu(this);
    ContextMenuHelper cmHelper(&menu);

    cmHelper.addAction("full_screen");
    cmHelper.addSeparator();
    cmHelper.addStandardActionPaste(this, SLOT(slotPaste()));

    cmHelper.exec(event->globalPos());
}

void TableView::showTreeViewContextMenuOnItem(QContextMenuEvent* const event, const QModelIndex& indexAtMenu)
{
    // get a list of currently selected images' ids
    const QList<qlonglong> selectedImageIds =  selectedImageIdsCurrentFirst();
    kDebug()<<selectedImageIds;

    // Temporary actions --------------------------------------

    KAction* const viewAction = new KAction(i18nc("View the selected image", "Preview"), this);
    viewAction->setIcon(SmallIcon("viewimage"));
    viewAction->setEnabled(selectedImageIds.count() == 1);

    // Creation of the menu -----------------------------------
    KMenu menu(this);
    ContextMenuHelper cmHelper(&menu);

    cmHelper.addAction("full_screen");
    cmHelper.addSeparator();
    // ---
    cmHelper.addAction("move_selection_to_album");
    cmHelper.addAction(viewAction);
    cmHelper.addAction("image_edit");
    cmHelper.addServicesMenu(s->tableViewModel->selectedUrls());
    cmHelper.addGotoMenu(selectedImageIds);
    cmHelper.addAction("image_rotate");
    cmHelper.addSeparator();
    // ---
    cmHelper.addAction("image_find_similar");
    cmHelper.addStandardActionLightTable();
    cmHelper.addQueueManagerMenu();
    cmHelper.addSeparator();
    // ---
    cmHelper.addAction("image_rename");
    cmHelper.addAction("cut_album_selection");
    cmHelper.addAction("copy_album_selection");
    cmHelper.addAction("paste_album_selection");
    cmHelper.addStandardActionItemDelete(this, SLOT(slotDeleteSelected()), selectedImageIds.count());
    cmHelper.addSeparator();
    // ---
    cmHelper.addStandardActionThumbnail(selectedImageIds, currentAlbum());
    // ---
    cmHelper.addAssignTagsMenu(selectedImageIds);
    cmHelper.addRemoveTagsMenu(selectedImageIds);
    cmHelper.addSeparator();
    cmHelper.addLabelsAction();

    /// @todo check in digikamimageview.cpp how this works
//     if (!d->faceMode)
//     {
//         cmHelper.addGroupMenu(selectedImageIDs);
//     }

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
    connect(&cmHelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SLOT(slotInsertSelectedToExistingQueue(int)));
    connect(&cmHelper, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));
    connect(&cmHelper, SIGNAL(signalGotoTag(int)),
            this, SIGNAL(signalGotoTagAndImageRequested(int)));
    connect(&cmHelper, SIGNAL(signalGotoAlbum(ImageInfo)),
            this, SIGNAL(signalGotoAlbumAndImageRequested(ImageInfo)));
    connect(&cmHelper, SIGNAL(signalGotoDate(ImageInfo)),
            this, SIGNAL(signalGotoDateAndImageRequested(ImageInfo)));
    connect(&cmHelper, SIGNAL(signalSetThumbnail(ImageInfo)),
            this, SLOT(slotSetAsAlbumThumbnail(ImageInfo)));


    QAction* const choice = cmHelper.exec(event->globalPos());

    if (choice && (choice == viewAction) )
    {
        emit(signalPreviewRequested(s->tableViewModel->imageInfo(indexAtMenu)));
    }
}

QList<ImageInfo> TableView::selectedImageInfos() const
{
    const QModelIndexList selectedIndexes = s->tableViewSelectionModel->selectedIndexes();
    kDebug()<<selectedIndexes;

    return s->tableViewModel->imageInfos(selectedIndexes);
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

QList<qlonglong> TableView::selectedImageIdsCurrentFirst() const
{
    const QModelIndexList selectedIndexes = s->tableViewSelectionModel->selectedIndexes();
    QList<qlonglong> selectedImageIds =  s->tableViewModel->imageIds(selectedIndexes);

    const QModelIndex currentIndex = s->tableViewSelectionModel->currentIndex();
    qlonglong currentId = s->tableViewModel->imageId(currentIndex);
    if (currentId>=0)
    {
        if (selectedImageIds.first()!=currentId)
        {
            selectedImageIds.removeOne(currentId);
            selectedImageIds.prepend(currentId);
        }
    }
    kDebug()<<selectedImageIds;
    return selectedImageIds;
}

void TableView::slotInsertSelectedToExistingQueue(const int queueId)
{
    kDebug()<<"_________________________"<<queueId;
    const ImageInfoList imageInfoList = selectedImageInfos();

    if (!imageInfoList.isEmpty())
    {
        d->imageViewUtilities->insertSilentToQueueManager(imageInfoList, imageInfoList.first(), queueId);
    }
}

void TableView::slotSetAsAlbumThumbnail(const ImageInfo& info)
{
    Album* const theCurrentAlbum = currentAlbum();
    if (!theCurrentAlbum)
    {
        return;
    }

    d->imageViewUtilities->setAsAlbumThumbnail(theCurrentAlbum, info);
}

Album* TableView::currentAlbum()
{
    ImageAlbumModel* const albumModel = qobject_cast<ImageAlbumModel*>(s->imageModel);

    if (!albumModel)
    {
        return 0;
    }

    return albumModel->currentAlbum();
}

void TableView::slotPaste()
{
    DragDropViewImplementation* const dragDropViewImplementation = s->treeView;
    dragDropViewImplementation->paste();
}

ImageInfo TableView::currentInfo()
{
    return s->tableViewModel->imageInfo(s->tableViewSelectionModel->currentIndex());
}

ImageInfoList TableView::allInfo() const
{
    return s->tableViewModel->allImageInfo();
}

void TableView::slotDeleteSelected(const bool permanently)
{
    const ImageInfoList infoList = selectedImageInfos();

    d->imageViewUtilities->deleteImages(infoList, permanently);
}

} /* namespace Digikam */
