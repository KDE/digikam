/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Tom Albers <tomalbers@kde.nl>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2002-16-10
 * Description : main interface implementation
 * 
 * Copyright 2002-2005 by Renchi Raju and Gilles Caulier
 * Copyright      2006 by Tom Albers
 * Copyright      2006 by Gilles Caulier
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

#include <qcstring.h>
#include <qdatastream.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qkeysequence.h>
#include <qsignalmapper.h>
#include <qtimer.h>
#include <qdir.h>

// KDE includes.

#include <kaboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kaction.h>
#include <kaccel.h>
#include <kstdaction.h>
#include <kstdaccel.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kiconloader.h>
#include <ktip.h>
#include <kpopupmenu.h>
#include <kdeversion.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kwin.h>
#include <dcopref.h>

// libKipi includes.

#include <libkipi/pluginloader.h>
#include <libkipi/plugin.h>

// Local includes.

#include "ddebug.h"
#include "albummanager.h"
#include "album.h"
#include "albumlister.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"
#include "cameralist.h"
#include "cameratype.h"
#include "cameraui.h"
#include "setup.h"
#include "setupplugins.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "setupimgplugins.h"
#include "imagepluginloader.h"
#include "imagewindow.h"
#include "splashscreen.h"
#include "thumbnailsize.h"
#include "themeengine.h"
#include "kipiinterface.h"
#include "scanlib.h"
#include "loadingcacheinterface.h"
#include "imageattributeswatch.h"
#include "dcrawbinary.h"
#include "batchthumbsgenerator.h"
#include "digikamview.h"
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
    m_instance     = this;
    m_config       = kapp->config();
    m_accelerators = 0;
    mFullScreen    = false;
    mView          = 0;
    mValidIccPath  = true;
    
    mSplash = 0;
    if(m_config->readBoolEntry("Show Splash", true) &&
       !kapp->isRestored())
    {
        mSplash = new SplashScreen("digikam-splash.png");
    }

    mAlbumSettings = new AlbumSettings();
    mAlbumSettings->readSettings();

    mAlbumManager = AlbumManager::instance();
    AlbumLister::instance();

    mCameraMediaList = new KPopupMenu;

    connect(mCameraMediaList, SIGNAL( aboutToShow() ),
            this, SLOT(slotCameraMediaMenu()));

    mCameraList = new CameraList(this, locateLocal("appdata", "cameras.xml"));

    connect(mCameraList, SIGNAL(signalCameraAdded(CameraType *)),
            this, SLOT(slotCameraAdded(CameraType *)));

    connect(mCameraList, SIGNAL(signalCameraRemoved(CameraType *)),
            this, SLOT(slotCameraRemoved(CameraType *)));

    setupView();
    setupAccelerators();
    setupActions();

    applyMainWindowSettings(m_config);

    // Check ICC profiles repository availability

    if(mSplash)
        mSplash->message(i18n("Checking ICC repository"), AlignLeft, white);

    mValidIccPath = SetupICC::iccRepositoryIsValid();

    // Check witch dcraw version available

    if(mSplash)
        mSplash->message(i18n("Checking dcraw version"), AlignLeft, white);

    DcrawBinary::instance()->checkSystem();

    // Actual file scanning is done in main() - is this necessary here?
    mAlbumManager->setLibraryPath(mAlbumSettings->getAlbumLibraryPath());

    // Read albums from database
    if(mSplash)
        mSplash->message(i18n("Reading database"), AlignLeft, white);

    mAlbumManager->startScan();

    // Load KIPI Plugins.
    loadPlugins();

    // Load Themes
    populateThemes();

    setAutoSaveSettings();

    mDcopIface = new DCOPIface(this, "camera");

    connect(mDcopIface, SIGNAL(signalCameraAutoDetect()), 
            this, SLOT(slotDcopCameraAutoDetect()));

    connect(mDcopIface, SIGNAL(signalDownloadImages( const QString & )),
            this, SLOT(slotDcopDownloadImages(const QString &)));
}

DigikamApp::~DigikamApp()
{
    ImageAttributesWatch::shutDown();

    if (ImageWindow::imagewindowCreated())
        // close and delete
        ImageWindow::imagewindow()->close(true);

    if (mView)
        delete mView;

    mAlbumSettings->saveSettings();
    delete mAlbumSettings;

    delete mAlbumManager;
    delete AlbumLister::instance();

    ImageAttributesWatch::cleanUp();
    LoadingCacheInterface::cleanUp();
    DcrawBinary::cleanUp();
    AlbumThumbnailLoader::cleanUp();

    m_instance = 0;
}

DigikamApp* DigikamApp::getinstance()
{
    return m_instance;
}

void DigikamApp::show()
{
    // Remove Splashscreen.

    if(mSplash)
    {
        mSplash->finish(this);
        delete mSplash;
        mSplash = 0;
    }

    // Display application window.

    KMainWindow::show();

    // Report errors from ICC repository path.

    if(!mValidIccPath)
    {
        QString message = i18n("<qt><p>ICC profiles path seems to be invalid.</p>"
                               "<p>If you want to set it now, select \"Yes\", otherwise "
                               "select \"No\". In this case, \"Color Management\" feature "
                               "will be disabled until you solve this issue</p></qt>");

        if (KMessageBox::warningYesNo(this, message) == KMessageBox::Yes)
        {
            if (!setup(true))
            {
                m_config->setGroup("Color Management");
                m_config->writeEntry("EnableCM", false);
                m_config->sync();
            }
        }
        else
        {
            m_config->setGroup("Color Management");
            m_config->writeEntry("EnableCM", false);
            m_config->sync();
        }
    }

    // Report errors from dcraw detection.

    DcrawBinary::instance()->checkReport();  
}

const QPtrList<KAction>& DigikamApp::menuImageActions()
{
    return m_kipiImageActions;
}

const QPtrList<KAction>& DigikamApp::menuBatchActions()
{
    return m_kipiBatchActions;
}

const QPtrList<KAction>& DigikamApp::menuAlbumActions()
{
    return m_kipiAlbumActions;
}

const QPtrList<KAction> DigikamApp::menuImportActions()
{
    QPtrList<KAction> importMenu;
    importMenu = m_kipiFileActionsImport;
    importMenu.append( mAlbumImportAction );
    importMenu.append( mAddImagesAction );
    return importMenu;
}

void DigikamApp::autoDetect()
{
    // Called from main if command line option is set

    if(mSplash)
        mSplash->message(i18n("Auto-detect camera"), AlignLeft, white);

    QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
}

void DigikamApp::downloadFrom(const QString &cameraGuiPath)
{
    // Called from main if command line option is set

    if (!cameraGuiPath.isNull())
    {
        mCameraGuiPath = cameraGuiPath;

        if(mSplash)
            mSplash->message(i18n("Opening Download Dialog"), AlignLeft, white);

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
    if(mSplash)
        mSplash->message(i18n("Initializing Main View"), AlignLeft, white);

    mView = new DigikamView(this);
    setCentralWidget(mView);
    mView->applySettings(mAlbumSettings);

    connect(mView, SIGNAL(signal_albumSelected(bool)),
            this, SLOT(slot_albumSelected(bool)));
            
    connect(mView, SIGNAL(signal_tagSelected(bool)),
            this, SLOT(slot_tagSelected(bool)));

    connect(mView, SIGNAL(signal_imageSelected(bool)),
            this, SLOT(slot_imageSelected(bool)));
}

void DigikamApp::setupAccelerators()
{
    m_accelerators = new KAccel(this);

    m_accelerators->insert("Exit Preview Mode", i18n("Exit Preview"),
                           i18n("Exit out of the preview mode"),
                           Key_Escape, this, SIGNAL(signalEscapePressed()),
                           false, true);
    
    m_accelerators->insert("Next Image Key_Space", i18n("Next Image"),
                           i18n("Next Image"),
                           Key_Space, this, SIGNAL(signalNextItem()),
                           false, true);

    m_accelerators->insert("Previous Image Key_Backspace", i18n("Previous Image"),
                           i18n("Previous Image"),
                           Key_Backspace, this, SIGNAL(signalPrevItem()),
                           false, true);

    m_accelerators->insert("Next Image Key_Next", i18n("Next Image"),
                           i18n("Next Image"),
                           Key_Next, this, SIGNAL(signalNextItem()),
                           false, true);

    m_accelerators->insert("Previous Image Key_Prior", i18n("Previous Image"),
                           i18n("Previous Image"),
                           Key_Prior, this, SIGNAL(signalPrevItem()),
                           false, true);

    m_accelerators->insert("First Image Key_Home", i18n("First Image"),
                           i18n("First Image"),
                           Key_Home, this, SIGNAL(signalFirstItem()),
                           false, true);

    m_accelerators->insert("Last Image Key_End", i18n("Last Image"),
                           i18n("Last Image"),
                           Key_End, this, SIGNAL(signalLastItem()),
                           false, true);

    m_accelerators->insert("Copy Album Items Selection CTRL+Key_C", i18n("Copy Album Items Selection"),
                           i18n("Copy Album Items Selection"),
                           CTRL+Key_C, this, SIGNAL(signalCopyAlbumItemsSelection()),
                           false, true);

    m_accelerators->insert("Paste Album Items Selection CTRL+Key_V", i18n("Paste Album Items Selection"),
                           i18n("Paste Album Items Selection"),
                           CTRL+Key_V, this, SIGNAL(signalPasteAlbumItemsSelection()),
                           false, true);
}

void DigikamApp::setupActions()
{
    // -----------------------------------------------------------------

    mCameraMenuAction = new KActionMenu(i18n("&Camera"),
                                    "digitalcam",
                                    actionCollection(),
                                    "camera_menu");
    mCameraMenuAction->setDelayed(false);

    // -----------------------------------------------------------------

    mThemeMenuAction = new KSelectAction(i18n("&Themes"), 0, actionCollection(), "theme_menu");
    connect(mThemeMenuAction, SIGNAL(activated(const QString&)),
            this, SLOT(slotChangeTheme(const QString&)));

    // -----------------------------------------------------------------

    mBackwardActionMenu = new KToolBarPopupAction(i18n("&Back"),
                                    "back",
                                    ALT+Key_Left,
                                    mView,
                                    SLOT(slotAlbumHistoryBack()),
                                    actionCollection(),
                                    "album_back");
    mBackwardActionMenu->setEnabled(false);

    connect(mBackwardActionMenu->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowBackwardMenu()));
    
    connect(mBackwardActionMenu->popupMenu(), SIGNAL(activated(int)),
            mView, SLOT(slotAlbumHistoryBack(int)));

    mForwardActionMenu = new  KToolBarPopupAction(i18n("Forward"),
                                    "forward",
                                    ALT+Key_Right,
                                    mView,
                                    SLOT(slotAlbumHistoryForward()),
                                    actionCollection(),
                                    "album_forward");
    mForwardActionMenu->setEnabled(false);
    
    connect(mForwardActionMenu->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowForwardMenu()));

    connect(mForwardActionMenu->popupMenu(), SIGNAL(activated(int)),
            mView, SLOT(slotAlbumHistoryForward(int)));

    mNewAction = new KAction(i18n("&New Album..."),
                                    "albumfoldernew",
                                    KStdAccel::shortcut(KStdAccel::New),
                                    mView,
                                    SLOT(slot_newAlbum()),
                                    actionCollection(),
                                    "album_new");
    mNewAction->setWhatsThis(i18n("This option creates a new empty Album in the database."));

    mAlbumSortAction = new KSelectAction(i18n("&Sort Albums"),
                                    0,
                                    0,
                                    actionCollection(),
                                    "album_sort");

    connect(mAlbumSortAction, SIGNAL(activated(int)),
            mView, SLOT(slot_sortAlbums(int)));

    // Use same list order as in albumsettings enum
    QStringList sortActionList;
    sortActionList.append(i18n("By Folder"));
    sortActionList.append(i18n("By Collection"));
    sortActionList.append(i18n("By Date"));
    mAlbumSortAction->setItems(sortActionList);

    mDeleteAction = new KAction(i18n("Delete Album"),
                                    "editdelete",
                                    0,
                                    mView,
                                    SLOT(slot_deleteAlbum()),
                                    actionCollection(),
                                    "album_delete");

    mAddImagesAction = new KAction( i18n("Add Images..."),
                                    "addimagefolder",
                                    CTRL+Key_I,
                                    mView,
                                    SLOT(slot_albumAddImages()),
                                    actionCollection(),
                                    "album_addImages");
    mAddImagesAction->setWhatsThis(i18n("This option adds new images to the current Album."));

    mAlbumImportAction = new KAction( i18n("Import Folders..."),
                                    "fileopen",
                                    0,
                                    mView,
                                    SLOT(slotAlbumImportFolder()),
                                    actionCollection(),
                                    "album_importFolder");


    mPropsEditAction = new KAction( i18n("Edit Album Properties..."),
                                    "albumfoldercomment",
                                    0,
                                    mView,
                                    SLOT(slot_albumPropsEdit()),
                                    actionCollection(),
                                    "album_propsEdit");
    mPropsEditAction->setWhatsThis(i18n("This option allows you to set the Album Properties information "
                                        "about the Collection."));

    mRefreshAlbumAction = new KAction( i18n("Refresh"),
                                    "rebuild",
                                    Key_F5,
                                    mView,
                                    SLOT(slot_albumRefresh()),
                                    actionCollection(),
                                    "album_refresh");
    mRefreshAlbumAction->setWhatsThis(i18n("This option refresh all album content."));

    mOpenInKonquiAction = new KAction( i18n("Open in Konqueror"),
                                    "konqueror",
                                    0,
                                    mView,
                                    SLOT(slot_albumOpenInKonqui()),
                                    actionCollection(),
                                    "album_openinkonqui");

    // -----------------------------------------------------------

    mNewTagAction = new KAction(i18n("New &Tag..."), "tag",
                                0, mView, SLOT(slotNewTag()),
                                actionCollection(), "tag_new");

    mDeleteTagAction = new KAction(i18n("Delete Tag"), "tag",
                                   0, mView, SLOT(slotDeleteTag()),
                                   actionCollection(), "tag_delete");

    mEditTagAction = new KAction( i18n("Edit Tag Properties..."), "tag",
                                  0, mView, SLOT(slotEditTag()),
                                  actionCollection(), "tag_edit");

    // -----------------------------------------------------------

    mImagePreviewAction = new KAction(i18n("View..."),
                                      "viewimage",
                                      Key_F3,
                                      mView,
                                      SLOT(slot_imagePreview()),
                                      actionCollection(),
                                      "image_view");

    mImageViewAction = new KAction(i18n("Edit..."),
                                   "editimage",
                                   Key_F4,
                                   mView,
                                   SLOT(slot_imageEdit()),
                                   actionCollection(),
                                   "image_edit");
    mImageViewAction->setWhatsThis(i18n("This option allows you to open the Image Editor with the "
                                        "currently selected image."));

    mImageRenameAction = new KAction(i18n("Rename..."),
                                    "pencil",
                                    Key_F2,
                                    mView,
                                    SLOT(slot_imageRename()),
                                    actionCollection(),
                                    "image_rename");
    mImageRenameAction->setWhatsThis(i18n("This option allows you to rename the filename of the currently selected "
                                          "image."));

    // Pop up dialog to ask user whether to move to trash
    mImageDeleteAction            = new KAction(i18n("Delete"),
                                                "edittrash",
                                                Key_Delete,
                                                mView,
                                                SLOT(slot_imageDelete()),
                                                actionCollection(),
                                                "image_delete");

    // Pop up dialog to ask user whether to permanently delete
    mImageDeletePermanentlyAction = new KAction(i18n("Delete Permanently"),
                                                "editdelete",
                                                SHIFT+Key_Delete,
                                                mView,
                                                SLOT(slot_imageDeletePermanently()),
                                                actionCollection(),
                                                "image_delete_permanently");

    // These two actions are hidden, no menu entry, no toolbar entry, no shortcut.
    // Power users may add them.
    mImageDeletePermanentlyDirectlyAction = new KAction(i18n("Delete Permanently without Confirmation"),
                                                        "editdelete",
                                                        0,
                                                        mView,
                                                        SLOT(slot_imageDeletePermanentlyDirectly()),
                                                        actionCollection(),
                                                        "image_delete_permanently_directly");

    mImageTrashDirectlyAction             = new KAction(i18n("Move to Trash without Confirmation"),
                                                        "edittrash",
                                                        0,
                                                        mView,
                                                        SLOT(slot_imageTrashDirectly()),
                                                        actionCollection(),
                                                        "image_trash_directly");

    mImageSortAction = new KSelectAction(i18n("&Sort Images"),
                                    0,
                                    0,
                                    actionCollection(),
                                    "image_sort");

    connect(mImageSortAction, SIGNAL(activated(int)),
            mView, SLOT(slotSortImages(int)));

    // Use same list order as in albumsettings enum
    QStringList sortImagesActionList;
    sortImagesActionList.append(i18n("By Name"));
    sortImagesActionList.append(i18n("By Path"));
    sortImagesActionList.append(i18n("By Date"));
    sortImagesActionList.append(i18n("By File Size"));
    sortImagesActionList.append(i18n("By Rating"));
    mImageSortAction->setItems(sortImagesActionList);

    // -----------------------------------------------------------------

    QSignalMapper *exifOrientationMapper = new QSignalMapper( mView );
    
    connect( exifOrientationMapper, SIGNAL( mapped( int ) ),
             mView, SLOT( slot_imageExifOrientation( int ) ) );

    mImageExifOrientationActionMenu = new KActionMenu(i18n("Correct Exif Orientation Tag"),
                                                      actionCollection(),
                                                      "image_set_exif_orientation");
    mImageExifOrientationActionMenu->setDelayed(false);

    mImageSetExifOrientation1Action =
        new KAction(i18n("Normal"),0,mImageExifOrientationActionMenu);
    mImageSetExifOrientation2Action =
        new KAction(i18n("Flipped Horizontally"),0,mImageExifOrientationActionMenu);
    mImageSetExifOrientation3Action =
        new KAction(i18n("Rotated 180 Degrees"),0,mImageExifOrientationActionMenu);
    mImageSetExifOrientation4Action =
        new KAction(i18n("Flipped Vertically"),0,mImageExifOrientationActionMenu);
    mImageSetExifOrientation5Action =
        new KAction(i18n("Rotated 90 Degrees / Horiz. Flipped"),
                    0, mImageExifOrientationActionMenu);
    mImageSetExifOrientation6Action =
        new KAction(i18n("Rotated 90 Degrees"),0,mImageExifOrientationActionMenu);
    mImageSetExifOrientation7Action =
        new KAction(i18n("Rotated 90 Degrees / Vert. Flipped"),
                    0, mImageExifOrientationActionMenu);
    mImageSetExifOrientation8Action =
        new KAction(i18n("Rotated 270 Degrees"),0,mImageExifOrientationActionMenu);

    mImageExifOrientationActionMenu->insert(mImageSetExifOrientation1Action);
    mImageExifOrientationActionMenu->insert(mImageSetExifOrientation2Action);
    mImageExifOrientationActionMenu->insert(mImageSetExifOrientation3Action);
    mImageExifOrientationActionMenu->insert(mImageSetExifOrientation4Action);
    mImageExifOrientationActionMenu->insert(mImageSetExifOrientation5Action);
    mImageExifOrientationActionMenu->insert(mImageSetExifOrientation6Action);
    mImageExifOrientationActionMenu->insert(mImageSetExifOrientation7Action);
    mImageExifOrientationActionMenu->insert(mImageSetExifOrientation8Action);

    connect( mImageSetExifOrientation1Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( mImageSetExifOrientation2Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( mImageSetExifOrientation3Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( mImageSetExifOrientation4Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( mImageSetExifOrientation5Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( mImageSetExifOrientation6Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( mImageSetExifOrientation7Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );
    connect( mImageSetExifOrientation8Action, SIGNAL( activated() ), exifOrientationMapper, SLOT( map() ) );

    exifOrientationMapper->setMapping( mImageSetExifOrientation1Action, 1);
    exifOrientationMapper->setMapping( mImageSetExifOrientation2Action, 2);
    exifOrientationMapper->setMapping( mImageSetExifOrientation3Action, 3);
    exifOrientationMapper->setMapping( mImageSetExifOrientation4Action, 4);
    exifOrientationMapper->setMapping( mImageSetExifOrientation5Action, 5);
    exifOrientationMapper->setMapping( mImageSetExifOrientation6Action, 6);
    exifOrientationMapper->setMapping( mImageSetExifOrientation7Action, 7);
    exifOrientationMapper->setMapping( mImageSetExifOrientation8Action, 8);

    // -----------------------------------------------------------------

    mSelectAllAction = new KAction(i18n("Select All"),
                                    0,
                                    CTRL+Key_A,
                                    mView,
                                    SLOT(slotSelectAll()),
                                    actionCollection(),
                                    "selectAll");

    mSelectNoneAction = new KAction(i18n("Select None"),
                                    0,
                                    CTRL+Key_U,
                                    mView,
                                    SLOT(slotSelectNone()),
                                    actionCollection(),
                                    "selectNone");

    mSelectInvertAction = new KAction(i18n("Invert Selection"),
                                    0,
                                    CTRL+Key_Asterisk,
                                    mView,
                                    SLOT(slotSelectInvert()),
                                    actionCollection(),
                                    "selectInvert");

    // -----------------------------------------------------------

    KStdAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStdAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStdAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // -----------------------------------------------------------

    mThumbSizePlusAction = new KAction(i18n("Increase Thumbnail Size"),
                                   "viewmag+",
                                   CTRL+Key_Plus,
                                   mView,
                                   SLOT(slot_thumbSizePlus()),
                                   actionCollection(),
                                   "album_thumbSizeIncrease");
    mThumbSizePlusAction->setWhatsThis(i18n("This option allows you to increase the Album thumbnails size."));

    mThumbSizeMinusAction = new KAction(i18n("Decrease Thumbnail Size"),
                                   "viewmag-",
                                   CTRL+Key_Minus,
                                   mView,
                                   SLOT(slot_thumbSizeMinus()),
                                   actionCollection(),
                                   "album_thumbSizeDecrease");
    mThumbSizeMinusAction->setWhatsThis(i18n("This option allows you to decrease the Album thumbnails size."));

    mFullScreenAction = new KAction(i18n("Toggle Full Screen"),
                                   "window_fullscreen",
                                   CTRL+SHIFT+Key_F,
                                   this,
                                   SLOT(slotToggleFullScreen()),
                                   actionCollection(),
                                   "full_screen");
    mFullScreenAction->setWhatsThis(i18n("This option allows you to toggle the main windows in full screen mode."));

    mQuitAction = KStdAction::quit(this,
                                   SLOT(slot_exit()),
                                   actionCollection(),
                                   "app_exit");

    mKipiHelpAction = new KAction(i18n("Kipi Plugins Handbook"),
                                   "kipi",
                                   0,
                                   this,
                                   SLOT(slotShowKipiHelp()),
                                   actionCollection(),
                                   "help_kipi");

    mTipAction = KStdAction::tipOfDay(this,
                                   SLOT(slotShowTip()),
                                   actionCollection(),
                                   "help_tipofday");

    mGammaAdjustmentAction = new KAction(i18n("Gamma Adjustment..."),
                                   "kgamma",
                                   0,
                                   this,
                                   SLOT(slot_gammaAdjustment()),
                                   actionCollection(),
                                   "gamma_adjustment");

    // -- Rating actions ---------------------------------------------------------------

    m_0Star = new KAction(i18n("Assign Rating \"No Star\""), CTRL+Key_0,
                          mView, SLOT(slotAssignRatingNoStar()),
                          actionCollection(), "imageview_ratenostar");
    m_1Star = new KAction(i18n("Assign Rating \"One Star\""), CTRL+Key_1,
                          mView, SLOT(slotAssignRatingOneStar()),
                          actionCollection(), "imageview_rateonestar");
    m_2Star = new KAction(i18n("Assign Rating \"Two Stars\""), CTRL+Key_2, 
                          mView, SLOT(slotAssignRatingTwoStar()),
                          actionCollection(), "imageview_ratetwostar");
    m_3Star = new KAction(i18n("Assign Rating \"Three Stars\""), CTRL+Key_3, 
                          mView, SLOT(slotAssignRatingThreeStar()),
                          actionCollection(), "imageview_ratethreestar");
    m_4Star = new KAction(i18n("Assign Rating \"Four Stars\""), CTRL+Key_4, 
                          mView, SLOT(slotAssignRatingFourStar()),
                          actionCollection(), "imageview_ratefourstar");
    m_5Star = new KAction(i18n("Assign Rating \"Five Stars\""), CTRL+Key_5, 
                          mView, SLOT(slotAssignRatingFiveStar()),
                          actionCollection(), "imageview_ratefivestar");

    // -----------------------------------------------------------

    KAction* findAction = KStdAction::find(mView, SLOT(slotNewQuickSearch()),
                                           actionCollection(), "search_quick");
    findAction->setText(i18n("Quick Search..."));
    findAction->setIconSet(BarIcon("filefind"));

    KAction* advFindAction = KStdAction::find(mView, SLOT(slotNewAdvancedSearch()),
                                              actionCollection(), "search_advanced");
    advFindAction->setText(i18n("Advanced Search..."));
    advFindAction->setShortcut("Ctrl+Alt+F");

    new KAction(i18n("Scan for New Images"), "reload_page", 0,
                this, SLOT(slotDatabaseRescan()), actionCollection(), 
                "database_rescan");

    new KAction(i18n("Rebuild all Thumbnails..."), "reload_page", 0,
                this, SLOT(slotRebuildAllThumbs()), actionCollection(),
                "thumbs_rebuild");

    // -----------------------------------------------------------

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Load Cameras -- do this before the createGUI so that the cameras
    // are plugged into the toolbar at startup
    if (mSplash)
        mSplash->message(i18n("Loading cameras"), AlignLeft, white);
    
    loadCameras();

    createGUI(QString::fromLatin1( "digikamui.rc" ), false);

    // Initialize Actions ---------------------------------------

    mDeleteAction->setEnabled(false);
    mAddImagesAction->setEnabled(false);
    mPropsEditAction->setEnabled(false);
    mOpenInKonquiAction->setEnabled(false);

    mImageViewAction->setEnabled(false);
    mImagePreviewAction->setEnabled(false);
    mImageRenameAction->setEnabled(false);
    mImageDeleteAction->setEnabled(false);
    mImageExifOrientationActionMenu->setEnabled(false);

    mAlbumSortAction->setCurrentItem((int)mAlbumSettings->getAlbumSortOrder());
    mImageSortAction->setCurrentItem((int)mAlbumSettings->getImageSortOrder());

    int size = mAlbumSettings->getDefaultIconSize();
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
    mThumbSizePlusAction->setEnabled(val);
}

void DigikamApp::enableThumbSizeMinusAction(bool val)
{
    mThumbSizeMinusAction->setEnabled(val);
}

void DigikamApp::enableAlbumBackwardHistory(bool enable)
{
    mBackwardActionMenu->setEnabled(enable);
}

void DigikamApp::enableAlbumForwardHistory(bool enable)
{
    mForwardActionMenu->setEnabled(enable);
}

void DigikamApp::slotAboutToShowBackwardMenu()
{
    mBackwardActionMenu->popupMenu()->clear();
    QStringList titles;
    mView->getBackwardHistory(titles);
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();
        for(; iter != titles.end(); ++iter,++id)
        {
            mBackwardActionMenu->popupMenu()->insertItem(*iter, id);
        }
    }
}

void DigikamApp::slotAboutToShowForwardMenu()
{
    mForwardActionMenu->popupMenu()->clear();
    QStringList titles;
    mView->getForwardHistory(titles);
    
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();
        for(; iter != titles.end(); ++iter,++id)
        {
            mForwardActionMenu->popupMenu()->insertItem(*iter, id);
        }
    }
}

void DigikamApp::slot_albumSelected(bool val)
{
    Album *album = mAlbumManager->currentAlbum();
    
    if(album && !val)
    {
        // No PAlbum is selected
        mDeleteAction->setEnabled(false);
        mAddImagesAction->setEnabled(false);
        mPropsEditAction->setEnabled(false);
        mOpenInKonquiAction->setEnabled(false);
        mNewAction->setEnabled(false);
        mAlbumImportAction->setEnabled(false);

    }
    else if(!album && !val)
    {
        // Groupitem selected (Collection/date)
        mDeleteAction->setEnabled(false);
        mAddImagesAction->setEnabled(false);
        mPropsEditAction->setEnabled(false);
        mOpenInKonquiAction->setEnabled(false);
        mNewAction->setEnabled(false);
        mAlbumImportAction->setEnabled(false);
        
        KAction *action;
        for (action = m_kipiFileActionsImport.first(); action;
             action = m_kipiFileActionsImport.next())
        {
            action->setEnabled(false);
        }

        for (action = m_kipiFileActionsExport.first(); action;
             action = m_kipiFileActionsExport.next())
        {
            action->setEnabled(false);
        }
    }
    else if(album && !album->isRoot() && album->type() == Album::PHYSICAL)
    {
        // Normal Album selected
        mDeleteAction->setEnabled(true);
        mAddImagesAction->setEnabled(true);
        mPropsEditAction->setEnabled(true);
        mOpenInKonquiAction->setEnabled(true);
        mNewAction->setEnabled(true);
        mAlbumImportAction->setEnabled(true);
        
        KAction *action;
        for (action = m_kipiFileActionsImport.first(); action; 
             action = m_kipiFileActionsImport.next())
        {
            action->setEnabled(true);
        }

        for (action = m_kipiFileActionsExport.first(); action; 
             action = m_kipiFileActionsExport.next())
        {
            action->setEnabled(true);    
        }        
    }
    else if(album && album->isRoot() && album->type() == Album::PHYSICAL)
    {
        // Root Album selected
        mDeleteAction->setEnabled(false);
        mAddImagesAction->setEnabled(false);
        mPropsEditAction->setEnabled(false);
       

        if(album->type() == Album::PHYSICAL)
        {
            mNewAction->setEnabled(true);
            mOpenInKonquiAction->setEnabled(true);
            mAlbumImportAction->setEnabled(true);
        }
        else
        {
            mNewAction->setEnabled(false);
            mOpenInKonquiAction->setEnabled(false);
            mAlbumImportAction->setEnabled(false);            
        }
        
        KAction *action;
        for (action = m_kipiFileActionsImport.first(); action; 
             action = m_kipiFileActionsImport.next())
        {
            action->setEnabled(false);
        }

        for (action = m_kipiFileActionsExport.first(); action; 
             action = m_kipiFileActionsExport.next())
        {
            action->setEnabled(true);
        }
    }
}

void DigikamApp::slot_tagSelected(bool val)
{
    Album *album = mAlbumManager->currentAlbum();
    
    if(!val)
    {
        mDeleteTagAction->setEnabled(false);
        mEditTagAction->setEnabled(false);
    }
    else if(!album->isRoot())
    {
        mDeleteTagAction->setEnabled(true);
        mEditTagAction->setEnabled(true);
        
        KAction *action;
        for (action = m_kipiFileActionsImport.first(); action;
             action = m_kipiFileActionsImport.next())
        {
            action->setEnabled(false);
        }

        for (action = m_kipiFileActionsExport.first(); action;
             action = m_kipiFileActionsExport.next())
        {
            action->setEnabled(true);
        }
    }
    else
    {
        mDeleteTagAction->setEnabled(false);
        mEditTagAction->setEnabled(false);
        
        KAction *action;
        for (action = m_kipiFileActionsImport.first(); action; 
             action = m_kipiFileActionsImport.next())
        {
            action->setEnabled(false);
        }

        for (action = m_kipiFileActionsExport.first(); action; 
             action = m_kipiFileActionsExport.next())
        {
            action->setEnabled(true);
        }
    }
}

void DigikamApp::slot_imageSelected(bool val)
{
    mImageViewAction->setEnabled(val);
    mImagePreviewAction->setEnabled(val);
    mImageRenameAction->setEnabled(val);
    mImageDeleteAction->setEnabled(val);
    mImageExifOrientationActionMenu->setEnabled(val);
}

void DigikamApp::slot_gammaAdjustment()
{
   QStringList args;
   QString *perror = 0;
   int *ppid = 0;

   args << "kgamma";
   int ValRet = KApplication::kdeinitExec(QString::fromLatin1("kcmshell"), args, perror, ppid);

   if ( ValRet != 0 )
      KMessageBox::error(this, i18n("Cannot start \"KGamma\" extension from KDE control center;\n"
                                 "please check your installation."));
}

void DigikamApp::slot_exit()
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
        mCameraGuiPath = folder;

        QTimer::singleShot(0, this, SLOT(slotDownloadImages()));
    }
}

void DigikamApp::slotDownloadImages()
{
    if (mCameraGuiPath.isNull())
        return;

    // Fetch the contents of the device. This is needed to make sure that the
    // media:/device gets mounted.
    KIO::ListJob *job = KIO::listDir(KURL(mCameraGuiPath), false, false);
    KIO::NetAccess::synchronousRun(job,0);

    QString localUrl = convertToLocalUrl(mCameraGuiPath);
    DDebug() << "slotDownloadImages: convertToLocalUrl " << mCameraGuiPath << " to " << localUrl << endl;

    if (localUrl.isNull())
        return;

    bool alreadyThere = false;

    for (uint i = 0 ; i != actionCollection()->count() ; i++)
    {
        if (actionCollection()->action(i)->name() == mCameraGuiPath)
            alreadyThere = true;
    }

    if (!alreadyThere)
    {
        KAction *cAction  = new KAction(
                 i18n("Browse %1").arg(KURL(mCameraGuiPath).prettyURL()),
                 "kipi",
                 0,
                 this,
                 SLOT(slotDownloadImages()),
                 actionCollection(),
                 mCameraGuiPath.latin1() );

        mCameraMenuAction->insert(cAction, 0);
    }

    // the CameraUI will delete itself when it has finished
    CameraUI* cgui = new CameraUI(this,
                                  i18n("Images found in %1").arg(mCameraGuiPath),
                                  "directory browse","Fixed", localUrl, QDateTime::currentDateTime());
    cgui->show();

    connect(cgui, SIGNAL(signalLastDestination(const KURL&)),
            mView, SLOT(slotSelectAlbum(const KURL&)));

    connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
            this, SLOT(slotSetupChanged()));

}

void DigikamApp::slotCameraConnect()
{
    CameraType* ctype = mCameraList->find(QString::fromUtf8(sender()->name()));

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
                    mView, SLOT(slotSelectAlbum(const KURL&)));

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
    mCameraMenuAction->insert(cAction, 0);
    ctype->setAction(cAction);
}

void DigikamApp::slotCameraMediaMenu()
{
    mMediaItems.clear();
    
    mCameraMediaList->clear();
    mCameraMediaList->insertItem(i18n("No Media Devices Found"), 0);
    mCameraMediaList->setItemEnabled(0, false);
        
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
                mCameraMediaList->clear();

            mMediaItems[i] = path;

            mCameraMediaList->insertItem(name, this, SLOT(slotDownloadImagesFromMedia(int)), 0, i);
            mCameraMediaList->setItemParameter(i, i);
            i++;
        }
    }
}

void DigikamApp::slotDownloadImagesFromMedia( int id )
{
    slotDownloadImages( mMediaItems[id] );
}

void DigikamApp::slotCameraRemoved(CameraType *ctype)
{
    if (!ctype) return;

    KAction *cAction = ctype->action();

    if (cAction)
        mCameraMenuAction->remove(cAction);
}

void DigikamApp::slotCameraAutoDetect()
{
    bool retry = false;

    CameraType* ctype = mCameraList->autoDetect(retry);
    
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

    KIPI::PluginLoader::PluginList list = KipiPluginLoader_->pluginList();
    setup.kipiPluginsPage()->initPlugins((int)list.count());

    if (setup.exec() != QDialog::Accepted)
        return false;

    setup.kipiPluginsPage()->applyPlugins();
    m_ImagePluginsLoader->loadPluginsFromList(setup.imagePluginsPage()->getImagePluginsListEnable());

    slotSetupChanged();

    return true;
}

void DigikamApp::slotSetupCamera()
{
    Setup setup(this, 0, Setup::Camera);

    // For to show the number of KIPI plugins in the setup dialog.

    KIPI::PluginLoader::PluginList list = KipiPluginLoader_->pluginList();
    setup.kipiPluginsPage()->initPlugins((int)list.count());

    if (setup.exec() != QDialog::Accepted)
        return;

    setup.kipiPluginsPage()->applyPlugins();
    m_ImagePluginsLoader->loadPluginsFromList(setup.imagePluginsPage()->getImagePluginsListEnable());

    slotSetupChanged();
}

void DigikamApp::slotSetupChanged()
{
    // raw loading options might have changed
    LoadingCacheInterface::cleanCache();

    if(mAlbumSettings->getAlbumLibraryPath() != mAlbumManager->getLibraryPath())
        mView->clearHistory();

    mAlbumManager->setLibraryPath(mAlbumSettings->getAlbumLibraryPath());
    mAlbumManager->startScan();

    mView->applySettings(mAlbumSettings);

    if (ImageWindow::imagewindowCreated())
        ImageWindow::imagewindow()->applySettings();

    m_config->sync();
}

void DigikamApp::slotEditKeys()
{
    KKeyDialog* dialog = new KKeyDialog();
    dialog->insert( actionCollection(), i18n( "General" ) );

    KIPI::PluginLoader::PluginList list = KipiPluginLoader_->pluginList();

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
        plugActionList( QString::fromLatin1("file_actions_import"), m_kipiFileActionsImport );
        plugActionList( QString::fromLatin1("image_actions"), m_kipiImageActions );
        plugActionList( QString::fromLatin1("tool_actions"), m_kipiToolsActions );
        plugActionList( QString::fromLatin1("batch_actions"), m_kipiBatchActions );
        plugActionList( QString::fromLatin1("album_actions"), m_kipiAlbumActions );
        plugActionList( QString::fromLatin1("file_actions_export"), m_kipiFileActionsExport );
    }
    
    delete dlg;
}

void DigikamApp::slotToggleFullScreen()
{
    if (mFullScreen)
    {
#if QT_VERSION >= 0x030300
        setWindowState( windowState() & ~WindowFullScreen );
#else
        showNormal();
#endif
        mFullScreen = false;
    }
    else
    {
        showFullScreen();
        mFullScreen = true;
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
    KipiInterface_ = new DigikamKipiInterface( this, "Digikam_KIPI_interface" );

    ignores.append( "HelloWorld" );
    ignores.append( "KameraKlient" );

    KipiPluginLoader_ = new KIPI::PluginLoader( ignores, KipiInterface_ );

    connect( KipiPluginLoader_, SIGNAL( replug() ),
             this, SLOT( slotKipiPluginPlug() ) );

    KipiPluginLoader_->loadPlugins();

    KipiInterface_->slotCurrentAlbumChanged(mAlbumManager->currentAlbum());

    // Setting the initial menu options after all plugins have been loaded
    mView->slot_albumSelected(mAlbumManager->currentAlbum());

    m_ImagePluginsLoader = new ImagePluginLoader(this, mSplash);
}

void DigikamApp::slotKipiPluginPlug()
{
    unplugActionList( QString::fromLatin1("file_actions_export") );
    unplugActionList( QString::fromLatin1("file_actions_import") );
    unplugActionList( QString::fromLatin1("image_actions") );
    unplugActionList( QString::fromLatin1("tool_actions") );
    unplugActionList( QString::fromLatin1("batch_actions") );
    unplugActionList( QString::fromLatin1("album_actions") );

    m_kipiImageActions.clear();
    m_kipiFileActionsExport.clear();
    m_kipiFileActionsImport.clear();
    m_kipiToolsActions.clear();
    m_kipiBatchActions.clear();
    m_kipiAlbumActions.clear();

    KIPI::PluginLoader::PluginList list = KipiPluginLoader_->pluginList();

    int cpt = 0;

    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin() ; it != list.end() ; ++it )
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if ( !plugin || !(*it)->shouldLoad() )
            continue;

        ++cpt;

        if(mSplash)
            mSplash->message(i18n("Loading: %1").arg((*it)->name()));

        plugin->setup( this );
        QPtrList<KAction>* popup = 0;

        // Plugin category identification using KAction method based.

        KActionPtrList actions = plugin->actions();

        for( KActionPtrList::Iterator it2 = actions.begin(); it2 != actions.end(); ++it2 )
        {
            if ( plugin->category(*it2) == KIPI::IMAGESPLUGIN )
               popup = &m_kipiImageActions;

            else if ( plugin->category(*it2) == KIPI::EXPORTPLUGIN )
               popup = &m_kipiFileActionsExport;

            else if ( plugin->category(*it2) == KIPI::IMPORTPLUGIN )
               popup = &m_kipiFileActionsImport;

            else if ( plugin->category(*it2) == KIPI::TOOLSPLUGIN )
               popup = &m_kipiToolsActions;

            else if ( plugin->category(*it2) == KIPI::BATCHPLUGIN )
               popup = &m_kipiBatchActions;

            else if ( plugin->category(*it2) == KIPI::COLLECTIONSPLUGIN )
               popup = &m_kipiAlbumActions;

            // Plug the KIPI plugins actions in according with the KAction method.

            if ( popup )
               popup->append( *it2 );
            else
               DDebug() << "No menu found for a plugin!!!" << endl;
        }

        plugin->actionCollection()->readShortcutSettings();
    }

    if(mSplash)
        mSplash->message(i18n("1 Kipi Plugin Loaded", "%n Kipi Plugins Loaded", cpt));

    // Create GUI menu in according with plugins.

    plugActionList( QString::fromLatin1("file_actions_export"), m_kipiFileActionsExport );
    plugActionList( QString::fromLatin1("file_actions_import"), m_kipiFileActionsImport );
    plugActionList( QString::fromLatin1("image_actions"), m_kipiImageActions );
    plugActionList( QString::fromLatin1("tool_actions"), m_kipiToolsActions );
    plugActionList( QString::fromLatin1("batch_actions"), m_kipiBatchActions );
    plugActionList( QString::fromLatin1("album_actions"), m_kipiAlbumActions );
}

void DigikamApp::loadCameras()
{
    mCameraList->load();
    
    mCameraMenuAction->popupMenu()->insertSeparator();
    
    mCameraMenuAction->popupMenu()->insertItem(i18n("Media Browse"), mCameraMediaList);
    
    mCameraMenuAction->popupMenu()->insertSeparator();
    
    mCameraMenuAction->insert(new KAction(i18n("Add Camera..."), 0,
                                          this, SLOT(slotSetupCamera()),
                                          actionCollection(),
                                          "camera_add"));
}

void DigikamApp::populateThemes()
{
    if(mSplash)
        mSplash->message(i18n("Loading themes"), AlignLeft, white);

    ThemeEngine::instance()->scanThemes();
    QStringList themes(ThemeEngine::instance()->themeNames());

    mThemeMenuAction->setItems(themes);
    int index = themes.findIndex(mAlbumSettings->getCurrentTheme());
    
    if (index == -1)
        index = themes.findIndex(i18n("Default"));
        
    mThemeMenuAction->setCurrentItem(index);
    ThemeEngine::instance()->slotChangeTheme(mThemeMenuAction->currentText());
}

void DigikamApp::slotChangeTheme(const QString& theme)
{
    mAlbumSettings->setCurrentTheme(theme);
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
    mView->applySettings(mAlbumSettings);
}

}  // namespace Digikam

