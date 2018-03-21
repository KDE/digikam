/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C)      2006 by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2014-2015 by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikamapp.h"
/// @todo Order should be changed according to krazy2, but compilation fails. Try again later. MH
#include "digikamapp_p.h"

// Qt includes

#include <QApplication>
#include <QPointer>
#include <QSignalMapper>
#include <QStringList>
#include <QDomDocument>
#include <QStandardPaths>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QTimer>
#include <QIcon>
#include <QMessageBox>
#include <QStatusBar>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <ktoolbar.h>
#include <ktoolbarpopupaction.h>
#include <kwindowsystem.h>

#include <solid/camera.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/predicate.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>

// Local includes

#include "actioncategorizedview.h"
#include "drawdecoder.h"
#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "album.h"
#include "coredb.h"
#include "albummodel.h"
#include "albumselectdialog.h"
#include "albumthumbnailloader.h"
#include "cameratype.h"
#include "dbinfoiface.h"
#include "imagegps.h"
#include "importui.h"
#include "cameranamehelper.h"
#include "categorizeditemmodel.h"
#include "collectionscanner.h"
#include "collectionmanager.h"
#include "componentsinfo.h"
#include "coredbthumbinfoprovider.h"
#include "dio.h"
#include "dlogoaction.h"
#include "fileactionmngr.h"
#include "filterstatusbar.h"
#include "iccsettings.h"
#include "imageattributeswatch.h"
#include "imageinfo.h"
#include "imagepropertiestab.h"
#include "imagewindow.h"
#include "lighttablewindow.h"
#include "queuemgrwindow.h"
#include "loadingcache.h"
#include "loadingcacheinterface.h"
#include "loadsavethread.h"
#include "metaengine_rotation.h"
#include "scancontroller.h"
#include "setup.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "thememanager.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "dmetadata.h"
#include "tagscache.h"
#include "tagsactionmngr.h"
#include "databaseserverstarter.h"
#include "metadatasettings.h"
#include "statusbarprogresswidget.h"
#include "dbmigrationdlg.h"
#include "progressmanager.h"
#include "progressview.h"
#include "maintenancedlg.h"
#include "maintenancemngr.h"
#include "newitemsfinder.h"
#include "dbcleaner.h"
#include "tagsmanager.h"
#include "imagesortsettings.h"
#include "metadatahubmngr.h"
#include "metadataedit.h"
#include "expoblendingmanager.h"
#include "calwizard.h"
#include "mailwizard.h"
#include "advprintwizard.h"
#include "dfiledialog.h"
#include "dmediaservermngr.h"
#include "dmediaserverdlg.h"
#include "dbwindow.h"
#include "fbwindow.h"
#include "flickrwindow.h"
#include "gswindow.h"
#include "imageshackwindow.h"
#include "imgurwindow.h"
#include "piwigowindow.h"
#include "rajcewindow.h"
#include "smugwindow.h"
#include "yfwindow.h"

#ifdef HAVE_MEDIAWIKI
#   include "mediawikiwindow.h"
#endif

#ifdef HAVE_VKONTAKTE
#   include "vkwindow.h"
#endif

#ifdef HAVE_KIO
#   include "ftexportwindow.h"
#   include "ftimportwindow.h"
#endif

#ifdef HAVE_MARBLE
#   include "geolocationedit.h"
#endif

#ifdef HAVE_HTMLGALLERY
#   include "htmlwizard.h"
#endif

#ifdef HAVE_DBUS
#   include "digikamadaptor.h"
#endif

#ifdef HAVE_PANORAMA
#   include "panomanager.h"
#endif

#ifdef HAVE_MEDIAPLAYER
#   include "vidslidewizard.h"
#endif

#ifdef HAVE_KFILEMETADATA
#   include "baloowrap.h"
#endif

namespace Digikam
{

DigikamApp* DigikamApp::m_instance = 0;

DigikamApp::DigikamApp()
    : DXmlGuiWindow(0),
      d(new Private)
{
    setObjectName(QLatin1String("Digikam"));
    setConfigGroupName(ApplicationSettings::instance()->generalConfigGroupName());
    setFullScreenOptions(FS_ALBUMGUI);
    setXMLFile(QLatin1String("digikamui5.rc"));

    m_instance         = this;
    d->config          = KSharedConfig::openConfig();
    KConfigGroup group = d->config->group(configGroupName());


#ifdef HAVE_DBUS
    new DigikamAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/Digikam"), this);
    QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.digikam-") +
                                                  QString::number(QCoreApplication::instance()->applicationPid()));
#endif

    // collection scan
    if (!CollectionScanner::databaseInitialScanDone())
    {
        ScanController::instance()->completeCollectionScanDeferFiles();
    }

    if (ApplicationSettings::instance()->getShowSplashScreen() &&
        !qApp->isSessionRestored())
    {
        d->splashScreen = new DSplashScreen();
        d->splashScreen->show();
    }
    else
    {
        // Windows need here QCoreApplication::processEvents().
        qApp->processEvents();
    }

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Initializing..."));
    }

    // ensure creation
    AlbumManager::instance();
    LoadingCacheInterface::initialize();
    IccSettings::instance()->loadAllProfilesProperties();
    MetadataSettings::instance();
    DMetadataSettings::instance();
    ProgressManager::instance();
    ThumbnailLoadThread::setDisplayingWidget(this);
    DIO::instance();

    // creation of the engine on first use - when drawing -
    // can take considerable time and cause a noticeable hang in the UI thread.
    QFontMetrics fm(font());
    fm.width(QLatin1String("a"));

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    connect(IccSettings::instance(), SIGNAL(settingsChanged()),
            this, SLOT(slotColorManagementOptionsChanged()));

    d->cameraMenu      = new QMenu(this);
    d->usbMediaMenu    = new QMenu(this);
    d->cardReaderMenu  = new QMenu(this);
    d->quickImportMenu = new QMenu(this);

    d->cameraList = new CameraList(this, QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1String("/cameras.xml"));

    connect(d->cameraList, SIGNAL(signalCameraAdded(CameraType*)),
            this, SLOT(slotCameraAdded(CameraType*)));

    connect(d->cameraList, SIGNAL(signalCameraRemoved(QAction*)),
            this, SLOT(slotCameraRemoved(QAction*)));

    d->modelCollection = new DigikamModelCollection;

    // This manager must be created after collection setup and before accelerators setup.
    d->tagsActionManager = new TagsActionMngr(this);

    // First create everything, then connect.
    // Otherwise some items may send signals and the slots can try
    // to access items which were not created yet.
    setupView();
    setupAccelerators();
    setupActions();
    setupStatusBar();

    initGui();

    setupViewConnections();
    applyMainWindowSettings(group);
    slotColorManagementOptionsChanged();

    // Check ICC profiles repository availability

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Checking ICC repository..."));
    }

    d->validIccPath = SetupICC::iccRepositoryIsValid();

    // Read albums from database
    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Reading database..."));
    }

    AlbumManager::instance()->startScan();
    
    // Setting the initial menu options after all tools have been loaded
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    d->view->slotAlbumSelected(albumList);

    // preload additional windows
    preloadWindows();

    readFullScreenSettings(group);

#ifdef HAVE_KFILEMETADATA
    // Create BalooWrap object, because it need to register a listener
    // to update digiKam data when changes in Baloo occur
    BalooWrap* const baloo = BalooWrap::instance();
    Q_UNUSED(baloo);
#endif //HAVE_KFILEMETADATA

    setAutoSaveSettings(group, true);

    LoadSaveThread::setInfoProvider(new DatabaseLoadSaveFileInfoProvider);

    setupSelectToolsAction();
}

DigikamApp::~DigikamApp()
{
    ProgressManager::instance()->slotAbortAll();

    ImageAttributesWatch::shutDown();

    // Close and delete image editor instance.

    if (ImageWindow::imageWindowCreated())
    {
        // Delete after close
        ImageWindow::imageWindow()->setAttribute(Qt::WA_DeleteOnClose, true);
        // close the window
        ImageWindow::imageWindow()->close();
    }

    // Close and delete light table instance.

    if (LightTableWindow::lightTableWindowCreated())
    {
        LightTableWindow::lightTableWindow()->setAttribute(Qt::WA_DeleteOnClose, true);
        LightTableWindow::lightTableWindow()->close();
    }

    // Close and delete Batch Queue Manager instance.

    if (QueueMgrWindow::queueManagerWindowCreated())
    {
        QueueMgrWindow::queueManagerWindow()->setAttribute(Qt::WA_DeleteOnClose, true);
        QueueMgrWindow::queueManagerWindow()->close();
    }

    if (TagsManager::isCreated())
    {
        TagsManager::instance()->close();
    }

    if (MetadataHubMngr::isCreated())
    {
        delete MetadataHubMngr::internalPtr;
    }

#ifdef HAVE_KFILEMETADATA
    if (BalooWrap::isCreated())
    {
        BalooWrap::internalPtr.clear();
    }
#endif

    if (ExpoBlendingManager::isCreated())
    {
        delete ExpoBlendingManager::internalPtr;
    }

#ifdef HAVE_PANORAMA
    if (PanoManager::isCreated())
    {
        delete PanoManager::internalPtr;
    }
#endif

    delete d->view;

    ApplicationSettings::instance()->setRecurseAlbums(d->recurseAlbumsAction->isChecked());
    ApplicationSettings::instance()->setRecurseTags(d->recurseTagsAction->isChecked());
    ApplicationSettings::instance()->setShowThumbbar(d->showBarAction->isChecked());
    ApplicationSettings::instance()->saveSettings();

    ScanController::instance()->shutDown();
    AlbumManager::instance()->cleanUp();
    ImageAttributesWatch::cleanUp();
    ThumbnailLoadThread::cleanUp();
    AlbumThumbnailLoader::instance()->cleanUp();
    LoadingCacheInterface::cleanUp();
    DIO::cleanUp();
    DMediaServerMngr::instance()->saveAtShutdown();

    // close database server
    if (ApplicationSettings::instance()->getDbEngineParameters().internalServer)
    {
        DatabaseServerStarter::instance()->stopServerManagerProcess();
    }

    AlbumManager::instance()->removeFakeConnection();

    m_instance = 0;

    delete d->modelCollection;

    delete d;
}

DigikamApp* DigikamApp::instance()
{
    return m_instance;
}

DigikamView* DigikamApp::view() const
{
    return d->view;
}

void DigikamApp::show()
{
    // Remove Splashscreen.

    if (d->splashScreen)
    {
        d->splashScreen->finish(this);
        delete d->splashScreen;
        d->splashScreen = 0;
    }

    // Display application window.

    KMainWindow::show();

    // Report errors from ICC repository path.

    if (!d->validIccPath)
    {
        QString message = i18n("<p>The ICC profiles folder seems to be invalid.</p>"
                               "<p>If you want to try setting it again, choose \"Yes\" here, otherwise "
                               "choose \"No\", and the \"Color Management\" feature "
                               "will be disabled until you solve this issue.</p>");

        if (QMessageBox::warning(this, qApp->applicationName(), message, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            if (!setupICC())
            {
                d->config->group(QLatin1String("Color Management")).writeEntry(QLatin1String("EnableCM"), false);
                d->config->sync();
            }
        }
        else
        {
            d->config->group(QLatin1String("Color Management")).writeEntry(QLatin1String("EnableCM"), false);
            d->config->sync();
        }
    }

    // Init album icon view zoom factor.
    slotThumbSizeChanged(ApplicationSettings::instance()->getDefaultIconSize());
    slotZoomSliderChanged(ApplicationSettings::instance()->getDefaultIconSize());
    d->autoShowZoomToolTip = true;

    // Enable finished the collection scan as deferred process

    if (ApplicationSettings::instance()->getScanAtStart() ||
        !CollectionScanner::databaseInitialScanDone())
    {
        NewItemsFinder* const tool = new NewItemsFinder(NewItemsFinder::ScanDeferredFiles);
        QTimer::singleShot(1000, tool, SLOT(start()));
    }

    if (ApplicationSettings::instance()->getCleanAtStart())
    {
        DbCleaner* const tool = new DbCleaner(false,false);
        QTimer::singleShot(1000, tool, SLOT(start()));
    }

    // Start the Media Server if necessary
    
    DMediaServerMngr::instance()->loadAtStartup();
}

void DigikamApp::restoreSession()
{
    //TODO: show and restore ImageEditor, Lighttable, and Batch Queue Manager main windows
    if (qApp->isSessionRestored())
    {
        int n = 1;

        while (KMainWindow::canBeRestored(n))
        {
            const QString className = KMainWindow::classNameOfToplevel(n);

            if (className == QLatin1String(Digikam::DigikamApp::staticMetaObject.className()))
            {
                restore(n, false);
                break;
            }

            ++n;
        }
    }
}

void DigikamApp::closeEvent(QCloseEvent* e)
{
    // may show a progress dialog to finish actions
    FileActionMngr::instance()->requestShutDown();
    // may show a progress dialog to apply pending metadata
    if (MetadataHubMngr::isCreated())
        MetadataHubMngr::instance()->requestShutDown();

    DXmlGuiWindow::closeEvent(e);
    e->accept();
}

void DigikamApp::autoDetect()
{
    // Called from main if command line option is set, or via DBus

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Auto-Detecting Camera..."));
    }

    QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
}

void DigikamApp::downloadFrom(const QString& cameraGuiPath)
{
    // Called from main if command line option is set, or via DBus

    if (!cameraGuiPath.isEmpty())
    {
        if (d->splashScreen)
        {
            d->splashScreen->setMessage(i18n("Opening Download Dialog..."));
        }

        emit queuedOpenCameraUiFromPath(cameraGuiPath);
    }
}

void DigikamApp::downloadFromUdi(const QString& udi)
{
    // Called from main if command line option is set, or via DBus

    if (!udi.isEmpty())
    {
        if (d->splashScreen)
        {
            d->splashScreen->setMessage(i18n("Opening Download Dialog..."));
        }

        emit queuedOpenSolidDevice(udi);
    }
}

bool DigikamApp::queryClose()
{
    bool ret = true;

    if (ImageWindow::imageWindowCreated())
    {
        ret &= ImageWindow::imageWindow()->queryClose();
    }

    if (QueueMgrWindow::queueManagerWindowCreated())
    {
        ret &= QueueMgrWindow::queueManagerWindow()->queryClose();
    }

    return ret;
}

void DigikamApp::setupView()
{
    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Initializing Main View..."));
    }

    d->view = new DigikamView(this, d->modelCollection);
    setCentralWidget(d->view);
    d->view->applySettings();
}

void DigikamApp::setupViewConnections()
{
    connect(d->view, SIGNAL(signalAlbumSelected(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

    connect(d->view, SIGNAL(signalSelectionChanged(int)),
            this, SLOT(slotSelectionChanged(int)));

    connect(d->view, SIGNAL(signalImageSelected(ImageInfoList,ImageInfoList)),
            this, SLOT(slotImageSelected(ImageInfoList,ImageInfoList)));

    connect(d->view, SIGNAL(signalSwitchedToPreview()),
            this, SLOT(slotSwitchedToPreview()));

    connect(d->view, SIGNAL(signalSwitchedToIconView()),
            this, SLOT(slotSwitchedToIconView()));

    connect(d->view, SIGNAL(signalSwitchedToMapView()),
            this, SLOT(slotSwitchedToMapView()));

    connect(d->view, SIGNAL(signalSwitchedToTableView()),
            this, SLOT(slotSwitchedToTableView()));

    connect(d->view, SIGNAL(signalSwitchedToTrashView()),
            this, SLOT(slotSwitchedToTrashView()));
}

void DigikamApp::setupStatusBar()
{
    d->statusLabel = new DAdjustableLabel(statusBar());
    d->statusLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statusBar()->addWidget(d->statusLabel, 80);

    //------------------------------------------------------------------------------

    d->metadataStatusBar = new MetadataStatusBar(statusBar());
    statusBar()->addWidget(d->metadataStatusBar, 50);

    //------------------------------------------------------------------------------

    d->filterStatusBar = new FilterStatusBar(statusBar());
    statusBar()->addWidget(d->filterStatusBar, 50);
    d->view->connectIconViewFilter(d->filterStatusBar);

    //------------------------------------------------------------------------------

    ProgressView* const view = new ProgressView(statusBar(), this);
    view->hide();

    StatusbarProgressWidget* const littleProgress = new StatusbarProgressWidget(view, statusBar());
    littleProgress->show();
    statusBar()->addPermanentWidget(littleProgress);

    //------------------------------------------------------------------------------

    d->zoomBar = new DZoomBar(statusBar());
    d->zoomBar->setZoomToFitAction(d->zoomFitToWindowAction);
    d->zoomBar->setZoomTo100Action(d->zoomTo100percents);
    d->zoomBar->setZoomPlusAction(d->zoomPlusAction);
    d->zoomBar->setZoomMinusAction(d->zoomMinusAction);
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    statusBar()->addPermanentWidget(d->zoomBar);

    //------------------------------------------------------------------------------

    connect(d->zoomBar, SIGNAL(signalZoomSliderChanged(int)),
            this, SLOT(slotZoomSliderChanged(int)));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->zoomBar, SLOT(slotUpdateTrackerPos()));

    connect(d->zoomBar, SIGNAL(signalZoomValueEdited(double)),
            d->view, SLOT(setZoomFactor(double)));

    connect(d->view, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));

    connect(d->view, SIGNAL(signalThumbSizeChanged(int)),
            this, SLOT(slotThumbSizeChanged(int)));
}

void DigikamApp::setupAccelerators()
{
    KActionCollection* const ac = actionCollection();

    // Action are added by <MainWindow> tag in ui.rc XML file
    QAction* const escapeAction = new QAction(i18n("Exit Preview Mode"), this);
    ac->addAction(QLatin1String("exit_preview_mode"), escapeAction);
    ac->setDefaultShortcut(escapeAction, Qt::Key_Escape);
    connect(escapeAction, SIGNAL(triggered()), this, SIGNAL(signalEscapePressed()));

    QAction* const nextImageAction = new QAction(i18n("Next Image"), this);
    nextImageAction->setIcon(QIcon::fromTheme(QLatin1String("go-next")));
    ac->addAction(QLatin1String("next_image"), nextImageAction);
    ac->setDefaultShortcut(nextImageAction, Qt::Key_Space);
    connect(nextImageAction, SIGNAL(triggered()), this, SIGNAL(signalNextItem()));

    QAction* const previousImageAction = new QAction(i18n("Previous Image"), this);
    previousImageAction->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));
    ac->addAction(QLatin1String("previous_image"), previousImageAction);
    ac->setDefaultShortcuts(previousImageAction, QList<QKeySequence>() << Qt::Key_Backspace << Qt::SHIFT+Qt::Key_Space);
    connect(previousImageAction, SIGNAL(triggered()), this, SIGNAL(signalPrevItem()));

    QAction* const firstImageAction = new QAction(i18n("First Image"), this);
    ac->addAction(QLatin1String("first_image"), firstImageAction);
    ac->setDefaultShortcuts(firstImageAction, QList<QKeySequence>() << Qt::CTRL + Qt::Key_Home);
    connect(firstImageAction, SIGNAL(triggered()), this, SIGNAL(signalFirstItem()));

    QAction* const lastImageAction = new QAction(i18n("Last Image"), this);
    ac->addAction(QLatin1String("last_image"), lastImageAction);
    ac->setDefaultShortcuts(lastImageAction, QList<QKeySequence>() << Qt::CTRL + Qt::Key_End);
    connect(lastImageAction, SIGNAL(triggered()), this, SIGNAL(signalLastItem()));

    d->cutItemsAction = new QAction(i18n("Cu&t"), this);
    d->cutItemsAction->setIcon(QIcon::fromTheme(QLatin1String("edit-cut")));
    d->cutItemsAction->setWhatsThis(i18n("Cut selection to clipboard"));
    ac->addAction(QLatin1String("cut_album_selection"), d->cutItemsAction);
    // NOTE: shift+del keyboard shortcut must not be assigned to Cut action
    // else the shortcut for Delete permanently collides with secondary shortcut of Cut
    ac->setDefaultShortcut(d->cutItemsAction, Qt::CTRL + Qt::Key_X);
    connect(d->cutItemsAction, SIGNAL(triggered()), this, SIGNAL(signalCutAlbumItemsSelection()));

    d->copyItemsAction = buildStdAction(StdCopyAction, this, SIGNAL(signalCopyAlbumItemsSelection()), this);
    ac->addAction(QLatin1String("copy_album_selection"), d->copyItemsAction);

    d->pasteItemsAction = buildStdAction(StdPasteAction, this, SIGNAL(signalPasteAlbumItemsSelection()), this);
    ac->addAction(QLatin1String("paste_album_selection"), d->pasteItemsAction);

    // Labels shortcuts must be registered here to be saved in XML GUI files if user customize it.
    d->tagsActionManager->registerLabelsActions(ac);

    QAction* const editTitles = new QAction(i18n("Edit Titles"), this);
    ac->addAction(QLatin1String("edit_titles"), editTitles);
    ac->setDefaultShortcut(editTitles, Qt::META + Qt::Key_T);
    connect(editTitles, SIGNAL(triggered()), d->view, SLOT(slotRightSideBarActivateTitles()));

    QAction* const editComments = new QAction(i18n("Edit Comments"), this);
    ac->addAction(QLatin1String("edit_comments"), editComments);
    ac->setDefaultShortcut(editComments, Qt::META + Qt::Key_C);
    connect(editComments, SIGNAL(triggered()), d->view, SLOT(slotRightSideBarActivateComments()));

    QAction* const assignedTags = new QAction(i18n("Show Assigned Tags"), this);
    ac->addAction(QLatin1String("assigned _tags"), assignedTags);
    ac->setDefaultShortcut(assignedTags, Qt::META + Qt::Key_A);
    connect(assignedTags, SIGNAL(triggered()), d->view, SLOT(slotRightSideBarActivateAssignedTags()));
}

void DigikamApp::setupActions()
{
    KActionCollection* const ac = actionCollection();

    d->solidCameraActionGroup = new QActionGroup(this);
    connect(d->solidCameraActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenSolidCamera(QAction*)));

    d->solidUsmActionGroup = new QActionGroup(this);
    connect(d->solidUsmActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenSolidUsmDevice(QAction*)));

    d->manualCameraActionGroup = new QActionGroup(this);
    connect(d->manualCameraActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenManualCamera(QAction*)));

    // -----------------------------------------------------------------

    d->backwardActionMenu = new KToolBarPopupAction(QIcon::fromTheme(QLatin1String("go-previous")), i18n("&Back"), this);
    d->backwardActionMenu->setEnabled(false);
    ac->addAction(QLatin1String("album_back"), d->backwardActionMenu);
    ac->setDefaultShortcut(d->backwardActionMenu, Qt::ALT+Qt::Key_Left);

    connect(d->backwardActionMenu->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowBackwardMenu()));

    // we are using a signal mapper to identify which of a bunch of actions was triggered
    d->backwardSignalMapper = new QSignalMapper(this);

    // connect mapper to view
    connect(d->backwardSignalMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotAlbumHistoryBack(int)));

    // connect action to mapper
    connect(d->backwardActionMenu, SIGNAL(triggered()),
            d->backwardSignalMapper, SLOT(map()));

    // inform mapper about number of steps
    d->backwardSignalMapper->setMapping(d->backwardActionMenu, 1);

    // -----------------------------------------------------------------

    d->forwardActionMenu = new KToolBarPopupAction(QIcon::fromTheme(QLatin1String("go-next")), i18n("Forward"), this);
    d->forwardActionMenu->setEnabled(false);
    ac->addAction(QLatin1String("album_forward"), d->forwardActionMenu);
    ac->setDefaultShortcut(d->forwardActionMenu, Qt::ALT+Qt::Key_Right);

    connect(d->forwardActionMenu->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowForwardMenu()));

    d->forwardSignalMapper = new QSignalMapper(this);

    connect(d->forwardSignalMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotAlbumHistoryForward(int)));

    connect(d->forwardActionMenu, SIGNAL(triggered()), d->forwardSignalMapper, SLOT(map()));
    d->forwardSignalMapper->setMapping(d->forwardActionMenu, 1);

    // -----------------------------------------------------------------

    d->refreshAction = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")), i18n("Refresh"), this);
    d->refreshAction->setWhatsThis(i18n("Refresh the current contents."));
    connect(d->refreshAction, SIGNAL(triggered()), d->view, SLOT(slotRefresh()));
    ac->addAction(QLatin1String("view_refresh"), d->refreshAction);
    ac->setDefaultShortcut(d->refreshAction, Qt::Key_F5);

    // -----------------------------------------------------------------

    QSignalMapper* const browseActionsMapper = new QSignalMapper(this);
    connect(browseActionsMapper, SIGNAL(mapped(QWidget*)),
            d->view, SLOT(slotLeftSideBarActivate(QWidget*)));

    foreach(SidebarWidget* const leftWidget, d->view->leftSidebarWidgets())
    {
        QString actionName = QLatin1String("browse_") + leftWidget->objectName()
                                                        .remove(QLatin1Char(' '))
                                                        .remove(QLatin1String("Sidebar"))
                                                        .remove(QLatin1String("FolderView"))
                                                        .remove(QLatin1String("View")).toLower();
        qCDebug(DIGIKAM_GENERAL_LOG) << actionName;

        QAction* const action = new QAction(leftWidget->getIcon(), leftWidget->getCaption(), this);
        ac->addAction(actionName, action);
        ac->setDefaultShortcut(action, QKeySequence(leftWidget->property("Shortcut").toInt()));
        connect(action, SIGNAL(triggered()), browseActionsMapper, SLOT(map()));
        browseActionsMapper->setMapping(action, leftWidget);
    }

    // -----------------------------------------------------------------

    d->newAction = new QAction(QIcon::fromTheme(QLatin1String("folder-new")), i18n("&New..."), this);
    d->newAction->setWhatsThis(i18n("Creates a new empty Album in the collection."));
    connect(d->newAction, SIGNAL(triggered()), d->view, SLOT(slotNewAlbum()));
    ac->addAction(QLatin1String("album_new"), d->newAction);
    ac->setDefaultShortcuts(d->newAction, QList<QKeySequence>() << Qt::CTRL + Qt::Key_N);

    // -----------------------------------------------------------------

    d->moveSelectionToAlbumAction = new QAction(QIcon::fromTheme(QLatin1String("folder-new")), i18n("&Move to Album..."), this);
    d->moveSelectionToAlbumAction->setWhatsThis(i18n("Move selected images into an album."));
    connect(d->moveSelectionToAlbumAction, SIGNAL(triggered()), d->view, SLOT(slotMoveSelectionToAlbum()));
    ac->addAction(QLatin1String("move_selection_to_album"), d->moveSelectionToAlbumAction);

    // -----------------------------------------------------------------

    d->deleteAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete Album"), this);
    connect(d->deleteAction, SIGNAL(triggered()), d->view, SLOT(slotDeleteAlbum()));
    ac->addAction(QLatin1String("album_delete"), d->deleteAction);

    // -----------------------------------------------------------------

    d->renameAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Rename..."), this);
    connect(d->renameAction, SIGNAL(triggered()), d->view, SLOT(slotRenameAlbum()));
    ac->addAction(QLatin1String("album_rename"), d->renameAction);
    ac->setDefaultShortcut(d->renameAction, Qt::SHIFT + Qt::Key_F2);

    // -----------------------------------------------------------------

    d->propsEditAction = new QAction(QIcon::fromTheme(QLatin1String("configure")), i18n("Properties"), this);
    d->propsEditAction->setWhatsThis(i18n("Edit album properties and collection information."));
    connect(d->propsEditAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumPropsEdit()));
    ac->addAction(QLatin1String("album_propsEdit"), d->propsEditAction);
    ac->setDefaultShortcut(d->propsEditAction, Qt::ALT + Qt::Key_Return);

    // -----------------------------------------------------------------

    d->writeAlbumMetadataAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Write Metadata to Files"), this);
    d->writeAlbumMetadataAction->setWhatsThis(i18n("Updates metadata of files in the current "
                                                   "album with the contents of digiKam database "
                                                   "(file metadata will be overwritten with data from "
                                                   "the database)."));
    connect(d->writeAlbumMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumWriteMetadata()));
    ac->addAction(QLatin1String("album_write_metadata"), d->writeAlbumMetadataAction);

    // -----------------------------------------------------------------

    d->readAlbumMetadataAction = new QAction(QIcon::fromTheme(QLatin1String("edit-redo")), i18n("Reread Metadata From Files"), this);
    d->readAlbumMetadataAction->setWhatsThis(i18n("Updates the digiKam database from the metadata "
                                                  "of the files in the current album "
                                                  "(information in the database will be overwritten with data from "
                                                  "the files' metadata)."));
    connect(d->readAlbumMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumReadMetadata()));
    ac->addAction(QLatin1String("album_read_metadata"), d->readAlbumMetadataAction);

    // -----------------------------------------------------------------

    d->openInFileManagerAction = new QAction(QIcon::fromTheme(QLatin1String("folder-open")), i18n("Open in File Manager"), this);
    connect(d->openInFileManagerAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumOpenInFileManager()));
    ac->addAction(QLatin1String("album_openinfilemanager"), d->openInFileManagerAction);

    // -----------------------------------------------------------

    d->openTagMngrAction = new QAction(QIcon::fromTheme(QLatin1String("tag")), i18n("Tag Manager"), this);
    connect(d->openTagMngrAction, SIGNAL(triggered()), d->view, SLOT(slotOpenTagsManager()));
    ac->addAction(QLatin1String("open_tag_mngr"), d->openTagMngrAction);

    // -----------------------------------------------------------

    d->newTagAction = new QAction(QIcon::fromTheme(QLatin1String("tag-new")), i18nc("new tag", "N&ew..."), this);
    connect(d->newTagAction, SIGNAL(triggered()), d->view, SLOT(slotNewTag()));
    ac->addAction(QLatin1String("tag_new"), d->newTagAction);

    // -----------------------------------------------------------

    d->editTagAction = new QAction(QIcon::fromTheme(QLatin1String("tag-properties")), i18n("Properties"), this);
    connect(d->editTagAction, SIGNAL(triggered()), d->view, SLOT(slotEditTag()));
    ac->addAction(QLatin1String("tag_edit"), d->editTagAction);

    // -----------------------------------------------------------

    d->deleteTagAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")), i18n("Delete"), this);
    connect(d->deleteTagAction, SIGNAL(triggered()), d->view, SLOT(slotDeleteTag()));
    ac->addAction(QLatin1String("tag_delete"), d->deleteTagAction);

    // -----------------------------------------------------------

    d->assignTagAction = new QAction(QIcon::fromTheme(QLatin1String("tag-new")), i18n("Assign Tag"), this);
    connect(d->assignTagAction, SIGNAL(triggered()), d->view, SLOT(slotAssignTag()));
    ac->addAction(QLatin1String("tag_assign"), d->assignTagAction);
    ac->setDefaultShortcut(d->assignTagAction, Qt::Key_T);

    // -----------------------------------------------------------

    d->imageViewSelectionAction = new KSelectAction(QIcon::fromTheme(QLatin1String("view-preview")), i18n("Views"), this);
    ac->addAction(QLatin1String("view_selection"), d->imageViewSelectionAction);

    d->imageIconViewAction = new QAction(QIcon::fromTheme(QLatin1String("view-list-icons")),
                                         i18nc("@action Go to thumbnails (icon) view", "Thumbnails"), this);
    d->imageIconViewAction->setCheckable(true);
    ac->addAction(QLatin1String("icon_view"), d->imageIconViewAction);
    connect(d->imageIconViewAction, SIGNAL(triggered()), d->view, SLOT(slotIconView()));
    d->imageViewSelectionAction->addAction(d->imageIconViewAction);

    d->imagePreviewAction = new QAction(QIcon::fromTheme(QLatin1String("view-preview")),
                                        i18nc("View the selected image", "Preview"), this);
    d->imagePreviewAction->setCheckable(true);
    ac->addAction(QLatin1String("image_view"), d->imagePreviewAction);
    ac->setDefaultShortcut(d->imagePreviewAction, Qt::Key_F3);
    connect(d->imagePreviewAction, SIGNAL(triggered()), d->view, SLOT(slotImagePreview()));
    d->imageViewSelectionAction->addAction(d->imagePreviewAction);

#ifdef HAVE_MARBLE
    d->imageMapViewAction = new QAction(QIcon::fromTheme(QLatin1String("globe")),
                                        i18nc("@action Switch to map view", "Map"), this);
    d->imageMapViewAction->setCheckable(true);
    ac->addAction(QLatin1String("map_view"), d->imageMapViewAction);
    connect(d->imageMapViewAction, SIGNAL(triggered()), d->view, SLOT(slotMapWidgetView()));
    d->imageViewSelectionAction->addAction(d->imageMapViewAction);
#endif // HAVE_MARBLE

    d->imageTableViewAction = new QAction(QIcon::fromTheme(QLatin1String("view-list-details")),
                                          i18nc("@action Switch to table view", "Table"), this);
    d->imageTableViewAction->setCheckable(true);
    ac->addAction(QLatin1String("table_view"), d->imageTableViewAction);
    connect(d->imageTableViewAction, SIGNAL(triggered()), d->view, SLOT(slotTableView()));
    d->imageViewSelectionAction->addAction(d->imageTableViewAction);

    // -----------------------------------------------------------

    d->imageViewAction = new QAction(QIcon::fromTheme(QLatin1String("quickopen-file")), i18n("Open..."), this);
    d->imageViewAction->setWhatsThis(i18n("Open the selected item."));
    connect(d->imageViewAction, SIGNAL(triggered()), d->view, SLOT(slotImageEdit()));
    ac->addAction(QLatin1String("image_edit"), d->imageViewAction);
    ac->setDefaultShortcut(d->imageViewAction, Qt::Key_F4);

    d->openWithAction = new QAction(QIcon::fromTheme(QLatin1String("preferences-desktop-filetype-association")), i18n("Open With Default Application"), this);
    d->openWithAction->setWhatsThis(i18n("Open the selected item with default assigned application."));
    connect(d->openWithAction, SIGNAL(triggered()), d->view, SLOT(slotFileWithDefaultApplication()));
    ac->addAction(QLatin1String("open_with_default_application"), d->openWithAction);
    ac->setDefaultShortcut(d->openWithAction, Qt::META + Qt::Key_F4);

    d->ieAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Image Editor"), this);
    d->ieAction->setWhatsThis(i18n("Open the image editor."));
    connect(d->ieAction, SIGNAL(triggered()), d->view, SLOT(slotEditor()));
    ac->addAction(QLatin1String("imageeditor"), d->ieAction);

    // -----------------------------------------------------------

    d->ltAction = new QAction(QIcon::fromTheme(QLatin1String("lighttable")), i18n("Light Table"), this);
    connect(d->ltAction, SIGNAL(triggered()), d->view, SLOT(slotLightTable()));
    ac->addAction(QLatin1String("light_table"), d->ltAction);
    ac->setDefaultShortcut(d->ltAction, Qt::Key_L);

    d->imageLightTableAction = new QAction(QIcon::fromTheme(QLatin1String("lighttable")), i18n("Place onto Light Table"), this);
    d->imageLightTableAction->setWhatsThis(i18n("Place the selected items on the light table thumbbar."));
    connect(d->imageLightTableAction, SIGNAL(triggered()), d->view, SLOT(slotImageLightTable()));
    ac->addAction(QLatin1String("image_lighttable"), d->imageLightTableAction);
    ac->setDefaultShortcut(d->imageLightTableAction, Qt::CTRL+Qt::Key_L);

    d->imageAddLightTableAction = new QAction(QIcon::fromTheme(QLatin1String("list-add")), i18n("Add to Light Table"), this);
    d->imageAddLightTableAction->setWhatsThis(i18n("Add selected items to the light table thumbbar."));
    connect(d->imageAddLightTableAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToLightTable()));
    ac->addAction(QLatin1String("image_add_to_lighttable"), d->imageAddLightTableAction);
    ac->setDefaultShortcut(d->imageAddLightTableAction, Qt::SHIFT+Qt::CTRL+Qt::Key_L);

    // -----------------------------------------------------------

    d->bqmAction = new QAction(QIcon::fromTheme(QLatin1String("run-build")), i18n("Batch Queue Manager"), this);
    connect(d->bqmAction, SIGNAL(triggered()), d->view, SLOT(slotQueueMgr()));
    ac->addAction(QLatin1String("queue_manager"), d->bqmAction);
    ac->setDefaultShortcut(d->bqmAction, Qt::Key_B);

    d->imageAddCurrentQueueAction = new QAction(QIcon::fromTheme(QLatin1String("go-up")), i18n("Add to Current Queue"), this);
    d->imageAddCurrentQueueAction->setWhatsThis(i18n("Add selected items to current queue from batch manager."));
    connect(d->imageAddCurrentQueueAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToCurrentQueue()));
    ac->addAction(QLatin1String("image_add_to_current_queue"), d->imageAddCurrentQueueAction);
    ac->setDefaultShortcut(d->imageAddCurrentQueueAction, Qt::CTRL+Qt::Key_B);

    d->imageAddNewQueueAction = new QAction(QIcon::fromTheme(QLatin1String("list-add")), i18n("Add to New Queue"), this);
    d->imageAddNewQueueAction->setWhatsThis(i18n("Add selected items to a new queue from batch manager."));
    connect(d->imageAddNewQueueAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToNewQueue()));
    ac->addAction(QLatin1String("image_add_to_new_queue"), d->imageAddNewQueueAction);
    ac->setDefaultShortcut(d->imageAddNewQueueAction, Qt::SHIFT+Qt::CTRL+Qt::Key_B);

    // -----------------------------------------------------------------

    d->quickImportMenu->setTitle(i18nc("@action Import photos from camera", "Import"));
    d->quickImportMenu->setIcon(QIcon::fromTheme(QLatin1String("camera-photo")));
    ac->addAction(QLatin1String("import_auto"), d->quickImportMenu->menuAction());

    // -----------------------------------------------------------------

    d->imageWriteMetadataAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")),
                                              i18n("Write Metadata to Selected Files"), this);
    d->imageWriteMetadataAction->setWhatsThis(i18n("Updates metadata of files in the current "
                                                   "album with the contents of digiKam database "
                                                   "(file metadata will be overwritten with data from "
                                                   "the database)."));
    connect(d->imageWriteMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotImageWriteMetadata()));
    ac->addAction(QLatin1String("image_write_metadata"), d->imageWriteMetadataAction);

    // -----------------------------------------------------------------

    d->imageReadMetadataAction = new QAction(QIcon::fromTheme(QLatin1String("edit-redo")),
                                             i18n("Reread Metadata From Selected Files"), this);
    d->imageReadMetadataAction->setWhatsThis(i18n("Updates the digiKam database from the metadata "
                                                  "of the files in the current album "
                                                  "(information in the database will be overwritten with data from "
                                                  "the files' metadata)."));
    connect(d->imageReadMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotImageReadMetadata()));
    ac->addAction(QLatin1String("image_read_metadata"), d->imageReadMetadataAction);

    // -----------------------------------------------------------

    d->imageFindSimilarAction = new QAction(QIcon::fromTheme(QLatin1String("tools-wizard")), i18n("Find Similar..."), this);
    d->imageFindSimilarAction->setWhatsThis(i18n("Find similar images using selected item as reference."));
    connect(d->imageFindSimilarAction, SIGNAL(triggered()), d->view, SLOT(slotImageFindSimilar()));
    ac->addAction(QLatin1String("image_find_similar"), d->imageFindSimilarAction);

    // -----------------------------------------------------------

    d->imageRenameAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Rename..."), this);
    d->imageRenameAction->setWhatsThis(i18n("Change the filename of the currently selected item."));
    connect(d->imageRenameAction, SIGNAL(triggered()), d->view, SLOT(slotImageRename()));
    ac->addAction(QLatin1String("image_rename"), d->imageRenameAction);
    ac->setDefaultShortcut(d->imageRenameAction, Qt::Key_F2);

    // -----------------------------------------------------------

    // Pop up dialog to ask user whether to move to trash
    d->imageDeleteAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")), i18nc("Non-pluralized", "Move to Trash"), this);
    connect(d->imageDeleteAction, SIGNAL(triggered()), d->view, SLOT(slotImageDelete()));
    ac->addAction(QLatin1String("image_delete"), d->imageDeleteAction);
    ac->setDefaultShortcut(d->imageDeleteAction, Qt::Key_Delete);

    // -----------------------------------------------------------

    // Pop up dialog to ask user whether to permanently delete
    // FIXME: This action is never used?? How can someone delete a album directly, without moving it to the trash first?
    //        This is especially important when deleting from a different partition or from a net source.
    //        Also note that we use the wrong icon for the default album delete action, which should have a trashcan icon instead
    //        of a red cross, it confuses users.
    d->imageDeletePermanentlyAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete Permanently"), this);
    connect(d->imageDeletePermanentlyAction, SIGNAL(triggered()), d->view, SLOT(slotImageDeletePermanently()));
    ac->addAction(QLatin1String("image_delete_permanently"), d->imageDeletePermanentlyAction);
    ac->setDefaultShortcut(d->imageDeletePermanentlyAction, Qt::SHIFT+Qt::Key_Delete);

    // -----------------------------------------------------------

    // These two actions are hidden, no menu entry, no toolbar entry, no shortcut.
    // Power users may add them.
    d->imageDeletePermanentlyDirectlyAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),
                                                          i18n("Delete permanently without confirmation"), this);
    connect(d->imageDeletePermanentlyDirectlyAction, SIGNAL(triggered()),
            d->view, SLOT(slotImageDeletePermanentlyDirectly()));
    ac->addAction(QLatin1String("image_delete_permanently_directly"), d->imageDeletePermanentlyDirectlyAction);

    // -----------------------------------------------------------

    d->imageTrashDirectlyAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")),
                                              i18n("Move to trash without confirmation"), this);
    connect(d->imageTrashDirectlyAction, SIGNAL(triggered()),
            d->view, SLOT(slotImageTrashDirectly()));
    ac->addAction(QLatin1String("image_trash_directly"), d->imageTrashDirectlyAction);

    // -----------------------------------------------------------------

    d->albumSortAction = new KSelectAction(i18n("&Sort Albums"), this);
    d->albumSortAction->setWhatsThis(i18n("Sort Albums in tree-view."));
    connect(d->albumSortAction, SIGNAL(triggered(int)), d->view, SLOT(slotSortAlbums(int)));
    ac->addAction(QLatin1String("album_sort"), d->albumSortAction);

    // Use same list order as in applicationsettings enum
    QStringList sortActionList;
    sortActionList.append(i18n("By Folder"));
    sortActionList.append(i18n("By Category"));
    sortActionList.append(i18n("By Date"));
    d->albumSortAction->setItems(sortActionList);

    // -----------------------------------------------------------

    d->recurseAlbumsAction = new QAction(i18n("Include Album Sub-Tree"), this);
    d->recurseAlbumsAction->setCheckable(true);
    d->recurseAlbumsAction->setWhatsThis(i18n("Activate this option to show all sub-albums below "
                                              "the current album."));
    connect(d->recurseAlbumsAction, SIGNAL(toggled(bool)), this, SLOT(slotRecurseAlbums(bool)));
    ac->addAction(QLatin1String("albums_recursive"), d->recurseAlbumsAction);

    d->recurseTagsAction = new QAction(i18n("Include Tag Sub-Tree"), this);
    d->recurseTagsAction->setCheckable(true);
    d->recurseTagsAction->setWhatsThis(i18n("Activate this option to show all images marked by the given tag "
                                            "and all its sub-tags."));
    connect(d->recurseTagsAction, SIGNAL(toggled(bool)), this, SLOT(slotRecurseTags(bool)));
    ac->addAction(QLatin1String("tags_recursive"), d->recurseTagsAction);

    // -----------------------------------------------------------

    d->imageSortAction                   = new KSelectAction(i18n("&Sort Items"), this);
    d->imageSortAction->setWhatsThis(i18n("The value by which the images in one album are sorted in the thumbnail view"));
    QSignalMapper* const imageSortMapper = new QSignalMapper(this);
    connect(imageSortMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSortImages(int)));
    ac->addAction(QLatin1String("image_sort"), d->imageSortAction);

    // map to ImageSortSettings enum
    QAction* const sortByNameAction        = d->imageSortAction->addAction(i18n("By Name"));
    QAction* const sortByPathAction        = d->imageSortAction->addAction(i18n("By Path"));
    QAction* const sortByDateAction        = d->imageSortAction->addAction(i18n("By Date"));
    QAction* const sortByFileSizeAction    = d->imageSortAction->addAction(i18n("By File Size"));
    QAction* const sortByRatingAction      = d->imageSortAction->addAction(i18n("By Rating"));
    QAction* const sortByImageSizeAction   = d->imageSortAction->addAction(i18n("By Image Size"));
    QAction* const sortByAspectRatioAction = d->imageSortAction->addAction(i18n("By Aspect Ratio"));
    QAction* const sortBySimilarityAction  = d->imageSortAction->addAction(i18n("By Similarity"));

    // activate the sort by similarity if the fuzzy search sidebar is active. Deactivate at start.
    sortBySimilarityAction->setEnabled(false);
    connect(d->view, SIGNAL(signalFuzzySidebarActive(bool)), sortBySimilarityAction, SLOT(setEnabled(bool)));

    connect(sortByNameAction,        SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByPathAction,        SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByDateAction,        SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByFileSizeAction,    SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByRatingAction,      SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByImageSizeAction,   SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByAspectRatioAction, SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortBySimilarityAction,  SIGNAL(triggered()), imageSortMapper, SLOT(map()));

    imageSortMapper->setMapping(sortByNameAction,        (int)ImageSortSettings::SortByFileName);
    imageSortMapper->setMapping(sortByPathAction,        (int)ImageSortSettings::SortByFilePath);
    imageSortMapper->setMapping(sortByDateAction,        (int)ImageSortSettings::SortByCreationDate);
    imageSortMapper->setMapping(sortByFileSizeAction,    (int)ImageSortSettings::SortByFileSize);
    imageSortMapper->setMapping(sortByRatingAction,      (int)ImageSortSettings::SortByRating);
    imageSortMapper->setMapping(sortByImageSizeAction,   (int)ImageSortSettings::SortByImageSize);
    imageSortMapper->setMapping(sortByAspectRatioAction, (int)ImageSortSettings::SortByAspectRatio);
    imageSortMapper->setMapping(sortBySimilarityAction,  (int)ImageSortSettings::SortBySimilarity);

    // -----------------------------------------------------------

    d->imageSortOrderAction                   = new KSelectAction(i18n("Item Sort &Order"), this);
    d->imageSortOrderAction->setWhatsThis(i18n("Defines whether images are sorted in ascending or descending manner."));
    QSignalMapper* const imageSortOrderMapper = new QSignalMapper(this);
    connect(imageSortOrderMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSortImagesOrder(int)));
    ac->addAction(QLatin1String("image_sort_order"), d->imageSortOrderAction);

    QAction* const sortAscendingAction  = d->imageSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-ascending")),  i18n("Ascending"));
    QAction* const sortDescendingAction = d->imageSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-descending")), i18n("Descending"));

    connect(sortAscendingAction,  SIGNAL(triggered()), imageSortOrderMapper, SLOT(map()));
    connect(sortDescendingAction, SIGNAL(triggered()), imageSortOrderMapper, SLOT(map()));

    imageSortOrderMapper->setMapping(sortAscendingAction,  (int)ImageSortSettings::AscendingOrder);
    imageSortOrderMapper->setMapping(sortDescendingAction, (int)ImageSortSettings::DescendingOrder);

    // -----------------------------------------------------------

    d->imageSeparationAction                   = new KSelectAction(i18n("Separate Items"), this);
    d->imageSeparationAction->setWhatsThis(i18n("The categories in which the images in the thumbnail view are displayed"));
    QSignalMapper* const imageSeparationMapper = new QSignalMapper(this);
    connect(imageSeparationMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSeparateImages(int)));
    ac->addAction(QLatin1String("image_separation"), d->imageSeparationAction);

    // map to ImageSortSettings enum
    QAction* const noCategoriesAction     = d->imageSeparationAction->addAction(i18n("Flat List"));
    QAction* const separateByAlbumAction  = d->imageSeparationAction->addAction(i18n("By Album"));
    QAction* const separateByFormatAction = d->imageSeparationAction->addAction(i18n("By Format"));

    connect(noCategoriesAction,  SIGNAL(triggered()), imageSeparationMapper, SLOT(map()));
    connect(separateByAlbumAction,  SIGNAL(triggered()), imageSeparationMapper, SLOT(map()));
    connect(separateByFormatAction, SIGNAL(triggered()), imageSeparationMapper, SLOT(map()));

    imageSeparationMapper->setMapping(noCategoriesAction,  (int)ImageSortSettings::OneCategory);
    imageSeparationMapper->setMapping(separateByAlbumAction,  (int)ImageSortSettings::CategoryByAlbum);
    imageSeparationMapper->setMapping(separateByFormatAction, (int)ImageSortSettings::CategoryByFormat);

    // -----------------------------------------------------------------

    d->imageSeparationSortOrderAction                   = new KSelectAction(i18n("Item Separation Order"), this);
    d->imageSeparationSortOrderAction->setWhatsThis(i18n("The sort order of the groups of separated items"));
    QSignalMapper* const imageSeparationSortOrderMapper = new QSignalMapper(this);
    connect(imageSeparationSortOrderMapper, SIGNAL(mapped(int)), d->view, SLOT(slotImageSeparationSortOrder(int)));
    ac->addAction(QLatin1String("image_separation_sort_order"), d->imageSeparationSortOrderAction);

    QAction* const sortSeparationsAscending  = d->imageSeparationSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-ascending")),  i18n("Ascending"));
    QAction* const sortSeparationsDescending = d->imageSeparationSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-descending")), i18n("Descending"));

    connect(sortSeparationsAscending,  SIGNAL(triggered()), imageSeparationSortOrderMapper, SLOT(map()));
    connect(sortSeparationsDescending, SIGNAL(triggered()), imageSeparationSortOrderMapper, SLOT(map()));

    imageSeparationSortOrderMapper->setMapping(sortSeparationsAscending, (int)ImageSortSettings::AscendingOrder);
    imageSeparationSortOrderMapper->setMapping(sortSeparationsDescending, (int)ImageSortSettings::DescendingOrder);

    // -----------------------------------------------------------------

    setupImageTransformActions();
    setupExifOrientationActions();
    createMetadataEditAction();
    createGeolocationEditAction();
    createExportActions();
    createImportActions();

    // -----------------------------------------------------------------

    d->selectAllAction = new QAction(i18n("Select All"), this);
    connect(d->selectAllAction, SIGNAL(triggered()), d->view, SLOT(slotSelectAll()));
    ac->addAction(QLatin1String("selectAll"), d->selectAllAction);
    ac->setDefaultShortcut(d->selectAllAction, Qt::CTRL+Qt::Key_A);

    // -----------------------------------------------------------------

    d->selectNoneAction = new QAction(i18n("Select None"), this);
    connect(d->selectNoneAction, SIGNAL(triggered()), d->view, SLOT(slotSelectNone()));
    ac->addAction(QLatin1String("selectNone"), d->selectNoneAction);
    ac->setDefaultShortcut(d->selectNoneAction, Qt::CTRL+Qt::SHIFT+Qt::Key_A);

    // -----------------------------------------------------------------

    d->selectInvertAction = new QAction(i18n("Invert Selection"), this);
    connect(d->selectInvertAction, SIGNAL(triggered()), d->view, SLOT(slotSelectInvert()));
    ac->addAction(QLatin1String("selectInvert"), d->selectInvertAction);
    ac->setDefaultShortcut(d->selectInvertAction, Qt::CTRL+Qt::Key_I);

    // -----------------------------------------------------------

    d->showBarAction = new QAction(QIcon::fromTheme(QLatin1String("view-choose")), i18n("Show Thumbbar"), this);
    d->showBarAction->setCheckable(true);
    connect(d->showBarAction, SIGNAL(triggered()), this, SLOT(slotToggleShowBar()));
    ac->addAction(QLatin1String("showthumbs"), d->showBarAction);
    ac->setDefaultShortcut(d->showBarAction, Qt::CTRL+Qt::Key_T);

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // Standard 'Configure' menu actions
    createSettingsActions();

    // -----------------------------------------------------------

    d->zoomPlusAction  = buildStdAction(StdZoomInAction, d->view, SLOT(slotZoomIn()), this);
    QKeySequence keysPlus(d->zoomPlusAction->shortcut()[0], Qt::Key_Plus);
    ac->addAction(QLatin1String("album_zoomin"), d->zoomPlusAction);
    ac->setDefaultShortcut(d->zoomPlusAction, keysPlus);

    // -----------------------------------------------------------

    d->zoomMinusAction  = buildStdAction(StdZoomOutAction, d->view, SLOT(slotZoomOut()), this);
    QKeySequence keysMinus(d->zoomMinusAction->shortcut()[0], Qt::Key_Minus);
    ac->addAction(QLatin1String("album_zoomout"), d->zoomMinusAction);
    ac->setDefaultShortcut(d->zoomMinusAction, keysMinus);

    // -----------------------------------------------------------

    d->zoomTo100percents = new QAction(QIcon::fromTheme(QLatin1String("zoom-original")), i18n("Zoom to 100%"), this);
    connect(d->zoomTo100percents, SIGNAL(triggered()), d->view, SLOT(slotZoomTo100Percents()));
    ac->addAction(QLatin1String("album_zoomto100percents"), d->zoomTo100percents);
    ac->setDefaultShortcut(d->zoomTo100percents, Qt::CTRL + Qt::Key_Period);

    // -----------------------------------------------------------

    d->zoomFitToWindowAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")), i18n("Fit to &Window"), this);
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), d->view, SLOT(slotFitToWindow()));
    ac->addAction(QLatin1String("album_zoomfit2window"), d->zoomFitToWindowAction);
    ac->setDefaultShortcut(d->zoomFitToWindowAction, Qt::ALT + Qt::CTRL + Qt::Key_E);

    // -----------------------------------------------------------

    createFullScreenAction(QLatin1String("full_screen"));
    createSidebarActions();

    // -----------------------------------------------------------

    d->slideShowAction = new QMenu(i18n("Slideshow"), this);
    d->slideShowAction->setIcon(QIcon::fromTheme(QLatin1String("view-presentation")));
    ac->addAction(QLatin1String("slideshow"), d->slideShowAction->menuAction());

    d->slideShowAllAction = new QAction(i18n("All"), this);
    connect(d->slideShowAllAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowAll()));
    ac->addAction(QLatin1String("slideshow_all"), d->slideShowAllAction);
    ac->setDefaultShortcut(d->slideShowAllAction, Qt::Key_F9);
    d->slideShowAction->addAction(d->slideShowAllAction);

    d->slideShowSelectionAction = new QAction(i18n("Selection"), this);
    connect(d->slideShowSelectionAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowSelection()));
    ac->addAction(QLatin1String("slideshow_selected"), d->slideShowSelectionAction);
    ac->setDefaultShortcut(d->slideShowSelectionAction, Qt::ALT+Qt::Key_F9);
    d->slideShowAction->addAction(d->slideShowSelectionAction);

    d->slideShowRecursiveAction = new QAction(i18n("With All Sub-Albums"), this);
    connect(d->slideShowRecursiveAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowRecursive()));
    ac->addAction(QLatin1String("slideshow_recursive"), d->slideShowRecursiveAction);
    ac->setDefaultShortcut(d->slideShowRecursiveAction, Qt::SHIFT+Qt::Key_F9);
    d->slideShowAction->addAction(d->slideShowRecursiveAction);

    createPresentationAction();

    // -----------------------------------------------------------

    d->viewCMViewAction = new QAction(QIcon::fromTheme(QLatin1String("video-display")), i18n("Color-Managed View"), this);
    d->viewCMViewAction->setCheckable(true);
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    ac->addAction(QLatin1String("color_managed_view"), d->viewCMViewAction);
    ac->setDefaultShortcut(d->viewCMViewAction, Qt::Key_F12);

    // -----------------------------------------------------------

    d->quitAction = buildStdAction(StdQuitAction, this, SLOT(slotExit()), this);
    ac->addAction(QLatin1String("app_exit"), d->quitAction);

    // -----------------------------------------------------------

    createHelpActions();

    // -----------------------------------------------------------

    QAction* const findAction = new QAction(QIcon::fromTheme(QLatin1String("edit-find")), i18n("Search..."), this);
    connect(findAction, SIGNAL(triggered()), d->view, SLOT(slotNewKeywordSearch()));
    ac->addAction(QLatin1String("search_quick"), findAction);
    ac->setDefaultShortcut(findAction, Qt::CTRL+Qt::Key_F);

    // -----------------------------------------------------------

    d->advSearchAction = new QAction(QIcon::fromTheme(QLatin1String("edit-find")), i18n("Advanced Search..."), this);
    connect(d->advSearchAction, SIGNAL(triggered()), d->view, SLOT(slotNewAdvancedSearch()));
    ac->addAction(QLatin1String("search_advanced"), d->advSearchAction);
    ac->setDefaultShortcut(d->advSearchAction, Qt::CTRL+Qt::ALT+Qt::Key_F);

    // -----------------------------------------------------------

    QAction* const duplicatesAction = new QAction(QIcon::fromTheme(QLatin1String("tools-wizard")), i18n("Find Duplicates..."), this);
    connect(duplicatesAction, SIGNAL(triggered()), d->view, SLOT(slotNewDuplicatesSearch()));
    ac->addAction(QLatin1String("find_duplicates"), duplicatesAction);
    ac->setDefaultShortcut(duplicatesAction, Qt::CTRL+Qt::Key_D);

    // -----------------------------------------------------------

#ifdef HAVE_MYSQLSUPPORT
    QAction* const databaseMigrationAction = new QAction(QIcon::fromTheme(QLatin1String("network-server-database")), i18n("Database Migration..."), this);
    connect(databaseMigrationAction, SIGNAL(triggered()), this, SLOT(slotDatabaseMigration()));
    ac->addAction(QLatin1String("database_migration"), databaseMigrationAction);
#endif

    // -----------------------------------------------------------

    d->maintenanceAction = new QAction(QIcon::fromTheme(QLatin1String("run-build-prune")), i18n("Maintenance..."), this);
    connect(d->maintenanceAction, SIGNAL(triggered()), this, SLOT(slotMaintenance()));
    ac->addAction(QLatin1String("maintenance"), d->maintenanceAction);

    createExpoBlendingAction();
    createPanoramaAction();
    createHtmlGalleryAction();
    createCalendarAction();
    createVideoSlideshowAction();
    createSendByMailAction();
    createPrintCreatorAction();
    createMediaServerAction();

    // -----------------------------------------------------------

    QAction* const cameraAction = new QAction(i18n("Add Camera Manually..."), this);
    connect(cameraAction, SIGNAL(triggered()), this, SLOT(slotSetupCamera()));
    ac->addAction(QLatin1String("camera_add"), cameraAction);

    // -----------------------------------------------------------

    // Load Cameras -- do this before the createGUI so that the cameras
    // are plugged into the toolbar at startup
    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Loading cameras..."));
    }

    loadCameras();

    // Load Themes

    populateThemes();

    createGUI(xmlFile());

    cleanupActions();

    // NOTE: see bug #252130 and #283281 : we need to disable these actions when BQM is running.
    // These connections must be done after loading color theme else theme menu cannot be plugged to Settings menu,

    connect(QueueMgrWindow::queueManagerWindow(), SIGNAL(signalBqmIsBusy(bool)),
            d->bqmAction, SLOT(setDisabled(bool)));

    connect(QueueMgrWindow::queueManagerWindow(), SIGNAL(signalBqmIsBusy(bool)),
            d->imageAddCurrentQueueAction, SLOT(setDisabled(bool)));

    connect(QueueMgrWindow::queueManagerWindow(), SIGNAL(signalBqmIsBusy(bool)),
            d->imageAddNewQueueAction, SLOT(setDisabled(bool)));
}

void DigikamApp::initGui()
{
    // Initialize Actions ---------------------------------------

    d->deleteAction->setEnabled(false);
    d->renameAction->setEnabled(false);
    d->addImagesAction->setEnabled(false);
    d->propsEditAction->setEnabled(false);
    d->openInFileManagerAction->setEnabled(false);

    d->imageViewAction->setEnabled(false);
    d->imagePreviewAction->setEnabled(false);
    d->imageLightTableAction->setEnabled(false);
    d->imageAddLightTableAction->setEnabled(false);
    d->imageFindSimilarAction->setEnabled(false);
    d->imageRenameAction->setEnabled(false);
    d->imageDeleteAction->setEnabled(false);
    d->imageExifOrientationActionMenu->setEnabled(false);
    d->openWithAction->setEnabled(false);
    d->slideShowSelectionAction->setEnabled(false);
    m_metadataEditAction->setEnabled(false);
    d->imageAutoExifActionMenu->setEnabled(false);

#ifdef HAVE_MARBLE
    m_geolocationEditAction->setEnabled(false);
#endif

    d->albumSortAction->setCurrentItem((int)ApplicationSettings::instance()->getAlbumSortRole());
    d->imageSortAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSortOrder());
    d->imageSortOrderAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSorting());
    d->imageSeparationAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSeparationMode()-1); // no action for enum 0
    d->imageSeparationSortOrderAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSeparationSortOrder());
    d->recurseAlbumsAction->setChecked(ApplicationSettings::instance()->getRecurseAlbums());
    d->recurseTagsAction->setChecked(ApplicationSettings::instance()->getRecurseTags());
    d->showBarAction->setChecked(ApplicationSettings::instance()->getShowThumbbar());
    showMenuBarAction()->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080

    slotSwitchedToIconView();
}

void DigikamApp::enableZoomPlusAction(bool val)
{
    d->zoomPlusAction->setEnabled(val);
}

void DigikamApp::enableZoomMinusAction(bool val)
{
    d->zoomMinusAction->setEnabled(val);
}

void DigikamApp::enableAlbumBackwardHistory(bool enable)
{
    d->backwardActionMenu->setEnabled(enable);
}

void DigikamApp::enableAlbumForwardHistory(bool enable)
{
    d->forwardActionMenu->setEnabled(enable);
}

void DigikamApp::slotAboutToShowBackwardMenu()
{
    d->backwardActionMenu->menu()->clear();
    QStringList titles;
    d->view->getBackwardHistory(titles);

    for (int i = 0; i < titles.size(); ++i)
    {
        QAction* const action = d->backwardActionMenu->menu()->addAction(titles.at(i), d->backwardSignalMapper, SLOT(map()));
        d->backwardSignalMapper->setMapping(action, i + 1);
    }
}

void DigikamApp::slotAboutToShowForwardMenu()
{
    d->forwardActionMenu->menu()->clear();
    QStringList titles;
    d->view->getForwardHistory(titles);

    for (int i = 0; i < titles.size(); ++i)
    {
        QAction* const action = d->forwardActionMenu->menu()->addAction(titles.at(i), d->forwardSignalMapper, SLOT(map()));
        d->forwardSignalMapper->setMapping(action, i + 1);
    }
}

void DigikamApp::slotAlbumSelected(Album* album)
{
    if (album)
    {
        PAlbum* const palbum = dynamic_cast<PAlbum*>(album);

        if (album->type() != Album::PHYSICAL || !palbum)
        {
            // Rules if not Physical album.

            d->deleteAction->setEnabled(false);
            d->renameAction->setEnabled(false);
            d->addImagesAction->setEnabled(false);
            d->propsEditAction->setEnabled(false);
            d->openInFileManagerAction->setEnabled(false);
            d->newAction->setEnabled(false);
            d->addFoldersAction->setEnabled(false);
            d->writeAlbumMetadataAction->setEnabled(true);
            d->readAlbumMetadataAction->setEnabled(true);
            d->pasteItemsAction->setEnabled(!album->isRoot());

            // Special case if Tag album.

            bool enabled = (album->type() == Album::TAG) && !album->isRoot();
            d->newTagAction->setEnabled(enabled);
            d->deleteTagAction->setEnabled(enabled);
            d->editTagAction->setEnabled(enabled);
        }
        else
        {
            // Rules if Physical album.

            // We have either the abstract root album,
            // the album root album for collection base dirs, or normal albums.

            bool isRoot          = palbum->isRoot();
            bool isAlbumRoot     = palbum->isAlbumRoot();
            bool isNormalAlbum   = !isRoot && !isAlbumRoot;

            d->deleteAction->setEnabled(isNormalAlbum);
            d->renameAction->setEnabled(isNormalAlbum);
            d->addImagesAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->propsEditAction->setEnabled(isNormalAlbum);
            d->openInFileManagerAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->newAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->addFoldersAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->writeAlbumMetadataAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->readAlbumMetadataAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->pasteItemsAction->setEnabled(isNormalAlbum || isAlbumRoot);
        }
    }
    else
    {
        // Rules if no current album.

        d->deleteAction->setEnabled(false);
        d->renameAction->setEnabled(false);
        d->addImagesAction->setEnabled(false);
        d->propsEditAction->setEnabled(false);
        d->openInFileManagerAction->setEnabled(false);
        d->newAction->setEnabled(false);
        d->addFoldersAction->setEnabled(false);
        d->writeAlbumMetadataAction->setEnabled(false);
        d->readAlbumMetadataAction->setEnabled(false);
        d->pasteItemsAction->setEnabled(false);

        d->newTagAction->setEnabled(false);
        d->deleteTagAction->setEnabled(false);
        d->editTagAction->setEnabled(false);
    }
}

void DigikamApp::slotImageSelected(const ImageInfoList& selection, const ImageInfoList& listAll)
{
    int numImagesWithGrouped              = listAll.count();
    int numImagesWithoutGrouped           = d->view->allUrls(false).count();
    ImageInfoList selectionWithoutGrouped = d->view->selectedInfoList(true, false);

    QString statusBarSelectionText;
    QString statusBarSelectionToolTip;

    switch (selection.count())
    {
        case 0:
        {
            if (numImagesWithGrouped == numImagesWithoutGrouped)
            {
                statusBarSelectionText
                        = i18np("No item selected (%1 item)", "No item selected (%1 items)",
                                numImagesWithoutGrouped);
                break;
            }
            statusBarSelectionText
                        = i18np("No item selected (%1 [%2] item)",
                                "No item selected (%1 [%2] items)",
                                numImagesWithoutGrouped, numImagesWithGrouped);
            statusBarSelectionToolTip
                        = i18np("No item selected (%1 item. With grouped items: %2)",
                                "No item selected (%1 items. With grouped items: %2)",
                                numImagesWithoutGrouped, numImagesWithGrouped);
            break;
        }
        default:
        {
            if (numImagesWithGrouped == numImagesWithoutGrouped)
            {
                statusBarSelectionText = i18n("%1/%2 items selected",
                                              selection.count(), numImagesWithoutGrouped);
                break;
            }

            if (selectionWithoutGrouped.count() > 1)
            {
                if (selection.count() == selectionWithoutGrouped.count())
                {
                    statusBarSelectionText
                            = i18n("%1/%2 [%3] items selected", selectionWithoutGrouped.count(),
                                   numImagesWithoutGrouped, numImagesWithGrouped);
                    statusBarSelectionToolTip
                            = i18n("%1/%2 items selected. Total with grouped items: %3",
                                   selectionWithoutGrouped.count(), numImagesWithoutGrouped,
                                   numImagesWithGrouped);
                }
                else
                {
                    statusBarSelectionText
                            = i18n("%1/%2 [%3/%4] items selected",
                                   selectionWithoutGrouped.count(), numImagesWithoutGrouped,
                                   selection.count(), numImagesWithGrouped);
                    statusBarSelectionToolTip
                            = i18n("%1/%2 items selected. With grouped items: %3/%4",
                                   selectionWithoutGrouped.count(), numImagesWithoutGrouped,
                                   selection.count(), numImagesWithGrouped);
                }

                break;
            }
            // no break; is completely intentional, arriving here is equivalent to case 1:
            [[fallthrough]];
        }
        case 1:
        {
            slotSetCheckedExifOrientationAction(selectionWithoutGrouped.first());

            int index = listAll.indexOf(selection.first()) + 1;
            if (numImagesWithGrouped == numImagesWithoutGrouped)
            {
                statusBarSelectionText = selection.first().fileUrl().fileName()
                                            + i18n(" (%1 of %2)",
                                                   index, numImagesWithoutGrouped);
            }
            else
            {
                int indexWithoutGrouped
                        = d->view->allInfo(false).indexOf(selectionWithoutGrouped.first()) + 1;
                statusBarSelectionText
                        = selection.first().fileUrl().fileName()
                          + i18n(" (%1 of %2 [%3])", indexWithoutGrouped,
                                 numImagesWithoutGrouped, numImagesWithGrouped);
                statusBarSelectionToolTip
                        = selection.first().fileUrl().fileName()
                          + i18n(" (%1 of %2. Total with grouped items: %3)", indexWithoutGrouped,
                                 numImagesWithoutGrouped, numImagesWithGrouped);
            }

            break;
        }
    }

    d->statusLabel->setAdjustedText(statusBarSelectionText);
    d->statusLabel->setToolTip(statusBarSelectionToolTip);
}

void DigikamApp::slotSelectionChanged(int selectionCount)
{
    // The preview can either be activated when only one image is selected,
    // or if multiple images are selected, but one image is the 'current image'.
    bool hasAtLeastCurrent =(selectionCount == 1) || ( (selectionCount > 0) && d->view->hasCurrentItem());

    d->imagePreviewAction->setEnabled(hasAtLeastCurrent);
    d->imageViewAction->setEnabled(hasAtLeastCurrent);
    d->imageFindSimilarAction->setEnabled(selectionCount == 1);
    d->imageRenameAction->setEnabled(selectionCount > 0);
    d->imageLightTableAction->setEnabled(selectionCount > 0);
    d->imageAddLightTableAction->setEnabled(selectionCount > 0);
    d->imageAddCurrentQueueAction->setEnabled((selectionCount > 0) && !QueueMgrWindow::queueManagerWindow()->isBusy());
    d->imageAddNewQueueAction->setEnabled((selectionCount > 0) && !QueueMgrWindow::queueManagerWindow()->isBusy());
    d->imageWriteMetadataAction->setEnabled(selectionCount > 0);
    d->imageReadMetadataAction->setEnabled(selectionCount > 0);
    d->imageDeleteAction->setEnabled(selectionCount > 0);
    d->imageRotateActionMenu->setEnabled(selectionCount > 0);
    d->imageFlipActionMenu->setEnabled(selectionCount > 0);
    d->imageExifOrientationActionMenu->setEnabled(selectionCount > 0);
    d->slideShowSelectionAction->setEnabled(selectionCount > 0);
    d->moveSelectionToAlbumAction->setEnabled(selectionCount > 0);
    d->cutItemsAction->setEnabled(selectionCount > 0);
    d->copyItemsAction->setEnabled(selectionCount > 0);
    m_metadataEditAction->setEnabled(selectionCount > 0);
    d->openWithAction->setEnabled(selectionCount > 0);
    d->imageAutoExifActionMenu->setEnabled(selectionCount > 0);

#ifdef HAVE_MARBLE
    m_geolocationEditAction->setEnabled(selectionCount > 0);
#endif

    if (selectionCount > 0)
    {
        d->imageWriteMetadataAction->setText(i18np("Write Metadata to File",
                                                   "Write Metadata to Selected Files", selectionCount));
        d->imageReadMetadataAction->setText(i18np("Reread Metadata From File",
                                                  "Reread Metadata From Selected Files", selectionCount));

        slotResetExifOrientationActions();
    }
}

void DigikamApp::slotExit()
{
    close();
}

void DigikamApp::downloadImages( const QString& folder )
{
    if (!folder.isNull())
    {
        // activate window when called by media menu and DCOP
        if (isMinimized())
        {
            KWindowSystem::unminimizeWindow(winId());
        }

        KWindowSystem::activateWindow(winId());

        emit queuedOpenCameraUiFromPath(folder);
    }
}

void DigikamApp::cameraAutoDetect()
{
    // activate window when called by media menu and DCOP
    if (isMinimized())
    {
        KWindowSystem::unminimizeWindow(winId());
    }

    KWindowSystem::activateWindow(winId());

    slotCameraAutoDetect();
}

void DigikamApp::loadCameras()
{
    KActionCollection *ac = actionCollection();

    d->cameraMenu->setTitle(i18n("Cameras"));
    d->cameraMenu->setIcon(QIcon::fromTheme(QLatin1String("camera-photo")));
    d->usbMediaMenu->setTitle(i18n("USB Storage Devices"));
    d->usbMediaMenu->setIcon(QIcon::fromTheme(QLatin1String("drive-removable-media")));
    d->cardReaderMenu->setTitle(i18n("Card Readers"));
    d->cardReaderMenu->setIcon(QIcon::fromTheme(QLatin1String("media-flash-sd-mmc")));

    ac->addAction(QLatin1String("cameras"),     d->cameraMenu->menuAction());
    ac->addAction(QLatin1String("usb_media"),   d->usbMediaMenu->menuAction());
    ac->addAction(QLatin1String("card_reader"), d->cardReaderMenu->menuAction());

    // -----------------------------------------------------------------

    d->addImagesAction = new QAction(QIcon::fromTheme(QLatin1String("document-import")), i18n("Add Images..."), this);
    d->addImagesAction->setWhatsThis(i18n("Adds new items to an Album."));
    connect(d->addImagesAction, SIGNAL(triggered()), this, SLOT(slotImportAddImages()));
    ac->addAction(QLatin1String("import_addImages"), d->addImagesAction);
    ac->setDefaultShortcut(d->addImagesAction, Qt::CTRL+Qt::ALT+Qt::Key_I);

    // -----------------------------------------------------------------

    d->addFoldersAction = new QAction(QIcon::fromTheme(QLatin1String("folder-new")), i18n("Add Folders..."), this);
    d->addFoldersAction->setWhatsThis(i18n("Adds new folders to Album library."));
    connect(d->addFoldersAction, SIGNAL(triggered()), this, SLOT(slotImportAddFolders()));
    ac->addAction(QLatin1String("import_addFolders"), d->addFoldersAction);

    // -- fill manually added cameras ----------------------------------

    d->cameraList->load();

    // -- scan Solid devices -------------------------------------------

    fillSolidMenus();

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
            this, SLOT(slotSolidDeviceChanged(QString)));

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
            this, SLOT(slotSolidDeviceChanged(QString)));

    // -- queued connections -------------------------------------------

    connect(this, SIGNAL(queuedOpenCameraUiFromPath(QString)),
            this, SLOT(slotOpenCameraUiFromPath(QString)),
            Qt::QueuedConnection);

    connect(this, SIGNAL(queuedOpenSolidDevice(QString)),
            this, SLOT(slotOpenSolidDevice(QString)),
            Qt::QueuedConnection);
}

void DigikamApp::slotCameraAdded(CameraType* ctype)
{
    if (!ctype)
    {
        return;
    }

    QAction* const cAction = new QAction(QIcon::fromTheme(QLatin1String("camera-photo")), ctype->title(), d->manualCameraActionGroup);
    cAction->setData(ctype->title());
    actionCollection()->addAction(ctype->title(), cAction);

    ctype->setAction(cAction);
    updateCameraMenu();
    updateQuickImportAction();
}

void DigikamApp::slotCameraRemoved(QAction* cAction)
{
    if (cAction)
    {
        d->manualCameraActionGroup->removeAction(cAction);
    }

    updateCameraMenu();
    updateQuickImportAction();
}

void DigikamApp::slotCameraAutoDetect()
{
    bool retry = false;

    CameraType* const ctype = d->cameraList->autoDetect(retry);

    if (!ctype && retry)
    {
        QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
        return;
    }

    if (ctype && ctype->action())
    {
        ctype->action()->activate(QAction::Trigger);
    }
}

void DigikamApp::slotOpenCameraUiFromPath(const QString& path)
{
    if (path.isEmpty())
    {
        return;
    }

    // the ImportUI will delete itself when it has finished
    ImportUI* const cgui = new ImportUI(i18n("Images found in %1", path),
                                        QLatin1String("directory browse"), QLatin1String("Fixed"), path, 1);
    cgui->show();

    connect(cgui, SIGNAL(signalLastDestination(QUrl)),
            d->view, SLOT(slotSelectAlbum(QUrl)));
}

void DigikamApp::slotOpenManualCamera(QAction* action)
{
    CameraType* const ctype = d->cameraList->find(action->data().toString());

    if (ctype)
    {
        // check not to open two dialogs for the same camera
        if (ctype->currentImportUI() && !ctype->currentImportUI()->isClosed())
        {
            // show and raise dialog
            if (ctype->currentImportUI()->isMinimized())
            {
                KWindowSystem::unminimizeWindow(ctype->currentImportUI()->winId());
            }

            KWindowSystem::activateWindow(ctype->currentImportUI()->winId());
        }
        else
        {
            // the ImportUI will delete itself when it has finished
            ImportUI* const cgui = new ImportUI(ctype->title(), ctype->model(),
                                                ctype->port(), ctype->path(), ctype->startingNumber());

            ctype->setCurrentImportUI(cgui);

            cgui->show();

            connect(cgui, SIGNAL(signalLastDestination(QUrl)),
                    d->view, SLOT(slotSelectAlbum(QUrl)));
        }
    }
}

void DigikamApp::slotOpenSolidDevice(const QString& udi)
{
    // Identifies device as either Camera or StorageAccess and calls methods accordingly

    Solid::Device device(udi);

    if (!device.isValid())
    {
        QMessageBox::critical(this, qApp->applicationName(), i18n("The specified device (\"%1\") is not valid.", udi));
        return;
    }

    if (device.is<Solid::StorageAccess>())
    {
        openSolidUsmDevice(udi);
    }
    else if (device.is<Solid::Camera>())
    {
        if (!checkSolidCamera(device))
        {
            QMessageBox::critical(this, qApp->applicationName(), i18n("The specified camera (\"%1\") is not supported.", udi));
            return;
        }

        openSolidCamera(udi);
    }
}

void DigikamApp::slotOpenSolidCamera(QAction* action)
{
    QString udi = action->data().toString();
    openSolidCamera(udi, action->iconText());
}

void DigikamApp::openSolidCamera(const QString& udi, const QString& cameraLabel)
{
    // if there is already an open ImportUI for the device, show and raise it, and be done
    if (d->cameraUIMap.contains(udi))
    {
        ImportUI* const ui = d->cameraUIMap.value(udi);

        if (ui && !ui->isClosed())
        {
            if (ui->isMinimized())
            {
                KWindowSystem::unminimizeWindow(ui->winId());
            }

            KWindowSystem::activateWindow(ui->winId());
            return;
        }
    }

    // recreate device from unambiguous UDI
    Solid::Device device(udi);

    if ( device.isValid() )
    {
        if (cameraLabel.isNull())
        {
            QString label = labelForSolidCamera(device);
        }

        Solid::Camera* const camera = device.as<Solid::Camera>();
        QList<QVariant> list = camera->driverHandle(QLatin1String("gphoto")).toList();

        // all sanity checks have already been done when creating the action
        if (list.size() < 3)
        {
            return;
        }

        // NOTE: See bug #262296: With KDE 4.6, Solid API return device vendor id
        // and product id in hexadecimal strings.
        bool ok;
        int vendorId  = list.at(1).toString().toInt(&ok, 16);
        int productId = list.at(2).toString().toInt(&ok, 16);
        QString model, port;

        if (CameraList::findConnectedCamera(vendorId, productId, model, port))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Found camera from ids " << vendorId << " " << productId
                                         << " camera is: " << model << " at " << port;

            // the ImportUI will delete itself when it has finished
            ImportUI* const cgui = new ImportUI(cameraLabel, model, port, QLatin1String("/"), 1);
            d->cameraUIMap[udi]  = cgui;

            cgui->show();

            connect(cgui, SIGNAL(signalLastDestination(QUrl)),
                    d->view, SLOT(slotSelectAlbum(QUrl)));
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to detect camera with GPhoto2 from Solid information";
        }
    }
}

void DigikamApp::slotOpenSolidUsmDevice(QAction* action)
{
    QString udi = action->data().toString();
    openSolidUsmDevice(udi, action->iconText());
}

void DigikamApp::openSolidUsmDevice(const QString& udi, const QString& givenLabel)
{
    QString mediaLabel = givenLabel;

    // if there is already an open ImportUI for the device, show and raise it
    if (d->cameraUIMap.contains(udi))
    {
        ImportUI* const ui = d->cameraUIMap.value(udi);

        if (ui && !ui->isClosed())
        {
            if (ui->isMinimized())
            {
                KWindowSystem::unminimizeWindow(ui->winId());
            }

            KWindowSystem::activateWindow(ui->winId());
            return;
        }
    }

    // recreate device from unambiguous UDI
    Solid::Device device(udi);

    if ( device.isValid() )
    {
        Solid::StorageAccess* const access = device.as<Solid::StorageAccess>();

        if (!access)
        {
            return;
        }

        if (!access->isAccessible())
        {
            QApplication::setOverrideCursor(Qt::WaitCursor);

            if (!access->setup())
            {
                return;
            }

            d->eventLoop = new QEventLoop(this);

            connect(access, SIGNAL(setupDone(Solid::ErrorType,QVariant,QString)),
                    this, SLOT(slotSolidSetupDone(Solid::ErrorType,QVariant,QString)));

            int returnCode = d->eventLoop->exec(QEventLoop::ExcludeUserInputEvents);

            delete d->eventLoop;
            d->eventLoop = 0;
            QApplication::restoreOverrideCursor();

            if (returnCode == 1)
            {
                QMessageBox::critical(this, qApp->applicationName(), d->solidErrorMessage);
                return;
            }
        }

        // Create Camera UI

        QString path = QDir::fromNativeSeparators(access->filePath());

        if (mediaLabel.isNull())
        {
            mediaLabel = path;
        }

        // the ImportUI will delete itself when it has finished
        ImportUI* const cgui = new ImportUI(i18n("Images on %1", mediaLabel),
                                            QLatin1String("directory browse"), QLatin1String("Fixed"), path, 1);
        d->cameraUIMap[udi]  = cgui;

        cgui->show();

        connect(cgui, SIGNAL(signalLastDestination(QUrl)),
                d->view, SLOT(slotSelectAlbum(QUrl)));
    }
}

void DigikamApp::slotSolidSetupDone(Solid::ErrorType errorType, QVariant errorData, const QString& /*udi*/)
{
    if (!d->eventLoop)
    {
        return;
    }

    if (errorType == Solid::NoError)
    {
        d->eventLoop->exit(0);
    }
    else
    {
        d->solidErrorMessage = i18n("Cannot access the storage device.\n");
        d->solidErrorMessage += errorData.toString();
        d->eventLoop->exit(1);
    }
}

void DigikamApp::slotSolidDeviceChanged(const QString& udi)
{
    Q_UNUSED(udi)
    fillSolidMenus();
}

bool DigikamApp::checkSolidCamera(const Solid::Device& cameraDevice)
{
    const Solid::Camera* const camera = cameraDevice.as<Solid::Camera>();

    if (!camera)
    {
        return false;
    }

    QStringList drivers = camera->supportedDrivers();

    qCDebug(DIGIKAM_GENERAL_LOG) << "fillSolidMenus: Found Camera "
                                 << QString::fromUtf8("%1 %2").arg(cameraDevice.vendor()).arg(cameraDevice.product())
                                 << " protocols " << camera->supportedProtocols()
                                 << " drivers " << camera->supportedDrivers(QLatin1String("ptp"));

    // We handle gphoto2 cameras in this loop
    if (! (camera->supportedDrivers().contains(QLatin1String("gphoto")) || camera->supportedProtocols().contains(QLatin1String("ptp"))) )
    {
        return false;
    }

    QVariant driverHandle = camera->driverHandle(QLatin1String("gphoto"));

    if (!driverHandle.canConvert(QVariant::List))
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Solid returns unsupported driver handle for gphoto2";
        return false;
    }

    QList<QVariant> driverHandleList = driverHandle.toList();

    if (driverHandleList.size() < 3 || driverHandleList.at(0).toString() != QLatin1String("usb")
        || !driverHandleList.at(1).canConvert(QVariant::Int)
        || !driverHandleList.at(2).canConvert(QVariant::Int)
       )
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Solid returns unsupported driver handle for gphoto2";
        return false;
    }

    return true;
}

QString DigikamApp::labelForSolidCamera(const Solid::Device& cameraDevice)
{
    QString vendor  = cameraDevice.vendor();
    QString product = cameraDevice.product();

    if (product == QLatin1String("USB Imaging Interface") || product == QLatin1String("USB Vendor Specific Interface"))
    {
        Solid::Device parentUsbDevice = cameraDevice.parent();

        if (parentUsbDevice.isValid())
        {
            vendor  = parentUsbDevice.vendor();
            product = parentUsbDevice.product();

            if (!vendor.isEmpty() && !product.isEmpty())
            {
                if (vendor == QLatin1String("Canon, Inc."))
                {
                    vendor = QLatin1String("Canon");

                    if (product.startsWith(QLatin1String("Canon ")))
                    {
                        product = product.mid(6);    // cut off another "Canon " from product
                    }

                    if (product.endsWith(QLatin1String(" (ptp)")))
                    {
                        product.chop(6);             // cut off " (ptp)"
                    }
                }
                else if (vendor == QLatin1String("Fuji Photo Film Co., Ltd"))
                {
                    vendor = QLatin1String("Fuji");
                }
                else if (vendor == QLatin1String("Nikon Corp."))
                {
                    vendor = QLatin1String("Nikon");

                    if (product.startsWith(QLatin1String("NIKON ")))
                    {
                        product = product.mid(6);
                    }
                }
            }
        }
    }

    return vendor + QLatin1Char(' ') + product;
}

void DigikamApp::fillSolidMenus()
{
    QHash<QString, QDateTime> newAppearanceTimes;
    d->usbMediaMenu->clear();
    d->cardReaderMenu->clear();

    // delete the actionGroups to avoid duplicate menu entries
    delete d->solidUsmActionGroup;
    delete d->solidCameraActionGroup;

    d->solidCameraActionGroup = new QActionGroup(this);
    connect(d->solidCameraActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenSolidCamera(QAction*)));

    d->solidUsmActionGroup = new QActionGroup(this);
    connect(d->solidUsmActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenSolidUsmDevice(QAction*)));

    // --------------------------------------------------------

    QList<Solid::Device> cameraDevices = Solid::Device::listFromType(Solid::DeviceInterface::Camera);

    foreach(const Solid::Device& cameraDevice, cameraDevices)
    {
        // USM camera: will be handled below
        if (cameraDevice.is<Solid::StorageAccess>())
        {
            continue;
        }

        if (!checkSolidCamera(cameraDevice))
        {
            continue;
        }

        // --------------------------------------------------------

        QString l     = labelForSolidCamera(cameraDevice);
        QString label = CameraNameHelper::cameraNameAutoDetected(l.trimmed());

        // --------------------------------------------------------

        QString iconName = cameraDevice.icon();

        if (iconName.isEmpty())
        {
            iconName = QLatin1String("camera-photo");
        }

        QAction* const action = new QAction(label, d->solidCameraActionGroup);

        action->setIcon(QIcon::fromTheme(iconName));
        // set data to identify device in action slot slotSolidSetupDevice
        action->setData(cameraDevice.udi());
        newAppearanceTimes[cameraDevice.udi()] = d->cameraAppearanceTimes.contains(cameraDevice.udi()) ?
                                                 d->cameraAppearanceTimes.value(cameraDevice.udi()) : QDateTime::currentDateTime();

        d->cameraMenu->addAction(action);
    }

    QList<Solid::Device> storageDevices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);

    foreach(const Solid::Device& accessDevice, storageDevices)
    {
        // check for StorageAccess
        if (!accessDevice.is<Solid::StorageAccess>())
        {
            continue;
        }

        // check for StorageDrive
        Solid::Device driveDevice;

        for (Solid::Device currentDevice = accessDevice; currentDevice.isValid(); currentDevice = currentDevice.parent())
        {
            if (currentDevice.is<Solid::StorageDrive>())
            {
                driveDevice = currentDevice;
                break;
            }
        }

        if (!driveDevice.isValid())
        {
            continue;
        }

        const Solid::StorageDrive* const drive = driveDevice.as<Solid::StorageDrive>();

        QString driveType;

        bool isHarddisk = false;

        switch (drive->driveType())
        {
                // skip these
            case Solid::StorageDrive::CdromDrive:
            case Solid::StorageDrive::Floppy:
            case Solid::StorageDrive::Tape:
            default:
                continue;
                // accept card readers
            case Solid::StorageDrive::CompactFlash:
                driveType = i18n("CompactFlash Card Reader");
                break;
            case Solid::StorageDrive::MemoryStick:
                driveType = i18n("Memory Stick Reader");
                break;
            case Solid::StorageDrive::SmartMedia:
                driveType = i18n("SmartMedia Card Reader");
                break;
            case Solid::StorageDrive::SdMmc:
                driveType = i18n("SD / MMC Card Reader");
                break;
            case Solid::StorageDrive::Xd:
                driveType = i18n("xD Card Reader");
                break;
            case Solid::StorageDrive::HardDisk:

                // We don't want to list HardDisk partitions, but USB Mass Storage devices.
                // Don't know what is the exact difference between removable and hotpluggable.
                if (drive->isRemovable() || drive->isHotpluggable())
                {
                    isHarddisk = true;

                    if (drive->bus() == Solid::StorageDrive::Usb)
                    {
                        driveType = i18n("USB Disk");
                    }
                    else
                    {
                        driveType = i18nc("non-USB removable storage device", "Disk");
                    }

                    break;
                }
                else
                {
                    continue;
                }
        }

        // check for StorageVolume
        Solid::Device volumeDevice;

        for (Solid::Device currentDevice = accessDevice; currentDevice.isValid(); currentDevice = currentDevice.parent())
        {
            if (currentDevice.is<Solid::StorageVolume>())
            {
                volumeDevice = currentDevice;
                break;
            }
        }

        if (!volumeDevice.isValid())
        {
            continue;
        }

        bool isCamera                            = accessDevice.is<Solid::Camera>();
        const Solid::StorageAccess* const access = accessDevice.as<Solid::StorageAccess>();
        const Solid::StorageVolume* const volume = volumeDevice.as<Solid::StorageVolume>();

        if (volume->isIgnored())
        {
            continue;
        }

        QString label;

        if (isCamera)
        {
            label = accessDevice.vendor() + QLatin1Char(' ') + accessDevice.product();
        }
        else
        {
            QString labelOrProduct;

            if (!volume->label().isEmpty())
            {
                labelOrProduct = volume->label();
            }
            else if (!volumeDevice.product().isEmpty())
            {
                labelOrProduct = volumeDevice.product();
            }
            else if (!volumeDevice.vendor().isEmpty())
            {
                labelOrProduct = volumeDevice.vendor();
            }
            else if (!driveDevice.product().isEmpty())
            {
                labelOrProduct = driveDevice.product();
            }

            if (!labelOrProduct.isNull())
            {
                if (!access->filePath().isEmpty())
                {
                    label += i18nc("<drive type> \"<device name or label>\" at <mount path>",
                                   "%1 \"%2\" at %3", driveType, labelOrProduct, QDir::toNativeSeparators(access->filePath()));
                }
                else
                {
                    label += i18nc("<drive type> \"<device name or label>\"",
                                   "%1 \"%2\"", driveType, labelOrProduct);
                }
            }
            else
            {
                if (!access->filePath().isEmpty())
                {
                    label += i18nc("<drive type> at <mount path>",
                                   "%1 at %2", driveType, QDir::toNativeSeparators(access->filePath()));
                }
                else
                {
                    label += driveType;
                }
            }

            if (volume->size())
            {
                label += i18nc("device label etc... (<formatted byte size>)",
                               " (%1)", ImagePropertiesTab::humanReadableBytesCount(volume->size()));
            }
        }

        QString iconName;

        if (!driveDevice.icon().isEmpty())
        {
            iconName = driveDevice.icon();
        }
        else if (!accessDevice.icon().isEmpty())
        {
            iconName = accessDevice.icon();
        }
        else if (!volumeDevice.icon().isEmpty())
        {
            iconName = volumeDevice.icon();
        }

        QAction* const action = new QAction(label, d->solidUsmActionGroup);

        if (!iconName.isEmpty())
        {
            action->setIcon(QIcon::fromTheme(iconName));
        }

        // set data to identify device in action slot slotSolidSetupDevice
        action->setData(accessDevice.udi());
        newAppearanceTimes[accessDevice.udi()] = d->cameraAppearanceTimes.contains(accessDevice.udi()) ?
                                                 d->cameraAppearanceTimes.value(accessDevice.udi()) : QDateTime::currentDateTime();

        if (isCamera)
        {
            d->cameraMenu->addAction(action);
        }

        if (isHarddisk)
        {
            d->usbMediaMenu->addAction(action);
        }
        else
        {
            d->cardReaderMenu->addAction(action);
        }
    }

/*
    //TODO: Find best usable solution when no devices are connected: One entry, hide, or disable?

    // Add one entry telling that no device is available
    if (d->cameraSolidMenu->isEmpty())
    {
        QAction* const action = d->cameraSolidMenu->addAction(i18n("No Camera Connected"));
        action->setEnabled(false);
    }
    if (d->usbMediaMenu->isEmpty())
    {
        QAction* const action = d->usbMediaMenu->addAction(i18n("No Storage Devices Found"));
        action->setEnabled(false);
    }
    if (d->cardReaderMenu->isEmpty())
    {
        QAction* const action = d->cardReaderMenu->addAction(i18n("No Card Readers Available"));
        action->setEnabled(false);
    }

    // hide empty menus
    d->cameraSolidMenu->menuAction()->setVisible(!d->cameraSolidMenu->isEmpty());
    d->usbMediaMenu->menuAction()->setVisible(!d->usbMediaMenu->isEmpty());
    d->cardReaderMenu->menuAction()->setVisible(!d->cardReaderMenu->isEmpty());
*/

    d->cameraAppearanceTimes = newAppearanceTimes;

    // disable empty menus
    d->usbMediaMenu->setEnabled(!d->usbMediaMenu->isEmpty());
    d->cardReaderMenu->setEnabled(!d->cardReaderMenu->isEmpty());

    updateCameraMenu();
    updateQuickImportAction();
}

void DigikamApp::slotSetup()
{
    setup();
}

bool DigikamApp::setup()
{
    return Setup::execDialog(this, Setup::LastPageUsed);
}

bool DigikamApp::setupICC()
{
    return Setup::execSinglePage(this, Setup::ICCPage);
}

void DigikamApp::slotSetupCamera()
{
    Setup::execSinglePage(this, Setup::CameraPage);
}

void DigikamApp::slotSetupChanged()
{
    // raw loading options might have changed
    LoadingCacheInterface::cleanCache();

    // TODO: clear history when location changed
    //if(ApplicationSettings::instance()->getAlbumLibraryPath() != AlbumManager::instance()->getLibraryPath())
    //  d->view->clearHistory();

    const DbEngineParameters prm = ApplicationSettings::instance()->getDbEngineParameters();

    if (!AlbumManager::instance()->databaseEqual(prm))
    {
        AlbumManager::instance()->changeDatabase(ApplicationSettings::instance()->getDbEngineParameters());
    }

    if (ApplicationSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        AlbumManager::instance()->prepareItemCounts();
    }

    // Load full-screen options
    KConfigGroup group = KSharedConfig::openConfig()->group(configGroupName());
    readFullScreenSettings(group);

    d->view->applySettings();

    AlbumThumbnailLoader::instance()->setThumbnailSize(ApplicationSettings::instance()->getTreeViewIconSize());

    if (LightTableWindow::lightTableWindowCreated())
    {
        LightTableWindow::lightTableWindow()->applySettings();
    }

    if (QueueMgrWindow::queueManagerWindowCreated())
    {
        QueueMgrWindow::queueManagerWindow()->applySettings();
    }

    d->config->sync();
}

void DigikamApp::slotEditKeys()
{
    editKeyboardShortcuts();
}

void DigikamApp::slotDBStat()
{
    showDigikamDatabaseStat();
}

void DigikamApp::populateThemes()
{
    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Loading themes..."));
    }

    ThemeManager::instance()->setThemeMenuAction(new QMenu(i18n("&Themes"), this));
    ThemeManager::instance()->registerThemeActions(this);
    ThemeManager::instance()->setCurrentTheme(ApplicationSettings::instance()->getCurrentTheme());

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

void DigikamApp::slotThemeChanged()
{
    ApplicationSettings::instance()->setCurrentTheme(ThemeManager::instance()->currentThemeName());
}

void DigikamApp::preloadWindows()
{
    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Loading tools..."));
    }

    QueueMgrWindow::queueManagerWindow();
    ImageWindow::imageWindow();
    LightTableWindow::lightTableWindow();

    d->tagsActionManager->registerTagsActionCollections();
}

void DigikamApp::slotDatabaseMigration()
{
    DatabaseMigrationDialog dlg(this);
    dlg.exec();
}

void DigikamApp::slotMaintenance()
{
    MaintenanceDlg* const dlg = new MaintenanceDlg(this);

    if (dlg->exec() == QDialog::Accepted)
    {
        d->maintenanceAction->setEnabled(false);

        MaintenanceMngr* const mngr = new MaintenanceMngr(this);

        connect(mngr, SIGNAL(signalComplete()),
                this, SLOT(slotMaintenanceDone()));

        mngr->setSettings(dlg->settings());
    }
}

void DigikamApp::slotMaintenanceDone()
{
    d->maintenanceAction->setEnabled(true);
    d->view->refreshView();

    if (LightTableWindow::lightTableWindowCreated())
    {
        LightTableWindow::lightTableWindow()->refreshView();
    }

    if (QueueMgrWindow::queueManagerWindowCreated())
    {
        QueueMgrWindow::queueManagerWindow()->refreshView();
    }
}

void DigikamApp::slotExpoBlending()
{
    ExpoBlendingManager::instance()->checkBinaries();
    ExpoBlendingManager::instance()->setItemsList(view()->selectedUrls(ApplicationSettings::Tools));
    ExpoBlendingManager::instance()->run();
}

void DigikamApp::slotPanorama()
{
#ifdef HAVE_PANORAMA
    PanoManager::instance()->checkBinaries();
    PanoManager::instance()->setItemsList(view()->selectedUrls(ApplicationSettings::Tools));
    PanoManager::instance()->run();
#endif
}

void DigikamApp::slotVideoSlideshow()
{
#ifdef HAVE_MEDIAPLAYER
    VidSlideWizard w(this, new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools));
    w.exec();
#endif
}

void DigikamApp::slotHtmlGallery()
{
#ifdef HAVE_HTMLGALLERY
    HTMLWizard w(this, new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools));
    w.exec();
#endif
}

void DigikamApp::slotCalendar()
{
    CalWizard w(view()->selectedUrls(ApplicationSettings::Tools), this);
    w.exec();
}

void DigikamApp::slotSendByMail()
{
    MailWizard w(this, new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools));
    w.exec();
}

void DigikamApp::slotPrintCreator()
{
    AdvPrintWizard w(this, new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools));
    w.exec();
}

void DigikamApp::slotMediaServer()
{
    DBInfoIface* const iface = new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools);
    // NOTE: We overwrite the default albums chooser object name for load save check items state between sessions.
    // The goal is not mix these settings with other export tools.
    iface->setObjectName(QLatin1String("SetupMediaServerIface"));

    DMediaServerDlg w(this, iface);
    w.exec();
}

void DigikamApp::slotRecurseAlbums(bool checked)
{
    d->view->setRecurseAlbums(checked);
}

void DigikamApp::slotRecurseTags(bool checked)
{
    d->view->setRecurseTags(checked);
}

void DigikamApp::slotZoomSliderChanged(int size)
{
    d->view->setThumbSize(size);
}

void DigikamApp::slotThumbSizeChanged(int size)
{
    d->zoomBar->setThumbsSize(size);

    if (!fullScreenIsActive() && d->autoShowZoomToolTip)
    {
        d->zoomBar->triggerZoomTrackerToolTip();
    }
}

void DigikamApp::slotZoomChanged(double zoom)
{
    double zmin = d->view->zoomMin();
    double zmax = d->view->zoomMax();
    d->zoomBar->setZoom(zoom, zmin, zmax);

    if (!fullScreenIsActive() && d->autoShowZoomToolTip)
    {
        d->zoomBar->triggerZoomTrackerToolTip();
    }
}

void DigikamApp::slotImportAddImages()
{
    QString startingPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QUrl url             = DFileDialog::getExistingDirectoryUrl(this, i18n("Select folder to parse"),
                                                                QUrl::fromLocalFile(startingPath));

    if (url.isEmpty() || !url.isLocalFile())
    {
        return;
    }

    // The folder contents will be parsed by Camera interface in "Directory Browse" mode.
    downloadFrom(url.toLocalFile());
}

void DigikamApp::slotImportAddFolders()
{
    // NOTE: QFileDialog don't have an option to permit multiple selection of directories.
    // This work around is inspired from http://www.qtcentre.org/threads/34226-QFileDialog-select-multiple-directories
    // Check Later Qt 5.4 if a new native Qt way have been introduced.

    QPointer<DFileDialog> dlg = new DFileDialog(this);
    dlg->setWindowTitle(i18n("Select folders to import into album"));
    dlg->setFileMode(DFileDialog::DirectoryOnly);

    QListView* const l = dlg->findChild<QListView*>(QLatin1String("listView"));

    if (l)
    {
        l->setSelectionMode(QAbstractItemView::MultiSelection);
    }

    QTreeView* const t = dlg->findChild<QTreeView*>();

    if (t)
    {
        t->setSelectionMode(QAbstractItemView::MultiSelection);
    }

    if (dlg->exec() != QDialog::Accepted)
    {
        delete dlg;
        return;
    }

    QList<QUrl> urls = dlg->selectedUrls();
    delete dlg;

    if (urls.empty())
    {
        return;
    }

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    Album* album = 0;

    if (!albumList.isEmpty())
    {
        album = albumList.first();
    }

    if (album && album->type() != Album::PHYSICAL)
    {
        album = 0;
    }

    QString header(i18n("<p>Please select the destination album from the digiKam library to "
                        "import folders into.</p>"));

    album = AlbumSelectDialog::selectAlbum(this, (PAlbum*)album, header);

    if (!album)
    {
        return;
    }

    PAlbum* const pAlbum = dynamic_cast<PAlbum*>(album);

    if (!pAlbum)
    {
        return;
    }

    DIO::copy(urls, pAlbum);
}

void DigikamApp::slotToggleShowBar()
{
    d->view->toggleShowBar(d->showBarAction->isChecked());
}

void DigikamApp::moveEvent(QMoveEvent*)
{
    emit signalWindowHasMoved();
}

void DigikamApp::updateCameraMenu()
{
    d->cameraMenu->clear();

    foreach(QAction* const action, d->solidCameraActionGroup->actions())
    {
        d->cameraMenu->addAction(action);
    }

    d->cameraMenu->addSeparator();

    foreach(QAction* const action, d->manualCameraActionGroup->actions())
    {
        // remove duplicate entries, prefer manually added cameras
        foreach(QAction* const actionSolid, d->solidCameraActionGroup->actions())
        {
            if (CameraNameHelper::sameDevices(actionSolid->iconText(), action->iconText()))
            {
                d->cameraMenu->removeAction(actionSolid);
                d->solidCameraActionGroup->removeAction(actionSolid);
            }
        }

        d->cameraMenu->addAction(action);
    }

    d->cameraMenu->addSeparator();
    d->cameraMenu->addAction(actionCollection()->action(QLatin1String("camera_add")));
}

void DigikamApp::updateQuickImportAction()
{
    d->quickImportMenu->clear();

    foreach(QAction* const action, d->solidCameraActionGroup->actions())
    {
        d->quickImportMenu->addAction(action);
    }

    foreach(QAction* const action, d->solidUsmActionGroup->actions())
    {
        d->quickImportMenu->addAction(action);
    }

    foreach(QAction* const action, d->manualCameraActionGroup->actions())
    {
        d->quickImportMenu->addAction(action);
    }

    if (d->quickImportMenu->actions().isEmpty())
    {
        d->quickImportMenu->setEnabled(false);
    }
    else
    {
        disconnect(d->quickImportMenu->menuAction(), SIGNAL(triggered()), 0, 0);

        QAction*  primaryAction = 0;
        QDateTime latest;

        foreach(QAction* const action, d->quickImportMenu->actions())
        {
            QDateTime appearanceTime = d->cameraAppearanceTimes.value(action->data().toString());

            if (latest.isNull() || appearanceTime > latest)
            {
                primaryAction = action;
                latest        = appearanceTime;
            }
        }

        if (!primaryAction)
        {
            primaryAction = d->quickImportMenu->actions().first();
        }

        connect(d->quickImportMenu->menuAction(), SIGNAL(triggered()),
                primaryAction, SLOT(trigger()));

        d->quickImportMenu->setEnabled(true);
    }
}

void DigikamApp::setupExifOrientationActions()
{
    KActionCollection* const ac                = actionCollection();
    QSignalMapper* const exifOrientationMapper = new QSignalMapper(d->view);

    connect(exifOrientationMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotImageExifOrientation(int)));

    d->imageExifOrientationActionMenu = new QMenu(i18n("Adjust Exif Orientation Tag"), this);
    ac->addAction(QLatin1String("image_set_exif_orientation"), d->imageExifOrientationActionMenu->menuAction());

    d->imageSetExifOrientation1Action = new QAction(i18nc("normal exif orientation", "Normal"), this);
    d->imageSetExifOrientation1Action->setCheckable(true);
    d->imageSetExifOrientation2Action = new QAction(i18n("Flipped Horizontally"),               this);
    d->imageSetExifOrientation2Action->setCheckable(true);
    d->imageSetExifOrientation3Action = new QAction(i18n("Rotated Upside Down"),                this);
    d->imageSetExifOrientation3Action->setCheckable(true);
    d->imageSetExifOrientation4Action = new QAction(i18n("Flipped Vertically"),                 this);
    d->imageSetExifOrientation4Action->setCheckable(true);
    d->imageSetExifOrientation5Action = new QAction(i18n("Rotated Right / Horiz. Flipped"),     this);
    d->imageSetExifOrientation5Action->setCheckable(true);
    d->imageSetExifOrientation6Action = new QAction(i18n("Rotated Right"),                      this);
    d->imageSetExifOrientation6Action->setCheckable(true);
    d->imageSetExifOrientation7Action = new QAction(i18n("Rotated Right / Vert. Flipped"),      this);
    d->imageSetExifOrientation7Action->setCheckable(true);
    d->imageSetExifOrientation8Action = new QAction(i18n("Rotated Left"),                       this);
    d->imageSetExifOrientation8Action->setCheckable(true);

    d->exifOrientationActionGroup = new QActionGroup(d->imageExifOrientationActionMenu);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation1Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation2Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation3Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation4Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation5Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation6Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation7Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation8Action);
    d->imageSetExifOrientation1Action->setChecked(true);

    ac->addAction(QLatin1String("image_set_exif_orientation_normal"),                    d->imageSetExifOrientation1Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_flipped_horizontal"),        d->imageSetExifOrientation2Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_upside_down"),       d->imageSetExifOrientation3Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_flipped_vertically"),        d->imageSetExifOrientation4Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_right_hor_flipped"), d->imageSetExifOrientation5Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_right"),             d->imageSetExifOrientation6Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_right_ver_flipped"), d->imageSetExifOrientation7Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_left"),              d->imageSetExifOrientation8Action);

    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation1Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation2Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation3Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation4Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation5Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation6Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation7Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation8Action);

    connect(d->imageSetExifOrientation1Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation2Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation3Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation4Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation5Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation6Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation7Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation8Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    exifOrientationMapper->setMapping(d->imageSetExifOrientation1Action, 1);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation2Action, 2);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation3Action, 3);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation4Action, 4);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation5Action, 5);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation6Action, 6);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation7Action, 7);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation8Action, 8);
}

void DigikamApp::slotResetExifOrientationActions()
{
    d->imageSetExifOrientation1Action->setChecked(false);
    d->imageSetExifOrientation2Action->setChecked(false);
    d->imageSetExifOrientation3Action->setChecked(false);
    d->imageSetExifOrientation4Action->setChecked(false);
    d->imageSetExifOrientation5Action->setChecked(false);
    d->imageSetExifOrientation6Action->setChecked(false);
    d->imageSetExifOrientation7Action->setChecked(false);
    d->imageSetExifOrientation8Action->setChecked(false);
}

void DigikamApp::slotSetCheckedExifOrientationAction(const ImageInfo& info)
{
    //DMetadata meta(info.fileUrl().toLocalFile());
    //int orientation = (meta.isEmpty()) ? 0 : meta.getImageOrientation();
    int orientation = info.orientation();

    switch (orientation)
    {
        case 1:
            d->imageSetExifOrientation1Action->setChecked(true);
            break;
        case 2:
            d->imageSetExifOrientation2Action->setChecked(true);
            break;
        case 3:
            d->imageSetExifOrientation3Action->setChecked(true);
            break;
        case 4:
            d->imageSetExifOrientation4Action->setChecked(true);
            break;
        case 5:
            d->imageSetExifOrientation5Action->setChecked(true);
            break;
        case 6:
            d->imageSetExifOrientation6Action->setChecked(true);
            break;
        case 7:
            d->imageSetExifOrientation7Action->setChecked(true);
            break;
        case 8:
            d->imageSetExifOrientation8Action->setChecked(true);
            break;
        default:
            slotResetExifOrientationActions();
            break;
    }
}

void DigikamApp::setupImageTransformActions()
{
    KActionCollection* const ac = actionCollection();

    d->imageRotateActionMenu = new QMenu(i18n("Rotate"), this);
    d->imageRotateActionMenu->setIcon(QIcon::fromTheme(QLatin1String("object-rotate-right")));

    QAction* const left = ac->addAction(QLatin1String("rotate_ccw"));
    left->setText(i18nc("rotate image left", "Left"));
    ac->setDefaultShortcut(left, Qt::SHIFT+Qt::CTRL+Qt::Key_Left);
    connect(left, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageRotateActionMenu->addAction(left);

    QAction* const right = ac->addAction(QLatin1String("rotate_cw"));
    right->setText(i18nc("rotate image right", "Right"));
    ac->setDefaultShortcut(right, Qt::SHIFT+Qt::CTRL+Qt::Key_Right);
    connect(right, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageRotateActionMenu->addAction(right);

    ac->addAction(QLatin1String("image_rotate"), d->imageRotateActionMenu->menuAction());

    // -----------------------------------------------------------------------------------

    d->imageFlipActionMenu = new QMenu(i18n("Flip"), this);
    d->imageFlipActionMenu->setIcon(QIcon::fromTheme(QLatin1String("flip-horizontal")));

    QAction* const hori = ac->addAction(QLatin1String("flip_horizontal"));
    hori->setText(i18n("Horizontally"));
    ac->setDefaultShortcut(hori, Qt::CTRL+Qt::Key_Asterisk);
    connect(hori, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageFlipActionMenu->addAction(hori);

    QAction* const verti = ac->addAction(QLatin1String("flip_vertical"));
    verti->setText(i18n("Vertically"));
    ac->setDefaultShortcut(verti, Qt::CTRL+Qt::Key_Slash);
    connect(verti, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageFlipActionMenu->addAction(verti);

    ac->addAction(QLatin1String("image_flip"), d->imageFlipActionMenu->menuAction());

    // -----------------------------------------------------------------------------------

    d->imageAutoExifActionMenu = new QAction(i18n("Auto Rotate/Flip Using Exif Information"), this);
    connect(d->imageAutoExifActionMenu, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));

    ac->addAction(QLatin1String("image_transform_exif"), d->imageAutoExifActionMenu);
}

void DigikamApp::slotTransformAction()
{
    if (sender()->objectName() == QLatin1String("rotate_ccw"))
    {
        d->view->imageTransform(MetaEngineRotation::Rotate270);
    }
    else if (sender()->objectName() == QLatin1String("rotate_cw"))
    {
        d->view->imageTransform(MetaEngineRotation::Rotate90);
    }
    else if (sender()->objectName() == QLatin1String("flip_horizontal"))
    {
        d->view->imageTransform(MetaEngineRotation::FlipHorizontal);
    }
    else if (sender()->objectName() == QLatin1String("flip_vertical"))
    {
        d->view->imageTransform(MetaEngineRotation::FlipVertical);
    }
    else if (sender()->objectName() == QLatin1String("image_transform_exif"))
    {
        // special value for FileActionMngr
        d->view->imageTransform(MetaEngineRotation::NoTransformation);
    }
}

QMenu* DigikamApp::slideShowMenu() const
{
    return d->slideShowAction;
}

void DigikamApp::rebuild()
{
    QString file = xmlFile();
    if (!file.isEmpty())
    {
        setXMLGUIBuildDocument(QDomDocument());
        loadStandardsXmlFile();
        setXMLFile(file, true);
    }
}

void DigikamApp::showSideBars(bool visible)
{
    visible ? d->view->showSideBars()
            : d->view->hideSideBars();
}

void DigikamApp::slotToggleLeftSideBar()
{
    d->view->toggleLeftSidebar();
}

void DigikamApp::slotToggleRightSideBar()
{
    d->view->toggleRightSidebar();
}

void DigikamApp::slotPreviousLeftSideBarTab()
{
    d->view->previousLeftSideBarTab();
}

void DigikamApp::slotNextLeftSideBarTab()
{
    d->view->nextLeftSideBarTab();
}

void DigikamApp::slotNextRightSideBarTab()
{
    d->view->nextRightSideBarTab();
}

void DigikamApp::slotPreviousRightSideBarTab()
{
    d->view->previousRightSideBarTab();
}

void DigikamApp::showThumbBar(bool visible)
{
    view()->toggleShowBar(visible);
}

bool DigikamApp::thumbbarVisibility() const
{
    return d->showBarAction->isChecked();
}

void DigikamApp::slotSwitchedToPreview()
{
    d->imagePreviewAction->setChecked(true);
    d->zoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    toggleShowBar();
}

void DigikamApp::slotSwitchedToIconView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    d->imageIconViewAction->setChecked(true);
    toggleShowBar();
}

void DigikamApp::slotSwitchedToMapView()
{
    //TODO: Link to map view's zoom actions
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
#ifdef HAVE_MARBLE
    d->imageMapViewAction->setChecked(true);
#endif // HAVE_MARBLE
    toggleShowBar();
}

void DigikamApp::slotSwitchedToTableView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    d->imageTableViewAction->setChecked(true);
    toggleShowBar();
}

void DigikamApp::slotSwitchedToTrashView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    // TODO: disable all other views
    toggleShowBar();
}

void DigikamApp::customizedFullScreenMode(bool set)
{
    toolBarMenuAction()->setEnabled(!set);
    showMenuBarAction()->setEnabled(!set);
    showStatusBarAction()->setEnabled(!set);
    set ? d->showBarAction->setEnabled(false)
        : toggleShowBar();

    d->view->toggleFullScreen(set);
}

void DigikamApp::toggleShowBar()
{
    switch (d->view->viewMode())
    {
        case StackedView::PreviewImageMode:
        case StackedView::MediaPlayerMode:
            d->showBarAction->setEnabled(true);
            break;

        default:
            d->showBarAction->setEnabled(false);
            break;
    }
}

void DigikamApp::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void DigikamApp::slotToggleColorManagedView()
{
    if (!IccSettings::instance()->isEnabled())
    {
        return;
    }

    bool cmv = !IccSettings::instance()->settings().useManagedPreviews;
    IccSettings::instance()->setUseManagedPreviews(cmv);
}

void DigikamApp::slotColorManagementOptionsChanged()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    d->viewCMViewAction->blockSignals(true);
    d->viewCMViewAction->setEnabled(settings.enableCM);
    d->viewCMViewAction->setChecked(settings.useManagedPreviews);
    d->viewCMViewAction->blockSignals(false);
}

QString DigikamApp::scannerTargetPlace()
{
    QString place    = QDir::homePath();
    QStringList pics = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    if (!pics.isEmpty())
        place = pics.first();

    Album* const album = AlbumManager::instance()->currentAlbums().first();

    if (album->type() == Album::PHYSICAL)
    {
        PAlbum* const p = dynamic_cast<PAlbum*>(album);

        if (p)
        {
            place = p->folderPath();
        }
    }
    else
    {
        QStringList cols = CollectionManager::instance()->allAvailableAlbumRootPaths();

        if (!cols.isEmpty())
            place = cols.first();
    }

    return place;
}

void DigikamApp::slotEditGeolocation()
{
#ifdef HAVE_MARBLE
    ImageInfoList infos = d->view->selectedInfoList(ApplicationSettings::Metadata);

    if (infos.isEmpty())
        return;

    TagModel* const tagModel                    = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, this);
    TagPropertiesFilterModel* const filterModel = new TagPropertiesFilterModel(this);
    filterModel->setSourceAlbumModel(tagModel);
    filterModel->sort(0);

    QPointer<GeolocationEdit> dialog = new GeolocationEdit(filterModel,
                                                           new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools),
                                                           QApplication::activeWindow());
    dialog->setItems(ImageGPS::infosToItems(infos));
    dialog->exec();

    delete dialog;

    // Refresh Database with new metadata from files.
    foreach(const ImageInfo& inf, infos)
    {
        ScanController::instance()->scannedInfo(inf.fileUrl().toLocalFile());
    }
#endif
}

void DigikamApp::slotEditMetadata()
{
    QList<QUrl> urls = view()->selectedUrls(ApplicationSettings::Metadata);

    if (urls.isEmpty())
        return;

    QPointer<MetadataEditDialog> dialog = new MetadataEditDialog(QApplication::activeWindow(), urls);
    dialog->exec();

    delete dialog;

    // Refresh Database with new metadata from files.
    CollectionScanner scanner;

    foreach(const QUrl& url, urls)
    {
        scanner.scanFile(url.toLocalFile(), CollectionScanner::Rescan);
    }
}

void DigikamApp::slotImportFromScanner()
{
#ifdef HAVE_KSANE
    m_ksaneAction->activate(scannerTargetPlace(), configGroupName());
#endif
}

void DigikamApp::setupSelectToolsAction()
{
    // Create action model
    ActionItemModel* const actionModel = new ActionItemModel(this);
    actionModel->setMode(ActionItemModel::ToplevelMenuCategory | ActionItemModel::SortCategoriesByInsertionOrder);

    // Builtin actions

    QString mainCategory               = i18nc("@title Main Tools",          "Main Tools");
    actionModel->addAction(d->ieAction,                   mainCategory);
    actionModel->addAction(d->openTagMngrAction,          mainCategory);
    actionModel->addAction(d->bqmAction,                  mainCategory);
    actionModel->addAction(d->maintenanceAction,          mainCategory);
    actionModel->addAction(d->ltAction,                   mainCategory);
    actionModel->addAction(d->advSearchAction,            mainCategory);

    QString postCategory             = i18nc("@title Post Processing Tools", "Post-Processing");
    actionModel->addAction(m_expoBlendingAction,          postCategory);
    actionModel->addAction(m_calendarAction,              postCategory);
    actionModel->addAction(m_metadataEditAction,          postCategory);
    actionModel->addAction(m_presentationAction,          postCategory);
    actionModel->addAction(m_sendByMailAction,            postCategory);
    actionModel->addAction(m_printCreatorAction,          postCategory);
    actionModel->addAction(m_mediaServerAction,           postCategory);

#ifdef HAVE_PANORAMA
    actionModel->addAction(m_panoramaAction,              postCategory);
#endif

#ifdef HAVE_MEDIAPLAYER
    actionModel->addAction(m_videoslideshowAction,        postCategory);
#endif

#ifdef HAVE_HTMLGALLERY
    actionModel->addAction(m_htmlGalleryAction,           postCategory);
#endif

#ifdef HAVE_MARBLE
    actionModel->addAction(m_geolocationEditAction,       postCategory);
#endif

    QString exportCategory           = i18nc("@title Export Tools",          "Export");

    foreach(QAction* const ac, exportActions())
    {
        actionModel->addAction(ac,                        exportCategory);
    }

    QString importCategory           = i18nc("@title Import Tools",          "Import");

    foreach(QAction* const ac, importActions())
    {
        actionModel->addAction(ac,                        importCategory);
    }

#ifdef HAVE_KSANE
    actionModel->addAction(m_ksaneAction,                 importCategory);
#endif

    // setup categorized view
    DCategorizedSortFilterProxyModel* const filterModel = actionModel->createFilterModel();

    ActionCategorizedView* const selectToolsActionView  = new ActionCategorizedView;
    selectToolsActionView->setupIconMode();
    selectToolsActionView->setModel(filterModel);
    selectToolsActionView->adjustGridSize();

    connect(selectToolsActionView, SIGNAL(clicked(QModelIndex)),
            actionModel, SLOT(trigger(QModelIndex)));

    d->view->setToolsIconView(selectToolsActionView);
}

void DigikamApp::slotPresentation()
{
    d->view->presentation();
}

void DigikamApp::slotExportTool()
{
    QAction* const tool = dynamic_cast<QAction*>(sender());

    if (tool == m_exportDropboxAction)
    {
        DBWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportFacebookAction)
    {
        FbWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportFlickrAction)
    {
        FlickrWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportGdriveAction)
    {
        GSWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport),
                   this, QLatin1String("googledriveexport"));
        w.exec();
    }
    else if (tool == m_exportGphotoAction)
    {
        GSWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport),
                   this, QLatin1String("googlephotoexport"));
        w.exec();
    }
    else if (tool == m_exportImageshackAction)
    {
        ImageShackWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportImgurAction)
    {
        ImgurWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportPiwigoAction)
    {
        PiwigoWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportRajceAction)
    {
        RajceWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportSmugmugAction)
    {
        SmugWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportYandexfotkiAction)
    {
        YFWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }

#ifdef HAVE_MEDIAWIKI
    else if (tool == m_exportMediawikiAction)
    {
        MediaWikiWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
#endif

#ifdef HAVE_VKONTAKTE
    else if (tool == m_exportVkontakteAction)
    {
        VKWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
#endif

#ifdef HAVE_KIO
    else if (tool == m_exportFileTransferAction)
    {
        FTExportWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
#endif
}

void DigikamApp::slotImportTool()
{
    QAction* const tool = dynamic_cast<QAction*>(sender());

    if (tool == m_importGphotoAction)
    {
        GSWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport),
                   this, QLatin1String("googlephotoimport"));
        w.exec();
    }
    else if (tool == m_importSmugmugAction)
    {
        SmugWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport),
                     this, true);
        w.exec();
    }

#ifdef HAVE_KIO
    else if (tool == m_importFileTransferAction)
    {
        FTImportWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
#endif
}

}  // namespace Digikam
