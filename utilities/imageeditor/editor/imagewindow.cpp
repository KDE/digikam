/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-02-12
 * Description : digiKam image editor GUI
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

// C++ Includes.

#include <cstdio>

// Qt includes.

#include <qpopupmenu.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qimage.h>
#include <qsplitter.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <ktempfile.h>
#include <kimageio.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kaccel.h>
#include <kaction.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kstdguiitem.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kpopupmenu.h>

// LibKexif includes.

#include <libkexif/kexifdata.h>
#include <libkexif/kexifutils.h>

// Local includes.

#include "canvas.h"
#include "dimginterface.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageresizedlg.h"
#include "imageprint.h"
#include "albummanager.h"
#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "syncjob.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "imageinfo.h"
#include "imagepropertiessidebardb.h"
#include "tagspopupmenu.h"
#include "iccsettingscontainer.h"
#include "iofilesettingscontainer.h"
#include "imagewindow.h"

namespace Digikam
{

ImageWindow* ImageWindow::imagewindow()
{
    if (!m_instance)
        new ImageWindow();

    return m_instance;
}

ImageWindow* ImageWindow::m_instance = 0;

ImageWindow::ImageWindow()
           : KMainWindow(0, 0, WType_TopLevel|WDestructiveClose)
{
    m_instance              = this;
    m_rotatedOrFlipped      = false;
    m_allowSaving           = true;
    m_fullScreen            = false;
    m_fullScreenHideToolBar = false;
    m_isReadOnly            = false;
    m_view                  = 0L;

    m_ICCSettings           = new ICCSettingsContainer();
    m_IOFileSettings        = new IOFileSettingsContainer();

    // -- construct the view ---------------------------------

    QWidget* widget  = new QWidget(this);
    QHBoxLayout *lay = new QHBoxLayout(widget);
    
    m_splitter       = new QSplitter(widget);
    m_canvas         = new Canvas(m_splitter);
    m_rightSidebar   = new ImagePropertiesSideBarDB(widget, m_splitter,
                                                    Sidebar::Right, true, false);
    
    lay->addWidget(m_splitter);
    lay->addWidget(m_rightSidebar);
    
    m_splitter->setOpaqueResize(false);
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

    buildGUI();
        
    QPtrList<ImagePlugin> pluginList = ImagePluginLoader::instance()->pluginList();

    for (ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin) 
        {
            guiFactory()->addClient(plugin);
            plugin->setParentWidget(this);
            plugin->setEnabledSelectionActions(false);
        }
    }

    m_contextMenu = dynamic_cast<QPopupMenu*>(factory()->container("RMBMenu", this));
    
    // -- Some Accels not available from actions -------------

    m_accel = new KAccel(this);
    m_accel->insert("Exit fullscreen", i18n("Exit Fullscreen"),
                    i18n("Exit out of the fullscreen mode"),
                    Key_Escape, this, SLOT(slotEscapePressed()),
                    false, true);

    // -- setup connections ---------------------------
           
    connect(m_canvas, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));
            
    connect(m_canvas, SIGNAL(signalZoomChanged(float)),
            this, SLOT(slotZoomChanged(float)));
            
    connect(m_canvas, SIGNAL(signalSelected(bool)),
            this, SLOT(slotSelected(bool)));
            
    connect(m_canvas, SIGNAL(signalChanged(bool, bool)),
            this, SLOT(slotChanged(bool, bool)));
            
    connect(m_canvas, SIGNAL(signalShowNextImage()),
            this, SLOT(slotLoadNext()));
            
    connect(m_canvas, SIGNAL(signalShowPrevImage()),
            this, SLOT(slotLoadPrev()));

    connect(m_rightSidebar, SIGNAL(signalNextItem()),
            this, SLOT(slotLoadNext()));
                
    connect(m_rightSidebar, SIGNAL(signalPrevItem()),
            this, SLOT(slotLoadPrev()));
    
    // -- read settings --------------------------------
    
    readSettings();
    applySettings();

    // This is just a bloody workaround until we have found the problem
    // which leads the imagewindow to open in a wrong size
    resize(640, 480);
    
    setAutoSaveSettings("ImageViewer Settings");    
    m_rightSidebar->populateTags();
}

ImageWindow::~ImageWindow()
{
    m_instance = 0;

    QPtrList<ImagePlugin> pluginList
        = ImagePluginLoader::instance()->pluginList();
    for (ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin) 
        {
            guiFactory()->removeClient(plugin);
            plugin->setParentWidget(0);
            plugin->setEnabledSelectionActions(false);
        }
    }
    
    delete m_canvas; 
    delete m_rightSidebar;
    delete m_ICCSettings;
    delete m_IOFileSettings;
}

void ImageWindow::closeEvent(QCloseEvent *e)
{
    if (!e) return;

    if(!promptUserSave())
        return;

    saveSettings();
    e->accept();
}

void ImageWindow::buildGUI()
{
    // -- File actions -----------------------------------------------------------
    
    m_navPrevAction = new KAction(i18n("&Previous"), "back",
                                  KStdAccel::shortcut( KStdAccel::Prior),
                                  this, SLOT(slotLoadPrev()),
                                  actionCollection(), "imageview_prev");

    m_navNextAction = new KAction(i18n("&Next"), "forward",
                                  KStdAccel::shortcut( KStdAccel::Next),
                                  this, SLOT(slotLoadNext()),
                                  actionCollection(), "imageview_next");

    m_navFirstAction = new KAction(i18n("&First"), "start",
                                   KStdAccel::shortcut( KStdAccel::Home),
                                   this, SLOT(slotLoadFirst()),
                                   actionCollection(), "imageview_first");

    m_navLastAction = new KAction(i18n("&Last"), "finish",
                                  KStdAccel::shortcut( KStdAccel::End),
                                  this, SLOT(slotLoadLast()),
                                  actionCollection(), "imageview_last");

    m_saveAction = KStdAction::save(this, SLOT(slotSave()),
                                    actionCollection(), "imageview_save");
    m_saveAsAction = KStdAction::saveAs(this, SLOT(slotSaveAs()),
                                        actionCollection(), "imageview_saveas");
    m_restoreAction = KStdAction::revert(m_canvas, SLOT(slotRestore()),
                                        actionCollection(), "imageview_restore");
    m_saveAction->setEnabled(false);
    m_restoreAction->setEnabled(false);
    
    m_fileprint = new KAction(i18n("Print Image..."), "fileprint",
                              CTRL+Key_P,
                              this, SLOT(slotFilePrint()),
                              actionCollection(), "imageview_print");

    m_fileDelete = new KAction(i18n("Delete File"), "editdelete",
                                   SHIFT+Key_Delete,
                                   this, SLOT(slotDeleteCurrentItem()),
                                   actionCollection(), "imageview_delete");

    KStdAction::quit(this, SLOT(close()),
                     actionCollection(), "imageview_exit");

    // -- Edit actions ----------------------------------------------------------------                     

    m_copyAction = KStdAction::copy(m_canvas, SLOT(slotCopy()),
                                    actionCollection(), "imageview_copy");
    m_copyAction->setEnabled(false);

    m_undoAction = new KToolBarPopupAction(i18n("Undo"), "undo", 
                                           KStdAccel::shortcut(KStdAccel::Undo),
                                           m_canvas, SLOT(slotUndo()),
                                           actionCollection(), "imageview_undo");
    connect(m_undoAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowUndoMenu()));
    connect(m_undoAction->popupMenu(), SIGNAL(activated(int)),
            m_canvas, SLOT(slotUndo(int)));
    m_undoAction->setEnabled(false);

    m_redoAction = new KToolBarPopupAction(i18n("Redo"), "redo", 
                                           KStdAccel::shortcut(KStdAccel::Redo),
                                           m_canvas, SLOT(slotRedo()),
                                           actionCollection(), "imageview_redo");
    connect(m_redoAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowRedoMenu()));
    connect(m_redoAction->popupMenu(), SIGNAL(activated(int)),
            m_canvas, SLOT(slotRedo(int)));
    m_redoAction->setEnabled(false);
           
    // -- View actions ----------------------------------------------------------------
    
    m_zoomPlusAction = new KAction(i18n("Zoom &In"), "viewmag+",
                                   CTRL+Key_Plus,
                                   m_canvas, SLOT(slotIncreaseZoom()),
                                   actionCollection(), "imageview_zoom_plus");

    m_zoomMinusAction = new KAction(i18n("Zoom &Out"), "viewmag-",
                                    CTRL+Key_Minus,
                                    m_canvas, SLOT(slotDecreaseZoom()),
                                    actionCollection(), "imageview_zoom_minus");

    m_zoomFitAction = new KToggleAction(i18n("Zoom &AutoFit"), "viewmagfit",
                                        Key_A,
                                        this, SLOT(slotToggleAutoZoom()),
                                        actionCollection(), "imageview_zoom_fit");

    m_fullScreenAction = new KToggleAction(i18n("Toggle Full Screen"),
                                           "window_fullscreen",
                                           CTRL+SHIFT+Key_F,
                                           this, SLOT(slotToggleFullScreen()),
                                           actionCollection(), "toggle_fullScreen");

    m_viewHistogramAction = new KSelectAction(i18n("&Histogram"), "histogram",
                                              Key_H,
                                              this, SLOT(slotViewHistogram()),
                                              actionCollection(), "imageview_histogram");
    m_viewHistogramAction->setEditable(false);

    QStringList selectItems;
    selectItems << i18n("Hide");
    selectItems << i18n("Luminosity");
    selectItems << i18n("Red");
    selectItems << i18n("Green");
    selectItems << i18n("Blue");
    selectItems << i18n("Alpha");
    m_viewHistogramAction->setItems(selectItems);

    // -- Transform actions ----------------------------------------------------------
    
    m_resizeAction = new KAction(i18n("&Resize..."), "resize_image", 0,
                                 this, SLOT(slotResize()),
                                 actionCollection(), "imageview_resize");

    m_cropAction = new KAction(i18n("&Crop"), "crop",
                               CTRL+Key_X,
                               m_canvas, SLOT(slotCrop()),
                               actionCollection(), "imageview_crop");
    m_cropAction->setEnabled(false);
    m_cropAction->setWhatsThis( i18n("This option can be used to crop the image. "
                                     "Select the image region to enable this action.") );

    // -- rotate actions -------------------------------------------------------------
    
    m_rotateAction = new KActionMenu(i18n("&Rotate"), "rotate_cw",
                                     actionCollection(),
                                     "imageview_rotate");
    m_rotateAction->setDelayed(false);

    m_rotate90Action  = new KAction(i18n("90 Degrees"),
                                    0, Key_1, m_canvas, SLOT(slotRotate90()),
                                    actionCollection(),
                                    "rotate_90");
    m_rotate180Action = new KAction(i18n("180 Degrees"),
                                    0, Key_2, m_canvas, SLOT(slotRotate180()),
                                    actionCollection(),
                                    "rotate_180");
    m_rotate270Action = new KAction(i18n("270 Degrees"),
                                    0, Key_3, m_canvas, SLOT(slotRotate270()),
                                    actionCollection(),
                                    "rotate_270");
    m_rotateAction->insert(m_rotate90Action);
    m_rotateAction->insert(m_rotate180Action);
    m_rotateAction->insert(m_rotate270Action);

    // -- flip actions ---------------------------------------------------------------
    
    m_flipAction = new KActionMenu(i18n("Flip"),
                                   "flip",
                                   actionCollection(),
                                   "imageview_flip");
    m_flipAction->setDelayed(false);

    m_flipHorzAction = new KAction(i18n("Horizontally"), 0,
                                   m_canvas, SLOT(slotFlipHoriz()),
                                   actionCollection(),
                                   "flip_horizontal"); 

    m_flipVertAction = new KAction(i18n("Vertically"), 0,
                                   m_canvas, SLOT(slotFlipVert()),
                                   actionCollection(),
                                   "flip_vertical");
    m_flipAction->insert(m_flipHorzAction);
    m_flipAction->insert(m_flipVertAction);

    // -- help actions ---------------------------------------------------------------
    
    m_imagePluginsHelp = new KAction(i18n("Image Plugins Handbooks"), 
                                     "digikamimageplugins", 0, 
                                     this, SLOT(slotImagePluginsHelp()),
                                     actionCollection(), "imageview_imagepluginshelp");

    // -- Configure toolbar and shortcuts ---------------------------------------------
    
    KStdAction::keyBindings(this, SLOT(slotEditKeys()),
                            actionCollection());
    KStdAction::configureToolbars(this, SLOT(slotConfToolbars()),
                                  actionCollection());

    // --- Create the gui --------------------------------------------------------------
    
    createGUI("digikamimagewindowui.rc", false);

    // -- if rotating/flipping set the rotatedflipped flag to true ---------------------

    connect(m_rotate90Action, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));
            
    connect(m_rotate180Action, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));
            
    connect(m_rotate270Action, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));
            
    connect(m_flipHorzAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));
            
    connect(m_flipVertAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));
}

void ImageWindow::loadURL(const KURL::List& urlList,
                          const KURL& urlCurrent,
                          const QString& caption,
                          bool  allowSaving,
                          AlbumIconView* view)
{
    if (!promptUserSave())
        return;
    
    setCaption(i18n("digiKam Image Editor - Album \"%1\"").arg(caption));

    m_view        = view;
    m_urlList     = urlList;
    m_urlCurrent  = urlCurrent;
    m_allowSaving = allowSaving;
    
    m_saveAction->setEnabled(false);
    m_restoreAction->setEnabled(false);
    m_undoAction->setEnabled(false);
    m_redoAction->setEnabled(false);

    QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
}

void ImageWindow::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    // Background color.
    QColor bgColor(Qt::black);
    m_canvas->setBackgroundColor(config->readColorEntry("BackgroundColor", &bgColor));
    m_canvas->update();

    // JPEG quality slider settings : 0 - 100 ==> libjpeg settings : 25 - 100.
    m_IOFileSettings->JPEGCompression  = (int)((75.0/99.0)*(float)config->readNumEntry("JPEGCompression", 75)
                                               + 25.0 - (75.0/99.0));

    // PNG compression slider settings : 1 - 9 ==> libpng settings : 100 - 1.
    m_IOFileSettings->PNGCompression   = (int)(((1.0-100.0)/8.0)*(float)config->readNumEntry("PNGCompression", 1)
                                                 + 100.0 - ((1.0-100.0)/8.0));

    m_IOFileSettings->TIFFCompression  = config->readBoolEntry("TIFFCompression", false);

    m_IOFileSettings->rawDecodingSettings.automaticColorBalance = config->readBoolEntry("AutomaticColorBalance", true);
    m_IOFileSettings->rawDecodingSettings.cameraColorBalance    = config->readBoolEntry("CameraColorBalance", true);
    m_IOFileSettings->rawDecodingSettings.RGBInterpolate4Colors = config->readBoolEntry("RGBInterpolate4Colors", false);
    m_IOFileSettings->rawDecodingSettings.enableRAWQuality      = config->readBoolEntry("EnableRAWQuality", false);
    m_IOFileSettings->rawDecodingSettings.RAWQuality            = config->readNumEntry("RAWQuality", 0);

    AlbumSettings *settings = AlbumSettings::instance();
    if (settings->getUseTrash())
    {
        m_fileDelete->setIcon("edittrash");
        m_fileDelete->setText(i18n("Move to Trash"));
    }
    else
    {
        m_fileDelete->setIcon("editdelete");
        m_fileDelete->setText(i18n("Delete File"));
    }

    m_canvas->setExifOrient(settings->getExifRotate());
    
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    if(config->hasKey("Splitter Sizes"))
        m_splitter->setSizes(config->readIntListEntry("Splitter Sizes"));
    else 
        m_canvas->setSizePolicy(rightSzPolicy);
}

void ImageWindow::readSettings()
{
    bool autoZoom = false;

    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");    

    // GUI options.
    autoZoom = config->readBoolEntry("AutoZoom", true);
    m_fullScreen = config->readBoolEntry("FullScreen", false);
    m_fullScreenHideToolBar = config->readBoolEntry("FullScreen Hide ToolBar", false);

    if (autoZoom)
    {
        m_zoomFitAction->activate();
        m_zoomPlusAction->setEnabled(false);
        m_zoomMinusAction->setEnabled(false);
    }

    if (m_fullScreen)
    {
        m_fullScreen = false;
        m_fullScreenAction->activate();
    }
    
    QRect histogramRect = config->readRectEntry("Histogram Rectangle");
    if (!histogramRect.isNull())
        m_canvas->setHistogramPosition(histogramRect.topLeft());
    
    int histogramType = config->readNumEntry("HistogramType", 0);
    histogramType = (histogramType < 0 || histogramType > 5) ? 0 : histogramType;
    m_viewHistogramAction->setCurrentItem(histogramType);
    slotViewHistogram(); // update

    // Settings for Color Management stuff
    config->setGroup("Color Management");

    m_ICCSettings->enableCMSetting = config->readBoolEntry("EnableCM");
    m_ICCSettings->askOrApplySetting = config->readBoolEntry("BehaviourICC");
    m_ICCSettings->BPCSetting = config->readBoolEntry("BPCAlgorithm");
    m_ICCSettings->renderingSetting = config->readNumEntry("RenderingIntent");
    m_ICCSettings->inputSetting = config->readPathEntry("InProfileFile");
    m_ICCSettings->workspaceSetting = config->readPathEntry("WorkProfileFile");
    m_ICCSettings->monitorSetting = config->readPathEntry("MonitorProfileFile");
    m_ICCSettings->proofSetting = config->readPathEntry("ProofProfileFile");
}

void ImageWindow::saveSettings()
{
    KConfig* config = kapp->config();
    
    config->setGroup("ImageViewer Settings");
    config->writeEntry("AutoZoom", m_zoomFitAction->isChecked());
    config->writeEntry("Splitter Sizes", m_splitter->sizes());

    int histogramType = m_viewHistogramAction->currentItem();
    histogramType = (histogramType < 0 || histogramType > 5) ? 0 : histogramType;
    config->writeEntry("HistogramType", histogramType);

    config->writeEntry("FullScreen", m_fullScreen);
    
    QPoint pt;
    QRect rc(0, 0, 0, 0);
    if (m_canvas->getHistogramPosition(pt)) 
        rc = QRect(pt.x(), pt.y(), 1, 1);
    config->writeEntry("Histogram Rectangle", rc);
    config->sync();
}

void ImageWindow::slotLoadCurrent()
{
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);
    
    if (m_view)
    {
        IconItem* item = m_view->findItem((*it).url());
        if (item)
           m_view->setCurrentItem(item);
    }

    uint index = m_urlList.findIndex(m_urlCurrent);

    if (it != m_urlList.end()) 
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        if (m_ICCSettings->enableCMSetting)
        {
            kdDebug() << "enableCMSetting=true" << endl;
            m_isReadOnly = m_canvas->load(m_urlCurrent.path(), m_ICCSettings, m_IOFileSettings, m_instance);
        }
        else
        {
            kdDebug() << "enableCMSetting=false" << endl;
            m_isReadOnly = m_canvas->load(m_urlCurrent.path(), 0, m_IOFileSettings, 0);
        }
        
        m_rotatedOrFlipped = false;

        QString text = m_urlCurrent.filename() +
                       i18n(" (%2 of %3)")
                       .arg(QString::number(index+1))
                       .arg(QString::number(m_urlList.count()));
        m_nameLabel->setText(text);

        ++it;
        if (it != m_urlList.end())
            m_canvas->preload((*it).path());

        QApplication::restoreOverrideCursor();
    }

    if (m_urlList.count() == 1) 
    {
        m_navPrevAction->setEnabled(false);
        m_navNextAction->setEnabled(false);
        m_navFirstAction->setEnabled(false);
        m_navLastAction->setEnabled(false);
    }
    else 
    {
        m_navPrevAction->setEnabled(true);
        m_navNextAction->setEnabled(true);
        m_navFirstAction->setEnabled(true);
        m_navLastAction->setEnabled(true);
    }

    if (index == 0) 
    {
        m_navPrevAction->setEnabled(false);
        m_navFirstAction->setEnabled(false);
    }

    if (index == m_urlList.count()-1) 
    {
        m_navNextAction->setEnabled(false);
        m_navLastAction->setEnabled(false);
    }

    // Disable some menu actions if the current root image URL
    // isn't include in the digiKam Albums library database.
    // This is necessary when ImageEditor is opened from cameraclient.

    KURL u(m_urlCurrent.directory());
    PAlbum *palbum = AlbumManager::instance()->findPAlbum(u);

    if (!palbum)
    {
       m_fileDelete->setEnabled(false);
    }
    else
    {
       m_fileDelete->setEnabled(true);
    }
}

void ImageWindow::slotLoadNext()
{
    if(!promptUserSave())
        return;

    KURL::List::iterator it = m_urlList.find(m_urlCurrent);

    if (it != m_urlList.end()) 
    {
        if (m_urlCurrent != m_urlList.last()) 
        {
           KURL urlNext = *(++it);
           m_urlCurrent = urlNext;
           slotLoadCurrent();
        }
    }
}

void ImageWindow::slotLoadPrev()
{
    if(!promptUserSave())
        return;

    KURL::List::iterator it = m_urlList.find(m_urlCurrent);

    if (it != m_urlList.begin()) 
    {
        if (m_urlCurrent != m_urlList.first())
        {
            KURL urlPrev = *(--it);
            m_urlCurrent = urlPrev;
            slotLoadCurrent();
        }
    }
}

void ImageWindow::slotLoadFirst()
{
    if(!promptUserSave())
        return;
    
    m_urlCurrent = m_urlList.first();
    slotLoadCurrent();
}

void ImageWindow::slotLoadLast()
{
    if(!promptUserSave())
        return;
    
    m_urlCurrent = m_urlList.last();
    slotLoadCurrent();
}

void ImageWindow::slotAboutToShowUndoMenu()
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

void ImageWindow::slotAboutToShowRedoMenu()
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

void ImageWindow::slotToggleAutoZoom()
{
    bool checked = m_zoomFitAction->isChecked();

    m_zoomPlusAction->setEnabled(!checked);
    m_zoomMinusAction->setEnabled(!checked);

    m_canvas->slotToggleAutoZoom();
}

void ImageWindow::slotViewHistogram()
{
    int curItem = m_viewHistogramAction->currentItem();
    m_canvas->setHistogramType(curItem);
}

void ImageWindow::slotResize()
{
    int width  = m_canvas->imageWidth();
    int height = m_canvas->imageHeight();

    ImageResizeDlg dlg(this, &width, &height);
    if (dlg.exec() == QDialog::Accepted &&
        (width != m_canvas->imageWidth() ||
        height != m_canvas->imageHeight()))
        m_canvas->resizeImage(width, height);
}

void ImageWindow::slotContextMenu()
{
    if (m_contextMenu)
    {
        TagsPopupMenu* assignTagsMenu = 0;
        TagsPopupMenu* removeTagsMenu = 0;
        int separatorID = -1;

        if (m_view)
        {
            IconItem* item = m_view->findItem(m_urlCurrent.url());
            if (item)
            {
                Q_LLONG id = ((AlbumIconItem*)item)->imageInfo()->id();
                QValueList<Q_LLONG> idList;
                idList.append(id);

                assignTagsMenu = new TagsPopupMenu(idList, 1000,
                                                   TagsPopupMenu::ASSIGN);
                removeTagsMenu = new TagsPopupMenu(idList, 2000,
                                                   TagsPopupMenu::REMOVE);

                separatorID = m_contextMenu->insertSeparator();

                m_contextMenu->insertItem(i18n("Assign Tag"), assignTagsMenu);
                int i = m_contextMenu->insertItem(i18n("Remove Tag"), removeTagsMenu);

                connect(assignTagsMenu, SIGNAL(signalTagActivated(int)),
                        SLOT(slotAssignTag(int)));
                connect(removeTagsMenu, SIGNAL(signalTagActivated(int)),
                        SLOT(slotRemoveTag(int)));

                AlbumDB* db = AlbumManager::instance()->albumDB();
                if (!db->hasTags( idList ))
                    m_contextMenu->setItemEnabled(i,false);
            }
        }
        
        m_contextMenu->exec(QCursor::pos());

        if (separatorID != -1)
        {
            m_contextMenu->removeItem(separatorID);
        }
        
        delete assignTagsMenu;
        delete removeTagsMenu;
    }
}

void ImageWindow::slotZoomChanged(float zoom)
{
    m_zoomLabel->setText(i18n("Zoom: ") +
                         QString::number(zoom*100, 'f', 2) +
                         QString("%"));

    m_zoomPlusAction->setEnabled(!m_canvas->maxZoom() &&
                                 !m_zoomFitAction->isChecked());
    m_zoomMinusAction->setEnabled(!m_canvas->minZoom() &&
                                  !m_zoomFitAction->isChecked());
}

void ImageWindow::slotChanged(bool moreUndo, bool moreRedo)
{
    m_resLabel->setText(QString::number(m_canvas->imageWidth())  +
                        QString("x") +
                        QString::number(m_canvas->imageHeight()) +
                        QString(" ") +
                        i18n("pixels"));

    m_restoreAction->setEnabled(moreUndo);
    m_undoAction->setEnabled(moreUndo);
    m_redoAction->setEnabled(moreRedo);

    if (m_allowSaving)
    {
        m_saveAction->setEnabled(moreUndo);
    }

    if (!moreUndo)
        m_rotatedOrFlipped = false;
        
    if (m_urlCurrent.isValid())
    {
        KURL u(m_urlCurrent.directory());
        PAlbum *palbum = AlbumManager::instance()->findPAlbum(u);
        
        QRect sel           = m_canvas->getSelectedArea();
        DImg* img           = DImgInterface::instance()->getImg();
        AlbumIconItem* item = 0;
        
        if (palbum)
           item = m_view->findItem(m_urlCurrent.url());
            
        m_rightSidebar->itemChanged(m_urlCurrent.url(), m_view, item,
                                   sel.isNull() ? 0 : &sel, img);
    }
}

void ImageWindow::slotSelected(bool val)
{
    m_cropAction->setEnabled(val);
    m_copyAction->setEnabled(val);

    ImagePluginLoader* loader = ImagePluginLoader::instance();
    for (ImagePlugin* plugin = loader->pluginList().first();
         plugin; plugin = loader->pluginList().next()) {
        if (plugin) {
            plugin->setEnabledSelectionActions(val);
        }
    }
    
    // Update histogram.
    
    QRect sel = m_canvas->getSelectedArea();
    m_rightSidebar->imageSelectionChanged( sel.isNull() ? 0 : &sel);
}

void ImageWindow::slotRotatedOrFlipped()
{
    m_rotatedOrFlipped = true;
}

void ImageWindow::slotDeleteCurrentItem()
{
    KURL u(m_urlCurrent.directory());
    PAlbum *palbum = AlbumManager::instance()->findPAlbum(u);

    if (!palbum)
        return;

    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings->getUseTrash())
    {
        QString warnMsg(i18n("About to Delete File \"%1\"\nAre you sure?")
                        .arg(m_urlCurrent.filename()));
        if (KMessageBox::warningContinueCancel(this,
                                               warnMsg,
                                               i18n("Warning"),
                                               i18n("Delete"))
            !=  KMessageBox::Continue)
        {
            return;
        }
    }

    if (!SyncJob::userDelete(m_urlCurrent))
    {
        QString errMsg(SyncJob::lastErrorMsg());
        KMessageBox::error(this, errMsg, errMsg);
        return;
    }

    emit signalFileDeleted(m_urlCurrent);

    KURL CurrentToRemove = m_urlCurrent;
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);

    if (it != m_urlList.end())
    {
        if (m_urlCurrent != m_urlList.last())
        {
            // Try to get the next image in the current Album...

            KURL urlNext = *(++it);
            m_urlCurrent = urlNext;
            m_urlList.remove(CurrentToRemove);
            slotLoadCurrent();
            return;
        }
        else if (m_urlCurrent != m_urlList.first())
        {
            // Try to get the previous image in the current Album...

            KURL urlPrev = *(--it);
            m_urlCurrent = urlPrev;
            m_urlList.remove(CurrentToRemove);
            slotLoadCurrent();
            return;
        }
    }

    // No image in the current Album -> Quit ImageEditor...

    KMessageBox::information(this,
                             i18n("There is no image to show in the current album.\n"
                                  "The image editor will be closed."),
                             i18n("No Image in Current Album"));

    close();
}

void ImageWindow::slotFilePrint()
{
    uchar* ptr      = DImgInterface::instance()->getImage();
    int w           = DImgInterface::instance()->origWidth();
    int h           = DImgInterface::instance()->origHeight();
    bool hasAlpha   = DImgInterface::instance()->hasAlpha();
    bool sixteenBit = DImgInterface::instance()->sixteenBit();

    if (!ptr || !w || !h)
        return;

    DImg image(w, h, sixteenBit, hasAlpha, ptr);

    KPrinter printer;
    printer.setDocName( m_urlCurrent.filename() );
    printer.setCreator( "digiKam-ImageEditor");
#if KDE_IS_VERSION(3,2,0)
    printer.setUsePrinterResolution(true);
#endif

    KPrinter::addDialogPage( new ImageEditorPrintDialogPage( this, "ImageEditor page"));

    if ( printer.setup( this, i18n("Print %1").arg(printer.docName().section('/', -1)) ) )
    {
        ImagePrint printOperations(image, printer, m_urlCurrent.filename());
        if (!printOperations.printImageWithQt())
        {
            KMessageBox::error(this, i18n("Failed to print file: '%1'")
                               .arg(m_urlCurrent.filename()));
        }
    }
}

bool ImageWindow::save()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    KTempFile tmpFile(m_urlCurrent.directory(false), QString::null);
    tmpFile.setAutoDelete(true);

    bool result = m_canvas->saveAsTmpFile(tmpFile.name(), m_IOFileSettings);
    
    if (!result)
    {
            kapp->restoreOverrideCursor();
            KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to album\n\"%2\".")
                                .arg(m_urlCurrent.filename())
                                .arg(m_urlCurrent.path().section('/', -2, -2)));
        return false;
    }

    if( m_rotatedOrFlipped || m_canvas->exifRotated() )
        KExifUtils::writeOrientation(tmpFile.name(), KExifData::NORMAL);

    if (::rename(QFile::encodeName(tmpFile.name()),
                 QFile::encodeName(m_urlCurrent.path())) != 0)
    {
        kapp->restoreOverrideCursor();
        KMessageBox::error(this, i18n("Failed to overwrite original file"),
                           i18n("Error Saving File"));
        return false;
    }
    
    m_canvas->setModified( false );
    emit signalFileModified(m_urlCurrent);
    QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
    
    kapp->restoreOverrideCursor();
    return true;
}

bool ImageWindow::saveAs()
{
    // FIXME : Add 16 bits file formats and others files format like TIFF not supported by kimgio.

    QString mimetypes = KImageIO::mimeTypes(KImageIO::Writing).join(" ");
    mimetypes.append(" image/tiff");
    kdDebug () << "mimetypes=" << mimetypes << endl;    

    KFileDialog imageFileSaveDialog(m_urlCurrent.directory(),
                                    QString::null,
                                    this,
                                    "imageFileSaveDialog",
                                    false);

    imageFileSaveDialog.setOperationMode( KFileDialog::Saving );
    imageFileSaveDialog.setMode( KFile::File );
    imageFileSaveDialog.setSelection(m_urlCurrent.fileName());
    imageFileSaveDialog.setCaption( i18n("New Image File Name") );
    imageFileSaveDialog.setFilter(mimetypes);

    // Check for cancel.
    if ( imageFileSaveDialog.exec() != KFileDialog::Accepted )
    {
       return false;
    }

    KURL newURL = imageFileSaveDialog.selectedURL();

    // Check if target image format have been selected from Combo List of SaveAs dialog.
    QString format = KImageIO::typeForMime(imageFileSaveDialog.currentMimeFilter());

    if ( format.isEmpty() )
    {
        // Else, check if target image format have been add to target image file name using extension.

        QFileInfo fi(newURL.path());
        format = fi.extension(false);
        
        if ( format.isEmpty() )
        {
            // If format is empty then file format is same as that of the original file.
            format = QImageIO::imageFormat(m_urlCurrent.path());
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
    
            if ( !imgExtPattern.contains( format.upper() ) )
            {
                KMessageBox::error(this, i18n("Target image file format \"%1\" unsupported.")
                        .arg(format));
                kdWarning() << k_funcinfo << "target image file format " << format << " unsupported!" << endl;
                return false;
            }
        }
    }
    
    if (!newURL.isValid())
    {
        KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to Album\n\"%2\".")
                           .arg(newURL.filename())
                           .arg(newURL.path().section('/', -2, -2)));
        kdWarning() << k_funcinfo << "target URL isn't valid !" << endl;
        return false;
    }

    // if new and original url are save use slotSave() ------------------------------
    
    KURL currURL(m_urlCurrent);
    currURL.cleanPath();
    newURL.cleanPath();

    if (currURL.equals(newURL))
    {
        slotSave();
        return false;
    }

    // Check for overwrite ----------------------------------------------------------
    
    QFileInfo fi(newURL.path());
    bool fileExists = false;
    if ( fi.exists() )
    {
        int result =

            KMessageBox::warningYesNo( this, 
                                       i18n("A file named \"%1\" already "
                                            "exists. Are you sure you want "
                                            "to overwrite it?")
                                       .arg(newURL.filename()),
                                       i18n("Overwrite File?"),
                                       i18n("Overwrite"),
                                       KStdGuiItem::cancel() );

        if (result != KMessageBox::Yes)
            return false;

        fileExists = true;
    }

    // Now do the actual saving -----------------------------------------------------

    kapp->setOverrideCursor( KCursor::waitCursor() );
    KTempFile tmpFile(newURL.directory(false), QString::null);
    tmpFile.setAutoDelete(true);

    int result = m_canvas->saveAsTmpFile(tmpFile.name(), m_IOFileSettings, format.lower());

    if (result == false)
    {
        kapp->restoreOverrideCursor();
        KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to album\n\"%2\".")
                           .arg(newURL.filename())
                           .arg(newURL.path().section('/', -2, -2)));

        return false;
    }

    // only try to write exif if both src and destination are jpeg files
    if (QString(QImageIO::imageFormat(m_urlCurrent.path())).upper() == "JPEG" &&
        format.upper() == "JPEG")
    {
        if( m_rotatedOrFlipped )
            KExifUtils::writeOrientation(tmpFile.name(), KExifData::NORMAL);
    }

    if (::rename(QFile::encodeName(tmpFile.name()),
                 QFile::encodeName(newURL.path())) != 0)
    {
        kapp->restoreOverrideCursor();
        KMessageBox::error(this, i18n("Failed to save to new file"),
                           i18n("Error Saving File"));
        return false;
    }

    // Find the src and dest albums ------------------------------------------

    KURL srcDirURL(QDir::cleanDirPath(m_urlCurrent.directory()));
    PAlbum* srcAlbum = AlbumManager::instance()->findPAlbum(srcDirURL);
    if (!srcAlbum)
    {
        kapp->restoreOverrideCursor();
        kdWarning() << k_funcinfo << "Cannot find the source album" << endl;
        return false;
    }

    KURL dstDirURL(QDir::cleanDirPath(newURL.directory()));
    PAlbum* dstAlbum = AlbumManager::instance()->findPAlbum(dstDirURL);
    if (!dstAlbum)
    {
        kapp->restoreOverrideCursor();
        kdWarning() << k_funcinfo << "Cannot find the destination album" << endl;
        return false;
    }
        
    // Now copy the metadata of the original file to the new file ------------

    AlbumDB* db = AlbumManager::instance()->albumDB();
    db->copyItem(srcAlbum->id(), m_urlCurrent.fileName(),
                 dstAlbum->id(), newURL.fileName());

    // Add new file URL into list if the new file has been added in the current Album
    
    if ( srcAlbum == dstAlbum &&                        // Target Album = current Album ?
         m_urlList.find(newURL) == m_urlList.end() )    // The image file not already exist
    {                                                   // in the list.
        KURL::List::iterator it = m_urlList.find(m_urlCurrent);
        m_urlList.insert(it, newURL);
        m_urlCurrent = newURL;
    }

    if(fileExists)
        emit signalFileModified(newURL);
    else
        emit signalFileAdded(newURL);

    m_canvas->setModified( false );
    kapp->restoreOverrideCursor();
    QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));

    return true;
}

void ImageWindow::slotToggleFullScreen()
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

        QObject* obj = child("ToolBar","KToolBar");
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            if (m_fullScreenAction->isPlugged(toolBar) && m_removeFullScreenButton)
                m_fullScreenAction->unplug(toolBar);
            if (toolBar->isHidden())
                toolBar->show();
        }

        // -- remove the imageguiclient action accels ----

        unplugActionAccel(m_navNextAction);
        unplugActionAccel(m_navPrevAction);
        unplugActionAccel(m_navFirstAction);
        unplugActionAccel(m_navLastAction);
        unplugActionAccel(m_saveAction);
        unplugActionAccel(m_saveAsAction);
        unplugActionAccel(m_zoomPlusAction);
        unplugActionAccel(m_zoomMinusAction);
        unplugActionAccel(m_zoomFitAction);
        unplugActionAccel(m_cropAction);
        unplugActionAccel(m_fileprint);
        unplugActionAccel(m_fileDelete);

        m_fullScreen = false;
    }
    else
    {
        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();

        QObject* obj = child("ToolBar","KToolBar");
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            if (m_fullScreenHideToolBar)
                toolBar->hide();
            else
            {    
                if ( !m_fullScreenAction->isPlugged(toolBar) )
                {
                    m_fullScreenAction->plug(toolBar);
                    m_removeFullScreenButton=true;
                }
                else    
                {
                    // If FullScreen button is enable in toolbar settings
                    // We don't remove it at full screen out.
                    m_removeFullScreenButton=false;
                }
            }

        }

        // -- Insert all the imageguiclient actions into the accel --

        plugActionAccel(m_navNextAction);
        plugActionAccel(m_navPrevAction);
        plugActionAccel(m_navFirstAction);
        plugActionAccel(m_navLastAction);
        plugActionAccel(m_saveAction);
        plugActionAccel(m_saveAsAction);
        plugActionAccel(m_zoomPlusAction);
        plugActionAccel(m_zoomMinusAction);
        plugActionAccel(m_zoomFitAction);
        plugActionAccel(m_cropAction);
        plugActionAccel(m_fileprint);
        plugActionAccel(m_fileDelete);

        showFullScreen();
        m_fullScreen = true;
    }
}

void ImageWindow::slotEscapePressed()
{
    if (m_fullScreen)
    {
        m_fullScreenAction->activate();
    }
}

bool ImageWindow::promptUserSave()
{
    if (m_saveAction->isEnabled())
    {
        int result = KMessageBox::warningYesNoCancel(this,
                                  i18n("The image \"%1\" has been modified.\n"
                                       "Do you want to save it?")
                                       .arg(m_urlCurrent.filename()),
                                  QString::null,
                                  KStdGuiItem::save(),
                                  KStdGuiItem::discard());

        if (result == KMessageBox::Yes)
        {
            if (m_isReadOnly)    
                return saveAs();
            else
                return save();
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

void ImageWindow::plugActionAccel(KAction* action)
{
    if (!action)
        return;

    m_accel->insert(action->text(),
                    action->text(),
                    action->whatsThis(),
                    action->shortcut(),
                    action,
                    SLOT(activate()));
}

void ImageWindow::unplugActionAccel(KAction* action)
{
    m_accel->remove(action->text());
}

void ImageWindow::slotImagePluginsHelp()
{
    KApplication::kApplication()->invokeHelp( QString::null, "digikamimageplugins" );
}

void ImageWindow::slotEditKeys()
{
    KKeyDialog dialog(true, this);
    dialog.insert( actionCollection(), i18n( "General" ) );

    ImagePluginLoader* loader = ImagePluginLoader::instance();
    for (ImagePlugin* plugin = loader->pluginList().first();
         plugin; plugin = loader->pluginList().next())
    {
        if (plugin)
        {
            dialog.insert( plugin->actionCollection(), plugin->name() );
        }
    }
    
    dialog.configure();
}

void ImageWindow::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config(), "ImageViewer Settings");
    KEditToolbar dlg(factory(), this);
    connect(&dlg, SIGNAL(newToolbarConfig()),
            SLOT(slotNewToolbarConfig()));
    dlg.exec();
}

void ImageWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config(), "ImageViewer Settings");
}

void ImageWindow::slotAssignTag(int tagID)
{
    IconItem* item = m_view->findItem(m_urlCurrent.url());
    if (item)
    {
        ((AlbumIconItem*)item)->imageInfo()->setTag(tagID);
                
    }
}

void ImageWindow::slotRemoveTag(int tagID)
{
    IconItem* item = m_view->findItem(m_urlCurrent.url());
    if (item)
    {
        ((AlbumIconItem*)item)->imageInfo()->removeTag(tagID);
                
    }
}

}  // namespace Digikam

#include "imagewindow.moc"

