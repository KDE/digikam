////////////////////////////////////////////////////////////////////////////////
//
//    DIGIKAMAPP.CPP
//
//    Copyright (C) 2002-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
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
#include <qstringlist.h>
#include <qkeysequence.h>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kstdaccel.h>
#include <dcopclient.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <ktip.h>

// Local includes.

#include "albummanager.h"
#include "albuminfo.h"
#include "cameralist.h"
#include "cameratype.h"
#include "albumsettings.h"
#include "setup.h"
#include "setupplugins.h"
#include "digikamview.h"
#include "digikamcameraprocess.h"
#include "digikamapp.h"

// Includes files for plugins support.

#ifdef HAVE_KIPI
#include "kipiinterface.h"
#include <libkipi/pluginloader.h>
#include <libkipi/plugin.h>
#else
#include "digikampluginmanager.h"
#endif

 
DigikamApp::DigikamApp() : KMainWindow( 0, "Digikam" )
{
    m_instance = this;
    m_config = kapp->config();

    mFullScreen = false;
    mView = 0;

    mAlbumSettings = new AlbumSettings();
    mAlbumSettings->readSettings();

    mAlbumManager = new Digikam::AlbumManager(this);

    mCameraList = new CameraList(this, locateLocal("appdata", "cameras.xml"));

    connect(mCameraList, SIGNAL(signalCameraAdded(CameraType *)),
            this, SLOT(slotCameraAdded(CameraType *)));
            
    connect(mCameraList, SIGNAL(signalCameraRemoved(CameraType *)),
            this, SLOT(slotCameraRemoved(CameraType *)));

    setupView();
    setupActions();

    setAutoSaveSettings();
    applyMainWindowSettings (m_config);

    mAlbumManager->setLibraryPath(mAlbumSettings->getAlbumLibraryPath());
    mCameraList->load();

    // Load Plugins
        
    loadPlugins();
              
    // Start the camera process
    DigikamCameraProcess *process = new DigikamCameraProcess(this);
    process->start();
}

DigikamApp::~DigikamApp()
{
    if (mView)
        delete mView;

    delete m_config;
    
    mAlbumSettings->saveSettings();
    delete mAlbumSettings;
    
    m_instance = 0;
}

DigikamApp* DigikamApp::getinstance()
{
    return m_instance;
}

#ifdef HAVE_KIPI
const QPtrList<KAction>& DigikamApp::menuMergeActions()
{
    return m_kipiImageActions;
}
#endif

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
            
    connect(mView, SIGNAL(signal_imageSelected(bool)),
            this, SLOT(slot_imageSelected(bool)));
}

void DigikamApp::setupActions()
{
    mCameraMenuAction = new KActionMenu(i18n("&Camera"),
                                    "digitalcam",
                                    actionCollection(),
                                    "camera_menu");
    mCameraMenuAction->setDelayed(false);

    // -----------------------------------------------------------------

    mNewAction = new KAction(i18n("&New Album"),
                                    "albumfoldernew",
                                    KStdAccel::shortcut(KStdAccel::New),
                                    mView,
                                    SLOT(slot_newAlbum()),
                                    actionCollection(),
                                    "album_new");

    mAlbumSortAction = new KSelectAction(i18n("&Sort Albums"),
                                    0,
                                    0,
                                    actionCollection(),
                                    "album_sort");

    connect(mAlbumSortAction, SIGNAL(activated(int)),
            mView, SLOT(slot_sortAlbums(int)));

    // Use same list order as in albumsettings enum
    QStringList sortActionList;
    sortActionList.append(i18n("By Collection"));
    sortActionList.append(i18n("By Date"));
    sortActionList.append(i18n("Flat"));
    mAlbumSortAction->setItems(sortActionList);

    mDeleteAction = new KAction(i18n("Delete Album from HardDisk"),
                                    "edittrash",
                                    0,
                                    mView,
                                    SLOT(slot_deleteAlbum()),
                                    actionCollection(),
                                    "album_delete");

    mAddImagesAction = new KAction( i18n("Add Images"),
                                    "addimagefolder",
                                    CTRL+Key_I,
                                    mView,
                                    SLOT(slot_albumAddImages()),
                                    actionCollection(),
                                    "album_addImages");

    mPropsEditAction = new KAction( i18n("Edit Album Properties"),
                                    "albumfoldercomment",
                                    CTRL+Key_F3,
                                    mView,
                                    SLOT(slot_albumPropsEdit()),
                                    actionCollection(),
                                    "album_propsEdit");

    // -----------------------------------------------------------

    mImageViewAction = new KAction(i18n("View/Edit"),
                                    "editimage",
                                    Key_F4,
                                    mView,
                                    SLOT(slot_imageView()),
                                    actionCollection(),
                                    "image_view");

    mImageCommentsAction = new KAction(i18n("Edit Image Comments"),
                                    "imagecomment",
                                    Key_F3,
                                    mView,
                                    SLOT(slot_imageCommentsEdit()),
                                    actionCollection(),
                                    "image_comments");

    mImageExifAction = new KAction(i18n("View Exif Information"),
                                    "exifinfo",
                                    Key_F6,
                                    mView,
                                    SLOT(slot_imageExifInfo()),
                                    actionCollection(),
                                    "image_exif");

    mImageRenameAction = new KAction(i18n("Rename"),
                                    "pencil",
                                    Key_F2,
                                    mView,
                                    SLOT(slot_imageRename()),
                                    actionCollection(),
                                    "image_rename");

    mImageDeleteAction = new KAction(i18n("Delete"),
                                    "editdelete",
                                    SHIFT+Key_Delete,
                                    mView,
                                    SLOT(slot_imageDelete()),
                                    actionCollection(),
                                    "image_delete");

    mImagePropsAction = new KAction(i18n("Properties"),
                                    "image",
                                    ALT+Key_Return,
                                    mView,
                                    SLOT(slotImageProperties()),
                                    actionCollection(),
                                    "image_properties");

    // -----------------------------------------------------------------
    mSelectAllAction = new KAction(i18n("Select All"),
                                    0,
                                    CTRL+Key_A,
                                    mView,
                                    SLOT(slotSelectAll()),
                                    actionCollection(),
                                    "selectAll");

    // -----------------------------------------------------------------
    mSelectNoneAction = new KAction(i18n("Select None"),
                                    0,
                                    CTRL+Key_U,
                                    mView,
                                    SLOT(slotSelectNone()),
                                    actionCollection(),
                                    "selectNone");

    // -----------------------------------------------------------------
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
                                   ALT+Key_Plus,
                                   mView,
                                   SLOT(slot_thumbSizePlus()),
                                   actionCollection(),
                                   "album_thumbSizeIncrease");

    mThumbSizeMinusAction = new KAction(i18n("Decrease Thumbnail Size"),
                                   "viewmag-",
                                   ALT+Key_Minus,
                                   mView,
                                   SLOT(slot_thumbSizeMinus()),
                                   actionCollection(),
                                   "album_thumbSizeDecrease");

    mFullScreenAction = new KAction(i18n("Toggle Full Screen"),
                                   "window_fullscreen",
                                   CTRL+SHIFT+Key_F,
                                   this,
                                   SLOT(slotToggleFullScreen()),
                                   actionCollection(),
                                   "full_screen");

    mQuitAction = KStdAction::quit(this,
                                   SLOT(slot_exit()),
                                   actionCollection(),
                                   "app_exit");

    mTipAction = KStdAction::tipOfDay(this,
                                   SLOT(slotShowTip()),
                                   actionCollection(),
                                   "help_tipofday");
    
    #ifdef HAVE_KIPI    // For KIPI plugins.
    createGUI(QString::fromLatin1( "digikamui.rc" ), false);                                   
    #else               // For DigikamPlugins.
    createGUI();
    #endif
    
    // Initialize Actions ---------------------------------------
    mDeleteAction->setEnabled(false);
    mAddImagesAction->setEnabled(false);
    mPropsEditAction->setEnabled(false);

    mImageViewAction->setEnabled(false);
    mImageCommentsAction->setEnabled(false);
    mImageExifAction->setEnabled(false);
    mImageRenameAction->setEnabled(false);
    mImageDeleteAction->setEnabled(false);
    mImagePropsAction->setEnabled(false);

    mAlbumSortAction->setCurrentItem(
        (int)mAlbumSettings->getAlbumSortOrder());
}


void DigikamApp::enableThumbSizePlusAction(bool val)
{
    mThumbSizePlusAction->setEnabled(val);
}

void DigikamApp::enableThumbSizeMinusAction(bool val)
{
    mThumbSizeMinusAction->setEnabled(val);
}

void DigikamApp::slot_albumSelected(bool val)
{
    mDeleteAction->setEnabled(val);
    mAddImagesAction->setEnabled(val);
    mPropsEditAction->setEnabled(val);
}

void DigikamApp::slot_imageSelected(bool val)
{
    mImageViewAction->setEnabled(val);
    mImageCommentsAction->setEnabled(val);
    mImageExifAction->setEnabled(val);
    mImageRenameAction->setEnabled(val);
    mImageDeleteAction->setEnabled(val);
    mImagePropsAction->setEnabled(val);
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
        QString selectedAlbum = "";
        if (mAlbumManager->currentAlbum())
            selectedAlbum = mAlbumManager->currentAlbum()->getTitle();

        QByteArray arg, arg2;
        QDataStream stream(arg, IO_WriteOnly);
        stream << mAlbumSettings->getAlbumLibraryPath();
        stream << selectedAlbum;
        stream << ctype->title();
        stream << ctype->model();
        stream << ctype->port();
        stream << ctype->path();

        DCOPClient *client = kapp->dcopClient();
        
        if (!client->send("digikamcameraclient", "DigikamCameraClient",
                          "cameraOpen(QString,QString,QString,QString,QString,QString)",
                          arg))
            qWarning("DigikamApp: DCOP Communication Error");
        
        if (!client->send("digikamcameraclient", "DigikamCameraClient",
                          "cameraConnect()",
                          arg2))
            qWarning("DigikamApp: DCOP Communication Error");
        }
}

void DigikamApp::slotCameraAdded(CameraType *ctype)
{
    if (!ctype) return;

    KAction *cAction = new KAction(ctype->title(), 0,
                                   this, SLOT(slotCameraConnect()),
                                   actionCollection(),
                                   ctype->title().utf8());
    mCameraMenuAction->insert(cAction);
    ctype->setAction(cAction);
}

void DigikamApp::slotCameraRemoved(CameraType *ctype)
{
    if (!ctype) return;

    KAction *cAction = ctype->action();
    
    if (cAction)
        mCameraMenuAction->remove(cAction);
}

void DigikamApp::slotSetup()
{
    m_setup = new Setup;
    
    connect(m_setup, SIGNAL(okClicked()),
            this,  SLOT(slotSetupChanged()));
    
    #ifdef HAVE_KIPI    // KIPI plugins.
    
    KIPI::PluginLoader::List list = pluginLoader_->pluginList();
    QStringList availablePlugins;
    
    for( KIPI::PluginLoader::List::Iterator it = list.begin() ; it != list.end() ; ++it )
        {
        KIPI::Plugin* plugin = (*it).plugin;
        availablePlugins << (*it).name << (*it).comment;
        }
    
    m_setup->pluginsPage_->initPlugins(availablePlugins,
                                       availablePlugins);
    #else   // DigikamPlugins        
    m_setup->pluginsPage_->initPlugins(pluginManager_->availablePluginList(),
                                       pluginManager_->loadedPluginList());
    #endif                                   
    
    m_setup->show();
}

void DigikamApp::slotSetupChanged()
{
    #ifdef HAVE_KIPI    // KIPI plugins.
    /*pluginLoader_->loadPlugins(m_setup->pluginsPage_->getPluginList());
    m_config->writeEntry( "KIPI Plugins List", pluginLoader_->loadedPlugins() );*/
    #else               // DigikamPlugins 
    pluginManager_->loadPlugins(m_setup->pluginsPage_->getPluginList());
    m_config->writeEntry( "Digikam Plugins List", pluginManager_->loadedPluginList() );
    #endif     
    
    mView->applySettings(mAlbumSettings);
    mAlbumManager->setLibraryPath(mAlbumSettings->getAlbumLibraryPath());
    m_config->sync();
}

void DigikamApp::slotEditKeys()
{
    KKeyDialog* dialog = new KKeyDialog();
    dialog->insert( actionCollection(), i18n( "General" ) );
    
    #ifdef HAVE_KIPI    // KIPI plugins.

    KIPI::PluginLoader::List list = pluginLoader_->pluginList();
    
    for( KIPI::PluginLoader::List::Iterator it = list.begin() ; it != list.end() ; ++it ) 
        {
        KIPI::Plugin* plugin = (*it).plugin;
        dialog->insert( plugin->actionCollection(), (*it).comment );
        }
    
    #endif 
    
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
        }
    
    delete dlg;
}

void DigikamApp::slotToggleFullScreen()
{
    if (mFullScreen)
    {
        showNormal();
        mFullScreen = false;
        move(0, 0);
    }
    else
    {
        showFullScreen();
        mFullScreen = true;
    }
}

void DigikamApp::slotShowTip()
{
    KTipDialog::showTip("digikam/tips", true);
}

void DigikamApp::loadPlugins()
{
    #ifdef HAVE_KIPI    // Loading KIPI plugins.
    
    QStringList ignores;
    
    ignores << QString::fromLatin1( "HelloWorld" );
            
    interface_ = new KipiInterface( this, "KIPI interface" );

    pluginLoader_ = new KIPI::PluginLoader( ignores, interface_ );
    
/*    if ( m_config->readEntry("KIPI Plugins List") == QString::null )         
       pluginLoader_->loadPlugins();                             
    else 
       pluginLoader_->loadPlugins(m_config->readListEntry("KIPI Plugins List"));
*/
    KIPI::PluginLoader::List list = pluginLoader_->pluginList();
    
    for( KIPI::PluginLoader::List::Iterator it = list.begin(); it != list.end(); ++it ) 
        {
        KIPI::Plugin* plugin = (*it).plugin;
        plugin->setup( this );
        QPtrList<KAction>* popup = 0;
        
        if ( plugin->category() == KIPI::IMAGESPLUGIN )
            popup = &m_kipiImageActions;

        else if ( plugin->category() == KIPI::EXPORTPLUGIN  || plugin->category() == KIPI::IMPORTPLUGIN )
            popup = &m_kipiFileActions;

        else if ( plugin->category() == KIPI::TOOLSPLUGIN )
            popup = &m_kipiToolsActions;

        if ( popup ) 
            {
            KActionPtrList actions = plugin->actions();
            
            for( KActionPtrList::Iterator it = actions.begin(); it != actions.end(); ++it ) 
                {
                popup->append( *it );
                }
            }
        else 
            {
            qDebug("No menu found for a plugin !!!" ); // , plugin->id().latin1());
            }
        plugin->actionCollection()->readShortcutSettings();
        }
        
    // For this to work I need to pass false as second arg for createGUI
    
    plugActionList( QString::fromLatin1("file_actions"), m_kipiFileActions );
    plugActionList( QString::fromLatin1("image_actions"), m_kipiImageActions );
    plugActionList( QString::fromLatin1("tool_actions"), m_kipiToolsActions );

    #else   // Loading DigikamPlugins
    
    pluginManager_ = new DigikamPluginManager(this);
    
    if ( m_config->readEntry("Digikam Plugins List") == QString::null )         
       pluginManager_->loadPlugins();                             
    else 
       pluginManager_->loadPlugins(m_config->readListEntry("Digikam Plugins List"));
    
    #endif
    }

DigikamApp* DigikamApp::m_instance = 0;    

#include "digikamapp.moc"
