/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-02-12
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

// Qt includes.

#include <qpopupmenu.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qimage.h>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
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
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kpopupmenu.h>

// LibKexif includes.

#include <libkexif/kexifdata.h>
#include <libkexif/kexifutils.h>

// Local includes.

#include "exifrestorer.h"
#include "canvas.h"
#include "imlibinterface.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageresizedlg.h"
#include "imageprint.h"
#include "albummanager.h"
#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "syncjob.h"
#include "imagewindow.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "imageproperties.h"
#include "imagedescedit.h"


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
    m_view                  = 0L;
    
    // -- construct the view ---------------------------------

    m_canvas    = new Canvas(this);
    setCentralWidget(m_canvas);

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
        
    QPtrList<Digikam::ImagePlugin> pluginList
        = ImagePluginLoader::instance()->pluginList();
    for (Digikam::ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin) {
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
    
    // -- read settings --------------------------------
    
    readSettings();
    applySettings();
}

ImageWindow::~ImageWindow()
{
    m_instance = 0;

    saveSettings();

    QPtrList<Digikam::ImagePlugin> pluginList
        = ImagePluginLoader::instance()->pluginList();
    for (Digikam::ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin) {
            guiFactory()->removeClient(plugin);
            plugin->setParentWidget(0);
            plugin->setEnabledSelectionActions(false);
        }
    }
    
    delete m_canvas; 
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

    m_fileproperties = new KAction(i18n("Properties"), "exifinfo",
                                   ALT+Key_Return,
                                   this, SLOT(slotFileProperties()),
                                   actionCollection(), "file_properties");

    m_fileproperties->setWhatsThis( i18n( "This option display the current image properties, meta-data, "
                                          "and histogram. If you have selected a region, you can choose an "
                                          "histogram rendering for the full image or the current image "
                                          "selection."));
                                   
    m_fileDelete = new KAction(i18n("Delete File"), "editdelete",
                                   SHIFT+Key_Delete,
                                   this, SLOT(slotDeleteCurrentItem()),
                                   actionCollection(), "imageview_delete");

    m_commentedit = new KAction(i18n("Edit Comments && Tags..."), "imagecomment",
                                Key_F3,
                                this, SLOT(slotCommentsEdit()),
                                actionCollection(), "comments_edit");

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
    m_canvas->setBackgroundColor(config->readColorEntry("BackgroundColor",
                                                        &bgColor));
    m_canvas->update();

    // JPEG quality value.
    // JPEG quality slider setting : 0 - 100 ==> imlib2 setting : 25 - 100.
    m_JPEGCompression = (int)((75.0/99.0)*(float)config->readNumEntry("JPEGCompression", 75)
                              + 25.0 - (75.0/99.0));

    // PNG compression value.
    // PNG compression slider setting : 1 - 9 ==> imlib2 setting : 100 - 1.
    m_PNGCompression = (int)(((1.0-100.0)/8.0)*(float)config->readNumEntry("PNGCompression", 1)
                             + 100.0 - ((1.0-100.0)/8.0));

    // TIFF compression.
    m_TIFFCompression = config->readBoolEntry("TIFFCompression", false);

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
}

void ImageWindow::readSettings()
{
    bool autoZoom = false;

    KConfig* config = kapp->config();
    applyMainWindowSettings(config, "ImageViewer Settings");
    config->setGroup("ImageViewer Settings");

    // GUI options.
    autoZoom = config->readBoolEntry("AutoZoom", true);
    
    m_fullScreen = config->readBoolEntry("FullScreen", false);
    m_fullScreenHideToolBar = config->readBoolEntry("FullScreen Hide ToolBar",
                                                    false);

    if (autoZoom) {
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
}

void ImageWindow::saveSettings()
{
    KConfig* config = kapp->config();
    saveMainWindowSettings(config, "ImageViewer Settings");
    
    config->setGroup("ImageViewer Settings");
    config->writeEntry("AutoZoom", m_zoomFitAction->isChecked());

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
        ThumbItem* item = m_view->findItem((*it).url());
        if (item)
        {
            item->setSelected(true);
            m_view->ensureItemVisible(item);
        }
    }
    
    uint index = m_urlList.findIndex(m_urlCurrent);

    if (it != m_urlList.end()) {

        QApplication::setOverrideCursor(Qt::WaitCursor);

        m_canvas->load(m_urlCurrent.path());
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

    if (m_urlList.count() == 1) {
        m_navPrevAction->setEnabled(false);
        m_navNextAction->setEnabled(false);
        m_navFirstAction->setEnabled(false);
        m_navLastAction->setEnabled(false);
    }
    else {
        m_navPrevAction->setEnabled(true);
        m_navNextAction->setEnabled(true);
        m_navFirstAction->setEnabled(true);
        m_navLastAction->setEnabled(true);
    }

    if (index == 0) {
        m_navPrevAction->setEnabled(false);
        m_navFirstAction->setEnabled(false);
    }

    if (index == m_urlList.count()-1) {
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
       m_commentedit->setEnabled(false);
    }
    else
    {
       m_fileDelete->setEnabled(true);
       m_commentedit->setEnabled(true);
    }
}

void ImageWindow::slotLoadNext()
{
    if(!promptUserSave())
        return;

    KURL::List::iterator it = m_urlList.find(m_urlCurrent);

    if (it != m_urlList.end()) {

        if (m_urlCurrent != m_urlList.last()) {
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

    if (it != m_urlList.begin()) {

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
        m_contextMenu->exec(QCursor::pos());
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
}

void ImageWindow::slotSelected(bool val)
{
    m_cropAction->setEnabled(val);
    m_copyAction->setEnabled(val);

    ImagePluginLoader* loader = ImagePluginLoader::instance();
    for (Digikam::ImagePlugin* plugin = loader->pluginList().first();
         plugin; plugin = loader->pluginList().next()) {
        if (plugin) {
            plugin->setEnabledSelectionActions(val);
        }
    }
}

void ImageWindow::slotRotatedOrFlipped()
{
    m_rotatedOrFlipped = true;
}

void ImageWindow::slotFileProperties()
{
    if (m_urlCurrent.isValid())
    {
        QRect sel = m_canvas->getSelectedArea();
        uint* data   = Digikam::ImlibInterface::instance()->getData();
        int   width  = Digikam::ImlibInterface::instance()->origWidth();
        int   height = Digikam::ImlibInterface::instance()->origHeight();
            
        ImageProperties properties(this, m_urlCurrent, 
                                   sel.isNull() ? 0 : &sel,
                                   data, width, height);
        properties.exec();
    }
}

void ImageWindow::slotCommentsEdit()
{
    if (m_view)
    {
        AlbumIconItem *iconItem = m_view->findItem(m_urlCurrent.url());

        if (iconItem)
        {
            ImageDescEdit descEdit(m_view, iconItem, this, true);
            descEdit.exec();
        }
    }
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
    uint* data   = Digikam::ImlibInterface::instance()->getData();
    int   width  = Digikam::ImlibInterface::instance()->origWidth();
    int   height = Digikam::ImlibInterface::instance()->origHeight();

    if (!data || !width || !height)
        return;

    KPrinter printer;
    printer.setDocName( m_urlCurrent.filename() );
    printer.setCreator( "digiKam-ImageEditor");
#if KDE_IS_VERSION(3,2,0)
    printer.setUsePrinterResolution(true);
#endif

    KPrinter::addDialogPage( new ImageEditorPrintDialogPage( this, "ImageEditor page"));

    if ( printer.setup( this, i18n("Print %1").arg(printer.docName().section('/', -1)) ) )
    {
        QImage image((uchar*)data, width, height, 32, 0, 0, QImage::IgnoreEndian);
        image = image.copy();
    
        ImagePrint printOperations(image, printer, m_urlCurrent.filename());
        if (!printOperations.printImageWithQt())
        {
            KMessageBox::error(this, i18n("Failed to print file: '%1'")
                               .arg(m_urlCurrent.filename()));
        }
    }
}

void ImageWindow::slotSave()
{
    save();
}

bool ImageWindow::save()
{
    QString tmpFile = locateLocal("tmp", m_urlCurrent.filename());

    bool result = m_canvas->saveAsTmpFile(tmpFile, m_JPEGCompression, 
                                          m_PNGCompression, m_TIFFCompression);
    
    if (!result)
    {
        KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to album\n\"%2\".")
                                .arg(m_urlCurrent.filename())
                                .arg(m_urlCurrent.path().section('/', -2, -2)));
        return false;
    }

    ExifRestorer exifHolder;
    exifHolder.readFile(m_urlCurrent.path(), ExifRestorer::ExifOnly);

    if (exifHolder.hasExif())
    {
        ExifRestorer restorer;
        restorer.readFile(tmpFile, ExifRestorer::EntireImage);
        restorer.insertExifData(exifHolder.exifData());
        restorer.writeFile(tmpFile);
    }
    else
        kdWarning() << ("slotSave::No Exif Data Found") << endl;

    if( m_rotatedOrFlipped || m_canvas->exifRotated() )
        KExifUtils::writeOrientation(tmpFile, KExifData::NORMAL);

    if(!SyncJob::file_move(KURL(tmpFile), m_urlCurrent))
    {
        QString errMsg(SyncJob::lastErrorMsg());
        KMessageBox::error(this, errMsg, i18n("Error Saving File"));
        return false;
    }

    m_canvas->setModified( false );
    emit signalFileModified(m_urlCurrent);
    QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
    
    return true;
}

void ImageWindow::slotSaveAs()
{
    // Get the new filename.

    // The typemines listed is the base imagefiles supported by imlib2.

    QStringList mimetypes;
    mimetypes << "image/jpeg" << "image/png" << "image/tiff" << "image/gif"
              << "image/x-tga" << "image/x-bmp" <<  "image/x-xpm"
              << "image/x-portable-anymap";

    KFileDialog *imageFileSaveDialog = new KFileDialog(m_urlCurrent.directory(),
                                                       QString::null,
                                                       this,
                                                       "imageFileSaveDialog",
                                                       false);

    imageFileSaveDialog->setOperationMode( KFileDialog::Saving );
    imageFileSaveDialog->setMode( KFile::File );
    imageFileSaveDialog->setCaption( i18n("New Image File Name") );
    imageFileSaveDialog->setMimeFilter(mimetypes);

    // Check for cancel.
    if ( imageFileSaveDialog->exec() != KFileDialog::Accepted )
    {
       delete imageFileSaveDialog;
       return;
    }

    m_newFile = imageFileSaveDialog->selectedURL();
    QString format = KImageIO::typeForMime(imageFileSaveDialog->currentMimeFilter());
    if (format.isEmpty())
    {
        // if the format is empty then file format is same as that of the
        // original file
        format = QImageIO::imageFormat(m_urlCurrent.path());
    }
    
    delete imageFileSaveDialog;

    if (!m_newFile.isValid())
    {
        KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to Album\n\"%2\".")
                           .arg(m_newFile.filename())
                           .arg(m_newFile.path().section('/', -2, -2)));
        kdWarning() << ("slotSaveAs:: target URL isn't valid !") << endl;
        return;
    }

    KURL currURL(m_urlCurrent);
    currURL.cleanPath();
    m_newFile.cleanPath();

    if (currURL.equals(m_newFile))
    {
        slotSave();
        return;
    }

    QFileInfo fi(m_newFile.path());
    if ( fi.exists() )
    {
        int result =
            KMessageBox::warningYesNo( this, i18n("About to overwrite file %1. "
                                                  "Are you sure you want to continue?")
                                       .arg(m_newFile.filename()) );

        if (result != KMessageBox::Yes)
            return;
    }

    QString tmpFile = locateLocal("tmp", m_newFile.filename());

    int result = m_canvas->saveAsTmpFile(tmpFile, m_JPEGCompression, m_PNGCompression,
                                         m_TIFFCompression, format.lower());

    if (result == false)
    {
        KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to album\n\"%2\".")
                           .arg(m_newFile.filename())
                           .arg(m_newFile.path().section('/', -2, -2)));

        return;
    }

    // only try to write exif if both src and destination are jpeg files
    if (QString(QImageIO::imageFormat(m_urlCurrent.path())).upper() == "JPEG" &&
        format.upper() == "JPEG")
    {
        ExifRestorer exifHolder;
        exifHolder.readFile(m_urlCurrent.path(), ExifRestorer::ExifOnly);

        if (exifHolder.hasExif())
        {
            ExifRestorer restorer;
            restorer.readFile(tmpFile, ExifRestorer::EntireImage);
            restorer.insertExifData(exifHolder.exifData());
            restorer.writeFile(tmpFile);
        }
        else
            kdWarning() << ("slotSaveAs::No Exif Data Found") << endl;

        if( m_rotatedOrFlipped )
            KExifUtils::writeOrientation(tmpFile, KExifData::NORMAL);
    }

    KIO::FileCopyJob* job = KIO::file_move(KURL(tmpFile), m_newFile,
                                           -1, true, false, false);

    connect(job, SIGNAL(result(KIO::Job *) ),
            this, SLOT(slotSaveAsResult(KIO::Job *)));
}

void ImageWindow::slotSaveAsResult(KIO::Job *job)
{
    if (job->error())
    {
      job->showErrorDialog(this);
      return;
    }

    // Added new file URL into list if the new file has been added in the current Album

    KURL su(m_urlCurrent.directory());
    PAlbum *sourcepAlbum = AlbumManager::instance()->findPAlbum(su);

    if (!sourcepAlbum)
    {
        kdWarning() << ("slotSaveAsResult::Cannot found the source album!") << endl;
        return;
    }

    KURL tu(m_newFile.directory());
    PAlbum *targetpAlbum = AlbumManager::instance()->findPAlbum(tu);

    if (!targetpAlbum)
    {
        kdWarning() << ("slotSaveAsResult::Cannot found the target album!") << endl;
        return;
    }

    // Copy the metadata from the original image to the target image.

    AlbumDB* db = AlbumManager::instance()->albumDB();
    db->copyItem(sourcepAlbum, m_urlCurrent.fileName(),
                 targetpAlbum, m_newFile.fileName());

    if ( sourcepAlbum == targetpAlbum &&                       // Target Album = current Album ?
         m_urlList.find(m_newFile) == m_urlList.end() )        // The image file not already exist
    {                                                          // in the list.
        KURL::List::iterator it = m_urlList.find(m_urlCurrent);
        m_urlList.insert(it, m_newFile);
        m_urlCurrent = m_newFile;
    }

    emit signalFileAdded(m_newFile);

    QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));      // Load the new target image.
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
            if (m_fullScreenAction->isPlugged(toolBar))
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
        unplugActionAccel(m_fileproperties);
        unplugActionAccel(m_fileDelete);
        unplugActionAccel(m_commentedit);

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
                m_fullScreenAction->plug(toolBar);
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
        plugActionAccel(m_fileproperties);
        plugActionAccel(m_fileDelete);
        plugActionAccel(m_commentedit);

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
        int result =
            KMessageBox::warningYesNoCancel(this,
                                            i18n("The image \"%1\" has been modified.\n"
                                                 "Do you want to save it?")
                                                 .arg(m_urlCurrent.filename()),
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

void ImageWindow::closeEvent(QCloseEvent *e)
{
    if (!e) return;

    if(!promptUserSave())
        return;
    
    e->accept();
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
    for (Digikam::ImagePlugin* plugin = loader->pluginList().first();
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

#include "imagewindow.moc"

