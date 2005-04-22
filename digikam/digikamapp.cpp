////////////////////////////////////////////////////////////////////////////////
//
//    DIGIKAMAPP.CPP
//
//    Copyright (C) 2002-2005 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles Caulier <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

// QT includes.

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

// Includes files for plugins support.

#include "kipiinterface.h"
#include <libkipi/pluginloader.h>
#include <libkipi/plugin.h>

// Local includes.

#include "albummanager.h"
#include "album.h"
#include "themeengine.h"
#include "cameralist.h"
#include "cameratype.h"
#include "cameraui.h"
#include "albumsettings.h"
#include "setup.h"
#include "setupplugins.h"
#include "setupeditor.h"
#include "digikamview.h"
#include "imagepluginloader.h"
#include "imagewindow.h"
#include "digikamapp.h"
#include "splashscreen.h"
#include "thumbnailsize.h"
#include "digikamapp.h"

DigikamApp::DigikamApp(bool detectCamera)
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
        mSplash = new SplashScreen("digikam-splash.png");
    }

    mAlbumSettings = new AlbumSettings();
    mAlbumSettings->readSettings();

    mAlbumManager = new AlbumManager();

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

    // Load Cameras
    if(mSplash)
        mSplash->message(i18n("Loading cameras"), AlignLeft, white);
    loadCameras();

    // Load KIPI Plugins.
    loadPlugins();

    // Load Themes
    populateThemes();

    setAutoSaveSettings();

    // Auto-detect camera if requested so
    if (detectCamera)
    {
        if(mSplash)
            mSplash->message(i18n("Auto-detect camera"), AlignLeft, white);
        QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
    }
}

DigikamApp::~DigikamApp()
{
    if (ImageWindow::imagewindow())
        delete ImageWindow::imagewindow();
    
    if (mView)
        delete mView;

    mAlbumSettings->saveSettings();
    delete mAlbumSettings;

    delete mAlbumManager;

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

    mImageCommentsAction = new KAction(i18n("Edit Image Comments && Tags..."),
                                   "imagecomment",
                                    Key_F3,
                                    mView,
                                    SLOT(slot_imageCommentsEdit()),
                                    actionCollection(),
                                    "image_comments");
    mImageCommentsAction->setWhatsThis(i18n("This option allows you to edit the comments and tags of the currently "
                                            "selected image."));

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

    mImagePropsAction = new KAction(i18n("Properties"),
                                    "exifinfo",
                                    ALT+Key_Return,
                                    mView,
                                    SLOT(slotImageProperties()),
                                    actionCollection(),
                                    "image_properties");
    mImagePropsAction->setWhatsThis(i18n("This option allows you to display the file properties, the meta-data "
                                         "and the histogram of the currently selected image."));

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
    mImageSortAction->setItems(sortImagesActionList);

    // -----------------------------------------------------------------

    QSignalMapper *exifOrientationMapper = new QSignalMapper( mView );
    connect( exifOrientationMapper, SIGNAL( mapped( int ) ),
              mView, SLOT( slot_imageExifOrientation( int ) ) );

    mImageExifOrientationActionMenu = new KActionMenu(i18n("Correct Exif Orientation Tag"),
                                                      actionCollection(),
                                                      "image_set_exif_orientation");

    mImageSetExifOrientation1Action = new KAction(i18n("Normal"),0,actionCollection());
    mImageSetExifOrientation2Action = new KAction(i18n("Flipped Horizontally"),0,actionCollection());
    mImageSetExifOrientation3Action = new KAction(i18n("Rotated 180 Degrees"),0,actionCollection());
    mImageSetExifOrientation4Action = new KAction(i18n("Flipped Vertically"),0,actionCollection());
    mImageSetExifOrientation5Action = new KAction(i18n("Rotated 90 Degrees / Horiz. Flipped"),
                                                       0, actionCollection());
    mImageSetExifOrientation6Action = new KAction(i18n("Rotated 90 Degrees"),0,actionCollection());
    mImageSetExifOrientation7Action = new KAction(i18n("Rotated 90 Degrees / Vert. Flipped"),
                                                       0, actionCollection());
    mImageSetExifOrientation8Action = new KAction(i18n("Rotated 270 Degrees"),0,actionCollection());

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

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    createGUI(QString::fromLatin1( "digikamui.rc" ), false);

    // Initialize Actions ---------------------------------------

    mDeleteAction->setEnabled(false);
    mAddImagesAction->setEnabled(false);
    mPropsEditAction->setEnabled(false);
    mOpenInKonquiAction->setEnabled(false);

    mImageViewAction->setEnabled(false);
    mImageCommentsAction->setEnabled(false);
    mImageRenameAction->setEnabled(false);
    mImageDeleteAction->setEnabled(false);
    mImagePropsAction->setEnabled(false);
    mImagePropsAction->setEnabled(false);
    mImageExifOrientationActionMenu->setEnabled(false);

    mAlbumSortAction->setCurrentItem((int)mAlbumSettings->getAlbumSortOrder());
    mImageSortAction->setCurrentItem((int)mAlbumSettings->getImageSortOrder());
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
    if(album && !album->isRoot())
    {
        mDeleteAction->setEnabled(val);
        mAddImagesAction->setEnabled(val);
        mPropsEditAction->setEnabled(val);
        mOpenInKonquiAction->setEnabled(val);
    }
    else
    {
        mDeleteAction->setEnabled(false);
        mAddImagesAction->setEnabled(false);
        mPropsEditAction->setEnabled(false);
        mOpenInKonquiAction->setEnabled(false);
    }
}

void DigikamApp::slot_tagSelected(bool val)
{
    Album *album = mAlbumManager->currentAlbum();
    if(album && !album->isRoot())
    {
        mDeleteTagAction->setEnabled(val);
        mEditTagAction->setEnabled(val);
    }
    else
    {
        mDeleteTagAction->setEnabled(false);
        mEditTagAction->setEnabled(false);
    }
}

void DigikamApp::slot_imageSelected(bool val)
{
    mImageViewAction->setEnabled(val);
    mImageCommentsAction->setEnabled(val);
    mImageRenameAction->setEnabled(val);
    mImageDeleteAction->setEnabled(val);
    mImagePropsAction->setEnabled(val);
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

void DigikamApp::slotCameraConnect()
{
    CameraType* ctype = mCameraList->find(QString::fromUtf8(sender()->name()));

    if (ctype)
    {
        CameraUI* cgui = new CameraUI(this, ctype->title(), ctype->model(),
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

    KAction *cAction = new KAction(ctype->title(), 0,
                                   this, SLOT(slotCameraConnect()),
                                   actionCollection(),
                                   ctype->title().utf8());
    mCameraMenuAction->insert(cAction, 0);
    ctype->setAction(cAction);
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
    Setup setup(this);

    // For to show the number of KIPI plugins in the setup dialog.

    KIPI::PluginLoader::PluginList list = KipiPluginLoader_->pluginList();
    setup.pluginsPage_->initPlugins((int)list.count());

    if (setup.exec() != QDialog::Accepted)
        return;

    setup.pluginsPage_->applyPlugins();
    m_ImagePluginsLoader->loadPluginsFromList(setup.editorPage_->getImagePluginsListEnable());

    slotSetupChanged();
}

void DigikamApp::slotSetupCamera()
{
    Setup setup(this, 0, Setup::Camera);

    // For to show the number of KIPI plugins in the setup dialog.

    KIPI::PluginLoader::PluginList list = KipiPluginLoader_->pluginList();
    setup.pluginsPage_->initPlugins((int)list.count());

    if (setup.exec() != QDialog::Accepted)
        return;

    setup.pluginsPage_->applyPlugins();
    m_ImagePluginsLoader->loadPluginsFromList(setup.editorPage_->getImagePluginsListEnable());

    slotSetupChanged();
}

void DigikamApp::slotSetupChanged()
{
    mAlbumManager->setLibraryPath(mAlbumSettings->getAlbumLibraryPath());
    mView->applySettings(mAlbumSettings);
    updateDeleteTrashMenu();
    if (ImageWindow::imagewindow())
        ImageWindow::imagewindow()->applySettings();

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

    if (dlg->exec())
    {
        createGUI("digikamui.rc");
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

DigikamApp* DigikamApp::m_instance = 0;

#include "digikamapp.moc"
