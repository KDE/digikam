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
 * Copyright (C) 2002-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2014      by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "digikamapp.moc"
/// @todo Order should be changed according to krazy2, but compilation fails. Try again later. MH
#include "digikamapp_p.h"

// Qt includes

#include <QDataStream>
#include <QLabel>
#include <QPointer>
#include <QRegExp>
#include <QSignalMapper>
#include <QStringList>
#include <QtDBus>
#include <QDesktopServices>
#include <QDomDocument>

// KDE includes

#include <kdeversion.h>
#if KDE_IS_VERSION(4,1,68)
#include <kactioncategory.h>
#endif

#include <kdebug.h>
#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kapplication.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khbox.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <knotifyconfigwidget.h>
#include <kshortcut.h>
#include <kshortcutsdialog.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kstandardshortcut.h>
#include <ktip.h>
#include <ktoggleaction.h>
#include <ktogglefullscreenaction.h>
#include <ktoolbar.h>
#include <ktoolbarpopupaction.h>
#include <ktoolinvocation.h>
#include <kwindowsystem.h>

#include <solid/camera.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/predicate.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>

// Libkdcraw includes

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/version.h>

// Libkexiv2

#include <libkexiv2/rotationmatrix.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albumselectdialog.h"
#include "albumthumbnailloader.h"
#include "cameratype.h"
#include "importui.h"
#include "cameranamehelper.h"
#include "collectionscanner.h"
#include "componentsinfo.h"
#include "databasethumbnailinfoprovider.h"
#include "digikamadaptor.h"
#include "dio.h"
#include "dlogoaction.h"
#include "fileactionmngr.h"
#include "filterstatusbar.h"
#include "iccsettings.h"
#include "imageattributeswatch.h"
#include "imageinfo.h"
#include "imagewindow.h"
#include "lighttablewindow.h"
#include "queuemgrwindow.h"
#include "loadingcache.h"
#include "loadingcacheinterface.h"
#include "loadsavethread.h"
#include "scancontroller.h"
#include "setup.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "thememanager.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "dmetadata.h"
#include "uifilevalidator.h"
#include "tagscache.h"
#include "tagsactionmngr.h"
#include "databaseserverstarter.h"
#include "metadatasettings.h"
#include "statusbarprogresswidget.h"
#include "migrationdlg.h"
#include "progressmanager.h"
#include "progressview.h"
#include "maintenancedlg.h"
#include "maintenancemngr.h"
#include "newitemsfinder.h"
#include "imagepluginloader.h"
#include "tagsmanager.h"
#include "imagesortsettings.h"

#ifdef HAVE_KIPI
#include "kipipluginloader.h"
#endif

#ifdef HAVE_BALOO
#include "baloowrap.h"
#endif

using KIO::Job;
using KIO::UDSEntryList;
using KIO::UDSEntry;

namespace Digikam
{

DigikamApp* DigikamApp::m_instance = 0;

DigikamApp::DigikamApp()
    : DXmlGuiWindow(0), d(new Private)
{
    // --------------------------------------------------------

    setFullScreenOptions(FS_ALBUMGUI);
    setXMLFile("digikamui.rc");

    // --------------------------------------------------------

    // To adept the global KDE toolbar style, the toolbar needs to be named "mainToolBar".
    // digiKam used to name the toolbars "ToolBar", which makes it not behave like other KDE applications do.
    // A simple rename in the *ui.rc files does not prevent users from the "faulty" behavior if they have a custom
    // *ui.rc file in their home directories.
    // In this case, we need to parse the ui files and alter the name on startup. See BKO: 210823

    UiFileValidator validator(localXMLFile());

    if (!validator.isValid())
    {
        validator.fixConfigFile();
    }

    // --------------------------------------------------------

    m_instance = this;
    d->config  = KGlobal::config();

    setObjectName("Digikam");

    KConfigGroup group = ApplicationSettings::instance()->generalConfigGroup();

    if (group.readEntry("Show Splash", true) &&
        !kapp->isSessionRestored())
    {
        d->splashScreen = new SplashScreen();
        d->splashScreen->show();
    }

    if (d->splashScreen)
    {
        d->splashScreen->message(i18n("Scanning Albums..."));
    }

    new DigikamAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/Digikam", this);
    QDBusConnection::sessionBus().registerService("org.kde.digikam-"
            + QString::number(QCoreApplication::instance()->applicationPid()));

    // collection scan
    if (group.readEntry("Scan At Start", true) ||
        !CollectionScanner::databaseInitialScanDone())
    {
        ScanController::instance()->completeCollectionScanDeferFiles(d->splashScreen);
    }

    if (d->splashScreen)
    {
        d->splashScreen->message(i18n("Initializing..."));
    }

    // ensure creation
    AlbumManager::instance();
    LoadingCacheInterface::initialize();
    IccSettings::instance()->loadAllProfilesProperties();
    MetadataSettings::instance();
    ProgressManager::instance();
    ThumbnailLoadThread::setDisplayingWidget(this);
    DIO::instance();

    // creation of the engine on first use - when drawing -
    // can take considerable time and cause a noticeable hang in the UI thread.
    QFontMetrics fm(font());
    fm.width("a");

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    connect(IccSettings::instance(), SIGNAL(settingsChanged()),
            this, SLOT(slotColorManagementOptionsChanged()));

    d->cameraMenu      = new KActionMenu(this);
    d->usbMediaMenu    = new KActionMenu(this);
    d->cardReaderMenu  = new KActionMenu(this);
    d->quickImportMenu = new KActionMenu(this);

    d->cameraList = new CameraList(this, KStandardDirs::locateLocal("appdata", "cameras.xml"));

    connect(d->cameraList, SIGNAL(signalCameraAdded(CameraType*)),
            this, SLOT(slotCameraAdded(CameraType*)));

    connect(d->cameraList, SIGNAL(signalCameraRemoved(KAction*)),
            this, SLOT(slotCameraRemoved(KAction*)));

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
        d->splashScreen->message(i18n("Checking ICC repository..."));
    }

    d->validIccPath = SetupICC::iccRepositoryIsValid();

    // Read albums from database
    if (d->splashScreen)
    {
        d->splashScreen->message(i18n("Reading database..."));
    }

    AlbumManager::instance()->startScan();

    // Load Plugins.
    loadPlugins();

    // preload additional windows
    preloadWindows();

    readFullScreenSettings(group);

#ifdef HAVE_BALOO
    //Create BalooWrap object, because it need to register a listener
    // to update digiKam data when changes in Baloo occur
    BalooWrap* const baloo = BalooWrap::instance();
    Q_UNUSED(baloo);
#endif //HAVE_BALOO

    setAutoSaveSettings(group, true);

    // Now, enable finished the collection scan as deferred process
    NewItemsFinder* const tool = new NewItemsFinder(NewItemsFinder::ScanDeferredFiles);
    tool->start();

    LoadSaveThread::setInfoProvider(new DatabaseLoadSaveFileInfoProvider);
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
        // pass ownership of object - needed by ImageWindow destructor
        ImagePluginLoader::instance()->setParent(ImageWindow::imageWindow());
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

    if(TagsManager::isCreated())
    {
        TagsManager::instance()->close();
    }

#ifdef HAVE_BALOO
    if(BalooWrap::isCreated())
    {
        delete BalooWrap::instance();
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

    // close database server
    if (ApplicationSettings::instance()->getInternalDatabaseServer())
    {
        DatabaseServerStarter::cleanUp();
    }

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

        if (KMessageBox::warningYesNo(this, message) == KMessageBox::Yes)
        {
            if (!setupICC())
            {
                d->config->group("Color Management").writeEntry("EnableCM", false);
                d->config->sync();
            }
        }
        else
        {
            d->config->group("Color Management").writeEntry("EnableCM", false);
            d->config->sync();
        }
    }

    // Init album icon view zoom factor.
    slotThumbSizeChanged(ApplicationSettings::instance()->getDefaultIconSize());
    slotZoomSliderChanged(ApplicationSettings::instance()->getDefaultIconSize());
    d->autoShowZoomToolTip = true;
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

    DXmlGuiWindow::closeEvent(e);
    e->accept();
}

void DigikamApp::autoDetect()
{
    // Called from main if command line option is set, or via DBus

    if (d->splashScreen)
    {
        d->splashScreen->message(i18n("Auto-Detecting Camera..."));
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
            d->splashScreen->message(i18n("Opening Download Dialog..."));
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
            d->splashScreen->message(i18n("Opening Download Dialog..."));
        }

        emit queuedOpenSolidDevice(udi);
    }
}

QString DigikamApp::currentDatabaseParameters() const
{
    DatabaseParameters parameters = DatabaseAccess::parameters();
    KUrl url;
    parameters.insertInUrl(url);
    return url.url();
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
        d->splashScreen->message(i18n("Initializing Main View..."));
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
}

void DigikamApp::setupStatusBar()
{
    d->statusLabel = new KSqueezedTextLabel(statusBar());
    d->statusLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statusBar()->addWidget(d->statusLabel, 100);

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
    // Action are added by <MainWindow> tag in ui.rc XML file
    KAction* const escapeAction = new KAction(i18n("Exit Preview Mode"), this);
    actionCollection()->addAction("exit_preview_mode", escapeAction);
    escapeAction->setShortcut( KShortcut(Qt::Key_Escape) );
    connect(escapeAction, SIGNAL(triggered()), this, SIGNAL(signalEscapePressed()));

    KAction* const nextImageAction = new KAction(i18n("Next Image"), this);
    nextImageAction->setIcon(SmallIcon("go-next"));
    actionCollection()->addAction("next_image", nextImageAction);
    nextImageAction->setShortcut(KShortcut(Qt::Key_Space));
    connect(nextImageAction, SIGNAL(triggered()), this, SIGNAL(signalNextItem()));

    KAction* const previousImageAction = new KAction(i18n("Previous Image"), this);
    previousImageAction->setIcon(SmallIcon("go-previous"));
    actionCollection()->addAction("previous_image", previousImageAction);
    previousImageAction->setShortcut(KShortcut(Qt::Key_Backspace));
    connect(previousImageAction, SIGNAL(triggered()), this, SIGNAL(signalPrevItem()));

    KAction* const altpreviousImageAction = new KAction(i18n("Previous Image"), this);
    actionCollection()->addAction("alt_previous_image_shift_space", altpreviousImageAction);
    altpreviousImageAction->setShortcut( KShortcut(Qt::SHIFT+Qt::Key_Space) );
    connect(altpreviousImageAction, SIGNAL(triggered()), this, SIGNAL(signalPrevItem()));

    KAction* const firstImageAction = new KAction(i18n("First Image"), this);
    actionCollection()->addAction("first_image", firstImageAction);
    firstImageAction->setShortcut(KShortcut(Qt::Key_Home) );
    connect(firstImageAction, SIGNAL(triggered()), this, SIGNAL(signalFirstItem()));

    KAction* const lastImageAction = new KAction(i18n("Last Image"), this);
    actionCollection()->addAction("last_image", lastImageAction);
    lastImageAction->setShortcut(KShortcut(Qt::Key_End) );
    connect(lastImageAction, SIGNAL(triggered()), this, SIGNAL(signalLastItem()));

    d->cutItemsAction = KStandardAction::cut(this, SIGNAL(signalCutAlbumItemsSelection()), this);
    // need to remove shift+del from cut action,
    // else the shortcut for Delete permanently collides with secondary shortcut of Cut
    KShortcut cutShortcut = d->cutItemsAction->shortcut();
    cutShortcut.remove(Qt::SHIFT | Qt::Key_Delete, KShortcut::KeepEmpty);
    d->cutItemsAction->setShortcut(cutShortcut);
    actionCollection()->addAction("cut_album_selection", d->cutItemsAction);

    d->copyItemsAction = KStandardAction::copy(this, SIGNAL(signalCopyAlbumItemsSelection()), this);
    actionCollection()->addAction("copy_album_selection", d->copyItemsAction);

    d->pasteItemsAction = KStandardAction::paste(this, SIGNAL(signalPasteAlbumItemsSelection()), this);
    actionCollection()->addAction("paste_album_selection", d->pasteItemsAction);

    // Labels shortcuts must be registered here to be saved in XML GUI files if user customize it.
    d->tagsActionManager->registerLabelsActions(actionCollection());

    KAction* const editTitles = new KAction(i18n("Edit Titles"), this);
    editTitles->setShortcut( KShortcut(Qt::META + Qt::Key_T) );
    actionCollection()->addAction("edit_titles", editTitles);
    connect(editTitles, SIGNAL(triggered()), d->view, SLOT(slotRightSideBarActivateTitles()));

    KAction* const editComments = new KAction(i18n("Edit Comments"), this);
    editComments->setShortcut( KShortcut(Qt::META + Qt::Key_C) );
    actionCollection()->addAction("edit_comments", editComments);
    connect(editComments, SIGNAL(triggered()), d->view, SLOT(slotRightSideBarActivateComments()));

    KAction* const assignedTags = new KAction(i18n("Show Assigned Tags"), this);
    assignedTags->setShortcut( KShortcut(Qt::META + Qt::Key_A) );
    actionCollection()->addAction("assigned _tags", assignedTags);
    connect(assignedTags, SIGNAL(triggered()), d->view, SLOT(slotRightSideBarActivateAssignedTags()));
}

void DigikamApp::setupActions()
{
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

    d->backwardActionMenu = new KToolBarPopupAction(KIcon("go-previous"), i18n("&Back"), this);
    d->backwardActionMenu->setEnabled(false);
    d->backwardActionMenu->setShortcut(KShortcut(Qt::ALT+Qt::Key_Left));
    actionCollection()->addAction("album_back", d->backwardActionMenu);

    connect(d->backwardActionMenu->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowBackwardMenu()));

    // we are using a signal mapper to identify which of a bunch of actions was triggered
    d->backwardSignalMapper = new QSignalMapper(this);

    // connect mapper to view
    connect(d->backwardSignalMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotAlbumHistoryBack(int)));

    // connect action to mapper
    connect(d->backwardActionMenu, SIGNAL(triggered()), d->backwardSignalMapper, SLOT(map()));
    // inform mapper about number of steps
    d->backwardSignalMapper->setMapping(d->backwardActionMenu, 1);

    // -----------------------------------------------------------------

    d->forwardActionMenu = new KToolBarPopupAction(KIcon("go-next"), i18n("Forward"), this);
    d->forwardActionMenu->setEnabled(false);
    d->forwardActionMenu->setShortcut(KShortcut(Qt::ALT+Qt::Key_Right));
    actionCollection()->addAction("album_forward", d->forwardActionMenu);

    connect(d->forwardActionMenu->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowForwardMenu()));

    d->forwardSignalMapper = new QSignalMapper(this);

    connect(d->forwardSignalMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotAlbumHistoryForward(int)));

    connect(d->forwardActionMenu, SIGNAL(triggered()), d->forwardSignalMapper, SLOT(map()));
    d->forwardSignalMapper->setMapping(d->forwardActionMenu, 1);

    // -----------------------------------------------------------------

    d->refreshAction = new KAction(KIcon("view-refresh"), i18n("Refresh"), this);
    d->refreshAction->setShortcut(KShortcut(Qt::Key_F5));
    d->refreshAction->setWhatsThis(i18n("Refresh the current contents."));
    connect(d->refreshAction, SIGNAL(triggered()), d->view, SLOT(slotRefresh()));
    actionCollection()->addAction("view_refresh", d->refreshAction);

    // -----------------------------------------------------------------

    QSignalMapper* const browseActionsMapper = new QSignalMapper(this);
    connect(browseActionsMapper, SIGNAL(mapped(QWidget*)),
            d->view, SLOT(slotLeftSideBarActivate(QWidget*)));

    foreach(SidebarWidget* const leftWidget, d->view->leftSidebarWidgets())
    {
        QString actionName = "browse_" + leftWidget->objectName().remove(' ').remove("Sidebar").remove("FolderView").remove("View").toLower();
        kDebug() << actionName;

        KAction* const action = new KAction(KIcon(leftWidget->getIcon()), leftWidget->getCaption(), this);
        action->setShortcut(KShortcut(leftWidget->property("Shortcut").toInt()));
        actionCollection()->addAction(actionName, action);
        connect(action, SIGNAL(triggered()), browseActionsMapper, SLOT(map()));
        browseActionsMapper->setMapping(action, leftWidget);
    }

    // -----------------------------------------------------------------

    d->newAction = new KAction(KIcon("albumfolder-new"), i18n("&New..."), this);
    d->newAction->setShortcut(KStandardShortcut::openNew());
    d->newAction->setWhatsThis(i18n("Creates a new empty Album in the collection."));
    connect(d->newAction, SIGNAL(triggered()), d->view, SLOT(slotNewAlbum()));
    actionCollection()->addAction("album_new", d->newAction);

    // -----------------------------------------------------------------

    d->moveSelectionToAlbumAction = new KAction(KIcon("albumfolder-new"), i18n("&Move to Album..."), this);
    d->moveSelectionToAlbumAction->setWhatsThis(i18n("Move selected images into an album."));
    connect(d->moveSelectionToAlbumAction, SIGNAL(triggered()), d->view, SLOT(slotMoveSelectionToAlbum()));
    actionCollection()->addAction("move_selection_to_album", d->moveSelectionToAlbumAction);

    // -----------------------------------------------------------------

    d->deleteAction = new KAction(KIcon("albumfolder-user-trash"), i18n("Delete Album"), this);
    connect(d->deleteAction, SIGNAL(triggered()), d->view, SLOT(slotDeleteAlbum()));
    actionCollection()->addAction("album_delete", d->deleteAction);

    // -----------------------------------------------------------------

    d->renameAction = new KAction(KIcon("edit-rename"), i18n("Rename..."), this);
    d->renameAction->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_F2));
    connect(d->renameAction, SIGNAL(triggered()), d->view, SLOT(slotRenameAlbum()));
    actionCollection()->addAction("album_rename", d->renameAction);

    // -----------------------------------------------------------------

    d->propsEditAction = new KAction(KIcon("albumfolder-properties"), i18n("Properties"), this);
    d->propsEditAction->setShortcut(KShortcut(Qt::ALT + Qt::Key_Return));
    d->propsEditAction->setWhatsThis(i18n("Edit album properties and collection information."));
    connect(d->propsEditAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumPropsEdit()));
    actionCollection()->addAction("album_propsEdit", d->propsEditAction);

    // -----------------------------------------------------------------

    d->writeAlbumMetadataAction = new KAction(KIcon("document-edit"), i18n("Write Metadata to Images"), this);
    d->writeAlbumMetadataAction->setWhatsThis(i18n("Updates metadata of images in the current "
                                                   "album with the contents of digiKam database "
                                                   "(image metadata will be overwritten with data from "
                                                   "the database)."));
    connect(d->writeAlbumMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumWriteMetadata()));
    actionCollection()->addAction("album_write_metadata", d->writeAlbumMetadataAction);

    // -----------------------------------------------------------------

    d->readAlbumMetadataAction = new KAction(KIcon("edit-redo"), i18n("Reread Metadata From Images"), this);
    d->readAlbumMetadataAction->setWhatsThis(i18n("Updates the digiKam database from the metadata "
                                                  "of the files in the current album "
                                                  "(information in the database will be overwritten with data from "
                                                  "the files' metadata)."));
    connect(d->readAlbumMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumReadMetadata()));
    actionCollection()->addAction("album_read_metadata", d->readAlbumMetadataAction);

    // -----------------------------------------------------------------

    d->openInFileManagerAction = new KAction(KIcon("folder-open"), i18n("Open in File Manager"), this);
    connect(d->openInFileManagerAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumOpenInFileManager()));
    actionCollection()->addAction("album_openinfilemanager", d->openInFileManagerAction);

    // -----------------------------------------------------------

    d->openInTerminalAction = new KAction(KIcon("utilities-terminal"), i18n("Open in Terminal"), this);
    connect(d->openInTerminalAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumOpenInTerminal()));
    actionCollection()->addAction("album_openinterminal", d->openInTerminalAction);

    // -----------------------------------------------------------

    d->openTagMngrAction = new KAction(KIcon("tag"), i18n("Open Tag Manager"), this);
    connect(d->openTagMngrAction, SIGNAL(triggered()), d->view, SLOT(slotOpenTagsManager()));
    actionCollection()->addAction("open_tag_mngr", d->openTagMngrAction);

    // -----------------------------------------------------------

    d->newTagAction = new KAction(KIcon("tag-new"), i18nc("new tag", "N&ew..."), this);
    connect(d->newTagAction, SIGNAL(triggered()), d->view, SLOT(slotNewTag()));
    actionCollection()->addAction("tag_new", d->newTagAction);

    // -----------------------------------------------------------

    d->editTagAction = new KAction(KIcon("tag-properties"), i18n("Properties"), this);
    connect(d->editTagAction, SIGNAL(triggered()), d->view, SLOT(slotEditTag()));
    actionCollection()->addAction("tag_edit", d->editTagAction);

    // -----------------------------------------------------------

    d->deleteTagAction = new KAction(KIcon("user-trash"), i18n("Delete"), this);
    connect(d->deleteTagAction, SIGNAL(triggered()), d->view, SLOT(slotDeleteTag()));
    actionCollection()->addAction("tag_delete", d->deleteTagAction);

    // -----------------------------------------------------------

    d->assignTagAction = new KAction(KIcon("tag-new"), i18n("Assign Tag"), this);
    d->assignTagAction->setShortcut(KShortcut(Qt::Key_T));
    connect(d->assignTagAction, SIGNAL(triggered()), d->view, SLOT(slotAssignTag()));
    actionCollection()->addAction("tag_assign", d->assignTagAction);

    // -----------------------------------------------------------

    d->imageViewSelectionAction = new KSelectAction(KIcon("viewimage"), i18n("Views"), this);
    //d->imageViewSelectionAction->setDelayed(false);
    actionCollection()->addAction("view_selection", d->imageViewSelectionAction);

    d->imageIconViewAction = new KToggleAction(KIcon("view-list-icons"),
                                               i18nc("@action Go to thumbnails (icon) view", "Thumbnails"), this);
    actionCollection()->addAction("icon_view", d->imageIconViewAction);
    connect(d->imageIconViewAction, SIGNAL(triggered()), d->view, SLOT(slotIconView()));
    d->imageViewSelectionAction->addAction(d->imageIconViewAction);

    d->imagePreviewAction = new KToggleAction(KIcon("viewimage"), i18nc("View the selected image", "Preview Image"), this);
    d->imagePreviewAction->setShortcut(KShortcut(Qt::Key_F3));
    actionCollection()->addAction("image_view", d->imagePreviewAction);
    connect(d->imagePreviewAction, SIGNAL(triggered()), d->view, SLOT(slotImagePreview()));
    d->imageViewSelectionAction->addAction(d->imagePreviewAction);

#ifdef HAVE_KGEOMAP
    d->imageMapViewAction = new KToggleAction(KIcon("applications-internet"),
                                              i18nc("@action Switch to map view", "Map"), this);
    actionCollection()->addAction("map_view", d->imageMapViewAction);
    connect(d->imageMapViewAction, SIGNAL(triggered()), d->view, SLOT(slotMapWidgetView()));
    d->imageViewSelectionAction->addAction(d->imageMapViewAction);
#endif // HAVE_KGEOMAP

    d->imageTableViewAction = new KToggleAction(KIcon("view-list-details"),
                                                i18nc("@action Switch to table view", "Table"), this);
    actionCollection()->addAction("table_view", d->imageTableViewAction);
    connect(d->imageTableViewAction, SIGNAL(triggered()), d->view, SLOT(slotTableView()));
    d->imageViewSelectionAction->addAction(d->imageTableViewAction);

    // -----------------------------------------------------------

    d->imageViewAction = new KAction(KIcon("quickopen-file"), i18n("Open..."), this);
    d->imageViewAction->setShortcut(KShortcut(Qt::Key_F4));
    d->imageViewAction->setWhatsThis(i18n("Open the selected item."));
    connect(d->imageViewAction, SIGNAL(triggered()), d->view, SLOT(slotImageEdit()));
    actionCollection()->addAction("image_edit", d->imageViewAction);

    KAction* const openWithAction = new KAction(KIcon("preferences-desktop-filetype-association"), i18n("Open With Default Application"), this);
    openWithAction->setShortcut(KShortcut(Qt::META + Qt::Key_F4));
    openWithAction->setWhatsThis(i18n("Open the selected item with default assigned application."));
    connect(openWithAction, SIGNAL(triggered()), d->view, SLOT(slotFileWithDefaultApplication()));
    actionCollection()->addAction("open_with_default_application", openWithAction);

    KAction* const ieAction = new KAction(KIcon("editimage"), i18n("Image Editor"), this);
    ieAction->setWhatsThis(i18n("Open the image editor."));
    connect(ieAction, SIGNAL(triggered()), d->view, SLOT(slotEditor()));
    actionCollection()->addAction("imageeditor", ieAction);

    // -----------------------------------------------------------

    KAction* const ltAction = new KAction(KIcon("lighttable"), i18n("Light Table"), this);
    ltAction->setShortcut(KShortcut(Qt::Key_L));
    connect(ltAction, SIGNAL(triggered()), d->view, SLOT(slotLightTable()));
    actionCollection()->addAction("light_table", ltAction);

    d->imageLightTableAction = new KAction(KIcon("lighttable"), i18n("Place onto Light Table"), this);
    d->imageLightTableAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_L));
    d->imageLightTableAction->setWhatsThis(i18n("Place the selected items on the light table thumbbar."));
    connect(d->imageLightTableAction, SIGNAL(triggered()), d->view, SLOT(slotImageLightTable()));
    actionCollection()->addAction("image_lighttable", d->imageLightTableAction);

    d->imageAddLightTableAction = new KAction(KIcon("lighttableadd"), i18n("Add to Light Table"), this);
    d->imageAddLightTableAction->setShortcut(KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_L));
    d->imageAddLightTableAction->setWhatsThis(i18n("Add selected items to the light table thumbbar."));
    connect(d->imageAddLightTableAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToLightTable()));
    actionCollection()->addAction("image_add_to_lighttable", d->imageAddLightTableAction);

    // -----------------------------------------------------------

    d->bqmAction = new KAction(KIcon("bqm-diff"), i18n("Batch Queue Manager"), this);
    d->bqmAction->setShortcut(KShortcut(Qt::Key_B));
    connect(d->bqmAction, SIGNAL(triggered()), d->view, SLOT(slotQueueMgr()));
    actionCollection()->addAction("queue_manager", d->bqmAction);

    d->imageAddCurrentQueueAction = new KAction(KIcon("bqm-commit"), i18n("Add to Current Queue"), this);
    d->imageAddCurrentQueueAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_B));
    d->imageAddCurrentQueueAction->setWhatsThis(i18n("Add selected items to current queue from batch manager."));
    connect(d->imageAddCurrentQueueAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToCurrentQueue()));
    actionCollection()->addAction("image_add_to_current_queue", d->imageAddCurrentQueueAction);

    d->imageAddNewQueueAction = new KAction(KIcon("bqm-add"), i18n("Add to New Queue"), this);
    d->imageAddNewQueueAction->setShortcut(KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_B));
    d->imageAddNewQueueAction->setWhatsThis(i18n("Add selected items to a new queue from batch manager."));
    connect(d->imageAddNewQueueAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToNewQueue()));
    actionCollection()->addAction("image_add_to_new_queue", d->imageAddNewQueueAction);

    // NOTE: see bug #252130 and #283281 : we need to disable these actions when BQM is running.

    connect(QueueMgrWindow::queueManagerWindow(), SIGNAL(signalBqmIsBusy(bool)),
            d->bqmAction, SLOT(setDisabled(bool)));

    connect(QueueMgrWindow::queueManagerWindow(), SIGNAL(signalBqmIsBusy(bool)),
            d->imageAddCurrentQueueAction, SLOT(setDisabled(bool)));

    connect(QueueMgrWindow::queueManagerWindow(), SIGNAL(signalBqmIsBusy(bool)),
            d->imageAddNewQueueAction, SLOT(setDisabled(bool)));

    // -----------------------------------------------------------------

    d->quickImportMenu->setText(i18nc("@action Import photos from camera", "Import"));
    d->quickImportMenu->setIcon(KIcon("camera-photo"));
    actionCollection()->addAction("import_auto", d->quickImportMenu);

    // -----------------------------------------------------------------

    d->imageWriteMetadataAction = new KAction(KIcon("document-edit"), i18n("Write Metadata to Selected Images"), this);
    d->imageWriteMetadataAction->setWhatsThis(i18n("Updates metadata of images in the current "
                                                   "album with the contents of digiKam database "
                                                   "(image metadata will be overwritten with data from "
                                                   "the database)."));
    connect(d->imageWriteMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotImageWriteMetadata()));
    actionCollection()->addAction("image_write_metadata", d->imageWriteMetadataAction);

    // -----------------------------------------------------------------

    d->imageReadMetadataAction = new KAction(KIcon("edit-redo"), i18n("Reread Metadata From Selected Images"), this);
    d->imageReadMetadataAction->setWhatsThis(i18n("Updates the digiKam database from the metadata "
                                                  "of the files in the current album "
                                                  "(information in the database will be overwritten with data from "
                                                  "the files' metadata)."));
    connect(d->imageReadMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotImageReadMetadata()));
    actionCollection()->addAction("image_read_metadata", d->imageReadMetadataAction);

    // -----------------------------------------------------------

    d->imageFindSimilarAction = new KAction(KIcon("tools-wizard"), i18n("Find Similar..."), this);
    d->imageFindSimilarAction->setWhatsThis(i18n("Find similar images using selected item as reference."));
    connect(d->imageFindSimilarAction, SIGNAL(triggered()), d->view, SLOT(slotImageFindSimilar()));
    actionCollection()->addAction("image_find_similar", d->imageFindSimilarAction);

    // -----------------------------------------------------------

    d->imageRenameAction = new KAction(KIcon("edit-rename"), i18n("Rename..."), this);
    d->imageRenameAction->setShortcut(KShortcut(Qt::Key_F2));
    d->imageRenameAction->setWhatsThis(i18n("Change the filename of the currently selected item."));
    connect(d->imageRenameAction, SIGNAL(triggered()), d->view, SLOT(slotImageRename()));
    actionCollection()->addAction("image_rename", d->imageRenameAction);

    // -----------------------------------------------------------

    // Pop up dialog to ask user whether to move to trash
    d->imageDeleteAction = new KAction(KIcon("user-trash"), i18nc("Non-pluralized", "Move to Trash"), this);
    d->imageDeleteAction->setShortcut(KShortcut(Qt::Key_Delete));
    connect(d->imageDeleteAction, SIGNAL(triggered()), d->view, SLOT(slotImageDelete()));
    actionCollection()->addAction("image_delete", d->imageDeleteAction);

    // -----------------------------------------------------------

    // Pop up dialog to ask user whether to permanently delete
    // FIXME: This action is never used?? How can someone delete a album directly, without moving it to the trash first?
    //        This is especially important when deleting from a different partiton or from a net source.
    //        Also note that we use the wrong icon for the default album delete action, which should have a thrashcan icon instead
    //        of a red cross, it confuses users.
    d->imageDeletePermanentlyAction = new KAction(KIcon("edit-delete"), i18n("Delete Permanently"), this);
    d->imageDeletePermanentlyAction->setShortcut(KShortcut(Qt::SHIFT+Qt::Key_Delete));
    connect(d->imageDeletePermanentlyAction, SIGNAL(triggered()), d->view, SLOT(slotImageDeletePermanently()));
    actionCollection()->addAction("image_delete_permanently", d->imageDeletePermanentlyAction);

    // -----------------------------------------------------------

    // These two actions are hidden, no menu entry, no toolbar entry, no shortcut.
    // Power users may add them.
    d->imageDeletePermanentlyDirectlyAction = new KAction(KIcon("edit-delete"),
                                                          i18n("Delete permanently without confirmation"), this);
    connect(d->imageDeletePermanentlyDirectlyAction, SIGNAL(triggered()),
            d->view, SLOT(slotImageDeletePermanentlyDirectly()));
    actionCollection()->addAction("image_delete_permanently_directly", d->imageDeletePermanentlyDirectlyAction);

    // -----------------------------------------------------------

    d->imageTrashDirectlyAction = new KAction(KIcon("user-trash"),
                                              i18n("Move to trash without confirmation"), this);
    connect(d->imageTrashDirectlyAction, SIGNAL(triggered()),
            d->view, SLOT(slotImageTrashDirectly()));
    actionCollection()->addAction("image_trash_directly", d->imageTrashDirectlyAction);

    // -----------------------------------------------------------------

    d->albumSortAction = new KSelectAction(i18n("&Sort Albums"), this);
    d->albumSortAction->setWhatsThis(i18n("Sort Albums in tree-view."));
    connect(d->albumSortAction, SIGNAL(triggered(int)), d->view, SLOT(slotSortAlbums(int)));
    actionCollection()->addAction("album_sort", d->albumSortAction);

    // Use same list order as in applicationsettings enum
    QStringList sortActionList;
    sortActionList.append(i18n("By Folder"));
    sortActionList.append(i18n("By Category"));
    sortActionList.append(i18n("By Date"));
    d->albumSortAction->setItems(sortActionList);

    // -----------------------------------------------------------

    d->recurseAlbumsAction = new KToggleAction(i18n("Include Album Sub-Tree"), this);
    d->recurseAlbumsAction->setWhatsThis(i18n("Activate this option to show all sub-albums below "
                                              "the current album."));
    connect(d->recurseAlbumsAction, SIGNAL(toggled(bool)), this, SLOT(slotRecurseAlbums(bool)));
    actionCollection()->addAction("albums_recursive", d->recurseAlbumsAction);

    d->recurseTagsAction = new KToggleAction(i18n("Include Tag Sub-Tree"), this);
    d->recurseTagsAction->setWhatsThis(i18n("Activate this option to show all images marked by the given tag "
                                            "and all its sub-tags."));
    connect(d->recurseTagsAction, SIGNAL(toggled(bool)), this, SLOT(slotRecurseTags(bool)));
    actionCollection()->addAction("tags_recursive", d->recurseTagsAction);

    // -----------------------------------------------------------

    d->imageSortAction                   = new KSelectAction(i18n("&Sort Images"), this);
    d->imageSortAction->setWhatsThis(i18n("The value by which the images in one album are sorted in the thumbnail view"));
    QSignalMapper* const imageSortMapper = new QSignalMapper(this);
    connect(imageSortMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSortImages(int)));
    actionCollection()->addAction("image_sort", d->imageSortAction);

    // map to ImageSortSettings enum
    QAction* const sortByNameAction        = d->imageSortAction->addAction(i18n("By Name"));
    QAction* const sortByPathAction        = d->imageSortAction->addAction(i18n("By Path"));
    QAction* const sortByDateAction        = d->imageSortAction->addAction(i18n("By Date"));
    QAction* const sortByFileSizeAction    = d->imageSortAction->addAction(i18n("By File Size"));
    QAction* const sortByRatingAction      = d->imageSortAction->addAction(i18n("By Rating"));
    QAction* const sortByImageSizeAction   = d->imageSortAction->addAction(i18n("By Image Size"));
    QAction* const sortByAspectRatioAction = d->imageSortAction->addAction(i18n("By Aspect Ratio"));

    connect(sortByNameAction,        SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByPathAction,        SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByDateAction,        SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByFileSizeAction,    SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByRatingAction,      SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByImageSizeAction,   SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByAspectRatioAction, SIGNAL(triggered()), imageSortMapper, SLOT(map()));

    imageSortMapper->setMapping(sortByNameAction,        (int)ImageSortSettings::SortByFileName);
    imageSortMapper->setMapping(sortByPathAction,        (int)ImageSortSettings::SortByFilePath);
    imageSortMapper->setMapping(sortByDateAction,        (int)ImageSortSettings::SortByCreationDate);
    imageSortMapper->setMapping(sortByFileSizeAction,    (int)ImageSortSettings::SortByFileSize);
    imageSortMapper->setMapping(sortByRatingAction,      (int)ImageSortSettings::SortByRating);
    imageSortMapper->setMapping(sortByImageSizeAction,   (int)ImageSortSettings::SortByImageSize);
    imageSortMapper->setMapping(sortByAspectRatioAction, (int)ImageSortSettings::SortByAspectRatio);

    // -----------------------------------------------------------

    d->imageSortOrderAction                   = new KSelectAction(i18n("Image Sorting &Order"), this);
    d->imageSortOrderAction->setWhatsThis(i18n("Defines whether images are sorted in ascending or descending manner."));
    QSignalMapper* const imageSortOrderMapper = new QSignalMapper(this);
    connect(imageSortOrderMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSortImagesOrder(int)));
    actionCollection()->addAction("image_sort_order", d->imageSortOrderAction);

    QAction* const sortAscendingAction  = d->imageSortOrderAction->addAction(KIcon("view-sort-ascending"), i18n("Ascending"));
    QAction* const sortDescendingAction = d->imageSortOrderAction->addAction(KIcon("view-sort-descending"), i18n("Descending"));

    connect(sortAscendingAction,  SIGNAL(triggered()), imageSortOrderMapper, SLOT(map()));
    connect(sortDescendingAction, SIGNAL(triggered()), imageSortOrderMapper, SLOT(map()));

    imageSortOrderMapper->setMapping(sortAscendingAction,  (int)ImageSortSettings::AscendingOrder);
    imageSortOrderMapper->setMapping(sortDescendingAction, (int)ImageSortSettings::DescendingOrder);

    // -----------------------------------------------------------

    d->imageGroupAction                   = new KSelectAction(i18n("&Group Images"), this);
    d->imageGroupAction->setWhatsThis(i18n("The categories in which the images in the thumbnail view are displayed"));
    QSignalMapper* const imageGroupMapper = new QSignalMapper(this);
    connect(imageGroupMapper, SIGNAL(mapped(int)), d->view, SLOT(slotGroupImages(int)));
    actionCollection()->addAction("image_group", d->imageGroupAction);

    // map to ImageSortSettings enum
    QAction* const noCategoriesAction  = d->imageGroupAction->addAction(i18n("Flat List"));
    QAction* const groupByAlbumAction  = d->imageGroupAction->addAction(i18n("By Album"));
    QAction* const groupByFormatAction = d->imageGroupAction->addAction(i18n("By Format"));

    connect(noCategoriesAction,  SIGNAL(triggered()), imageGroupMapper, SLOT(map()));
    connect(groupByAlbumAction,  SIGNAL(triggered()), imageGroupMapper, SLOT(map()));
    connect(groupByFormatAction, SIGNAL(triggered()), imageGroupMapper, SLOT(map()));

    imageGroupMapper->setMapping(noCategoriesAction,  (int)ImageSortSettings::OneCategory);
    imageGroupMapper->setMapping(groupByAlbumAction,  (int)ImageSortSettings::CategoryByAlbum);
    imageGroupMapper->setMapping(groupByFormatAction, (int)ImageSortSettings::CategoryByFormat);

    // -----------------------------------------------------------------

    d->imageGroupSortOrderAction                   = new KSelectAction(i18n("Group Sorting Order"), this);
    d->imageGroupSortOrderAction->setWhatsThis(i18n("The sort order of images groups"));
    QSignalMapper* const imageGroupSortOrderMapper = new QSignalMapper(this);
    connect(imageGroupSortOrderMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSortImageGroupOrder(int)));
    actionCollection()->addAction("image_group_sort_order", d->imageGroupSortOrderAction);

    QAction* const sortGroupsAscending  = d->imageGroupSortOrderAction->addAction(KIcon("view-sort-ascending"), i18n("Ascending"));
    QAction* const sortGroupsDescending = d->imageGroupSortOrderAction->addAction(KIcon("view-sort-descending"), i18n("Descending"));

    connect(sortGroupsAscending,  SIGNAL(triggered()), imageGroupSortOrderMapper, SLOT(map()));
    connect(sortGroupsDescending, SIGNAL(triggered()), imageGroupSortOrderMapper, SLOT(map()));

    imageGroupSortOrderMapper->setMapping(sortGroupsAscending, (int)ImageSortSettings::AscendingOrder);
    imageGroupSortOrderMapper->setMapping(sortGroupsDescending, (int)ImageSortSettings::DescendingOrder);

    // -----------------------------------------------------------------

    setupImageTransformActions();
    setupExifOrientationActions();

    // -----------------------------------------------------------------

    d->selectAllAction = new KAction(i18n("Select All"), this);
    d->selectAllAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_A));
    connect(d->selectAllAction, SIGNAL(triggered()), d->view, SLOT(slotSelectAll()));
    actionCollection()->addAction("selectAll", d->selectAllAction);

    // -----------------------------------------------------------------

    d->selectNoneAction = new KAction(i18n("Select None"), this);
    d->selectNoneAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_A));
    connect(d->selectNoneAction, SIGNAL(triggered()), d->view, SLOT(slotSelectNone()));
    actionCollection()->addAction("selectNone", d->selectNoneAction);

    // -----------------------------------------------------------------

    d->selectInvertAction = new KAction(i18n("Invert Selection"), this);
    d->selectInvertAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_I));
    connect(d->selectInvertAction, SIGNAL(triggered()), d->view, SLOT(slotSelectInvert()));
    actionCollection()->addAction("selectInvert", d->selectInvertAction);

    // -----------------------------------------------------------

    d->showBarAction = new KToggleAction(KIcon("view-choose"), i18n("Show Thumbbar"), this);
    d->showBarAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_T));
    connect(d->showBarAction, SIGNAL(triggered()), this, SLOT(slotToggleShowBar()));
    actionCollection()->addAction("showthumbs", d->showBarAction);

    d->showMenuBarAction = KStandardAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());

    KStandardAction::keyBindings(this,            SLOT(slotEditKeys()),          actionCollection());
    KStandardAction::configureToolbars(this,      SLOT(slotConfToolbars()),      actionCollection());
    KStandardAction::configureNotifications(this, SLOT(slotConfNotifications()), actionCollection());
    KStandardAction::preferences(this,            SLOT(slotSetup()),             actionCollection());

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // -----------------------------------------------------------

    d->zoomPlusAction  = KStandardAction::zoomIn(d->view, SLOT(slotZoomIn()), this);
    KShortcut keysPlus = d->zoomPlusAction->shortcut();
    keysPlus.setAlternate(Qt::Key_Plus);
    d->zoomPlusAction->setShortcut(keysPlus);
    actionCollection()->addAction("album_zoomin", d->zoomPlusAction);

    // -----------------------------------------------------------

    d->zoomMinusAction  = KStandardAction::zoomOut(d->view, SLOT(slotZoomOut()), this);
    KShortcut keysMinus = d->zoomMinusAction->shortcut();
    keysMinus.setAlternate(Qt::Key_Minus);
    d->zoomMinusAction->setShortcut(keysMinus);
    actionCollection()->addAction("album_zoomout", d->zoomMinusAction);

    // -----------------------------------------------------------

    d->zoomTo100percents = new KAction(KIcon("zoom-original"), i18n("Zoom to 100%"), this);
    d->zoomTo100percents->setShortcut(KShortcut(Qt::CTRL + Qt::Key_Comma));
    connect(d->zoomTo100percents, SIGNAL(triggered()), d->view, SLOT(slotZoomTo100Percents()));
    actionCollection()->addAction("album_zoomto100percents", d->zoomTo100percents);

    // -----------------------------------------------------------

    d->zoomFitToWindowAction = new KAction(KIcon("zoom-fit-best"), i18n("Fit to &Window"), this);
    d->zoomFitToWindowAction->setShortcut(KShortcut(Qt::ALT + Qt::CTRL + Qt::Key_E));
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), d->view, SLOT(slotFitToWindow()));
    actionCollection()->addAction("album_zoomfit2window", d->zoomFitToWindowAction);

    // -----------------------------------------------------------

    createFullScreenAction("full_screen");
    createSidebarActions();

    // -----------------------------------------------------------

    d->slideShowAction = new KActionMenu(KIcon("view-presentation"), i18n("Slideshow"), this);
    d->slideShowAction->setDelayed(false);
    actionCollection()->addAction("slideshow", d->slideShowAction);

    d->slideShowAllAction = new KAction(i18n("All"), this);
    d->slideShowAllAction->setShortcut(KShortcut(Qt::Key_F9));
    connect(d->slideShowAllAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowAll()));
    actionCollection()->addAction("slideshow_all", d->slideShowAllAction);
    d->slideShowAction->addAction(d->slideShowAllAction);

    d->slideShowSelectionAction = new KAction(i18n("Selection"), this);
    d->slideShowSelectionAction->setShortcut(KShortcut(Qt::ALT+Qt::Key_F9));
    connect(d->slideShowSelectionAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowSelection()));
    actionCollection()->addAction("slideshow_selected", d->slideShowSelectionAction);
    d->slideShowAction->addAction(d->slideShowSelectionAction);

    d->slideShowRecursiveAction = new KAction(i18n("With All Sub-Albums"), this);
    d->slideShowRecursiveAction->setShortcut(KShortcut(Qt::SHIFT+Qt::Key_F9));
    connect(d->slideShowRecursiveAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowRecursive()));
    actionCollection()->addAction("slideshow_recursive", d->slideShowRecursiveAction);
    d->slideShowAction->addAction(d->slideShowRecursiveAction);

    // -----------------------------------------------------------

    d->viewCMViewAction = new KToggleAction(KIcon("video-display"), i18n("Color-Managed View"), this);
    d->viewCMViewAction->setShortcut(KShortcut(Qt::Key_F12));
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    actionCollection()->addAction("color_managed_view", d->viewCMViewAction);

    // -----------------------------------------------------------

    d->quitAction = KStandardAction::quit(this, SLOT(slotExit()), this);
    actionCollection()->addAction("app_exit", d->quitAction);

    // -----------------------------------------------------------

    createHelpActions();

    // -----------------------------------------------------------

    d->kipiHelpAction = new KAction(KIcon("kipi"), i18n("Kipi Plugins Handbook"), this);
    connect(d->kipiHelpAction, SIGNAL(triggered()), this, SLOT(slotShowKipiHelp()));
    actionCollection()->addAction("help_kipi", d->kipiHelpAction);

    // -----------------------------------------------------------

    d->tipAction = actionCollection()->addAction(KStandardAction::TipofDay, "help_tipofday",
                                                 this, SLOT(slotShowTip()));

    //------------------------------------------------------------

    KAction* const findAction = new KAction(KIcon("system-search"), i18n("Search..."), this);
    findAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_F));
    connect(findAction, SIGNAL(triggered()), d->view, SLOT(slotNewKeywordSearch()));
    actionCollection()->addAction("search_quick", findAction);

    // -----------------------------------------------------------

    KAction* const advFindAction = new KAction(KIcon("system-search"), i18n("Advanced Search..."), this);
    advFindAction->setShortcut(KShortcut(Qt::CTRL+Qt::ALT+Qt::Key_F));
    connect(advFindAction, SIGNAL(triggered()), d->view, SLOT(slotNewAdvancedSearch()));
    actionCollection()->addAction("search_advanced", advFindAction);

    // -----------------------------------------------------------

    KAction* const duplicatesAction = new KAction(KIcon("tools-wizard"), i18n("Find Duplicates..."), this);
    duplicatesAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_D));
    connect(duplicatesAction, SIGNAL(triggered()), d->view, SLOT(slotNewDuplicatesSearch()));
    actionCollection()->addAction("find_duplicates", duplicatesAction);

    // -----------------------------------------------------------

#ifdef HAVE_MYSQLSUPPORT
    KAction* const databaseMigrationAction = new KAction(KIcon("server-database"), i18n("Database Migration..."), this);
    connect(databaseMigrationAction, SIGNAL(triggered()), this, SLOT(slotDatabaseMigration()));
    actionCollection()->addAction("database_migration", databaseMigrationAction);
#endif

    // -----------------------------------------------------------

    d->maintenanceAction = new KAction(KIcon("run-build-prune"), i18n("Maintenance..."), this);
    connect(d->maintenanceAction, SIGNAL(triggered()), this, SLOT(slotMaintenance()));
    actionCollection()->addAction("maintenance", d->maintenanceAction);

    // -----------------------------------------------------------

    KAction* const cameraAction = new KAction(i18n("Add Camera Manually..."), this);
    connect(cameraAction, SIGNAL(triggered()), this, SLOT(slotSetupCamera()));
    actionCollection()->addAction("camera_add", cameraAction);

    // -----------------------------------------------------------

    // Load Cameras -- do this before the createGUI so that the cameras
    // are plugged into the toolbar at startup
    if (d->splashScreen)
    {
        d->splashScreen->message(i18n("Loading cameras..."));
    }

    loadCameras();

    // Load Themes

    populateThemes();

    createGUI(xmlFile());
}

void DigikamApp::initGui()
{
    // Initialize Actions ---------------------------------------

    d->deleteAction->setEnabled(false);
    d->renameAction->setEnabled(false);
    d->addImagesAction->setEnabled(false);
    d->propsEditAction->setEnabled(false);
    d->openInFileManagerAction->setEnabled(false);
    d->openInTerminalAction->setEnabled(false);

    d->imageViewAction->setEnabled(false);
    d->imagePreviewAction->setEnabled(false);
    d->imageLightTableAction->setEnabled(false);
    d->imageAddLightTableAction->setEnabled(false);
    d->imageFindSimilarAction->setEnabled(false);
    d->imageRenameAction->setEnabled(false);
    d->imageDeleteAction->setEnabled(false);
    d->imageExifOrientationActionMenu->setEnabled(false);
    d->slideShowSelectionAction->setEnabled(false);

    d->albumSortAction->setCurrentItem((int)ApplicationSettings::instance()->getAlbumSortOrder());
    d->imageSortAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSortOrder());
    d->imageSortOrderAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSorting());
    d->imageGroupAction->setCurrentItem((int)ApplicationSettings::instance()->getImageGroupMode()-1); // no action for enum 0
    d->imageGroupSortOrderAction->setCurrentItem((int)ApplicationSettings::instance()->getImageGroupSortOrder());
    d->recurseAlbumsAction->setChecked(ApplicationSettings::instance()->getRecurseAlbums());
    d->recurseTagsAction->setChecked(ApplicationSettings::instance()->getRecurseTags());
    d->showBarAction->setChecked(ApplicationSettings::instance()->getShowThumbbar());
    d->showMenuBarAction->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080

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
            d->openInTerminalAction->setEnabled(false);
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
            d->openInTerminalAction->setEnabled(isNormalAlbum || isAlbumRoot);
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
        d->openInTerminalAction->setEnabled(false);
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
    /// @todo Currently only triggered by IconView, need to adapt to TableView
    int num_images = listAll.count();
    QString text;

    switch (selection.count())
    {
        case 0:
        {
            d->statusBarSelectionText = i18np("No item selected (%1 item)",
                                              "No item selected (%1 items)",
                                              num_images);
            break;
        }
        case 1:
        {
            slotSetCheckedExifOrientationAction(selection.first());

            int index = listAll.indexOf(selection.first()) + 1;

            d->statusBarSelectionText = selection.first().fileUrl().fileName()
                                        + i18n(" (%1 of %2)", index, num_images);
            break;
        }
        default:
        {
            d->statusBarSelectionText = i18np("%2/%1 item selected",
                                              "%2/%1 items selected",
                                              num_images, selection.count());
            break;
        }
    }

    d->statusLabel->setText(d->statusBarSelectionText);
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

    if (selectionCount > 0)
    {
        d->imageWriteMetadataAction->setText(i18np("Write Metadata to Image",
                                                   "Write Metadata to Selected Images", selectionCount));
        d->imageReadMetadataAction->setText(i18np("Reread Metadata From Image",
                                                  "Reread Metadata From Selected Images", selectionCount));

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
    d->cameraMenu->setText(i18n("Cameras"));
    d->cameraMenu->setIcon(KIcon("camera-photo"));
    d->usbMediaMenu->setText(i18n("USB Storage Devices"));
    d->usbMediaMenu->setIcon(KIcon("drive-removable-media-usb"));
    d->cardReaderMenu->setText(i18n("Card Readers"));
    d->cardReaderMenu->setIcon(KIcon("media-flash-smart-media"));

    actionCollection()->addAction("cameras",     d->cameraMenu);
    actionCollection()->addAction("usb_media",   d->usbMediaMenu);
    actionCollection()->addAction("card_reader", d->cardReaderMenu);

    // -----------------------------------------------------------------

    d->addImagesAction = new KAction(KIcon("albumfolder-importimages"), i18n("Add Images..."), this);
    d->addImagesAction->setShortcut(KShortcut(Qt::CTRL+Qt::ALT+Qt::Key_I));
    d->addImagesAction->setWhatsThis(i18n("Adds new items to an Album."));
    connect(d->addImagesAction, SIGNAL(triggered()), this, SLOT(slotImportAddImages()));
    actionCollection()->addAction("import_addImages", d->addImagesAction);

    // -----------------------------------------------------------------

    d->addFoldersAction = new KAction(KIcon("albumfolder-importdir"), i18n("Add Folders..."), this);
    d->addFoldersAction->setWhatsThis(i18n("Adds new folders to Album library."));
    connect(d->addFoldersAction, SIGNAL(triggered()), this, SLOT(slotImportAddFolders()));
    actionCollection()->addAction("import_addFolders", d->addFoldersAction);

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

    KAction* const cAction = new KAction(KIcon("camera-photo"), ctype->title(), d->manualCameraActionGroup);
    cAction->setData(ctype->title());
    actionCollection()->addAction(ctype->title().toUtf8(), cAction);

    ctype->setAction(cAction);
    updateCameraMenu();
    updateQuickImportAction();
}

void DigikamApp::slotCameraRemoved(KAction* cAction)
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
    ImportUI* const cgui = new ImportUI(this, i18n("Images found in %1", path),
                                                   "directory browse", "Fixed", path, 1);
    cgui->show();

    connect(cgui, SIGNAL(signalLastDestination(KUrl)),
            d->view, SLOT(slotSelectAlbum(KUrl)));
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
            ImportUI* const cgui = new ImportUI(this, ctype->title(), ctype->model(),
                                                ctype->port(), ctype->path(), ctype->startingNumber());

            ctype->setCurrentImportUI(cgui);

            cgui->show();

            connect(cgui, SIGNAL(signalLastDestination(KUrl)),
                    d->view, SLOT(slotSelectAlbum(KUrl)));
        }
    }
}

void DigikamApp::slotOpenSolidDevice(const QString& udi)
{
    // Identifies device as either Camera or StorageAccess and calls methods accordingly

    Solid::Device device(udi);

    if (!device.isValid())
    {
        KMessageBox::error(this, i18n("The specified device (\"%1\") is not valid.", udi));
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
            KMessageBox::error(this, i18n("The specified camera (\"%1\") is not supported.", udi));
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
        QList<QVariant> list = camera->driverHandle("gphoto").toList();

        // all sanity checks have already been done when creating the action
        if (list.size() < 3)
        {
            return;
        }

        // NOTE: See bug #262296: With KDE 4.6, Solid API return device vendor id
        // and product id in hexadecimal strings.
#if KDE_IS_VERSION(4,5,90)
        bool ok;
        int vendorId  = list.at(1).toString().toInt(&ok, 16);
        int productId = list.at(2).toString().toInt(&ok, 16);
#else
        int vendorId  = list.at(1).toInt();
        int productId = list.at(2).toInt();
#endif
        QString model, port;

        if (CameraList::findConnectedCamera(vendorId, productId, model, port))
        {
            kDebug() << "Found camera from ids " << vendorId << " " << productId
                     << " camera is: " << model << " at " << port;

            // the ImportUI will delete itself when it has finished
            ImportUI* const cgui = new ImportUI(this, cameraLabel, model, port, "/", 1);
            d->cameraUIMap[udi]  = cgui;

            cgui->show();

            connect(cgui, SIGNAL(signalLastDestination(KUrl)),
                    d->view, SLOT(slotSelectAlbum(KUrl)));
        }
        else
        {
            kError() << "Failed to detect camera with GPhoto2 from Solid information";
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
                KMessageBox::error(this, d->solidErrorMessage);
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
        ImportUI* const cgui = new ImportUI(this, i18n("Images on %1", mediaLabel),
                                                      "directory browse", "Fixed", path, 1);
        d->cameraUIMap[udi]  = cgui;

        cgui->show();

        connect(cgui, SIGNAL(signalLastDestination(KUrl)),
                d->view, SLOT(slotSelectAlbum(KUrl)));
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

    kDebug() << "fillSolidMenus: Found Camera " << cameraDevice.vendor() + ' ' + cameraDevice.product() << " protocols " << camera->supportedProtocols() << " drivers " << camera->supportedDrivers("ptp");

    // We handle gphoto2 cameras in this loop
    if (! (camera->supportedDrivers().contains("gphoto") || camera->supportedProtocols().contains("ptp")) )
    {
        return false;
    }

    QVariant driverHandle = camera->driverHandle("gphoto");

    if (!driverHandle.canConvert(QVariant::List))
    {
        kWarning() << "Solid returns unsupported driver handle for gphoto2";
        return false;
    }

    QList<QVariant> driverHandleList = driverHandle.toList();

    if (driverHandleList.size() < 3 || driverHandleList.at(0).toString() != "usb"
        || !driverHandleList.at(1).canConvert(QVariant::Int)
        || !driverHandleList.at(2).canConvert(QVariant::Int)
       )
    {
        kWarning() << "Solid returns unsupported driver handle for gphoto2";
        return false;
    }

    return true;
}

QString DigikamApp::labelForSolidCamera(const Solid::Device& cameraDevice)
{
    QString vendor  = cameraDevice.vendor();
    QString product = cameraDevice.product();

    if (product == "USB Imaging Interface" || product == "USB Vendor Specific Interface")
    {
        Solid::Device parentUsbDevice = cameraDevice.parent();

        if (parentUsbDevice.isValid())
        {
            vendor  = parentUsbDevice.vendor();
            product = parentUsbDevice.product();

            if (!vendor.isEmpty() && !product.isEmpty())
            {
                if (vendor == "Canon, Inc.")
                {
                    vendor = "Canon";

                    if (product.startsWith(QLatin1String("Canon ")))
                    {
                        product = product.mid(6);    // cut off another "Canon " from product
                    }

                    if (product.endsWith(QLatin1String(" (ptp)")))
                    {
                        product.chop(6);    // cut off " (ptp)"
                    }
                }
                else if (vendor == "Fuji Photo Film Co., Ltd")
                {
                    vendor = "Fuji";
                }
                else if (vendor == "Nikon Corp.")
                {
                    vendor = "Nikon";

                    if (product.startsWith(QLatin1String("NIKON ")))
                    {
                        product = product.mid(6);
                    }
                }
            }
        }
    }

    return vendor + ' ' + product;
}

void DigikamApp::fillSolidMenus()
{
    QHash<QString, QDateTime> newAppearanceTimes;
    d->usbMediaMenu->menu()->clear();
    d->cardReaderMenu->menu()->clear();

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
            iconName = "camera-photo";
        }

        KAction* const action = new KAction(label, d->solidCameraActionGroup);

        action->setIcon(KIcon(iconName));
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
            label = accessDevice.vendor() + ' ' + accessDevice.product();
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
                    label += i18nc("<drive type> \"<device name or label>\" at <mount path>",
                                   "%1 \"%2\" at %3", driveType, labelOrProduct, QDir::toNativeSeparators(access->filePath()));
                else
                    label += i18nc("<drive type> \"<device name or label>\"",
                                   "%1 \"%2\"", driveType, labelOrProduct);
            }
            else
            {
                if (!access->filePath().isEmpty())
                    label += i18nc("<drive type> at <mount path>",
                                   "%1 at %2", driveType, QDir::toNativeSeparators(access->filePath()));
                else
                {
                    label += driveType;
                }
            }

            if (volume->size())
                label += i18nc("device label etc... (<formatted byte size>)",
                               " (%1)", KGlobal::locale()->formatByteSize(volume->size()));
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

        KAction* const action = new KAction(label, d->solidUsmActionGroup);

        if (!iconName.isEmpty())
        {
            action->setIcon(KIcon(iconName));
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
    d->usbMediaMenu->setEnabled(!d->usbMediaMenu->menu()->isEmpty());
    d->cardReaderMenu->setEnabled(!d->cardReaderMenu->menu()->isEmpty());

    updateCameraMenu();
    updateQuickImportAction();
}

void DigikamApp::slotSetup()
{
    setup();
}

bool DigikamApp::setup()
{
    return Setup::exec(this, Setup::LastPageUsed);
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

    if (!AlbumManager::instance()->databaseEqual(ApplicationSettings::instance()->getDatabaseType(),
                                                 ApplicationSettings::instance()->getDatabaseName(), ApplicationSettings::instance()->getDatabaseHostName(),
                                                 ApplicationSettings::instance()->getDatabasePort(), ApplicationSettings::instance()->getInternalDatabaseServer()))
    {
        AlbumManager::instance()->changeDatabase(ApplicationSettings::instance()->getDatabaseParameters());
    }

    if (ApplicationSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        AlbumManager::instance()->prepareItemCounts();
    }

    // Load full-screen options
    KConfigGroup group = ApplicationSettings::instance()->generalConfigGroup();
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
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection(actionCollection(), i18nc("general keyboard shortcuts", "General"));

#ifdef HAVE_KIPI
    dialog.addCollection(KipiPluginLoader::instance()->pluginsActionCollection(),
                         i18nc("KIPI-Plugins keyboard shortcuts", "KIPI-Plugins"));
#endif /* HAVE_KIPI */

    dialog.configure();
}

void DigikamApp::slotConfToolbars()
{
    saveMainWindowSettings(ApplicationSettings::instance()->generalConfigGroup());
    KEditToolBar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolBarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void DigikamApp::slotNewToolbarConfig()
{
    applyMainWindowSettings(ApplicationSettings::instance()->generalConfigGroup());
}

void DigikamApp::slotConfNotifications()
{
    KNotifyConfigWidget::configure(this);
}

void DigikamApp::slotShowTip()
{
    QStringList tipsFiles;
    tipsFiles.append("digikam/tips");
    tipsFiles.append("kipi/tips");
    KTipDialog::showMultiTip(this, tipsFiles, true);
}

void DigikamApp::slotShowKipiHelp()
{
    KToolInvocation::invokeHelp( QString(), "kipi-plugins" );
}

void DigikamApp::slotDBStat()
{
    showDigikamDatabaseStat();
}

void DigikamApp::loadPlugins()
{
#ifdef HAVE_KIPI
    // Load KIPI plugins
    new KipiPluginLoader(this, d->splashScreen);
#endif /* HAVE_KIPI */

    // Setting the initial menu options after all plugins have been loaded
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();

    d->view->slotAlbumSelected(albumList);

    // Load Image Editor plugins.
    new ImagePluginLoader(this, d->splashScreen);
}

void DigikamApp::populateThemes()
{
    if (d->splashScreen)
    {
        d->splashScreen->message(i18n("Loading themes..."));
    }

    ThemeManager::instance()->setThemeMenuAction(new KActionMenu(i18n("&Themes"), this));
    ThemeManager::instance()->registerThemeActions(this);
    ThemeManager::instance()->setCurrentTheme(ApplicationSettings::instance()->getCurrentTheme());

    connect (ThemeManager::instance(), SIGNAL(signalThemeChanged()),
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
        d->splashScreen->message(i18n("Loading tools..."));
    }

    QueueMgrWindow::queueManagerWindow();
    ImageWindow::imageWindow();
    LightTableWindow::lightTableWindow();

    d->tagsActionManager->registerTagsActionCollections();
}

void DigikamApp::slotDatabaseMigration()
{
    MigrationDlg dlg(this);
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
    QString startingPath;
#if KDE_IS_VERSION(4,1,61)
    startingPath = KGlobalSettings::picturesPath();
#else
    startingPath = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
#endif
    QString path = KFileDialog::getExistingDirectory(startingPath, this,
                                                     i18n("Select folder to parse"));

    if (path.isEmpty())
    {
        return;
    }

    // The folder contents will be parsed by Camera interface in "Directory Browse" mode.
    downloadFrom(path);
}

void DigikamApp::slotImportAddFolders()
{
    QPointer<KFileDialog> dlg = new KFileDialog(KUrl(), "inode/directory", this);
    dlg->setCaption(i18n("Select folders to import into album"));
    dlg->setMode(KFile::Directory |  KFile::Files);

    if (dlg->exec() != QDialog::Accepted)
    {
        delete dlg;
        return;
    }

    KUrl::List urls = dlg->selectedUrls();
    delete dlg;

    if (urls.empty())
    {
        return;
    }

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    Album* album = 0;

    if(!albumList.isEmpty())
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

void DigikamApp::slotShowMenuBar()
{
    menuBar()->setVisible(d->showMenuBarAction->isChecked());
}

void DigikamApp::moveEvent(QMoveEvent*)
{
    emit signalWindowHasMoved();
}

void DigikamApp::updateCameraMenu()
{
    d->cameraMenu->menu()->clear();

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
    d->cameraMenu->addAction(actionCollection()->action("camera_add"));
}

void DigikamApp::updateQuickImportAction()
{
    d->quickImportMenu->menu()->clear();

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

    if (d->quickImportMenu->menu()->actions().isEmpty())
    {
        d->quickImportMenu->setEnabled(false);
    }
    else
    {
        disconnect(d->quickImportMenu, SIGNAL(triggered()), 0, 0);

        QAction* primaryAction = 0;
        QDateTime latest;

        foreach(QAction* const action, d->quickImportMenu->menu()->actions())
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
            primaryAction = d->quickImportMenu->menu()->actions().first();
        }

        connect(d->quickImportMenu, SIGNAL(triggered()),
                primaryAction, SLOT(trigger()));

        d->quickImportMenu->setDelayed(d->quickImportMenu->menu()->actions().size() > 1);
    }
}

void DigikamApp::setupExifOrientationActions()
{
    QSignalMapper* const exifOrientationMapper = new QSignalMapper(d->view);

    connect(exifOrientationMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotImageExifOrientation(int)));

    d->imageExifOrientationActionMenu = new KActionMenu(i18n("Adjust Exif Orientation Tag"), this);
    d->imageExifOrientationActionMenu->setDelayed(false);
    actionCollection()->addAction("image_set_exif_orientation", d->imageExifOrientationActionMenu);

    d->imageSetExifOrientation1Action = new KToggleAction(i18nc("normal exif orientation", "Normal"), this);
    d->imageSetExifOrientation2Action = new KToggleAction(i18n("Flipped Horizontally"), this);
    d->imageSetExifOrientation3Action = new KToggleAction(i18n("Rotated Upside Down"), this);
    d->imageSetExifOrientation4Action = new KToggleAction(i18n("Flipped Vertically"), this);
    d->imageSetExifOrientation5Action = new KToggleAction(i18n("Rotated Right / Horiz. Flipped"), this);
    d->imageSetExifOrientation6Action = new KToggleAction(i18n("Rotated Right"), this);
    d->imageSetExifOrientation7Action = new KToggleAction(i18n("Rotated Right / Vert. Flipped"), this);
    d->imageSetExifOrientation8Action = new KToggleAction(i18n("Rotated Left"), this);

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

    actionCollection()->addAction("image_set_exif_orientation_normal",
                                  d->imageSetExifOrientation1Action);
    actionCollection()->addAction("image_set_exif_orientation_flipped_horizontal",
                                  d->imageSetExifOrientation2Action);
    actionCollection()->addAction("image_set_exif_orientation_rotated_upside_down",
                                  d->imageSetExifOrientation3Action);
    actionCollection()->addAction("image_set_exif_orientation_flipped_vertically",
                                  d->imageSetExifOrientation4Action);
    actionCollection()->addAction("image_set_exif_orientation_rotated_right_hor_flipped",
                                  d->imageSetExifOrientation5Action);
    actionCollection()->addAction("image_set_exif_orientation_rotated_right",
                                  d->imageSetExifOrientation6Action);
    actionCollection()->addAction("image_set_exif_orientation_rotated_right_ver_flipped",
                                  d->imageSetExifOrientation7Action);
    actionCollection()->addAction("image_set_exif_orientation_rotated_left",
                                  d->imageSetExifOrientation8Action);

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
    d->imageRotateActionMenu = new KActionMenu(KIcon("object-rotate-right"), i18n("Rotate"), actionCollection());
    d->imageRotateActionMenu->setDelayed(false);

    KAction* const left = actionCollection()->addAction("rotate_ccw");
    left->setText(i18nc("rotate image left", "Left"));
    left->setShortcut(KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_Left));
    connect(left, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageRotateActionMenu->addAction(left);

    KAction* const right = actionCollection()->addAction("rotate_cw");
    right->setText(i18nc("rotate image right", "Right"));
    right->setShortcut(KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_Right));
    connect(right, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageRotateActionMenu->addAction(right);

    actionCollection()->addAction("image_rotate", d->imageRotateActionMenu);

    // -----------------------------------------------------------------------------------

    d->imageFlipActionMenu = new KActionMenu(KIcon("flip-horizontal"), i18n("Flip"), actionCollection());
    d->imageFlipActionMenu->setDelayed(false);

    KAction* const hori = actionCollection()->addAction("flip_horizontal");
    hori->setText(i18n("Horizontally"));
    hori->setShortcut(KShortcut(Qt::CTRL+Qt::Key_Asterisk));
    connect(hori, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageFlipActionMenu->addAction(hori);

    KAction* const verti = actionCollection()->addAction("flip_vertical");
    verti->setText(i18n("Vertically"));
    verti->setShortcut(KShortcut(Qt::CTRL+Qt::Key_Slash));
    connect(verti, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageFlipActionMenu->addAction(verti);

    actionCollection()->addAction("image_flip", d->imageFlipActionMenu);

    // -----------------------------------------------------------------------------------

    d->imageAutoExifActionMenu = new KAction(i18n("Auto Rotate/Flip Using Exif Information"), this);
    connect(d->imageAutoExifActionMenu, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));

    actionCollection()->addAction("image_transform_exif", d->imageAutoExifActionMenu);
}

void DigikamApp::slotTransformAction()
{
    if (sender()->objectName() == "rotate_ccw")
    {
        d->view->imageTransform(KExiv2Iface::RotationMatrix::Rotate270);
    }
    else if (sender()->objectName() == "rotate_cw")
    {
        d->view->imageTransform(KExiv2Iface::RotationMatrix::Rotate90);
    }
    else if (sender()->objectName() == "flip_horizontal")
    {
        d->view->imageTransform(KExiv2Iface::RotationMatrix::FlipHorizontal);
    }
    else if (sender()->objectName() == "flip_vertical")
    {
        d->view->imageTransform(KExiv2Iface::RotationMatrix::FlipVertical);
    }
    else if (sender()->objectName() == "image_transform_exif")
    {
        // special value for FileActionMngr
        d->view->imageTransform(KExiv2Iface::RotationMatrix::NoTransformation);
    }
}

KActionMenu* DigikamApp::slideShowMenu() const
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
    toogleShowBar();
}

void DigikamApp::slotSwitchedToIconView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    d->imageIconViewAction->setChecked(true);
    toogleShowBar();
}

void DigikamApp::slotSwitchedToMapView()
{
    //TODO: Link to map view's zoom actions
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
#ifdef HAVE_KGEOMAP
    d->imageMapViewAction->setChecked(true);
#endif // HAVE_KGEOMAP
    toogleShowBar();
}

void DigikamApp::slotSwitchedToTableView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    d->imageTableViewAction->setChecked(true);
    toogleShowBar();
}

void DigikamApp::customizedFullScreenMode(bool set)
{
    statusBarMenuAction()->setEnabled(!set);
    toolBarMenuAction()->setEnabled(!set);
    d->showMenuBarAction->setEnabled(!set);
    set ? d->showBarAction->setEnabled(false)
        : toogleShowBar();

    d->view->toggleFullScreen(set);
}

void DigikamApp::toogleShowBar()
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

}  // namespace Digikam
