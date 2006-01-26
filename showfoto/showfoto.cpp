/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
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

// Lib KExif includes.

#include <libkexif/kexifdata.h>
#include <libkexif/kexifutils.h>

// Local includes.

#include "rawfiles.h"
#include "canvas.h"
#include "thumbbar.h"
#include "imagepropertiessidebar.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageresizedlg.h"
#include "imageprint.h"
#include "dimginterface.h"
#include "splashscreen.h"
#include "setup.h"
#include "setupimgplugins.h"
#include "slideshow.h"
#include "iofileprogressbar.h"
#include "iccsettingscontainer.h"
#include "iofilesettingscontainer.h"
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

    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    KGlobal::dirs()->addResourceType("data", KGlobal::dirs()->kde_default("data") + "digikam");
    KGlobal::iconLoader()->addAppDir("digikam");
    
    if(config->readBoolEntry("ShowSplash", true) && !kapp->isRestored())
    {
        m_splash = new Digikam::SplashScreen("showfoto-splash.png");
    }

    // Construct the view

    setupUserArea();
    setupStatusBar();

    // Build the GUI

    setupActions();
    m_slideShow = new Digikam::SlideShow(m_firstAction, m_forwardAction);
    
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

    m_contextMenu = static_cast<QPopupMenu*>(factory()->container("RMBMenu", this));

    // -- setup connections ---------------------------

    setupConnections();
    
    // -- read settings --------------------------------
        
    readSettings();
    applySettings();
    setAutoSaveSettings("ImageViewer Settings");    

    //-------------------------------------------------------------

    for (KURL::List::const_iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        new Digikam::ThumbBarItem(m_bar, *it);
        m_lastOpenedDirectory=(*it);
    }

    if ( urlList.isEmpty() )
    {
       m_rightSidebar->noCurrentItem();
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
    delete m_slideShow;
    delete m_rightSidebar;
}

void ShowFoto::setupConnections()
{
    setupStandardConnections();
    
    connect(m_bar, SIGNAL(signalURLSelected(const KURL&)),
            this, SLOT(slotOpenURL(const KURL&)));

    connect(m_bar, SIGNAL(signalItemAdded()),
            this, SLOT(slotUpdateItemInfo()));
    
    connect(m_slideShow, SIGNAL(finished()),
            m_slideShowAction, SLOT(activate()) );
}

void ShowFoto::setupUserArea()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    
    QWidget* widget = new QWidget(this);

    if(!config->readBoolEntry("HorizontalThumbbar", false)) // Vertical thumbbar layout
    {
        QHBoxLayout *hlay = new QHBoxLayout(widget);
        m_splitter        = new QSplitter(widget);
        m_canvas          = new Digikam::Canvas(m_splitter);
        m_rightSidebar    = new Digikam::ImagePropertiesSideBar(widget, m_splitter, Digikam::Sidebar::Right);
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
        m_bar             = new Digikam::ThumbBarView(widget2, Digikam::ThumbBarView::Horizontal);

        vlay->addWidget(m_canvas);
        vlay->addWidget(m_bar);
                
        QHBoxLayout *hlay = new QHBoxLayout(widget);
        m_rightSidebar    = new Digikam::ImagePropertiesSideBar(widget, m_splitter, Digikam::Sidebar::Right);

        hlay->addWidget(m_splitter);
        hlay->addWidget(m_rightSidebar);        
    }        

    m_splitter->setOpaqueResize(false);
    setCentralWidget(widget);        
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

    m_slideShowAction = new KToggleAction(i18n("Slide Show"), "slideshow", 0,
                                          this, SLOT(slotToggleSlideShow()),
                                          actionCollection(),"shofoto_slideshow");

    // -- Configure toolbar and shortcuts ---------------------------------------------

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

    m_fullScreenHideThumbBar = config->readBoolEntry("FullScreenHideThumbBar", true);
    
    // Slideshow Settings.
    m_slideShowInFullScreen = config->readBoolEntry("SlideShowFullScreen", true);
    m_slideShow->setStartWithCurrent(config->readBoolEntry("SlideShowStartCurrent", false));
    m_slideShow->setLoop(config->readBoolEntry("SlideShowLoop", false));
    m_slideShow->setDelay(config->readNumEntry("SlideShowDelay", 5));
}

void ShowFoto::slotOpenFile()
{
    if (!promptUserSave())
        return;

    QString mimetypes = KImageIO::mimeTypes(KImageIO::Reading).join(" ");
    
    // Added RAW file format type mimes supported by dcraw program.
    mimetypes.append (" image/x-raw");
    
    KURL::List urls =  KFileDialog::getOpenURLs(m_lastOpenedDirectory.path(),
                                                mimetypes,
                                                this,
                                                i18n("Open Images"));

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

void ShowFoto::slotFirst()
{
    if (!promptUserSave())
        return;

    m_bar->setSelected( m_bar->firstItem() );
}

void ShowFoto::slotLast()
{
    if (!promptUserSave())
        return;

    m_bar->setSelected( m_bar->lastItem() );
}

void ShowFoto::slotForward()
{
    if (!promptUserSave())
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
    if (!promptUserSave())
        return;

    Digikam::ThumbBarItem* curr = m_bar->currentItem();
    if (curr && curr->prev())
    {
        m_bar->setSelected(curr->prev());
        m_currentItem = m_bar->currentItem();
    }
}

bool ShowFoto::saveAs()
{
    if (!m_currentItem)
    {
        kdWarning() << k_funcinfo << "This should not happen" << endl;
        return false;
    }

    m_savingContext->currentURL = m_currentItem->url();

    // FIXME : Add 16 bits file formats

    QString mimetypes = KImageIO::mimeTypes(KImageIO::Writing).join(" ");

    kdDebug () << "mimetypes=" << mimetypes << endl;    

    KFileDialog saveDialog(m_savingContext->currentURL.isLocalFile() ? m_savingContext->currentURL.directory() : QDir::homeDirPath(),
                           QString::null,
                           this,
                           "imageFileSaveDialog",
                           false);
    saveDialog.setOperationMode( KFileDialog::Saving );
    saveDialog.setMode( KFile::File );
    saveDialog.setSelection(m_savingContext->currentURL.fileName());
    saveDialog.setCaption( i18n("New Image File Name") );
    saveDialog.setFilter(mimetypes);

    if ( saveDialog.exec() != KFileDialog::Accepted )
    {
        return false;
    }

    m_savingContext->saveAsURL = saveDialog.selectedURL();

    // Check if target image format have been selected from Combo List of SaveAs dialog.
    QString format = KImageIO::typeForMime(saveDialog.currentMimeFilter());

    if ( m_savingContext->format.isEmpty() )
    {
        // Else, check if target image format have been add to target image file name using extension.

        QFileInfo fi(m_savingContext->saveAsURL.path());
        m_savingContext->format = fi.extension(false);
        
        if ( m_savingContext->format.isEmpty() )
        {
            // If format is empty then file format is same as that of the original file.
            m_savingContext->format = QImageIO::imageFormat(m_savingContext->currentURL.path());
        }
        else
        {
            // Else, check if format from file name extension is include on file mime type list.

            QString imgExtPattern;
            QStringList imgExtList = QStringList::split(" ", mimetypes);
            for (QStringList::ConstIterator it = imgExtList.begin() ; it != imgExtList.end() ; it++)
            {    
                imgExtPattern.append (KImageIO::typeForMime(*it).upper());
                imgExtPattern.append (" ");
            }    
            imgExtPattern.append (" TIF TIFF");
            if ( imgExtPattern.contains("JPEG") ) imgExtPattern.append (" JPG");
    
            if ( !imgExtPattern.contains( m_savingContext->format.upper() ) )
            {
                KMessageBox::error(this, i18n("Target image file format \"%1\" unsupported.")
                        .arg(m_savingContext->format));
                kdWarning() << k_funcinfo << "target image file format " << m_savingContext->format << " unsupported!" << endl;
                return false;
            }
        }
    }

    if (!m_savingContext->saveAsURL.isValid())
    {
        KMessageBox::error(this, i18n("Invalid target selected"));
        return false;
    }

    m_savingContext->currentURL.cleanPath();
    m_savingContext->saveAsURL.cleanPath();

    if (m_savingContext->currentURL.equals(m_savingContext->saveAsURL))
    {
        slotSave();
        return false;
    }

    QFileInfo fi(m_savingContext->saveAsURL.path());
    if ( fi.exists() )
    {
        int result =
            KMessageBox::warningYesNo( this, i18n("About to overwrite file %1. "
                                                  "Are you sure you want to continue?")
                                       .arg(m_savingContext->saveAsURL.filename()) );

        if (result != KMessageBox::Yes)
            return false;
    }
    
    m_savingContext->tmpFile = m_savingContext->saveAsURL.directory() + QString("/.showfoto-tmp-")
                      + m_savingContext->saveAsURL.filename();
    m_savingContext->fromSave = false;
    m_canvas->saveAsTmpFile(m_savingContext->tmpFile, m_IOFileSettings, m_savingContext->format.lower());

    return true;
}

bool ShowFoto::promptUserSave()
{
    if (!m_currentItem)
        return true;

    if (m_saveAction->isEnabled())
    {
        int result = KMessageBox::warningYesNoCancel(this,
                                  i18n("The image '%1\' has been modified.\n"
                                       "Do you want to save it?")
                                       .arg(m_currentItem->url().filename()),
                                  QString::null,
                                  KStdGuiItem::save(),
                                  KStdGuiItem::discard());

        if (result == KMessageBox::Yes)
        {
            m_savingContext->progressDialog = new KProgressDialog(this, 0, QString::null,
                    i18n("Saving \"%1\"").arg(m_currentItem->url().filename()), true );
            m_savingContext->progressDialog->setAllowCancel(false);

            bool saving;
            if (m_isReadOnly)
                saving = saveAs();
            else
                saving = save();

            if (saving)
            {
                // if saving has started, enter modal dialog to synchronize
                // on the saving operation. When saving is finished, the dialog is closed.
                m_savingContext->progressDialog->exec();
                delete m_savingContext->progressDialog;
                m_savingContext->progressDialog = 0;
                return m_savingContext->synchronousSavingResult;
            }
            else
                return false;
        }
        else if (result == KMessageBox::No)
        {
            m_saveAction->setEnabled(false);
            return true;
        }
        else
            return false;
    }
    return true;
}

bool ShowFoto::save()
{
    if (!m_currentItem)
    {
        kdWarning() << k_funcinfo << "This should not happen" << endl;
        return true;
    }

    m_savingContext->currentURL = m_currentItem->url();
    if (!m_savingContext->currentURL.isLocalFile())
    {
        KMessageBox::sorry(this, i18n("No support for saving non-local files"));
        return false;
    }

    m_savingContext->tmpFile = m_savingContext->currentURL.directory() + QString("/.showfoto-tmp-")
                      + m_savingContext->currentURL.filename();
    
    m_savingContext->fromSave = true;
    m_canvas->saveAsTmpFile(m_savingContext->tmpFile, m_IOFileSettings);

    return true;
}

void ShowFoto::slotSavingStarted(const QString &filename)
{
    //TODO: disable actions as appropriate
    kapp->setOverrideCursor( KCursor::waitCursor() );
}

void ShowFoto::slotSavingFinished(const QString &filename, bool success)
{
    //TODO
    if (m_savingContext->fromSave)
    {
        // from save()
        if (!success)
        {
            kapp->restoreOverrideCursor();
            KMessageBox::error(this, i18n("Failed to save file '%1'")
                    .arg(m_savingContext->currentURL.filename()));
                ::unlink(QFile::encodeName(m_savingContext->tmpFile));
                finishSaving(false);
                return;
        }

        if ( m_canvas->exifRotated() )
            KExifUtils::writeOrientation(m_savingContext->tmpFile, KExifData::NORMAL);

        kdDebug() << "renaming to " << m_savingContext->currentURL.path() << endl;
        if (::rename(QFile::encodeName(m_savingContext->tmpFile),
              QFile::encodeName(m_savingContext->currentURL.path())) != 0)
        {
            kapp->restoreOverrideCursor();
            KMessageBox::error(this, i18n("Failed to overwrite original file"));
                ::unlink(QFile::encodeName(m_savingContext->tmpFile));
                finishSaving(false);
                return;
        }

        m_canvas->setModified( false );
        m_bar->invalidateThumb(m_currentItem);
        kapp->restoreOverrideCursor();
        QTimer::singleShot(0, this, SLOT(slotOpenURL(m_currentItem->url())));
    }
    else
    {
        // from saveAs()
        if (!success)
        {
            kapp->restoreOverrideCursor();
            KMessageBox::error(this, i18n("Failed to save file '%1'")
                    .arg(m_savingContext->saveAsURL.filename()));
                ::unlink(QFile::encodeName(m_savingContext->tmpFile));
                finishSaving(false);
                return;
        }

        // only try to write exif if both src and destination are jpeg files
        if (QString(QImageIO::imageFormat(m_savingContext->currentURL.path())).upper() == "JPEG" &&
            m_savingContext->format.upper() == "JPEG")
        {
            if ( m_canvas->exifRotated() )
                KExifUtils::writeOrientation(m_savingContext->tmpFile, KExifData::NORMAL);
        }

        kdDebug() << "renaming to " << m_savingContext->saveAsURL.path() << endl;
        if (::rename(QFile::encodeName(m_savingContext->tmpFile),
              QFile::encodeName(m_savingContext->saveAsURL.path())) != 0)
        {
            kapp->restoreOverrideCursor();
            KMessageBox::error(this, i18n("Failed to overwrite original file"));
                ::unlink(QFile::encodeName(m_savingContext->tmpFile));
                finishSaving(false);
                return;
        }

        m_canvas->setModified( false );

        // add the file to the list of images if it's not there already
        Digikam::ThumbBarItem* foundItem = m_bar->findItemByURL(m_savingContext->saveAsURL);
        m_bar->invalidateThumb(foundItem);

        if (!foundItem)
            foundItem = new Digikam::ThumbBarItem(m_bar, m_savingContext->saveAsURL);

        m_bar->setSelected(foundItem);
        kapp->restoreOverrideCursor();
    }

    finishSaving(true);
}

void ShowFoto::finishSaving(bool success)
{
    if (m_savingContext->progressDialog)
    {
        m_savingContext->synchronousSavingResult = success;
        m_savingContext->progressDialog->close();
    }
}

void ShowFoto::slotSavingProgress(const QString& filePath, float progress)
{
    //TODO
}

void ShowFoto::slotOpenURL(const KURL& url)
{
    if(!promptUserSave())
    {
        m_bar->blockSignals(true);
        m_bar->setSelected(m_currentItem);
        m_bar->blockSignals(false);
        return;
    }

    m_currentItem = m_bar->currentItem();
    if(!m_currentItem)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString localFile;
#if KDE_IS_VERSION(3,2,0)
    KIO::NetAccess::download(url, localFile, this);
#else
    KIO::NetAccess::download(url, localFile);
#endif
    
    if (m_ICCSettings->enableCMSetting)
    {
        kdDebug() << "enableCMSetting=true" << endl;
        m_canvas->load(localFile, m_ICCSettings, m_IOFileSettings);
    }
    else
    {
        kdDebug() << "enableCMSetting=false" << endl;
        m_canvas->load(localFile, 0, m_IOFileSettings);
    }
}

void ShowFoto::slotLoadingStarted(const QString &filename)
{
    //TODO: Disable actions as appropriate

    m_nameLabel->progressBarVisible(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);
}

void ShowFoto::slotLoadingFinished(const QString &filename, bool success, bool isReadOnly)
{
    //TODO: handle success == false
    m_nameLabel->progressBarVisible(false);
    m_isReadOnly = isReadOnly;
    slotUpdateItemInfo();
    QApplication::restoreOverrideCursor();
}

void ShowFoto::slotLoadingProgress(const QString& filePath, float progress)
{
    m_nameLabel->setProgressValue((int)(progress*100.0));
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

void ShowFoto::toggleGUI2FullScreenMode()
{
    if (m_fullScreen)
    {
        // If Hide Thumbbar option is checked.
        if (!m_showBarAction->isChecked())
            m_bar->show();
    }
    else
    {
        // If Hide Thumbbar option is checked.
        if (!m_showBarAction->isChecked())
        {
            if (m_fullScreenHideThumbBar)
                m_bar->hide();
            else
                m_fullScreenAction->plug(m_bar);
        }
    }
}

void ShowFoto::slotToggleShowBar()
{
    if (m_showBarAction->isChecked())
    {
        m_bar->hide();
    }
    else
    {
        m_bar->show();
    }
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

void ShowFoto::slotChanged(bool moreUndo, bool moreRedo)
{
    m_resLabel->setText(QString::number(m_canvas->imageWidth())  +
                        QString("x") +
                        QString::number(m_canvas->imageHeight()) +
                        QString(" ") +
                        i18n("pixels"));

    m_revertAction->setEnabled(moreUndo);
    m_undoAction->setEnabled(moreUndo);
    m_redoAction->setEnabled(moreRedo);
    m_saveAction->setEnabled(moreUndo);

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

void ShowFoto::slotSelected(bool val)
{
    // Update menu actions.
    m_cropAction->setEnabled(val);
    m_copyAction->setEnabled(val);

    QPtrList<Digikam::ImagePlugin> pluginList
        = m_imagePluginLoader->pluginList();
    for (Digikam::ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next()) 
    {
        if (plugin) 
        {
            plugin->setEnabledSelectionActions(val);
        }
    }
    
    // Update histogram.
    
    if (m_currentItem)
    {
        QRect sel = m_canvas->getSelectedArea();
        m_rightSidebar->imageSelectionChanged( sel.isNull() ? 0 : &sel);
    }
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

void ShowFoto::toggleActionsDuringSlideShow(bool val)
{
    toggleActions(val);
    
    // if slideshow mode then toogle file open actions.
    m_fileOpenAction->setEnabled(val);
    m_openFilesInFolderAction->setEnabled(val);
}

void ShowFoto::slotToggleSlideShow()
{
    if (m_slideShowAction->isChecked())
    {
        m_rightSidebar->shrink();
        m_rightSidebar->hide();
        
        if (!m_fullScreenAction->isChecked() && m_slideShowInFullScreen)
        {
            m_fullScreenAction->activate();
        }

        toggleActionsDuringSlideShow(false);
        m_slideShow->start();
    }
    else
    {
        m_slideShow->stop();
        toggleActionsDuringSlideShow(true);

        if (m_fullScreenAction->isChecked() && m_slideShowInFullScreen)
        {
            m_fullScreenAction->activate();
        }
        
        m_rightSidebar->show();
        m_rightSidebar->expand();
    }
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

void ShowFoto::slotSetup()
{
    Setup setup(this);
    
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
        m_rightSidebar->noCurrentItem();
        slotUpdateItemInfo();
        toggleActions(false);
        m_canvas->load(QString::null, 0, m_IOFileSettings);
        m_currentItem = 0;
        m_isReadOnly = false;
    }
    else
    {
        m_currentItem = m_bar->currentItem();
        QTimer::singleShot(0, this, SLOT(slotOpenURL(m_currentItem->url())));
    }
}

void ShowFoto::slotUpdateItemInfo(void)
{
    m_itemsNb = m_bar->countItems();
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
    
void ShowFoto::slotContextMenu()
{
    m_contextMenu->exec(QCursor::pos());
}

void ShowFoto::slotOpenFolder(const KURL& url)
{
    if (!promptUserSave())
        return;

    m_canvas->load(QString::null, 0, m_IOFileSettings);
    m_bar->clear(true);
    m_rightSidebar->noCurrentItem();
    m_currentItem = 0;
    m_isReadOnly = false;
    
    if (!url.isValid() || !url.isLocalFile())
       return;

    // Parse KDE image IO mime types registration to get files filter pattern.

    QStringList mimeTypes = KImageIO::mimeTypes(KImageIO::Reading);
    QString filter;

    for (QStringList::ConstIterator it = mimeTypes.begin() ; it != mimeTypes.end() ; it++)
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
    filter.append ( QString::QString(raw_file_extentions) );  
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
        new Digikam::ThumbBarItem( m_bar, KURL::KURL(fi->filePath()) );
        ++it;
    }
        
    toggleActions(true);
    toggleNavigation(1);
}
    
void ShowFoto::slotOpenFilesInFolder()
{
    if (!promptUserSave())
        return;

    KURL url(KFileDialog::getExistingDirectory(m_lastOpenedDirectory.directory(), 
                                               this, i18n("Open Images From Directory")));

    if (!url.isEmpty())
    {
       m_lastOpenedDirectory = url;
       slotOpenFolder(url);
    }
}

}   // namespace ShowFoto

#include "showfoto.moc"
