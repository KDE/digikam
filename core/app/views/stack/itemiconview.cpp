/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : implementation of item icon view interface.
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2011-2013 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * Copyright (C) 2017      by Simon Frei <freisim93 at gmail dot com>
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

#include "itemiconview.h"

// Qt includes

#include <QTimer>
#include <QShortcut>
#include <QApplication>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "albumhistory.h"
#include "labelstreeview.h"
#include "albumpointer.h"
#include "coredbsearchxml.h"
#include "dbinfoiface.h"
#include "digikam_config.h"
#include "digikam_debug.h"
#include "digikam_globals.h"
#include "digikamapp.h"
#include "digikamitemview.h"
#include "dfileoperations.h"
#include "dmessagebox.h"
#include "dzoombar.h"
#include "dtrashitemmodel.h"
#include "facescansettings.h"
#include "facesdetector.h"
#include "fileactionmngr.h"
#include "fileactionprogress.h"
#include "filtersidebarwidget.h"
#include "filterstatusbar.h"
#include "itemalbummodel.h"
#include "itemdescedittab.h"
#include "itempreviewview.h"
#include "itempropertiessidebardb.h"
#include "itempropertiesversionstab.h"
#include "itemthumbnailbar.h"
#include "itemviewutilities.h"
#include "leftsidebarwidgets.h"
#include "loadingcacheinterface.h"
#include "metadatahub.h"
#include "metaenginesettings.h"
#include "metadatasynchronizer.h"
#include "newitemsfinder.h"
#include "queuemgrwindow.h"
#include "scancontroller.h"
#include "setup.h"
#include "sidebar.h"
#include "slideshow.h"
#include "slideshowbuilder.h"
#include "statusprogressbar.h"
#include "tableview.h"
#include "tagmodificationhelper.h"
#include "tagsactionmngr.h"
#include "tagscache.h"
#include "tagsmanager.h"
#include "thumbsgenerator.h"
#include "trashview.h"
#include "versionmanagersettings.h"
#include "contextmenuhelper.h"

#ifdef HAVE_MEDIAPLAYER
#   include "mediaplayerview.h"
#endif //HAVE_MEDIAPLAYER

#ifdef HAVE_MARBLE
#   include "mapwidgetview.h"
#endif // HAVE_MARBLE

namespace Digikam
{

class Q_DECL_HIDDEN ItemIconView::Private
{
public:

    explicit Private()
      : needDispatchSelection(false),
        useAlbumHistory(false),
        initialAlbumID(0),
        thumbSize(ThumbnailSize::Medium),
        dockArea(nullptr),
        splitter(nullptr),
        selectionTimer(nullptr),
        thumbSizeTimer(nullptr),
        albumFolderSideBar(nullptr),
        tagViewSideBar(nullptr),
        labelsSideBar(nullptr),
        dateViewSideBar(nullptr),
        timelineSideBar(nullptr),
        searchSideBar(nullptr),
        fuzzySearchSideBar(nullptr),

#ifdef HAVE_MARBLE
        gpsSearchSideBar(nullptr),
        mapView(nullptr),
#endif // HAVE_MARBLE

        peopleSideBar(nullptr),
        parent(nullptr),
        iconView(nullptr),
        tableView(nullptr),
        trashView(nullptr),
        utilities(nullptr),
        albumManager(nullptr),
        albumHistory(nullptr),
        stackedview(nullptr),
        lastViewMode(StackedView::IconViewMode),
        albumModificationHelper(nullptr),
        tagModificationHelper(nullptr),
        searchModificationHelper(nullptr),
        leftSideBar(nullptr),
        rightSideBar(nullptr),
        filterWidget(nullptr),
        optionAlbumViewPrefix(QLatin1String("AlbumView")),
        modelCollection(nullptr),
        labelsSearchHandler(nullptr)
    {
    }

    QString userPresentableAlbumTitle(const QString& album) const;
    void    addPageUpDownActions(ItemIconView* const q, QWidget* const w);

public:

    bool                          needDispatchSelection;
    bool                          useAlbumHistory;

    int                           initialAlbumID;
    int                           thumbSize;

    QMainWindow*                  dockArea;

    SidebarSplitter*              splitter;

    QTimer*                       selectionTimer;
    QTimer*                       thumbSizeTimer;

    // left side bar
    AlbumFolderViewSideBarWidget* albumFolderSideBar;
    TagViewSideBarWidget*         tagViewSideBar;
    LabelsSideBarWidget*          labelsSideBar;
    DateFolderViewSideBarWidget*  dateViewSideBar;
    TimelineSideBarWidget*        timelineSideBar;
    SearchSideBarWidget*          searchSideBar;
    FuzzySearchSideBarWidget*     fuzzySearchSideBar;

#ifdef HAVE_MARBLE
    GPSSearchSideBarWidget*       gpsSearchSideBar;
    MapWidgetView*                mapView;
#endif // HAVE_MARBLE

    PeopleSideBarWidget*          peopleSideBar;
    DigikamApp*                   parent;
    DigikamItemView*             iconView;
    TableView*                    tableView;
    TrashView*                    trashView;
    ItemViewUtilities*           utilities;
    AlbumManager*                 albumManager;
    AlbumHistory*                 albumHistory;
    StackedView*                  stackedview;
    StackedView::StackedViewMode  lastViewMode;

    AlbumModificationHelper*      albumModificationHelper;
    TagModificationHelper*        tagModificationHelper;
    SearchModificationHelper*     searchModificationHelper;

    Sidebar*                      leftSideBar;
    ItemPropertiesSideBarDB*     rightSideBar;

    FilterSideBarWidget*          filterWidget;

    QString                       optionAlbumViewPrefix;

    QList<SidebarWidget*>         leftSideBarWidgets;

    DModelFactory*       modelCollection;
    AlbumLabelsSearchHandler*     labelsSearchHandler;
};

QString ItemIconView::Private::userPresentableAlbumTitle(const QString& title) const
{
    if (title == SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarSketchSearch))
    {
        return i18n("Fuzzy Sketch Search");
    }
    else if (title == SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch))
    {
        return i18n("Fuzzy Image Search");
    }
    else if (title == SAlbum::getTemporaryTitle(DatabaseSearch::MapSearch))
    {
        return i18n("Map Search");
    }
    else if (title == SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch) ||
             title == SAlbum::getTemporaryTitle(DatabaseSearch::KeywordSearch))
    {
        return i18n("Last Search");
    }
    else if (title == SAlbum::getTemporaryTitle(DatabaseSearch::TimeLineSearch))
    {
        return i18n("Timeline");
    }

    return title;
}

void ItemIconView::Private::addPageUpDownActions(ItemIconView* const q, QWidget* const w)
{
    defineShortcut(w, Qt::Key_PageDown, q, SLOT(slotNextItem()));
    defineShortcut(w, Qt::Key_Down,     q, SLOT(slotNextItem()));
    defineShortcut(w, Qt::Key_Right,    q, SLOT(slotNextItem()));

    defineShortcut(w, Qt::Key_PageUp,   q, SLOT(slotPrevItem()));
    defineShortcut(w, Qt::Key_Up,       q, SLOT(slotPrevItem()));
    defineShortcut(w, Qt::Key_Left,     q, SLOT(slotPrevItem()));
}

// -------------------------------------------------------------------------------------------

ItemIconView::ItemIconView(QWidget* const parent, DModelFactory* const modelCollection)
    : DHBox(parent),
      d(new Private)
{
    qRegisterMetaType<SlideShowSettings>("SlideShowSettings");

    d->parent                   = static_cast<DigikamApp*>(parent);
    d->modelCollection          = modelCollection;
    d->albumManager             = AlbumManager::instance();

    d->albumModificationHelper  = new AlbumModificationHelper(this, this);
    d->tagModificationHelper    = new TagModificationHelper(this, this);
    d->searchModificationHelper = new SearchModificationHelper(this, this);

    const int spacing           = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->splitter    = new SidebarSplitter;
    d->splitter->setFrameStyle(QFrame::NoFrame);
    d->splitter->setFrameShadow(QFrame::Plain);
    d->splitter->setFrameShape(QFrame::NoFrame);
    d->splitter->setOpaqueResize(false);

    d->leftSideBar = new Sidebar(this, d->splitter, Qt::LeftEdge);
    d->leftSideBar->setObjectName(QLatin1String("Digikam Left Sidebar"));
    d->leftSideBar->setContentsMargins(0, 0, spacing, 0);
    d->splitter->setParent(this);

    // The dock area where the thumbnail bar is allowed to go.
    d->dockArea    = new QMainWindow(this, Qt::Widget);
    d->dockArea->setContentsMargins(QMargins());
    d->splitter->addWidget(d->dockArea);
    d->stackedview = new StackedView(d->dockArea);
    d->dockArea->setCentralWidget(d->stackedview);
    d->stackedview->setDockArea(d->dockArea);

    d->iconView  = d->stackedview->imageIconView();

#ifdef HAVE_MARBLE
    d->mapView   = d->stackedview->mapWidgetView();
#endif // HAVE_MARBLE

    d->tableView = d->stackedview->tableView();
    d->trashView = d->stackedview->trashView();

    d->utilities = d->iconView->utilities();

    d->addPageUpDownActions(this, d->stackedview->imagePreviewView());
    d->addPageUpDownActions(this, d->stackedview->thumbBar());

#ifdef HAVE_MEDIAPLAYER
    d->addPageUpDownActions(this, d->stackedview->mediaPlayerView());
#endif //HAVE_MEDIAPLAYER

    d->rightSideBar = new ItemPropertiesSideBarDB(this, d->splitter, Qt::RightEdge, true);
    d->rightSideBar->setObjectName(QLatin1String("Digikam Right Sidebar"));

    // album folder view
    d->albumFolderSideBar = new AlbumFolderViewSideBarWidget(d->leftSideBar,
                                                             d->modelCollection->getAlbumModel(),
                                                             d->albumModificationHelper);
    d->leftSideBarWidgets << d->albumFolderSideBar;

    connect(d->albumFolderSideBar, SIGNAL(signalFindDuplicates(PAlbum*)),
            this, SLOT(slotNewDuplicatesSearch(PAlbum*)));

    // Tags sidebar tab contents.
    d->tagViewSideBar = new TagViewSideBarWidget(d->leftSideBar, d->modelCollection->getTagModel());
    d->leftSideBarWidgets << d->tagViewSideBar;

    connect(d->tagViewSideBar, SIGNAL(signalFindDuplicates(QList<TAlbum*>)),
            this, SLOT(slotNewDuplicatesSearch(QList<TAlbum*>)));

    // Labels sidebar
    d->labelsSideBar       = new LabelsSideBarWidget(d->leftSideBar);
    d->leftSideBarWidgets << d->labelsSideBar;
    d->labelsSearchHandler = new AlbumLabelsSearchHandler(d->labelsSideBar->labelsTree());

    // date view
    d->dateViewSideBar = new DateFolderViewSideBarWidget(d->leftSideBar,
                                                         d->modelCollection->getDateAlbumModel(),
                                                         d->iconView->imageAlbumFilterModel());
    d->leftSideBarWidgets << d->dateViewSideBar;

    // timeline side bar
    d->timelineSideBar = new TimelineSideBarWidget(d->leftSideBar,
                                                   d->modelCollection->getSearchModel(),
                                                   d->searchModificationHelper);
    d->leftSideBarWidgets << d->timelineSideBar;

    // Search sidebar tab contents.
    d->searchSideBar = new SearchSideBarWidget(d->leftSideBar,
                                               d->modelCollection->getSearchModel(),
                                               d->searchModificationHelper);
    d->leftSideBarWidgets << d->searchSideBar;

    // Fuzzy search
    d->fuzzySearchSideBar = new FuzzySearchSideBarWidget(d->leftSideBar,
                                                         d->modelCollection->getSearchModel(),
                                                         d->searchModificationHelper);
    d->leftSideBarWidgets << d->fuzzySearchSideBar;

    connect(d->fuzzySearchSideBar,SIGNAL(signalActive(bool)),
            this, SIGNAL(signalFuzzySidebarActive(bool)));

#ifdef HAVE_MARBLE
    d->gpsSearchSideBar = new GPSSearchSideBarWidget(d->leftSideBar,
                                                     d->modelCollection->getSearchModel(),
                                                     d->searchModificationHelper,
                                                     d->iconView->imageFilterModel(),
                                                     d->iconView->getSelectionModel());

    d->leftSideBarWidgets << d->gpsSearchSideBar;
#endif // HAVE_MARBLE

    // People Sidebar
    d->peopleSideBar = new PeopleSideBarWidget(d->leftSideBar,
                                               d->modelCollection->getTagFacesModel(),
                                               d->searchModificationHelper);

    connect(d->peopleSideBar, SIGNAL(requestFaceMode(bool)),
            d->iconView, SLOT(setFaceMode(bool)));

    connect(d->peopleSideBar, SIGNAL(signalFindDuplicates(QList<TAlbum*>)),
            this, SLOT(slotNewDuplicatesSearch(QList<TAlbum*>)));

    d->leftSideBarWidgets << d->peopleSideBar;

    foreach (SidebarWidget* const leftWidget, d->leftSideBarWidgets)
    {
        d->leftSideBar->appendTab(leftWidget, leftWidget->getIcon(), leftWidget->getCaption());

        connect(leftWidget, SIGNAL(requestActiveTab(SidebarWidget*)),
                this, SLOT(slotLeftSideBarActivate(SidebarWidget*)));
    }

    // add only page up and down to work correctly with QCompleter
    defineShortcut(d->rightSideBar->imageDescEditTab(), Qt::Key_PageDown, this, SLOT(slotNextItem()));
    defineShortcut(d->rightSideBar->imageDescEditTab(), Qt::Key_PageUp,   this, SLOT(slotPrevItem()));

    // Tags Filter sidebar tab contents.
    d->filterWidget   = new FilterSideBarWidget(d->rightSideBar, d->modelCollection->getTagFilterModel());
    d->rightSideBar->appendTab(d->filterWidget, QIcon::fromTheme(QLatin1String("view-filter")), i18n("Filters"));

    // Versions sidebar overlays
    d->rightSideBar->getFiltersHistoryTab()->addOpenAlbumAction(d->iconView->imageModel());
    d->rightSideBar->getFiltersHistoryTab()->addShowHideOverlay();

    d->selectionTimer = new QTimer(this);
    d->selectionTimer->setSingleShot(true);
    d->selectionTimer->setInterval(75);
    d->thumbSizeTimer = new QTimer(this);
    d->thumbSizeTimer->setSingleShot(true);
    d->thumbSizeTimer->setInterval(300);

    d->albumHistory = new AlbumHistory();

    slotSidebarTabTitleStyleChanged();
    setupConnections();

    connect(d->rightSideBar->imageDescEditTab()->getNewTagEdit(), SIGNAL(taggingActionFinished()),
            this, SLOT(slotFocusAndNextImage()));

    connect(d->rightSideBar, SIGNAL(signalSetupMetadataFilters(int)),
            this, SLOT(slotSetupMetadataFilters(int)));
}

ItemIconView::~ItemIconView()
{
    //saveViewState();

    delete d->labelsSearchHandler;
    delete d->albumHistory;
    delete d;
}

void ItemIconView::applySettings()
{
    foreach (SidebarWidget* const sidebarWidget, d->leftSideBarWidgets)
    {
        sidebarWidget->applySettings();
    }

    d->iconView->imageFilterModel()->setVersionItemFilterSettings(VersionItemFilterSettings(ApplicationSettings::instance()->getVersionManagerSettings()));

    refreshView();
}

void ItemIconView::refreshView()
{
    d->rightSideBar->refreshTagsView();
}

void ItemIconView::setupConnections()
{
    // -- DigikamApp connections ----------------------------------

    connect(d->parent, SIGNAL(signalEscapePressed()),
            this, SLOT(slotEscapePreview()));

    connect(d->parent, SIGNAL(signalEscapePressed()),
            d->stackedview, SLOT(slotEscapePreview()));

    connect(d->parent, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->parent, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->parent, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));

    connect(d->parent, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));

    connect(d->parent, SIGNAL(signalCutAlbumItemsSelection()),
            d->iconView, SLOT(cut()));

    connect(d->parent, SIGNAL(signalCopyAlbumItemsSelection()),
            d->iconView, SLOT(copy()));

    connect(d->parent, SIGNAL(signalPasteAlbumItemsSelection()),
            this, SLOT(slotImagePaste()));

    // -- AlbumManager connections --------------------------------

    connect(d->albumManager, SIGNAL(signalAlbumCurrentChanged(QList<Album*>)),
            this, SLOT(slotAlbumSelected(QList<Album*>)));

    connect(d->albumManager, SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(slotAllAlbumsLoaded()));

    connect(d->albumManager, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    // -- IconView Connections -------------------------------------

    connect(d->iconView->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotImageSelected()));

    connect(d->iconView->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(slotImageSelected()));

    connect(d->iconView->model(), SIGNAL(layoutChanged()),
            this, SLOT(slotImageSelected()));

    connect(d->iconView, SIGNAL(selectionChanged()),
            this, SLOT(slotImageSelected()));

    connect(d->iconView, SIGNAL(previewRequested(ItemInfo)),
            this, SLOT(slotTogglePreviewMode(ItemInfo)));

    connect(d->iconView, SIGNAL(fullscreenRequested(ItemInfo)),
            this, SLOT(slotSlideShowManualFrom(ItemInfo)));

    connect(d->iconView, SIGNAL(zoomOutStep()),
            this, SLOT(slotZoomOut()));

    connect(d->iconView, SIGNAL(zoomInStep()),
            this, SLOT(slotZoomIn()));

    connect(d->iconView, SIGNAL(signalShowContextMenu(QContextMenuEvent*,QList<QAction*>)),
            this, SLOT(slotShowContextMenu(QContextMenuEvent*,QList<QAction*>)));

    connect(d->iconView, SIGNAL(signalShowContextMenuOnInfo(QContextMenuEvent*,ItemInfo,QList<QAction*>,ItemFilterModel*)),
            this, SLOT(slotShowContextMenuOnInfo(QContextMenuEvent*,ItemInfo,QList<QAction*>,ItemFilterModel*)));

    connect(d->iconView, SIGNAL(signalShowGroupContextMenu(QContextMenuEvent*,QList<ItemInfo>,ItemFilterModel*)),
            this, SLOT(slotShowGroupContextMenu(QContextMenuEvent*,QList<ItemInfo>,ItemFilterModel*)));

    // -- TableView Connections -----------------------------------

    connect(d->tableView, SIGNAL(signalPreviewRequested(ItemInfo)),
            this, SLOT(slotTogglePreviewMode(ItemInfo)));

    connect(d->tableView, SIGNAL(signalZoomOutStep()),
            this, SLOT(slotZoomOut()));

    connect(d->tableView, SIGNAL(signalZoomInStep()),
            this, SLOT(slotZoomIn()));

    connect(d->tableView, SIGNAL(signalShowContextMenu(QContextMenuEvent*,QList<QAction*>)),
            this, SLOT(slotShowContextMenu(QContextMenuEvent*,QList<QAction*>)));

    connect(d->tableView, SIGNAL(signalShowContextMenuOnInfo(QContextMenuEvent*,ItemInfo,QList<QAction*>,ItemFilterModel*)),
            this, SLOT(slotShowContextMenuOnInfo(QContextMenuEvent*,ItemInfo,QList<QAction*>,ItemFilterModel*)));

    // TableView::signalItemsChanged is emitted when something changes in the model that
    // ItemIconView should care about, not only the selection.
    connect(d->tableView, SIGNAL(signalItemsChanged()),
            this, SLOT(slotImageSelected()));

    // -- Trash View Connections ----------------------------------

    connect(d->trashView, SIGNAL(selectionChanged()),
            this, SLOT(slotImageSelected()));

    // -- Sidebar Connections -------------------------------------

    connect(d->leftSideBar, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotLeftSidebarChangedTab(QWidget*)));

    connect(d->rightSideBar, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));

    connect(d->rightSideBar, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->rightSideBar, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->rightSideBar, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSideBar, SLOT(slotNoCurrentItem()));

#ifdef HAVE_MARBLE
    connect(d->gpsSearchSideBar, SIGNAL(signalMapSoloItems(QList<qlonglong>,QString)),
            d->iconView->imageFilterModel(), SLOT(setIdWhitelist(QList<qlonglong>,QString)));
#endif // HAVE_MARBLE

    // -- Filter Bars Connections ---------------------------------

    ItemAlbumFilterModel* const model = d->iconView->imageAlbumFilterModel();

    connect(d->filterWidget,
            SIGNAL(signalTagFilterChanged(QList<int>,QList<int>,ItemFilterSettings::MatchingCondition,bool,QList<int>,QList<int>)),
            d->iconView->imageFilterModel(), SLOT(setTagFilter(QList<int>,QList<int>,ItemFilterSettings::MatchingCondition,bool,QList<int>,QList<int>)));

    connect(d->filterWidget, SIGNAL(signalRatingFilterChanged(int,ItemFilterSettings::RatingCondition,bool)),
            model, SLOT(setRatingFilter(int,ItemFilterSettings::RatingCondition,bool)));

    connect(d->filterWidget, SIGNAL(signalSearchTextFilterChanged(SearchTextFilterSettings)),
            model, SLOT(setTextFilter(SearchTextFilterSettings)));

    connect(model, SIGNAL(filterMatchesForText(bool)),
            d->filterWidget, SLOT(slotFilterMatchesForText(bool)));

    connect(d->filterWidget, SIGNAL(signalMimeTypeFilterChanged(int)),
            model, SLOT(setMimeTypeFilter(int)));

    connect(d->filterWidget, SIGNAL(signalGeolocationFilterChanged(ItemFilterSettings::GeolocationCondition)),
            model, SLOT(setGeolocationFilter(ItemFilterSettings::GeolocationCondition)));

    // -- Preview image widget Connections ------------------------

    connect(d->stackedview, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->stackedview, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->stackedview, SIGNAL(signalDeleteItem()),
            this, SLOT(slotImageDelete()));

    connect(d->stackedview, SIGNAL(signalViewModeChanged()),
            this, SLOT(slotViewModeChanged()));

    connect(d->stackedview, SIGNAL(signalEscapePreview()),
            this, SLOT(slotEscapePreview()));

    connect(d->stackedview, SIGNAL(signalSlideShowCurrent()),
            this, SLOT(slotSlideShowManualFromCurrent()));

    connect(d->stackedview, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    connect(d->stackedview, SIGNAL(signalAddToExistingQueue(int)),
            this, SLOT(slotImageAddToExistingQueue(int)));

    connect(d->stackedview, SIGNAL(signalGotoAlbumAndItem(ItemInfo)),
            this, SLOT(slotGotoAlbumAndItem(ItemInfo)));

    connect(d->stackedview, SIGNAL(signalGotoDateAndItem(ItemInfo)),
            this, SLOT(slotGotoDateAndItem(ItemInfo)));

    connect(d->stackedview, SIGNAL(signalGotoTagAndItem(int)),
            this, SLOT(slotGotoTagAndItem(int)));

    connect(d->stackedview, SIGNAL(signalPopupTagsView()),
            d->rightSideBar, SLOT(slotPopupTagsView()));

    // -- FileActionMngr progress ---------------

    connect(FileActionMngr::instance(), SIGNAL(signalImageChangeFailed(QString,QStringList)),
            this, SLOT(slotImageChangeFailed(QString,QStringList)));

    // -- timers ---------------

    connect(d->selectionTimer, SIGNAL(timeout()),
            this, SLOT(slotDispatchImageSelected()));

    connect(d->thumbSizeTimer, SIGNAL(timeout()),
            this, SLOT(slotThumbSizeEffect()));

    // -- Album Settings ----------------

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSidebarTabTitleStyleChanged()));

    // -- Album History -----------------

    connect(this, SIGNAL(signalAlbumSelected(Album*)),
            d->albumHistory, SLOT(slotAlbumSelected()));

    connect(this, SIGNAL(signalImageSelected(ItemInfoList,ItemInfoList)),
            d->albumHistory, SLOT(slotImageSelected(ItemInfoList)));

    connect(d->iconView, SIGNAL(currentChanged(ItemInfo)),
            d->albumHistory, SLOT(slotCurrentChange(ItemInfo)));

    connect(d->iconView->imageModel(), SIGNAL(imageInfosAdded(QList<ItemInfo>)),
            d->albumHistory, SLOT(slotAlbumCurrentChanged()));

    connect(d->albumHistory, SIGNAL(signalSetCurrent(qlonglong)),
            this, SLOT(slotSetCurrentWhenAvailable(qlonglong)));

    connect(d->albumHistory, SIGNAL(signalSetSelectedInfos(QList<ItemInfo>)),
            d->iconView, SLOT(setSelectedItemInfos(QList<ItemInfo>)));

    connect(d->albumManager, SIGNAL(signalAlbumDeleted(Album*)),
            d->albumHistory, SLOT(slotAlbumDeleted(Album*)));

    connect(d->albumManager, SIGNAL(signalAlbumsCleared()),
            d->albumHistory, SLOT(slotAlbumsCleared()));

    // -- Image versions ----------------

    connect(d->rightSideBar->getFiltersHistoryTab(), SIGNAL(imageSelected(ItemInfo)),
            d->iconView, SLOT(hintAt(ItemInfo)));

    connect(d->rightSideBar->getFiltersHistoryTab(), SIGNAL(actionTriggered(ItemInfo)),
            this, SLOT(slotGotoAlbumAndItem(ItemInfo)));
}

void ItemIconView::connectIconViewFilter(FilterStatusBar* const filterbar)
{
    ItemAlbumFilterModel* const model = d->iconView->imageAlbumFilterModel();

    connect(model, SIGNAL(filterMatches(bool)),
            filterbar, SLOT(slotFilterMatches(bool)));

    connect(model, SIGNAL(filterSettingsChanged(ItemFilterSettings)),
            filterbar, SLOT(slotFilterSettingsChanged(ItemFilterSettings)));

    connect(filterbar, SIGNAL(signalResetFilters()),
            d->filterWidget, SLOT(slotResetFilters()));

    connect(filterbar, SIGNAL(signalPopupFiltersView()),
            this, SLOT(slotPopupFiltersView()));
}

void ItemIconView::slotPopupFiltersView()
{
    d->rightSideBar->setActiveTab(d->filterWidget);
    d->filterWidget->setFocusToTextFilter();
}

void ItemIconView::loadViewState()
{
    foreach (SidebarWidget* const widget, d->leftSideBarWidgets)
    {
        widget->loadState();
    }

    d->filterWidget->loadState();

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("MainWindow"));

    // Restore the splitter
    d->splitter->restoreState(group);

    // Restore the thumbnail bar dock.
    QByteArray thumbbarState;
    thumbbarState     = group.readEntry(QLatin1String("ThumbbarState"), thumbbarState);
    d->dockArea->restoreState(QByteArray::fromBase64(thumbbarState));

    d->initialAlbumID = group.readEntry(QLatin1String("InitialAlbumID"), 0);

#ifdef HAVE_MARBLE
    d->mapView->loadState();
#endif // HAVE_MARBLE

    d->tableView->loadState();
    d->rightSideBar->loadState();
}

void ItemIconView::saveViewState()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("MainWindow"));

    foreach (SidebarWidget* const widget, d->leftSideBarWidgets)
    {
        widget->saveState();
    }

    d->filterWidget->saveState();

    // Save the splitter states.
    d->splitter->saveState(group);

    // Save the position and size of the thumbnail bar. The thumbnail bar dock
    // needs to be closed explicitly, because when it is floating and visible
    // (when the user is in image preview mode) when the layout is saved, it
    // also reappears when restoring the view, while it should always be hidden.
    d->stackedview->thumbBarDock()->close();
    group.writeEntry(QLatin1String("ThumbbarState"), d->dockArea->saveState().toBase64());

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    Album* album            = nullptr;

    if (!albumList.isEmpty())
    {
        album = albumList.first();
    }

    if (album)
    {
        group.writeEntry(QLatin1String("InitialAlbumID"), album->globalID());
    }
    else
    {
        group.writeEntry(QLatin1String("InitialAlbumID"), 0);
    }

#ifdef HAVE_MARBLE
    d->mapView->saveState();
#endif // HAVE_MARBLE

    d->tableView->saveState();
    d->rightSideBar->saveState();
}

QList<SidebarWidget*> ItemIconView::leftSidebarWidgets() const
{
    return d->leftSideBarWidgets;
}

QList<QUrl> ItemIconView::allUrls(bool grouping) const
{
    /// @todo This functions seems not to be used anywhere right now

    return allInfo(grouping).toImageUrlList();
}

QList<QUrl> ItemIconView::selectedUrls(bool grouping) const
{
    return selectedInfoList(false, grouping).toImageUrlList();
}

QList<QUrl> ItemIconView::selectedUrls(const ApplicationSettings::OperationType type) const
{
    return selectedInfoList(type).toImageUrlList();
}

void ItemIconView::showSideBars()
{
    d->leftSideBar->restore();
    d->rightSideBar->restore();
}

void ItemIconView::hideSideBars()
{
    d->leftSideBar->backup();
    d->rightSideBar->backup();
}

void ItemIconView::toggleLeftSidebar()
{
    d->leftSideBar->isExpanded() ? d->leftSideBar->shrink()
                                 : d->leftSideBar->expand();
}

void ItemIconView::toggleRightSidebar()
{
    d->rightSideBar->isExpanded() ? d->rightSideBar->shrink()
                                  : d->rightSideBar->expand();
}

void ItemIconView::previousLeftSideBarTab()
{
    d->leftSideBar->activePreviousTab();
}

void ItemIconView::nextLeftSideBarTab()
{
    d->leftSideBar->activeNextTab();
}

void ItemIconView::previousRightSideBarTab()
{
    d->rightSideBar->activePreviousTab();
}

void ItemIconView::nextRightSideBarTab()
{
    d->rightSideBar->activeNextTab();
}

void ItemIconView::slotFirstItem()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->slotGoToRow(0, false);
            break;

        default:
            // all other views are tied to IconView's selection model
            d->iconView->toFirstIndex();
    }
}

void ItemIconView::slotPrevItem()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->slotGoToRow(-1, true);
            break;

        default:
            // all other views are tied to IconView's selection model
            d->iconView->toPreviousIndex();
    }
}

void ItemIconView::slotNextItem()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->slotGoToRow(1, true);
            break;

        default:
            // all other views are tied to IconView's selection model
            d->iconView->toNextIndex();
    }
}

void ItemIconView::slotLastItem()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->slotGoToRow(-1, false);
            break;

        default:
            // all other views are tied to IconView's selection model
            d->iconView->toLastIndex();
    }
}

void ItemIconView::slotSelectItemByUrl(const QUrl& url)
{
    /// @todo This functions seems not to be used anywhere right now
    /// @todo Adapt to TableView
    d->iconView->toIndex(url);
}

void ItemIconView::slotAllAlbumsLoaded()
{
    disconnect(d->albumManager, SIGNAL(signalAllAlbumsLoaded()),
               this, SLOT(slotAllAlbumsLoaded()));

    loadViewState();
    d->leftSideBar->loadState();
    d->rightSideBar->loadState();
    d->rightSideBar->populateTags();

    // now that all albums have been loaded, activate the albumHistory
    d->useAlbumHistory = true;
    Album* const album = d->albumManager->findAlbum(d->initialAlbumID);
    d->albumManager->setCurrentAlbums(QList<Album*>() << album);
}

void ItemIconView::slotSortAlbums(int role)
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setAlbumSortRole((ApplicationSettings::AlbumSortRole) role);
    settings->saveSettings();
    //A dummy way to force the tree view to resort if the album sort role changed

    PAlbum* const albumBeforeSorting = d->albumFolderSideBar->currentAlbum();
    settings->setAlbumSortChanged(true);
    d->albumFolderSideBar->doSaveState();
    d->albumFolderSideBar->doLoadState();
    d->albumFolderSideBar->doSaveState();
    d->albumFolderSideBar->doLoadState();
    settings->setAlbumSortChanged(false);

    if (d->leftSideBar->getActiveTab() == d->albumFolderSideBar)
    {
        d->albumFolderSideBar->setCurrentAlbum(albumBeforeSorting);
    }
}

void ItemIconView::slotNewAlbum()
{
    // TODO use the selection model of the view instead
    d->albumModificationHelper->slotAlbumNew(d->albumFolderSideBar->currentAlbum());
}

void ItemIconView::slotDeleteAlbum()
{
    d->albumModificationHelper->slotAlbumDelete(d->albumFolderSideBar->currentAlbum());
}

void ItemIconView::slotRenameAlbum()
{
    d->albumModificationHelper->slotAlbumRename(d->albumFolderSideBar->currentAlbum());
}

void ItemIconView::slotNewTag()
{
    QList<TAlbum*> talbums = AlbumManager::instance()->currentTAlbums();

    if (!talbums.isEmpty())
        d->tagModificationHelper->slotTagNew(talbums.first());
}

void ItemIconView::slotDeleteTag()
{
    QList<TAlbum*> talbums = AlbumManager::instance()->currentTAlbums();

    if (!talbums.isEmpty())
        d->tagModificationHelper->slotTagDelete(talbums.first());
}

void ItemIconView::slotEditTag()
{
    QList<TAlbum*> talbums = AlbumManager::instance()->currentTAlbums();

    if (!talbums.isEmpty())
        d->tagModificationHelper->slotTagEdit(talbums.first());
}

void ItemIconView::slotOpenTagsManager()
{
    TagsManager* const tagMngr = TagsManager::instance();
    tagMngr->show();
    tagMngr->activateWindow();
    tagMngr->raise();
}

void ItemIconView::slotAssignTag()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->setFocusToNewTagEdit();
}

void ItemIconView::slotNewKeywordSearch()
{
    slotLeftSideBarActivate(d->searchSideBar);
    d->searchSideBar->newKeywordSearch();
}

void ItemIconView::slotNewAdvancedSearch()
{
    slotLeftSideBarActivate(d->searchSideBar);
    d->searchSideBar->newAdvancedSearch();
}

void ItemIconView::slotNewDuplicatesSearch(PAlbum* album)
{
    slotLeftSideBarActivate(d->fuzzySearchSideBar);
    d->fuzzySearchSideBar->newDuplicatesSearch(album);
}

void ItemIconView::slotNewDuplicatesSearch(const QList<PAlbum*>& albums)
{
    slotLeftSideBarActivate(d->fuzzySearchSideBar);
    d->fuzzySearchSideBar->newDuplicatesSearch(albums);
}

void ItemIconView::slotNewDuplicatesSearch(const QList<TAlbum*>& albums)
{
    slotLeftSideBarActivate(d->fuzzySearchSideBar);
    d->fuzzySearchSideBar->newDuplicatesSearch(albums);
}

void ItemIconView::slotAlbumsCleared()
{
    emit signalAlbumSelected(nullptr);
}

void ItemIconView::slotAlbumHistoryBack(int steps)
{
    QList<Album*> albums;
    QWidget* widget = nullptr;

    d->albumHistory->back(albums, &widget, steps);

    changeAlbumFromHistory(albums, widget);
}

void ItemIconView::slotAlbumHistoryForward(int steps)
{
    QList<Album*> albums;
    QWidget* widget = nullptr;

    d->albumHistory->forward(albums, &widget, steps);

    changeAlbumFromHistory(albums , widget);
}

// TODO update, use SideBarWidget instead of QWidget
void ItemIconView::changeAlbumFromHistory(const QList<Album*>& album, QWidget* const widget)
{
    if (!(album.isEmpty()) && widget)
    {
        // TODO update, temporary casting until signature is changed
        SidebarWidget* const sideBarWidget = dynamic_cast<SidebarWidget*>(widget);

        if (sideBarWidget)
        {
            sideBarWidget->changeAlbumFromHistory(album);
            slotLeftSideBarActivate(sideBarWidget);

            if (sideBarWidget == d->labelsSideBar)
            {
                d->labelsSearchHandler->restoreSelectionFromHistory(d->albumHistory->neededLabels());
            }
        }

        d->parent->enableAlbumBackwardHistory(d->useAlbumHistory && !d->albumHistory->isBackwardEmpty());
        d->parent->enableAlbumForwardHistory(d->useAlbumHistory && !d->albumHistory->isForwardEmpty());
    }
}

void ItemIconView::clearHistory()
{
    d->albumHistory->clearHistory();
    d->parent->enableAlbumBackwardHistory(false);
    d->parent->enableAlbumForwardHistory(false);
}

void ItemIconView::getBackwardHistory(QStringList& titles)
{
    d->albumHistory->getBackwardHistory(titles);

    for (int i = 0 ; i < titles.size() ; ++i)
    {
        titles[i] = d->userPresentableAlbumTitle(titles.at(i));
    }
}

void ItemIconView::getForwardHistory(QStringList& titles)
{
    d->albumHistory->getForwardHistory(titles);

    for (int i = 0 ; i < titles.size() ; ++i)
    {
        titles[i] = d->userPresentableAlbumTitle(titles.at(i));
    }
}

void ItemIconView::slotGotoAlbumAndItem(const ItemInfo& imageInfo)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "going to " << imageInfo;

    emit signalNoCurrentItem();

    PAlbum* const album = AlbumManager::instance()->findPAlbum(imageInfo.albumId());

    d->albumFolderSideBar->setCurrentAlbum(album);
    slotLeftSideBarActivate(d->albumFolderSideBar);

    // Set the activate item url to find in the Album View after
    // all items have be reloaded.
    slotSetCurrentWhenAvailable(imageInfo.id());

    // And finally toggle album manager to handle album history and
    // reload all items.
    d->albumManager->setCurrentAlbums(QList<Album*>() << album);
}

void ItemIconView::slotGotoDateAndItem(const ItemInfo& imageInfo)
{
    QDate date = imageInfo.dateTime().date();

    emit signalNoCurrentItem();

    // Change to Date Album view.
    // Note, that this also opens the side bar if it is closed; this is
    // considered as a feature, because it highlights that the view was changed.
    slotLeftSideBarActivate(d->dateViewSideBar);

    // Set the activate item url to find in the Album View after
    // all items have be reloaded.
    slotSetCurrentWhenAvailable(imageInfo.id());

    // Change the year and month of the iconItem (day is unused).
    d->dateViewSideBar->gotoDate(date);
}

void ItemIconView::slotGotoTagAndItem(int tagID)
{
    // FIXME: Arnd: don't know yet how to get the iconItem passed through ...
    //  then we would know how to use the following ...
    //  KURL url(iconItem->imageInfo()->kurl());
    //  url.cleanPath();

    emit signalNoCurrentItem();

    // Change to Tag Folder view.
    // Note, that this also opens the side bar if it is closed; this is
    // considered as a feature, because it highlights that the view was changed.
    slotLeftSideBarActivate(d->tagViewSideBar);

    // Set the current tag in the tag folder view.
    // TODO this slot should use a TAlbum pointer directly
    TAlbum* const tag = AlbumManager::instance()->findTAlbum(tagID);

    if (tag)
    {
        d->tagViewSideBar->setCurrentAlbum(tag);
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Could not find a tag album for tag id " << tagID;
    }

    // Set the activate item url to find in the Tag View after
    // all items have be reloaded.
    // FIXME: see above
    // d->iconView->setAlbumItemToFind(url);
}

void ItemIconView::slotSelectAlbum(const QUrl& url)
{
    PAlbum* const album = d->albumManager->findPAlbum(url);

    if (!album)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Unable to find album for " << url;
        return;
    }

    slotLeftSideBarActivate(d->albumFolderSideBar);
    d->albumFolderSideBar->setCurrentAlbum(album);
}

void ItemIconView::slotAlbumSelected(const QList<Album*>& albums)
{
    emit signalNoCurrentItem();
    emit signalAlbumSelected(nullptr);

    if (albums.isEmpty() || !albums.first())
    {
        d->iconView->openAlbum(QList<Album*>());

#ifdef HAVE_MARBLE
        d->mapView->openAlbum(nullptr);
#endif // HAVE_MARBLE

        slotTogglePreviewMode(ItemInfo());
        return;
    }

    Album* const album = albums.first();
    emit signalAlbumSelected(album);

    if (d->useAlbumHistory && !d->labelsSearchHandler->isRestoringSelectionFromHistory())
    {
        if (!(d->leftSideBar->getActiveTab() == d->labelsSideBar))
        {
            d->albumHistory->addAlbums(albums, d->leftSideBar->getActiveTab());
        }
        else
        {
            if (albums.first()->isUsedByLabelsTree())
            {
                d->albumHistory->addAlbums(albums, d->leftSideBar->getActiveTab(), d->labelsSideBar->selectedLabels());
            }
        }
    }

    d->parent->enableAlbumBackwardHistory(d->useAlbumHistory && !d->albumHistory->isBackwardEmpty());
    d->parent->enableAlbumForwardHistory(d->useAlbumHistory && !d->albumHistory->isForwardEmpty());

    d->iconView->openAlbum(albums);

    if (album->isRoot())
    {
        d->stackedview->setViewMode(StackedView::WelcomePageMode);
    }
    else if (album->isTrashAlbum())
    {
        PAlbum* const palbum = d->albumManager->findPAlbum(album->parent()->id());

        if (palbum)
        {
            QUrl url = palbum->fileUrl().adjusted(QUrl::StripTrailingSlash);
            d->trashView->model()->loadItemsForCollection(url.toLocalFile());
            d->stackedview->setViewMode(StackedView::TrashViewMode);
            d->filterWidget->setEnabled(false);
        }
    }
    else
    {
        switch (viewMode())
        {
            case StackedView::PreviewImageMode:
            case StackedView::MediaPlayerMode:
            case StackedView::WelcomePageMode:
            case StackedView::TrashViewMode:
                slotTogglePreviewMode(ItemInfo());
                break;
            default:
                break;
        }

        d->filterWidget->setEnabled(true);
    }
}

void ItemIconView::slotAlbumOpenInFileManager()
{
    Album* const album = d->albumManager->currentAlbums().first();

    if (!album || album->type() != Album::PHYSICAL)
    {
        return;
    }

    if (album->isRoot())
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18n("Cannot open the root album. It is not a physical location."));
        return;
    }

    QList<QUrl> urls = selectedInfoList(true, true).toImageUrlList();

    if (!urls.isEmpty())
    {
        DFileOperations::openInFileManager(urls);
    }
    else
    {
        PAlbum* const palbum = dynamic_cast<PAlbum*>(album);

        if (palbum)
        {
            QUrl url(QUrl::fromLocalFile(palbum->folderPath()));
            DFileOperations::openInFileManager(QList<QUrl>() << url);
        }
    }
}

void ItemIconView::slotRefresh()
{
    switch (viewMode())
    {
        case StackedView::PreviewImageMode:
            d->stackedview->imagePreviewView()->reload();
            break;
#ifdef HAVE_MEDIAPLAYER
        case StackedView::MediaPlayerMode:
            d->stackedview->mediaPlayerView()->reload();
            break;
#endif //HAVE_MEDIAPLAYER
        default:
            Album* const album = currentAlbum();
            if (!album) return;

            // force reloading of thumbnails
            LoadingCacheInterface::cleanThumbnailCache();

            ThumbsGenerator* const tool = new ThumbsGenerator(true, album->id());
            tool->start();

            // if physical album, schedule a collection scan of current album's path
            if (album->type() == Album::PHYSICAL)
            {
                NewItemsFinder* const tool = new NewItemsFinder(NewItemsFinder::ScheduleCollectionScan,
                                                                QStringList() << static_cast<PAlbum*>(album)->folderPath());

                connect(tool, SIGNAL(signalComplete()),
                        this, SLOT(slotAlbumRefreshComplete()));

                tool->start();
            }
            break;
    }
}

void ItemIconView::slotAlbumRefreshComplete()
{
    // force reload. Should normally not be necessary, but we may have bugs
    qlonglong currentId = currentInfo().id();
    d->iconView->imageAlbumModel()->refresh();

    if (currentId != -1)
    {
        slotSetCurrentWhenAvailable(currentId);
    }
}

void ItemIconView::slotImageSelected()
{
    // delay to slotDispatchImageSelected
    d->needDispatchSelection = true;
    d->selectionTimer->start();

    switch (viewMode())
    {
        case StackedView::TableViewMode:
            emit signalSelectionChanged(d->tableView->numberOfSelectedItems());
            break;

        default:
            emit signalSelectionChanged(d->iconView->numberOfSelectedIndexes());
    }
}

void ItemIconView::slotDispatchImageSelected()
{
    if (viewMode() == StackedView::TrashViewMode)
    {
        d->rightSideBar->itemChanged(d->trashView->lastSelectedItemUrl());
        return;
    }

    if (d->needDispatchSelection)
    {
        // the list of ItemInfos of currently selected items, currentItem first
        const ItemInfoList list      = selectedInfoList(true, true);
        const ItemInfoList allImages = allInfo(true);

        if (list.isEmpty())
        {
            d->stackedview->setPreviewItem();
            emit signalImageSelected(list, allImages);
            emit signalNoCurrentItem();
        }
        else
        {
            d->rightSideBar->itemChanged(list);

            ItemInfo previousInfo;
            ItemInfo nextInfo;

            if (viewMode() == StackedView::TableViewMode)
            {
                previousInfo = d->tableView->previousInfo();
                nextInfo     = d->tableView->nextInfo();
            }
            else
            {
                previousInfo = d->iconView->previousInfo(list.first());
                nextInfo     = d->iconView->nextInfo(list.first());
            }

            if (viewMode() != StackedView::IconViewMode  &&
                viewMode() != StackedView::MapWidgetMode &&
                viewMode() != StackedView::TableViewMode)
            {
                d->stackedview->setPreviewItem(list.first(), previousInfo, nextInfo);
            }

            emit signalImageSelected(list, allImages);
        }

        d->needDispatchSelection = false;
    }
}

double ItemIconView::zoomMin() const
{
    return d->stackedview->zoomMin();
}

double ItemIconView::zoomMax() const
{
    return d->stackedview->zoomMax();
}

void ItemIconView::setZoomFactor(double zoom)
{
    d->stackedview->setZoomFactorSnapped(zoom);
}

void ItemIconView::slotZoomFactorChanged(double zoom)
{
    toggleZoomActions();
    emit signalZoomChanged(zoom);
}

void ItemIconView::setThumbSize(int size)
{
    if (viewMode() == StackedView::PreviewImageMode)
    {
        double z = DZoomBar::zoomFromSize(size, zoomMin(), zoomMax());
        setZoomFactor(z);
    }
    else if (viewMode() == StackedView::IconViewMode  ||
             viewMode() == StackedView::TableViewMode ||
             viewMode() == StackedView::TrashViewMode)
    {
        if (size > ThumbnailSize::maxThumbsSize())
        {
            d->thumbSize = ThumbnailSize::maxThumbsSize();
        }
        else if (size < ThumbnailSize::Small)
        {
            d->thumbSize = ThumbnailSize::Small;
        }
        else
        {
            d->thumbSize = size;
        }

        emit signalThumbSizeChanged(d->thumbSize);

        d->thumbSizeTimer->start();
    }
}

void ItemIconView::slotThumbSizeEffect()
{
    d->iconView->setThumbnailSize(ThumbnailSize(d->thumbSize));
    d->tableView->setThumbnailSize(ThumbnailSize(d->thumbSize));
    d->trashView->setThumbnailSize(ThumbnailSize(d->thumbSize));
    toggleZoomActions();

    ApplicationSettings::instance()->setDefaultIconSize(d->thumbSize);
}

void ItemIconView::toggleZoomActions()
{
    if (viewMode() == StackedView::PreviewImageMode)
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);

        if (d->stackedview->maxZoom())
        {
            d->parent->enableZoomPlusAction(false);
        }

        if (d->stackedview->minZoom())
        {
            d->parent->enableZoomMinusAction(false);
        }
    }
    else if (viewMode() == StackedView::IconViewMode ||
             viewMode() == StackedView::TableViewMode)
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);

        if (d->thumbSize >= ThumbnailSize::maxThumbsSize())
        {
            d->parent->enableZoomPlusAction(false);
        }

        if (d->thumbSize <= ThumbnailSize::Small)
        {
            d->parent->enableZoomMinusAction(false);
        }
    }
    else
    {
        d->parent->enableZoomMinusAction(false);
        d->parent->enableZoomPlusAction(false);
    }
}

void ItemIconView::slotZoomIn()
{
    if (viewMode() == StackedView::IconViewMode ||
        viewMode() == StackedView::TableViewMode)
    {
        setThumbSize(d->thumbSize + ThumbnailSize::Step);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }
    else if (viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedview->increaseZoom();
    }
}

void ItemIconView::slotZoomOut()
{
    if (viewMode() == StackedView::IconViewMode ||
        viewMode() == StackedView::TableViewMode)
    {
        setThumbSize(d->thumbSize - ThumbnailSize::Step);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }
    else if (viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedview->decreaseZoom();
    }
}

void ItemIconView::slotZoomTo100Percents()
{
    if (viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedview->toggleFitToWindowOr100();
    }
}

void ItemIconView::slotFitToWindow()
{
    if (viewMode() == StackedView::TableViewMode)
    {
        /// @todo We should choose an appropriate thumbnail size here
    }
    else if (viewMode() == StackedView::IconViewMode)
    {
        int nts = d->iconView->fitToWidthIcons();
        qCDebug(DIGIKAM_GENERAL_LOG) << "new thumb size = " << nts;
        setThumbSize(nts);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }
    else if (viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedview->fitToWindow();
    }
}

void ItemIconView::slotAlbumPropsEdit()
{
    d->albumModificationHelper->slotAlbumEdit(d->albumManager->currentPAlbum());
}

void ItemIconView::slotAlbumWriteMetadata()
{
    Album* const album = d->albumManager->currentAlbums().first();

    if (!album)
    {
        return;
    }

    MetadataSynchronizer* const tool = new MetadataSynchronizer(AlbumList() << album, MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->start();
}

void ItemIconView::slotAlbumReadMetadata()
{
    Album* const album = d->albumManager->currentAlbums().first();

    if (!album)
    {
        return;
    }

    MetadataSynchronizer* const tool = new MetadataSynchronizer(AlbumList() << album, MetadataSynchronizer::ReadFromFileToDatabase);
    tool->start();
}

void ItemIconView::slotImageWriteMetadata()
{
    const ItemInfoList selected     = selectedInfoList(ApplicationSettings::Metadata);
    MetadataSynchronizer* const tool = new MetadataSynchronizer(selected, MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->start();
}

void ItemIconView::slotImageReadMetadata()
{
    const ItemInfoList selected     = selectedInfoList(ApplicationSettings::Metadata);
    MetadataSynchronizer* const tool = new MetadataSynchronizer(selected, MetadataSynchronizer::ReadFromFileToDatabase);
    tool->start();
}

// ----------------------------------------------------------------

void ItemIconView::slotEscapePreview()
{
    if (viewMode() == StackedView::IconViewMode  ||
        viewMode() == StackedView::MapWidgetMode ||
        viewMode() == StackedView::TableViewMode ||
        viewMode() == StackedView::WelcomePageMode)
    {
        return;
    }

    // pass a null image info, because we want to fall back to the old
    // view mode
    slotTogglePreviewMode(ItemInfo());
}

void ItemIconView::slotMapWidgetView()
{
    d->stackedview->setViewMode(StackedView::MapWidgetMode);
}

void ItemIconView::slotTableView()
{
    d->stackedview->setViewMode(StackedView::TableViewMode);
}

void ItemIconView::slotIconView()
{
    if (viewMode() == StackedView::PreviewImageMode)
    {
        emit signalThumbSizeChanged(d->thumbSize);
    }

    // and switch to icon view
    d->stackedview->setViewMode(StackedView::IconViewMode);

    // make sure the next/previous buttons are updated
    slotImageSelected();
}

void ItemIconView::slotImagePreview()
{
    slotTogglePreviewMode(currentInfo());
}

/**
 * @brief This method toggles between AlbumView/MapWidgetView and ImagePreview modes, depending on the context.
 */
void ItemIconView::slotTogglePreviewMode(const ItemInfo& info)
{
    if ((viewMode() == StackedView::IconViewMode   ||
         viewMode() == StackedView::TableViewMode  ||
         viewMode() == StackedView::MapWidgetMode) && !info.isNull())
    {
        if (info.isLocationAvailable())
        {
            d->lastViewMode = viewMode();

            if (viewMode() == StackedView::IconViewMode)
            {
                d->stackedview->setPreviewItem(info, d->iconView->previousInfo(info), d->iconView->nextInfo(info));
            }
            else
            {
                d->stackedview->setPreviewItem(info, ItemInfo(), ItemInfo());
            }
        }
        else
        {
            QModelIndex index = d->iconView->indexForInfo(info);
            d->iconView->showIndexNotification(index,
                                               i18nc("@info", "<i>The storage location of this image<br/>is currently not available</i>"));
        }
    }
    else
    {
        // go back to either AlbumViewMode or MapWidgetMode
        d->stackedview->setViewMode(d->lastViewMode);
    }

    // make sure the next/previous buttons are updated
    slotImageSelected();
}

void ItemIconView::slotViewModeChanged()
{
    toggleZoomActions();

    switch (viewMode())
    {
        case StackedView::IconViewMode:
            emit signalSwitchedToIconView();
            emit signalThumbSizeChanged(d->thumbSize);
            break;
        case StackedView::PreviewImageMode:
            emit signalSwitchedToPreview();
            slotZoomFactorChanged(d->stackedview->zoomFactor());
            break;
        case StackedView::WelcomePageMode:
            emit signalSwitchedToIconView();
            break;
        case StackedView::MediaPlayerMode:
            emit signalSwitchedToPreview();
            break;
        case StackedView::MapWidgetMode:
            emit signalSwitchedToMapView();
            //TODO: connect map view's zoom buttons to main status bar zoom buttons
            break;
        case StackedView::TableViewMode:
            emit signalSwitchedToTableView();
            emit signalThumbSizeChanged(d->thumbSize);
            break;
        case StackedView::TrashViewMode:
            emit signalSwitchedToTrashView();
            break;
    }
}

void ItemIconView::slotImageFindSimilar()
{
    const ItemInfo current = currentInfo();

    if (!current.isNull())
    {
        d->fuzzySearchSideBar->newSimilarSearch(current);
        slotLeftSideBarActivate(d->fuzzySearchSideBar);
    }
}

void ItemIconView::slotImageScanForFaces()
{
    FaceScanSettings settings;

    settings.accuracy               = ApplicationSettings::instance()->getFaceDetectionAccuracy();
    settings.recognizeAlgorithm     = RecognitionDatabase::RecognizeAlgorithm::LBP;
    settings.task                   = FaceScanSettings::DetectAndRecognize;
    settings.alreadyScannedHandling = FaceScanSettings::Rescan;
    settings.infos                  = selectedInfoList(ApplicationSettings::Tools);

    FacesDetector* const tool = new FacesDetector(settings);

    connect(tool, SIGNAL(signalComplete()),
            this, SLOT(slotRefreshImagePreview()));

    tool->start();
}

void ItemIconView::slotRefreshImagePreview()
{
    if (viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedview->imagePreviewView()->reload();
    }
}

void ItemIconView::slotEditor()
{
    const ItemInfoList imageInfoList = selectedInfoList(ApplicationSettings::Tools);
    ItemInfo singleInfo              = currentInfo();

    if (singleInfo.isNull() && !imageInfoList.isEmpty())
    {
        singleInfo = imageInfoList.first();
    }

    Album* const current = currentAlbum();
    d->utilities->openInfos(singleInfo, imageInfoList, current);
}

void ItemIconView::slotFileWithDefaultApplication()
{
    d->utilities->openInfosWithDefaultApplication(selectedInfoList(ApplicationSettings::Tools));
}

void ItemIconView::slotLightTable()
{
    bool grouping = selectedNeedGroupResolving(ApplicationSettings::LightTable);
    const ItemInfoList selectedList = selectedInfoList(false, grouping);

    if (selectedList.isEmpty())
    {
        grouping = allNeedGroupResolving(ApplicationSettings::LightTable);
    }

    const ItemInfoList allInfoList  = allInfo(grouping);
    const ItemInfo currentItemInfo = currentInfo();

    d->utilities->insertToLightTableAuto(allInfoList, selectedList, currentItemInfo);
}

void ItemIconView::slotQueueMgr()
{
    bool grouping = selectedNeedGroupResolving(ApplicationSettings::BQM);
    ItemInfoList imageInfoList = selectedInfoList(false, grouping);
    ItemInfo     singleInfo    = currentInfo();

    if (singleInfo.isNull() && !imageInfoList.isEmpty())
    {
        singleInfo = imageInfoList.first();
    }

    if (singleInfo.isNull())
    {
        grouping = allNeedGroupResolving(ApplicationSettings::BQM);
        const ItemInfoList allItems = allInfo(grouping);

        if (!allItems.isEmpty())
        {
            singleInfo = allItems.first();
        }
    }

    d->utilities->insertToQueueManager(imageInfoList, singleInfo, true);
}

void ItemIconView::slotImageEdit()
{
    // Where is the difference to slotEditor?
    slotEditor();
}

void ItemIconView::slotImageLightTable()
{
    const ItemInfoList selectedList = selectedInfoList(ApplicationSettings::LightTable);
    const ItemInfo currentItemInfo = currentInfo();

    // replace images in light table
    d->utilities->insertToLightTable(selectedList, currentItemInfo, false);
}

void ItemIconView::slotImageAddToLightTable()
{
    const ItemInfoList selectedList = selectedInfoList(ApplicationSettings::LightTable);
    const ItemInfo currentItemInfo = currentInfo();

    // add to images in light table
    d->utilities->insertToLightTable(selectedList, currentItemInfo, true);
}

void ItemIconView::slotImageAddToCurrentQueue()
{
    const ItemInfoList selectedList = selectedInfoList(ApplicationSettings::BQM);
    const ItemInfo currentItemInfo = currentInfo();

    d->utilities->insertToQueueManager(selectedList, currentItemInfo, false);
}

void ItemIconView::slotImageAddToNewQueue()
{
    const bool newQueue = QueueMgrWindow::queueManagerWindowCreated() &&
                    !QueueMgrWindow::queueManagerWindow()->queuesMap().isEmpty();

    const ItemInfoList selectedList = selectedInfoList(ApplicationSettings::BQM);
    const ItemInfo currentItemInfo = currentInfo();

    d->utilities->insertToQueueManager(selectedList, currentItemInfo, newQueue);
}

void ItemIconView::slotImageAddToExistingQueue(int queueid)
{
    const ItemInfoList selectedList = selectedInfoList(ApplicationSettings::BQM);
    const ItemInfo currentItemInfo = currentInfo();

    if (!selectedList.isEmpty())
    {
        d->utilities->insertSilentToQueueManager(selectedList, currentItemInfo, queueid);
    }
}

void ItemIconView::slotImageRename()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->rename();
            break;

        default:
            d->iconView->rename();
    }
}

void ItemIconView::slotImageDelete()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->slotDeleteSelected(ItemViewUtilities::DeleteUseTrash);
            break;

        default:
            d->iconView->deleteSelected(ItemViewUtilities::DeleteUseTrash);
    }
}

void ItemIconView::slotImageDeletePermanently()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->slotDeleteSelected(ItemViewUtilities::DeletePermanently);
            break;

        default:
            d->iconView->deleteSelected(ItemViewUtilities::DeletePermanently);
    }
}

void ItemIconView::slotImageDeletePermanentlyDirectly()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->slotDeleteSelectedWithoutConfirmation(ItemViewUtilities::DeletePermanently);
            break;

        default:
            d->iconView->deleteSelectedDirectly(ItemViewUtilities::DeletePermanently);
    }
}

void ItemIconView::slotImageTrashDirectly()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->slotDeleteSelectedWithoutConfirmation(ItemViewUtilities::DeleteUseTrash);
            break;

        default:
            d->iconView->deleteSelectedDirectly(ItemViewUtilities::DeleteUseTrash);
    }
}

void ItemIconView::slotSelectAll()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->selectAll();
            break;

        default:
            d->iconView->selectAll();
    }
}

void ItemIconView::slotSelectNone()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->clearSelection();
            break;

        default:
            d->iconView->clearSelection();
    }
}

void ItemIconView::slotSelectInvert()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->invertSelection();
            break;

        default:
            d->iconView->invertSelection();
    }
}

void ItemIconView::slotSortImages(int sortRole)
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSortOrder(sortRole);
    d->iconView->imageFilterModel()->setSortRole((ItemSortSettings::SortRole) sortRole);
    settings->emitSetupChanged();
}

void ItemIconView::slotSortImagesOrder(int order)
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSorting(order);
    d->iconView->imageFilterModel()->setSortOrder((ItemSortSettings::SortOrder) order);
    settings->emitSetupChanged();
}

void ItemIconView::slotSeparateImages(int categoryMode)
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSeparationMode(categoryMode);
    d->iconView->imageFilterModel()->setCategorizationMode((ItemSortSettings::CategorizationMode) categoryMode);
}

void ItemIconView::slotImageSeparationSortOrder(int order)
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSeparationSortOrder(order);
    d->iconView->imageFilterModel()->setCategorizationSortOrder((ItemSortSettings::SortOrder) order);
}

void ItemIconView::slotMoveSelectionToAlbum()
{
    d->utilities->createNewAlbumForInfos(selectedInfoList(false, true),
                                         currentAlbum());
}

void ItemIconView::slotImagePaste()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->slotPaste();
            break;

        default:
            d->iconView->paste();
    }
}


void ItemIconView::slotLeftSidebarChangedTab(QWidget* w)
{
    // TODO update, temporary cast
    SidebarWidget* const widget = dynamic_cast<SidebarWidget*>(w);

    foreach (SidebarWidget* const sideBarWidget, d->leftSideBarWidgets)
    {
        bool active = (widget && (widget == sideBarWidget));
        sideBarWidget->setActive(active);
    }
}

void ItemIconView::toggleTag(int tagID)
{
    ItemInfoList tagToRemove, tagToAssign;
    const ItemInfoList selectedList = selectedInfoList(ApplicationSettings::Metadata);

    foreach (const ItemInfo& info, selectedList)
    {
        if (info.tagIds().contains(tagID))
            tagToRemove.append(info);
        else
            tagToAssign.append(info);
    }

    FileActionMngr::instance()->assignTag(tagToAssign, tagID);
    FileActionMngr::instance()->removeTag(tagToRemove, tagID);
}

void ItemIconView::slotAssignPickLabel(int pickId)
{
    FileActionMngr::instance()->assignPickLabel(selectedInfoList(ApplicationSettings::Metadata), pickId);
}

void ItemIconView::slotAssignColorLabel(int colorId)
{
    FileActionMngr::instance()->assignColorLabel(selectedInfoList(ApplicationSettings::Metadata), colorId);
}

void ItemIconView::slotAssignRating(int rating)
{
    FileActionMngr::instance()->assignRating(selectedInfoList(ApplicationSettings::Metadata), rating);
}

void ItemIconView::slotAssignTag(int tagID)
{
    FileActionMngr::instance()->assignTags(selectedInfoList(ApplicationSettings::Metadata), QList<int>() << tagID);
}

void ItemIconView::slotRemoveTag(int tagID)
{
    FileActionMngr::instance()->removeTags(selectedInfoList(ApplicationSettings::Metadata), QList<int>() << tagID);
}

void ItemIconView::slotSlideShowAll()
{
    slideShow(allInfo(ApplicationSettings::Slideshow));
}

void ItemIconView::slotSlideShowSelection()
{
    slideShow(selectedInfoList(ApplicationSettings::Slideshow));
}

void ItemIconView::slotSlideShowRecursive()
{
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    Album* album            = nullptr;

    if (!albumList.isEmpty())
    {
        album = albumList.first();
    }

    if (album)
    {
        SlideShowBuilder* const builder = new SlideShowBuilder(album);

        connect(builder, SIGNAL(signalComplete(SlideShowSettings)),
                this, SLOT(slotSlideShowBuilderComplete(SlideShowSettings)));

        builder->run();
    }
}

void ItemIconView::slotSlideShowManualFromCurrent()
{
    slotSlideShowManualFrom(currentInfo());
}

void ItemIconView::slotSlideShowManualFrom(const ItemInfo& info)
{
   SlideShowBuilder* const builder
           = new SlideShowBuilder(allInfo(ApplicationSettings::Slideshow));
   builder->setOverrideStartFrom(info);
   builder->setAutoPlayEnabled(false);

   connect(builder, SIGNAL(signalComplete(SlideShowSettings)),
           this, SLOT(slotSlideShowBuilderComplete(SlideShowSettings)));

   builder->run();
}

void ItemIconView::slideShow(const ItemInfoList& infoList)
{
    SlideShowBuilder* const builder = new SlideShowBuilder(infoList);

    connect(builder, SIGNAL(signalComplete(SlideShowSettings)),
            this, SLOT(slotSlideShowBuilderComplete(SlideShowSettings)));

    builder->run();
}

void ItemIconView::slotSlideShowBuilderComplete(const SlideShowSettings& settings)
{
    QPointer<Digikam::SlideShow> slide = new SlideShow(new DBInfoIface(this, QList<QUrl>()), settings);
    TagsActionMngr::defaultManager()->registerActionsToWidget(slide);

    if (settings.imageUrl.isValid())
    {
        slide->setCurrentItem(settings.imageUrl);
    }
    else if (settings.startWithCurrent)
    {
        slide->setCurrentItem(currentInfo().fileUrl());
    }

    connect(slide, SIGNAL(signalRatingChanged(QUrl,int)),
            this, SLOT(slotRatingChanged(QUrl,int)));

    connect(slide, SIGNAL(signalColorLabelChanged(QUrl,int)),
            this, SLOT(slotColorLabelChanged(QUrl,int)));

    connect(slide, SIGNAL(signalPickLabelChanged(QUrl,int)),
            this, SLOT(slotPickLabelChanged(QUrl,int)));

    connect(slide, SIGNAL(signalToggleTag(QUrl,int)),
            this, SLOT(slotToggleTag(QUrl,int)));

    connect(slide, SIGNAL(signalLastItemUrl(QUrl)),
            d->iconView, SLOT(setCurrentUrl(QUrl)));

    slide->show();
}

void ItemIconView::toggleShowBar(bool b)
{
    d->stackedview->thumbBarDock()->showThumbBar(b);

    // See bug #319876 : force to reload current view mode to set thumbbar visibility properly.
    d->stackedview->setViewMode(viewMode());
}

void ItemIconView::setRecurseAlbums(bool recursive)
{
    d->iconView->imageAlbumModel()->setRecurseAlbums(recursive);
}

void ItemIconView::setRecurseTags(bool recursive)
{
    d->iconView->imageAlbumModel()->setRecurseTags(recursive);
}

void ItemIconView::slotSidebarTabTitleStyleChanged()
{
    d->leftSideBar->setStyle(ApplicationSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->setStyle(ApplicationSettings::instance()->getSidebarTitleStyle());

    /// @todo Which settings actually have to be reloaded?
    //     d->rightSideBar->applySettings();
}

void ItemIconView::slotImageChangeFailed(const QString& message, const QStringList& fileNames)
{
    if (fileNames.isEmpty())
    {
        return;
    }

    DMessageBox::showInformationList(QMessageBox::Critical,
                                     qApp->activeWindow(),
                                     qApp->applicationName(),
                                     message,
                                     fileNames);
}

void ItemIconView::slotLeftSideBarActivateAlbums()
{
    d->leftSideBar->setActiveTab(d->albumFolderSideBar);
}

void ItemIconView::slotLeftSideBarActivateTags()
{
    d->leftSideBar->setActiveTab(d->tagViewSideBar);
}

void ItemIconView::slotLeftSideBarActivate(SidebarWidget* widget)
{
    d->leftSideBar->setActiveTab(widget);
}

void ItemIconView::slotLeftSideBarActivate(QWidget* widget)
{
    slotLeftSideBarActivate(static_cast<SidebarWidget*>(widget));
}

void ItemIconView::slotRightSideBarActivateTitles()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->setFocusToTitlesEdit();
}

void ItemIconView::slotRightSideBarActivateComments()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->setFocusToCommentsEdit();
}

void ItemIconView::slotRightSideBarActivateAssignedTags()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->activateAssignedTagsButton();
}

void ItemIconView::slotRatingChanged(const QUrl& url, int rating)
{
    rating = qMin(RatingMax, qMax(RatingMin, rating));
    ItemInfo info = ItemInfo::fromUrl(url);

    if (!info.isNull())
    {
        FileActionMngr::instance()->assignRating(info, rating);
    }
}

void ItemIconView::slotColorLabelChanged(const QUrl& url, int color)
{
    ItemInfo info = ItemInfo::fromUrl(url);

    if (!info.isNull())
    {
        FileActionMngr::instance()->assignColorLabel(info, color);
    }
}

void ItemIconView::slotPickLabelChanged(const QUrl& url, int pick)
{
    ItemInfo info = ItemInfo::fromUrl(url);

    if (!info.isNull())
    {
        FileActionMngr::instance()->assignPickLabel(info, pick);
    }
}

void ItemIconView::slotToggleTag(const QUrl& url, int tagID)
{
    ItemInfo info = ItemInfo::fromUrl(url);

    if (!info.isNull())
    {
        if (info.tagIds().contains(tagID))
            FileActionMngr::instance()->removeTag(info, tagID);
        else
            FileActionMngr::instance()->assignTag(info, tagID);
    }
}

bool ItemIconView::hasCurrentItem() const
{
    return !currentInfo().isNull();
}

void ItemIconView::slotFocusAndNextImage()
{
    //slot is called on pressing "return" a second time after assigning a tag
    d->stackedview->currentWidget()->setFocus();

    //select next image, since the user is probably done tagging the current image
    slotNextItem();
}

void ItemIconView::slotImageExifOrientation(int orientation)
{
    FileActionMngr::instance()->setExifOrientation(
                selectedInfoList(ApplicationSettings::Metadata), orientation);
}

void ItemIconView::imageTransform(MetaEngineRotation::TransformationAction transform)
{
    FileActionMngr::instance()->transform(
                selectedInfoList(ApplicationSettings::Metadata), transform);
}

ItemInfo ItemIconView::currentInfo() const
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            return d->tableView->currentInfo();

#ifdef HAVE_MARBLE
        case StackedView::MapWidgetMode:
            return d->mapView->currentItemInfo();
#endif // HAVE_MARBLE

        case StackedView::MediaPlayerMode:
        case StackedView::PreviewImageMode:
        case StackedView::IconViewMode:
            // all of these modes use the same selection model and data as the IconViewMode
            return d->iconView->currentInfo();

        default:
            return ItemInfo();
    }
}

Album* ItemIconView::currentAlbum() const
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            return d->tableView->currentAlbum();

        case StackedView::PreviewImageMode:
        case StackedView::MediaPlayerMode:
        case StackedView::MapWidgetMode:
        case StackedView::IconViewMode:
            // all of these modes use the same selection model and data as the IconViewMode
            return d->iconView->currentAlbum();
        default:
            return nullptr;
    }
}

ItemInfoList ItemIconView::selectedInfoList(const bool currentFirst,
                                            const bool grouping) const
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:

            if (currentFirst)
            {
                return d->tableView->selectedItemInfosCurrentFirst(grouping);
            }

            return d->tableView->selectedItemInfos(grouping);

        case StackedView::PreviewImageMode:
        case StackedView::MediaPlayerMode:
        case StackedView::MapWidgetMode:
        case StackedView::IconViewMode:

            // all of these modes use the same selection model and data as the IconViewMode
            if (currentFirst)
            {
                return d->iconView->selectedItemInfosCurrentFirst(grouping);
            }

            return d->iconView->selectedItemInfos(grouping);

        default:

            return ItemInfoList();
    }
}

ItemInfoList ItemIconView::selectedInfoList(const ApplicationSettings::OperationType type,
                                            const bool currentFirst) const
{
    return selectedInfoList(currentFirst, selectedNeedGroupResolving(type));
}

ItemInfoList ItemIconView::allInfo(const bool grouping) const
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            return d->tableView->allItemInfos(grouping);

        case StackedView::MapWidgetMode:
        case StackedView::PreviewImageMode:
        case StackedView::MediaPlayerMode:
        case StackedView::IconViewMode:
            // all of these modes use the same selection model and data as the IconViewMode
            return d->iconView->allItemInfos(grouping);

        default:
            return ItemInfoList();
    }
}

ItemInfoList ItemIconView::allInfo(const ApplicationSettings::OperationType type) const
{
    return allInfo(allNeedGroupResolving(type));
}

bool ItemIconView::allNeedGroupResolving(const ApplicationSettings::OperationType type) const
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            return d->tableView->allNeedGroupResolving(type);
        case StackedView::MapWidgetMode:
        case StackedView::PreviewImageMode:
        case StackedView::MediaPlayerMode:
        case StackedView::IconViewMode:
            // all of these modes use the same selection model and data as the IconViewMode
            return d->iconView->allNeedGroupResolving(type);
        default:
            return false;
    }
}

bool ItemIconView::selectedNeedGroupResolving(const ApplicationSettings::OperationType type) const
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            return d->tableView->selectedNeedGroupResolving(type);
        case StackedView::MapWidgetMode:
        case StackedView::PreviewImageMode:
        case StackedView::MediaPlayerMode:
        case StackedView::IconViewMode:
            // all of these modes use the same selection model and data as the IconViewMode
            return d->iconView->selectedNeedGroupResolving(type);
        default:
            return false;
    }
}

QUrl ItemIconView::currentUrl() const
{
    const ItemInfo cInfo = currentInfo();

    return cInfo.fileUrl();
}

void ItemIconView::slotSetCurrentWhenAvailable(const qlonglong id)
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->slotSetCurrentWhenAvailable(id);
            break;

        default:
            d->iconView->setCurrentWhenAvailable(id);
    }
}

void ItemIconView::slotAwayFromSelection()
{
    switch (viewMode())
    {
        case StackedView::TableViewMode:
            d->tableView->slotAwayFromSelection();
            break;

        default:
            d->iconView->awayFromSelection();
    }
}

StackedView::StackedViewMode ItemIconView::viewMode() const
{
    return d->stackedview->viewMode();
}

void ItemIconView::slotSetupMetadataFilters(int tab)
{
    Setup::execMetadataFilters(this, tab);
}

void ItemIconView::toggleFullScreen(bool set)
{
    d->stackedview->imagePreviewView()->toggleFullScreen(set);
}

void ItemIconView::setToolsIconView(DCategorizedView* const view)
{
    d->rightSideBar->appendTab(view,
                               QIcon::fromTheme(QLatin1String("document-edit")),
                               i18n("Tools"));
}

void ItemIconView::slotShowContextMenu(QContextMenuEvent* event,
                                      const QList<QAction*>& extraGroupingActions)
{
    Album* const album = currentAlbum();

    if (!album          ||
        album->isRoot() ||
        (album->type() != Album::PHYSICAL && album->type() != Album::TAG))
    {
        return;
    }

    QMenu menu(this);
    ContextMenuHelper cmHelper(&menu);

    cmHelper.addAction(QLatin1String("full_screen"));
    cmHelper.addAction(QLatin1String("options_show_menubar"));
    cmHelper.addSeparator();
    // --------------------------------------------------------
    cmHelper.addStandardActionPaste(this, SLOT(slotImagePaste()));
    // --------------------------------------------------------
    if (!extraGroupingActions.isEmpty())
    {
        cmHelper.addSeparator();
        cmHelper.addGroupMenu(QList<qlonglong>(), extraGroupingActions);
    }

    cmHelper.exec(event->globalPos());
}

void ItemIconView::slotShowContextMenuOnInfo(QContextMenuEvent* event, const ItemInfo& info,
                                            const QList<QAction*>& extraGroupingActions,
                                            ItemFilterModel* imageFilterModel)
{
    QList<qlonglong> selectedImageIds = selectedInfoList(true, true).toImageIdList();

    // --------------------------------------------------------
    QMenu menu(this);
    ContextMenuHelper cmHelper(&menu);
    cmHelper.setItemFilterModel(imageFilterModel);

    cmHelper.addAction(QLatin1String("full_screen"));
    cmHelper.addAction(QLatin1String("options_show_menubar"));
    cmHelper.addSeparator();

    // --------------------------------------------------------

    QAction* const viewAction = new QAction(i18nc("View the selected image", "Preview"), this);
    viewAction->setIcon(QIcon::fromTheme(QLatin1String("view-preview")));
    viewAction->setEnabled(selectedImageIds.count() == 1);
    cmHelper.addAction(viewAction);

    cmHelper.addOpenAndNavigateActions(selectedImageIds);
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addAction(QLatin1String("image_scan_for_faces"));
    cmHelper.addAction(QLatin1String("image_find_similar"));
    cmHelper.addStandardActionLightTable();
    cmHelper.addQueueManagerMenu();
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addAction(QLatin1String("image_rotate"));
    cmHelper.addAction(QLatin1String("cut_album_selection"));
    cmHelper.addAction(QLatin1String("copy_album_selection"));
    cmHelper.addAction(QLatin1String("paste_album_selection"));
    cmHelper.addAction(QLatin1String("image_rename"));
    cmHelper.addStandardActionItemDelete(this, SLOT(slotImageDelete()), selectedImageIds.count());
    cmHelper.addSeparator();

    // --------------------------------------------------------

    cmHelper.addStandardActionThumbnail(selectedImageIds, currentAlbum());
    cmHelper.addAssignTagsMenu(selectedImageIds);
    cmHelper.addRemoveTagsMenu(selectedImageIds);
    cmHelper.addLabelsAction();

    if (d->leftSideBar->getActiveTab() != d->peopleSideBar)
    {
        cmHelper.addSeparator();

        cmHelper.addGroupMenu(selectedImageIds, extraGroupingActions);
    }

    // special action handling --------------------------------

    connect(&cmHelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabel(int)));

    connect(&cmHelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabel(int)));

    connect(&cmHelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    connect(&cmHelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(slotAssignTag(int)));

    connect(&cmHelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTag(int)));

    connect(&cmHelper, SIGNAL(signalPopupTagsView()),
            d->rightSideBar, SLOT(slotPopupTagsView()));

    connect(&cmHelper, SIGNAL(signalGotoTag(int)),
            this, SLOT(slotGotoTagAndItem(int)));

    connect(&cmHelper, SIGNAL(signalGotoTag(int)),
            d->albumHistory, SLOT(slotClearSelectTAlbum(int)));

    connect(&cmHelper, SIGNAL(signalGotoAlbum(ItemInfo)),
            this, SLOT(slotGotoAlbumAndItem(ItemInfo)));

    connect(&cmHelper, SIGNAL(signalGotoAlbum(ItemInfo)),
            d->albumHistory, SLOT(slotClearSelectPAlbum(ItemInfo)));

    connect(&cmHelper, SIGNAL(signalGotoDate(ItemInfo)),
            this, SLOT(slotGotoDateAndItem(ItemInfo)));

    connect(&cmHelper, SIGNAL(signalSetThumbnail(ItemInfo)),
            this, SLOT(slotSetAsAlbumThumbnail(ItemInfo)));

    connect(&cmHelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SLOT(slotImageAddToExistingQueue(int)));

    connect(&cmHelper, SIGNAL(signalCreateGroup()),
            this, SLOT(slotCreateGroupFromSelection()));

    connect(&cmHelper, SIGNAL(signalCreateGroupByTime()),
            this, SLOT(slotCreateGroupByTimeFromSelection()));

    connect(&cmHelper, SIGNAL(signalCreateGroupByFilename()),
            this, SLOT(slotCreateGroupByFilenameFromSelection()));

    connect(&cmHelper, SIGNAL(signalCreateGroupByTimelapse()),
            this, SLOT(slotCreateGroupByTimelapseFromSelection()));

    connect(&cmHelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(slotRemoveSelectedFromGroup()));

    connect(&cmHelper, SIGNAL(signalUngroup()),
            this, SLOT(slotUngroupSelected()));

    // --------------------------------------------------------

    QAction* const choice = cmHelper.exec(event->globalPos());

    if (choice && (choice == viewAction))
    {
        slotTogglePreviewMode(info);
    }
}

void ItemIconView::slotShowGroupContextMenu(QContextMenuEvent* event,
                                           const QList<ItemInfo>& selectedInfos,
                                           ItemFilterModel* imageFilterModel)
{
    QList<qlonglong> selectedImageIDs;

    foreach (const ItemInfo& info, selectedInfos)
    {
        selectedImageIDs << info.id();
    }

    QMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.setItemFilterModel(imageFilterModel);
    cmhelper.addGroupActions(selectedImageIDs);

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalCreateGroup()),
            this, SLOT(slotCreateGroupFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByTime()),
            this, SLOT(slotCreateGroupByTimeFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByFilename()),
            this, SLOT(slotCreateGroupByFilenameFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByTimelapse()),
            this, SLOT(slotCreateGroupByTimelapseFromSelection()));

    connect(&cmhelper, SIGNAL(signalUngroup()),
            this, SLOT(slotUngroupSelected()));

    connect(&cmhelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(slotRemoveSelectedFromGroup()));

    cmhelper.exec(event->globalPos());
}

void ItemIconView::slotSetAsAlbumThumbnail(const ItemInfo& info)
{
    d->utilities->setAsAlbumThumbnail(currentAlbum(), info);
}

void ItemIconView::slotCreateGroupFromSelection()
{
    FileActionMngr::instance()->addToGroup(currentInfo(), selectedInfoList(false, true));
}

void ItemIconView::slotCreateGroupByTimeFromSelection()
{
    d->utilities->createGroupByTimeFromInfoList(selectedInfoList(false, true));
}

void ItemIconView::slotCreateGroupByFilenameFromSelection()
{
    d->utilities->createGroupByFilenameFromInfoList(selectedInfoList(false, true));
}

void ItemIconView::slotCreateGroupByTimelapseFromSelection()
{
    d->utilities->createGroupByTimelapseFromInfoList(selectedInfoList(false, true));
}

void ItemIconView::slotRemoveSelectedFromGroup()
{
    FileActionMngr::instance()->removeFromGroup(selectedInfoList(false, true));
}

void ItemIconView::slotUngroupSelected()
{
    FileActionMngr::instance()->ungroup(selectedInfoList(false, true));
}

} // namespace Digikam
