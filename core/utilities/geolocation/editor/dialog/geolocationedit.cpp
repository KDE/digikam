/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-05-16
 * Description : A tool to edit geolocation
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2010      by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2014      by Justus Schwartz <justus at gmx dot li>
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

#include "geolocationedit.h"

// Qt includes

#include <QtConcurrentMap>
#include <QButtonGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QFuture>
#include <QFutureWatcher>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QPointer>
#include <QRadioButton>
#include <QSplitter>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QMenu>
#include <QUndoView>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_config.h"
#include "mapwidget.h"
#include "itemmarkertiler.h"
#include "trackmanager.h"
#include "gpscommon.h"
#include "gpsimagemodel.h"
#include "mapdragdrophandler.h"
#include "gpsimagelist.h"
#include "gpsimagelistdragdrophandler.h"
#include "gpsimagelistcontextmenu.h"
#include "gpscorrelatorwidget.h"
#include "digikam_debug.h"
#include "dmessagebox.h"
#include "gpsundocommand.h"
#include "rgwidget.h"
#include "kmlwidget.h"
#include "statusprogressbar.h"
#include "searchwidget.h"
#include "backend-rg.h"
#include "gpsimagedetails.h"
#include "gpsgeoifacemodelhelper.h"
#include "dxmlguiwindow.h"
#include "gpsbookmarkowner.h"
#include "gpsbookmarkmodelhelper.h"
#include "dinfointerface.h"

#ifdef GPSSYNC_MODELTEST
#   include <modeltest.h>
#endif

namespace Digikam
{

struct SaveChangedImagesHelper
{
public:

    SaveChangedImagesHelper(GPSImageModel* const model)
        : imageModel(model)
    {
    }

    QPair<QUrl, QString> operator()(const QPersistentModelIndex& itemIndex)
    {
        GPSImageItem* const item = imageModel->itemFromIndex(itemIndex);

        if (!item)
            return QPair<QUrl, QString>(QUrl(), QString());

        return QPair<QUrl, QString>(item->url(), item->saveChanges());
    }

public:

    typedef QPair<QUrl, QString> result_type;
    GPSImageModel* const         imageModel;
};

// ---------------------------------------------------------------------------------

struct LoadFileMetadataHelper
{
public:

    LoadFileMetadataHelper(GPSImageModel* const model)
        : imageModel(model)
    {
    }

    QPair<QUrl, QString> operator()(const QPersistentModelIndex& itemIndex)
    {
        GPSImageItem* const item = imageModel->itemFromIndex(itemIndex);

        if (!item)
            return QPair<QUrl, QString>(QUrl(), QString());

        item->loadImageData();

        return QPair<QUrl, QString>(item->url(), QString());
    }

public:

    typedef QPair<QUrl, QString> result_type;
    GPSImageModel* const         imageModel;
};

// ---------------------------------------------------------------------------------

class GeolocationEdit::Private
{
public:

    Private()
    {
        imageModel               = 0;
        selectionModel           = 0;
        uiEnabled                = true;
        listViewContextMenu      = 0;
        trackManager             = 0;
        fileIOFutureWatcher      = 0;
        fileIOCountDone          = 0;
        fileIOCountTotal         = 0;
        fileIOCloseAfterSaving   = false;
        buttonBox                = 0;
        VSplitter                = 0;
        HSplitter                = 0;
        treeView                 = 0;
        stackedWidget            = 0;
        tabBar                   = 0;
        splitterSize             = 0;
        undoStack                = 0;
        undoView                 = 0;
        progressBar              = 0;
        progressCancelButton     = 0;
        progressCancelObject     = 0;
        detailsWidget            = 0;
        correlatorWidget         = 0;
        rgWidget                 = 0;
        searchWidget             = 0;
        kmlWidget                = 0;
        mapSplitter              = 0;
        mapWidget                = 0;
        mapWidget2               = 0;
        mapDragDropHandler       = 0;
        mapModelHelper           = 0;
        geoifaceMarkerModel      = 0;
        sortActionOldestFirst    = 0;
        sortActionYoungestFirst  = 0;
        sortMenu                 = 0;
        mapLayout                = MapLayoutOne;
        cbMapLayout              = 0;
        bookmarkOwner            = 0;
        actionBookmarkVisibility = 0;
        iface                    = 0;
    }

    // General things
    GPSImageModel*                           imageModel;
    QItemSelectionModel*                     selectionModel;
    bool                                     uiEnabled;
    GPSImageListContextMenu*                 listViewContextMenu;
    TrackManager*                            trackManager;

    // Loading and saving
    QFuture<QPair<QUrl,QString> >            fileIOFuture;
    QFutureWatcher<QPair<QUrl,QString> >*    fileIOFutureWatcher;
    int                                      fileIOCountDone;
    int                                      fileIOCountTotal;
    bool                                     fileIOCloseAfterSaving;

    // UI
    QDialogButtonBox*                        buttonBox;
    QSplitter*                               VSplitter;
    QSplitter*                               HSplitter;
    GPSImageList*                            treeView;
    QStackedWidget*                          stackedWidget;
    QTabBar*                                 tabBar;
    int                                      splitterSize;
    QUndoStack*                              undoStack;
    QUndoView*                               undoView;

    // UI: progress
    StatusProgressBar*                       progressBar;
    QPushButton*                             progressCancelButton;
    QObject*                                 progressCancelObject;
    QString                                  progressCancelSlot;

    // UI: tab widgets
    GPSImageDetails*                         detailsWidget;
    GPSCorrelatorWidget*                     correlatorWidget;
    RGWidget*                                rgWidget;
    SearchWidget*                            searchWidget;
    KmlWidget*                               kmlWidget;

    // map: UI
    MapLayout                                mapLayout;
    QSplitter*                               mapSplitter;
    MapWidget*                               mapWidget;
    MapWidget*                               mapWidget2;

    // map: helpers
    MapDragDropHandler*                      mapDragDropHandler;
    GPSGeoIfaceModelHelper*                  mapModelHelper;
    ItemMarkerTiler*                         geoifaceMarkerModel;

    // map: actions
    QAction*                                 sortActionOldestFirst;
    QAction*                                 sortActionYoungestFirst;
    QMenu*                                   sortMenu;
    QComboBox*                               cbMapLayout;

    GPSBookmarkOwner*                        bookmarkOwner;
    QAction*                                 actionBookmarkVisibility;

    DInfoInterface*                          iface;
};

GeolocationEdit::GeolocationEdit(QAbstractItemModel* const externTagModel,
                                 DInfoInterface* const iface,
                                 QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowTitle(i18n("Geolocation Editor"));
    setMinimumSize(300, 400);

    d->iface          = iface;
    d->imageModel     = new GPSImageModel(this);
    d->selectionModel = new QItemSelectionModel(d->imageModel);
    d->trackManager   = new TrackManager(this);

#ifdef GPSSYNC_MODELTEST
    new ModelTest(d->imageModel, this);
#endif

    d->bookmarkOwner = new GPSBookmarkOwner(d->imageModel, this);
    d->undoStack     = new QUndoStack(this);
    d->stackedWidget = new QStackedWidget();
    d->searchWidget  = new SearchWidget(d->bookmarkOwner,
                                        d->imageModel,
                                        d->selectionModel,
                                        d->stackedWidget);

    GPSImageItem::setHeaderData(d->imageModel);
    d->mapModelHelper      = new GPSGeoIfaceModelHelper(d->imageModel, d->selectionModel, this);
    d->mapModelHelper->addUngroupedModelHelper(d->bookmarkOwner->bookmarkModelHelper());

    d->mapModelHelper->addUngroupedModelHelper(d->searchWidget->getModelHelper());
    d->mapDragDropHandler  = new MapDragDropHandler(d->imageModel, d->mapModelHelper);
    d->geoifaceMarkerModel = new ItemMarkerTiler(d->mapModelHelper, this);

    d->actionBookmarkVisibility = new QAction(this);
    d->actionBookmarkVisibility->setIcon(QIcon::fromTheme(QLatin1String("bookmark-new")));
    d->actionBookmarkVisibility->setToolTip(i18n("Display bookmarked positions on the map."));
    d->actionBookmarkVisibility->setCheckable(true);

    connect(d->actionBookmarkVisibility, SIGNAL(changed()),
            this, SLOT(slotBookmarkVisibilityToggled()));

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    DHBox* const hboxMain = new DHBox(this);
    mainLayout->addWidget(hboxMain, 10);

    d->HSplitter          = new QSplitter(Qt::Horizontal, hboxMain);
    d->HSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ------------------------------------------------------------------------------------------------

    DHBox* const hbox            = new DHBox(this);
    QLabel* const labelMapLayout = new QLabel(i18n("Layout:"), hbox);
    d->cbMapLayout               = new QComboBox(hbox);
    d->cbMapLayout->addItem(i18n("One map"),               QVariant::fromValue(MapLayoutOne));
    d->cbMapLayout->addItem(i18n("Two maps - horizontal"), QVariant::fromValue(MapLayoutHorizontal));
    d->cbMapLayout->addItem(i18n("Two maps - vertical"),   QVariant::fromValue(MapLayoutVertical));
    labelMapLayout->setBuddy(d->cbMapLayout);

    d->progressBar          = new StatusProgressBar(hbox);
    d->progressBar->setVisible(false);
    d->progressBar->setProgressBarMode(StatusProgressBar::ProgressBarMode);
    d->progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    d->progressCancelButton = new QPushButton(hbox);
    d->progressCancelButton->setVisible(false);
    d->progressCancelButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    d->progressCancelButton->setIcon(QIcon::fromTheme(QLatin1String("dialog-cancel")));

    connect(d->progressCancelButton, SIGNAL(clicked()),
            this, SLOT(slotProgressCancelButtonClicked()));

    d->buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Close, hbox);

    connect(d->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &GeolocationEdit::slotApplyClicked);

    connect(d->buttonBox->button(QDialogButtonBox::Close), &QPushButton::clicked,
            this, &GeolocationEdit::close);

    mainLayout->addWidget(hbox, 0);

    // ------------------------------------------------------------------------------------------------

    d->VSplitter = new QSplitter(Qt::Vertical, d->HSplitter);
    d->HSplitter->addWidget(d->VSplitter);
    d->HSplitter->setStretchFactor(0, 10);

    d->sortMenu = new QMenu(this);
    d->sortMenu->setTitle(i18n("Sorting"));
    QActionGroup* const sortOrderExclusive = new QActionGroup(d->sortMenu);
    sortOrderExclusive->setExclusive(true);

    connect(sortOrderExclusive, SIGNAL(triggered(QAction*)),
            this, SLOT(slotSortOptionTriggered(QAction*)));

    d->sortActionOldestFirst = new QAction(i18n("Show oldest first"), sortOrderExclusive);
    d->sortActionOldestFirst->setCheckable(true);
    d->sortMenu->addAction(d->sortActionOldestFirst);

    d->sortActionYoungestFirst = new QAction(i18n("Show youngest first"), sortOrderExclusive);
    d->sortMenu->addAction(d->sortActionYoungestFirst);
    d->sortActionYoungestFirst->setCheckable(true);

    QWidget* mapVBox = 0;
    d->mapWidget     = makeMapWidget(&mapVBox);
    d->searchWidget->setPrimaryMapWidget(d->mapWidget);
    d->mapSplitter   = new QSplitter(this);
    d->mapSplitter->addWidget(mapVBox);
    d->VSplitter->addWidget(d->mapSplitter);

    d->treeView      = new GPSImageList(this);
    d->treeView->setModelAndSelectionModel(d->imageModel, d->selectionModel);
    d->treeView->setDragDropHandler(new GPSImageListDragDropHandler(this));
    d->treeView->setDragEnabled(true);
    // TODO: save and restore the state of the header
    // TODO: add a context menu to the header to select which columns should be visible
    // TODO: add sorting by column
    d->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d->treeView->setSortingEnabled(true);
    d->VSplitter->addWidget(d->treeView);

    d->listViewContextMenu  = new GPSImageListContextMenu(d->treeView, d->bookmarkOwner);
    d->HSplitter->addWidget(d->stackedWidget);
    d->HSplitter->setCollapsible(1, true);
    d->splitterSize         = 0;

    DVBox* const vboxTabBar = new DVBox(hboxMain);
    vboxTabBar->layout()->setContentsMargins(QMargins());
    vboxTabBar->layout()->setSpacing(0);

    d->tabBar = new QTabBar(vboxTabBar);
    d->tabBar->setShape(QTabBar::RoundedEast);

    dynamic_cast<QVBoxLayout*>(vboxTabBar->layout())->addStretch(200);

    d->tabBar->addTab(i18n("Details"));
    d->tabBar->addTab(i18n("GPS Correlator"));
    d->tabBar->addTab(i18n("Undo/Redo"));
    d->tabBar->addTab(i18n("Reverse Geocoding"));
    d->tabBar->addTab(i18n("Search"));
    d->tabBar->addTab(i18n("KML Export"));

    d->tabBar->installEventFilter(this);

    d->detailsWidget    = new GPSImageDetails(d->stackedWidget, d->imageModel);
    d->stackedWidget->addWidget(d->detailsWidget);

    d->correlatorWidget = new GPSCorrelatorWidget(d->stackedWidget, d->imageModel, d->trackManager);
    d->stackedWidget->addWidget(d->correlatorWidget);

    d->undoView         = new QUndoView(d->undoStack, d->stackedWidget);
    d->stackedWidget->addWidget(d->undoView);

    d->rgWidget         = new RGWidget(d->imageModel, d->selectionModel, externTagModel, d->stackedWidget);
    d->stackedWidget->addWidget(d->rgWidget);

    d->stackedWidget->addWidget(d->searchWidget);

    d->kmlWidget        = new KmlWidget(this, d->imageModel, d->iface);
    d->stackedWidget->addWidget(d->kmlWidget);

    // ---------------------------------------------------------------

    connect(d->treeView, SIGNAL(signalImageActivated(QModelIndex)),
            this, SLOT(slotImageActivated(QModelIndex)));

    connect(d->correlatorWidget, SIGNAL(signalSetUIEnabled(bool)),
            this, SLOT(slotSetUIEnabled(bool)));

    connect(d->correlatorWidget, SIGNAL(signalSetUIEnabled(bool,QObject*const,QString)),
            this, SLOT(slotSetUIEnabled(bool,QObject*const,QString)));

    connect(d->correlatorWidget, SIGNAL(signalProgressSetup(int,QString)),
            this, SLOT(slotProgressSetup(int,QString)));

    connect(d->correlatorWidget, SIGNAL(signalProgressChanged(int)),
            this, SLOT(slotProgressChanged(int)));

    connect(d->correlatorWidget, SIGNAL(signalUndoCommand(GPSUndoCommand*)),
            this, SLOT(slotGPSUndoCommand(GPSUndoCommand*)));

    connect(d->mapModelHelper, SIGNAL(signalUndoCommand(GPSUndoCommand*)),
            this, SLOT(slotGPSUndoCommand(GPSUndoCommand*)));

    connect(d->rgWidget, SIGNAL(signalSetUIEnabled(bool)),
            this, SLOT(slotSetUIEnabled(bool)));

    connect(d->rgWidget, SIGNAL(signalSetUIEnabled(bool,QObject*const,QString)),
            this, SLOT(slotSetUIEnabled(bool,QObject*const,QString)));

    connect(d->rgWidget, SIGNAL(signalProgressSetup(int,QString)),
            this, SLOT(slotProgressSetup(int,QString)));

    connect(d->rgWidget, SIGNAL(signalProgressChanged(int)),
            this, SLOT(slotProgressChanged(int)));

    connect(d->rgWidget, SIGNAL(signalUndoCommand(GPSUndoCommand*)),
            this, SLOT(slotGPSUndoCommand(GPSUndoCommand*)));

    connect(d->searchWidget, SIGNAL(signalUndoCommand(GPSUndoCommand*)),
            this, SLOT(slotGPSUndoCommand(GPSUndoCommand*)));

    connect(d->listViewContextMenu, SIGNAL(signalSetUIEnabled(bool)),
            this, SLOT(slotSetUIEnabled(bool)));

    connect(d->listViewContextMenu, SIGNAL(signalSetUIEnabled(bool,QObject*const,QString)),
            this, SLOT(slotSetUIEnabled(bool,QObject*const,QString)));

    connect(d->listViewContextMenu, SIGNAL(signalProgressSetup(int,QString)),
            this, SLOT(slotProgressSetup(int,QString)));

    connect(d->listViewContextMenu, SIGNAL(signalProgressChanged(int)),
            this, SLOT(slotProgressChanged(int)));

    connect(d->listViewContextMenu, SIGNAL(signalUndoCommand(GPSUndoCommand*)),
            this, SLOT(slotGPSUndoCommand(GPSUndoCommand*)));

    connect(d->tabBar, SIGNAL(currentChanged(int)),
            this, SLOT(slotCurrentTabChanged(int)));

    connect(d->bookmarkOwner->bookmarkModelHelper(),
            SIGNAL(signalUndoCommand(GPSUndoCommand*)),
            this, SLOT(slotGPSUndoCommand(GPSUndoCommand*)));

    connect(d->detailsWidget, SIGNAL(signalUndoCommand(GPSUndoCommand*)),
            this, SLOT(slotGPSUndoCommand(GPSUndoCommand*)));

    connect(d->cbMapLayout, SIGNAL(activated(int)),
            this, SLOT(slotLayoutChanged(int)));

    readSettings();

    d->mapWidget->setActive(true);

    // Workaround for a drag & drop problem of images to the map for Qt>=5.9.2
    show();
}

GeolocationEdit::~GeolocationEdit()
{
    delete d;
}

bool GeolocationEdit::eventFilter(QObject* const o, QEvent* const e)
{
    if ( ( o == d->tabBar ) && ( e->type() == QEvent::MouseButtonPress ) )
    {
        QMouseEvent const* m = static_cast<QMouseEvent*>(e);

        QPoint p (m->x(), m->y());
        const int var = d->tabBar->tabAt(p);

        if (var < 0)
        {
            return false;
        }

        QList<int> sizes = d->HSplitter->sizes();

        if (d->splitterSize == 0)
        {
            if (sizes.at(1) == 0)
            {
                sizes[1] = d->stackedWidget->widget(var)->minimumSizeHint().width();
            }
            else if (d->tabBar->currentIndex() == var)
            {
                d->splitterSize = sizes.at(1);
                sizes[1] = 0;
            }
        }
        else
        {
            sizes[1] = d->splitterSize;
            d->splitterSize = 0;
        }

        d->tabBar->setCurrentIndex(var);
        d->stackedWidget->setCurrentIndex(var);
        d->HSplitter->setSizes(sizes);
        d->detailsWidget->slotSetActive( (d->stackedWidget->currentWidget()==d->detailsWidget) && (d->splitterSize==0) );

        return true;
    }

    return QWidget::eventFilter(o, e);
}

void GeolocationEdit::slotCurrentTabChanged(int index)
{
    d->tabBar->setCurrentIndex(index);
    d->stackedWidget->setCurrentIndex(index);
    d->detailsWidget->slotSetActive(d->stackedWidget->currentWidget()==d->detailsWidget);
}

void GeolocationEdit::setCurrentTab(int index)
{
    d->tabBar->setCurrentIndex(index);
    d->stackedWidget->setCurrentIndex(index);
    QList<int> sizes = d->HSplitter->sizes();

    if (d->splitterSize >= 0)
    {
        sizes[1] = d->splitterSize;
        d->splitterSize = 0;
    }

    d->HSplitter->setSizes(sizes);
    d->detailsWidget->slotSetActive((d->stackedWidget->currentWidget() == d->detailsWidget) &&
                                    (d->splitterSize == 0));
}

void GeolocationEdit::setImages(const QList<QUrl>& images)
{
    QList<GPSImageItem*> items;

    foreach(const QUrl& u, images)
    {
        items << new GPSImageItem(u);
    }

    setItems(items);
}

void GeolocationEdit::setItems(const QList<GPSImageItem*>& items)
{
    foreach(GPSImageItem* const newItem, items)
    {
        newItem->loadImageData();
        d->imageModel->addItem(newItem);
    }

    QList<QPersistentModelIndex> imagesToLoad;

    for (int i = 0; i < d->imageModel->rowCount(); ++i)
    {
        imagesToLoad << d->imageModel->index(i, 0);
    }

    slotSetUIEnabled(false);
    slotProgressSetup(imagesToLoad.count(), i18n("Loading metadata -"));

    // initiate the saving
    d->fileIOCountDone     = 0;
    d->fileIOCountTotal    = imagesToLoad.count();
    d->fileIOFutureWatcher = new QFutureWatcher<QPair<QUrl, QString> >(this);

    connect(d->fileIOFutureWatcher, SIGNAL(resultsReadyAt(int,int)),
            this, SLOT(slotFileMetadataLoaded(int,int)));

    d->fileIOFuture = QtConcurrent::mapped(imagesToLoad, LoadFileMetadataHelper(d->imageModel));
    d->fileIOFutureWatcher->setFuture(d->fileIOFuture);
}

void GeolocationEdit::slotFileMetadataLoaded(int beginIndex, int endIndex)
{
    qCDebug(DIGIKAM_GENERAL_LOG)<<beginIndex<<endIndex;
    d->fileIOCountDone += (endIndex-beginIndex);
    slotProgressChanged(d->fileIOCountDone);

    if (d->fileIOCountDone == d->fileIOCountTotal)
    {
        slotSetUIEnabled(true);
    }
}

void GeolocationEdit::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group("Geolocation Edit Settings");

    // --------------------------

    // TODO: sanely determine a default backend
    const KConfigGroup groupMapWidget = KConfigGroup(&group, "Map Widget");
    d->mapWidget->readSettingsFromGroup(&groupMapWidget);

    const KConfigGroup groupCorrelatorWidget = KConfigGroup(&group, "Correlator Widget");
    d->correlatorWidget->readSettingsFromGroup(&groupCorrelatorWidget);

    const KConfigGroup groupTreeView = KConfigGroup(&group, "Tree View");
    d->treeView->readSettingsFromGroup(&groupTreeView);

    const KConfigGroup groupSearchWidget = KConfigGroup(&group, "Search Widget");
    d->searchWidget->readSettingsFromGroup(&groupSearchWidget);

    const KConfigGroup groupRGWidget = KConfigGroup(&group, "Reverse Geocoding Widget");
    d->rgWidget->readSettingsFromGroup(&groupRGWidget);

    const KConfigGroup groupDialog = KConfigGroup(&group, "Dialog");

    winId();
    windowHandle()->resize(800, 600);
    DXmlGuiWindow::restoreWindowSize(windowHandle(), groupDialog);
    resize(windowHandle()->size());

    // --------------------------

    setCurrentTab(group.readEntry("Current Tab", 0));
    const bool showOldestFirst = group.readEntry("Show oldest images first", false);

    if (showOldestFirst)
    {
        d->sortActionOldestFirst->setChecked(true);
        d->mapWidget->setSortKey(1);
    }
    else
    {
        d->sortActionYoungestFirst->setChecked(true);
        d->mapWidget->setSortKey(0);
    }

    d->actionBookmarkVisibility->setChecked(group.readEntry("Bookmarks visible", false));
    slotBookmarkVisibilityToggled();

    if (group.hasKey("SplitterState V1"))
    {
        const QByteArray splitterState = QByteArray::fromBase64(group.readEntry("SplitterState V1", QByteArray()));

        if (!splitterState.isEmpty())
        {
            d->VSplitter->restoreState(splitterState);
        }
    }

    if (group.hasKey("SplitterState H1"))
    {
        const QByteArray splitterState = QByteArray::fromBase64(group.readEntry("SplitterState H1", QByteArray()));

        if (!splitterState.isEmpty())
        {
            d->HSplitter->restoreState(splitterState);
        }
    }

    d->splitterSize = group.readEntry("Splitter H1 CollapsedSize", 0);

    // ----------------------------------

    d->mapLayout = MapLayout(group.readEntry("Map Layout", QVariant::fromValue(int(MapLayoutOne))).value<int>());
    d->cbMapLayout->setCurrentIndex(d->mapLayout);
    adjustMapLayout(false);

    if (d->mapWidget2)
    {
        const KConfigGroup groupMapWidget = KConfigGroup(&group, "Map Widget 2");
        d->mapWidget2->readSettingsFromGroup(&groupMapWidget);

        d->mapWidget2->setActive(true);
    }
}

void GeolocationEdit::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group("Geolocation Edit Settings");

    // --------------------------

    KConfigGroup groupMapWidget = KConfigGroup(&group, "Map Widget");
    d->mapWidget->saveSettingsToGroup(&groupMapWidget);

    if (d->mapWidget2)
    {
        KConfigGroup groupMapWidget = KConfigGroup(&group, "Map Widget 2");
        d->mapWidget2->saveSettingsToGroup(&groupMapWidget);
    }

    KConfigGroup groupCorrelatorWidget = KConfigGroup(&group, "Correlator Widget");
    d->correlatorWidget->saveSettingsToGroup(&groupCorrelatorWidget);

    KConfigGroup groupTreeView = KConfigGroup(&group, "Tree View");
    d->treeView->saveSettingsToGroup(&groupTreeView);

    KConfigGroup groupSearchWidget = KConfigGroup(&group, "Search Widget");
    d->searchWidget->saveSettingsToGroup(&groupSearchWidget);

    KConfigGroup groupRGWidget = KConfigGroup(&group, "Reverse Geocoding Widget");
    d->rgWidget->saveSettingsToGroup(&groupRGWidget);

    KConfigGroup groupDialog = KConfigGroup(&group, "Dialog");
    DXmlGuiWindow::saveWindowSize(windowHandle(), groupDialog);

    // --------------------------

    group.writeEntry("Current Tab",               d->tabBar->currentIndex());
    group.writeEntry("Show oldest images first",  d->sortActionOldestFirst->isChecked());
    group.writeEntry("SplitterState V1",          d->VSplitter->saveState().toBase64());
    group.writeEntry("SplitterState H1",          d->HSplitter->saveState().toBase64());
    group.writeEntry("Splitter H1 CollapsedSize", d->splitterSize);
    group.writeEntry("Map Layout",                QVariant::fromValue(int(d->mapLayout)));
    group.writeEntry("Bookmarks visible",         d->actionBookmarkVisibility->isChecked());

    config->sync();
}

void GeolocationEdit::closeEvent(QCloseEvent *e)
{
    if (!e) return;

    // is the UI locked?
    if (!d->uiEnabled)
    {
        // please wait until we are done ...
        return;
    }

    // are there any modified images?
    int dirtyImagesCount = 0;

    for (int i = 0; i < d->imageModel->rowCount(); ++i)
    {
        const QModelIndex itemIndex = d->imageModel->index(i, 0);
        GPSImageItem* const item    = d->imageModel->itemFromIndex(itemIndex);

        if (item->isDirty() || item->isTagListDirty())
        {
            dirtyImagesCount++;
        }
    }

    if (dirtyImagesCount > 0)
    {
        const QString message = i18np(
                    "You have 1 modified image.",
                    "You have %1 modified images.",
                    dirtyImagesCount
                );

        const int chosenAction = DMessageBox::showYesNo(QMessageBox::Warning,
                                                        this,
                                                        i18n("Unsaved changes"),
                                                        i18n("%1 Would you like to save the changes you made to them?", message)
                                                       );

        if (chosenAction == QMessageBox::No)
        {
            saveSettings();
            e->accept();
            return;
        }

        if (chosenAction == QMessageBox::Yes)
        {
            // the user wants to save his changes.
            // this will initiate the saving process and then close the dialog.
            saveChanges(true);
        }

        // do not close the dialog for now
        e->ignore();
        return;
    }

    saveSettings();
    e->accept();
}

void GeolocationEdit::slotImageActivated(const QModelIndex& index)
{
    d->detailsWidget->slotSetCurrentImage(index);

    if (!index.isValid())
        return;

    GPSImageItem* const item = d->imageModel->itemFromIndex(index);

    if (!item)
        return;

    const GeoCoordinates imageCoordinates = item->coordinates();

    if (imageCoordinates.hasCoordinates())
    {
        d->mapWidget->setCenter(imageCoordinates);
    }
}

void GeolocationEdit::slotSetUIEnabled(const bool enabledState,
                                       QObject* const cancelObject,
                                       const QString& cancelSlot)
{
    if (enabledState)
    {
        // hide the progress bar
        d->progressBar->setVisible(false);
        d->progressCancelButton->setVisible(false);
        d->progressBar->setProgressValue(d->progressBar->progressTotalSteps());
    }

    // TODO: disable the worldmapwidget and the images list (at least disable editing operations)
    d->progressCancelObject = cancelObject;
    d->progressCancelSlot   = cancelSlot;
    d->uiEnabled            = enabledState;
    d->buttonBox->setEnabled(enabledState);
    d->correlatorWidget->setUIEnabledExternal(enabledState);
    d->detailsWidget->setUIEnabledExternal(enabledState);
    d->rgWidget->setUIEnabled(enabledState);
    d->treeView->setEditEnabled(enabledState);
    d->listViewContextMenu->setEnabled(enabledState);
    d->mapWidget->setAllowModifications(enabledState);
}

void GeolocationEdit::slotSetUIEnabled(const bool enabledState)
{
    slotSetUIEnabled(enabledState, 0, QString());
}

void GeolocationEdit::saveChanges(const bool closeAfterwards)
{
    // TODO: actually save the changes
    // are there any modified images?
    QList<QPersistentModelIndex> dirtyImages;

    for (int i = 0; i < d->imageModel->rowCount(); ++i)
    {
        const QModelIndex itemIndex = d->imageModel->index(i, 0);
        GPSImageItem* const item    = d->imageModel->itemFromIndex(itemIndex);

        if (item->isDirty() || item->isTagListDirty())
        {
            dirtyImages << itemIndex;
        }
    }

    if (dirtyImages.isEmpty())
    {
        if (closeAfterwards)
        {
            close();
        }

        return;
    }

    // TODO: disable the UI and provide progress and cancel information
    slotSetUIEnabled(false);
    slotProgressSetup(dirtyImages.count(), i18n("Saving changes -"));

    // initiate the saving
    d->fileIOCountDone        = 0;
    d->fileIOCountTotal       = dirtyImages.count();
    d->fileIOCloseAfterSaving = closeAfterwards;
    d->fileIOFutureWatcher    = new QFutureWatcher<QPair<QUrl, QString> >(this);

    connect(d->fileIOFutureWatcher, SIGNAL(resultsReadyAt(int,int)),
            this, SLOT(slotFileChangesSaved(int,int)));

    d->fileIOFuture = QtConcurrent::mapped(dirtyImages, SaveChangedImagesHelper(d->imageModel));
    d->fileIOFutureWatcher->setFuture(d->fileIOFuture);
}

void GeolocationEdit::slotFileChangesSaved(int beginIndex, int endIndex)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << beginIndex << endIndex;

    d->fileIOCountDone += (endIndex-beginIndex);
    slotProgressChanged(d->fileIOCountDone);

    if (d->fileIOCountDone == d->fileIOCountTotal)
    {
        slotSetUIEnabled(true);

        // any errors?
        QList<QPair<QUrl, QString> > errorList;

        for (int i = 0; i < d->fileIOFuture.resultCount(); ++i)
        {
            if (!d->fileIOFuture.resultAt(i).second.isEmpty())
                errorList << d->fileIOFuture.resultAt(i);
        }

        if (!errorList.isEmpty())
        {
            QStringList errorStrings;

            for (int i = 0; i < errorList.count(); ++i)
            {
                // TODO: how to do kurl->qstring?
                errorStrings << QString::fromLatin1("%1: %2")
                    .arg(errorList.at(i).first.toLocalFile())
                    .arg(errorList.at(i).second);
            }

            DMessageBox::showInformationList(QMessageBox::Critical,
                                             this,
                                             i18n("Error"),
                                             i18n("Failed to save some information:"),
                                             errorStrings);
        }

        // done saving files
        if (d->fileIOCloseAfterSaving)
        {
            close();
        }
    }
}

void GeolocationEdit::slotApplyClicked()
{
    // save the changes, but do not close afterwards
    saveChanges(false);
}

void GeolocationEdit::slotProgressChanged(const int currentProgress)
{
    d->progressBar->setProgressValue(currentProgress);
}

void GeolocationEdit::slotProgressSetup(const int maxProgress, const QString& progressText)
{
    d->progressBar->setProgressText(progressText);
    d->progressBar->setProgressTotalSteps(maxProgress);
    d->progressBar->setProgressValue(0);
    d->progressBar->setNotify(true);
    d->progressBar->setNotificationTitle(i18n("Edit Geolocation"), QIcon::fromTheme(QLatin1String("globe")));
    d->progressBar->setVisible(true);
    d->progressCancelButton->setVisible(d->progressCancelObject != 0);
}

void GeolocationEdit::slotGPSUndoCommand(GPSUndoCommand* undoCommand)
{
    d->undoStack->push(undoCommand);
}

void GeolocationEdit::slotSortOptionTriggered(QAction* /*sortAction*/)
{
    int newSortKey = 0;

    if (d->sortActionOldestFirst->isChecked())
    {
        newSortKey |= 1;
    }

    d->mapWidget->setSortKey(newSortKey);
}

void GeolocationEdit::slotProgressCancelButtonClicked()
{
    if (d->progressCancelObject)
    {
        QTimer::singleShot(0, d->progressCancelObject, d->progressCancelSlot.toUtf8().constData());

        d->progressBar->setProgressValue(d->progressBar->progressTotalSteps());
    }
}

void GeolocationEdit::slotBookmarkVisibilityToggled()
{
    d->bookmarkOwner->bookmarkModelHelper()->setVisible(d->actionBookmarkVisibility->isChecked());
}

void GeolocationEdit::slotLayoutChanged(int lay)
{
    d->mapLayout = (MapLayout)lay;
    adjustMapLayout(true);
}

MapWidget* GeolocationEdit::makeMapWidget(QWidget** const pvbox)
{
    QWidget* const dummyWidget = new QWidget(this);
    QVBoxLayout* const vbox    = new QVBoxLayout(dummyWidget);
    MapWidget* const mapWidget = new MapWidget(dummyWidget);
    mapWidget->setAvailableMouseModes(MouseModePan|MouseModeZoomIntoGroup|MouseModeSelectThumbnail);
    mapWidget->setVisibleMouseModes(MouseModePan|MouseModeZoomIntoGroup|MouseModeSelectThumbnail);
    mapWidget->setMouseMode(MouseModeSelectThumbnail);
    mapWidget->setGroupedModel(d->geoifaceMarkerModel);
    mapWidget->setDragDropHandler(d->mapDragDropHandler);
    mapWidget->addUngroupedModel(d->bookmarkOwner->bookmarkModelHelper());
    mapWidget->addUngroupedModel(d->searchWidget->getModelHelper());
    mapWidget->setTrackManager(d->trackManager);
    mapWidget->setSortOptionsMenu(d->sortMenu);

    vbox->addWidget(mapWidget);
    vbox->addWidget(mapWidget->getControlWidget());

    QToolButton* const bookmarkVisibilityButton = new QToolButton(mapWidget);
    bookmarkVisibilityButton->setDefaultAction(d->actionBookmarkVisibility);
    mapWidget->addWidgetToControlWidget(bookmarkVisibilityButton);

    *pvbox = dummyWidget;

    return mapWidget;
}

void GeolocationEdit::adjustMapLayout(const bool syncSettings)
{
    if (d->mapLayout == MapLayoutOne)
    {
        if (d->mapSplitter->count()>1)
        {
            delete d->mapSplitter->widget(1);
            d->mapWidget2 = 0;
        }
    }
    else
    {
        if (d->mapSplitter->count()==1)
        {
            QWidget* mapHolder = 0;
            d->mapWidget2      = makeMapWidget(&mapHolder);
            d->mapSplitter->addWidget(mapHolder);

            if (syncSettings)
            {
                KSharedConfig::Ptr config = KSharedConfig::openConfig();
                KConfigGroup group        = config->group("Geolocation Edit Settings");
                const KConfigGroup groupMapWidget = KConfigGroup(&group, "Map Widget");
                d->mapWidget2->readSettingsFromGroup(&groupMapWidget);
                d->mapWidget2->setActive(true);
            }
        }

        if (d->mapLayout==MapLayoutHorizontal)
        {
            d->mapSplitter->setOrientation(Qt::Horizontal);
        }
        else
        {
            d->mapSplitter->setOrientation(Qt::Vertical);
        }
    }
}

} // namespace Digikam
