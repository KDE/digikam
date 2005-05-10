/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-22
 * Description : 
 * 
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
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

// C++ includes.

#include <cstdio>
#include <unistd.h> 
 
// Qt includes. 

#include <qlabel.h> 
#include <qlayout.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qtimer.h>

// KDE includes.

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
#include <kkeydialog.h>
#include <kedittoolbar.h>

// Lib KExif includes.

#include <libkexif/kexifdata.h>
#include <libkexif/kexifutils.h>

// Local includes.

#include "exifrestorer.h"
#include "canvas.h"
#include "thumbbar.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageresizedlg.h"
#include "imageprint.h"
#include "imageproperties.h"
#include "imlibinterface.h"
#include "splashscreen.h"
#include "setup.h"
#include "setupeditor.h"
#include "setupplugins.h"
#include "showfoto.h"

ShowFoto::ShowFoto(const KURL::List& urlList)
        : KMainWindow( 0, "Showfoto" )
{
    m_splash                 = 0;
    m_disableBCGActions      = false;
    m_deleteItem2Trash       = true;
    m_fullScreen             = false;
    m_fullScreenHideToolBar  = false;
    m_fullScreenHideThumbBar = true;
    m_config                 = kapp->config();
    m_config->setGroup("ImageViewer Settings");
    
    if(m_config->readBoolEntry("ShowSplash", true) &&
       !kapp->isRestored())
    {
       m_splash = new SplashScreen("showfoto-splash.png");
    }
    
    // -- construct the view ---------------------------------
    
    QWidget* widget  = new QWidget(this);
    QHBoxLayout *lay = new QHBoxLayout(widget);

    m_canvas = new Canvas(widget);
    m_bar    = new Digikam::ThumbBarView(widget);
    lay->addWidget(m_canvas);
    lay->addWidget(m_bar);

    setCentralWidget(widget);
    
    m_nameLabel = new QLabel(statusBar());
    m_nameLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_nameLabel,1);
    m_zoomLabel = new QLabel(statusBar());
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_zoomLabel,1);
    m_resLabel  = new QLabel(statusBar());
    m_resLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_resLabel,1);
    
    // -- build the gui -------------------------------------
    
    setupActions();

    KGlobal::dirs()->addResourceType("data",
                                     KGlobal::dirs()->kde_default("data")
                                     + "digikam");

    KGlobal::iconLoader()->addAppDir("digikam");

    // Load image plugins.
    
    m_imagePluginLoader = new ImagePluginLoader(this, m_splash);
    loadPlugins();
    
    // If plugin core is available, unplug BCG actions.
    if ( m_imagePluginLoader->pluginLibraryIsLoaded("digikamimageplugin_core") )   
       {
       actionCollection()->remove(m_BCGAction);
       m_disableBCGActions = true;
       }
    
    m_contextMenu = static_cast<QPopupMenu*>(factory()->container("RMBMenu", this));    
    applySettings();

    // -- setup connections ---------------------------
    
    connect(m_bar, SIGNAL(signalURLSelected(const KURL&)),
            this, SLOT(slotOpenURL(const KURL&)));
    
    connect(m_canvas, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));
                
    connect(m_canvas, SIGNAL(signalZoomChanged(float)),
            this, SLOT(slotZoomChanged(float)));
            
    connect(m_canvas, SIGNAL(signalChanged(bool, bool)),
            this, SLOT(slotChanged(bool, bool)));
            
    connect(m_canvas, SIGNAL(signalSelected(bool)),
            this, SLOT(slotSelected(bool)));

    //-------------------------------------------------------------
        
    for (KURL::List::const_iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        new Digikam::ThumbBarItem(m_bar, *it);
    }

    resize(640,480);
    setAutoSaveSettings();
    
    if ( urlList.isEmpty() )
    {
       toggleActions(false);
       QTimer::singleShot(0, this, SLOT(slotOpenFile())); 
    }
}

ShowFoto::~ShowFoto()
{
    unLoadPlugins();
    saveSettings();
    delete m_bar;
    delete m_canvas;
    delete m_imagePluginLoader;
}

void ShowFoto::setupActions()
{
    KStdAction::open(this, SLOT(slotOpenFile()),
                     actionCollection(), "open_file");
    KStdAction::quit(this, SLOT(close()),
                     actionCollection());
    
    m_forwardAction = KStdAction::forward(this, SLOT(slotNext()),
                                  actionCollection(), "file_fwd");
    m_backAction = KStdAction::back(this, SLOT(slotPrev()),
                               actionCollection(), "file_bwd");

    m_firstAction = new KAction(i18n("&First"), "start",
                                KStdAccel::shortcut( KStdAccel::Home),
                                this, SLOT(slotFirst()),
                                actionCollection(), "file_first");

    m_lastAction = new KAction(i18n("&Last"), "finish",
                               KStdAccel::shortcut( KStdAccel::End),
                               this, SLOT(slotLast()),
                               actionCollection(), "file_last");
                                  
    m_saveAction   = KStdAction::save(this, SLOT(slotSave()),
                                      actionCollection(), "save");
    
    m_saveAsAction  = KStdAction::saveAs(this, SLOT(slotSaveAs()),
                                         actionCollection(), "saveas");
    
    m_revertAction = KStdAction::revert(m_canvas, SLOT(slotRestore()),
                                        actionCollection(), "revert");

    m_revertAction->setEnabled(false);
    m_saveAction->setEnabled(false);
    m_saveAsAction->setEnabled(false);
    
    m_filePrintAction = new KAction(i18n("Print Image..."), "fileprint",
                              CTRL+Key_P,
                              this, SLOT(slotFilePrint()),
                              actionCollection(), "print");

    m_propertiesAction = new KAction(i18n("Properties"), "exifinfo",
                                     ALT+Key_Return,
                                     this, SLOT(slotFileProperties()),
                                     actionCollection(), "file_properties");
    
    m_fileDeleteAction = new KAction(i18n("Delete File"), "editdelete",
                               SHIFT+Key_Delete,
                               this, SLOT(slotDeleteCurrentItem()),
                               actionCollection(), "delete");
                                  
    // -- Edit actions ----------------------------------------------------------------                     

    m_copyAction = KStdAction::copy(m_canvas, SLOT(slotCopy()),
                                    actionCollection(), "copy");
    m_copyAction->setEnabled(false);
    
    m_undoAction = new KToolBarPopupAction(i18n("Undo"), "undo", 
                                           KStdAccel::shortcut(KStdAccel::Undo),
                                           m_canvas, SLOT(slotUndo()),
                                           actionCollection(), "undo");
    connect(m_undoAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowUndoMenu()));
    connect(m_undoAction->popupMenu(), SIGNAL(activated(int)),
            m_canvas, SLOT(slotUndo(int)));
    m_undoAction->setEnabled(false);

    m_redoAction = new KToolBarPopupAction(i18n("Redo"), "redo", 
                                           KStdAccel::shortcut(KStdAccel::Redo),
                                           m_canvas, SLOT(slotRedo()),
                                           actionCollection(), "redo");
    connect(m_redoAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowRedoMenu()));
    connect(m_redoAction->popupMenu(), SIGNAL(activated(int)),
            m_canvas, SLOT(slotRedo(int)));
    m_redoAction->setEnabled(false);
                     
    // -- View Actions -----------------------------------------------
    
    m_zoomPlusAction = KStdAction::zoomIn(m_canvas, SLOT(slotIncreaseZoom()),
                                          actionCollection(), "zoom_plus");
    m_zoomMinusAction = KStdAction::zoomOut(m_canvas, SLOT(slotDecreaseZoom()),
                                            actionCollection(), "zoom_minus");
    m_zoomFitAction = new KToggleAction(i18n("Zoom &AutoFit"), "viewmagfit",
                                        Key_A, this, SLOT(slotAutoFit()),
                                        actionCollection(), "zoom_fit");

#if KDE_IS_VERSION(3,2,0)
    m_fullScreenAction =
        KStdAction::fullScreen(this, SLOT(slotToggleFullScreen()),
                               actionCollection(), this, "full_screen");
#else 
    m_fullScreenAction = new KToggleAction(i18n("Fullscreen"), "window_fullscreen",
                                           CTRL+SHIFT+Key_F, this,
                                           SLOT(slotToggleFullScreen()),
                                           actionCollection(), "full_screen");
#endif

    m_showBarAction = new KToggleAction(i18n("Hide Thumbnails"), 0, Key_T,
                                        this, SLOT(slotToggleShowBar()),
                                        actionCollection(), "show_thumbs");

    m_viewHistogramAction = new KSelectAction(i18n("&Histogram"), 0, Key_H,
                                              this, SLOT(slotViewHistogram()),
                                              actionCollection(), "view_histogram");
    m_viewHistogramAction->setEditable(false);
    
    QStringList selectItems;
    selectItems << i18n("Hide");
    selectItems << i18n("Luminosity");
    selectItems << i18n("Red");
    selectItems << i18n("Green");
    selectItems << i18n("Blue");
    selectItems << i18n("Alpha");
    m_viewHistogramAction->setItems(selectItems);

    // -- rotate actions ---------------------------------------------
    
    m_rotateAction = new KActionMenu(i18n("&Rotate"), "rotate_cw",
                                     actionCollection(),
                                     "rotate");
    m_rotateAction->setDelayed(false);

    m_rotate90Action  = new KAction(i18n("90 Degrees"),
                                    0, Key_9, m_canvas, SLOT(slotRotate90()),
                                    actionCollection(),
                                    "rotate_90");
    m_rotate180Action = new KAction(i18n("180 Degrees"),
                                    0, Key_8, m_canvas, SLOT(slotRotate180()),
                                    actionCollection(),
                                    "rotate_180");
    m_rotate270Action = new KAction(i18n("270 Degrees"),
                                    0, Key_7, m_canvas, SLOT(slotRotate270()),
                                    actionCollection(),
                                    "rotate_270");
    m_rotateAction->insert(m_rotate90Action);
    m_rotateAction->insert(m_rotate180Action);
    m_rotateAction->insert(m_rotate270Action);
    
    // -- flip actions ---------------------------------------------------------------
    
    m_flipAction = new KActionMenu(i18n("Flip"),
                                   "flip_image",
                                   actionCollection(),
                                   "flip");
    m_flipAction->setDelayed(false);

    m_flipHorzAction = new KAction(i18n("Horizontally"), 0, Key_Asterisk,
                                   m_canvas, SLOT(slotFlipHoriz()),
                                   actionCollection(),
                                   "flip_horizontal"); 

    m_flipVertAction = new KAction(i18n("Vertically"), 0, Key_Slash,
                                   m_canvas, SLOT(slotFlipVert()),
                                   actionCollection(),
                                   "flip_vertical");
    m_flipAction->insert(m_flipHorzAction);
    m_flipAction->insert(m_flipVertAction);
    
    // -- Transform actions ----------------------------------------------------------
    
    m_resizeAction = new KAction(i18n("&Resize..."), "resize_image", 0,
                                 this, SLOT(slotResize()),
                                 actionCollection(), "resize");

    m_cropAction = new KAction(i18n("Crop"), "crop",
                               CTRL+Key_C,
                               m_canvas, SLOT(slotCrop()),
                               actionCollection(), "crop");
    m_cropAction->setEnabled(false);

    // -- BCG actions ----------------------------------------------------------------

    m_BCGAction = new KActionMenu(i18n("Brightness/Contrast/Gamma"),
                                  0, actionCollection(), "bcg");
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
                
    // -- help actions -----------------------------------------------
    
    m_imagePluginsHelpAction = new KAction(i18n("Image Plugins Handbooks"), 
                                     "digikamimageplugins", 0, 
                                     this, SLOT(slotImagePluginsHelp()),
                                     actionCollection(), "imagepluginshelp");
    
    // -- Configure toolbar and shortcuts ---------------------------------------------
    
    KStdAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStdAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStdAction::preferences(this, SLOT(slotSetup()),              actionCollection());
    
    // ---------------------------------------------------------------
    
    createGUI("showfotoui.rc", false);

    KAccel *accel = new KAccel(this);
    accel->insert("Exit fullscreen", i18n("Exit Fullscreen"),
                  i18n("Exit out of the fullscreen mode"),
                  Key_Escape, this, SLOT(slotEscapePressed()),
                  false, true);
    accel->insert("Next Image Key_Space", i18n("Next Image"),
                  i18n("Load Next Image"),
                  Key_Space, this, SLOT(slotNext()),
                  false, true);
    accel->insert("Previous Image Key_Backspace", i18n("Previous Image"),
                  i18n("Load Previous Image"),
                  Key_Backspace, this, SLOT(slotPrev()),
                  false, true);
    accel->insert("Next Image Key_Next", i18n("Next Image"),
                  i18n("Load Next Image"),
                  Key_Next, this, SLOT(slotNext()),
                  false, true);
    accel->insert("Previous Image Key_Prior", i18n("Previous Image"),
                  i18n("Load Previous Image"),
                  Key_Prior, this, SLOT(slotPrev()),
                  false, true);
    accel->insert("Zoom Plus Key_Plus", i18n("Zoom In"),
                  i18n("Zoom into Image"),
                  Key_Plus, m_canvas, SLOT(slotIncreaseZoom()),
                  false, true);
    accel->insert("Zoom Plus Key_Minus", i18n("Zoom Out"),
                  i18n("Zoom out of Image"),
                  Key_Minus, m_canvas, SLOT(slotDecreaseZoom()),
                  false, true);
}

void ShowFoto::applySettings()
{
    m_config->setGroup("ImageViewer Settings");

    // Current image deleted go to trash ?
    m_deleteItem2Trash = m_config->readBoolEntry("DeleteItem2Trash", true);
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
    
    // Background color.
    QColor bgColor(Qt::black);
    m_canvas->setBackgroundColor(m_config->readColorEntry("BackgroundColor", &bgColor));
    m_canvas->update();
    
    m_fullScreenHideToolBar = m_config->readBoolEntry("FullScreenHideToolBar", false);
    m_fullScreenHideThumbBar = m_config->readBoolEntry("FullScreenHideThumbBar", true);

    // JPEG quality value.
    // JPEG quality slider setting : 0 - 100 ==> imlib2 setting : 25 - 100.
    m_JPEGCompression = (int)((75.0/99.0)*(float)m_config->readNumEntry("JPEGCompression", 75)
                              + 25.0 - (75.0/99.0));

    // PNG compression value.
    // PNG compression slider setting : 1 - 9 ==> imlib2 setting : 100 - 1.
    m_PNGCompression = (int)(((1.0-100.0)/8.0)*(float)m_config->readNumEntry("PNGCompression", 1)
                             + 100.0 - ((1.0-100.0)/8.0));

    // TIFF compression.
    m_TIFFCompression = m_config->readBoolEntry("TIFFCompression", false);
    
    bool showBar = false;
    bool autoFit = true;
    
    m_config->setGroup("MainWindow");
    showBar = m_config->readBoolEntry("Show Thumbnails", true);
    autoFit = m_config->readBoolEntry("Zoom Autofit", true);
        
    if (!showBar && m_showBarAction->isChecked())
        m_showBarAction->activate();

    if (autoFit && !m_zoomFitAction->isChecked())
        m_zoomFitAction->activate();        

    QRect histogramRect = m_config->readRectEntry("Histogram Rectangle");
    if (!histogramRect.isNull())
        m_canvas->setHistogramPosition(histogramRect.topLeft());
    
    int histogramType = m_config->readNumEntry("HistogramType", 0);
    histogramType = (histogramType < 0 || histogramType > 5) ? 0 : histogramType;
    m_viewHistogramAction->setCurrentItem(histogramType);
    slotViewHistogram(); // update
}

void ShowFoto::saveSettings()
{
    m_config->setGroup("MainWindow");
    m_config->writeEntry("Show Thumbnails", !m_showBarAction->isChecked());
    m_config->writeEntry("Zoom Autofit", m_zoomFitAction->isChecked());
    
    int histogramType = m_viewHistogramAction->currentItem();
    histogramType = (histogramType < 0 || histogramType > 5) ? 0 : histogramType;
    m_config->writeEntry("HistogramType", histogramType);

    QPoint pt;
    QRect rc(0, 0, 0, 0);
    if (m_canvas->getHistogramPosition(pt)) 
        rc = QRect(pt.x(), pt.y(), 1, 1);
    m_config->writeEntry("Histogram Rectangle", rc);
}

void ShowFoto::closeEvent(QCloseEvent* e)
{
    if (!e)
        return;

    if (!promptUserSave())
        return;
    
    e->accept();
}

void ShowFoto::slotOpenFile()
{
    if (!promptUserSave())
        return;
    
    QString mimes = KImageIO::mimeTypes(KImageIO::Reading).join(" ");
    KURL::List urls =  KFileDialog::getOpenURLs(QString::null,
                                                mimes,
                                                this,
                                                i18n("Open Images"));
        
    if (!urls.isEmpty())
    {
        m_bar->clear();
        for (KURL::List::const_iterator it = urls.begin();
             it != urls.end(); ++it)
        {
            new Digikam::ThumbBarItem(m_bar, *it);
        }
           
        toggleActions(true);
    }
}

void ShowFoto::slotFileProperties()
{
    Digikam::ThumbBarItem* curr = m_bar->currentItem();
    
    if (curr)
    {
        if (curr->url().isValid())
        {
            QRect sel = m_canvas->getSelectedArea();
            uint* data   = Digikam::ImlibInterface::instance()->getData();
            int   width  = Digikam::ImlibInterface::instance()->origWidth();
            int   height = Digikam::ImlibInterface::instance()->origHeight();
                
            ImageProperties properties(this, curr->url(), 
                                    sel.isNull() ? 0 : &sel,
                                    data, width, height);
            properties.exec();
        }
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

void ShowFoto::slotNext()
{
    if (!promptUserSave())
        return;

    Digikam::ThumbBarItem* curr = m_bar->currentItem();
    if (curr && curr->next())
    {
        m_bar->setSelected(curr->next());
    }
}

void ShowFoto::slotPrev()
{
    if (!promptUserSave())
        return;

    Digikam::ThumbBarItem* curr = m_bar->currentItem();
    if (curr && curr->prev())
    {
        m_bar->setSelected(curr->prev());
    }
}

void ShowFoto::slotSave()
{
    save();
}

void ShowFoto::slotSaveAs()
{
    Digikam::ThumbBarItem* curr = m_bar->currentItem();
    if (!curr)
    {
        kdWarning() << k_funcinfo << "This should not happen" << endl;
        return;
    }
    
    KURL url(curr->url());
    
    // The typemines listed is the base imagefiles supported by imlib2.
    QStringList mimetypes;
    mimetypes << "image/jpeg" << "image/png" << "image/tiff" << "image/gif"
              << "image/x-tga" << "image/x-bmp" <<  "image/x-xpm"
              << "image/x-portable-anymap";

    KFileDialog saveDialog(url.isLocalFile() ? url.directory() : QDir::homeDirPath(),
                           QString::null,
                           this,
                           "imageFileSaveDialog",
                           false);
    saveDialog.setOperationMode( KFileDialog::Saving );
    saveDialog.setMode( KFile::File );
    saveDialog.setCaption( i18n("New Image File Name") );
    saveDialog.setMimeFilter(mimetypes);

    if ( saveDialog.exec() != KFileDialog::Accepted )
    {
       return;
    }

    KURL saveAsURL = saveDialog.selectedURL();
    QString format = KImageIO::typeForMime(saveDialog.currentMimeFilter());

    if (!saveAsURL.isValid())
    {
        KMessageBox::error(this, i18n("Invalid target selected"));
        return;
    }

    url.cleanPath();
    saveAsURL.cleanPath();

    if (url.equals(saveAsURL))
    {
        slotSave();
        return;
    }

    QFileInfo fi(saveAsURL.path());
    if ( fi.exists() )
    {
        int result =
            KMessageBox::warningYesNo( this, i18n("About to overwrite file %1. "
                                                  "Are you sure you want to continue?")
                                       .arg(saveAsURL.filename()) );

        if (result != KMessageBox::Yes)
            return;
    }

    QString tmpFile = saveAsURL.directory() + QString("/.showfoto-tmp-")
                      + saveAsURL.filename();
    if (!m_canvas->saveAsTmpFile(tmpFile, m_JPEGCompression, m_PNGCompression, 
                                 m_TIFFCompression, format.lower()))
    {
        KMessageBox::error(this, i18n("Failed to save file '%1'")
                           .arg(saveAsURL.filename()));
        ::unlink(QFile::encodeName(tmpFile));
        return;
    }

    // only try to write exif if both src and destination are jpeg files
    if (QString(QImageIO::imageFormat(url.path())).upper() == "JPEG" &&
        format.upper() == "JPEG")
    {
        ExifRestorer exifHolder;
        exifHolder.readFile(url.path(), ExifRestorer::ExifOnly);

        if (exifHolder.hasExif())
        {
            ExifRestorer restorer;
            restorer.readFile(tmpFile, ExifRestorer::EntireImage);
            restorer.insertExifData(exifHolder.exifData());
            restorer.writeFile(tmpFile);
            KExifUtils::writeOrientation(tmpFile, KExifData::NORMAL);
        }
        else
            kdDebug() << ("slotSaveAs::No Exif Data Found") << endl;
    }

    kdDebug() << "renaming to " << saveAsURL.path() << endl;
    if (::rename(QFile::encodeName(tmpFile),
                 QFile::encodeName(saveAsURL.path())) != 0)
    {
        KMessageBox::error(this, i18n("Failed to overwrite original file"));
        ::unlink(QFile::encodeName(tmpFile));
        return;
    }

    // add the file to the list of images if it's not there already
    Digikam::ThumbBarItem* foundItem = 0;
    for (Digikam::ThumbBarItem *item = m_bar->firstItem(); item; item = item->next())
    {
        if (item->url().equals(saveAsURL))
        {
            foundItem = item;
            m_bar->invalidateThumb(item);
            break;
        }
    }

    if (!foundItem)
    {
        foundItem = new Digikam::ThumbBarItem(m_bar, saveAsURL);
    }

    m_bar->setSelected(foundItem);
}

bool ShowFoto::promptUserSave()
{
    Digikam::ThumbBarItem* curr = m_bar->currentItem();
    if (!curr)
        return true;
    
    if (m_saveAction->isEnabled())
    {
        int result =
            KMessageBox::warningYesNoCancel(this,
                                            i18n("The image '%1\' has been modified.\n"
                                                 "Do you want to save it?")
                                                 .arg(curr->url().filename()),
                                            QString::null,
                                            KStdGuiItem::save(),
                                            KStdGuiItem::discard());
        if (result == KMessageBox::Yes)
        {
            return save();
        }
        else if (result == KMessageBox::No)
        {
            return true;
        }
        else
            return false;
    }
    return true;
}

bool ShowFoto::save()
{
    Digikam::ThumbBarItem* curr = m_bar->currentItem();
    if (!curr)
    {
        kdWarning() << k_funcinfo << "This should not happen" << endl;
        return true;
    }

    KURL url = curr->url();
    if (!url.isLocalFile())
    {
        KMessageBox::sorry(this, i18n("No support for saving non-local files"));
        return false;
    }
    
    QString tmpFile = url.directory() + QString("/.showfoto-tmp-")
                      + url.filename();
    if (!m_canvas->saveAsTmpFile(tmpFile, m_JPEGCompression, m_PNGCompression,
                                 m_TIFFCompression))
    {
        KMessageBox::error(this, i18n("Failed to save file '%1'")
                           .arg(url.filename()));
        ::unlink(QFile::encodeName(tmpFile));
        return false;
    }

    ExifRestorer exifHolder;
    exifHolder.readFile(url.path(), ExifRestorer::ExifOnly);
    if (exifHolder.hasExif())
    {
        ExifRestorer restorer;
        restorer.readFile(tmpFile, ExifRestorer::EntireImage);
        restorer.insertExifData(exifHolder.exifData());
        restorer.writeFile(tmpFile);
    }
    else
        kdDebug() << ("slotSave::No Exif Data Found") << endl;

    if ( m_canvas->exifRotated() )
        KExifUtils::writeOrientation(tmpFile, KExifData::NORMAL);

    kdDebug() << "renaming to " << url.path() << endl;
    if (::rename(QFile::encodeName(tmpFile),
                 QFile::encodeName(url.path())) != 0)
    {
        KMessageBox::error(this, i18n("Failed to overwrite original file"));
        ::unlink(QFile::encodeName(tmpFile));
        return false;
    }

    m_canvas->setModified( false );
    slotOpenURL(curr->url());
    m_bar->invalidateThumb(curr);
    
    return true;
}

void ShowFoto::slotOpenURL(const KURL& url)
{
    if(!promptUserSave())
        return;

    QString localFile;
#if KDE_IS_VERSION(3,2,0)
    KIO::NetAccess::download(url, localFile, this);
#else
    KIO::NetAccess::download(url, localFile);
#endif
    m_canvas->load(localFile);
    
    int index = 1;
    for (Digikam::ThumbBarItem *item = m_bar->firstItem(); item; item = item->next())
    {
        if (item->url().equals(m_bar->currentItem()->url()))
            {
            break;
            }
    index++;
    }
    
    QString text = m_bar->currentItem()->url().filename() +
                   i18n(" (%2 of %3)")
                   .arg(QString::number(index))
                   .arg(QString::number(m_bar->countItems()));
    m_nameLabel->setText(text);
    
    setCaption(i18n("Showfoto - %1").arg(m_bar->currentItem()->url().directory()));

    if (m_bar->countItems() == 1) {
        m_backAction->setEnabled(false);
        m_forwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }
    else {
        m_backAction->setEnabled(true);
        m_forwardAction->setEnabled(true);
        m_firstAction->setEnabled(true);
        m_lastAction->setEnabled(true);
    }

    if (index == 1) {
        m_backAction->setEnabled(false);
        m_firstAction->setEnabled(false);
    }

    if (index == m_bar->countItems()) {
        m_forwardAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }
}

void ShowFoto::slotAutoFit()
{
    bool checked = m_zoomFitAction->isChecked();

    m_zoomPlusAction->setEnabled(!checked);
    m_zoomMinusAction->setEnabled(!checked);

    m_canvas->slotToggleAutoZoom();
}

void ShowFoto::slotToggleFullScreen()
{
    if (m_fullScreen)
    {

#if QT_VERSION >= 0x030300
        setWindowState( windowState() & ~WindowFullScreen );
#else
        showNormal();
#endif
        menuBar()->show();
        statusBar()->show();

        
        // If Hide Thumbbar option is checked.
        if (!m_showBarAction->isChecked())
           m_bar->show();

        QObject* obj = child("mainToolBar","KToolBar");
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            toolBar->show();
        }
    
        m_fullScreen = false;
    }
    else
    {
        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();

        // If Hide Thumbbar option is checked.
        if (!m_showBarAction->isChecked())
        {
            if (m_fullScreenHideThumbBar)
               m_bar->hide();
            else
               m_fullScreenAction->plug(m_bar);
        }

        QObject* obj = child("mainToolBar","KToolBar");
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            if (m_fullScreenHideToolBar)
                toolBar->hide();
            else
                m_fullScreenAction->plug(toolBar);
        }

        showFullScreen();
        m_fullScreen = true;
    }
}

void ShowFoto::slotEscapePressed()
{
    if (!m_fullScreen)
        return;

    m_fullScreenAction->activate();
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

void ShowFoto::slotViewHistogram()
{
    int curItem = m_viewHistogramAction->currentItem();
    m_canvas->setHistogramType(curItem);
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

void ShowFoto::slotImagePluginsHelp()
{
    KApplication::kApplication()->invokeHelp( QString::null, "digikamimageplugins" );
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
}

void ShowFoto::slotAboutToShowUndoMenu()
{
    m_undoAction->popupMenu()->clear();
    QStringList titles;
    m_canvas->getUndoHistory(titles);
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();        
        for(; iter != titles.end(); ++iter,++id)
        {
            m_undoAction->popupMenu()->insertItem(*iter, id);
        }        
    }
}

void ShowFoto::slotAboutToShowRedoMenu()
{
    m_redoAction->popupMenu()->clear();
    QStringList titles;
    m_canvas->getRedoHistory(titles);
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();        
        for(; iter != titles.end(); ++iter,++id)
        {
            m_redoAction->popupMenu()->insertItem(*iter, id);
        }        
    }
}

void ShowFoto::slotSelected(bool val)
{
    m_cropAction->setEnabled(val);
    m_copyAction->setEnabled(val);

    QPtrList<Digikam::ImagePlugin> pluginList
        = m_imagePluginLoader->pluginList();
    for (Digikam::ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next()) {
        if (plugin) {
            plugin->setEnabledSelectionActions(val);
        }
    }
}

void ShowFoto::toggleActions(bool val)
{
    m_zoomFitAction->setEnabled(val);
    m_saveAsAction->setEnabled(val);
    m_propertiesAction->setEnabled(val);
    m_viewHistogramAction->setEnabled(val);
    m_rotateAction->setEnabled(val);
    m_flipAction->setEnabled(val);
    m_filePrintAction->setEnabled(val);
    m_resizeAction->setEnabled(val);
    m_fileDeleteAction->setEnabled(val);
    
    if (!m_disableBCGActions)
       m_BCGAction->setEnabled(val);
    
    QPtrList<Digikam::ImagePlugin> pluginList
        = m_imagePluginLoader->pluginList();
    for (Digikam::ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next()) 
    {
        if (plugin) {
            plugin->setEnabledActions(val);
        }
    }
}

void ShowFoto::slotEditKeys()
{
    KKeyDialog dialog(true, this);
    dialog.insert( actionCollection(), i18n( "General" ) );

    QPtrList<Digikam::ImagePlugin> pluginList
        = m_imagePluginLoader->pluginList();
    for (Digikam::ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next()) 
    {
        if (plugin)
        {
            dialog.insert( plugin->actionCollection(), plugin->name() );
        }
    }
    
    dialog.configure();
}

void ShowFoto::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config(), "ImageViewer Settings");
    KEditToolbar dlg(factory(), this);
    connect(&dlg, SIGNAL(newToolbarConfig()),
            SLOT(slotNewToolbarConfig()));
    dlg.exec();
}

void ShowFoto::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config(), "ImageViewer Settings");
}

void ShowFoto::slotFilePrint()
{
    uint* data   = Digikam::ImlibInterface::instance()->getData();
    int   width  = Digikam::ImlibInterface::instance()->origWidth();
    int   height = Digikam::ImlibInterface::instance()->origHeight();

    if (!data || !width || !height)
        return;

    KPrinter printer;
    printer.setDocName( m_bar->currentItem()->url().filename() );
    printer.setCreator( "ShowFoto");
#if KDE_IS_VERSION(3,2,0)
    printer.setUsePrinterResolution(true);
#endif

    KPrinter::addDialogPage( new ImageEditorPrintDialogPage( this, "ShowFoto page"));

    if ( printer.setup( this, i18n("Print %1").arg(printer.docName().section('/', -1)) ) )
    {
        QImage image((uchar*)data, width, height, 32, 0, 0, QImage::IgnoreEndian);
        image = image.copy();
    
        ImagePrint printOperations(image, printer, m_bar->currentItem()->url().filename());
        if (!printOperations.printImageWithQt())
        {
            KMessageBox::error(this, i18n("Failed to print file: '%1'")
                               .arg(m_bar->currentItem()->url().filename()));
        }
    }
}

void ShowFoto::slotResize()
{
    int width  = m_canvas->imageWidth();
    int height = m_canvas->imageHeight();

    ImageResizeDlg dlg(this, &width, &height);
    if (dlg.exec() == QDialog::Accepted &&
        (width != m_canvas->imageWidth() ||
        height != m_canvas->imageHeight()))
        m_canvas->resizeImage(width, height);
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

void ShowFoto::slotZoomChanged(float zoom)
{
    m_zoomLabel->setText(i18n("Zoom: ") +
                         QString::number(zoom*100, 'f', 2) +
                         QString("%"));

    m_zoomPlusAction->setEnabled(!m_canvas->maxZoom() &&
                                 !m_zoomFitAction->isChecked());
    m_zoomMinusAction->setEnabled(!m_canvas->minZoom() &&
                                  !m_zoomFitAction->isChecked());
}

void ShowFoto::slotSetup()
{
    Setup setup(this);

    if (setup.exec() != QDialog::Accepted)
        return;

    m_imagePluginLoader->loadPluginsFromList(setup.pluginsPage_->getImagePluginsListEnable());
    m_config->sync();
    unLoadPlugins();
    loadPlugins();
    applySettings();

    if ( m_bar->countItems() == 0 )    
       toggleActions(false);
}

void ShowFoto::loadPlugins()
{
    QPtrList<Digikam::ImagePlugin> pluginList
        = m_imagePluginLoader->pluginList();
    for (Digikam::ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next()) 
    {
        if (plugin)
        {
            guiFactory()->addClient(plugin);
            plugin->setParentWidget(this);
            plugin->setEnabledSelectionActions(false);
        }
        else
            kdDebug() << "Invalid plugin to add!" << endl;
    }
}

void ShowFoto::unLoadPlugins()
{
    QPtrList<Digikam::ImagePlugin> pluginList
        = m_imagePluginLoader->pluginList();
    for (Digikam::ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin) {
            guiFactory()->removeClient(plugin);
            plugin->setParentWidget(0);
            plugin->setEnabledSelectionActions(false);
        }
    }
}

void ShowFoto::slotDeleteCurrentItem()
{
    KURL urlCurrent(m_bar->currentItem()->url());

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
    
    Digikam::ThumbBarItem *item2remove = m_bar->currentItem();
    
    for (Digikam::ThumbBarItem *item = m_bar->firstItem(); item; item = item->next())
     {
        if (item->url().equals(item2remove->url()))
        {
            m_bar->removeItem(item);
            break;
        }
    }
  
    // Disable menu actions if no current image.
    if ( m_bar->countItems() == 0 )    
       {
       toggleActions(false);
       m_canvas->load(QString::null);
       }
    else 
       slotOpenURL(m_bar->currentItem()->url());
}

void ShowFoto::slotContextMenu()
{
    m_contextMenu->exec(QCursor::pos());
}

#include "showfoto.moc"
