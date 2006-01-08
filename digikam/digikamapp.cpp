/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2002-16-10
 * Description : 
 * 
 * Copyright 2002-2005 by Renchi Raju and Gilles Caulier
 * Copyright (C) 2006 Tom Albers <tomalbers@kde.nl>
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

// KDE includes.

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kstdaccel.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <ktip.h>
#include <kpopupmenu.h>
#include <kdeversion.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>

using KIO::Job;
using KIO::UDSEntryList;
using KIO::UDSEntry;

// libKipi includes.

#include "kipiinterface.h"
#include <libkipi/pluginloader.h>
#include <libkipi/plugin.h>

// Local includes.

#include "albummanager.h"
#include "albumlister.h"
#include "album.h"
#include "themeengine.h"
#include "cameralist.h"
#include "cameratype.h"
#include "cameraui.h"
#include "albumsettings.h"
#include "setup.h"
#include "setupplugins.h"
#include "setupeditor.h"
#include "setupimgplugins.h"
#include "digikamview.h"
#include "imagepluginloader.h"
#include "imagewindow.h"
#include "digikamapp.h"
#include "splashscreen.h"
#include "thumbnailsize.h"
#include "scanlib.h"

namespace Digikam
{

DigikamApp::DigikamApp()
          : KMainWindow( 0, "Digikam" )
{
    m_instance = this;
    m_config = kapp->config();

    mFullScreen = false;
    mView = 0;

    mSplash = 0;
    if(m_config->readBoolEntry("Show Splash", true) &&
       !kapp->isRestored())
    {
        mSplash = new Digikam::SplashScreen("digikam-splash.png");
    }

    mAlbumSettings = new AlbumSettings();
    mAlbumSettings->readSettings();

    mAlbumManager = AlbumManager::instance();
    AlbumLister::instance();
    
    mCameraMediaList = new QPopupMenu;
    connect( mCameraMediaList, SIGNAL( aboutToShow() ),
             SLOT( slotCameraMediaMenu() ) );
    
    mCameraList = new CameraList(this, locateLocal("appdata", "cameras.xml"));

    connect(mCameraList, SIGNAL(signalCameraAdded(CameraType *)),
            this, SLOT(slotCameraAdded(CameraType *)));

    connect(mCameraList, SIGNAL(signalCameraRemoved(CameraType *)),
            this, SLOT(slotCameraRemoved(CameraType *)));

    setupView();
    setupActions();
    updateDeleteTrashMenu();

    applyMainWindowSettings(m_config);

    mAlbumManager->setLibraryPath(mAlbumSettings->getAlbumLibraryPath());
    mAlbumManager->startScan();

    // Load KIPI Plugins.
    loadPlugins();

    // Load Themes
    populateThemes();

    setAutoSaveSettings();
    
    mDcopIface = new DCOPIface(this, "camera");
    connect(mDcopIface, SIGNAL(signalCameraAutoDetect()), 
            this, SLOT(slotCameraAutoDetect()));
    connect(mDcopIface, SIGNAL(signalDownloadImages( const QString & )),
            this, SLOT(slotDownloadImages(const QString &)));
}

DigikamApp::~DigikamApp()
{
    if (Digikam::ImageWindow::imagewindow())
        delete Digikam::ImageWindow::imagewindow();

    if (mView)
        delete mView;

    mAlbumSettings->saveSettings();
    delete mAlbumSettings;

    delete mAlbumManager;
    delete AlbumLister::instance();

    m_instance = 0;
}

DigikamApp* DigikamApp::getinstance()
{
    return m_instance;
}

void DigikamApp::show()
{
    if(mSplash)
    {
        mSplash->finish(this);
        delete mSplash;
        mSplash = 0;
    }
    KMainWindow::show();
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
    // Auto-detect camera if requested so
    if(mSplash)
        mSplash->message(i18n("Auto-detect camera"), AlignLeft, white);
    QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
}

void DigikamApp::downloadFrom(const QString &cameraGuiPath)
{
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
    return true;
}

void DigikamApp::setupView()
{
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

void DigikamApp::setupActions()
{
    // -----------------------------------------------------------------

    mCameraMenuAction = new KActionMenu(i18n("&Camera"),
                                    "digitalcam",
                                    actionCollection(),
                                    "camera_menu");
    mCameraMenuAction->setDelayed(false);

    // -----------------------------------------------------------------

    mThemeMenuAction = new KSelectAction(i18n("&Themes"), 0,
                                         actionCollection(),
                                         "theme_menu");
    connect(mThemeMenuAction, SIGNAL(activated(const QString&)),
            SLOT(slotChangeTheme(const QString&)));

    // -----------------------------------------------------------------

    mBackwardActionMenu = new  KToolBarPopupAction(i18n("&Back"),
                                    "back",
                                    ALT+Key_Left,
                                    mView,
                                    SLOT(slotAlbumHistoryBack()),
                                    actionCollection(),
                                    "album_back");
    mBackwardActionMenu->setEnabled(false);
    connect(mBackwardActionMenu->popupMenu(),
            SIGNAL(aboutToShow()),
            this,
            SLOT(slotAboutToShowBackwardMenu()));
    connect(mBackwardActionMenu->popupMenu(),
            SIGNAL(activated(int)),
            mView,
            SLOT(slotAlbumHistoryBack(int)));

    mForwardActionMenu = new  KToolBarPopupAction(i18n("Forward"),
                                    "forward",
                                    ALT+Key_Right,
                                    mView,
                                    SLOT(slotAlbumHistoryForward()),
                                    actionCollection(),
                                    "album_forward");
    mForwardActionMenu->setEnabled(false);
    connect(mForwardActionMenu->popupMenu(),
            SIGNAL(aboutToShow()),
            this,
            SLOT(slotAboutToShowForwardMenu()));
    connect(mForwardActionMenu->popupMenu(),
            SIGNAL(activated(int)),
            mView,
            SLOT(slotAlbumHistoryForward(int)));

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

    mImageViewAction = new KAction(i18n("View/Edit..."),
                                    "editimage",
                                    Key_F4,
                                    mView,
                                    SLOT(slot_imageView()),
                                    actionCollection(),
                                    "image_view");
    mImageViewAction->setWhatsThis(i18n("This option allows you to open the Image Editor with the currently selected "
                                        "image."));

    mImageRenameAction = new KAction(i18n("Rename..."),
                                    "pencil",
                                    Key_F2,
                                    mView,
                                    SLOT(slot_imageRename()),
                                    actionCollection(),
                                    "image_rename");
    mImageRenameAction->setWhatsThis(i18n("This option allows you to rename the filename of the currently selected "
                                          "image."));

    mImageDeleteAction = new KAction(i18n("Delete"),
                                    "editdelete",
                                    SHIFT+Key_Delete,
                                    mView,
                                    SLOT(slot_imageDelete()),
                                    actionCollection(),
                                    "image_delete");

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

    // -----------------------------------------------------------

    KAction* findAction = KStdAction::find(mView, SLOT(slotNewQuickSearch()),
                                           actionCollection(), "search_quick");
    findAction->setText(i18n("Quick Search..."));

    findAction = KStdAction::find(mView, SLOT(slotNewAdvancedSearch()),
                                  actionCollection(), "search_advanced");
    findAction->setText(i18n("Advanced Search..."));
    findAction->setShortcut("Ctrl+Alt+F");

    new KAction(i18n("Scan for New Images"), "reload_page", 0,
                this, SLOT(slotDatabaseRescan()), actionCollection(), 
                "database_rescan");

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
        return KIO::NetAccess::mostLocalURL( url, 0 ).path();
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

    QString cameraGuiPath = convertToLocalUrl(mCameraGuiPath);
    kdDebug() << "IN: " << mCameraGuiPath << " OUT: " << cameraGuiPath << endl;

    bool alreadyThere = false;
    for (uint i = 0 ; i != actionCollection()->count() ; i++)
    {
        if (actionCollection()->action(i)->name() == mCameraGuiPath)
            alreadyThere = true;
    }

    if (!alreadyThere)
    {
        KAction *cAction  = new KAction(
                 i18n("Browse %1").arg(mCameraGuiPath),
                 "kipi",
                 0,
                 this,
                 SLOT(slotDownloadImages()),
                 actionCollection(),
                 mCameraGuiPath.latin1() );

        mCameraMenuAction->insert(cAction, 0);
     }
          
    Digikam::CameraUI* cgui = new Digikam::CameraUI(this, 
                              i18n("Images found in %1").arg(mCameraGuiPath),
                              "directory browse","Fixed", mCameraGuiPath);
    cgui->show();
    connect(cgui, SIGNAL(signalLastDestination(const KURL&)),
            mView, SLOT(slotSelectAlbum(const KURL&)));
    connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
            SLOT(slotSetupChanged()));
}

void DigikamApp::slotCameraConnect()
{
    CameraType* ctype = mCameraList->find(QString::fromUtf8(sender()->name()));

    if (ctype)
    {
        Digikam::CameraUI* cgui = new Digikam::CameraUI(this, ctype->title(), ctype->model(),
                                  ctype->port(), ctype->path());
        cgui->show();
        connect(cgui, SIGNAL(signalLastDestination(const KURL&)),
                mView, SLOT(slotSelectAlbum(const KURL&)));
        connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
                SLOT(slotSetupChanged()));
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
    mCameraMediaList->insertItem(i18n("No Media Devices Found"),1);
    mCameraMediaList->setItemEnabled(1,false);
        
    KURL kurl("media:/");
    KIO::ListJob *job = KIO::listDir(kurl, false, false);
    connect( job, SIGNAL(entries(KIO::Job*,const KIO::UDSEntryList&)),
             SLOT(slotCameraMediaMenuEntries(KIO::Job*,const KIO::UDSEntryList&)) );
}

void DigikamApp::slotCameraMediaMenuEntries( Job *, const UDSEntryList & list )
{
    int i=0;
    for(KIO::UDSEntryList::ConstIterator it = list.begin();
                  it!=list.end(); ++it)
    {
        QString name;
        QString path;
        for ( UDSEntry::const_iterator et = (*it).begin() ; et !=   (*it).end() ; ++ et ) {
            if ( (*et).m_uds == KIO::UDS_NAME)
                name = ( *et ).m_str;
            if ( (*et).m_uds == KIO::UDS_URL)
                path = ( *et ).m_str;
            kdDebug() << ( *et ).m_str << endl;
       }
       if (!name.isEmpty() && !path.isEmpty())
       {
            if (i==0)
                mCameraMediaList->clear();
            
            mMediaItems[i] = path;
            
            mCameraMediaList->insertItem( name,  this, 
                               SLOT(slotDownloadImagesFromMedia( int )),i,0);
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
    Digikam::Setup setup(this);

    // To show the number of KIPI plugins in the setup dialog.

    KIPI::PluginLoader::PluginList list = KipiPluginLoader_->pluginList();
    setup.pluginsPage_->initPlugins((int)list.count());

    if (setup.exec() != QDialog::Accepted)
        return;

    setup.pluginsPage_->applyPlugins();
    m_ImagePluginsLoader->loadPluginsFromList(setup.imgPluginsPage_->getImagePluginsListEnable());

    slotSetupChanged();
}

void DigikamApp::slotSetupCamera()
{
    Digikam::Setup setup(this, 0, Digikam::Setup::Camera);

    // For to show the number of KIPI plugins in the setup dialog.

    KIPI::PluginLoader::PluginList list = KipiPluginLoader_->pluginList();
    setup.pluginsPage_->initPlugins((int)list.count());

    if (setup.exec() != QDialog::Accepted)
        return;

    setup.pluginsPage_->applyPlugins();
    m_ImagePluginsLoader->loadPluginsFromList(setup.imgPluginsPage_->getImagePluginsListEnable());

    slotSetupChanged();
}

void DigikamApp::slotSetupChanged()
{
    if(mAlbumSettings->getAlbumLibraryPath() != mAlbumManager->getLibraryPath())
        mView->clearHistory();
    mAlbumManager->setLibraryPath(mAlbumSettings->getAlbumLibraryPath());
    mAlbumManager->startScan();

    mView->applySettings(mAlbumSettings);
    updateDeleteTrashMenu();
    if (Digikam::ImageWindow::imagewindow())
        Digikam::ImageWindow::imagewindow()->applySettings();

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

    m_ImagePluginsLoader = new Digikam::ImagePluginLoader(this, mSplash);
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
               kdDebug() << "No menu found for a plugin!!!" << endl;
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

void DigikamApp::updateDeleteTrashMenu()
{
    if (mAlbumSettings->getUseTrash())
    {
        mDeleteAction->setText(i18n("Move Album to Trash"));
        mDeleteAction->setIcon("edittrash");
        mImageDeleteAction->setText(i18n("Move to Trash"));
        mImageDeleteAction->setIcon("edittrash");
    }
    else
    {
        mDeleteAction->setText(i18n("Delete Album"));
        mDeleteAction->setIcon("editdelete");
        mImageDeleteAction->setText(i18n("Delete"));
        mImageDeleteAction->setIcon("editdelete");
    }
}

void DigikamApp::slotDatabaseRescan()
{
    ScanLib sLib;
    sLib.startScan();
}

DigikamApp* DigikamApp::m_instance = 0;

}  // namespace Digikam

#include "digikamapp.moc"
