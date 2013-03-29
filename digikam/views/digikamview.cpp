/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : implementation of album view interface.
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2011-2013 by Michael G. Hansen <mike at mghansen dot de>
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

#include "digikamview.moc"

// Qt includes

#include <QShortcut>

// KDE includes

#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>
#include <krun.h>

// Local includes

#include "albumhistory.h"
#include "albumsettings.h"
#include "stackedview.h"
#include "metadatasynchronizer.h"
#include "digikamapp.h"
#include "digikamimageview.h"
#include "dzoombar.h"
#include "imagealbummodel.h"
#include "imagedescedittab.h"
#include "imagepreviewview.h"
#include "imagepropertiessidebardb.h"
#include "imagethumbnailbar.h"
#include "imageviewutilities.h"
#include "filterstatusbar.h"
#include "leftsidebarwidgets.h"
#include "loadingcacheinterface.h"
#include "mapwidgetview.h"
#include "mediaplayerview.h"
#include "metadatasettings.h"
#include "globals.h"
#include "metadatahub.h"
#include "fileactionmngr.h"
#include "queuemgrwindow.h"
#include "scancontroller.h"
#include "sidebar.h"
#include "slideshow.h"
#include "slideshowbuilder.h"
#include "statusprogressbar.h"
#include "filtersidebarwidget.h"
#include "tagmodificationhelper.h"
#include "imagepropertiesversionstab.h"
#include "tagscache.h"
#include "searchxml.h"
#include "faceiface.h"
#include "fileactionprogress.h"
#include "versionmanagersettings.h"
#include "tableview.h"

#ifdef USE_PRESENTATION_MODE
#include "qmlshow.h"
#endif // USE_PRESENTATION_MODE

namespace Digikam
{

class DigikamView::Private
{
public:

    Private() :
        needDispatchSelection(false),
        useAlbumHistory(false),
        initialAlbumID(0),
        thumbSize(ThumbnailSize::Medium),
        dockArea(0),
        splitter(0),
        selectionTimer(0),
        thumbSizeTimer(0),
        albumFolderSideBar(0),
        tagViewSideBar(0),
        dateViewSideBar(0),
        timelineSideBar(0),
        searchSideBar(0),
        fuzzySearchSideBar(0),
        gpsSearchSideBar(0),
        peopleSideBar(0),
        parent(0),
        iconView(0),
        mapView(0),
        tableView(0),
        albumManager(0),
        albumHistory(0),
        stackedview(0),
        lastViewMode(StackedView::IconViewMode),
        albumModificationHelper(0),
        tagModificationHelper(0),
        searchModificationHelper(0),
        leftSideBar(0),
        rightSideBar(0),
        filterWidget(0),
        optionAlbumViewPrefix("AlbumView"),
        modelCollection(0)
    {
    }

    QString userPresentableAlbumTitle(const QString& album) const;
    void    addPageUpDownActions(DigikamView* const q, QWidget* const w);

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
    DateFolderViewSideBarWidget*  dateViewSideBar;
    TimelineSideBarWidget*        timelineSideBar;
    SearchSideBarWidget*          searchSideBar;
    FuzzySearchSideBarWidget*     fuzzySearchSideBar;

    GPSSearchSideBarWidget*       gpsSearchSideBar;

    PeopleSideBarWidget*          peopleSideBar;

    DigikamApp*                   parent;

    DigikamImageView*             iconView;
    MapWidgetView*                mapView;
    TableView*                    tableView;
    AlbumManager*                 albumManager;
    AlbumHistory*                 albumHistory;
    StackedView*                  stackedview;
    StackedView::StackedViewMode  lastViewMode;

    AlbumModificationHelper*      albumModificationHelper;
    TagModificationHelper*        tagModificationHelper;
    SearchModificationHelper*     searchModificationHelper;

    Sidebar*                      leftSideBar;
    ImagePropertiesSideBarDB*     rightSideBar;

    FilterSideBarWidget*          filterWidget;

    QString                       optionAlbumViewPrefix;

    QList<SidebarWidget*>         leftSideBarWidgets;

    DigikamModelCollection*       modelCollection;
};

QString DigikamView::Private::userPresentableAlbumTitle(const QString& title) const
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

void DigikamView::Private::addPageUpDownActions(DigikamView* const q, QWidget* const w)
{
    QShortcut* const nextImageShortcut = new QShortcut(w);
    nextImageShortcut->setKey(Qt::Key_PageDown);
    nextImageShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(nextImageShortcut, SIGNAL(activated()), q, SLOT(slotNextItem()));

    QShortcut* const prevImageShortcut = new QShortcut(w);
    prevImageShortcut->setKey(Qt::Key_PageUp);
    prevImageShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(prevImageShortcut, SIGNAL(activated()), q, SLOT(slotPrevItem()));
}

// -------------------------------------------------------------------------------------------

DigikamView::DigikamView(QWidget* const parent, DigikamModelCollection* const modelCollection)
    : KHBox(parent), d(new Private)
{
    qRegisterMetaType<SlideShowSettings>("SlideShowSettings");

    d->parent                   = static_cast<DigikamApp*>(parent);
    d->modelCollection          = modelCollection;
    d->albumManager             = AlbumManager::instance();

    d->albumModificationHelper  = new AlbumModificationHelper(this, this);
    d->tagModificationHelper    = new TagModificationHelper(this, this);
    d->searchModificationHelper = new SearchModificationHelper(this, this);

    d->splitter    = new SidebarSplitter;
    d->splitter->setFrameStyle( QFrame::NoFrame );
    d->splitter->setFrameShadow( QFrame::Plain );
    d->splitter->setFrameShape( QFrame::NoFrame );
    d->splitter->setOpaqueResize(false);

    d->leftSideBar = new Sidebar(this, d->splitter, KMultiTabBar::Left);
    d->leftSideBar->setObjectName("Digikam Left Sidebar");
    d->splitter->setParent(this);

    // The dock area where the thumbnail bar is allowed to go.
    d->dockArea    = new QMainWindow(this, Qt::Widget);
    d->splitter->addWidget(d->dockArea);
    d->stackedview = new StackedView(d->dockArea);
    d->dockArea->setCentralWidget(d->stackedview);
    d->stackedview->setDockArea(d->dockArea);

    d->iconView = d->stackedview->imageIconView();
    d->mapView  = d->stackedview->mapWidgetView();
    d->tableView = d->stackedview->tableView();

    d->addPageUpDownActions(this, d->stackedview->imagePreviewView());
    d->addPageUpDownActions(this, d->stackedview->thumbBar());
    d->addPageUpDownActions(this, d->stackedview->mediaPlayerView());

    d->rightSideBar = new ImagePropertiesSideBarDB(this, d->splitter, KMultiTabBar::Right, true);
    d->rightSideBar->setObjectName("Digikam Right Sidebar");

    // album folder view
    d->albumFolderSideBar = new AlbumFolderViewSideBarWidget(d->leftSideBar,
                                                             d->modelCollection->getAlbumModel(),
                                                             d->albumModificationHelper);
    d->leftSideBarWidgets << d->albumFolderSideBar;

    connect(d->albumFolderSideBar, SIGNAL(signalFindDuplicatesInAlbum(Album*)),
            this, SLOT(slotNewDuplicatesSearch(Album*)));

    // Tags sidebar tab contents.
    d->tagViewSideBar = new TagViewSideBarWidget(d->leftSideBar, d->modelCollection->getTagModel());
    d->leftSideBarWidgets << d->tagViewSideBar;

    connect(d->tagViewSideBar, SIGNAL(signalFindDuplicatesInAlbum(Album*)),
            this, SLOT(slotNewDuplicatesSearch(Album*)));

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

    d->gpsSearchSideBar = new GPSSearchSideBarWidget(d->leftSideBar,
                                                     d->modelCollection->getSearchModel(),
                                                     d->searchModificationHelper,
                                                     d->iconView->imageFilterModel(),d->iconView->getSelectionModel());

    d->leftSideBarWidgets << d->gpsSearchSideBar;

    // People Sidebar
    d->peopleSideBar = new PeopleSideBarWidget(d->leftSideBar,
                                               d->modelCollection->getTagFacesModel(),
                                               d->searchModificationHelper);

    connect(d->peopleSideBar, SIGNAL(requestFaceMode(bool)),
            d->iconView, SLOT(setFaceMode(bool)));

    d->leftSideBarWidgets << d->peopleSideBar;

    foreach(SidebarWidget* const leftWidget, d->leftSideBarWidgets)
    {
        d->leftSideBar->appendTab(leftWidget, leftWidget->getIcon(), leftWidget->getCaption());

        connect(leftWidget, SIGNAL(requestActiveTab(SidebarWidget*)),
                this, SLOT(slotLeftSideBarActivate(SidebarWidget*)));
    }

    // To the right.

    d->addPageUpDownActions(this, d->rightSideBar->imageDescEditTab());

    // Tags Filter sidebar tab contents.
    d->filterWidget = new FilterSideBarWidget(d->rightSideBar, d->modelCollection->getTagFilterModel());
    d->rightSideBar->appendTab(d->filterWidget, SmallIcon("view-filter"), i18n("Filters"));

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
}

DigikamView::~DigikamView()
{
    saveViewState();

    delete d->albumHistory;
    delete d;
}

void DigikamView::applySettings()
{
    foreach(SidebarWidget* const sidebarWidget, d->leftSideBarWidgets)
    {
        sidebarWidget->applySettings();
    }

    d->iconView->imageFilterModel()->setVersionImageFilterSettings(AlbumSettings::instance()->getVersionManagerSettings());

    refreshView();
}

void DigikamView::refreshView()
{
    d->rightSideBar->refreshTagsView();
}

void DigikamView::setupConnections()
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
            d->iconView, SLOT(paste()));

    // -- AlbumManager connections --------------------------------

    connect(d->albumManager, SIGNAL(signalAlbumCurrentChanged(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

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

    connect(d->iconView, SIGNAL(previewRequested(ImageInfo)),
            this, SLOT(slotTogglePreviewMode(ImageInfo)));

    connect(d->iconView, SIGNAL(gotoAlbumAndImageRequested(ImageInfo)),
            this, SLOT(slotGotoAlbumAndItem(ImageInfo)));

    connect(d->iconView, SIGNAL(gotoDateAndImageRequested(ImageInfo)),
            this, SLOT(slotGotoDateAndItem(ImageInfo)));

    connect(d->iconView, SIGNAL(gotoTagAndImageRequested(int)),
            this, SLOT(slotGotoTagAndItem(int)));

    connect(d->iconView, SIGNAL(zoomOutStep()),
            this, SLOT(slotZoomOut()));

    connect(d->iconView, SIGNAL(zoomInStep()),
            this, SLOT(slotZoomIn()));

    connect(d->iconView, SIGNAL(signalPopupTagsView()),
            d->rightSideBar, SLOT(slotPopupTagsView()));

    // -- TableView Connections -----------------------------------

    connect(d->tableView, SIGNAL(signalPreviewRequested(ImageInfo)),
            this, SLOT(slotTogglePreviewMode(ImageInfo)));

    connect(d->tableView, SIGNAL(signalZoomOutStep()),
            this, SLOT(slotZoomOut()));

    connect(d->tableView, SIGNAL(signalZoomInStep()),
            this, SLOT(slotZoomIn()));

    connect(d->tableView, SIGNAL(signalPopupTagsView()),
            d->rightSideBar, SLOT(slotPopupTagsView()));

    connect(d->tableView, SIGNAL(signalGotoAlbumAndImageRequested(ImageInfo)),
            this, SLOT(slotGotoAlbumAndItem(ImageInfo)));

    connect(d->tableView, SIGNAL(signalGotoDateAndImageRequested(ImageInfo)),
            this, SLOT(slotGotoDateAndItem(ImageInfo)));

    connect(d->tableView, SIGNAL(signalGotoTagAndImageRequested(int)),
            this, SLOT(slotGotoTagAndItem(int)));

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

    connect(d->gpsSearchSideBar, SIGNAL(signalMapSoloItems(QList<qlonglong>,QString)),
            d->iconView->imageFilterModel(), SLOT(setIdWhitelist(QList<qlonglong>,QString)));

    // -- Filter Bars Connections ---------------------------------

    ImageAlbumFilterModel* const model = d->iconView->imageAlbumFilterModel();

    connect(d->filterWidget,
            SIGNAL(signalTagFilterChanged(const QList<int>&, const QList<int>&,
                                          ImageFilterSettings::MatchingCondition, bool, const QList<int>&, const QList<int>&)),
            d->iconView->imageFilterModel(),
            SLOT(setTagFilter(const QList<int>&, const QList<int>&,
                              ImageFilterSettings::MatchingCondition, bool, const QList<int>&, const QList<int>&)));

    connect(d->filterWidget, SIGNAL(signalRatingFilterChanged(int,ImageFilterSettings::RatingCondition)),
            model, SLOT(setRatingFilter(int,ImageFilterSettings::RatingCondition)));

    connect(d->filterWidget, SIGNAL(signalSearchTextFilterChanged(SearchTextFilterSettings)),
            model, SLOT(setTextFilter(SearchTextFilterSettings)));

    connect(model, SIGNAL(filterMatchesForText(bool)),
            d->filterWidget, SLOT(slotFilterMatchesForText(bool)));

    connect(d->filterWidget, SIGNAL(signalMimeTypeFilterChanged(int)),
            model, SLOT(setMimeTypeFilter(int)));

    connect(d->filterWidget, SIGNAL(signalGeolocationFilterChanged(ImageFilterSettings::GeolocationCondition)),
            model, SLOT(setGeolocationFilter(ImageFilterSettings::GeolocationCondition)));

    // -- Preview image widget Connections ------------------------

    connect(d->stackedview, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->stackedview, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->stackedview, SIGNAL(signalEditItem()),
            this, SLOT(slotImageEdit()));

    connect(d->stackedview, SIGNAL(signalDeleteItem()),
            this, SLOT(slotImageDelete()));

    connect(d->stackedview, SIGNAL(signalViewModeChanged()),
            this, SLOT(slotViewModeChanged()));

    connect(d->stackedview, SIGNAL(signalEscapePreview()),
            this, SLOT(slotEscapePreview()));

    connect(d->stackedview, SIGNAL(signalSlideShow()),
            this, SLOT(slotSlideShowAll()));

    connect(d->stackedview, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    connect(d->stackedview, SIGNAL(signalInsert2LightTable()),
            this, SLOT(slotImageAddToLightTable()));

    connect(d->stackedview, SIGNAL(signalInsert2QueueMgr()),
            this, SLOT(slotImageAddToCurrentQueue()));

    connect(d->stackedview, SIGNAL(signalFindSimilar()),
            this, SLOT(slotImageFindSimilar()));

    connect(d->stackedview, SIGNAL(signalAddToExistingQueue(int)),
            this, SLOT(slotImageAddToExistingQueue(int)));

    connect(d->stackedview, SIGNAL(signalGotoAlbumAndItem(ImageInfo)),
            this, SLOT(slotGotoAlbumAndItem(ImageInfo)));

    connect(d->stackedview, SIGNAL(signalGotoDateAndItem(ImageInfo)),
            this, SLOT(slotGotoDateAndItem(ImageInfo)));

    connect(d->stackedview, SIGNAL(signalGotoTagAndItem(int)),
            this, SLOT(slotGotoTagAndItem(int)));

    // -- FileActionMngr progress ---------------

    connect(FileActionMngr::instance(), SIGNAL(signalImageChangeFailed(QString,QStringList)),
            this, SLOT(slotImageChangeFailed(QString,QStringList)));

    // -- timers ---------------

    connect(d->selectionTimer, SIGNAL(timeout()),
            this, SLOT(slotDispatchImageSelected()));

    connect(d->thumbSizeTimer, SIGNAL(timeout()),
            this, SLOT(slotThumbSizeEffect()) );

    // -- Album Settings ----------------

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSidebarTabTitleStyleChanged()));

    // -- Album History -----------------

    connect(this, SIGNAL(signalAlbumSelected(bool)),
            d->albumHistory, SLOT(slotAlbumSelected()));

    connect(this, SIGNAL(signalImageSelected(ImageInfoList,bool,bool,ImageInfoList)),
            d->albumHistory, SLOT(slotImageSelected(ImageInfoList)));

    connect(d->iconView, SIGNAL(currentChanged(ImageInfo)),
            d->albumHistory, SLOT(slotCurrentChange(ImageInfo)));

    connect(d->iconView, SIGNAL(gotoAlbumAndImageRequested(ImageInfo)),
            d->albumHistory, SLOT(slotClearSelectPAlbum(ImageInfo)));

    connect(d->iconView, SIGNAL(gotoTagAndImageRequested(int)),
            d->albumHistory, SLOT(slotClearSelectTAlbum(int)));

    connect(d->iconView->imageModel(), SIGNAL(imageInfosAdded(QList<ImageInfo>)),
            d->albumHistory, SLOT(slotAlbumCurrentChanged()));

    connect(d->albumHistory, SIGNAL(signalSetCurrent(qlonglong)),
            d->iconView, SLOT(setCurrentWhenAvailable(qlonglong)));

    connect(d->albumHistory, SIGNAL(signalSetSelectedInfos(QList<ImageInfo>)),
            d->iconView, SLOT(setSelectedImageInfos(QList<ImageInfo>)));

    connect(d->albumManager, SIGNAL(signalAlbumDeleted(Album*)),
            d->albumHistory, SLOT(slotAlbumDeleted(Album*)));

    connect(d->albumManager, SIGNAL(signalAlbumsCleared()),
            d->albumHistory, SLOT(slotAlbumsCleared()));

    // -- Image versions ----------------

    connect(d->rightSideBar->getFiltersHistoryTab(), SIGNAL(imageSelected(ImageInfo)),
            d->iconView, SLOT(hintAt(ImageInfo)));

    connect(d->rightSideBar->getFiltersHistoryTab(), SIGNAL(actionTriggered(ImageInfo)),
            this, SLOT(slotGotoAlbumAndItem(ImageInfo)));
}

void DigikamView::connectIconViewFilter(FilterStatusBar* const filterbar)
{
    ImageAlbumFilterModel* const model = d->iconView->imageAlbumFilterModel();

    connect(model, SIGNAL(filterMatches(bool)),
            filterbar, SLOT(slotFilterMatches(bool)));

    connect(model, SIGNAL(filterSettingsChanged(ImageFilterSettings)),
            filterbar, SLOT(slotFilterSettingsChanged(ImageFilterSettings)));

    connect(filterbar, SIGNAL(signalResetFilters()),
            d->filterWidget, SLOT(slotResetFilters()));

    connect(filterbar, SIGNAL(signalPopupFiltersView()),
            this, SLOT(slotPopupFiltersView()));
}

void DigikamView::slotPopupFiltersView()
{
    d->rightSideBar->setActiveTab(d->filterWidget);
    d->filterWidget->setFocusToTextFilter();
}

void DigikamView::loadViewState()
{
    foreach(SidebarWidget* const widget, d->leftSideBarWidgets)
    {
        widget->loadState();
    }

    d->filterWidget->loadState();

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("MainWindow");

    // Restore the splitter
    d->splitter->restoreState(group);

    // Restore the thumbnail bar dock.
    QByteArray thumbbarState;
    thumbbarState     = group.readEntry("ThumbbarState", thumbbarState);
    d->dockArea->restoreState(QByteArray::fromBase64(thumbbarState));

    d->initialAlbumID = group.readEntry("InitialAlbumID", 0);

    d->mapView->loadState();
    d->tableView->loadState();
    d->rightSideBar->loadState();
}

void DigikamView::saveViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("MainWindow");

    foreach(SidebarWidget* const widget, d->leftSideBarWidgets)
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
    group.writeEntry("ThumbbarState", d->dockArea->saveState().toBase64());

    Album* const album = AlbumManager::instance()->currentAlbum();

    if (album)
    {
        group.writeEntry("InitialAlbumID", album->globalID());
    }
    else
    {
        group.writeEntry("InitialAlbumID", 0);
    }

    d->mapView->saveState();
    d->tableView->saveState();
    d->rightSideBar->saveState();
}

QList<SidebarWidget*> DigikamView::leftSidebarWidgets() const
{
    return d->leftSideBarWidgets;
}

KUrl::List DigikamView::allUrls() const
{
    return d->iconView->urls();
}

KUrl::List DigikamView::selectedUrls() const
{
    return d->iconView->selectedUrls();
}

void DigikamView::showSideBars()
{
    d->leftSideBar->restore();
    d->rightSideBar->restore();
}

void DigikamView::hideSideBars()
{
    d->leftSideBar->backup();
    d->rightSideBar->backup();
}

void DigikamView::slotFirstItem()
{
    d->iconView->toFirstIndex();
}

void DigikamView::slotPrevItem()
{
    d->iconView->toPreviousIndex();
}

void DigikamView::slotNextItem()
{
    d->iconView->toNextIndex();
}

void DigikamView::slotLastItem()
{
    d->iconView->toLastIndex();
}

void DigikamView::slotSelectItemByUrl(const KUrl& url)
{
    d->iconView->toIndex(url);
}

void DigikamView::slotAllAlbumsLoaded()
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
    d->albumManager->setCurrentAlbum(album);
}

void DigikamView::slotSortAlbums(int order)
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setAlbumSortOrder((AlbumSettings::AlbumSortOrder) order);
    // TODO sorting by anything else then the name is currently not supported by the model
    //d->folderView->resort();
}

void DigikamView::slotNewAlbum()
{
    // TODO use the selection model of the view instead
    d->albumModificationHelper->slotAlbumNew(d->albumFolderSideBar->currentAlbum());
}

void DigikamView::slotDeleteAlbum()
{
    d->albumModificationHelper->slotAlbumDelete(d->albumFolderSideBar->currentAlbum());
}

void DigikamView::slotNewTag()
{
    d->tagModificationHelper->slotTagNew(d->tagViewSideBar->currentAlbum());
}

void DigikamView::slotDeleteTag()
{
    d->tagModificationHelper->slotTagDelete(d->tagViewSideBar->currentAlbum());
}

void DigikamView::slotEditTag()
{
    d->tagModificationHelper->slotTagEdit(d->tagViewSideBar->currentAlbum());
}

void DigikamView::slotNewKeywordSearch()
{
    slotLeftSideBarActivate(d->searchSideBar);
    d->searchSideBar->newKeywordSearch();
}

void DigikamView::slotNewAdvancedSearch()
{
    slotLeftSideBarActivate(d->searchSideBar);
    d->searchSideBar->newAdvancedSearch();
}

void DigikamView::slotNewDuplicatesSearch(Album* album)
{
    slotLeftSideBarActivate(d->fuzzySearchSideBar);
    d->fuzzySearchSideBar->newDuplicatesSearch(album);
}

void DigikamView::slotAlbumsCleared()
{
    emit signalAlbumSelected(false);
}

void DigikamView::slotAlbumHistoryBack(int steps)
{
    Album* album    = 0;
    QWidget* widget = 0;

    d->albumHistory->back(&album, &widget, steps);

    changeAlbumFromHistory(album, widget);
}

void DigikamView::slotAlbumHistoryForward(int steps)
{
    Album* album    = 0;
    QWidget* widget = 0;

    d->albumHistory->forward(&album, &widget, steps);

    changeAlbumFromHistory(album, widget);
}

// TODO update, use SideBarWidget instead of QWidget
void DigikamView::changeAlbumFromHistory(Album* const album, QWidget* const widget)
{
    if (album && widget)
    {
        // TODO update, temporary casting until signature is changed
        SidebarWidget* const sideBarWidget = dynamic_cast<SidebarWidget*>(widget);

        if (sideBarWidget)
        {
            sideBarWidget->changeAlbumFromHistory(album);
            slotLeftSideBarActivate(sideBarWidget);
        }

        d->parent->enableAlbumBackwardHistory(d->useAlbumHistory && !d->albumHistory->isBackwardEmpty());
        d->parent->enableAlbumForwardHistory(d->useAlbumHistory && !d->albumHistory->isForwardEmpty());
    }
}

void DigikamView::clearHistory()
{
    d->albumHistory->clearHistory();
    d->parent->enableAlbumBackwardHistory(false);
    d->parent->enableAlbumForwardHistory(false);
}

void DigikamView::getBackwardHistory(QStringList& titles)
{
    d->albumHistory->getBackwardHistory(titles);

    for (int i = 0; i < titles.size(); ++i)
    {
        titles[i] = d->userPresentableAlbumTitle(titles.at(i));
    }
}

void DigikamView::getForwardHistory(QStringList& titles)
{
    d->albumHistory->getForwardHistory(titles);

    for (int i = 0; i < titles.size(); ++i)
    {
        titles[i] = d->userPresentableAlbumTitle(titles.at(i));
    }
}

void DigikamView::slotGotoAlbumAndItem(const ImageInfo& imageInfo)
{

    kDebug() << "going to " << imageInfo;

    emit signalNoCurrentItem();

    PAlbum* const album = AlbumManager::instance()->findPAlbum(imageInfo.albumId());

    d->albumFolderSideBar->setCurrentAlbum(album);
    slotLeftSideBarActivate(d->albumFolderSideBar);

    // Set the activate item url to find in the Album View after
    // all items have be reloaded.
    d->iconView->setCurrentWhenAvailable(imageInfo.id());

    // And finally toggle album manager to handle album history and
    // reload all items.
    d->albumManager->setCurrentAlbum(album);

}

void DigikamView::slotGotoDateAndItem(const ImageInfo& imageInfo)
{
    QDate date = imageInfo.dateTime().date();

    emit signalNoCurrentItem();

    // Change to Date Album view.
    // Note, that this also opens the side bar if it is closed; this is
    // considered as a feature, because it highlights that the view was changed.
    slotLeftSideBarActivate(d->dateViewSideBar);

    // Set the activate item url to find in the Album View after
    // all items have be reloaded.
    d->iconView->setCurrentWhenAvailable(imageInfo.id());

    // Change the year and month of the iconItem (day is unused).
    d->dateViewSideBar->gotoDate(date);
}

void DigikamView::slotGotoTagAndItem(int tagID)
{
    // FIXME: Arnd: don't know yet how to get the iconItem passed through ...
    //  then we would know how to use the following ...
    //  KURL url( iconItem->imageInfo()->kurl() );
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
        kError() << "Could not find a tag album for tag id " << tagID;
    }

    // Set the activate item url to find in the Tag View after
    // all items have be reloaded.
    // FIXME: see above
    // d->iconView->setAlbumItemToFind(url);
}

void DigikamView::slotSelectAlbum(const KUrl& url)
{
    PAlbum* const album = d->albumManager->findPAlbum(url);

    if (!album)
    {
        kWarning() << "Unable to find album for " << url;
        return;
    }

    slotLeftSideBarActivate(d->albumFolderSideBar);
    d->albumFolderSideBar->setCurrentAlbum(album);
}

void DigikamView::slotAlbumSelected(Album* album)
{
    emit signalNoCurrentItem();

    if (!album)
    {
        d->iconView->openAlbum(0);
        d->mapView->openAlbum(0);
        emit signalAlbumSelected(false);
        emit signalTagSelected(false);
        slotTogglePreviewMode(ImageInfo());
        return;
    }

    if (album->type() == Album::PHYSICAL)
    {
        emit signalAlbumSelected(true);
        emit signalTagSelected(false);
    }
    else if (album->type() == Album::TAG)
    {
        emit signalAlbumSelected(false);
/*
        kDebug()<<"Album "<<album->title()<<" selected." ;

        // Now loop through children of the people album and check if this album is a child.
        Album* peopleAlbum = AlbumManager::instance()->findTAlbum(TagsCache::instance()->tagForPath("/People"));
        int thisAlbumId = album->id();

        QList<int> children =  peopleAlbum->childAlbumIds(true);

        foreach(int id, children)
        {
            if(id == thisAlbumId)
            {
                kDebug()<<"Is a people tag";

                showFaceAlbum(thisAlbumId);
                emit signalTagSelected(true);
                return;
            }
        }
*/
        emit signalTagSelected(true);
    }
    else
    {
        emit signalAlbumSelected(false);
        emit signalTagSelected(false);
    }

    if (d->useAlbumHistory)
    {
        d->albumHistory->addAlbum(album, d->leftSideBar->getActiveTab());
    }

    d->parent->enableAlbumBackwardHistory(d->useAlbumHistory && !d->albumHistory->isBackwardEmpty());
    d->parent->enableAlbumForwardHistory(d->useAlbumHistory && !d->albumHistory->isForwardEmpty());

    d->iconView->openAlbum(album);

    if (album->isRoot())
    {
        d->stackedview->setViewMode(StackedView::WelcomePageMode);
    }
    else
    {
        switch (d->stackedview->viewMode())
        {
            case StackedView::PreviewImageMode:
            case StackedView::MediaPlayerMode:
            case StackedView::WelcomePageMode:
                slotTogglePreviewMode(ImageInfo());
                break;
            default:
                break;
        }
    }
}

void DigikamView::slotAlbumOpenInFileManager()
{
    Album* const album = d->albumManager->currentAlbum();

    if (!album || album->type() != Album::PHYSICAL)
    {
        return;
    }

    if (album->isRoot())
    {
        KMessageBox::error(this, i18n("Cannot open the root. It is not a physical location."));
        return;
    }

    PAlbum* const palbum = dynamic_cast<PAlbum*>(album);

    if (palbum)
    {
        new KRun(KUrl(palbum->folderPath()), this); // KRun will delete itself.
    }
}

void DigikamView::slotAlbumOpenInTerminal()
{
    Album* const album = d->albumManager->currentAlbum();

    if (!album || album->type() != Album::PHYSICAL)
    {
        return;
    }

    if (album->isRoot())
    {
        KMessageBox::error(this, i18n("Cannot open the root. It is not a physical location."));
        return;
    }

    PAlbum* const palbum = dynamic_cast<PAlbum*>(album);

    if (!palbum)
    {
        return;
    }

    QString dir(palbum->folderPath());

    // If the given directory is not local, it can still be the URL of an
    // ioslave using UDS_LOCAL_PATH which to be converted first.
    KUrl url = KIO::NetAccess::mostLocalUrl(dir, this);

    //If the URL is local after the above conversion, set the directory.
    if (url.isLocalFile())
    {
        dir = url.toLocalFile();
    }

    KToolInvocation::invokeTerminal(QString(), dir);
}

void DigikamView::slotAlbumRefresh()
{
    // force reloading of thumbnails
    LoadingCacheInterface::cleanThumbnailCache();
    Album* const album = d->iconView->currentAlbum();

    // if physical album, schedule a collection scan of current album's path
    if (album && album->type() == Album::PHYSICAL)
    {
        ScanController::instance()->scheduleCollectionScan(static_cast<PAlbum*>(album)->folderPath());
    }

    // force reload. Should normally not be necessary, but we may have bugs
    qlonglong currentId = d->iconView->currentInfo().id();
    d->iconView->imageAlbumModel()->refresh();

    if (currentId != -1)
    {
        d->iconView->setCurrentWhenAvailable(currentId);
    }
}

void DigikamView::slotImageSelected()
{
    // delay to slotDispatchImageSelected
    d->needDispatchSelection = true;
    d->selectionTimer->start();
    emit signalSelectionChanged(d->iconView->numberOfSelectedIndexes());
}

void DigikamView::slotDispatchImageSelected()
{
    if (d->needDispatchSelection)
    {
        // the list of ImageInfos of currently selected items, currentItem first
        // since the iconView tracks the changes also while we are in map widget mode,
        // we can still pull the data from the iconView
        const ImageInfoList list      = d->iconView->selectedImageInfosCurrentFirst();
        const ImageInfoList allImages = d->iconView->imageInfos();

        if (list.isEmpty())
        {
            d->stackedview->setPreviewItem();
            emit signalImageSelected(list, false, false, allImages);
            emit signalNoCurrentItem();
        }
        else
        {
            d->rightSideBar->itemChanged(list);

            ImageInfo previousInfo;
            ImageInfo nextInfo;

            if ( (d->stackedview->viewMode() != StackedView::MapWidgetMode) &&
                 (d->stackedview->viewMode() != StackedView::TableViewMode) )
            {
                /// @todo next/previous should also be available in TableViewMode!
                previousInfo = d->iconView->previousInfo(list.first());
                nextInfo     = d->iconView->nextInfo(list.first());
            }

            if ((d->stackedview->viewMode() != StackedView::IconViewMode) &&
                (d->stackedview->viewMode() != StackedView::MapWidgetMode) &&
                (d->stackedview->viewMode() != StackedView::TableViewMode) )
            {
                d->stackedview->setPreviewItem(list.first(), previousInfo, nextInfo);
            }

            emit signalImageSelected(list, !previousInfo.isNull(), !nextInfo.isNull(), allImages);
        }

        d->needDispatchSelection = false;
    }
}

double DigikamView::zoomMin() const
{
    return d->stackedview->zoomMin();
}

double DigikamView::zoomMax() const
{
    return d->stackedview->zoomMax();
}

void DigikamView::setZoomFactor(double zoom)
{
    d->stackedview->setZoomFactorSnapped(zoom);
}

void DigikamView::slotZoomFactorChanged(double zoom)
{
    toggleZoomActions();
    emit signalZoomChanged(zoom);
}

void DigikamView::setThumbSize(int size)
{
    if (d->stackedview->viewMode() == StackedView::PreviewImageMode)
    {
        double z = DZoomBar::zoomFromSize(size, zoomMin(), zoomMax());
        setZoomFactor(z);
    }
    else if (   (d->stackedview->viewMode() == StackedView::IconViewMode)
             || (d->stackedview->viewMode() == StackedView::TableViewMode) )
    {
        if (size > ThumbnailSize::Huge)
        {
            d->thumbSize = ThumbnailSize::Huge;
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

void DigikamView::slotThumbSizeEffect()
{
    d->iconView->setThumbnailSize(d->thumbSize);
    d->tableView->setThumbnailSize(d->thumbSize);
    toggleZoomActions();

    AlbumSettings::instance()->setDefaultIconSize(d->thumbSize);
}

void DigikamView::toggleZoomActions()
{
    if (d->stackedview->viewMode() == StackedView::PreviewImageMode)
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
    else if (   (d->stackedview->viewMode() == StackedView::IconViewMode)
             || (d->stackedview->viewMode() == StackedView::TableViewMode) )
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);

        if (d->thumbSize >= ThumbnailSize::Huge)
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

void DigikamView::slotZoomIn()
{
    if (   (d->stackedview->viewMode() == StackedView::IconViewMode)
        || (d->stackedview->viewMode() == StackedView::TableViewMode) )
    {
        setThumbSize(d->thumbSize + ThumbnailSize::Step);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }
    else if (d->stackedview->viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedview->increaseZoom();
    }
}

void DigikamView::slotZoomOut()
{
    if (   (d->stackedview->viewMode() == StackedView::IconViewMode)
        || (d->stackedview->viewMode() == StackedView::TableViewMode) )
    {
        setThumbSize(d->thumbSize - ThumbnailSize::Step);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }
    else if (d->stackedview->viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedview->decreaseZoom();
    }
}

void DigikamView::slotZoomTo100Percents()
{
    if (d->stackedview->viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedview->toggleFitToWindowOr100();
    }
}

void DigikamView::slotFitToWindow()
{
    if (d->stackedview->viewMode() == StackedView::TableViewMode)
    {
        /// @todo We should choose an appropriate thumbnail size here
    }
    else if (d->stackedview->viewMode() == StackedView::IconViewMode)
    {
        int nts = d->iconView->fitToWidthIcons();
        kDebug() << "new thumb size = " << nts;
        setThumbSize(nts);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }
    else if (d->stackedview->viewMode() == StackedView::PreviewImageMode)
    {
        d->stackedview->fitToWindow();
    }
}

void DigikamView::slotAlbumPropsEdit()
{
    d->albumModificationHelper->slotAlbumEdit(d->albumManager->currentPAlbum());
}

void DigikamView::slotAlbumWriteMetadata()
{
    Album* const album = d->albumManager->currentAlbum();

    if (!album)
    {
        return;
    }
    MetadataSynchronizer* const tool = new MetadataSynchronizer(album, MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->start();
}

void DigikamView::slotAlbumReadMetadata()
{
    Album* const album = d->albumManager->currentAlbum();

    if (!album)
    {
        return;
    }
    MetadataSynchronizer* const tool = new MetadataSynchronizer(album, MetadataSynchronizer::ReadFromFileToDatabase);
    tool->start();
}

void DigikamView::slotImageWriteMetadata()
{
    ImageInfoList selected           = d->iconView->selectedImageInfos();
    MetadataSynchronizer* const tool = new MetadataSynchronizer(selected, MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->start();
}

void DigikamView::slotImageReadMetadata()
{
    ImageInfoList selected           = d->iconView->selectedImageInfos();
    MetadataSynchronizer* const tool = new MetadataSynchronizer(selected, MetadataSynchronizer::ReadFromFileToDatabase);
    tool->start();
}

// ----------------------------------------------------------------

void DigikamView::slotEscapePreview()
{
    if (d->stackedview->viewMode() == StackedView::IconViewMode ||
        d->stackedview->viewMode() == StackedView::WelcomePageMode)
    {
        return;
    }

    // pass a null image info, because we want to fall back to the old
    // view mode
    slotTogglePreviewMode(ImageInfo());
}

void DigikamView::slotMapWidgetView()
{
    d->stackedview->setViewMode(StackedView::MapWidgetMode);
}

void DigikamView::slotTableView()
{
    d->stackedview->setViewMode(StackedView::TableViewMode);
}

void DigikamView::slotIconView()
{
    if (d->stackedview->viewMode() == StackedView::PreviewImageMode)
    {
        emit signalThumbSizeChanged(d->iconView->thumbnailSize().size());
    }

    // and switch to icon view
    d->stackedview->setViewMode(StackedView::IconViewMode);

    // make sure the next/previous buttons are updated
    slotImageSelected();
}

void DigikamView::slotImagePreview()
{
    const int currentPreviewMode = d->stackedview->viewMode();
    ImageInfo currentInfo;

    if (    (currentPreviewMode == StackedView::IconViewMode)
         || (currentPreviewMode == StackedView::TableViewMode) )
    {
        currentInfo = d->iconView->currentInfo();
    }
    else if (currentPreviewMode == StackedView::MapWidgetMode)
    {
        currentInfo = d->mapView->currentImageInfo();
    }

    slotTogglePreviewMode(currentInfo);
}

/**
 * @brief This method toggles between AlbumView/MapWidgetView and ImagePreview modes, depending on the context.
 */
void DigikamView::slotTogglePreviewMode(const ImageInfo& info)
{
    if ( (d->stackedview->viewMode() == StackedView::IconViewMode ||
          d->stackedview->viewMode() == StackedView::TableViewMode ||
          d->stackedview->viewMode() == StackedView::MapWidgetMode)   &&
         !info.isNull() )
    {
        d->lastViewMode = d->stackedview->viewMode();

        if (d->stackedview->viewMode() == StackedView::IconViewMode)
        {
            d->stackedview->setPreviewItem(info, d->iconView->previousInfo(info), d->iconView->nextInfo(info));
        }
        else
        {
            d->stackedview->setPreviewItem(info, ImageInfo(), ImageInfo());
        }
    }
    else
    {
        // go back to either AlbumViewMode or MapWidgetMode
        d->stackedview->setViewMode( d->lastViewMode );
    }

    // make sure the next/previous buttons are updated
    slotImageSelected();
}

void DigikamView::slotViewModeChanged()
{
    toggleZoomActions();

    switch (d->stackedview->viewMode())
    {
        case StackedView::IconViewMode:
            emit signalSwitchedToIconView();
            emit signalThumbSizeChanged(d->iconView->thumbnailSize().size());
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
            break;
    }
}

void DigikamView::slotImageFindSimilar()
{
    ImageInfo current = d->iconView->currentInfo();

    if (!current.isNull())
    {
        d->fuzzySearchSideBar->newSimilarSearch(current);
        slotLeftSideBarActivate(d->fuzzySearchSideBar);
    }
}

void DigikamView::slotEditor()
{
    d->iconView->openEditor();
}

void DigikamView::slotLightTable()
{
    d->iconView->setOnLightTable();
}

void DigikamView::slotQueueMgr()
{
    d->iconView->insertToQueue();
}

void DigikamView::slotImageEdit()
{
    d->iconView->openCurrentInEditor();
}

void DigikamView::slotImageLightTable()
{
    ImageInfoList selectedImageInfoList;
    ImageInfo currentImageInfo;
    if (d->stackedview->viewMode() == StackedView::IconViewMode)
    {
        /// @todo remove d->iconView->setOnLightTable
        selectedImageInfoList << d->iconView->selectedImageInfos();
        currentImageInfo = d->iconView->currentInfo();
    }
    else if (d->stackedview->viewMode() == StackedView::TableViewMode)
    {
        selectedImageInfoList << d->tableView->selectedImageInfos();
        currentImageInfo = d->tableView->currentInfo();
    }
    else
    {
        currentImageInfo = d->stackedview->imagePreviewView()->getImageInfo();
        selectedImageInfoList << currentImageInfo;
    }

    ImageInfo firstInfo = selectedImageInfoList.first();
    kDebug()<<firstInfo.id()<<currentImageInfo.id();
    d->iconView->utilities()->insertToLightTable(selectedImageInfoList, currentImageInfo, false);
}

void DigikamView::slotImageAddToLightTable()
{
    ImageInfoList selectedImageInfoList;
    ImageInfo currentImageInfo;
    if (d->stackedview->viewMode() == StackedView::IconViewMode)
    {
        /// @todo remove d->iconView->addSelectedToLightTable
        selectedImageInfoList << d->iconView->selectedImageInfos();
        currentImageInfo = d->iconView->currentInfo();
    }
    else if (d->stackedview->viewMode() == StackedView::TableViewMode)
    {
        selectedImageInfoList << d->tableView->selectedImageInfos();
        currentImageInfo = d->tableView->currentInfo();
    }
    else
    {
        currentImageInfo = d->stackedview->imagePreviewView()->getImageInfo();
        selectedImageInfoList << currentImageInfo;
    }

    ImageInfo firstInfo = selectedImageInfoList.first();
    kDebug()<<firstInfo.id()<<currentImageInfo.id();
    d->iconView->utilities()->insertToLightTable(selectedImageInfoList, currentImageInfo, true);
}

void DigikamView::slotImageAddToCurrentQueue()
{
    /// @todo Care about MapWidgetMode
    if (d->stackedview->viewMode() == StackedView::IconViewMode)
    {
        d->iconView->insertSelectedToCurrentQueue();
    }
    else
    {
        ImageInfoList list;
        ImageInfo info = d->stackedview->imagePreviewView()->getImageInfo();
        list.append(info);
        d->iconView->utilities()->insertToQueueManager(list, info, false);
    }
}

void DigikamView::slotImageAddToNewQueue()
{
    /// @todo Care about MapWidgetMode
    bool newQueue = QueueMgrWindow::queueManagerWindowCreated() &&
                    !QueueMgrWindow::queueManagerWindow()->queuesMap().isEmpty();

    if (d->stackedview->viewMode() == StackedView::IconViewMode)
    {
        if (newQueue)
        {
            d->iconView->insertSelectedToNewQueue();
        }
        else
        {
            d->iconView->insertSelectedToCurrentQueue();
        }
    }
    else
    {
        // FIXME
        ImageInfoList list;
        ImageInfo info = d->stackedview->imagePreviewView()->getImageInfo();
        list.append(info);
        d->iconView->utilities()->insertToQueueManager(list, info, newQueue);
    }
}

void DigikamView::slotImageAddToExistingQueue(int queueid)
{
    ImageInfoList list;

    if (d->stackedview->viewMode() == StackedView::IconViewMode)
    {
        list = d->stackedview->imageIconView()->selectedImageInfos();
    }
    else
    {
        list << d->stackedview->imagePreviewView()->getImageInfo();
    }

    if (!list.isEmpty())
    {
        d->iconView->utilities()->insertSilentToQueueManager(list, list.first(), queueid);
    }
}

void DigikamView::slotImageRename()
{
    d->iconView->rename();
}

void DigikamView::slotImageDelete()
{
    d->iconView->deleteSelected(false);
}

void DigikamView::slotImageDeletePermanently()
{
    d->iconView->deleteSelected(true);
}

void DigikamView::slotImageDeletePermanentlyDirectly()
{
    d->iconView->deleteSelectedDirectly(false);
}

void DigikamView::slotImageTrashDirectly()
{
    d->iconView->deleteSelectedDirectly(true);
}

void DigikamView::slotSelectAll()
{
    d->iconView->selectAll();
}

void DigikamView::slotSelectNone()
{
    d->iconView->clearSelection();
}

void DigikamView::slotSelectInvert()
{
    d->iconView->invertSelection();
}

void DigikamView::slotSortImages(int sortRole)
{
    AlbumSettings* const settings = AlbumSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSortOrder(sortRole);
    d->iconView->imageFilterModel()->setSortRole((ImageSortSettings::SortRole) sortRole);
}

void DigikamView::slotSortImagesOrder(int order)
{
    AlbumSettings* const settings = AlbumSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSorting(order);
    d->iconView->imageFilterModel()->setSortOrder((ImageSortSettings::SortOrder) order);
}

void DigikamView::slotGroupImages(int categoryMode)
{
    AlbumSettings* const settings = AlbumSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageGroupMode(categoryMode);
    d->iconView->imageFilterModel()->setCategorizationMode((ImageSortSettings::CategorizationMode) categoryMode);
}

void DigikamView::slotMoveSelectionToAlbum()
{
    d->iconView->createNewAlbumForSelected();
}

void DigikamView::slotLeftSidebarChangedTab(QWidget* w)
{
    // TODO update, temporary cast
    SidebarWidget* const widget = dynamic_cast<SidebarWidget*>(w);

    foreach(SidebarWidget* const sideBarWidget, d->leftSideBarWidgets)
    {
        bool active = (widget && (widget == sideBarWidget));
        sideBarWidget->setActive(active);
    }
}

void DigikamView::toggleTag(int tagID)
{
    d->iconView->toggleTagToSelected(tagID);
}

void DigikamView::slotAssignPickLabel(int pickId)
{
    d->iconView->assignPickLabelToSelected(pickId);
}

void DigikamView::slotAssignColorLabel(int colorId)
{
    d->iconView->assignColorLabelToSelected(colorId);
}

void DigikamView::slotAssignRating(int rating)
{
    d->iconView->assignRatingToSelected(rating);
}

void DigikamView::slotSlideShowAll()
{
    slideShow(d->iconView->imageInfos());
}

void DigikamView::slotSlideShowSelection()
{
    slideShow(d->iconView->selectedImageInfos());
}

void DigikamView::slotSlideShowRecursive()
{
    Album* const album = AlbumManager::instance()->currentAlbum();

    if (album)
    {
        SlideShowBuilder* const builder = new SlideShowBuilder(album);

        connect(builder, SIGNAL(signalComplete(SlideShowSettings)),
                this, SLOT(slotSlideShowBuilderComplete(SlideShowSettings)));
    }
}

void DigikamView::slideShow(const ImageInfoList& infoList)
{
    SlideShowBuilder* const builder = new SlideShowBuilder(infoList);

    connect(builder, SIGNAL(signalComplete(SlideShowSettings)),
            this, SLOT(slotSlideShowBuilderComplete(SlideShowSettings)));
}

void DigikamView::slotSlideShowBuilderComplete(const SlideShowSettings& settings)
{
    SlideShow* const slide = new SlideShow(settings);

    if (settings.startWithCurrent)
    {
        slide->setCurrent(d->iconView->currentUrl());
    }

    connect(slide, SIGNAL(signalRatingChanged(KUrl,int)),
            this, SLOT(slotRatingChanged(KUrl,int)));

    connect(slide, SIGNAL(signalColorLabelChanged(KUrl,int)),
            this, SLOT(slotColorLabelChanged(KUrl,int)));

    connect(slide, SIGNAL(signalPickLabelChanged(KUrl,int)),
            this, SLOT(slotPickLabelChanged(KUrl,int)));

    slide->show();
}

void DigikamView::toggleShowBar(bool b)
{
    d->stackedview->thumbBarDock()->showThumbBar(b);
}

bool DigikamView::isThumbBarVisible()
{
    return d->stackedview->thumbBarDock()->isVisible();
}

void DigikamView::setRecurseAlbums(bool recursive)
{
    d->iconView->imageAlbumModel()->setRecurseAlbums(recursive);
}

void DigikamView::setRecurseTags(bool recursive)
{
    d->iconView->imageAlbumModel()->setRecurseTags(recursive);
}

void DigikamView::slotSidebarTabTitleStyleChanged()
{
    d->leftSideBar->setStyle(AlbumSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->setStyle(AlbumSettings::instance()->getSidebarTitleStyle());

    /// @todo Which settings actually have to be reloaded?
    //     d->rightSideBar->applySettings();
}

void DigikamView::slotImageChangeFailed(const QString& message, const QStringList& fileNames)
{
    if (fileNames.isEmpty())
    {
        return;
    }

    KMessageBox::errorList(0, message, fileNames);
}

void DigikamView::slotLeftSideBarActivateAlbums()
{
    d->leftSideBar->setActiveTab(d->albumFolderSideBar);
}

void DigikamView::slotLeftSideBarActivateTags()
{
    d->leftSideBar->setActiveTab(d->tagViewSideBar);
}

void DigikamView::slotLeftSideBarActivate(SidebarWidget* widget)
{
    d->leftSideBar->setActiveTab(widget);
}

void DigikamView::slotLeftSideBarActivate(QWidget* widget)
{
    slotLeftSideBarActivate(static_cast<SidebarWidget*>(widget));
}

void DigikamView::slotRatingChanged(const KUrl& url, int rating)
{
    rating = qMin(RatingMax, qMax(RatingMin, rating));
    ImageInfo info(url);

    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setRating(rating);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void DigikamView::slotColorLabelChanged(const KUrl& url, int color)
{
    ImageInfo info(url);

    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setColorLabel(color);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void DigikamView::slotPickLabelChanged(const KUrl& url, int pick)
{
    ImageInfo info(url);

    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setPickLabel(pick);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

bool DigikamView::hasCurrentItem() const
{
    // We should actually get this directly from the selection model,
    // but the iconView is fine for now.
    return !d->iconView->currentInfo().isNull();
}

void DigikamView::slotImageExifOrientation(int orientation)
{
    FileActionMngr::instance()->setExifOrientation(d->iconView->selectedImageInfos(), orientation);
}

void DigikamView::imageTransform(RotationMatrix::TransformationAction transform)
{
    FileActionMngr::instance()->transform(d->iconView->selectedImageInfos(), transform);
}

#ifdef USE_PRESENTATION_MODE

void DigikamView::slotSlideShowQml()
{
/*
    QStringList list;
    foreach (const ImageInfo& info, d->iconView->imageInfos())
    {
        list << info.filePath();
    }
*/
    SlideShowSettings settings;
    settings.readFromConfig();
    QmlShow* const qmlShow = new QmlShow(d->iconView->imageInfos(),settings);
    qmlShow->setWindowState(Qt::WindowFullScreen);
    qmlShow->show();
}

#endif // USE_PRESENTATION_MODE

}  // namespace Digikam
