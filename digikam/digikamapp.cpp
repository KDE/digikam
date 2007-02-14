/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Tom Albers <tomalbers@kde.nl>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2002-16-10
 * Description : main digiKam interface implementation
 * 
 * Copyright 2002-2005 by Renchi Raju and Gilles Caulier
 * Copyright      2006 by Tom Albers
 * Copyright 2006-2007 by Gilles Caulier
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

// Qt includes.

#include <qdatastream.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qsignalmapper.h>
#include <qdockarea.h>


// KDE includes.

#include <kaboutdata.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kstdaction.h>
#include <kstdaccel.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kiconloader.h>
#include <ktip.h>
#include <kdeversion.h>
#include <kapplication.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kwin.h>
#include <dcopref.h>

// libKipi includes.

#include <libkipi/plugin.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albumlister.h"
#include "albumthumbnailloader.h"
#include "cameratype.h"
#include "cameraui.h"
#include "setup.h"
#include "setupplugins.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "setupimgplugins.h"
#include "imagewindow.h"
#include "imageinfo.h"
#include "thumbnailsize.h"
#include "themeengine.h"
#include "scanlib.h"
#include "loadingcacheinterface.h"
#include "imageattributeswatch.h"
#include "dcrawbinary.h"
#include "batchthumbsgenerator.h"
#include "batchalbumssyncmetadata.h"
#include "dcopiface.h"
#include "digikamappprivate.h"
#include "digikamapp.h"
#include "digikamapp.moc"

using KIO::Job;
using KIO::UDSEntryList;
using KIO::UDSEntry;

namespace Digikam
{

DigikamApp* DigikamApp::m_instance = 0;

DigikamApp::DigikamApp()
          : KMainWindow( 0, "Digikam" )
{
    d = new DigikamAppPriv;
    m_instance = this;
    d->config  = kapp->config();

    if(d->config->readBoolEntry("Show Splash", true) &&
       !kapp->isRestored())
    {
        d->splashScreen = new SplashScreen("digikam-splash.png");
    }

    d->albumSettings = new AlbumSettings();
    d->albumSettings->readSettings();

    d->albumManager = AlbumManager::instance();
    AlbumLister::instance();

    d->cameraMediaList = new KPopupMenu;

    connect(d->cameraMediaList, SIGNAL( aboutToShow() ),
            this, SLOT(slotCameraMediaMenu()));

    d->cameraList = new CameraList(this, locateLocal("appdata", "cameras.xml"));

    connect(d->cameraList, SIGNAL(signalCameraAdded(CameraType *)),
            this, SLOT(slotCameraAdded(CameraType *)));

    connect(d->cameraList, SIGNAL(signalCameraRemoved(CameraType *)),
            this, SLOT(slotCameraRemoved(CameraType *)));

    setupView();
    setupStatusBar();
    setupAccelerators();
    setupActions();

    applyMainWindowSettings(d->config);

    // Check ICC profiles repository availability

    if(d->splashScreen)
        d->splashScreen->message(i18n("Checking ICC repository"), AlignLeft, white);

    d->validIccPath = SetupICC::iccRepositoryIsValid();

    // Check witch dcraw version available

    if(d->splashScreen)
        d->splashScreen->message(i18n("Checking dcraw version"), AlignLeft, white);

    DcrawBinary::instance()->checkSystem();

    // Actual file scanning is done in main() - is this necessary here?
    d->albumManager->setLibraryPath(d->albumSettings->getAlbumLibraryPath());

    // Read albums from database
    if(d->splashScreen)
        d->splashScreen->message(i18n("Reading database"), AlignLeft, white);

    d->albumManager->startScan();

    // Load KIPI Plugins.
    loadPlugins();

    // Load Themes
    populateThemes();

    setAutoSaveSettings();

    d->dcopIface = new DCOPIface(this, "camera");

    connect(d->dcopIface, SIGNAL(signalCameraAutoDetect()), 
            this, SLOT(slotDcopCameraAutoDetect()));

    connect(d->dcopIface, SIGNAL(signalDownloadImages( const QString & )),
            this, SLOT(slotDcopDownloadImages(const QString &)));
}

DigikamApp::~DigikamApp()
{
    ImageAttributesWatch::shutDown();

    if (ImageWindow::imagewindowCreated())
        // close and delete
        ImageWindow::imagewindow()->close(true);

    if (d->view)
        delete d->view;

    d->albumSettings->saveSettings();
    delete d->albumSettings;

    delete d->albumManager;
    delete AlbumLister::instance();

    ImageAttributesWatch::cleanUp();
    LoadingCacheInterface::cleanUp();
    DcrawBinary::cleanUp();
    AlbumThumbnailLoader::cleanUp();

    m_instance = 0;
    delete d;
}

DigikamApp* DigikamApp::getinstance()
{
    return m_instance;
}

void DigikamApp::show()
{
    // Remove Splashscreen.

    if(d->splashScreen)
    {
        d->splashScreen->finish(this);
        delete d->splashScreen;
        d->splashScreen = 0;
    }

    // Display application window.

    KMainWindow::show();

    // Report errors from ICC repository path.

    if(!d->validIccPath)
    {
        QString message = i18n("<qt><p>ICC profiles path seems to be invalid.</p>"
                               "<p>If you want to set it now, select \"Yes\", otherwise "
                               "select \"No\". In this case, \"Color Management\" feature "
                               "will be disabled until you solve this issue</p></qt>");

        if (KMessageBox::warningYesNo(this, message) == KMessageBox::Yes)
        {
            if (!setup(true))
            {
                d->config->setGroup("Color Management");
                d->config->writeEntry("EnableCM", false);
                d->config->sync();
            }
        }
        else
        {
            d->config->setGroup("Color Management");
            d->config->writeEntry("EnableCM", false);
            d->config->sync();
        }
    }

    // Report errors from dcraw detection.

    DcrawBinary::instance()->checkReport();  
}

const QPtrList<KAction>& DigikamApp::menuImageActions()
{
    return d->kipiImageActions;
}

const QPtrList<KAction>& DigikamApp::menuBatchActions()
{
    return d->kipiBatchActions;
}

const QPtrList<KAction>& DigikamApp::menuAlbumActions()
{
    return d->kipiAlbumActions;
}

const QPtrList<KAction> DigikamApp::menuImportActions()
{
    QPtrList<KAction> importMenu;
    importMenu = d->kipiFileActionsImport;
    importMenu.append( d->albumImportAction );
    importMenu.append( d->addImagesAction );
    return importMenu;
}

const QPtrList<KAction> DigikamApp::menuExportActions()
{
    return d->kipiFileActionsExport;
}

void DigikamApp::autoDetect()
{
    // Called from main if command line option is set

    if(d->splashScreen)
        d->splashScreen->message(i18n("Auto-detect camera"), AlignLeft, white);

    QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
}

void DigikamApp::downloadFrom(const QString &cameraGuiPath)
{
    // Called from main if command line option is set

    if (!cameraGuiPath.isNull())
    {
        d->cameraGuiPath = cameraGuiPath;

        if(d->splashScreen)
            d->splashScreen->message(i18n("Opening Download Dialog"), AlignLeft, white);

        QTimer::singleShot(0, this, SLOT(slotDownloadImages()));
    }
}

bool DigikamApp::queryClose()
{
    if (ImageWindow::imagewindowCreated())
    {
        return ImageWindow::imagewindow()->queryClose();
    }
    else
        return true;
}

void DigikamApp::setupView()
{
    if(d->splashScreen)
        d->splashScreen->message(i18n("Initializing Main View"), AlignLeft, white);

    d->view = new DigikamView(this);
    setCentralWidget(d->view);
    d->view->applySettings(d->albumSettings);

    connect(d->view, SIGNAL(signalAlbumSelected(bool)),
            this, SLOT(slotAlbumSelected(bool)));
            
    connect(d->view, SIGNAL(signalTagSelected(bool)),
            this, SLOT(slotTagSelected(bool)));

    connect(d->view, SIGNAL(signalImageSelected(const QPtrList<ImageInfo>&, bool, bool)),
            this, SLOT(slotImageSelected(const QPtrList<ImageInfo>&, bool, bool)));
}

void DigikamApp::setupStatusBar()
{
    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    d->statusProgressBar->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(d->statusProgressBar, 100, true);

    d->statusNavigateBar = new StatusNavigateBar(statusBar());
    d->statusNavigateBar->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(d->statusNavigateBar, 1, true);

    connect(d->statusNavigateBar, SIGNAL(signalFirstItem()),
            d->view, SLOT(slotFirstItem()));

    connect(d->statusNavigateBar, SIGNAL(signalNextItem()),
            d->view, SLOT(slotNextItem()));

    connect(d->statusNavigateBar, SIGNAL(signalPrevItem()),
            d->view, SLOT(slotPrevItem()));

    connect(d->statusNavigateBar, SIGNAL(signalLastItem()),
            d->view, SLOT(slotLastItem()));

    connect(d->statusProgressBar, SIGNAL(signalCancelButtonPressed()),
            this, SIGNAL(signalCancelButtonPressed()));
}

void DigikamApp::setupAccelerators()
{
    d->accelerators = new KAccel(this);

    d->accelerators->insert("Exit Preview Mode", i18n("Exit Preview"),
                           i18n("Exit out of the preview mode"),
                           Key_Escape, this, SIGNAL(signalEscapePressed()),
                           false, true);
    
    d->accelerators->insert("Next Image Key_Space", i18n("Next Image"),
                           i18n("Next Image"),
                           Key_Space, this, SIGNAL(signalNextItem()),
                           false, true);

    d->accelerators->insert("Previous Image Key_Backspace", i18n("Previous Image"),
                           i18n("Previous Image"),
                           Key_Backspace, this, SIGNAL(signalPrevItem()),
                           false, true);

    d->accelerators->insert("Next Image Key_Next", i18n("Next Image"),
                           i18n("Next Image"),
                           Key_Next, this, SIGNAL(signalNextItem()),
                           false, true);

    d->accelerators->insert("Previous Image Key_Prior", i18n("Previous Image"),
                           i18n("Previous Image"),
                           Key_Prior, this, SIGNAL(signalPrevItem()),
                           false, true);

    d->accelerators->insert("First Image Key_Home", i18n("First Image"),
                           i18n("First Image"),
                           Key_Home, this, SIGNAL(signalFirstItem()),
                           false, true);

    d->accelerators->insert("Last Image Key_End", i18n("Last Image"),
                           i18n("Last Image"),
                           Key_End, this, SIGNAL(signalLastItem()),
                           false, true);

    d->accelerators->insert("Copy Album Items Selection CTRL+Key_C", i18n("Copy Album Items Selection"),
                           i18n("Copy Album Items Selection"),
                           CTRL+Key_C, this, SIGNAL(signalCopyAlbumItemsSelection()),
                           false, true);

    d->accelerators->insert("Paste Album Items Selection CTRL+Key_V", i18n("Paste Album Items Selection"),
                           i18n("Paste Album Items Selection"),
                           CTRL+Key_V, this, SIGNAL(signalPasteAlbumItemsSelection()),
                           false, true);
}

void DigikamApp::setupActions()
{
    // -----------------------------------------------------------------

    d->cameraMenuAction = new KActionMenu(i18n("&Camera"),
                                    "digitalcam",
                                    actionCollection(),
                                    "camera_menu");
    d->cameraMenuAction->setDelayed(false);

    // -----------------------------------------------------------------

    d->themeMenuAction = new KSelectAction(i18n("&Themes"), 0, actionCollection(), "theme_menu");
    connect(d->themeMenuAction, SIGNAL(activated(const QString&)),
            this, SLOT(slotChangeTheme(const QString&)));

    // -----------------------------------------------------------------

    d->backwardActionMenu = new KToolBarPopupAction(i18n("&Back"),
                                    "back",
                                    ALT+Key_Left,
                                    d->view,
                                    SLOT(slotAlbumHistoryBack()),
                                    actionCollection(),
                                    "album_back");
    d->backwardActionMenu->setEnabled(false);

    connect(d->backwardActionMenu->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowBackwardMenu()));
    
    connect(d->backwardActionMenu->popupMenu(), SIGNAL(activated(int)),
            d->view, SLOT(slotAlbumHistoryBack(int)));

    d->forwardActionMenu = new  KToolBarPopupAction(i18n("Forward"),
                                    "forward",
                                    ALT+Key_Right,
                                    d->view,
                                    SLOT(slotAlbumHistoryForward()),
                                    actionCollection(),
                                    "album_forward");
    d->forwardActionMenu->setEnabled(false);
    
    connect(d->forwardActionMenu->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowForwardMenu()));

    connect(d->forwardActionMenu->popupMenu(), SIGNAL(activated(int)),
            d->view, SLOT(slotAlbumHistoryForward(int)));

    d->newAction = new KAction(i18n("&New Album..."),
                                   "albumfolder-new",
                                   KStdAccel::shortcut(KStdAccel::New),
                                   d->view,
                                   SLOT(slotNewAlbum()),
                                   actionCollection(),
                                   "album_new");
    d->newAction->setWhatsThis(i18n("This option creates a new empty Album in the database."));

    d->albumSortAction = new KSelectAction(i18n("&Sort Albums"),
                                    0,
                                    0,
                                    actionCollection(),
                                    "album_sort");

    connect(d->albumSortAction, SIGNAL(activated(int)),
            d->view, SLOT(slotSortAlbums(int)));

    // Use same list order as in albumsettings enum
    QStringList sortActionList;
    sortActionList.append(i18n("By Folder"));
    sortActionList.append(i18n("By Collection"));
    sortActionList.append(i18n("By Date"));
    d->albumSortAction->setItems(sortActionList);

    d->deleteAction = new KAction(i18n("Delete Album"),
                                    "editdelete",
                                    0,
                                    d->view,
                                    SLOT(slotDeleteAlbum()),
                                    actionCollection(),
                                    "album_delete");

    d->addImagesAction = new KAction( i18n("Add Images..."),
                                    "albumfolder-importimages",
                                    CTRL+Key_I,
                                    d->view,
                                    SLOT(slotAlbumAddImages()),
                                    actionCollection(),
                                    "album_addImages");
    d->addImagesAction->setWhatsThis(i18n("This option adds new items to the current Album."));

    d->albumImportAction = new KAction( i18n("Import Folders..."),
                                    "albumfolder-importdir",
                                    0,
                                    d->view,
                                    SLOT(slotAlbumImportFolder()),
                                    actionCollection(),
                                    "album_importFolder");

    d->propsEditAction = new KAction( i18n("Edit Album Properties..."),
                                    "albumfolder-properties",
                                    0,
                                    d->view,
                                    SLOT(slotAlbumPropsEdit()),
                                    actionCollection(),
                                    "album_propsEdit");
    d->propsEditAction->setWhatsThis(i18n("This option allows you to set the Album Properties information "
                                        "about the Collection."));

    d->refreshAlbumAction = new KAction( i18n("Refresh"),
                                    "rebuild",
                                    Key_F5,
                                    d->view,
                                    SLOT(slotAlbumRefresh()),
                                    actionCollection(),
                                    "album_refresh");
    d->refreshAlbumAction->setWhatsThis(i18n("This option refresh all album content."));

    d->syncAlbumMetadataAction = new KAction( i18n("Sync Pictures Metadata"),
                                    "rebuild",
                                    0,
                                    d->view,
                                    SLOT(slotAlbumSyncPicturesMetadata()),
                                    actionCollection(),
                                    "album_syncmetadata");
    d->syncAlbumMetadataAction->setWhatsThis(i18n("This option sync pictures metadata from current "
                                                "album with digiKam database contents."));

    d->openInKonquiAction = new KAction( i18n("Open in Konqueror"),
                                    "konqueror",
                                    0,
                                    d->view,
                                    SLOT(slotAlbumOpenInKonqui()),
                                    actionCollection(),
                                    "album_openinkonqui");

    // -----------------------------------------------------------

    d->newTagAction = new KAction(i18n("New &Tag..."), "tag-new",
                                0, d->view, SLOT(slotNewTag()),
                                actionCollection(), "tag_new");

    d->editTagAction = new KAction(i18n("Edit Tag Properties..."), "tag-properties",
                                 0, d->view, SLOT(slotEditTag()),
                                 actionCollection(), "tag_edit");

    d->deleteTagAction = new KAction(i18n("Delete Tag"), "tag-delete",
                                   0, d->view, SLOT(slotDeleteTag()),
                                   actionCollection(), "tag_delete");

    // -----------------------------------------------------------

    d->imagePreviewAction = new KToggleAction(i18n("View..."),
                                    "viewimage",
                                    Key_F3,
                                    d->view,
                                    SLOT(slotImagePreview()),
                                    actionCollection(),
                                    "image_view");

    d->imageViewAction = new KAction(i18n("Edit..."),
                                    "editimage",
                                    Key_F4,
                                    d->view,
                                    SLOT(slotImageEdit()),
                                    actionCollection(),
                                    "image_edit");
    d->imageViewAction->setWhatsThis(i18n("This option allows you to open the editor with the "
                                        "current selected item."));

    d->imageRenameAction = new KAction(i18n("Rename..."),
                                    "pencil",
                                    Key_F2,
                                    d->view,
                                    SLOT(slotImageRename()),
                                    actionCollection(),
                                    "image_rename");
    d->imageRenameAction->setWhatsThis(i18n("This option allows you to rename the filename "
                                          "of the current selected item"));

    // Pop up dialog to ask user whether to move to trash
    d->imageDeleteAction            = new KAction(i18n("Delete"),
                                                "edittrash",
                                                Key_Delete,
                                                d->view,
                                                SLOT(slotImageDelete()),
                                                actionCollection(),
                                                "image_delete");

    // Pop up dialog to ask user whether to permanently delete
    d->imageDeletePermanentlyAction = new KAction(i18n("Delete Permanently"),
                                                "editdelete",
                                                SHIFT+Key_Delete,
                                                d->view,
                                                SLOT(slotImageDeletePermanently()),
                                                actionCollection(),
                                                "image_delete_permanently");

    // These two actions are hidden, no menu entry, no toolbar entry, no shortcut.
    // Power users may add them.
    d->imageDeletePermanentlyDirectlyAction = new KAction(i18n("Delete Permanently without Confirmation"),
                                                        "editdelete",
                                                        0,
                                                        d->view,
                                                        SLOT(slotImageDeletePermanentlyDirectly()),
                                                        actionCollection(),
                                                        "image_delete_permanently_directly");

    d->imageTrashDirectlyAction             = new KAction(i18n("Move to Trash without Confirmation"),
                                                        "edittrash",
                                                        0,
                                                        d->view,
                                                        SLOT(slotImageTrashDirectly()),
                                                        actionCollection(),
                                                        "image_trash_directly");

    d->imageSortAction = new KSelectAction(i18n("&Sort Images"),
                                    0,
                                    0,
                                    actionCollection(),
                                    "image_sort");

    connect(d->imageSortAction, SIGNAL(activated(int)),
            d->view, SLOT(slotSortImages(int)));

    // Use same list order as in albumsettings enum
    QStringList sortImagesActionList;
    sortImagesActionList.append(i18n("By Name"));
    sortImagesActionList.append(i18n("By Path"));
    sortImagesActionList.append(i18n("By Date"));
    sortImagesActionList.append(i18n("By File Size"));
    sortImagesActionList.append(i18n("By Rating"));
    d->imageSortAction->setItems(sortImagesActionList);

    // -----------------------------------------------------------------

    QSignalMapper *exifOrientationMapper = new QSignalMapper( d->view );
    
    connect( exifOrientationMapper, SIGNAL( mapped( int ) ),
             d->view, SLOT( slotImageExifOrientation( int ) ) );

    d->imageExifOrientationActionMenu = new KActionMenu(i18n("Correct Exif Orientation Tag"),
                                                      actionCollection(),
                                                      "image_set_exif_orientation");
    d->imageExifOrientationActionMenu->setDelayed(false);

    d->imageSetExifOrientation1Action =
        new KAction(i18n("Normal"),0,d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation2Action =
        new KAction(i18n("Flipped Horizontally"),0,d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation3Action =
        new KAction(i18n("Rotated 180 Degrees"),0,d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation4Action =
        new KAction(i18n("Flipped Vertically"),0,d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation5Action =
        new KAction(i18n("Rotated 90 Degrees / Horiz. Flipped"),
                    0, d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation6Action =
        new KAction(i18n("Rotated 90 Degrees"),0,d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation7Action =
        new KAction(i18n("Rotated 90 Degrees / Vert. Flipped"),
                    0, d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation8Action =
        new KAction(i18n("Rotated 270 Degrees"),0,d->imageExifOrientationActionMenu);

    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation1Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation2Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation3Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation4Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation5Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation6Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation7Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation8Action);

    connect( d->imageSetExifOrientation1Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( d->imageSetExifOrientation2Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( d->imageSetExifOrientation3Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( d->imageSetExifOrientation4Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( d->imageSetExifOrientation5Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( d->imageSetExifOrientation6Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( d->imageSetExifOrientation7Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( d->imageSetExifOrientation8Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );

    exifOrientationMapper->setMapping( d->imageSetExifOrientation1Action, 1);
    exifOrientationMapper->setMapping( d->imageSetExifOrientation2Action, 2);
    exifOrientationMapper->setMapping( d->imageSetExifOrientation3Action, 3);
    exifOrientationMapper->setMapping( d->imageSetExifOrientation4Action, 4);
    exifOrientationMapper->setMapping( d->imageSetExifOrientation5Action, 5);
    exifOrientationMapper->setMapping( d->imageSetExifOrientation6Action, 6);
    exifOrientationMapper->setMapping( d->imageSetExifOrientation7Action, 7);
    exifOrientationMapper->setMapping( d->imageSetExifOrientation8Action, 8);

    // -----------------------------------------------------------------

    d->selectAllAction = new KAction(i18n("Select All"),
                                    0,
                                    CTRL+Key_A,
                                    d->view,
                                    SLOT(slotSelectAll()),
                                    actionCollection(),
                                    "selectAll");

    d->selectNoneAction = new KAction(i18n("Select None"),
                                    0,
                                    CTRL+Key_U,
                                    d->view,
                                    SLOT(slotSelectNone()),
                                    actionCollection(),
                                    "selectNone");

    d->selectInvertAction = new KAction(i18n("Invert Selection"),
                                    0,
                                    CTRL+Key_Asterisk,
                                    d->view,
                                    SLOT(slotSelectInvert()),
                                    actionCollection(),
                                    "selectInvert");

    // -----------------------------------------------------------

    KStdAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStdAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStdAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // -----------------------------------------------------------

    d->thumbSizePlusAction = new KAction(i18n("Increase Thumbnail Size"),
                                   "viewmag+",
                                   CTRL+Key_Plus,
                                   d->view,
                                   SLOT(slotThumbSizePlus()),
                                   actionCollection(),
                                   "album_thumbSizeIncrease");
    d->thumbSizePlusAction->setWhatsThis(i18n("This option allows you to increase "
                                            "the Album thumbnails size."));

    d->thumbSizeMinusAction = new KAction(i18n("Decrease Thumbnail Size"),
                                   "viewmag-",
                                   CTRL+Key_Minus,
                                   d->view,
                                   SLOT(slotThumbSizeMinus()),
                                   actionCollection(),
                                   "album_thumbSizeDecrease");
    d->thumbSizeMinusAction->setWhatsThis(i18n("This option allows you to decrease "
                                             "the Album thumbnails size."));

#if KDE_IS_VERSION(3,2,0)
    d->fullScreenAction = KStdAction::fullScreen(this, SLOT(slotToggleFullScreen()),
                                               actionCollection(), this, "full_screen");
#else
    d->fullScreenAction = new KToggleAction(i18n("Full Screen Mode"),
                                   "window_fullscreen",
                                   CTRL+SHIFT+Key_F,
                                   this,
                                   SLOT(slotToggleFullScreen()),
                                   actionCollection(),
                                   "full_screen");
    d->fullScreenAction->setWhatsThis(i18n("This option allows you to toggle the main window "
                                         "in full screen mode."));
#endif

    d->slideShowAction = new KActionMenu(i18n("Slide Show"), "slideshow",
                                         actionCollection(), "slideshow");

    d->slideShowAction->setDelayed(false);

    d->slideShowAllAction = new KAction(i18n("All"), 0, Key_F9,
                                d->view, SLOT(slotSlideShowAll()),
                                actionCollection(), "slideshow_all");
    d->slideShowAction->insert(d->slideShowAllAction);

    d->slideShowSelectionAction = new KAction(i18n("Selection"), 0, ALT+Key_F9,
                                              d->view, 
                                              SLOT(slotSlideShowSelection()),
                                              actionCollection(), 
                                              "slideshow_selected");
    d->slideShowAction->insert(d->slideShowSelectionAction);

    d->slideShowRecursiveAction = new KAction(i18n("Recursive"), 0, SHIFT+Key_F9,
                                              d->view, 
                                              SLOT(slotSlideShowRecursive()),
                                              actionCollection(), 
                                              "slideshow_recursive");
    d->slideShowAction->insert(d->slideShowRecursiveAction);

    d->quitAction = KStdAction::quit(this,
                                   SLOT(slotExit()),
                                   actionCollection(),
                                   "app_exit");

    d->kipiHelpAction = new KAction(i18n("Kipi Plugins Handbook"),
                                   "kipi",
                                   0,
                                   this,
                                   SLOT(slotShowKipiHelp()),
                                   actionCollection(),
                                   "help_kipi");

    d->tipAction = KStdAction::tipOfDay(this,
                                   SLOT(slotShowTip()),
                                   actionCollection(),
                                   "help_tipofday");

    d->donateMoneyAction = new KAction(i18n("Donate Money..."),
                                   0,
                                   0,
                                   this,
                                   SLOT(slotDonateMoney()),
                                   actionCollection(),
                                   "help_donatemoney");

    // -- Rating actions ---------------------------------------------------------------

    d->rating0Star = new KAction(i18n("Assign Rating \"No Star\""), CTRL+Key_0,
                          d->view, SLOT(slotAssignRatingNoStar()),
                          actionCollection(), "imageview_ratenostar");
    d->rating1Star = new KAction(i18n("Assign Rating \"One Star\""), CTRL+Key_1,
                          d->view, SLOT(slotAssignRatingOneStar()),
                          actionCollection(), "imageview_rateonestar");
    d->rating2Star = new KAction(i18n("Assign Rating \"Two Stars\""), CTRL+Key_2, 
                          d->view, SLOT(slotAssignRatingTwoStar()),
                          actionCollection(), "imageview_ratetwostar");
    d->rating3Star = new KAction(i18n("Assign Rating \"Three Stars\""), CTRL+Key_3, 
                          d->view, SLOT(slotAssignRatingThreeStar()),
                          actionCollection(), "imageview_ratethreestar");
    d->rating4Star = new KAction(i18n("Assign Rating \"Four Stars\""), CTRL+Key_4, 
                          d->view, SLOT(slotAssignRatingFourStar()),
                          actionCollection(), "imageview_ratefourstar");
    d->rating5Star = new KAction(i18n("Assign Rating \"Five Stars\""), CTRL+Key_5, 
                          d->view, SLOT(slotAssignRatingFiveStar()),
                          actionCollection(), "imageview_ratefivestar");

    // -----------------------------------------------------------

    KAction* findAction = KStdAction::find(d->view, SLOT(slotNewQuickSearch()),
                                           actionCollection(), "search_quick");
    findAction->setText(i18n("Quick Search..."));
    findAction->setIconSet(BarIcon("filefind"));

    KAction* advFindAction = KStdAction::find(d->view, SLOT(slotNewAdvancedSearch()),
                                              actionCollection(), "search_advanced");
    advFindAction->setText(i18n("Advanced Search..."));
    advFindAction->setShortcut("Ctrl+Alt+F");

    new KAction(i18n("Scan for New Images"), "reload_page", 0,
                this, SLOT(slotDatabaseRescan()), actionCollection(), 
                "database_rescan");

    new KAction(i18n("Rebuild all Thumbnails..."), "reload_page", 0,
                this, SLOT(slotRebuildAllThumbs()), actionCollection(),
                "thumbs_rebuild");

    new KAction(i18n("Sync all Pictures Metadata..."), "reload_page", 0,
                this, SLOT(slotSyncAllPicturesMetadata()), actionCollection(),
                "sync_metadata");

    // -----------------------------------------------------------

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // Load Cameras -- do this before the createGUI so that the cameras
    // are plugged into the toolbar at startup
    if (d->splashScreen)
        d->splashScreen->message(i18n("Loading cameras"), AlignLeft, white);
    
    loadCameras();

    createGUI(QString::fromLatin1( "digikamui.rc" ), false);

    // Initialize Actions ---------------------------------------

    d->deleteAction->setEnabled(false);
    d->addImagesAction->setEnabled(false);
    d->propsEditAction->setEnabled(false);
    d->openInKonquiAction->setEnabled(false);

    d->imageViewAction->setEnabled(false);
    d->imagePreviewAction->setEnabled(false);
    d->imageRenameAction->setEnabled(false);
    d->imageDeleteAction->setEnabled(false);
    d->imageExifOrientationActionMenu->setEnabled(false);
    d->slideShowSelectionAction->setEnabled(false);

    d->albumSortAction->setCurrentItem((int)d->albumSettings->getAlbumSortOrder());
    d->imageSortAction->setCurrentItem((int)d->albumSettings->getImageSortOrder());

    int size = d->albumSettings->getDefaultIconSize();
    if (size == ThumbnailSize::Huge)
    {
        enableThumbSizePlusAction(false);
        enableThumbSizeMinusAction(true);
    }
    else if (size == ThumbnailSize::Small)
    {
        enableThumbSizePlusAction(true);
        enableThumbSizeMinusAction(false);
    }
    else
    {
        enableThumbSizePlusAction(true);
        enableThumbSizeMinusAction(true);
    }
}

void DigikamApp::enableThumbSizePlusAction(bool val)
{
    d->thumbSizePlusAction->setEnabled(val);
}

void DigikamApp::enableThumbSizeMinusAction(bool val)
{
    d->thumbSizeMinusAction->setEnabled(val);
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
    d->backwardActionMenu->popupMenu()->clear();
    QStringList titles;
    d->view->getBackwardHistory(titles);
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();
        for(; iter != titles.end(); ++iter,++id)
        {
            d->backwardActionMenu->popupMenu()->insertItem(*iter, id);
        }
    }
}

void DigikamApp::slotAboutToShowForwardMenu()
{
    d->forwardActionMenu->popupMenu()->clear();
    QStringList titles;
    d->view->getForwardHistory(titles);
    
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();
        for(; iter != titles.end(); ++iter,++id)
        {
            d->forwardActionMenu->popupMenu()->insertItem(*iter, id);
        }
    }
}

void DigikamApp::slotAlbumSelected(bool val)
{
    Album *album = d->albumManager->currentAlbum();
    
    if(album && !val)
    {
        // No PAlbum is selected
        d->deleteAction->setEnabled(false);
        d->addImagesAction->setEnabled(false);
        d->propsEditAction->setEnabled(false);
        d->openInKonquiAction->setEnabled(false);
        d->newAction->setEnabled(false);
        d->albumImportAction->setEnabled(false);

    }
    else if(!album && !val)
    {
        // Groupitem selected (Collection/date)
        d->deleteAction->setEnabled(false);
        d->addImagesAction->setEnabled(false);
        d->propsEditAction->setEnabled(false);
        d->openInKonquiAction->setEnabled(false);
        d->newAction->setEnabled(false);
        d->albumImportAction->setEnabled(false);
        
        KAction *action;
        for (action = d->kipiFileActionsImport.first(); action;
             action = d->kipiFileActionsImport.next())
        {
            action->setEnabled(false);
        }

        for (action = d->kipiFileActionsExport.first(); action;
             action = d->kipiFileActionsExport.next())
        {
            action->setEnabled(false);
        }
    }
    else if(album && !album->isRoot() && album->type() == Album::PHYSICAL)
    {
        // Normal Album selected
        d->deleteAction->setEnabled(true);
        d->addImagesAction->setEnabled(true);
        d->propsEditAction->setEnabled(true);
        d->openInKonquiAction->setEnabled(true);
        d->newAction->setEnabled(true);
        d->albumImportAction->setEnabled(true);
        
        KAction *action;
        for (action = d->kipiFileActionsImport.first(); action; 
             action = d->kipiFileActionsImport.next())
        {
            action->setEnabled(true);
        }

        for (action = d->kipiFileActionsExport.first(); action; 
             action = d->kipiFileActionsExport.next())
        {
            action->setEnabled(true);    
        }        
    }
    else if(album && album->isRoot() && album->type() == Album::PHYSICAL)
    {
        // Root Album selected
        d->deleteAction->setEnabled(false);
        d->addImagesAction->setEnabled(false);
        d->propsEditAction->setEnabled(false);
       

        if(album->type() == Album::PHYSICAL)
        {
            d->newAction->setEnabled(true);
            d->openInKonquiAction->setEnabled(true);
            d->albumImportAction->setEnabled(true);
        }
        else
        {
            d->newAction->setEnabled(false);
            d->openInKonquiAction->setEnabled(false);
            d->albumImportAction->setEnabled(false);            
        }
        
        KAction *action;
        for (action = d->kipiFileActionsImport.first(); action; 
             action = d->kipiFileActionsImport.next())
        {
            action->setEnabled(false);
        }

        for (action = d->kipiFileActionsExport.first(); action; 
             action = d->kipiFileActionsExport.next())
        {
            action->setEnabled(true);
        }
    }
}

void DigikamApp::slotTagSelected(bool val)
{
    Album *album = d->albumManager->currentAlbum();
    
    if(!val)
    {
        d->deleteTagAction->setEnabled(false);
        d->editTagAction->setEnabled(false);
    }
    else if(!album->isRoot())
    {
        d->deleteTagAction->setEnabled(true);
        d->editTagAction->setEnabled(true);
        
        KAction *action;
        for (action = d->kipiFileActionsImport.first(); action;
             action = d->kipiFileActionsImport.next())
        {
            action->setEnabled(false);
        }

        for (action = d->kipiFileActionsExport.first(); action;
             action = d->kipiFileActionsExport.next())
        {
            action->setEnabled(true);
        }
    }
    else
    {
        d->deleteTagAction->setEnabled(false);
        d->editTagAction->setEnabled(false);
        
        KAction *action;
        for (action = d->kipiFileActionsImport.first(); action; 
             action = d->kipiFileActionsImport.next())
        {
            action->setEnabled(false);
        }

        for (action = d->kipiFileActionsExport.first(); action; 
             action = d->kipiFileActionsExport.next())
        {
            action->setEnabled(true);
        }
    }
}

void DigikamApp::slotImageSelected(const QPtrList<ImageInfo>& list, bool hasPrev, bool hasNext)
{
    QPtrList<ImageInfo> selection = list;
    bool val = selection.isEmpty() ? false : true;
    d->imageViewAction->setEnabled(val);
    d->imagePreviewAction->setEnabled(val);
    d->imageRenameAction->setEnabled(val);
    d->imageDeleteAction->setEnabled(val);
    d->imageExifOrientationActionMenu->setEnabled(val);
    d->slideShowSelectionAction->setEnabled(selection.count() != 0);

    switch (selection.count())
    {
        case 0:
            d->statusProgressBar->setText(i18n("No item selected"));
        break;
        case 1:
            d->statusProgressBar->setText(selection.first()->kurl().fileName());
        break;
        default:
            d->statusProgressBar->setText(i18n("%1 items selected").arg(selection.count()));
        break;
    }

    d->statusNavigateBar->setNavigateBarState(hasPrev, hasNext);
}

void DigikamApp::slotProgressBarMode(int mode, const QString& text)
{
    d->statusProgressBar->progressBarMode(mode, text);
}

void DigikamApp::slotProgressValue(int count)
{
    d->statusProgressBar->setProgressValue(count);
}

void DigikamApp::slotExit()
{
    close();
}

QString DigikamApp::convertToLocalUrl( const QString& folder )
{
    // This function is copied from k3b.

    KURL url( folder );
    if( !url.isLocalFile() )
    {
#if KDE_IS_VERSION(3,4,91)
        // Support for system:/ and media:/ (c) Stephan Kulow
        KURL mlu = KIO::NetAccess::mostLocalURL( url, 0 );
        if (mlu.isLocalFile())
            return mlu.path();

        DWarning() << folder << " mlu " << mlu << endl;

        QString path = mlu.path();

        if ( mlu.protocol() == "system" && path.startsWith("/media") )
            path = path.mid(7);
        else if (mlu.protocol() == "media")
            path = path.mid(1);
        else
            return folder; // nothing to see - go on

        DDebug() << "parsed import path is: " << path << endl;
        DCOPRef ref("kded", "mediamanager");
        DCOPReply reply = ref.call("properties", path);
        if (reply.isValid()) {
            QStringList slreply;
            reply.get(slreply);
            if ((slreply.count()>=9) && !slreply[9].isEmpty())
                return slreply[9];
            else
                return slreply[6];
        } else {
            DWarning() << "dcop call failed\n";
        }

        return path;
#else
#ifndef UDS_LOCAL_PATH
#define UDS_LOCAL_PATH (72 | KIO::UDS_STRING)
#else
        using namespace KIO;
#endif
        KIO::UDSEntry e;
        if( KIO::NetAccess::stat( url, e, 0 ) )
        {
            const KIO::UDSEntry::ConstIterator end = e.end();
            for( KIO::UDSEntry::ConstIterator it = e.begin(); it != end; ++it )
            {
                if( (*it).m_uds == UDS_LOCAL_PATH && !(*it).m_str.isEmpty() )
                    return KURL::fromPathOrURL( (*it).m_str ).path();
            }
        }
#endif
    }

    return url.path();
}

void DigikamApp::slotDcopDownloadImages( const QString& folder )
{
    if (!folder.isNull())
    {
        // activate window when called by media menu and DCOP
        if (isMinimized())
            KWin::deIconifyWindow(winId());
        KWin::activateWindow(winId());

        slotDownloadImages(folder);
    }
}

void DigikamApp::slotDcopCameraAutoDetect()
{
    // activate window when called by media menu and DCOP
    if (isMinimized())
        KWin::deIconifyWindow(winId());
    KWin::activateWindow(winId());

    slotCameraAutoDetect();
}

void DigikamApp::slotDownloadImages( const QString& folder)
{
    if (!folder.isNull())
    {
        d->cameraGuiPath = folder;

        QTimer::singleShot(0, this, SLOT(slotDownloadImages()));
    }
}

void DigikamApp::slotDownloadImages()
{
    if (d->cameraGuiPath.isNull())
        return;

    // Fetch the contents of the device. This is needed to make sure that the
    // media:/device gets mounted.
    KIO::ListJob *job = KIO::listDir(KURL(d->cameraGuiPath), false, false);
    KIO::NetAccess::synchronousRun(job,0);

    QString localUrl = convertToLocalUrl(d->cameraGuiPath);
    DDebug() << "slotDownloadImages: convertToLocalUrl " << d->cameraGuiPath << " to " << localUrl << endl;

    if (localUrl.isNull())
        return;

    bool alreadyThere = false;

    for (uint i = 0 ; i != actionCollection()->count() ; i++)
    {
        if (actionCollection()->action(i)->name() == d->cameraGuiPath)
            alreadyThere = true;
    }

    if (!alreadyThere)
    {
        KAction *cAction  = new KAction(
                 i18n("Browse %1").arg(KURL(d->cameraGuiPath).prettyURL()),
                 "kipi",
                 0,
                 this,
                 SLOT(slotDownloadImages()),
                 actionCollection(),
                 d->cameraGuiPath.latin1() );

        d->cameraMenuAction->insert(cAction, 0);
    }

    // the CameraUI will delete itself when it has finished
    CameraUI* cgui = new CameraUI(this,
                                  i18n("Images found in %1").arg(d->cameraGuiPath),
                                  "directory browse","Fixed", localUrl, QDateTime::currentDateTime());
    cgui->show();

    connect(cgui, SIGNAL(signalLastDestination(const KURL&)),
            d->view, SLOT(slotSelectAlbum(const KURL&)));

    connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
            this, SLOT(slotSetupChanged()));
}

void DigikamApp::slotCameraConnect()
{
    CameraType* ctype = d->cameraList->find(QString::fromUtf8(sender()->name()));

    if (ctype)
    {
        // check not to open two dialogs for the same camera
        if (ctype->currentCameraUI() && !ctype->currentCameraUI()->isClosed())
        {
            // show and raise dialog
            if (ctype->currentCameraUI()->isMinimized())
                KWin::deIconifyWindow(ctype->currentCameraUI()->winId());
            KWin::activateWindow(ctype->currentCameraUI()->winId());
        }
        else
        {
            // the CameraUI will delete itself when it has finished
            CameraUI* cgui = new CameraUI(this, ctype->title(), ctype->model(),
                                          ctype->port(), ctype->path(), ctype->lastAccess());

            ctype->setCurrentCameraUI(cgui);

            cgui->show();

            connect(cgui, SIGNAL(signalLastDestination(const KURL&)),
                    d->view, SLOT(slotSelectAlbum(const KURL&)));

            connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
                    this, SLOT(slotSetupChanged()));
        }
    }
}

void DigikamApp::slotCameraAdded(CameraType *ctype)
{
    if (!ctype) return;

    KAction *cAction = new KAction(ctype->title(), "camera", 0,
                                   this, SLOT(slotCameraConnect()),
                                   actionCollection(),
                                   ctype->title().utf8());
    d->cameraMenuAction->insert(cAction, 0);
    ctype->setAction(cAction);
}

void DigikamApp::slotCameraMediaMenu()
{
    d->mediaItems.clear();
    
    d->cameraMediaList->clear();
    d->cameraMediaList->insertItem(i18n("No Media Devices Found"), 0);
    d->cameraMediaList->setItemEnabled(0, false);
        
    KURL kurl("media:/");
    KIO::ListJob *job = KIO::listDir(kurl, false, false);
    
    connect( job, SIGNAL(entries(KIO::Job*,const KIO::UDSEntryList&)),
             this, SLOT(slotCameraMediaMenuEntries(KIO::Job*,const KIO::UDSEntryList&)) );
}

void DigikamApp::slotCameraMediaMenuEntries( Job *, const UDSEntryList & list )
{
    int i = 0;

    for(KIO::UDSEntryList::ConstIterator it = list.begin() ; it != list.end() ; ++it)
    {
        QString name;
        QString path;

        for ( UDSEntry::const_iterator et = (*it).begin() ; et != (*it).end() ; ++et ) 
        {
            if ( (*et).m_uds == KIO::UDS_NAME)
                name = ( *et ).m_str;
            if ( (*et).m_uds == KIO::UDS_URL)
                path = ( *et ).m_str;

            //DDebug() << ( *et ).m_str << endl;
        }

        if (!name.isEmpty() && !path.isEmpty())
        {
            //DDebug() << "slotCameraMediaMenuEntries: Adding " << name << ", path " << path << endl;
            if (i == 0)
                d->cameraMediaList->clear();

            d->mediaItems[i] = path;

            d->cameraMediaList->insertItem(name, this, SLOT(slotDownloadImagesFromMedia(int)), 0, i);
            d->cameraMediaList->setItemParameter(i, i);
            i++;
        }
    }
}

void DigikamApp::slotDownloadImagesFromMedia( int id )
{
    slotDownloadImages( d->mediaItems[id] );
}

void DigikamApp::slotCameraRemoved(CameraType *ctype)
{
    if (!ctype) return;

    KAction *cAction = ctype->action();

    if (cAction)
        d->cameraMenuAction->remove(cAction);
}

void DigikamApp::slotCameraAutoDetect()
{
    bool retry = false;

    CameraType* ctype = d->cameraList->autoDetect(retry);
    
    if (!ctype && retry)
    {
        QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
        return;
    }

    if (ctype && ctype->action())
    {
        ctype->action()->activate();
    }
}

void DigikamApp::slotSetup()
{
    setup();
}

bool DigikamApp::setup(bool iccSetupPage)
{
    Setup setup(this, 0, iccSetupPage ? Setup::IccProfiles : Setup::LastPageUsed);

    // To show the number of KIPI plugins in the setup dialog.

    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();
    setup.kipiPluginsPage()->initPlugins((int)list.count());

    if (setup.exec() != QDialog::Accepted)
        return false;

    setup.kipiPluginsPage()->applyPlugins();
    d->imagePluginsLoader->loadPluginsFromList(setup.imagePluginsPage()->getImagePluginsListEnable());

    slotSetupChanged();

    return true;
}

void DigikamApp::slotSetupCamera()
{
    Setup setup(this, 0, Setup::Camera);

    // For to show the number of KIPI plugins in the setup dialog.

    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();
    setup.kipiPluginsPage()->initPlugins((int)list.count());

    if (setup.exec() != QDialog::Accepted)
        return;

    setup.kipiPluginsPage()->applyPlugins();
    d->imagePluginsLoader->loadPluginsFromList(setup.imagePluginsPage()->getImagePluginsListEnable());

    slotSetupChanged();
}

void DigikamApp::slotSetupChanged()
{
    // raw loading options might have changed
    LoadingCacheInterface::cleanCache();

    if(d->albumSettings->getAlbumLibraryPath() != d->albumManager->getLibraryPath())
        d->view->clearHistory();

    d->albumManager->setLibraryPath(d->albumSettings->getAlbumLibraryPath());
    d->albumManager->startScan();

    d->view->applySettings(d->albumSettings);

    if (ImageWindow::imagewindowCreated())
        ImageWindow::imagewindow()->applySettings();

    d->config->sync();
}

void DigikamApp::slotEditKeys()
{
    KKeyDialog* dialog = new KKeyDialog();
    dialog->insert( actionCollection(), i18n( "General" ) );

    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();

    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin() ; it != list.end() ; ++it )
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if ( plugin )
           dialog->insert( plugin->actionCollection(), (*it)->comment() );
    }

    dialog->configure();
    delete dialog;
}

void DigikamApp::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config());
    KEditToolbar *dlg = new KEditToolbar(actionCollection(), "digikamui.rc");

    if(dlg->exec());
    {
        createGUI(QString::fromLatin1( "digikamui.rc" ), false);
        applyMainWindowSettings(KGlobal::config());
        plugActionList( QString::fromLatin1("file_actions_import"), d->kipiFileActionsImport );
        plugActionList( QString::fromLatin1("image_actions"), d->kipiImageActions );
        plugActionList( QString::fromLatin1("tool_actions"), d->kipiToolsActions );
        plugActionList( QString::fromLatin1("batch_actions"), d->kipiBatchActions );
        plugActionList( QString::fromLatin1("album_actions"), d->kipiAlbumActions );
        plugActionList( QString::fromLatin1("file_actions_export"), d->kipiFileActionsExport );
    }
    
    delete dlg;
}

void DigikamApp::slotToggleFullScreen()
{
    if (d->fullScreen)
    {
#if QT_VERSION >= 0x030300
        setWindowState( windowState() & ~WindowFullScreen );
#else
        showNormal();
#endif
        menuBar()->show();
        statusBar()->show();
        topDock()->show();
        bottomDock()->show();
        leftDock()->show();
        rightDock()->show();
        d->view->showSideBars();

        d->fullScreen = false;
    }
    else
    {
        KConfig* config = kapp->config();
        config->setGroup("ImageViewer Settings");
        bool fullScreenHideToolBar = config->readBoolEntry("FullScreen Hide ToolBar", false);

        menuBar()->hide();
        statusBar()->hide();
        if (fullScreenHideToolBar)
            topDock()->hide();
        bottomDock()->hide();
        leftDock()->hide();
        rightDock()->hide();
        d->view->hideSideBars();

        showFullScreen();

        d->fullScreen = true;
    }
}

void DigikamApp::slotShowTip()
{
#if KDE_IS_VERSION(3,2,0)
    QStringList tipsFiles;
    tipsFiles.append("digikam/tips");
    tipsFiles.append("digikamimageplugins/tips");
    tipsFiles.append("kipi/tips");

    KTipDialog::showMultiTip(this, tipsFiles, true);
#else
    KTipDialog::showTip(this, "digikam/tips", true);
#endif
}

void DigikamApp::slotShowKipiHelp()
{
    KApplication::kApplication()->invokeHelp( QString::null, "kipi-plugins" );
}

void DigikamApp::loadPlugins()
{
    QStringList ignores;
    d->kipiInterface = new DigikamKipiInterface( this, "Digikam_KIPI_interface" );

    ignores.append( "HelloWorld" );
    ignores.append( "KameraKlient" );

    d->kipiPluginLoader = new KIPI::PluginLoader( ignores, d->kipiInterface );

    connect( d->kipiPluginLoader, SIGNAL( replug() ),
             this, SLOT( slotKipiPluginPlug() ) );

    d->kipiPluginLoader->loadPlugins();

    d->kipiInterface->slotCurrentAlbumChanged(d->albumManager->currentAlbum());

    // Setting the initial menu options after all plugins have been loaded
    d->view->slotAlbumSelected(d->albumManager->currentAlbum());

    d->imagePluginsLoader = new ImagePluginLoader(this, d->splashScreen);
}

void DigikamApp::slotKipiPluginPlug()
{
    unplugActionList( QString::fromLatin1("file_actions_export") );
    unplugActionList( QString::fromLatin1("file_actions_import") );
    unplugActionList( QString::fromLatin1("image_actions") );
    unplugActionList( QString::fromLatin1("tool_actions") );
    unplugActionList( QString::fromLatin1("batch_actions") );
    unplugActionList( QString::fromLatin1("album_actions") );

    d->kipiImageActions.clear();
    d->kipiFileActionsExport.clear();
    d->kipiFileActionsImport.clear();
    d->kipiToolsActions.clear();
    d->kipiBatchActions.clear();
    d->kipiAlbumActions.clear();

    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();

    int cpt = 0;

    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin() ; it != list.end() ; ++it )
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if ( !plugin || !(*it)->shouldLoad() )
            continue;

        ++cpt;

        if(d->splashScreen)
            d->splashScreen->message(i18n("Loading: %1").arg((*it)->name()));

        plugin->setup( this );
        QPtrList<KAction>* popup = 0;

        // Plugin category identification using KAction method based.

        KActionPtrList actions = plugin->actions();

        for( KActionPtrList::Iterator it2 = actions.begin(); it2 != actions.end(); ++it2 )
        {
            if ( plugin->category(*it2) == KIPI::IMAGESPLUGIN )
               popup = &d->kipiImageActions;

            else if ( plugin->category(*it2) == KIPI::EXPORTPLUGIN )
               popup = &d->kipiFileActionsExport;

            else if ( plugin->category(*it2) == KIPI::IMPORTPLUGIN )
               popup = &d->kipiFileActionsImport;

            else if ( plugin->category(*it2) == KIPI::TOOLSPLUGIN )
               popup = &d->kipiToolsActions;

            else if ( plugin->category(*it2) == KIPI::BATCHPLUGIN )
               popup = &d->kipiBatchActions;

            else if ( plugin->category(*it2) == KIPI::COLLECTIONSPLUGIN )
               popup = &d->kipiAlbumActions;

            // Plug the KIPI plugins actions in according with the KAction method.

            if ( popup )
               popup->append( *it2 );
            else
               DDebug() << "No menu found for a plugin!!!" << endl;
        }

        plugin->actionCollection()->readShortcutSettings();
    }

    if(d->splashScreen)
        d->splashScreen->message(i18n("1 Kipi Plugin Loaded", "%n Kipi Plugins Loaded", cpt));

    // Create GUI menu in according with plugins.

    plugActionList( QString::fromLatin1("file_actions_export"), d->kipiFileActionsExport );
    plugActionList( QString::fromLatin1("file_actions_import"), d->kipiFileActionsImport );
    plugActionList( QString::fromLatin1("image_actions"), d->kipiImageActions );
    plugActionList( QString::fromLatin1("tool_actions"), d->kipiToolsActions );
    plugActionList( QString::fromLatin1("batch_actions"), d->kipiBatchActions );
    plugActionList( QString::fromLatin1("album_actions"), d->kipiAlbumActions );
}

void DigikamApp::loadCameras()
{
    d->cameraList->load();
    
    d->cameraMenuAction->popupMenu()->insertSeparator();
    
    d->cameraMenuAction->popupMenu()->insertItem(i18n("Media Browse"), d->cameraMediaList);
    
    d->cameraMenuAction->popupMenu()->insertSeparator();
    
    d->cameraMenuAction->insert(new KAction(i18n("Add Camera..."), 0,
                                          this, SLOT(slotSetupCamera()),
                                          actionCollection(),
                                          "camera_add"));
}

void DigikamApp::populateThemes()
{
    if(d->splashScreen)
        d->splashScreen->message(i18n("Loading themes"), AlignLeft, white);

    ThemeEngine::instance()->scanThemes();
    QStringList themes(ThemeEngine::instance()->themeNames());

    d->themeMenuAction->setItems(themes);
    int index = themes.findIndex(d->albumSettings->getCurrentTheme());
    
    if (index == -1)
        index = themes.findIndex(i18n("Default"));
        
    d->themeMenuAction->setCurrentItem(index);
    ThemeEngine::instance()->slotChangeTheme(d->themeMenuAction->currentText());
}

void DigikamApp::slotChangeTheme(const QString& theme)
{
    d->albumSettings->setCurrentTheme(theme);
    ThemeEngine::instance()->slotChangeTheme(theme);
}

void DigikamApp::slotDatabaseRescan()
{
    ScanLib sLib;
    sLib.startScan();
}

void DigikamApp::slotRebuildAllThumbs()
{
    QString msg = i18n("Rebuild all albums items thumbnails can take a while.\n"
                       "Do you want to continue?");
    int result = KMessageBox::warningContinueCancel(this, msg);
    if (result != KMessageBox::Continue)
        return;

    BatchThumbsGenerator *thumbsGenerator = new BatchThumbsGenerator(this);
    
    connect(thumbsGenerator, SIGNAL(signalRebuildAllThumbsDone()),
            this, SLOT(slotRebuildAllThumbsDone()));

    thumbsGenerator->exec();
}

void DigikamApp::slotRebuildAllThumbsDone()
{
    d->view->applySettings(d->albumSettings);
}

void DigikamApp::slotSyncAllPicturesMetadata()
{
    QString msg = i18n("Sync all pictures metadata from all albums with digiKam database "
                       "can take a while.\nDo you want to continue?");
    int result = KMessageBox::warningContinueCancel(this, msg);
    if (result != KMessageBox::Continue)
        return;

    BatchAlbumsSyncMetadata *syncMetadata = new BatchAlbumsSyncMetadata(this);
    
    connect(syncMetadata, SIGNAL(signalComplete()),
            this, SLOT(slotSyncAllPicturesMetadataDone()));

    syncMetadata->exec();
}

void DigikamApp::slotSyncAllPicturesMetadataDone()
{
    d->view->applySettings(d->albumSettings);
}

void DigikamApp::slotDonateMoney()
{
    KApplication::kApplication()->invokeBrowser("http://www.digikam.org/?q=donation");
}

void DigikamApp::toggledToPreviewMode(bool t)
{
    // NOTE: if 't' is true, we are in Preview Mode, else we are in AlbumView Mode

    // This is require if ESC is pressed to go out of Preview Mode. 
    // imagePreviewAction is handled by F3 key only. 
    d->imagePreviewAction->setChecked(t);

    // Here, we will toggle some menu actions depending of current Mode.
    
    // Select menu.
    d->selectAllAction->setEnabled(!t);
    d->selectNoneAction->setEnabled(!t);
    d->selectInvertAction->setEnabled(!t);

    // View menu     
    d->thumbSizePlusAction->setEnabled(!t);
    d->thumbSizeMinusAction->setEnabled(!t);
    d->albumSortAction->setEnabled(!t);
    d->imageSortAction->setEnabled(!t);
}

}  // namespace Digikam

