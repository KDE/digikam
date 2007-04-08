/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2006-01-20
 * Description : main image editor GUI implementation
 *
 * Copyright 2006-2007 by Gilles Caulier
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
#include <sys/types.h>
#include <sys/stat.h>
}

// C++ includes.

#include <cmath>

// Qt includes.

#include <qlabel.h>
#include <qdockarea.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qtoolbutton.h>
#include <qsplitter.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qfileinfo.h>

// KDE includes.

#include <kprinter.h>
#include <kkeydialog.h>
#include <kdeversion.h>
#include <kaction.h>
#include <kedittoolbar.h>
#include <kaboutdata.h>
#include <kcursor.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kimageio.h>
#include <kaccel.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kprotocolinfo.h>
#include <kglobalsettings.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kprogress.h>
#include <kwin.h>
#include <kcombobox.h>

// Local includes.

#include "ddebug.h"
#include "dpopupmenu.h"
#include "canvas.h"
#include "dimginterface.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageresize.h"
#include "imageprint.h"
#include "filesaveoptionsbox.h"
#include "statusprogressbar.h"
#include "iccsettingscontainer.h"
#include "exposurecontainer.h"
#include "iofilesettingscontainer.h"
#include "savingcontextcontainer.h"
#include "loadingcacheinterface.h"
#include "slideshowsettings.h"
#include "editorwindowprivate.h"
#include "editorwindow.h"
#include "editorwindow.moc"

void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );

namespace Digikam
{

EditorWindow::EditorWindow(const char *name)
            : KMainWindow(0, name, WType_TopLevel)
{
    d = new EditorWindowPriv;

    m_contextMenu            = 0;
    m_canvas                 = 0;
    m_imagePluginLoader      = 0;
    m_undoAction             = 0;
    m_redoAction             = 0;
    m_fullScreenAction       = 0;
    m_saveAction             = 0;
    m_saveAsAction           = 0;
    m_revertAction           = 0;
    m_fileDeleteAction       = 0;
    m_forwardAction          = 0;
    m_backwardAction         = 0;
    m_firstAction            = 0;
    m_lastAction             = 0;
    m_undoAction             = 0;
    m_redoAction             = 0;
    m_fullScreen             = false;
    m_rotatedOrFlipped       = false;
    m_setExifOrientationTag  = true;
    m_cancelSlideShow        = false;
    
    // Settings containers instance.

    d->ICCSettings      = new ICCSettingsContainer();
    d->exposureSettings = new ExposureSettingsContainer();
    m_IOFileSettings    = new IOFileSettingsContainer();
    m_savingContext     = new SavingContextContainer();
}

EditorWindow::~EditorWindow()
{
    delete m_canvas;
    delete m_IOFileSettings;
    delete m_savingContext;
    delete d->ICCSettings;
    delete d->exposureSettings;
    delete d;
}

void EditorWindow::setupContextMenu()
{
    m_contextMenu         = new DPopupMenu(this);
    KActionCollection *ac = actionCollection();
    if( ac->action("editorwindow_backward") ) ac->action("editorwindow_backward")->plug(m_contextMenu);
    if( ac->action("editorwindow_forward") ) ac->action("editorwindow_forward")->plug(m_contextMenu);
    m_contextMenu->insertSeparator();
    if( ac->action("editorwindow_slideshow") ) ac->action("editorwindow_slideshow")->plug(m_contextMenu);
    if( ac->action("editorwindow_rotate_left") ) ac->action("editorwindow_rotate_left")->plug(m_contextMenu);
    if( ac->action("editorwindow_rotate_right") ) ac->action("editorwindow_rotate_right")->plug(m_contextMenu);
    if( ac->action("editorwindow_crop") ) ac->action("editorwindow_crop")->plug(m_contextMenu);
    m_contextMenu->insertSeparator();
    if( ac->action("editorwindow_delete") ) ac->action("editorwindow_delete")->plug(m_contextMenu);
}

void EditorWindow::setupStandardConnections()
{
    // -- Canvas connections ------------------------------------------------

    connect(m_canvas, SIGNAL(signalShowNextImage()),
            this, SLOT(slotForward()));
            
    connect(m_canvas, SIGNAL(signalShowPrevImage()),
            this, SLOT(slotBackward()));

    connect(m_canvas, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));
            
    connect(m_canvas, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));
            
    connect(m_canvas, SIGNAL(signalChanged()),
            this, SLOT(slotChanged()));

    connect(m_canvas, SIGNAL(signalUndoStateChanged(bool, bool, bool)),
            this, SLOT(slotUndoStateChanged(bool, bool, bool)));

    connect(m_canvas, SIGNAL(signalSelected(bool)),
            this, SLOT(slotSelected(bool)));
    
    connect(m_canvas, SIGNAL(signalLoadingStarted(const QString &)),
            this, SLOT(slotLoadingStarted(const QString &)));

    connect(m_canvas, SIGNAL(signalLoadingFinished(const QString &, bool)),
            this, SLOT(slotLoadingFinished(const QString &, bool)));

    connect(m_canvas, SIGNAL(signalLoadingProgress(const QString &, float)),
            this, SLOT(slotLoadingProgress(const QString &, float)));

    connect(m_canvas, SIGNAL(signalSavingStarted(const QString&)),
            this, SLOT(slotSavingStarted(const QString&)));

    connect(m_canvas, SIGNAL(signalSavingFinished(const QString&, bool)),
            this, SLOT(slotSavingFinished(const QString&, bool)));

    connect(m_canvas, SIGNAL(signalSavingProgress(const QString&, float)),
            this, SLOT(slotSavingProgress(const QString&, float)));

    connect(m_canvas, SIGNAL(signalSelectionChanged(const QRect&)),
            this, SLOT(slotSelectionChanged(const QRect&)));

    // -- if rotating/flipping set the rotatedflipped flag to true -----------

    connect(d->rotateLeftAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));

    connect(d->rotateRightAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));
            
    connect(d->flipHorizAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));
            
    connect(d->flipVertAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));

    // -- status bar connections --------------------------------------

    connect(m_nameLabel, SIGNAL(signalCancelButtonPressed()),
            this, SLOT(slotNameLabelCancelButtonPressed()));

    // -- Core plugin connections -------------------------------------
    
    ImagePlugin *corePlugin = m_imagePluginLoader->pluginInstance("digikamimageplugin_core");
    if ( corePlugin )
    {                        
        connect(m_canvas, SIGNAL(signalColorManagementTool()),
                corePlugin, SLOT(slotColorManagement()));    
    }        
}

void EditorWindow::setupStandardActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    m_backwardAction = KStdAction::back(this, SLOT(slotBackward()),
                                   actionCollection(), "editorwindow_backward");

    m_forwardAction = KStdAction::forward(this, SLOT(slotForward()),
                                  actionCollection(), "editorwindow_forward");

    m_firstAction = new KAction(i18n("&First"), "start",
                                KStdAccel::shortcut( KStdAccel::Home),
                                this, SLOT(slotFirst()),
                                actionCollection(), "editorwindow_first");

    m_lastAction = new KAction(i18n("&Last"), "finish",
                               KStdAccel::shortcut( KStdAccel::End),
                               this, SLOT(slotLast()),
                               actionCollection(), "editorwindow_last");

    m_saveAction   = KStdAction::save(this, SLOT(slotSave()),
                                      actionCollection(), "editorwindow_save");

    m_saveAsAction = KStdAction::saveAs(this, SLOT(slotSaveAs()),
                                        actionCollection(), "editorwindow_saveas");

    m_revertAction = KStdAction::revert(m_canvas, SLOT(slotRestore()),
                                        actionCollection(), "editorwindow_revert");

    m_saveAction->setEnabled(false);
    m_saveAsAction->setEnabled(false);
    m_revertAction->setEnabled(false);

    d->filePrintAction = new KAction(i18n("Print Image..."), "fileprint",
                                     CTRL+Key_P,
                                     this, SLOT(slotFilePrint()),
                                     actionCollection(), "editorwindow_print");

    m_fileDeleteAction = new KAction(i18n("Move to Trash"), "edittrash",
                                     Key_Delete,
                                     this, SLOT(slotDeleteCurrentItem()),
                                     actionCollection(), "editorwindow_delete");

    KStdAction::quit(this, SLOT(close()), actionCollection(), "editorwindow_exit");

    // -- Standard 'Edit' menu actions ---------------------------------------------

    d->copyAction = KStdAction::copy(m_canvas, SLOT(slotCopy()),
                                     actionCollection(), "editorwindow_copy");
    
    d->copyAction->setEnabled(false);

    m_undoAction = new KToolBarPopupAction(i18n("Undo"), "undo",
                                           KStdAccel::shortcut(KStdAccel::Undo),
                                           m_canvas, SLOT(slotUndo()),
                                           actionCollection(), "editorwindow_undo");

    connect(m_undoAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowUndoMenu()));
            
    connect(m_undoAction->popupMenu(), SIGNAL(activated(int)),
            m_canvas, SLOT(slotUndo(int)));

    m_undoAction->setEnabled(false);

    m_redoAction = new KToolBarPopupAction(i18n("Redo"), "redo",
                                           KStdAccel::shortcut(KStdAccel::Redo),
                                           m_canvas, SLOT(slotRedo()),
                                           actionCollection(), "editorwindow_redo");

    connect(m_redoAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowRedoMenu()));
            
    connect(m_redoAction->popupMenu(), SIGNAL(activated(int)),
            m_canvas, SLOT(slotRedo(int)));

    m_redoAction->setEnabled(false);

    // -- Standard 'View' menu actions ---------------------------------------------

    d->zoomPlusAction = KStdAction::zoomIn(m_canvas, SLOT(slotIncreaseZoom()),
                                          actionCollection(), "editorwindow_zoomplus");


    d->zoomMinusAction = KStdAction::zoomOut(m_canvas, SLOT(slotDecreaseZoom()),
                                             actionCollection(), "editorwindow_zoomminus");

    d->zoomFitToWindowAction = new KToggleAction(i18n("Fit to &Window"), "view_fit_window",
                                         CTRL+SHIFT+Key_A, this, SLOT(slotToggleFitToWindow()),
                                         actionCollection(), "editorwindow_zoomfit2window");

    d->zoomFitToSelectAction = new KAction(i18n("Fit to &Selection"), "viewmagfit",
                                         CTRL+SHIFT+Key_S, this, SLOT(slotFitToSelect()),
                                         actionCollection(), "editorwindow_zoomfit2select");
    d->zoomFitToSelectAction->setEnabled(false);
    d->zoomFitToSelectAction->setWhatsThis(i18n("This option can be used to zoom the image to the "
                                                "current selection area."));

    d->zoomCombo = new KComboBox(true);
    d->zoomCombo->setDuplicatesEnabled(false);
    d->zoomCombo->setFocusPolicy(ClickFocus);
    d->zoomCombo->setInsertionPolicy(QComboBox::NoInsertion);
    d->zoomComboAction = new KWidgetAction(d->zoomCombo, i18n("Zoom"), 0, 0, 0, 
                                           actionCollection(), "editorwindow_zoomto");

    d->zoomCombo->insertItem(QString("10%"));
    d->zoomCombo->insertItem(QString("25%"));
    d->zoomCombo->insertItem(QString("50%"));
    d->zoomCombo->insertItem(QString("75%"));
    d->zoomCombo->insertItem(QString("100%"));
    d->zoomCombo->insertItem(QString("150%"));
    d->zoomCombo->insertItem(QString("200%"));
    d->zoomCombo->insertItem(QString("300%"));
    d->zoomCombo->insertItem(QString("450%"));
    d->zoomCombo->insertItem(QString("600%"));
    d->zoomCombo->insertItem(QString("800%"));
    d->zoomCombo->insertItem(QString("1200%"));

    connect(d->zoomCombo, SIGNAL(activated(int)),
            this, SLOT(slotZoomSelected()) );

    connect(d->zoomCombo, SIGNAL(returnPressed(const QString&)),
            this, SLOT(slotZoomTextChanged(const QString &)) );


#if KDE_IS_VERSION(3,2,0)
    m_fullScreenAction = KStdAction::fullScreen(this, SLOT(slotToggleFullScreen()),
                                                actionCollection(), this, "editorwindow_fullscreen");
#else
    m_fullScreenAction = new KToggleAction(i18n("Fullscreen"), "window_fullscreen",
                                           CTRL+SHIFT+Key_F, this,
                                           SLOT(slotToggleFullScreen()),
                                           actionCollection(), "editorwindow_fullscreen");
#endif

    d->slideShowAction = new KAction(i18n("Slide Show"), "slideshow", Key_F9,
                                     this, SLOT(slotToggleSlideShow()),
                                     actionCollection(),"editorwindow_slideshow");

    d->viewUnderExpoAction = new KToggleAction(i18n("Under-Exposure Indicator"), "underexposure", 
                                            Key_F10, this, 
                                            SLOT(slotToggleUnderExposureIndicator()),
                                            actionCollection(),"editorwindow_underexposure");

    d->viewOverExpoAction = new KToggleAction(i18n("Over-Exposure Indicator"), "overexposure", 
                                            Key_F11, this, 
                                            SLOT(slotToggleOverExposureIndicator()),
                                            actionCollection(),"editorwindow_overexposure");

    d->viewCMViewAction = new KToggleAction(i18n("Color Managed View"), "tv", 
                                            Key_F12, this, 
                                            SLOT(slotToggleColorManagedView()),
                                            actionCollection(),"editorwindow_cmview");

    // -- Standard 'Transform' menu actions ---------------------------------------------

    d->resizeAction = new KAction(i18n("&Resize..."), "resize_image", 0,
                                  this, SLOT(slotResize()),
                                  actionCollection(), "editorwindow_resize");

    d->cropAction = new KAction(i18n("Crop"), "crop",
                                CTRL+Key_X,
                                m_canvas, SLOT(slotCrop()),
                                actionCollection(), "editorwindow_crop");
    
    d->cropAction->setEnabled(false);
    d->cropAction->setWhatsThis(i18n("This option can be used to crop the image. "
                                     "Select a region of the image to enable this action."));

    // -- Standard 'Flip' menu actions ---------------------------------------------
    
    d->flipHorizAction = new KAction(i18n("Flip Horizontally"), "mirror", CTRL+Key_Asterisk,
                                   m_canvas, SLOT(slotFlipHoriz()),
                                   actionCollection(), "editorwindow_flip_horiz");
    d->flipHorizAction->setEnabled(false);

    d->flipVertAction = new KAction(i18n("Flip Vertically"), "flip", CTRL+Key_Slash,
                                   m_canvas, SLOT(slotFlipVert()),
                                   actionCollection(), "editorwindow_flip_vert");
    d->flipVertAction->setEnabled(false);
                                   
    // -- Standard 'Rotate' menu actions ----------------------------------------

    d->rotateLeftAction = new KAction(i18n("Rotate Left"),
                                     "rotate_ccw", SHIFT+CTRL+Key_Left, 
                                     m_canvas, SLOT(slotRotate270()),
                                     actionCollection(),
                                     "editorwindow_rotate_left");
    d->rotateLeftAction->setEnabled(false);
    d->rotateRightAction  = new KAction(i18n("Rotate Right"),
                                     "rotate_cw", SHIFT+CTRL+Key_Right, 
                                     m_canvas, SLOT(slotRotate90()),
                                     actionCollection(),
                                     "editorwindow_rotate_right");
    d->rotateRightAction->setEnabled(false);

    // -- Standard 'Configure' menu actions ----------------------------------------

    KStdAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStdAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStdAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // -- Standard 'Help' menu actions ---------------------------------------------

    d->imagePluginsHelpAction = new KAction(i18n("Image Plugins Handbooks"),
                                            "digikamimageplugins", 0,
                                            this, SLOT(slotImagePluginsHelp()),
                                            actionCollection(), 
                                            "editorwindow_imagepluginshelp");

    d->donateMoneyAction = new KAction(i18n("Donate Money..."),
                                       0, 0, 
                                       this, SLOT(slotDonateMoney()),
                                       actionCollection(),
                                       "editorwindow_donatemoney");
}

void EditorWindow::setupStandardAccelerators()
{
    d->accelerators = new KAccel(this);

    d->accelerators->insert("Exit fullscreen", i18n("Exit Fullscreen mode"),
                    i18n("Exit out of the fullscreen mode"),
                    Key_Escape, this, SLOT(slotEscapePressed()),
                    false, true);

    d->accelerators->insert("Next Image Key_Space", i18n("Next Image"),
                    i18n("Load Next Image"),
                    Key_Space, this, SLOT(slotForward()),
                    false, true);

    d->accelerators->insert("Previous Image Key_Backspace", i18n("Previous Image"),
                    i18n("Load Previous Image"),
                    Key_Backspace, this, SLOT(slotBackward()),
                    false, true);

    d->accelerators->insert("Next Image Key_Next", i18n("Next Image"),
                    i18n("Load Next Image"),
                    Key_Next, this, SLOT(slotForward()),
                    false, true);

    d->accelerators->insert("Previous Image Key_Prior", i18n("Previous Image"),
                    i18n("Load Previous Image"),
                    Key_Prior, this, SLOT(slotBackward()),
                    false, true);

    d->accelerators->insert("Zoom Plus Key_Plus", i18n("Zoom In"),
                    i18n("Zoom in on Image"),
                    Key_Plus, m_canvas, SLOT(slotIncreaseZoom()),
                    false, true);
    
    d->accelerators->insert("Zoom Plus Key_Minus", i18n("Zoom Out"),
                    i18n("Zoom out of Image"),
                    Key_Minus, m_canvas, SLOT(slotDecreaseZoom()),
                    false, true);
}

void EditorWindow::setupStatusBar()
{
    m_nameLabel = new StatusProgressBar(statusBar());
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setMaximumHeight(fontMetrics().height()+2);    
    statusBar()->addWidget(m_nameLabel, 100);

    d->selectLabel = new QLabel(i18n("No selection"), statusBar());
    d->selectLabel->setAlignment(Qt::AlignCenter);
    d->selectLabel->setMaximumHeight(fontMetrics().height()+2);   
    statusBar()->addWidget(d->selectLabel, 100);
    QToolTip::add(d->selectLabel, i18n("Information about current selection area"));

    m_resLabel  = new QLabel(statusBar());
    m_resLabel->setAlignment(Qt::AlignCenter);
    m_resLabel->setMaximumHeight(fontMetrics().height()+2);   
    statusBar()->addWidget(m_resLabel, 100);
    QToolTip::add(m_resLabel, i18n("Information about image size"));

    d->underExposureIndicator = new QToolButton(statusBar());
    d->underExposureIndicator->setIconSet(SmallIcon("underexposure"));
    d->underExposureIndicator->setToggleButton(true);
    statusBar()->addWidget(d->underExposureIndicator, 1);

    d->overExposureIndicator = new QToolButton(statusBar());
    d->overExposureIndicator->setIconSet(SmallIcon("overexposure"));
    d->overExposureIndicator->setToggleButton(true);
    statusBar()->addWidget(d->overExposureIndicator, 1);

    d->cmViewIndicator = new QToolButton(statusBar());
    d->cmViewIndicator->setIconSet(SmallIcon("tv"));
    d->cmViewIndicator->setToggleButton(true);
    statusBar()->addWidget(d->cmViewIndicator, 1);

    connect(d->underExposureIndicator, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleUnderExposureIndicator()));

    connect(d->overExposureIndicator, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleOverExposureIndicator()));

    connect(d->cmViewIndicator, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleColorManagedView()));
}

void EditorWindow::printImage(KURL url)
{
    uchar* ptr      = m_canvas->interface()->getImage();
    int w           = m_canvas->interface()->origWidth();
    int h           = m_canvas->interface()->origHeight();
    bool hasAlpha   = m_canvas->interface()->hasAlpha();
    bool sixteenBit = m_canvas->interface()->sixteenBit();

    if (!ptr || !w || !h)
        return;

    DImg image(w, h, sixteenBit, hasAlpha, ptr);

    KPrinter printer;
    QString appName = KApplication::kApplication()->aboutData()->appName();
    printer.setDocName( url.filename() );
    printer.setCreator( appName );
#if KDE_IS_VERSION(3,2,0)
    printer.setUsePrinterResolution(true);
#endif

    KPrinter::addDialogPage( new ImageEditorPrintDialogPage(image, this, (appName.append(" page")).ascii() ));

    if ( printer.setup( this, i18n("Print %1").arg(printer.docName().section('/', -1)) ) )
    {
        ImagePrint printOperations(image, printer, url.filename());
        if (!printOperations.printImageWithQt())
        {
            KMessageBox::error(this, i18n("Failed to print file: '%1'")
                               .arg(url.filename()));
        }
    }
}

void EditorWindow::slotImagePluginsHelp()
{
    KApplication::kApplication()->invokeHelp( QString(), "digikamimageplugins" );
}

void EditorWindow::slotEditKeys()
{
    KKeyDialog dialog(true, this);
    dialog.insert( actionCollection(), i18n( "General" ) );

    QPtrList<ImagePlugin> pluginList = ImagePluginLoader::instance()->pluginList();

    for (ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin)
        {
            dialog.insert( plugin->actionCollection(), plugin->name() );
        }
    }

    dialog.configure();
}

void EditorWindow::slotResize()
{
    ImageResize dlg(this);
    dlg.exec();
}

void EditorWindow::slotAboutToShowUndoMenu()
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

void EditorWindow::slotAboutToShowRedoMenu()
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

void EditorWindow::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config(), "ImageViewer Settings");
    KEditToolbar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void EditorWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config(), "ImageViewer Settings");
}

void EditorWindow::slotToggleFitToWindow()
{
    bool checked = d->zoomFitToWindowAction->isChecked();

    d->zoomPlusAction->setEnabled(!checked);
    d->zoomComboAction->setEnabled(!checked);
    d->zoomMinusAction->setEnabled(!checked);

    m_canvas->toggleFitToWindow();
}

void EditorWindow::slotFitToSelect()
{
    d->zoomFitToWindowAction->blockSignals(true);
    d->zoomFitToWindowAction->setChecked(false);
    d->zoomFitToWindowAction->blockSignals(false);
    d->zoomPlusAction->setEnabled(true);
    d->zoomComboAction->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_canvas->fitToSelect();
}

void EditorWindow::slotZoomTextChanged(const QString &txt)
{
    double zoom = KGlobal::locale()->readNumber(txt) / 100.0;
    if (zoom > 0.0)
        m_canvas->setZoomFactor(zoom);
}

void EditorWindow::slotZoomSelected()
{
    QString txt = d->zoomCombo->currentText();
    txt = txt.left(txt.find('%'));
    slotZoomTextChanged(txt);
}

void EditorWindow::slotZoomChanged(double zoom)
{
    d->zoomPlusAction->setEnabled(!m_canvas->maxZoom() && !m_canvas->fitToWindow());
    d->zoomMinusAction->setEnabled(!m_canvas->minZoom() && !m_canvas->fitToWindow());

    d->zoomCombo->blockSignals(true);
    d->zoomCombo->setCurrentText(QString::number(lround(zoom*100.0)) + QString("%"));
    d->zoomCombo->blockSignals(false);
}

void EditorWindow::slotEscapePressed()
{
    if (m_fullScreen)
        m_fullScreenAction->activate();
}

void EditorWindow::plugActionAccel(KAction* action)
{
    if (!action)
        return;

    d->accelerators->insert(action->text(),
                    action->text(),
                    action->whatsThis(),
                    action->shortcut(),
                    action,
                    SLOT(activate()));
}

void EditorWindow::unplugActionAccel(KAction* action)
{
    d->accelerators->remove(action->text());
}

void EditorWindow::loadImagePlugins()
{
    QPtrList<ImagePlugin> pluginList = m_imagePluginLoader->pluginList();

    for (ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin)
        {
            guiFactory()->addClient(plugin);
            plugin->setParentWidget(this);
            plugin->setEnabledSelectionActions(false);
        }
        else
            DDebug() << "Invalid plugin to add!" << endl;
    }
}

void EditorWindow::unLoadImagePlugins()
{
    QPtrList<ImagePlugin> pluginList = m_imagePluginLoader->pluginList();

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
}

void EditorWindow::readStandardSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    // Restore full screen Mode ?

    if (config->readBoolEntry("FullScreen", false))
    {
        m_fullScreenAction->activate();
        m_fullScreen = true;
    }

    // Restore Auto zoom action ?
    bool autoZoom = config->readBoolEntry("AutoZoom", true);

    if (autoZoom)
    {
        d->zoomFitToWindowAction->activate();
        d->zoomPlusAction->setEnabled(false);
        d->zoomMinusAction->setEnabled(false);
    }
}

void EditorWindow::applyStandardSettings()
{
    KConfig* config = kapp->config();

    // -- Settings for Color Management stuff ----------------------------------------------

    config->setGroup("Color Management");

    d->ICCSettings->enableCMSetting       = config->readBoolEntry("EnableCM", false);
    d->ICCSettings->askOrApplySetting     = config->readBoolEntry("BehaviourICC", false);
    d->ICCSettings->BPCSetting            = config->readBoolEntry("BPCAlgorithm",false);
    d->ICCSettings->managedViewSetting    = config->readBoolEntry("ManagedView", false);
    d->ICCSettings->renderingSetting      = config->readNumEntry("RenderingIntent");
    d->ICCSettings->inputSetting          = config->readPathEntry("InProfileFile", QString());
    d->ICCSettings->workspaceSetting      = config->readPathEntry("WorkProfileFile", QString());
    d->ICCSettings->monitorSetting        = config->readPathEntry("MonitorProfileFile", QString());
    d->ICCSettings->proofSetting          = config->readPathEntry("ProofProfileFile", QString());
    d->ICCSettings->CMInRawLoadingSetting = config->readBoolEntry("CMInRawLoading", false);

    d->viewCMViewAction->setEnabled(d->ICCSettings->enableCMSetting);
    d->viewCMViewAction->setChecked(d->ICCSettings->managedViewSetting);
    d->cmViewIndicator->setEnabled(d->ICCSettings->enableCMSetting);
    d->cmViewIndicator->setOn(d->ICCSettings->managedViewSetting);
    setColorManagedViewIndicatorToolTip(d->ICCSettings->enableCMSetting, d->ICCSettings->managedViewSetting);
    m_canvas->setICCSettings(d->ICCSettings);

    // -- JPEG, PNG, TIFF JPEG2000 files format settings --------------------------------------

    config->setGroup("ImageViewer Settings");

    // JPEG quality slider settings : 1 - 100 ==> libjpeg settings : 25 - 100.
    m_IOFileSettings->JPEGCompression     = (int)((75.0/100.0)*
                                                 (float)config->readNumEntry("JPEGCompression", 75)
                                                 + 26.0 - (75.0/100.0));

    // PNG compression slider settings : 1 - 9 ==> libpng settings : 100 - 1.
    m_IOFileSettings->PNGCompression      = (int)(((1.0-100.0)/8.0)*
                                                 (float)config->readNumEntry("PNGCompression", 1)
                                                 + 100.0 - ((1.0-100.0)/8.0));

    // TIFF compression setting.
    m_IOFileSettings->TIFFCompression     = config->readBoolEntry("TIFFCompression", false);

    // JPEG2000 quality slider settings : 1 - 100
    m_IOFileSettings->JPEG2000Compression = config->readNumEntry("JPEG2000Compression", 100);

    // JPEG2000 LossLess setting.
    m_IOFileSettings->JPEG2000LossLess    = config->readBoolEntry("JPEG2000LossLess", true);

    // -- RAW pictures decoding settings ------------------------------------------------------

    // If digiKam Color Management is enable, no need to correct color of decoded RAW image,
    // else, sRGB color workspace will be used.

    if (d->ICCSettings->enableCMSetting) 
        m_IOFileSettings->rawDecodingSettings.outputColorSpace = KDcrawIface::RawDecodingSettings::RAWCOLOR;
    else
        m_IOFileSettings->rawDecodingSettings.outputColorSpace = KDcrawIface::RawDecodingSettings::SRGB;

    m_IOFileSettings->rawDecodingSettings.sixteenBitsImage        = config->readBoolEntry("SixteenBitsImage", false);
    m_IOFileSettings->rawDecodingSettings.automaticColorBalance   = config->readBoolEntry("AutomaticColorBalance", true);
    m_IOFileSettings->rawDecodingSettings.cameraColorBalance      = config->readBoolEntry("CameraColorBalance", true);
    m_IOFileSettings->rawDecodingSettings.RGBInterpolate4Colors   = config->readBoolEntry("RGBInterpolate4Colors", false);
    m_IOFileSettings->rawDecodingSettings.DontStretchPixels = config->readBoolEntry("DontStretchPixels", false);
    m_IOFileSettings->rawDecodingSettings.enableNoiseReduction    = config->readBoolEntry("EnableNoiseReduction", false);
    m_IOFileSettings->rawDecodingSettings.unclipColors            = config->readNumEntry("UnclipColors", 0);
    m_IOFileSettings->rawDecodingSettings.RAWQuality              = (KDcrawIface::RawDecodingSettings::DecodingQuality)config->readNumEntry("RAWQuality",
                                                                    KDcrawIface::RawDecodingSettings::BILINEAR);
    m_IOFileSettings->rawDecodingSettings.NRThreshold             = config->readNumEntry("NRThreshold", 100);
    m_IOFileSettings->rawDecodingSettings.brightness              = config->readDoubleNumEntry("RAWBrightness", 1.0);

    // -- GUI Settings -------------------------------------------------------
    
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    if(config->hasKey("Splitter Sizes"))
        m_splitter->setSizes(config->readIntListEntry("Splitter Sizes"));
    else 
        m_canvas->setSizePolicy(rightSzPolicy);
    
    d->fullScreenHideToolBar = config->readBoolEntry("FullScreen Hide ToolBar", false);

    // -- Exposure Indicators Settings --------------------------------------- 

    QColor black(Qt::black);
    QColor white(Qt::white);
    d->exposureSettings->underExposureIndicator = config->readBoolEntry("UnderExposureIndicator", false);
    d->exposureSettings->overExposureIndicator  = config->readBoolEntry("OverExposureIndicator", false);
    d->exposureSettings->underExposureColor     = config->readColorEntry("UnderExposureColor", &white);
    d->exposureSettings->overExposureColor      = config->readColorEntry("OverExposureColor", &black);

    d->viewUnderExpoAction->setChecked(d->exposureSettings->underExposureIndicator);
    d->viewOverExpoAction->setChecked(d->exposureSettings->overExposureIndicator);
    d->underExposureIndicator->setOn(d->exposureSettings->underExposureIndicator);
    d->overExposureIndicator->setOn(d->exposureSettings->overExposureIndicator);
    setUnderExposureToolTip(d->exposureSettings->underExposureIndicator);
    setOverExposureToolTip(d->exposureSettings->overExposureIndicator);
    m_canvas->setExposureSettings(d->exposureSettings);
}

void EditorWindow::saveStandardSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    
    config->writeEntry("AutoZoom", d->zoomFitToWindowAction->isChecked());
    config->writeEntry("Splitter Sizes", m_splitter->sizes());

    config->writeEntry("FullScreen", m_fullScreenAction->isChecked());
    config->writeEntry("UnderExposureIndicator", d->exposureSettings->underExposureIndicator);
    config->writeEntry("OverExposureIndicator", d->exposureSettings->overExposureIndicator);

    config->sync();
}

void EditorWindow::toggleStandardActions(bool val)
{
    d->zoomFitToWindowAction->setEnabled(val);
    m_saveAsAction->setEnabled(val);
    d->rotateLeftAction->setEnabled(val);
    d->rotateRightAction->setEnabled(val);
    d->flipHorizAction->setEnabled(val);
    d->flipVertAction->setEnabled(val);
    d->filePrintAction->setEnabled(val);
    d->resizeAction->setEnabled(val);
    m_fileDeleteAction->setEnabled(val);
    m_saveAsAction->setEnabled(val);

    // these actions are special: They are turned off if val is false,
    // but if val is true, they may be turned on or off.
    if (val)
    {
        // Trigger sending of signalUndoStateChanged
        // Note that for saving and loading, this is not necessary
        // because the signal will be sent later anyway.
        m_canvas->updateUndoState();
    }
    else
    {
        m_saveAction->setEnabled(val);
        m_undoAction->setEnabled(val);
        m_redoAction->setEnabled(val);
    }

    QPtrList<ImagePlugin> pluginList = m_imagePluginLoader->pluginList();
    
    for (ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin) 
        {
            plugin->setEnabledActions(val);
        }
    }
}

void EditorWindow::slotToggleFullScreen()
{
    if (m_fullScreen) // out of fullscreen
    {
        m_canvas->setBackgroundColor(m_bgColor);

#if QT_VERSION >= 0x030300
        setWindowState( windowState() & ~WindowFullScreen );
#else
        showNormal();
#endif
        menuBar()->show();
        statusBar()->show();
        leftDock()->show();
        rightDock()->show();
        topDock()->show();
        bottomDock()->show();
        
        QObject* obj = child("ToolBar","KToolBar");
        
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            
            if (m_fullScreenAction->isPlugged(toolBar) && d->removeFullScreenButton)
                m_fullScreenAction->unplug(toolBar);
                
            if (toolBar->isHidden())
                showToolBars();
        }

        // -- remove the gui action accels ----

        unplugActionAccel(m_forwardAction);
        unplugActionAccel(m_backwardAction);
        unplugActionAccel(m_firstAction);
        unplugActionAccel(m_lastAction);
        unplugActionAccel(m_saveAction);
        unplugActionAccel(m_saveAsAction);
        unplugActionAccel(d->zoomPlusAction);
        unplugActionAccel(d->zoomMinusAction);
        unplugActionAccel(d->zoomFitToWindowAction);
        unplugActionAccel(d->zoomFitToSelectAction);
        unplugActionAccel(d->cropAction);
        unplugActionAccel(d->filePrintAction);
        unplugActionAccel(m_fileDeleteAction);

        toggleGUI2FullScreen();
        m_fullScreen = false;
    }
    else  // go to fullscreen
    {
        m_canvas->setBackgroundColor(QColor(Qt::black));
        
        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();
        topDock()->hide();
        leftDock()->hide();
        rightDock()->hide();
        bottomDock()->hide();
        
        QObject* obj = child("ToolBar","KToolBar");
        
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            
            if (d->fullScreenHideToolBar)
            {
                hideToolBars();
            }
            else
            {   
                showToolBars();

                if ( !m_fullScreenAction->isPlugged(toolBar) )
                {
                    m_fullScreenAction->plug(toolBar);
                    d->removeFullScreenButton=true;
                }
                else    
                {
                    // If FullScreen button is enable in toolbar settings
                    // We don't remove it when we out of fullscreen mode.
                    d->removeFullScreenButton=false;
                }
            }
        }

        // -- Insert all the gui actions into the accel --

        plugActionAccel(m_forwardAction);
        plugActionAccel(m_backwardAction);
        plugActionAccel(m_firstAction);
        plugActionAccel(m_lastAction);
        plugActionAccel(m_saveAction);
        plugActionAccel(m_saveAsAction);
        plugActionAccel(d->zoomPlusAction);
        plugActionAccel(d->zoomMinusAction);
        plugActionAccel(d->zoomFitToWindowAction);
        plugActionAccel(d->zoomFitToSelectAction);
        plugActionAccel(d->cropAction);
        plugActionAccel(d->filePrintAction);
        plugActionAccel(m_fileDeleteAction);

        toggleGUI2FullScreen();
        showFullScreen();
        m_fullScreen = true;
    }
}

void EditorWindow::slotRotatedOrFlipped()
{
    m_rotatedOrFlipped = true;
}

void EditorWindow::slotLoadingProgress(const QString&, float progress)
{
    m_nameLabel->setProgressValue((int)(progress*100.0));
}

void EditorWindow::slotSavingProgress(const QString&, float progress)
{
    m_nameLabel->setProgressValue((int)(progress*100.0));
}

bool EditorWindow::promptUserSave(const KURL& url)
{
    if (m_saveAction->isEnabled())
    {
        // if window is iconified, show it
        if (isMinimized())
        {
            KWin::deIconifyWindow(winId());
        }

        int result = KMessageBox::warningYesNoCancel(this,
                                  i18n("The image '%1' has been modified.\n"
                                       "Do you want to save it?")
                                       .arg(url.filename()),
                                  QString(),
                                  KStdGuiItem::save(),
                                  KStdGuiItem::discard());

        if (result == KMessageBox::Yes)
        {
            bool saving;

            if (m_canvas->isReadOnly())
                saving = saveAs();
            else
                saving = save();

            // save and saveAs return false if they were cancelled and did not enter saving at all
            // In this case, do not call enter_loop because exit_loop will not be called.
            if (saving)
            {
                // Waiting for asynchronous image file saving operation runing in separate thread.
                m_savingContext->synchronizingState = SavingContextContainer::SynchronousSaving;
                enter_loop();
                m_savingContext->synchronizingState = SavingContextContainer::NormalSaving;
                return m_savingContext->synchronousSavingResult;
            }
            else
            {
                return false;
            }
        }
        else if (result == KMessageBox::No)
        {
            m_saveAction->setEnabled(false);
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool EditorWindow::waitForSavingToComplete()
{
    // avoid reentrancy - return false means we have reentered the loop already.
    if (m_savingContext->synchronizingState == SavingContextContainer::SynchronousSaving)
        return false;

    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
    {
        // Waiting for asynchronous image file saving operation runing in separate thread.
        m_savingContext->synchronizingState = SavingContextContainer::SynchronousSaving;
        KMessageBox::queuedMessageBox(this,
                                      KMessageBox::Information,
                                      i18n("Please wait while the image is being saved..."));
        enter_loop();
        m_savingContext->synchronizingState = SavingContextContainer::NormalSaving;
    }
    return true;
}

void EditorWindow::enter_loop()
{
    QWidget dummy(0, 0, WType_Dialog | WShowModal);
    dummy.setFocusPolicy( QWidget::NoFocus );
    qt_enter_modal(&dummy);
    qApp->enter_loop();
    qt_leave_modal(&dummy);
}

void EditorWindow::slotSelected(bool val)
{
    // Update menu actions.
    d->cropAction->setEnabled(val);
    d->zoomFitToSelectAction->setEnabled(val);
    d->copyAction->setEnabled(val);

    for (ImagePlugin* plugin = m_imagePluginLoader->pluginList().first();
         plugin; plugin = m_imagePluginLoader->pluginList().next())
    {
        if (plugin) 
        {
            plugin->setEnabledSelectionActions(val);
        }
    }

    QRect sel = m_canvas->getSelectedArea();
    // Update histogram into sidebar.
    emit signalSelectionChanged(sel);

    // Update status bar
    if (val)
        d->selectLabel->setText(QString("(%1, %2) (%3 x %4)").arg(sel.x()).arg(sel.y())
                               .arg(sel.width()).arg(sel.height()));
    else 
        d->selectLabel->setText(i18n("No selection"));
}

void EditorWindow::hideToolBars()
{
    QPtrListIterator<KToolBar> it = toolBarIterator();
    KToolBar* bar;

    for(;it.current()!=0L; ++it)
    {
        bar=it.current();
        
        if (bar->area()) 
            bar->area()->hide();
        else 
            bar->hide();
    }
}

void EditorWindow::showToolBars()
{
    QPtrListIterator<KToolBar> it = toolBarIterator();
    KToolBar* bar;

    for( ; it.current()!=0L ; ++it)
    {
        bar=it.current();
        
        if (bar->area())
            bar->area()->show();
        else
            bar->show();
    }
}

void EditorWindow::slotLoadingStarted(const QString& /*filename*/)
{
    setCursor( KCursor::waitCursor() );

    // Disable actions as appropriate during loading
    emit signalNoCurrentItem();
    toggleActions(false);

    m_nameLabel->progressBarMode(StatusProgressBar::ProgressBarMode, i18n("Loading: "));
}

void EditorWindow::slotLoadingFinished(const QString& filename, bool success)
{
    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);
    slotUpdateItemInfo();

    // Enable actions as appropriate after loading
    // No need to re-enable image properties sidebar here, it's will be done
    // automatically by a signal from canvas
    toggleActions(success);
    unsetCursor();

    // Note: in showfoto, we using a null filename to clear canvas.
    if (!success && filename != QString())
    {
        QFileInfo fi(filename);
        QString message = i18n("Failed to load image \"%1\"").arg(fi.fileName());
        KMessageBox::error(this, message);
        DWarning() << "Failed to load image " << fi.fileName() << endl;
    }
}

void EditorWindow::slotNameLabelCancelButtonPressed()
{
    // If we saving a picture...
    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
    {
        m_savingContext->abortingSaving = true;
        m_canvas->abortSaving();
    }

    // If we preparing SlideShow...
    m_cancelSlideShow = true;
}

void EditorWindow::slotSave()
{
    if (m_canvas->isReadOnly())
        saveAs();
    else
        save();
}
void EditorWindow::slotSavingStarted(const QString& /*filename*/)
{
    setCursor( KCursor::waitCursor() );
    
    // Disable actions as appropriate during saving
    emit signalNoCurrentItem();
    toggleActions(false);

    m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode, i18n("Saving: "));
}

void EditorWindow::slotSavingFinished(const QString& filename, bool success)
{
    if (m_savingContext->savingState == SavingContextContainer::SavingStateSave)
    {
        // from save()
        m_savingContext->savingState = SavingContextContainer::SavingStateNone;

        if (!success)
        {
            if (!m_savingContext->abortingSaving)
            {
                KMessageBox::error(this, i18n("Failed to save file\n\"%1\"\nto\n\"%2\".")
                                .arg(m_savingContext->destinationURL.filename())
                                .arg(m_savingContext->destinationURL.path()));
            }
            finishSaving(false);
            return;
        }

        DDebug() << "renaming to " << m_savingContext->destinationURL.path() << endl;

        if (!moveFile())
        {
            finishSaving(false);
            return;
        }

        m_canvas->setUndoHistoryOrigin();

        // remove image from cache since it has changed
        LoadingCacheInterface::cleanFromCache(m_savingContext->destinationURL.path());
        // this won't be in the cache, but does not hurt to do it anyway
        LoadingCacheInterface::cleanFromCache(filename);

        // restore state of disabled actions. saveIsComplete can start any other task
        // (loading!) which might itself in turn change states
        finishSaving(true);

        saveIsComplete();
        
        // Take all actions necessary to update information and re-enable sidebar
        slotChanged();
    }
    else if (m_savingContext->savingState == SavingContextContainer::SavingStateSaveAs)
    {
        m_savingContext->savingState = SavingContextContainer::SavingStateNone;

        // from saveAs()
        if (!success)
        {
            if (!m_savingContext->abortingSaving)
            {
                KMessageBox::error(this, i18n("Failed to save file\n\"%1\"\nto\n\"%2\".")
                                .arg(m_savingContext->destinationURL.filename())
                                .arg(m_savingContext->destinationURL.path()));
            }
            finishSaving(false);
            return;
        }

        // Only try to write exif if both src and destination are jpeg files

        DDebug() << "renaming to " << m_savingContext->destinationURL.path() << endl;

        if (!moveFile())
        {
            finishSaving(false);
            return;
        }

        m_canvas->setUndoHistoryOrigin();

        LoadingCacheInterface::cleanFromCache(m_savingContext->destinationURL.path());
        LoadingCacheInterface::cleanFromCache(filename);

        finishSaving(true);
        saveAsIsComplete();

        // Take all actions necessary to update information and re-enable sidebar
        slotChanged();
    }
}

void EditorWindow::finishSaving(bool success)
{
    m_savingContext->synchronousSavingResult = success;

    if (m_savingContext->saveTempFile)
    {
        delete m_savingContext->saveTempFile;
        m_savingContext->saveTempFile = 0;
    }

    // Exit of internal Qt event loop to unlock promptUserSave() method.
    if (m_savingContext->synchronizingState == SavingContextContainer::SynchronousSaving)
        qApp->exit_loop();

    // Enable actions as appropriate after saving
    toggleActions(true);
    unsetCursor();

    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);

    // On error, continue using current image
    if (!success)
    {
        m_canvas->switchToLastSaved(m_savingContext->srcURL.path());
    }
}

void EditorWindow::startingSave(const KURL& url)
{
    // avoid any reentrancy. Should be impossible anyway since actions will be disabled.
    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
        return;

    if (!checkPermissions(url))
        return;

    m_savingContext->srcURL             = url;
    m_savingContext->destinationURL     = m_savingContext->srcURL;
    m_savingContext->destinationExisted = true;
    m_savingContext->originalFormat     = m_canvas->currentImageFileFormat();
    m_savingContext->format             = m_savingContext->originalFormat;
    m_savingContext->abortingSaving     = false;
    m_savingContext->savingState        = SavingContextContainer::SavingStateSave;
    // use magic file extension which tells the digikamalbums ioslave to ignore the file
    m_savingContext->saveTempFile       = new KTempFile(m_savingContext->srcURL.directory(false),
                                                        ".digikamtempfile.tmp");
    m_savingContext->saveTempFile->setAutoDelete(true);

    m_canvas->saveAs(m_savingContext->saveTempFile->name(), m_IOFileSettings,
                     m_setExifOrientationTag && (m_rotatedOrFlipped || m_canvas->exifRotated()));
}

bool EditorWindow::startingSaveAs(const KURL& url)
{
    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
        return false;

    QString mimetypes = KImageIO::mimeTypes(KImageIO::Writing).join(" ");
    mimetypes.append(" image/tiff");
    DDebug () << "mimetypes=" << mimetypes << endl;    

    m_savingContext->srcURL = url;

    FileSaveOptionsBox *options = new FileSaveOptionsBox();
    KFileDialog imageFileSaveDialog(m_savingContext->srcURL.isLocalFile() ? 
                                    m_savingContext->srcURL.directory() : QDir::homeDirPath(),
                                    QString(),
                                    this,
                                    "imageFileSaveDialog",
                                    false,
                                    options);

    imageFileSaveDialog.setOperationMode(KFileDialog::Saving);
    imageFileSaveDialog.setMode(KFile::File);
    imageFileSaveDialog.setSelection(m_savingContext->srcURL.fileName());
    imageFileSaveDialog.setCaption(i18n("New Image File Name"));
    imageFileSaveDialog.setFilter(mimetypes);

    connect(&imageFileSaveDialog, SIGNAL(filterChanged(const QString &)),
            options, SLOT(slotImageFileFormatChanged(const QString &)));

    connect(&imageFileSaveDialog, SIGNAL(fileSelected(const QString &)),
            options, SLOT(slotImageFileSelected(const QString &)));

    options->slotImageFileSelected(m_savingContext->srcURL.path());

    // Start dialog and check if canceled.
    if ( imageFileSaveDialog.exec() != KFileDialog::Accepted )
       return false;

    // Update file save settings in editor instance.
    options->applySettings();
    applyStandardSettings();

    KURL newURL = imageFileSaveDialog.selectedURL();

    // Check if target image format have been selected from Combo List of SaveAs dialog.
    m_savingContext->format = KImageIO::typeForMime(imageFileSaveDialog.currentMimeFilter());

    if ( m_savingContext->format.isEmpty() )
    {
        // Else, check if target image format have been add to target image file name using extension.

        QFileInfo fi(newURL.path());
        m_savingContext->format = fi.extension(false);
        
        if ( m_savingContext->format.isEmpty() )
        {
            // If format is empty then file format is same as that of the original file.
            m_savingContext->format = QImageIO::imageFormat(m_savingContext->srcURL.path());
        }
        else
        {
            // Else, check if format from file name extension is include on file mime type list.

            QString imgExtPattern;
            QStringList imgExtList = QStringList::split(" ", mimetypes);
            for (QStringList::ConstIterator it = imgExtList.begin() ; it != imgExtList.end() ; ++it)
            {    
                imgExtPattern.append (KImageIO::typeForMime(*it).upper());
                imgExtPattern.append (" ");
            }    
            imgExtPattern.append (" TIF TIFF");
            if ( imgExtPattern.contains("JPEG") ) 
            {
                imgExtPattern.append (" JPG");
                imgExtPattern.append (" JPE");
            }
    
            if ( !imgExtPattern.contains( m_savingContext->format.upper() ) )
            {
                KMessageBox::error(this, i18n("Target image file format \"%1\" unsupported.")
                        .arg(m_savingContext->format));
                DWarning() << k_funcinfo << "target image file format " << m_savingContext->format << " unsupported!" << endl;
                return false;
            }
        }
    }
    
    if (!newURL.isValid())
    {
        KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to\n\"%2\".")
                           .arg(newURL.filename())
                           .arg(newURL.path().section('/', -2, -2)));
        DWarning() << k_funcinfo << "target URL is not valid !" << endl;
        return false;
    }

    // if new and original url are equal use slotSave() ------------------------------
    
    KURL currURL(m_savingContext->srcURL);
    currURL.cleanPath();
    newURL.cleanPath();

    if (currURL.equals(newURL))
    {
        slotSave();
        return false;
    }

    // Check for overwrite ----------------------------------------------------------
    
    QFileInfo fi(newURL.path());
    m_savingContext->destinationExisted = fi.exists();
    if ( m_savingContext->destinationExisted )
    {
        int result =

            KMessageBox::warningYesNo( this, i18n("A file named \"%1\" already "
                                                  "exists. Are you sure you want "
                                                  "to overwrite it?")
                                       .arg(newURL.filename()),
                                       i18n("Overwrite File?"),
                                       i18n("Overwrite"),
                                       KStdGuiItem::cancel() );

        if (result != KMessageBox::Yes)
            return false;

        // There will be two message boxes if the file is not writable.
        // This may be controversial, and it may be changed, but it was a deliberate decision.
        if (!checkPermissions(newURL))
            return false;
    }

    // Now do the actual saving -----------------------------------------------------

    // use magic file extension which tells the digikamalbums ioslave to ignore the file
    m_savingContext->saveTempFile   = new KTempFile(newURL.directory(false), ".digikamtempfile.tmp");
    m_savingContext->destinationURL = newURL;
    m_savingContext->originalFormat = m_canvas->currentImageFileFormat();
    m_savingContext->savingState    = SavingContextContainer::SavingStateSaveAs;
    m_savingContext->saveTempFile->setAutoDelete(true);
    m_savingContext->abortingSaving = false;

    m_canvas->saveAs(m_savingContext->saveTempFile->name(), m_IOFileSettings,
                     m_setExifOrientationTag && (m_rotatedOrFlipped || m_canvas->exifRotated()),
                     m_savingContext->format.lower());

    return true;
}

bool EditorWindow::checkPermissions(const KURL& url)
{
    //TODO: Check that the permissions can actually be changed
    //      if write permissions are not available.

    QFileInfo fi(url.path());

    if (fi.exists() && !fi.isWritable())
    {
       int result =

            KMessageBox::warningYesNo( this, i18n("You do not have write permissions "
                                                  "for the file named \"%1\". "
                                                  "Are you sure you want "
                                                  "to overwrite it?")
                                       .arg(url.filename()),
                                       i18n("Overwrite File?"),
                                       i18n("Overwrite"),
                                       KStdGuiItem::cancel() );

        if (result != KMessageBox::Yes)
            return false;
    }

    return true;
}

bool EditorWindow::moveFile()
{
    QCString dstFileName = QFile::encodeName(m_savingContext->destinationURL.path());

    // store old permissions
    mode_t filePermissions = S_IREAD | S_IWRITE;
    if (m_savingContext->destinationExisted)
    {
        struct stat stbuf;
        if (::stat(dstFileName, &stbuf) == 0)
        {
            filePermissions = stbuf.st_mode;
        }
    }

    // rename tmp file to dest
    if (::rename(QFile::encodeName(m_savingContext->saveTempFile->name()), dstFileName) != 0)
    {
        KMessageBox::error(this, i18n("Failed to overwrite original file"),
                           i18n("Error Saving File"));
        return false;
    }

    // restore permissions
    if (m_savingContext->destinationExisted)
    {
        if (::chmod(dstFileName, filePermissions) != 0)
        {
            DWarning() << "Failed to restore file permissions for file " << dstFileName << endl;
        }
    }

    return true;
}

void EditorWindow::slotToggleColorManagedView()
{
    d->cmViewIndicator->blockSignals(true);
    d->viewCMViewAction->blockSignals(true);
    bool cmv = false;
    if (d->ICCSettings->enableCMSetting)
    {    
        cmv = !d->ICCSettings->managedViewSetting;
        d->ICCSettings->managedViewSetting = cmv;
        m_canvas->setICCSettings(d->ICCSettings);
    
        // Save Color Managed View setting in config file. For performance 
        // reason, no need to flush file, it cached in memory and will be flushed 
        // to disk at end of session.  
        KConfig* config = kapp->config();
        config->setGroup("Color Management");
        config->writeEntry("ManagedView", cmv);
    }

    d->cmViewIndicator->setOn(cmv);
    d->viewCMViewAction->setChecked(cmv);
    setColorManagedViewIndicatorToolTip(d->ICCSettings->enableCMSetting, cmv);
    d->cmViewIndicator->blockSignals(false);
    d->viewCMViewAction->blockSignals(false);
}    

void EditorWindow::setColorManagedViewIndicatorToolTip(bool available, bool cmv)
{
    QToolTip::remove(d->cmViewIndicator);
    QString tooltip;
    if (available)
    {
        if (cmv)
            tooltip = i18n("Color Managed View is enabled");
        else
            tooltip = i18n("Color Managed View is disabled");
    }
    else
    {
        tooltip = i18n("Color Management is not configured, so the Color Managed View is not available");
    }
    QToolTip::add(d->cmViewIndicator, tooltip);
}

void EditorWindow::slotToggleUnderExposureIndicator()
{
    d->underExposureIndicator->blockSignals(true);
    d->viewUnderExpoAction->blockSignals(true);
    bool uei = !d->exposureSettings->underExposureIndicator;
    d->underExposureIndicator->setOn(uei);
    d->viewUnderExpoAction->setChecked(uei);
    d->exposureSettings->underExposureIndicator = uei;
    m_canvas->setExposureSettings(d->exposureSettings);
    setUnderExposureToolTip(uei);
    d->underExposureIndicator->blockSignals(false);
    d->viewUnderExpoAction->blockSignals(false);
}    

void EditorWindow::setUnderExposureToolTip(bool uei)
{
    QToolTip::remove(d->underExposureIndicator); 
    QToolTip::add(d->underExposureIndicator, 
                  uei ? i18n("Under-Exposure indicator is enabled") 
                      : i18n("Under-Exposure indicator is disabled"));
}

void EditorWindow::slotToggleOverExposureIndicator()
{
    d->overExposureIndicator->blockSignals(true);
    d->viewOverExpoAction->blockSignals(true);
    bool oei = !d->exposureSettings->overExposureIndicator;
    d->overExposureIndicator->setOn(oei);
    d->viewOverExpoAction->setChecked(oei);
    d->exposureSettings->overExposureIndicator = oei;
    m_canvas->setExposureSettings(d->exposureSettings);
    setOverExposureToolTip(oei);
    d->overExposureIndicator->blockSignals(false);
    d->viewOverExpoAction->blockSignals(false);
}    

void EditorWindow::setOverExposureToolTip(bool oei)
{
    QToolTip::remove(d->overExposureIndicator); 
    QToolTip::add(d->overExposureIndicator, 
                  oei ? i18n("Over-Exposure indicator is enabled") 
                      : i18n("Over-Exposure indicator is disabled"));
}

void EditorWindow::slotDonateMoney()
{
    KApplication::kApplication()->invokeBrowser("http://www.digikam.org/?q=donation");
}

void EditorWindow::slotToggleSlideShow()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    bool startWithCurrent = config->readBoolEntry("SlideShowStartCurrent", false);

    SlideShowSettings settings;
    settings.delay                = config->readNumEntry("SlideShowDelay", 5) * 1000;
    settings.printName            = config->readBoolEntry("SlideShowPrintName", true);
    settings.printDate            = config->readBoolEntry("SlideShowPrintDate", false);
    settings.printApertureFocal   = config->readBoolEntry("SlideShowPrintApertureFocal", false);
    settings.printExpoSensitivity = config->readBoolEntry("SlideShowPrintExpoSensitivity", false);
    settings.printMakeModel       = config->readBoolEntry("SlideShowPrintMakeModel", false);
    settings.printComment         = config->readBoolEntry("SlideShowPrintComment", false);
    settings.loop                 = config->readBoolEntry("SlideShowLoop", false);
    slideShow(startWithCurrent, settings);
}

void EditorWindow::slotSelectionChanged(const QRect& sel)
{
    d->selectLabel->setText(QString("(%1, %2) (%3 x %4)").arg(sel.x()).arg(sel.y())
                           .arg(sel.width()).arg(sel.height()));
}

}  // namespace Digikam

