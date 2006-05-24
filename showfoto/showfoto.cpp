/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 *         Tom Albers <tomalbers@kde.nl>
 * Date  : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2005-2006 by Tom Albers 
 * Copyright 2006 by Gilles Caulier
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

// C Ansi includes.

extern "C"
{
#include <unistd.h>
}

// C++ includes.

#include <cstdio>

// Qt includes.

#include <qlabel.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qfileinfo.h>

// KDE includes.

#include <kcursor.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kimageio.h>
#include <kaccel.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kprotocolinfo.h>
#include <kglobalsettings.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kpopupmenu.h>
#include <kprogress.h>

// Local includes.

#include "rawfiles.h"
#include "canvas.h"
#include "thumbbar.h"
#include "imagepropertiessidebar.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "dimginterface.h"
#include "splashscreen.h"
#include "setup.h"
#include "setupimgplugins.h"
#include "iofileprogressbar.h"
#include "iccsettingscontainer.h"
#include "iofilesettingscontainer.h"
#include "loadingcacheinterface.h"
#include "savingcontextcontainer.h"
#include "showfoto.h"

namespace ShowFoto
{

ShowFoto::ShowFoto(const KURL::List& urlList)
        : Digikam::EditorWindow( "Showfoto" )
{
    m_currentItem            = 0;
    m_itemsNb                = 0;
    m_splash                 = 0;
    m_BCGAction              = 0;
    m_deleteItem2Trash       = true;
    m_fullScreenHideThumbBar = true;

    // -- Show splash at start ----------------------------
    
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    KGlobal::dirs()->addResourceType("data", KGlobal::dirs()->kde_default("data") + "digikam");
    KGlobal::iconLoader()->addAppDir("digikam");
    
    if(config->readBoolEntry("ShowSplash", true) && !kapp->isRestored())
    {
        m_splash = new Digikam::SplashScreen("showfoto-splash.png");
    }

    // -- Build the GUI -----------------------------------

    setupUserArea();
    setupStatusBar();
    setupActions();
    
    // Load image plugins to GUI

    m_imagePluginLoader = new Digikam::ImagePluginLoader(this, m_splash);
    loadImagePlugins();

    // If plugin core isn't available, plug BCG actions to collection instead.
    
    if ( !m_imagePluginLoader->pluginLibraryIsLoaded("digikamimageplugin_core") )
    {
        m_BCGAction = new KActionMenu(i18n("Brightness/Contrast/Gamma"), 0, 0, "showfoto_bcg");
        m_BCGAction->setDelayed(false);
    
        KAction *incGammaAction = new KAction(i18n("Increase Gamma"), 0, Key_G,
                                            this, SLOT(slotChangeBCG()),
                                            actionCollection(), "gamma_plus");
        KAction *decGammaAction = new KAction(i18n("Decrease Gamma"), 0, SHIFT+Key_G,
                                            this, SLOT(slotChangeBCG()),
                                            actionCollection(), "gamma_minus");
        KAction *incBrightAction = new KAction(i18n("Increase Brightness"), 0, Key_B,
                                            this, SLOT(slotChangeBCG()),
                                            actionCollection(), "brightness_plus");
        KAction *decBrightAction = new KAction(i18n("Decrease Brightness"), 0, SHIFT+Key_B,
                                            this, SLOT(slotChangeBCG()),
                                            actionCollection(), "brightness_minus");
        KAction *incContrastAction = new KAction(i18n("Increase Contrast"), 0, Key_C,
                                            this, SLOT(slotChangeBCG()),
                                            actionCollection(), "contrast_plus");
        KAction *decContrastAction = new KAction(i18n("Decrease Contrast"), 0, SHIFT+Key_C,
                                            this, SLOT(slotChangeBCG()),
                                            actionCollection(), "contrast_minus");
    
        m_BCGAction->insert(incBrightAction);
        m_BCGAction->insert(decBrightAction);
        m_BCGAction->insert(incContrastAction);
        m_BCGAction->insert(decContrastAction);
        m_BCGAction->insert(incGammaAction);
        m_BCGAction->insert(decGammaAction);

        QPtrList<KAction> bcg_actions;
        bcg_actions.append( m_BCGAction );
        unplugActionList( "showfoto_bcg" );
        plugActionList( "showfoto_bcg", bcg_actions );
    }

    // Create context menu.
    
    m_contextMenu = static_cast<QPopupMenu*>(factory()->container("RMBMenu", this));

    // Make signals/slots connections
    
    setupConnections();
    
    // -- Read settings --------------------------------
        
    readSettings();
    applySettings();
    setAutoSaveSettings("ImageViewer Settings");

    // -- Load current items ---------------------------

    for (KURL::List::const_iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        new Digikam::ThumbBarItem(m_bar, *it);
        m_lastOpenedDirectory=(*it);
    }

    if ( urlList.isEmpty() )
    {
        emit signalNoCurrentItem();
        toggleActions(false);
        toggleNavigation(0);
    }
    else
    {
        toggleNavigation(1);
        toggleActions(true);
    }
}

ShowFoto::~ShowFoto()
{
    unLoadImagePlugins();

    delete m_imagePluginLoader;
    delete m_bar;
    delete m_rightSidebar;
}

void ShowFoto::closeEvent(QCloseEvent* e)
{
    if (!e)
        return;

    if (m_currentItem && !promptUserSave(m_currentItem->url()))
        return;

    saveSettings();
    e->accept();
}

void ShowFoto::setupConnections()
{
    setupStandardConnections();

    connect(m_bar, SIGNAL(signalURLSelected(const KURL&)),
            this, SLOT(slotOpenURL(const KURL&)));

    connect(m_bar, SIGNAL(signalItemAdded()),
            this, SLOT(slotUpdateItemInfo()));

    connect(this, SIGNAL(signalSelectionChanged( QRect* )),
            m_rightSidebar, SLOT(slotImageSelectionChanged( QRect * )));

    connect(this, SIGNAL(signalNoCurrentItem()),
            m_rightSidebar, SLOT(slotNoCurrentItem()));
}

void ShowFoto::setupUserArea()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    QWidget* widget = new QWidget(this);
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    
    if(!config->readBoolEntry("HorizontalThumbbar", false)) // Vertical thumbbar layout
    {
        QHBoxLayout *hlay = new QHBoxLayout(widget);
        m_splitter        = new QSplitter(widget);
        m_canvas          = new Digikam::Canvas(m_splitter);
        m_canvas->setSizePolicy(rightSzPolicy);
        
        m_rightSidebar    = new Digikam::ImagePropertiesSideBar(widget, "ShowFoto Sidebar Right", m_splitter, 
                                                                Digikam::Sidebar::Right);
        m_bar             = new Digikam::ThumbBarView(widget, Digikam::ThumbBarView::Vertical);
        
        hlay->addWidget(m_bar);
        hlay->addWidget(m_splitter);
        hlay->addWidget(m_rightSidebar);
    }
    else                                                     // Horizontal thumbbar layout
    {
        m_splitter        = new QSplitter(widget);
        QWidget* widget2  = new QWidget(m_splitter);
        QVBoxLayout *vlay = new QVBoxLayout(widget2);
        m_canvas          = new Digikam::Canvas(widget2);
        m_canvas->setSizePolicy(rightSzPolicy);

        m_bar             = new Digikam::ThumbBarView(widget2, Digikam::ThumbBarView::Horizontal);

        vlay->addWidget(m_canvas);
        vlay->addWidget(m_bar);
                
        QHBoxLayout *hlay = new QHBoxLayout(widget);
        m_rightSidebar    = new Digikam::ImagePropertiesSideBar(widget, "ShowFoto Sidebar Right", m_splitter, 
                                                                Digikam::Sidebar::Right);

        hlay->addWidget(m_splitter);
        hlay->addWidget(m_rightSidebar);        
    }        

    m_splitter->setFrameStyle( QFrame::NoFrame );
    m_splitter->setFrameShadow( QFrame::Plain );
    m_splitter->setFrameShape( QFrame::NoFrame );
    m_splitter->setOpaqueResize(false);
    setCentralWidget(widget);
    m_rightSidebar->loadViewState();    
}

void ShowFoto::setupActions()
{
    setupStandardActions();

    // Extra 'File' menu actions ---------------------------------------------

    m_fileOpenAction = KStdAction::open(this, SLOT(slotOpenFile()),
                       actionCollection(), "showfoto_open_file");

    m_openFilesInFolderAction = new KAction(i18n("Open folder"),
                                            "folder_image",
                                            CTRL+SHIFT+Key_O,
                                            this,
                                            SLOT(slotOpenFilesInFolder()),
                                            actionCollection(),
                                            "showfoto_open_folder");

    // Extra 'View' menu actions ---------------------------------------------

    m_showBarAction = new KToggleAction(i18n("Hide Thumbnails"), 0, Key_T,
                                        this, SLOT(slotToggleShowBar()),
                                        actionCollection(), "shofoto_showthumbs");

    // --- Create the gui --------------------------------------------------------------

    createGUI("showfotoui.rc", false);

    setupStandardAccelerators();
}

void ShowFoto::readSettings()
{
    readStandardSettings();
    
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    
    bool showBar = false;
    showBar = config->readBoolEntry("Show Thumbnails", true);
    
    if (!showBar && m_showBarAction->isChecked())
        m_showBarAction->activate();

    m_lastOpenedDirectory.setPath( config->readEntry("Last Opened Directory",
                                   KGlobalSettings::documentPath()) );    
}

void ShowFoto::saveSettings()
{
    saveStandardSettings();
    
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    
    config->writeEntry("Last Opened Directory", m_lastOpenedDirectory.path() );
    config->writeEntry("Show Thumbnails", !m_showBarAction->isChecked());

    config->sync();    
}

void ShowFoto::applySettings()
{
    applyStandardSettings();
    
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    
    // Current image deleted go to trash ?
    m_deleteItem2Trash = config->readBoolEntry("DeleteItem2Trash", true);
    if (m_deleteItem2Trash)
    {
        m_fileDeleteAction->setIcon("edittrash");
        m_fileDeleteAction->setText(i18n("Move to Trash"));
    }
    else
    {
        m_fileDeleteAction->setIcon("editdelete");
        m_fileDeleteAction->setText(i18n("Delete File"));
    }

    bool exifRotate = config->readBoolEntry("EXIF Rotate", true);
    m_canvas->setExifOrient(exifRotate);
    m_bar->setExifRotate(exifRotate);

    m_setExifOrientationTag = config->readBoolEntry("EXIF Set Orientation", true);
    
    m_fullScreenHideThumbBar = config->readBoolEntry("FullScreenHideThumbBar", true);
}

void ShowFoto::slotOpenFile()
{
    if (m_currentItem && !promptUserSave(m_currentItem->url()))
        return;

    QString fileformats;
    
#if KDE_IS_VERSION(3,5,2)
    //-- With KDE version >= 3.5.2, "image/x-raw" type mime exist ------------------------------
    
    fileformats = KImageIO::mimeTypes(KImageIO::Reading).join(" ");
#else
    //-- with KDE version < 3.5.2, we need to add all camera RAW file formats ------------------
    
    QStringList patternList = QStringList::split('\n', KImageIO::pattern(KImageIO::Reading));
    
    // All Pictures from list must been always the first entry given by KDE API
    QString allPictures = patternList[0];
    
    // Add RAW file format to All Pictures" type mime and remplace current.
    allPictures.insert(allPictures.find("|"), QString(raw_file_extentions));
    patternList.remove(patternList[0]);
    patternList.prepend(allPictures);
    
    // Added RAW file formats supported by dcraw program like a type mime. 
    // Nota: we cannot use here "image/x-raw" type mime from KDE because it 
    // will be only available for KDE 3.5.2, not before (see file #121242 in B.K.O).
    patternList.append(QString("\n%1|Camera RAW files").arg(QString(raw_file_extentions)));
    
    fileformats = patternList.join("\n");
#endif

    kdDebug () << "fileformats=" << fileformats << endl;   
    
    KURL::List urls =  KFileDialog::getOpenURLs(m_lastOpenedDirectory.path(), fileformats, this, i18n("Open Images"));

    if (!urls.isEmpty())
    {
        m_bar->clear();

        for (KURL::List::const_iterator it = urls.begin();
             it != urls.end(); ++it)
        {
            new Digikam::ThumbBarItem(m_bar, *it);
            m_lastOpenedDirectory=(*it);
        }

        toggleActions(true);
    }
}

void ShowFoto::slotOpenURL(const KURL& url)
{
    if(m_currentItem && !promptUserSave(m_currentItem->url()))
    {
        m_bar->blockSignals(true);
        m_bar->setSelected(m_currentItem);
        m_bar->blockSignals(false);
        return;
    }

    m_currentItem = m_bar->currentItem();
    if(!m_currentItem)
        return;

    QString localFile;
#if KDE_IS_VERSION(3,2,0)
    KIO::NetAccess::download(url, localFile, this);
#else
    KIO::NetAccess::download(url, localFile);
#endif
    
    m_canvas->load(localFile, m_IOFileSettings);

    // TODO : add preload here like in ImageWindow::slotLoadCurrent() ???
}

void ShowFoto::toggleGUI2FullScreen()
{
    if (m_fullScreen)
    {
        m_rightSidebar->restore();

        // If Hide Thumbbar option is checked, restore it.
        if (!m_showBarAction->isChecked())
            m_bar->show();
    }
    else
    {
        m_rightSidebar->backup();

        // If Hide Thumbbar option is checked, catch it if necessary.
        if (!m_showBarAction->isChecked())
        {
            if (m_fullScreenHideThumbBar)
                m_bar->hide();
        }
    }
}

void ShowFoto::slotToggleShowBar()
{
    if (m_showBarAction->isChecked())
        m_bar->hide();
    else
        m_bar->show();
}

void ShowFoto::slotChangeBCG()
{
    QString name;
    if (sender())
        name = sender()->name();

    if (name == "gamma_plus")
    {
        m_canvas->increaseGamma();
    }
    else if  (name == "gamma_minus")
    {
        m_canvas->decreaseGamma();
    }
    else if  (name == "brightness_plus")
    {
        m_canvas->increaseBrightness();
    }
    else if  (name == "brightness_minus")
    {
        m_canvas->decreaseBrightness();
    }
    else if  (name == "contrast_plus")
    {
        m_canvas->increaseContrast();
    }
    else if  (name == "contrast_minus")
    {
        m_canvas->decreaseContrast();
    }
}

void ShowFoto::slotChanged()
{
    QString mpixels;
    QSize dims(m_canvas->imageWidth(), m_canvas->imageHeight());
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
    QString str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)")
                  .arg(dims.width()).arg(dims.height()).arg(mpixels);
    m_resLabel->setText(str);

    if (m_currentItem)
    {
        if (m_currentItem->url().isValid())
        {
            QRect sel          = m_canvas->getSelectedArea();
            Digikam::DImg* img = Digikam::DImgInterface::instance()->getImg();
            m_rightSidebar->itemChanged(m_currentItem->url(),
                                        sel.isNull() ? 0 : &sel, img);
        }
    }    
}

void ShowFoto::slotUndoStateChanged(bool moreUndo, bool moreRedo, bool canSave)
{
    m_revertAction->setEnabled(canSave);
    m_undoAction->setEnabled(moreUndo);
    m_redoAction->setEnabled(moreRedo);
    m_saveAction->setEnabled(canSave);
    
    if (!moreUndo)
        m_rotatedOrFlipped = false;        
}

void ShowFoto::toggleActions(bool val)
{
    toggleStandardActions(val);
        
    // if BCG actions exists then toggle it.
    if (m_BCGAction)
        m_BCGAction->setEnabled(val);

    // if no active slideshow then toggle it.
    if (!m_slideShowAction->isChecked())
        m_slideShowAction->setEnabled(val);
}

void ShowFoto::toggleActions2SlideShow(bool val)
{
    toggleActions(val);
    
    // if slideshow mode then toggle file open actions.
    m_fileOpenAction->setEnabled(val);
    m_openFilesInFolderAction->setEnabled(val);
}

void ShowFoto::slotFilePrint()
{
    printImage(m_currentItem->url());
}

void ShowFoto::show()
{
    if(m_splash)
    {
        m_splash->finish(this);
        delete m_splash;
        m_splash = 0;
    }
    KMainWindow::show();
}

void ShowFoto::setup(bool iccSetupPage)
{
    Setup setup(this, 0, iccSetupPage ? Setup::ICCPage : Setup::EditorPage);
    
    if (setup.exec() != QDialog::Accepted)
        return;

    unLoadImagePlugins();
    m_imagePluginLoader->loadPluginsFromList(setup.imagePluginsPage()->getImagePluginsListEnable());
    kapp->config()->sync();
    loadImagePlugins();
    
    applySettings();

    if ( m_itemsNb == 0 )
    {
        slotUpdateItemInfo();
        toggleActions(false);
    }
}

void ShowFoto::slotUpdateItemInfo(void)
{
    m_itemsNb = m_bar->countItems();
    
    m_rotatedOrFlipped = false;
    int index = 0;
    QString text;
    
    if (m_itemsNb > 0)
    {
        index = 1;
        
        for (Digikam::ThumbBarItem *item = m_bar->firstItem(); item; item = item->next())
        {
            if (item->url().equals(m_currentItem->url()))
            {
                break;
            }
            index++;
        }

        text = m_currentItem->url().filename() +
                   i18n(" (%2 of %3)")
                   .arg(QString::number(index))
                   .arg(QString::number(m_itemsNb));
    
        setCaption(m_currentItem->url().directory());
    }
    else 
    {
        text = "";
        setCaption("");
    }
    
    m_nameLabel->setText(text);
    
    toggleNavigation( index );
}
    
void ShowFoto::slotOpenFolder(const KURL& url)
{
    if (m_currentItem && !promptUserSave(m_currentItem->url()))
        return;

    m_canvas->load(QString::null, m_IOFileSettings);
    m_bar->clear(true);
    emit signalNoCurrentItem();
    m_currentItem = 0;
    
    if (!url.isValid() || !url.isLocalFile())
       return;

    // Parse KDE image IO mime types registration to get files filter pattern.

    QStringList mimeTypes = KImageIO::mimeTypes(KImageIO::Reading);
    QString filter;

    for (QStringList::ConstIterator it = mimeTypes.begin() ; it != mimeTypes.end() ; ++it)
    {    
        QString format = KImageIO::typeForMime(*it);
        filter.append ("*.");
        filter.append (format);
        filter.append (" ");
    }    

    // Because KImageIO return only *.JPEG and *.TIFF mime types.
    if ( filter.contains("*.TIFF") )
        filter.append (" *.TIF");
    if ( filter.contains("*.JPEG") )
        filter.append (" *.JPG");

    // Added RAW files estentions supported by dcraw program and 
    // defines to digikam/libs/dcraw/rawfiles.h
    filter.append (" ");
    filter.append ( QString(raw_file_extentions) );  
    filter.append (" ");

    QString patterns = filter.lower();
    patterns.append (" ");
    patterns.append (filter.upper());

    kdDebug () << "patterns=" << patterns << endl;    

    // Get all image files from directory.

    QDir dir(url.path(), patterns);
    
    if (!dir.exists())
       return;
    
    // Directory items sorting. Perhaps we need to add any settings in config dialog.
    dir.setFilter ( QDir::Files | QDir::NoSymLinks );
    dir.setSorting ( QDir::Time );

    const QFileInfoList* fileinfolist = dir.entryInfoList();
    if (!fileinfolist)
       return;
    
    QFileInfoListIterator it(*fileinfolist);
    QFileInfo* fi;

    // And open all items in image editor.

    while( (fi = it.current() ) )
    {
        new Digikam::ThumbBarItem( m_bar, KURL(fi->filePath()) );
        ++it;
    }
        
    toggleActions(true);
    toggleNavigation(1);
}
    
void ShowFoto::slotOpenFilesInFolder()
{
    if (m_currentItem && !promptUserSave(m_currentItem->url()))
        return;

    KURL url(KFileDialog::getExistingDirectory(m_lastOpenedDirectory.directory(), 
                                               this, i18n("Open Images From Directory")));

    if (!url.isEmpty())
    {
       m_lastOpenedDirectory = url;
       slotOpenFolder(url);
    }
}

void ShowFoto::slotFirst()
{
    if (m_currentItem && !promptUserSave(m_currentItem->url()))
        return;

    m_bar->setSelected( m_bar->firstItem() );
}

void ShowFoto::slotLast()
{
    if (m_currentItem && !promptUserSave(m_currentItem->url()))
        return;

    m_bar->setSelected( m_bar->lastItem() );
}

void ShowFoto::slotForward()
{
    if (m_currentItem && !promptUserSave(m_currentItem->url()))
        return;

    Digikam::ThumbBarItem* curr = m_bar->currentItem();
    if (curr && curr->next())
    {
        m_bar->setSelected(curr->next());
        m_currentItem = m_bar->currentItem();
    }
}

void ShowFoto::slotBackward()
{
    if (m_currentItem && !promptUserSave(m_currentItem->url()))
        return;

    Digikam::ThumbBarItem* curr = m_bar->currentItem();
    if (curr && curr->prev())
    {
        m_bar->setSelected(curr->prev());
        m_currentItem = m_bar->currentItem();
    }
}

void ShowFoto::toggleNavigation(int index)
{
    if ( m_itemsNb == 0 || m_itemsNb == 1 ) 
    {
        m_backwardAction->setEnabled(false);
        m_forwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }
    else 
    {
        m_backwardAction->setEnabled(true);
        m_forwardAction->setEnabled(true);
        m_firstAction->setEnabled(true);
        m_lastAction->setEnabled(true);
    }
    
    if (index == 1) 
    {
        m_backwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
    }

    if (index == m_itemsNb) 
    {
        m_forwardAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }
}

void ShowFoto::slotLoadingStarted(const QString& filename)
{
    Digikam::EditorWindow::slotLoadingStarted(filename);

    // Here we disable specific actions on showfoto.
    m_openFilesInFolderAction->setEnabled(false);
    m_fileOpenAction->setEnabled(false);
}

void ShowFoto::slotLoadingFinished(const QString& filename, bool success)
{
    Digikam::EditorWindow::slotLoadingFinished(filename, success);
    
    // Here we re-enable specific actions on showfoto.
    m_openFilesInFolderAction->setEnabled(true);
    m_fileOpenAction->setEnabled(true);
}

void ShowFoto::slotSavingStarted(const QString& filename)
{
    Digikam::EditorWindow::slotSavingStarted(filename);

    // Here we disable specific actions on showfoto.
    m_openFilesInFolderAction->setEnabled(false);
    m_fileOpenAction->setEnabled(false);
}

void ShowFoto::finishSaving(bool success)
{
    Digikam::EditorWindow::finishSaving(success);

    // Here we re-enable specific actions on showfoto.
    m_openFilesInFolderAction->setEnabled(true);
    m_fileOpenAction->setEnabled(true);
}

void ShowFoto::saveIsComplete()
{
    Digikam::LoadingCacheInterface::putImage(m_savingContext->destinationURL.path(), m_canvas->currentImage());
    m_bar->invalidateThumb(m_currentItem);
    //slotOpenURL(m_currentItem->url());
}

void ShowFoto::saveAsIsComplete()
{
    m_canvas->switchToLastSaved(m_savingContext->destinationURL.path());
    Digikam::LoadingCacheInterface::putImage(m_savingContext->destinationURL.path(), m_canvas->currentImage());

    // Add the file to the list of thumbbar images if it's not there already
    Digikam::ThumbBarItem* foundItem = m_bar->findItemByURL(m_savingContext->destinationURL);
    m_bar->invalidateThumb(foundItem);

    if (!foundItem)
        foundItem = new Digikam::ThumbBarItem(m_bar, m_savingContext->destinationURL);

    // shortcut slotOpenURL
    m_bar->blockSignals(true);
    m_bar->setSelected(foundItem);
    m_bar->blockSignals(false);
    m_currentItem = foundItem;
}

bool ShowFoto::save()
{
    if (!m_currentItem)
    {
        kdWarning() << k_funcinfo << "This should not happen" << endl;
        return true;
    }

    if (!m_currentItem->url().isLocalFile())
    {
        KMessageBox::sorry(this, i18n("No yet support for saving non-local files"));
        return false;
    }

    startingSave(m_currentItem->url());
    return true;
}

bool ShowFoto::saveAs()
{
    if (!m_currentItem)
    {
        kdWarning() << k_funcinfo << "This should not happen" << endl;
        return false;
    }

    return ( startingSaveAs(m_currentItem->url()) );
}

void ShowFoto::slotDeleteCurrentItem()
{
    KURL urlCurrent(m_currentItem->url());

    if (!m_deleteItem2Trash)
    {
        QString warnMsg(i18n("About to Delete File \"%1\"\nAre you sure?")
                        .arg(urlCurrent.filename()));
        if (KMessageBox::warningContinueCancel(this,
                                               warnMsg,
                                               i18n("Warning"),
                                               i18n("Delete"))
            !=  KMessageBox::Continue)
        {
            return;
        }
        else
        {
            KIO::Job* job = KIO::del( urlCurrent );
            connect( job, SIGNAL(result( KIO::Job* )),
                     this, SLOT(slotDeleteCurrentItemResult( KIO::Job*)) );
        }
    }
    else
    {
        KURL dest("trash:/");

        if (!KProtocolInfo::isKnownProtocol(dest))
        {
            dest = KGlobalSettings::trashPath();
        }

        KIO::Job* job = KIO::move( urlCurrent, dest );
        connect( job, SIGNAL(result( KIO::Job* )),
                 this, SLOT(slotDeleteCurrentItemResult( KIO::Job*)) );
    }
}

void ShowFoto::slotDeleteCurrentItemResult( KIO::Job * job )
{
    if (job->error() != 0)
    {
        QString errMsg(job->errorString());
        KMessageBox::error(this, errMsg);
        return;
    }

    // No error, remove item in thumbbar.

    Digikam::ThumbBarItem *item2remove = m_currentItem;

    for (Digikam::ThumbBarItem *item = m_bar->firstItem(); item; item = item->next())
    {
        if (item->url().equals(item2remove->url()))
        {
            m_bar->removeItem(item);
            break;
        }
    }
    
    m_itemsNb = m_bar->countItems();

    // Disable menu actions and SideBar if no current image.

    if ( m_itemsNb == 0 )
    {
        emit signalNoCurrentItem();
        slotUpdateItemInfo();
        toggleActions(false);
        m_canvas->load(QString::null, m_IOFileSettings);
        m_currentItem = 0;
    }
    else
    {
        m_currentItem = m_bar->currentItem();
        QTimer::singleShot(0, this, SLOT(slotOpenURL(m_currentItem->url())));
    }
}

}   // namespace ShowFoto

#include "showfoto.moc"
