/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-11
 * Description : Table view
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2017 by Simon Frei <freisim93 at gmail dot com>
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

#include "tableview.h"

// Qt includes

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QApplication>
#include <QPointer>

// Local includes

#include "advancedrenamedialog.h"
#include "advancedrenameprocessdialog.h"
#include "applicationsettings.h"
#include "contextmenuhelper.h"
#include "digikam_debug.h"
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

class Q_DECL_HIDDEN TableView::Private
{
public:

    explicit Private()
      : columnProfiles(),
        thumbnailSize(),
        imageViewUtilities(0)
    {
    }

    QList<TableViewColumnProfile> columnProfiles;
    ThumbnailSize                 thumbnailSize;
    ImageViewUtilities*           imageViewUtilities;
};

TableView::TableView(QItemSelectionModel* const selectionModel,
                     DCategorizedSortFilterProxyModel* const imageFilterModel,
                     QWidget* const parent)
    : QWidget(parent),
      StateSavingObject(this),
      d(new Private()),
      s(new TableViewShared())
{
    s->isActive                      = false;
    s->tableView                     = this;
    s->thumbnailLoadThread           = new ThumbnailLoadThread(this);
    s->imageFilterModel              = dynamic_cast<ImageFilterModel*>(imageFilterModel);
    s->imageModel                    = dynamic_cast<ImageModel*>(imageFilterModel->sourceModel());
    s->imageFilterSelectionModel     = selectionModel;
    s->columnFactory                 = new TableViewColumnFactory(s.data(), this);

    QVBoxLayout* const vbox1         = new QVBoxLayout();
    s->tableViewModel                = new TableViewModel(s.data(), this);
    s->tableViewSelectionModel       = new QItemSelectionModel(s->tableViewModel);
    s->tableViewSelectionModelSyncer = new TableViewSelectionModelSyncer(s.data(), this);
    s->treeView                      = new TableViewTreeView(s.data(), this);
    s->treeView->installEventFilter(this);

    d->imageViewUtilities            = new ImageViewUtilities(this);

    connect(s->treeView, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotItemActivated(QModelIndex)));

    connect(s->treeView, SIGNAL(signalZoomInStep()),
            this, SIGNAL(signalZoomInStep()));

    connect(s->treeView, SIGNAL(signalZoomOutStep()),
            this, SIGNAL(signalZoomOutStep()));

    connect(s->tableViewSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SIGNAL(signalItemsChanged()));

    connect(s->treeView, SIGNAL(collapsed(QModelIndex)),
            this, SIGNAL(signalItemsChanged()));

    connect(s->treeView, SIGNAL(expanded(QModelIndex)),
            this, SIGNAL(signalItemsChanged()));

    connect(s->tableViewModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SIGNAL(signalItemsChanged()));

    connect(s->tableViewModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SIGNAL(signalItemsChanged()));

    connect(s->tableViewModel, SIGNAL(layoutChanged()),
            this, SIGNAL(signalItemsChanged()));

    connect(s->tableViewModel, SIGNAL(modelReset()),
            this, SIGNAL(signalItemsChanged()));

    vbox1->addWidget(s->treeView);

    setLayout(vbox1);
}

TableView::~TableView()
{
}

void TableView::doLoadState()
{
    const KConfigGroup group                        = getConfigGroup();

    TableViewColumnProfile profile;
    const KConfigGroup groupCurrentProfile          = group.group("Current Profile");
    profile.loadSettings(groupCurrentProfile);
    s->tableViewModel->loadColumnProfile(profile);

    const TableViewModel::GroupingMode groupingMode = TableViewModel::GroupingMode(group.readEntry<int>("Grouping mode",
                                                      int(TableViewModel::GroupingShowSubItems)));
    s->tableViewModel->setGroupingMode(groupingMode);

    if (!profile.headerState.isEmpty())
    {
        s->treeView->header()->restoreState(profile.headerState);
    }
}

void TableView::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    TableViewColumnProfile profile   = s->tableViewModel->getColumnProfile();
    profile.headerState              = s->treeView->header()->saveState();
    KConfigGroup groupCurrentProfile = group.group("Current Profile");
    profile.saveSettings(groupCurrentProfile);
    group.writeEntry("Grouping mode", int(s->tableViewModel->groupingMode()));
}

void TableView::slotItemActivated(const QModelIndex& tableViewIndex)
{
    const ImageInfo info = s->tableViewModel->imageInfo(tableViewIndex);

    if (info.isNull())
    {
        return;
    }

    if (qApp->queryKeyboardModifiers() != Qt::MetaModifier)
    {
        if (ApplicationSettings::instance()->getItemLeftClickAction() == ApplicationSettings::ShowPreview)
        {
            emit signalPreviewRequested(info);
        }
        else
        {
            d->imageViewUtilities->openInfos(info, allImageInfos(), currentAlbum());
        }
    }
    else
    {
        d->imageViewUtilities->openInfosWithDefaultApplication(QList<ImageInfo>() << info);
    }
}

bool TableView::eventFilter(QObject* watched, QEvent* event)
{
    // we are looking for context menu events for the table view
    if ((watched == s->treeView) && (event->type() == QEvent::ContextMenu))
    {
        QContextMenuEvent* const e = static_cast<QContextMenuEvent*>(event);
        e->accept();

        const QModelIndex contextMenuIndex = s->treeView->indexAt(e->pos());

        if (contextMenuIndex.isValid())
        {
            emit signalShowContextMenuOnInfo(
                        e, s->tableViewModel->imageInfo(contextMenuIndex),
                        getExtraGroupingActions());
        }
        else
        {
            emit signalShowContextMenu(e, getExtraGroupingActions());
        }

        // event has been filtered by us
        return true;
    }

    return QObject::eventFilter(watched, event);
}

void TableView::setThumbnailSize(const ThumbnailSize& size)
{
    d->thumbnailSize                            = size;
    const QList<TableViewColumn*> columnObjects = s->tableViewModel->getColumnObjects();

    foreach(TableViewColumn* const iColumn, columnObjects)
    {
        iColumn->updateThumbnailSize();
    }
}

ThumbnailSize TableView::getThumbnailSize() const
{
    return d->thumbnailSize;
}

Album* TableView::currentAlbum() const
{
    ImageAlbumModel* const albumModel = qobject_cast<ImageAlbumModel*>(s->imageModel);

    if (!albumModel)
    {
        return 0;
    }

    if (albumModel->currentAlbums().isEmpty())
    {
        return 0;
    }

    return albumModel->currentAlbums().first();
}

void TableView::slotPaste()
{
    DragDropViewImplementation* const dragDropViewImplementation = s->treeView;
    dragDropViewImplementation->paste();
}

ImageInfo TableView::currentInfo() const
{
    return s->tableViewModel->imageInfo(s->tableViewSelectionModel->currentIndex());
}

ImageInfoList TableView::allImageInfos(bool grouping) const
{
    if (grouping)
    {
        return s->treeView->resolveGrouping(ImageInfoList(s->tableViewModel->allImageInfo()));
    }

    return ImageInfoList(s->tableViewModel->allImageInfo());
}

bool TableView::allNeedGroupResolving(const ApplicationSettings::OperationType type) const
{
    return s->treeView->needGroupResolving(type, allImageInfos());
}

bool TableView::selectedNeedGroupResolving(const ApplicationSettings::OperationType type) const
{
    return s->treeView->needGroupResolving(type, selectedImageInfos());
}

void TableView::slotDeleteSelected(const ImageViewUtilities::DeleteMode deleteMode)
{
    const ImageInfoList infoList = selectedImageInfos(true);

    /// @todo Update parameter naming for deleteImages
    if (d->imageViewUtilities->deleteImages(infoList, deleteMode))
    {
        slotAwayFromSelection();
    }
}

void TableView::slotDeleteSelectedWithoutConfirmation(const ImageViewUtilities::DeleteMode deleteMode)
{
    const ImageInfoList infoList = selectedImageInfos(true);

    d->imageViewUtilities->deleteImagesDirectly(infoList, deleteMode);
    slotAwayFromSelection();
}

QList<QAction*> TableView::getExtraGroupingActions()
{
    QList<QAction*> actionList;

    const TableViewModel::GroupingMode currentGroupingMode = s->tableViewModel->groupingMode();

    QAction* const actionHideGrouped = new QAction(i18n("Hide grouped items"), this);
    actionHideGrouped->setCheckable(true);
    actionHideGrouped->setChecked(currentGroupingMode == TableViewModel::GroupingHideGrouped);
    actionHideGrouped->setData(QVariant::fromValue<TableViewModel::GroupingMode>(TableViewModel::GroupingHideGrouped));

    connect(actionHideGrouped, SIGNAL(triggered(bool)),
            this, SLOT(slotGroupingModeActionTriggered()));

    actionList << actionHideGrouped;

    QAction* const actionIgnoreGrouping = new QAction(i18n("Ignore grouping"), this);
    actionIgnoreGrouping->setCheckable(true);
    actionIgnoreGrouping->setChecked(currentGroupingMode == TableViewModel::GroupingIgnoreGrouping);
    actionIgnoreGrouping->setData(QVariant::fromValue<TableViewModel::GroupingMode>(TableViewModel::GroupingIgnoreGrouping));

    connect(actionIgnoreGrouping, SIGNAL(triggered(bool)),
            this, SLOT(slotGroupingModeActionTriggered()));

    actionList << actionIgnoreGrouping;

    QAction* const actionShowSubItems = new QAction(i18n("Show grouping in tree"), this);
    actionShowSubItems->setCheckable(true);
    actionShowSubItems->setChecked(currentGroupingMode == TableViewModel::GroupingShowSubItems);
    actionShowSubItems->setData(QVariant::fromValue<TableViewModel::GroupingMode>(TableViewModel::GroupingShowSubItems));

    connect(actionShowSubItems, SIGNAL(triggered(bool)),
            this, SLOT(slotGroupingModeActionTriggered()));

    actionList << actionShowSubItems;

    return actionList;
}

void TableView::slotGroupingModeActionTriggered()
{
    const QAction* const senderAction = qobject_cast<QAction*>(sender());

    if (!senderAction)
    {
        return;
    }

    const TableViewModel::GroupingMode newGroupingMode = senderAction->data().value<TableViewModel::GroupingMode>();
    s->tableViewModel->setGroupingMode(newGroupingMode);
}

int TableView::numberOfSelectedItems() const
{
    return s->tableViewSelectionModel->selectedRows().count();
}

void TableView::slotGoToRow(const int rowNumber, const bool relativeMove)
{
    int nextDeepRowNumber = rowNumber;

    if (relativeMove)
    {
        const QModelIndex currentTableViewIndex = s->tableViewSelectionModel->currentIndex();
        const int currentDeepRowNumber          = s->tableViewModel->indexToDeepRowNumber(currentTableViewIndex);
        nextDeepRowNumber                      += currentDeepRowNumber;
    }

    const QModelIndex nextIndex = s->tableViewModel->deepRowIndex(nextDeepRowNumber);

    if (nextIndex.isValid())
    {
        const QItemSelection rowSelection = s->tableViewSelectionModelSyncer->targetIndexToRowItemSelection(nextIndex);
        s->tableViewSelectionModel->select(rowSelection, QItemSelectionModel::ClearAndSelect);
        s->tableViewSelectionModel->setCurrentIndex(nextIndex, QItemSelectionModel::Select);
    }
}

ImageInfo TableView::deepRowImageInfo(const int rowNumber, const bool relative) const
{
    int targetRowNumber = rowNumber;

    if (relative)
    {
        const QModelIndex& currentTableViewIndex = s->tableViewSelectionModel->currentIndex();

        if (!currentTableViewIndex.isValid())
        {
            return ImageInfo();
        }

        const int currentDeepRowNumber = s->tableViewModel->indexToDeepRowNumber(currentTableViewIndex);
        targetRowNumber               += currentDeepRowNumber;
    }

    const QModelIndex targetIndex = s->tableViewModel->deepRowIndex(targetRowNumber);
    return s->tableViewModel->imageInfo(targetIndex);
}

ImageInfo TableView::nextInfo() const
{
    const QModelIndex cIndex       = s->tableViewSelectionModel->currentIndex();
    const int currentDeepRowNumber = s->tableViewModel->indexToDeepRowNumber(cIndex);
    const int nextDeepRowNumber    = currentDeepRowNumber + 1;

    if (nextDeepRowNumber>=s->tableViewModel->deepRowCount())
    {
        return ImageInfo();
    }

    const QModelIndex nextDeepRowIndex = s->tableViewModel->deepRowIndex(nextDeepRowNumber);
    return s->tableViewModel->imageInfo(nextDeepRowIndex);
}

ImageInfo TableView::previousInfo() const
{
    const QModelIndex cIndex        = s->tableViewSelectionModel->currentIndex();
    const int currentDeepRowNumber  = s->tableViewModel->indexToDeepRowNumber(cIndex);
    const int previousDeepRowNumber = currentDeepRowNumber - 1;

    if (previousDeepRowNumber < 0)
    {
        return ImageInfo();
    }

    const QModelIndex previousDeepRowIndex = s->tableViewModel->deepRowIndex(previousDeepRowNumber);
    return s->tableViewModel->imageInfo(previousDeepRowIndex);
}

void TableView::slotSetCurrentWhenAvailable(const qlonglong id)
{
    const QModelIndex idx = s->tableViewModel->indexFromImageId(id, 0);

    if (!idx.isValid())
    {
        /// @todo Actually buffer this request until the model is fully populated
        return;
    }

    s->tableViewSelectionModel->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Unselects the current selection and changes the current item
 *
 * @todo This may not work correctly if grouped items are deleted, but are not selected
 */
void TableView::slotAwayFromSelection()
{
    QModelIndexList selection = s->tableViewSelectionModel->selectedRows(0);

    if (selection.isEmpty())
    {
        return;
    }

    const QModelIndex firstIndex = s->tableViewModel->deepRowIndex(0);
    const QModelIndex lastIndex  = s->tableViewModel->deepRowIndex(-1);

    if (selection.contains(firstIndex) && selection.contains(lastIndex))
    {
        // both the first and the last index are selected, we have to
        // select an index inbetween
        const int nextFreeDeepRow = s->tableViewModel->firstDeepRowNotInList(selection);

        if (nextFreeDeepRow < 0)
        {
            s->tableViewSelectionModel->clearSelection();
            s->tableViewSelectionModel->setCurrentIndex(QModelIndex(), QItemSelectionModel::ClearAndSelect);
        }
        else
        {
            const QModelIndex nextFreeIndex         = s->tableViewModel->deepRowIndex(nextFreeDeepRow);
            s->tableViewSelectionModel->setCurrentIndex(nextFreeIndex, QItemSelectionModel::ClearAndSelect);
            const QItemSelection nextFreeIndexAsRow = s->tableViewSelectionModelSyncer->targetIndexToRowItemSelection(nextFreeIndex);
            s->tableViewSelectionModel->select(nextFreeIndexAsRow, QItemSelectionModel::ClearAndSelect);
        }
    }
    else if (selection.contains(lastIndex))
    {
        const int firstSelectedRowNumber   = s->tableViewModel->indexToDeepRowNumber(selection.first());
        const QModelIndex newIndex         = s->tableViewModel->deepRowIndex(firstSelectedRowNumber-1);
        s->tableViewSelectionModel->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
        const QItemSelection newIndexAsRow = s->tableViewSelectionModelSyncer->targetIndexToRowItemSelection(newIndex);
        s->tableViewSelectionModel->select(newIndexAsRow, QItemSelectionModel::ClearAndSelect);
    }
    else
    {
        const int lastSelectedRowNumber    = s->tableViewModel->indexToDeepRowNumber(selection.last());
        const QModelIndex newIndex         = s->tableViewModel->deepRowIndex(lastSelectedRowNumber+1);
        s->tableViewSelectionModel->setCurrentIndex(newIndex, QItemSelectionModel::ClearAndSelect);
        const QItemSelection newIndexAsRow = s->tableViewSelectionModelSyncer->targetIndexToRowItemSelection(newIndex);
        s->tableViewSelectionModel->select(newIndexAsRow, QItemSelectionModel::ClearAndSelect);
    }
}

void TableView::clearSelection()
{
    s->tableViewSelectionModel->clearSelection();
}

void TableView::invertSelection()
{
    const int deepRowCount = s->tableViewModel->deepRowCount();
    QList<int> rowsToSelect;
    int lastSelectedRow    = -1;

    /// @todo Create a DeepRowIterator because there is a lot of overhead here

    for (int i = 0; i < deepRowCount; ++i)
    {
        const QModelIndex iIndex = s->tableViewModel->deepRowIndex(i);

        if (s->tableViewSelectionModel->isSelected(iIndex))
        {
            if (i - 1 > lastSelectedRow)
            {
                for (int j = lastSelectedRow + 1; j < i; ++j)
                {
                    rowsToSelect << j;
                }
            }

            lastSelectedRow = i;
        }
    }

    if (lastSelectedRow + 1 < deepRowCount)
    {
        for (int j = lastSelectedRow + 1; j < deepRowCount; ++j)
        {
            rowsToSelect << j;
        }
    }

    s->tableViewSelectionModel->clearSelection();

    foreach(const int i, rowsToSelect)
    {
        const QModelIndex iIndex = s->tableViewModel->deepRowIndex(i);
        const QItemSelection is  = s->tableViewSelectionModelSyncer->targetIndexToRowItemSelection(iIndex);
        s->tableViewSelectionModel->select(is, QItemSelectionModel::Select);
    }
}

void TableView::selectAll()
{
    /// @todo This only selects expanded items.
    s->treeView->selectAll();
}

void TableView::slotSetActive(const bool isActive)
{
    if (s->isActive != isActive)
    {
        s->isActive = isActive;
        s->tableViewModel->slotSetActive(isActive);
        s->tableViewSelectionModelSyncer->slotSetActive(isActive);
    }
}

ImageInfoList TableView::selectedImageInfos(bool grouping) const
{
    ImageInfoList infos = ImageInfoList(s->tableViewModel->imageInfos(s->tableViewSelectionModel->selectedRows()));

    if (grouping)
    {
        return s->treeView->resolveGrouping(infos);
    }

    return infos;
}

ImageInfoList TableView::selectedImageInfosCurrentFirst(bool grouping) const
{
    QModelIndexList indexes   = s->tableViewSelectionModel->selectedRows();
    const QModelIndex current = s->tableViewModel->toCol0(s->tableViewSelectionModel->currentIndex());

    if (!indexes.isEmpty())
    {
        if (indexes.first() != current)
        {
            if (indexes.removeOne(current))
            {
                indexes.prepend(current);
            }
        }
    }

    if (grouping)
    {
        return s->treeView->resolveGrouping(ImageInfoList(s->tableViewModel->imageInfos(indexes)));
    }

    return ImageInfoList(s->tableViewModel->imageInfos(indexes));
}

void TableView::rename()
{
    ImageInfoList infos = selectedImageInfos();

    if (s->treeView->needGroupResolving(ApplicationSettings::Rename, infos))
    {
        infos = s->treeView->resolveGrouping(infos);
    }

    QList<QUrl> urls = infos.toImageUrlList();
    bool loop        = false;
    NewNamesList newNamesList;

    do
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Selected URLs to rename: " << urls;

        QPointer<AdvancedRenameDialog> dlg = new AdvancedRenameDialog(this);
        dlg->slotAddImages(urls);

        if (dlg->exec() != QDialog::Accepted)
        {
            delete dlg;
            break;
        }

        if (!loop)
        {
            slotAwayFromSelection();
            loop = true;
        }

        newNamesList = dlg->newNames();
        delete dlg;

        setFocus();
        qApp->processEvents();

        if (!newNamesList.isEmpty())
        {
            QPointer<AdvancedRenameProcessDialog> dlg = new AdvancedRenameProcessDialog(newNamesList, this);
            dlg->exec();

            s->tableViewModel->scheduleResort();
            urls = dlg->failedUrls();
            delete dlg;
        }
    }
    while (!urls.isEmpty() && !newNamesList.isEmpty());
}

} // namespace Digikam
