/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-01
 * Description : A widget to search for places.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#include "searchwidget.h"

// Qt includes

#include <QContextMenuEvent>
#include <QListView>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QStandardPaths>
#include <QLineEdit>
#include <QMessageBox>
#include <QItemSelectionModel>
#include <QItemSelection>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "mapwidget.h"
#include "searchbackend.h"
#include "searchresultmodel.h"
#include "searchresultmodelhelper.h"
#include "gpscommon.h"
#include "gpsundocommand.h"
#include "gpsimagemodel.h"
#include "gpsbookmarkowner.h"

#ifdef GPSSYNC_MODELTEST
#   include <modeltest.h>
#endif

namespace Digikam
{

static int QItemSelectionModel_selectedRowsCount(const QItemSelectionModel* const selectionModel)
{
    if (!selectionModel->hasSelection())
    {
        return 0;
    }

    return selectionModel->selectedRows().count();
}

class SearchWidget::Private
{
public:

    Private()
    {
        gpsBookmarkOwner                              = 0;
        actionBookmark                                = 0;
        mapWidget                                     = 0;
        gpsImageModel                                 = 0;
        gosImageSelectionModel                        = 0;
        searchTermLineEdit                            = 0;
        searchButton                                  = 0;
        searchBackend                                 = 0;
        searchResultsModel                            = 0;
        searchResultsSelectionModel                   = 0;
        searchResultModelHelper                       = 0;
        treeView                                      = 0;
        mainVBox                                      = 0;
        backendSelectionBox                           = 0;
        actionClearResultsList                        = 0;
        actionKeepOldResults                          = 0;
        actionToggleAllResultsVisibility              = 0;
        actionCopyCoordinates                         = 0;
        actionMoveImagesToThisResult                  = 0;
        actionRemovedSelectedSearchResultsFromList    = 0;
        searchInProgress                              = false;
        actionToggleAllResultsVisibilityIconUnchecked = QIcon::fromTheme(QString::fromLatin1("layer-visible-off"));
        actionToggleAllResultsVisibilityIconChecked   = QIcon::fromTheme(QString::fromLatin1("layer-visible-on"));
    }

    // Map
    MapWidget*               mapWidget;
    GPSImageModel*           gpsImageModel;
    QItemSelectionModel*     gosImageSelectionModel;
    QLineEdit*               searchTermLineEdit;
    QPushButton*             searchButton;
    GPSBookmarkOwner*        gpsBookmarkOwner;
    QAction*                 actionBookmark;

    // Search: backend
    SearchBackend*           searchBackend;
    SearchResultModel*       searchResultsModel;
    QItemSelectionModel*     searchResultsSelectionModel;
    SearchResultModelHelper* searchResultModelHelper;

    // Search: UI
    QTreeView*               treeView;
    QVBoxLayout*             mainVBox;
    QComboBox*               backendSelectionBox;
    QAction*                 actionClearResultsList;
    QAction*                 actionKeepOldResults;
    QAction*                 actionToggleAllResultsVisibility;
    QAction*                 actionCopyCoordinates;
    QAction*                 actionMoveImagesToThisResult;
    QAction*                 actionRemovedSelectedSearchResultsFromList;
    bool                     searchInProgress;
    QIcon                    actionToggleAllResultsVisibilityIconUnchecked;
    QIcon                    actionToggleAllResultsVisibilityIconChecked;
};

SearchWidget::SearchWidget(GPSBookmarkOwner* const gpsBookmarkOwner,
                           GPSImageModel* const gpsImageModel,
                           QItemSelectionModel* const gosImageSelectionModel,
                           QWidget* const parent
                          )
    : QWidget(parent),
      d(new Private())
{
    d->gpsBookmarkOwner       = gpsBookmarkOwner;
    d->gpsImageModel          = gpsImageModel;
    d->gosImageSelectionModel = gosImageSelectionModel;
    d->searchBackend          = new SearchBackend(this);
    d->searchResultsModel     = new SearchResultModel(this);

#ifdef GPSSYNC_MODELTEST
    new ModelTest(d->searchResultsModel, this);
#endif

    d->searchResultsSelectionModel = new QItemSelectionModel(d->searchResultsModel);
    d->searchResultsModel->setSelectionModel(d->searchResultsSelectionModel);
    d->searchResultModelHelper     = new SearchResultModelHelper(d->searchResultsModel, d->searchResultsSelectionModel, d->gpsImageModel, this);

    d->mainVBox = new QVBoxLayout(this);
    setLayout(d->mainVBox);

    DHBox* const topHBox  = new DHBox(this);
    d->mainVBox->addWidget(topHBox);
    d->searchTermLineEdit = new QLineEdit(topHBox);
    d->searchTermLineEdit->setClearButtonEnabled(true);
    d->searchButton       = new QPushButton(i18nc("Start the search", "Search"), topHBox);

    DHBox* const actionHBox = new DHBox(this);
    d->mainVBox->addWidget(actionHBox);

    d->actionClearResultsList = new QAction(this);
    d->actionClearResultsList->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-clear")));
    d->actionClearResultsList->setToolTip(i18n("Clear the search results."));
    QToolButton* const tbClearResultsList = new QToolButton(actionHBox);
    tbClearResultsList->setDefaultAction(d->actionClearResultsList);

    d->actionKeepOldResults = new QAction(this);
    d->actionKeepOldResults->setIcon(QIcon::fromTheme(QString::fromLatin1("flag")));
    d->actionKeepOldResults->setCheckable(true);
    d->actionKeepOldResults->setChecked(false);
    d->actionKeepOldResults->setToolTip(i18n("Keep the results of old searches when doing a new search."));
    QToolButton* const tbKeepOldResults = new QToolButton(actionHBox);
    tbKeepOldResults->setDefaultAction(d->actionKeepOldResults);

    d->actionToggleAllResultsVisibility = new QAction(this);
    d->actionToggleAllResultsVisibility->setCheckable(true);
    d->actionToggleAllResultsVisibility->setChecked(true);
    d->actionToggleAllResultsVisibility->setToolTip(i18n("Toggle the visibility of the search results on the map."));
    QToolButton* const tbToggleAllVisibility = new QToolButton(actionHBox);
    tbToggleAllVisibility->setDefaultAction(d->actionToggleAllResultsVisibility);

    d->actionCopyCoordinates = new QAction(i18n("Copy coordinates"), this);
    d->actionCopyCoordinates->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-copy")));

    d->actionMoveImagesToThisResult = new QAction(i18n("Move selected images to this position"), this);

    d->actionRemovedSelectedSearchResultsFromList = new QAction(i18n("Remove from results list"), this);
    d->actionRemovedSelectedSearchResultsFromList->setIcon(QIcon::fromTheme(QString::fromLatin1("list-remove")));

    d->backendSelectionBox                            = new QComboBox(actionHBox);
    d->backendSelectionBox->setToolTip(i18n("Select which service you would like to use."));
    const QList<QPair<QString, QString> > backendList = d->searchBackend->getBackends();

    for (int i=0; i<backendList.count(); ++i)
    {
        d->backendSelectionBox->addItem(backendList.at(i).first, backendList.at(i).second);
    }

    // add stretch after the controls:
    QHBoxLayout* const hBoxLayout = reinterpret_cast<QHBoxLayout*>(actionHBox->layout());

    if (hBoxLayout)
    {
        hBoxLayout->addStretch();
    }

    d->treeView = new QTreeView(this);
    d->mainVBox->addWidget(d->treeView);
    d->treeView->setRootIsDecorated(false);
    d->treeView->setModel(d->searchResultsModel);
    d->treeView->setSelectionModel(d->searchResultsSelectionModel);
    d->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    d->actionBookmark = new QAction(i18n("Bookmarks"), this);
    d->actionBookmark->setMenu(d->gpsBookmarkOwner->getMenu());

    connect(d->actionMoveImagesToThisResult, SIGNAL(triggered(bool)),
            this, SLOT(slotMoveSelectedImagesToThisResult()));

    connect(d->searchButton, SIGNAL(clicked()),
            this, SLOT(slotTriggerSearch()));

    connect(d->searchBackend, SIGNAL(signalSearchCompleted()),
            this, SLOT(slotSearchCompleted()));

    connect(d->searchTermLineEdit, SIGNAL(returnPressed()),
            this, SLOT(slotTriggerSearch()));

    connect(d->searchTermLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateActionAvailability()));

    connect(d->searchResultsSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotCurrentlySelectedResultChanged(QModelIndex,QModelIndex)));

    connect(d->actionClearResultsList, SIGNAL(triggered(bool)),
            this, SLOT(slotClearSearchResults()));

    connect(d->actionToggleAllResultsVisibility, SIGNAL(triggered(bool)),
            this, SLOT(slotVisibilityChanged(bool)));

    connect(d->actionCopyCoordinates, SIGNAL(triggered(bool)),
            this, SLOT(slotCopyCoordinates()));

    connect(d->searchResultModelHelper, SIGNAL(signalUndoCommand(GPSUndoCommand*)),
            this, SIGNAL(signalUndoCommand(GPSUndoCommand*)));

    connect(d->actionRemovedSelectedSearchResultsFromList, SIGNAL(triggered(bool)),
            this, SLOT(slotRemoveSelectedFromResultsList()));

    d->treeView->installEventFilter(this);

    slotUpdateActionAvailability();
}

SearchWidget::~SearchWidget()
{
    delete d;
}

void SearchWidget::slotSearchCompleted()
{
    d->searchInProgress       = false;
    const QString errorString = d->searchBackend->getErrorMessage();

    if (!errorString.isEmpty())
    {
        QMessageBox::critical(this, i18n("Search failed"), i18n("Your search failed:\n%1", errorString));
        slotUpdateActionAvailability();
        return;
    }

    const SearchBackend::SearchResult::List searchResults = d->searchBackend->getResults();
    d->searchResultsModel->addResults(searchResults);

    slotUpdateActionAvailability();
}

void SearchWidget::slotTriggerSearch()
{
    // this is necessary since this slot is also connected to QLineEdit::returnPressed
    if (d->searchTermLineEdit->text().isEmpty() || d->searchInProgress)
    {
        return;
    }

    if (!d->actionKeepOldResults->isChecked())
    {
        slotClearSearchResults();
    }

    d->searchInProgress = true;

    const QString searchBackendName = d->backendSelectionBox->itemData(d->backendSelectionBox->currentIndex()).toString();
    d->searchBackend->search(searchBackendName, d->searchTermLineEdit->text());

    slotUpdateActionAvailability();
}

GeoModelHelper* SearchWidget::getModelHelper() const
{
    return d->searchResultModelHelper;
}

void SearchWidget::slotCurrentlySelectedResultChanged(const QModelIndex& current,
                                                      const QModelIndex& previous)
{
    Q_UNUSED(previous)

    if (!current.isValid())
    {
        return;
    }

    const SearchResultModel::SearchResultItem currentItem = d->searchResultsModel->resultItem(current);

    if (d->mapWidget)
    {
        d->mapWidget->setCenter(currentItem.result.coordinates);
    }
}

void SearchWidget::slotClearSearchResults()
{
    d->searchResultsModel->clearResults();

    slotUpdateActionAvailability();
}

void SearchWidget::slotVisibilityChanged(bool state)
{
    d->searchResultModelHelper->setVisibility(state);
    slotUpdateActionAvailability();
}

void SearchWidget::slotUpdateActionAvailability()
{
    const int nSelectedResults       = QItemSelectionModel_selectedRowsCount(d->searchResultsSelectionModel);
    const bool haveOneSelectedResult = nSelectedResults == 1;
    const bool haveSelectedImages    = !d->gosImageSelectionModel->selectedRows().isEmpty();

    d->actionCopyCoordinates->setEnabled(haveOneSelectedResult);
    d->actionMoveImagesToThisResult->setEnabled(haveOneSelectedResult && haveSelectedImages);
    d->actionRemovedSelectedSearchResultsFromList->setEnabled(nSelectedResults>=1);

    const bool haveSearchText        = !d->searchTermLineEdit->text().isEmpty();

    d->searchButton->setEnabled(haveSearchText&&!d->searchInProgress);
    d->actionClearResultsList->setEnabled(d->searchResultsModel->rowCount()>0);
    d->actionToggleAllResultsVisibility->setIcon(d->actionToggleAllResultsVisibility->isChecked() ? d->actionToggleAllResultsVisibilityIconChecked
                                                                                                  : d->actionToggleAllResultsVisibilityIconUnchecked);
}

bool SearchWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->treeView)
    {
        // we are only interested in context-menu events

        if (event->type() == QEvent::ContextMenu)
        {
            if (d->searchResultsSelectionModel->hasSelection())
            {
                const QModelIndex currentIndex                         = d->searchResultsSelectionModel->currentIndex();
                const SearchResultModel::SearchResultItem searchResult = d->searchResultsModel->resultItem(currentIndex);
                d->gpsBookmarkOwner->setPositionAndTitle(searchResult.result.coordinates, searchResult.result.name);
            }

            slotUpdateActionAvailability();

            // construct the context-menu:
            QMenu* const menu = new QMenu(d->treeView);
            menu->addAction(d->actionCopyCoordinates);
            menu->addAction(d->actionMoveImagesToThisResult);
            menu->addAction(d->actionRemovedSelectedSearchResultsFromList);
//          menu->addAction(d->actionBookmark);
            d->gpsBookmarkOwner->changeAddBookmark(true);

            QContextMenuEvent* const e = static_cast<QContextMenuEvent*>(event);
            menu->exec(e->globalPos());
            delete menu;
        }
    }

    return QObject::eventFilter(watched, event);
}

void SearchWidget::slotCopyCoordinates()
{
    const QModelIndex currentIndex                        = d->searchResultsSelectionModel->currentIndex();
    const SearchResultModel::SearchResultItem currentItem = d->searchResultsModel->resultItem(currentIndex);

    coordinatesToClipboard(currentItem.result.coordinates, QUrl(), currentItem.result.name);
}

void SearchWidget::saveSettingsToGroup(KConfigGroup* const group)
{
    group->writeEntry("Keep old results", d->actionKeepOldResults->isChecked());
    group->writeEntry("Search backend",   d->backendSelectionBox->itemData(d->backendSelectionBox->currentIndex()).toString());

    slotUpdateActionAvailability();
}

void SearchWidget::readSettingsFromGroup(const KConfigGroup* const group)
{
    d->actionKeepOldResults->setChecked(group->readEntry("Keep old results", false));
    const QString backendId = group->readEntry("Search backend", "osm");

    for (int i = 0; i < d->backendSelectionBox->count(); ++i)
    {
        if (d->backendSelectionBox->itemData(i).toString()==backendId)
        {
            d->backendSelectionBox->setCurrentIndex(i);
            break;
        }
    }
}

void SearchWidget::slotMoveSelectedImagesToThisResult()
{
    const QModelIndex currentIndex                        = d->searchResultsSelectionModel->currentIndex();
    const SearchResultModel::SearchResultItem currentItem = d->searchResultsModel->resultItem(currentIndex);
    const GeoCoordinates& targetCoordinates      = currentItem.result.coordinates;
    const QModelIndexList selectedImageIndices            = d->gosImageSelectionModel->selectedRows();

    if (selectedImageIndices.isEmpty())
    {
        return;
    }

    GPSUndoCommand* const undoCommand = new GPSUndoCommand();

    for (int i = 0; i < selectedImageIndices.count(); ++i)
    {
        const QPersistentModelIndex itemIndex = selectedImageIndices.at(i);
        GPSImageItem* const item             = d->gpsImageModel->itemFromIndex(itemIndex);

        GPSUndoCommand::UndoInfo undoInfo(itemIndex);
        undoInfo.readOldDataFromItem(item);

        GPSDataContainer newData;
        newData.setCoordinates(targetCoordinates);
        item->setGPSData(newData);

        undoInfo.readNewDataFromItem(item);

        undoCommand->addUndoInfo(undoInfo);
    }

    undoCommand->setText(i18np("1 image moved to '%2'",
                               "%1 images moved to '%2'", selectedImageIndices.count(), currentItem.result.name));

    emit(signalUndoCommand(undoCommand));
}

void SearchWidget::setPrimaryMapWidget(MapWidget* const mapWidget)
{
    d->mapWidget = mapWidget;
}

void SearchWidget::slotRemoveSelectedFromResultsList()
{
    const QItemSelection selectedRows = d->searchResultsSelectionModel->selection();

    if (selectedRows.isEmpty())
    {
        return;
    }

    d->searchResultsModel->removeRowsBySelection(selectedRows);

    slotUpdateActionAvailability();
}

} // namespace Digikam
