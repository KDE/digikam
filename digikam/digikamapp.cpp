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
#include <kdebug.h>
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

// Includes files for plugins support.

#include "kipiinterface.h"
#include <libkipi/pluginloader.h>
#include <libkipi/plugin.h>

// Local includes.

#include "albummanager.h"
#include "album.h"
#include "cameralist.h"
#include "cameratype.h"
#include "albumsettings.h"
#include "setup.h"
#include "setupplugins.h"
#include "digikamview.h"
#include "digikamcameraprocess.h"
#include "imagepluginloader.h"
#include "digikamapp.h"
 
DigikamApp::DigikamApp() : KMainWindow( 0, "Digikam" )
{
    m_instance = this;
    m_config = kapp->config();

    mFullScreen = false;
    mView = 0;

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

    setAutoSaveSettings();
    applyMainWindowSettings (m_config);

    mAlbumManager->setLibraryPath(mAlbumSettings->getAlbumLibraryPath());
    mCameraList->load();

    // Load KIPI Plugins.
        
    loadPlugins();
              
    // Start the camera process
    // todo:
    //DigikamCameraProcess *process = new DigikamCameraProcess(this);
    //process->start();

    mView->setInitialSizes();
}

DigikamApp::~DigikamApp()
{
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
    sortActionList.append(i18n("By Folder"));
    sortActionList.append(i18n("By Collection"));
    sortActionList.append(i18n("By Date"));
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

    mNewTagAction = new KAction(i18n("New &Tag"), "tag",
                                0, mView, SLOT(slotNewTag()),
                                actionCollection(), "tag_new");

    mDeleteTagAction = new KAction(i18n("Delete Tag"), "tag",
                                   0, mView, SLOT(slotDeleteTag()),
                                   actionCollection(), "tag_delete");

    mEditTagAction = new KAction( i18n("Edit Tag Properties"), "tag",
                                  0, mView, SLOT(slotEditTag()),
                                  actionCollection(), "tag_edit");

    // -----------------------------------------------------------
    
    mImageViewAction = new KAction(i18n("View/Edit"),
                                    "editimage",
                                    Key_F4,
                                    mView,
                                    SLOT(slot_imageView()),
                                    actionCollection(),
                                    "image_view");

    mImageCommentsAction = new KAction(i18n("Edit Image Comments and Tags"),
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

    mKipiHelpAction = new KAction(i18n("Plugins help"),
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

                                       
    createGUI(QString::fromLatin1( "digikamui.rc" ), false);                                   
    
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

    mAlbumSortAction->setCurrentItem((int)mAlbumSettings->getAlbumSortOrder());
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
    /* todo:
     *
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
            kdWarning() << "DigikamApp: DCOP Communication Error" << endl;
        
        if (!client->send("digikamcameraclient", "DigikamCameraClient",
                          "cameraConnect()",
                          arg2))
            kdWarning() << "DigikamApp: DCOP Communication Error" << endl;
    }
    */
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
    
    // For to show the number of KIPI plugins in the setup dialog.

    KIPI::PluginLoader::PluginList list = KipiPluginLoader_->pluginList();
    m_setup->pluginsPage_->initPlugins((int)list.count());
    
    m_setup->show();
}

void DigikamApp::slotSetupChanged()
{
    m_setup->pluginsPage_->applyPlugins();
    
    mAlbumManager->setLibraryPath(mAlbumSettings->getAlbumLibraryPath());
    mView->applySettings(mAlbumSettings);
    m_config->sync();

    KipiInterface_->readSettings();
}

void DigikamApp::slotEditKeys()
{
    KKeyDialog* dialog = new KKeyDialog();
    dialog->insert( actionCollection(), i18n( "General" ) );
    
    KIPI::PluginLoader::PluginList list = KipiPluginLoader_->pluginList();
    
    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin() ; it != list.end() ; ++it ) 
    {
        KIPI::Plugin* plugin = (*it)->plugin;
        dialog->insert( plugin->actionCollection(), (*it)->comment );
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
    QStringList tipsFiles;
    tipsFiles.append("digikam/tips");
    tipsFiles.append("kipi/tips");
    
    KTipDialog::showMultiTip(0, tipsFiles, true);
}

void DigikamApp::slotShowKipiHelp()
{
    KApplication::kApplication()->invokeHelp( QString::null, "kipi-plugins" ); 
}

void DigikamApp::loadPlugins()
{
    QStringList ignores;
    KipiInterface_ = new DigikamKipiInterface( this, "Digikam_KIPI_interface" );

    connect( mAlbumManager, SIGNAL( signalAlbumItemsSelected( bool ) ),
             KipiInterface_, SLOT( slotSelectionChanged( bool ) ) );

    connect( mAlbumManager, SIGNAL( signalAlbumCurrentChanged( AlbumInfo * ) ),
             KipiInterface_, SLOT( slotCurrentAlbumChanged( AlbumInfo * ) ) );
                 
    ignores << QString::fromLatin1( "HelloWorld" );    
    ignores << QString::fromLatin1( "KameraKlient" );    
    
    KipiPluginLoader_ = new KIPI::PluginLoader( ignores, KipiInterface_ );
    
    connect( KipiPluginLoader_, SIGNAL( replug() ),
             this, SLOT( slotKipiPluginPlug() ) );

    KipiPluginLoader_->loadPlugins();                             
    
    new ImagePluginLoader(this);
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
    
    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin() ; it != list.end() ; ++it ) 
        {
        KIPI::Plugin* plugin = (*it)->plugin;
        
        if ( !plugin || !(*it)->shouldLoad )
            continue;

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
        
    // Create GUI menu in according with plugins.
    
    plugActionList( QString::fromLatin1("file_actions_export"), m_kipiFileActionsExport );
    plugActionList( QString::fromLatin1("file_actions_import"), m_kipiFileActionsImport );
    plugActionList( QString::fromLatin1("image_actions"), m_kipiImageActions );
    plugActionList( QString::fromLatin1("tool_actions"), m_kipiToolsActions );
    plugActionList( QString::fromLatin1("batch_actions"), m_kipiBatchActions );
    plugActionList( QString::fromLatin1("album_actions"), m_kipiAlbumActions );
}

    
DigikamApp* DigikamApp::m_instance = 0;    

#include "digikamapp.moc"
