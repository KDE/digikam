/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2006-05-16
 * @brief  A plugin to synchronize pictures with a GPS device.
 *
 * @author Copyright (C) 2006-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2010, 2011, 2014 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gabriel Voicu
 *         <a href="mailto:ping dot gabi at gmail dot com">ping dot gabi at gmail dot com</a>
 * @author Copyright (C) 2014 by Justus Schwartz
 *         <a href="mailto:justus at gmx dot li">justus at gmx dot li</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "gpssyncdialog.h"

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
#include <QProgressBar>
#include <QDialogButtonBox>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kwindowconfig.h>

// Libkgeomap includes

#include <KGeoMap/MapWidget>
#include <KGeoMap/ItemMarkerTiler>
#include <KGeoMap/Tracks>

// Libkdcraw includes

#include <KDCRAW/RWidgetUtils>

// Local includes

#include "gpscommon.h"
#include "gpsimagemodel.h"
#include "gpsimageitem.h"
#include "mapdragdrophandler.h"
#include "gpsimagelist.h"
#include "gpsimagelistdragdrophandler.h"
#include "gpsimagelistcontextmenu.h"
#include "gpscorrelatorwidget.h"
#include "digikam_debug.h"
#include "gpsundocommand.h"
#include "rgwidget.h"
#include "gpsbookmarkowner.h"
#include "gpsbookmarkmodelhelper.h"
#include "searchwidget.h"
#include "backend-rg.h"
#include "gpsimagedetails.h"
#include "gpssynckgeomapmodelhelper.h"

#ifdef GPSSYNC_MODELTEST
#include <modeltest.h>
#endif /* GPSSYNC_MODELTEST */

using namespace KDcrawIface;

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

class GPSSyncDialog::Private
{
public:

    Private()
    {
        imageModel               = 0;
        selectionModel           = 0;
        uiEnabled                = true;
        bookmarkOwner            = 0;
        actionBookmarkVisibility = 0;
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
        mapSplitter              = 0;
        mapWidget                = 0;
        mapWidget2               = 0;
        mapDragDropHandler       = 0;
        mapModelHelper           = 0;
        kgeomapMarkerModel       = 0;
        sortActionOldestFirst    = 0;
        sortActionYoungestFirst  = 0;
        sortMenu                 = 0;
        mapLayout                = MapLayoutOne;
        cbMapLayout              = 0;
    }

    // General things
    GPSImageModel*                           imageModel;
    QItemSelectionModel*                     selectionModel;
    bool                                     uiEnabled;
    GPSBookmarkOwner*                        bookmarkOwner;
    QAction*                                 actionBookmarkVisibility;
    GPSImageListContextMenu*                 listViewContextMenu;
    KGeoMap::TrackManager*                   trackManager;

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
    QProgressBar*                            progressBar;
    QPushButton*                             progressCancelButton;
    QObject*                                 progressCancelObject;
    QString                                  progressCancelSlot;

    // UI: tab widgets
    GPSImageDetails*                         detailsWidget;
    GPSCorrelatorWidget*                     correlatorWidget;
    RGWidget*                                rgWidget;
    SearchWidget*                            searchWidget;

    // map: UI
    MapLayout                                mapLayout;
    QSplitter*                               mapSplitter;
    MapWidget*                               mapWidget;
    MapWidget*                               mapWidget2;

    // map: helpers
    MapDragDropHandler*                      mapDragDropHandler;
    GPSSyncKGeoMapModelHelper*               mapModelHelper;
    ItemMarkerTiler*                         kgeomapMarkerModel;

    // map: actions
    QAction*                                 sortActionOldestFirst;
    QAction*                                 sortActionYoungestFirst;
    QMenu*                                   sortMenu;
    QComboBox*                               cbMapLayout;
};

GPSSyncDialog::GPSSyncDialog(QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowTitle(i18n("Geolocation Editor"));
    setMinimumSize(300,400);

    d->imageModel     = new GPSImageModel(this);
    d->selectionModel = new QItemSelectionModel(d->imageModel);
    d->trackManager   = new KGeoMap::TrackManager(this);

#ifdef GPSSYNC_MODELTEST
    new ModelTest(d->imageModel, this);
#endif /* GPSSYNC_MODELTEST */

    d->undoStack     = new QUndoStack(this);
    d->bookmarkOwner = new GPSBookmarkOwner(d->imageModel, this);
    d->stackedWidget = new QStackedWidget();
    d->searchWidget  = new SearchWidget(d->bookmarkOwner, d->imageModel, d->selectionModel, d->stackedWidget);

    GPSImageItem::setHeaderData(d->imageModel);
    d->mapModelHelper     = new GPSSyncKGeoMapModelHelper(d->imageModel, d->selectionModel, this);
    d->mapModelHelper->addUngroupedModelHelper(d->bookmarkOwner->bookmarkModelHelper());
    d->mapModelHelper->addUngroupedModelHelper(d->searchWidget->getModelHelper());
    d->mapDragDropHandler = new MapDragDropHandler(d->imageModel, d->mapModelHelper);
    d->kgeomapMarkerModel = new ItemMarkerTiler(d->mapModelHelper, this);

    d->actionBookmarkVisibility = new QAction(this);
    d->actionBookmarkVisibility->setIcon(QIcon::fromTheme(QStringLiteral("user-trash")));
    d->actionBookmarkVisibility->setToolTip(i18n("Display bookmarked positions on the map."));
    d->actionBookmarkVisibility->setCheckable(true);

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    RHBox* const hboxMain = new RHBox(this);
    mainLayout->addWidget(hboxMain);

    d->HSplitter            = new QSplitter(Qt::Horizontal, hboxMain);
    d->HSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    RHBox* const hboxBottom = new RHBox(this);
    mainLayout->addWidget(hboxBottom);

    d->progressBar          = new QProgressBar(hboxBottom);
    d->progressBar->setVisible(false);
    d->progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    // we need a really large stretch factor here because the QDialogButtonBox also stretches a lot...
    dynamic_cast<QHBoxLayout*>(hboxBottom->layout())->setStretch(200, 0);

    d->progressCancelButton = new QPushButton(hboxBottom);
    d->progressCancelButton->setVisible(false);
    d->progressCancelButton->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    d->progressCancelButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-cancel")));

    connect(d->progressCancelButton, SIGNAL(clicked()),
            this, SLOT(slotProgressCancelButtonClicked()));

    // ------------------------------------------------------------------------------------------------

    RHBox* const hbox            = new RHBox(this);
    QLabel* const labelMapLayout = new QLabel(i18n("Layout:"), hbox);
    d->cbMapLayout               = new QComboBox(hbox);
    d->cbMapLayout->addItem(i18n("One map"),               QVariant::fromValue(MapLayoutOne));
    d->cbMapLayout->addItem(i18n("Two maps - horizontal"), QVariant::fromValue(MapLayoutHorizontal));
    d->cbMapLayout->addItem(i18n("Two maps - vertical"),   QVariant::fromValue(MapLayoutVertical));
    labelMapLayout->setBuddy(d->cbMapLayout);
    QWidget* const space         = new QWidget(hbox);
    hbox->setStretchFactor(space, 10);
   
    d->buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Close, hbox);
    d->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    connect(d->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &GPSSyncDialog::slotApplyClicked);

    connect(d->buttonBox->button(QDialogButtonBox::Close), &QPushButton::clicked,
            this, &GPSSyncDialog::close);

    mainLayout->addWidget(hbox);

    // ------------------------------------------------------------------------------------------------
    
    // TODO: the code below does not seem to have any effect, slotApplyClicked is still triggered
    //       when 'Enter' is pressed...
    // make sure the 'Apply' button is not triggered when enter is pressed,
    // because that causes problems with the search widget
    QAbstractButton* testButton = 0;

    Q_FOREACH(testButton, d->buttonBox->buttons())
    {
//         if (d->buttonBox->buttonRole(testButton)==QDialogButtonBox::AcceptRole)
        {
            QPushButton* const pushButton = dynamic_cast<QPushButton*>(testButton);
            qCDebug(DIGIKAM_GENERAL_LOG) << pushButton << pushButton->isDefault();

            if (pushButton)
            {
                pushButton->setDefault(false);
            }
        }
    }

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

    connect(d->actionBookmarkVisibility, SIGNAL(changed()),
            this, SLOT(slotBookmarkVisibilityToggled()));

    QWidget* mapVBox = 0;
    d->mapWidget           = makeMapWidget(&mapVBox);
    d->searchWidget->setPrimaryMapWidget(d->mapWidget);
    d->mapSplitter         = new QSplitter(this);
    d->mapSplitter->addWidget(mapVBox);
    d->VSplitter->addWidget(d->mapSplitter);

    d->treeView            = new GPSImageList(this);
    d->treeView->setModelAndSelectionModel(d->imageModel, d->selectionModel);
    d->treeView->setDragDropHandler(new GPSImageListDragDropHandler(this));
    d->treeView->setDragEnabled(true);
    // TODO: save and restore the state of the header
    // TODO: add a context menu to the header to select which columns should be visible
    // TODO: add sorting by column
    d->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d->treeView->setSortingEnabled(true);
    d->VSplitter->addWidget(d->treeView);

    d->listViewContextMenu = new GPSImageListContextMenu(d->treeView, d->bookmarkOwner);
    d->HSplitter->setCollapsible(1, true);
    d->HSplitter->addWidget(d->stackedWidget);
    d->splitterSize        = 0;

    KDcrawIface::RVBox* const vboxTabBar = new KDcrawIface::RVBox(hboxMain);
    vboxTabBar->layout()->setSpacing(0);
    vboxTabBar->layout()->setMargin(0);

    d->tabBar = new QTabBar(vboxTabBar);
    d->tabBar->setShape(QTabBar::RoundedEast);

    dynamic_cast<QVBoxLayout*>(vboxTabBar->layout())->addStretch(200);

    d->tabBar->addTab(i18n("Details"));
    d->tabBar->addTab(i18n("GPS Correlator"));
    d->tabBar->addTab(i18n("Undo/Redo"));
    d->tabBar->addTab(i18n("Reverse Geocoding"));
    d->tabBar->addTab(i18n("Search"));

    d->tabBar->installEventFilter(this);

    d->detailsWidget    = new GPSImageDetails(d->stackedWidget, d->imageModel);
    d->stackedWidget->addWidget(d->detailsWidget);

    d->correlatorWidget = new GPSCorrelatorWidget(d->stackedWidget, d->imageModel, d->trackManager);
    d->stackedWidget->addWidget(d->correlatorWidget);

    d->undoView         = new QUndoView(d->undoStack, d->stackedWidget);
    d->stackedWidget->addWidget(d->undoView);

    d->rgWidget         = new RGWidget(d->imageModel, d->selectionModel, d->stackedWidget);
    d->stackedWidget->addWidget(d->rgWidget);
    d->stackedWidget->addWidget(d->searchWidget);

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

    connect(d->bookmarkOwner->bookmarkModelHelper(), SIGNAL(signalUndoCommand(GPSUndoCommand*)),
            this, SLOT(slotGPSUndoCommand(GPSUndoCommand*)));

    connect(d->detailsWidget, SIGNAL(signalUndoCommand(GPSUndoCommand*)),
            this, SLOT(slotGPSUndoCommand(GPSUndoCommand*)));

    connect(d->cbMapLayout, SIGNAL(activated(int)),
            this, SLOT(slotLayoutChanged(int)));

    readSettings();

    d->mapWidget->setActive(true);
}

GPSSyncDialog::~GPSSyncDialog()
{
    delete d;
}

bool GPSSyncDialog::eventFilter(QObject* const o, QEvent* const e)
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

void GPSSyncDialog::slotCurrentTabChanged(int index)
{
    d->tabBar->setCurrentIndex(index);
    d->stackedWidget->setCurrentIndex(index);
    d->detailsWidget->slotSetActive(d->stackedWidget->currentWidget()==d->detailsWidget);
}

void GPSSyncDialog::setCurrentTab(int index)
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
    d->detailsWidget->slotSetActive( (d->stackedWidget->currentWidget()==d->detailsWidget) && (d->splitterSize==0) );
}

void GPSSyncDialog::setImages(const QList<QUrl>& images)
{
    for ( QList<QUrl>::ConstIterator it = images.begin(); it != images.end(); ++it )
    {
        GPSImageItem* const newItem = new GPSImageItem(*it);
        newItem->loadImageData();
        d->imageModel->addItem(newItem);
    }

    QList<QPersistentModelIndex> imagesToLoad;

    for (int i=0; i<d->imageModel->rowCount(); ++i)
    {
        imagesToLoad << d->imageModel->index(i, 0);
    }

    slotSetUIEnabled(false);
    slotProgressSetup(imagesToLoad.count(), i18n("Loading metadata - %p%"));

    // initiate the saving
    d->fileIOCountDone     = 0;
    d->fileIOCountTotal    = imagesToLoad.count();
    d->fileIOFutureWatcher = new QFutureWatcher<QPair<QUrl, QString> >(this);

    connect(d->fileIOFutureWatcher, SIGNAL(resultsReadyAt(int,int)),
            this, SLOT(slotFileMetadataLoaded(int,int)));

    d->fileIOFuture = QtConcurrent::mapped(imagesToLoad, LoadFileMetadataHelper(d->imageModel));
    d->fileIOFutureWatcher->setFuture(d->fileIOFuture);
}

void GPSSyncDialog::slotFileMetadataLoaded(int beginIndex, int endIndex)
{
    qCDebug(DIGIKAM_GENERAL_LOG)<<beginIndex<<endIndex;
    d->fileIOCountDone += (endIndex-beginIndex);
    slotProgressChanged(d->fileIOCountDone);

    if (d->fileIOCountDone == d->fileIOCountTotal)
    {
        slotSetUIEnabled(true);
    }
}

void GPSSyncDialog::readSettings()
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
    KWindowConfig::restoreWindowSize(windowHandle(), groupDialog);

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

void GPSSyncDialog::saveSettings()
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
    KWindowConfig::saveWindowSize(windowHandle(), groupDialog);

    // --------------------------

    group.writeEntry("Current Tab",               d->tabBar->currentIndex());
    group.writeEntry("Show oldest images first",  d->sortActionOldestFirst->isChecked());
    group.writeEntry("Bookmarks visible",         d->actionBookmarkVisibility->isChecked());
    group.writeEntry("SplitterState V1",          d->VSplitter->saveState().toBase64());
    group.writeEntry("SplitterState H1",          d->HSplitter->saveState().toBase64());
    group.writeEntry("Splitter H1 CollapsedSize", d->splitterSize);
    group.writeEntry("Map Layout",                QVariant::fromValue(int(d->mapLayout)));

    // --------------------------

    config->sync();
}

void GPSSyncDialog::closeEvent(QCloseEvent *e)
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

    for (int i=0; i < d->imageModel->rowCount(); ++i)
    {
        const QModelIndex itemIndex = d->imageModel->index(i, 0);
        GPSImageItem* const item   = d->imageModel->itemFromIndex(itemIndex);

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

        const int chosenAction = KMessageBox::warningYesNoCancel(this,
            i18n("%1 Would you like to save the changes you made to them?", message),
            i18n("Unsaved changes"),
            KGuiItem(i18n("Save changes")),
            KGuiItem(i18n("Close and discard changes"))
            );

        if (chosenAction == KMessageBox::No)
        {
            saveSettings();
            e->accept();
            return;
        }

        if (chosenAction == KMessageBox::Yes)
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

void GPSSyncDialog::slotImageActivated(const QModelIndex& index)
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

void GPSSyncDialog::slotSetUIEnabled(const bool enabledState, QObject* const cancelObject, const QString& cancelSlot)
{
    if (enabledState)
    {
        // hide the progress bar
        d->progressBar->setVisible(false);
        
        /* FIXME :use progress manager
        d->progressBar->progressCompleted();
        d->progressCancelButton->setVisible(false);
        */
    }

    // TODO: disable the worldmapwidget and the images list (at least disable editing operations)
    d->progressCancelObject = cancelObject;
    d->progressCancelSlot = cancelSlot;
    d->uiEnabled = enabledState;
    d->buttonBox->setEnabled(enabledState);
    d->correlatorWidget->setUIEnabledExternal(enabledState);
    d->detailsWidget->setUIEnabledExternal(enabledState);
    d->rgWidget->setUIEnabled(enabledState);
    d->treeView->setEditEnabled(enabledState);
    d->listViewContextMenu->setEnabled(enabledState);
    d->mapWidget->setAllowModifications(enabledState);
}

void GPSSyncDialog::slotSetUIEnabled(const bool enabledState)
{
    slotSetUIEnabled(enabledState, 0, QString());
}

void GPSSyncDialog::saveChanges(const bool closeAfterwards)
{
    // TODO: actually save the changes
    // are there any modified images?
    QList<QPersistentModelIndex> dirtyImages;

    for (int i=0; i<d->imageModel->rowCount(); ++i)
    {
        const QModelIndex itemIndex = d->imageModel->index(i, 0);
        GPSImageItem* const item   = d->imageModel->itemFromIndex(itemIndex);

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
    slotProgressSetup(dirtyImages.count(), i18n("Saving changes - %p%"));

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

void GPSSyncDialog::slotFileChangesSaved(int beginIndex, int endIndex)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << beginIndex << endIndex;

    d->fileIOCountDone += (endIndex-beginIndex);
    slotProgressChanged(d->fileIOCountDone);

    if (d->fileIOCountDone == d->fileIOCountTotal)
    {
        slotSetUIEnabled(true);

        // any errors?
        QList<QPair<QUrl, QString> > errorList;

        for (int i=0; i<d->fileIOFuture.resultCount(); ++i)
        {
            if (!d->fileIOFuture.resultAt(i).second.isEmpty())
                errorList << d->fileIOFuture.resultAt(i);
        }

        if (!errorList.isEmpty())
        {
            QStringList errorStrings;

            for (int i=0; i<errorList.count(); ++i)
            {
                // TODO: how to do kurl->qstring?
                errorStrings << QStringLiteral("%1: %2")
                    .arg(errorList.at(i).first.toLocalFile())
                    .arg(errorList.at(i).second);
            }

            KMessageBox::errorList(this, i18n("Failed to save some information:"), errorStrings, i18n("Error"));
        }

        // done saving files
        if (d->fileIOCloseAfterSaving)
        {
            close();
        }
    }
}

void GPSSyncDialog::slotApplyClicked()
{
    // save the changes, but do not close afterwards
    saveChanges(false);
}

void GPSSyncDialog::slotProgressChanged(const int currentProgress)
{
    d->progressBar->setValue(currentProgress);
}

void GPSSyncDialog::slotProgressSetup(const int maxProgress, const QString& progressText)
{
    d->progressBar->setFormat(progressText);
    d->progressBar->setMaximum(maxProgress);
    d->progressBar->setValue(0);
    d->progressBar->setVisible(true);
    /* FIXME :use progress manager    
    d->progressBar->progressScheduled(i18n("GPS sync"), true, true);
    d->progressBar->progressThumbnailChanged(QIcon::fromTheme(QStringLiteral("kipi")).pixmap(22, 22));
    */
    d->progressCancelButton->setVisible(d->progressCancelObject!=0);
}

void GPSSyncDialog::slotGPSUndoCommand(GPSUndoCommand* undoCommand)
{
    d->undoStack->push(undoCommand);
}

void GPSSyncDialog::slotSortOptionTriggered(QAction* /*sortAction*/)
{
    int newSortKey = 0;

    if (d->sortActionOldestFirst->isChecked())
    {
        newSortKey |= 1;
    }

    d->mapWidget->setSortKey(newSortKey);
}

void GPSSyncDialog::slotProgressCancelButtonClicked()
{
    if (d->progressCancelObject)
    {
        QTimer::singleShot(0, d->progressCancelObject, d->progressCancelSlot.toUtf8().constData());
        /* FIXME :use progress manager
        d->progressBar->progressCompleted();
        */
    }
}

void GPSSyncDialog::slotBookmarkVisibilityToggled()
{
    d->bookmarkOwner->bookmarkModelHelper()->setVisible(d->actionBookmarkVisibility->isChecked());
}

void GPSSyncDialog::slotLayoutChanged(int lay)
{
    d->mapLayout = (MapLayout)lay;
    adjustMapLayout(true);
}

MapWidget* GPSSyncDialog::makeMapWidget(QWidget** const pvbox)
{
    QWidget* const dummyWidget     = new QWidget(this);
    QVBoxLayout* const vbox        = new QVBoxLayout(dummyWidget);
    MapWidget* const mapWidget = new MapWidget(dummyWidget);
    mapWidget->setAvailableMouseModes(MouseModePan|MouseModeZoomIntoGroup|MouseModeSelectThumbnail);
    mapWidget->setVisibleMouseModes(MouseModePan|MouseModeZoomIntoGroup|MouseModeSelectThumbnail);
    mapWidget->setMouseMode(MouseModeSelectThumbnail);
    mapWidget->setGroupedModel(d->kgeomapMarkerModel);
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

void GPSSyncDialog::adjustMapLayout(const bool syncSettings)
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

}  // namespace Digikam
