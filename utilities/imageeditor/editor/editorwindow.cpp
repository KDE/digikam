/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : main image editor GUI implementation
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "editorwindow.h"
#include "editorwindow_p.h"
#include "editorwindow.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QByteArray>
#include <QCursor>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QLabel>
#include <QLayout>
#include <QProgressBar>
#include <QSignalMapper>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QWidgetAction>

// KDE includes

#include <kaboutdata.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kfilefiltercombo.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kprotocolinfo.h>
#include <kselectaction.h>
#include <kservice.h>
#include <kservicetype.h>
#include <kservicetypetrader.h>
#include <kshortcutsdialog.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kstandardshortcut.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <ktogglefullscreenaction.h>
#include <ktoolbar.h>
#include <ktoolbarpopupaction.h>
#include <ktoolinvocation.h>
#include <kwindowsystem.h>
#include <kxmlguifactory.h>
#include <kdeversion.h>
#include <kde_file.h>

// LibKDcraw includes

#include <libkdcraw/version.h>

// Local includes

#include "canvas.h"
#include "dimginterface.h"
#include "dpopupmenu.h"
#include "dlogoaction.h"
#include "editorstackview.h"
#include "editortooliface.h"
#include "exposurecontainer.h"
#include "filesaveoptionsbox.h"
#include "iccsettingscontainer.h"
#include "imagedialog.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "iofilesettingscontainer.h"
#include "libsinfodlg.h"
#include "loadingcacheinterface.h"
#include "rawcameradlg.h"
#include "savingcontextcontainer.h"
#include "sidebar.h"
#include "slideshowsettings.h"
#include "statusprogressbar.h"
#include "themeengine.h"
#include "thumbbar.h"
#include "printhelper.h"

namespace Digikam
{

EditorWindow::EditorWindow(const char *name)
            : KXmlGuiWindow(0), d(new EditorWindowPriv)
{
    setObjectName(name);
    setWindowFlags(Qt::Window);

    m_themeMenuAction        = 0;
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
    m_showBarAction          = 0;
    m_splitter               = 0;
    m_vSplitter              = 0;
    m_stackView              = 0;
    m_animLogo               = 0;
    m_fullScreen             = false;
    m_rotatedOrFlipped       = false;
    m_setExifOrientationTag  = true;
    m_cancelSlideShow        = false;
    m_fullScreenHideThumbBar = true;

    // Settings containers instance.

    d->ICCSettings      = new ICCSettingsContainer();
    d->exposureSettings = new ExposureSettingsContainer();
    d->toolIface        = new EditorToolIface(this);
    m_IOFileSettings    = new IOFileSettingsContainer();
    m_savingContext     = new SavingContextContainer();
    d->waitingLoop      = new QEventLoop(this);
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

EditorStackView* EditorWindow::editorStackView() const
{
    return m_stackView;
}

void EditorWindow::setupContextMenu()
{
    m_contextMenu         = new DPopupMenu(this);
    KActionCollection *ac = actionCollection();
    if (ac->action("editorwindow_backward"))
        m_contextMenu->addAction(ac->action("editorwindow_backward"));
    if (ac->action("editorwindow_forward"))
        m_contextMenu->addAction(ac->action("editorwindow_forward"));
    m_contextMenu->addSeparator();
    if (ac->action("editorwindow_slideshow"))
        m_contextMenu->addAction(ac->action("editorwindow_slideshow"));
    if (ac->action("editorwindow_rotate_left"))
        m_contextMenu->addAction(ac->action("editorwindow_rotate_left"));
    if (ac->action("editorwindow_rotate_right"))
        m_contextMenu->addAction(ac->action("editorwindow_rotate_right"));
    if (ac->action("editorwindow_crop"))
        m_contextMenu->addAction(ac->action("editorwindow_crop"));
    m_contextMenu->addSeparator();
    if (ac->action("editorwindow_delete"))
        m_contextMenu->addAction(ac->action("editorwindow_delete"));
}

void EditorWindow::setupStandardConnections()
{
    // -- Canvas connections ------------------------------------------------

    connect(m_canvas, SIGNAL(signalToggleOffFitToWindow()),
            this, SLOT(slotToggleOffFitToWindow()));

    connect(m_canvas, SIGNAL(signalShowNextImage()),
            this, SLOT(slotForward()));

    connect(m_canvas, SIGNAL(signalShowPrevImage()),
            this, SLOT(slotBackward()));

    connect(m_canvas, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(m_stackView, SIGNAL(signalZoomChanged(bool, bool, double)),
            this, SLOT(slotZoomChanged(bool, bool, double)));

    connect(m_canvas, SIGNAL(signalChanged()),
            this, SLOT(slotChanged()));

    connect(m_canvas, SIGNAL(signalUndoStateChanged(bool, bool, bool)),
            this, SLOT(slotUndoStateChanged(bool, bool, bool)));

    connect(m_canvas, SIGNAL(signalSelected(bool)),
            this, SLOT(slotSelected(bool)));

    connect(m_canvas, SIGNAL(signalPrepareToLoad()),
            this, SLOT(slotPrepareToLoad()));

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

    connect(m_nameLabel, SIGNAL(signalCancelButtonPressed()),
            d->toolIface, SLOT(slotToolAborted()));
}

void EditorWindow::setupStandardActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    m_backwardAction = KStandardAction::back(this, SLOT(slotBackward()), this);
    actionCollection()->addAction("editorwindow_backward", m_backwardAction);
    m_backwardAction->setShortcut( KShortcut(Qt::Key_PageUp, Qt::Key_Backspace) );

    m_forwardAction = KStandardAction::forward(this, SLOT(slotForward()), this);
    actionCollection()->addAction("editorwindow_forward", m_forwardAction);
    m_forwardAction->setShortcut( KShortcut(Qt::Key_PageDown, Qt::Key_Space) );

    m_firstAction = new KAction(KIcon("go-first"), i18n("&First"), this);
    m_firstAction->setShortcut(KStandardShortcut::begin());
    connect(m_firstAction, SIGNAL(triggered()), this, SLOT(slotFirst()));
    actionCollection()->addAction("editorwindow_first", m_firstAction);

    m_lastAction = new KAction(KIcon("go-last"), i18n("&Last"), this);
    m_lastAction->setShortcut(KStandardShortcut::end());
    connect(m_lastAction, SIGNAL(triggered()), this, SLOT(slotLast()));
    actionCollection()->addAction("editorwindow_last", m_lastAction);

    m_saveAction = KStandardAction::save(this, SLOT(slotSave()), this);
    actionCollection()->addAction("editorwindow_save", m_saveAction);

    m_saveAsAction = KStandardAction::saveAs(this, SLOT(slotSaveAs()), this);
    actionCollection()->addAction("editorwindow_saveas", m_saveAsAction);

    m_revertAction = KStandardAction::revert(this, SLOT(slotRevert()), this);
    actionCollection()->addAction("editorwindow_revert", m_revertAction);

    m_saveAction->setEnabled(false);
    m_saveAsAction->setEnabled(false);
    m_revertAction->setEnabled(false);

    d->filePrintAction = new KAction(KIcon("document-print-frame"), i18n("Print Image..."), this);
    d->filePrintAction->setShortcut(Qt::CTRL+Qt::Key_P);
    connect(d->filePrintAction, SIGNAL(triggered()), this, SLOT(slotFilePrint()));
    actionCollection()->addAction("editorwindow_print", d->filePrintAction);

    m_fileDeleteAction = new KAction(KIcon("user-trash"), i18n("Move to Trash"), this);
    m_fileDeleteAction->setShortcut(Qt::Key_Delete);
    connect(m_fileDeleteAction, SIGNAL(triggered()), this, SLOT(slotDeleteCurrentItem()));
    actionCollection()->addAction("editorwindow_delete", m_fileDeleteAction);

    KAction* closeAction = KStandardAction::close(this, SLOT(close()), this);
    actionCollection()->addAction("editorwindow_close", closeAction);

    // -- Standard 'Edit' menu actions ---------------------------------------------

    d->copyAction = KStandardAction::copy(m_canvas, SLOT(slotCopy()), this);
    actionCollection()->addAction("editorwindow_copy", d->copyAction);
    d->copyAction->setEnabled(false);

    m_undoAction = new KToolBarPopupAction(KIcon("edit-undo"), i18n("Undo"), this);
    m_undoAction->setShortcut(KStandardShortcut::undo());
    m_undoAction->setEnabled(false);
    actionCollection()->addAction("editorwindow_undo", m_undoAction);

    connect(m_undoAction->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowUndoMenu()));

    // we are using a signal mapper to identify which of a bunch of actions was triggered
    d->undoSignalMapper = new QSignalMapper(this);

    // connect mapper to view
    connect(d->undoSignalMapper, SIGNAL(mapped(int)),
            m_canvas, SLOT(slotUndo(int)));

    // connect simple undo action
    connect(m_undoAction, SIGNAL(triggered()), d->undoSignalMapper, SLOT(map()));
    d->undoSignalMapper->setMapping(m_undoAction, 1);

    m_redoAction = new KToolBarPopupAction(KIcon("edit-redo"), i18n("Redo"), this);
    m_redoAction->setShortcut(KStandardShortcut::redo());
    m_redoAction->setEnabled(false);
    actionCollection()->addAction("editorwindow_redo", m_redoAction);

    connect(m_redoAction->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowRedoMenu()));

    d->redoSignalMapper = new QSignalMapper(this);

    connect(d->redoSignalMapper, SIGNAL(mapped(int)),
            m_canvas, SLOT(slotRedo(int)));

    connect(m_redoAction, SIGNAL(triggered()), d->redoSignalMapper, SLOT(map()));
    d->redoSignalMapper->setMapping(m_redoAction, 1);

    d->selectAllAction = new KAction(i18n("Select All"), this);
    d->selectAllAction->setShortcut(Qt::CTRL+Qt::Key_A);
    connect(d->selectAllAction, SIGNAL(triggered()), m_canvas, SLOT(slotSelectAll()));
    actionCollection()->addAction("editorwindow_selectAll", d->selectAllAction);

    d->selectNoneAction = new KAction(i18n("Select None"), this);
    d->selectNoneAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_A);
    connect(d->selectNoneAction, SIGNAL(triggered()), m_canvas, SLOT(slotSelectNone()));
    actionCollection()->addAction("editorwindow_selectNone", d->selectNoneAction);

    // -- Standard 'View' menu actions ---------------------------------------------

    d->zoomPlusAction = KStandardAction::zoomIn(this, SLOT(slotIncreaseZoom()), this);
    actionCollection()->addAction("editorwindow_zoomplus", d->zoomPlusAction);
    d->zoomPlusAction->setShortcut(QKeySequence(Qt::Key_Plus));

    d->zoomMinusAction = KStandardAction::zoomOut(this, SLOT(slotDecreaseZoom()), this);
    actionCollection()->addAction("editorwindow_zoomminus", d->zoomMinusAction);
    d->zoomMinusAction->setShortcut(QKeySequence(Qt::Key_Minus));

    d->zoomTo100percents = new KAction(KIcon("zoom-original"), i18n("Zoom to 100%"), this);
    d->zoomTo100percents->setShortcut(Qt::ALT+Qt::CTRL+Qt::Key_0);       // NOTE: Photoshop 7 use ALT+CTRL+0
    connect(d->zoomTo100percents, SIGNAL(triggered()), this, SLOT(slotZoomTo100Percents()));
    actionCollection()->addAction("editorwindow_zoomto100percents", d->zoomTo100percents);

    d->zoomFitToWindowAction = new KToggleAction(KIcon("zoom-fit-best"), i18n("Fit to &Window"), this);
    d->zoomFitToWindowAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_E); // NOTE: Gimp 2 use CTRL+SHIFT+E.
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), this, SLOT(slotToggleFitToWindow()));
    actionCollection()->addAction("editorwindow_zoomfit2window", d->zoomFitToWindowAction);

    d->zoomFitToSelectAction = new KAction(KIcon("zoom-select-fit"), i18n("Fit to &Selection"), this);
    d->zoomFitToSelectAction->setShortcut(Qt::ALT+Qt::CTRL+Qt::Key_S);   // NOTE: Photoshop 7 use ALT+CTRL+0
    connect(d->zoomFitToSelectAction, SIGNAL(triggered()), this, SLOT(slotFitToSelect()));
    actionCollection()->addAction("editorwindow_zoomfit2select", d->zoomFitToSelectAction);
    d->zoomFitToSelectAction->setEnabled(false);
    d->zoomFitToSelectAction->setWhatsThis(i18n("This option can be used to zoom the image to the "
                                                "current selection area."));

    d->zoomCombo = new KComboBox(true);
    d->zoomCombo->setDuplicatesEnabled(false);
    d->zoomCombo->setFocusPolicy(Qt::ClickFocus);
    d->zoomCombo->setInsertPolicy(QComboBox::NoInsert);
    d->zoomCombo->insertItem(-1, QString("10%"));
    d->zoomCombo->insertItem(-1, QString("25%"));
    d->zoomCombo->insertItem(-1, QString("50%"));
    d->zoomCombo->insertItem(-1, QString("75%"));
    d->zoomCombo->insertItem(-1, QString("100%"));
    d->zoomCombo->insertItem(-1, QString("150%"));
    d->zoomCombo->insertItem(-1, QString("200%"));
    d->zoomCombo->insertItem(-1, QString("300%"));
    d->zoomCombo->insertItem(-1, QString("450%"));
    d->zoomCombo->insertItem(-1, QString("600%"));
    d->zoomCombo->insertItem(-1, QString("800%"));
    d->zoomCombo->insertItem(-1, QString("1200%"));

    connect(d->zoomCombo, SIGNAL(activated(int)),
            this, SLOT(slotZoomSelected()) );

    connect(d->zoomCombo, SIGNAL(returnPressed(const QString&)),
            this, SLOT(slotZoomTextChanged(const QString &)) );

    d->zoomComboAction = new QWidgetAction(this);
    d->zoomComboAction->setDefaultWidget(d->zoomCombo);
    d->zoomComboAction->setText(i18n("Zoom"));
    actionCollection()->addAction("editorwindow_zoomto", d->zoomComboAction);

    m_fullScreenAction = KStandardAction::fullScreen(this, SLOT(slotToggleFullScreen()), this, this);
    actionCollection()->addAction("editorwindow_fullscreen", m_fullScreenAction);

    d->slideShowAction = new KAction(KIcon("view-presentation"), i18n("Slideshow"), this);
    d->slideShowAction->setShortcut(Qt::Key_F9);
    connect(d->slideShowAction, SIGNAL(triggered()), this, SLOT(slotToggleSlideShow()));
    actionCollection()->addAction("editorwindow_slideshow", d->slideShowAction);

    d->viewUnderExpoAction = new KToggleAction(KIcon("underexposure"),
                                               i18n("Under-Exposure Indicator"), this);
    d->viewUnderExpoAction->setShortcut(Qt::Key_F10);
    connect(d->viewUnderExpoAction, SIGNAL(triggered()), this, SLOT(slotToggleUnderExposureIndicator()));
    actionCollection()->addAction("editorwindow_underexposure", d->viewUnderExpoAction);

    d->viewOverExpoAction = new KToggleAction(KIcon("overexposure"),
                                              i18n("Over-Exposure Indicator"), this);
    d->viewOverExpoAction->setShortcut(Qt::Key_F11);
    connect(d->viewOverExpoAction, SIGNAL(triggered()), this, SLOT(slotToggleOverExposureIndicator()));
    actionCollection()->addAction("editorwindow_overexposure", d->viewOverExpoAction);

    d->viewCMViewAction = new KToggleAction(KIcon("video-display"), i18n("Color-Managed View"), this);
    d->viewCMViewAction->setShortcut(Qt::Key_F12);
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    actionCollection()->addAction("editorwindow_cmview", d->viewCMViewAction);

    m_showBarAction = new KToggleAction(KIcon("view-choose"), i18n("Show Thumbnails"), this);
    m_showBarAction->setShortcut(Qt::CTRL+Qt::Key_T);
    connect(m_showBarAction, SIGNAL(triggered()), this, SLOT(slotToggleShowBar()));
    actionCollection()->addAction("editorwindow_showthumbs", m_showBarAction);

    // -- Standard 'Transform' menu actions ---------------------------------------------

    d->cropAction = new KAction(KIcon("transform-crop-and-resize"), i18n("Crop"), this);
    d->cropAction->setShortcut(Qt::CTRL+Qt::Key_X);
    connect(d->cropAction, SIGNAL(triggered()), m_canvas, SLOT(slotCrop()));
    actionCollection()->addAction("editorwindow_crop", d->cropAction);
    d->cropAction->setEnabled(false);
    d->cropAction->setWhatsThis(i18n("This option can be used to crop the image. "
                                     "Select a region of the image to enable this action."));

    // -- Standard 'Flip' menu actions ---------------------------------------------

    d->flipHorizAction = new KAction(KIcon("object-flip-horizontal"), i18n("Flip Horizontally"), this);
    d->flipHorizAction->setShortcut(Qt::CTRL+Qt::Key_Asterisk);
    connect(d->flipHorizAction, SIGNAL(triggered()), m_canvas, SLOT(slotFlipHoriz()));
    actionCollection()->addAction("editorwindow_flip_horiz", d->flipHorizAction);
    d->flipHorizAction->setEnabled(false);

    d->flipVertAction = new KAction(KIcon("object-flip-vertical"), i18n("Flip Vertically"), this);
    d->flipVertAction->setShortcut(Qt::CTRL+Qt::Key_Slash);
    connect(d->flipVertAction, SIGNAL(triggered()), m_canvas, SLOT(slotFlipVert()));
    actionCollection()->addAction("editorwindow_flip_vert", d->flipVertAction);
    d->flipVertAction->setEnabled(false);

    // -- Standard 'Rotate' menu actions ----------------------------------------

    d->rotateLeftAction = new KAction(KIcon("object-rotate-left"), i18n("Rotate Left"), this);
    d->rotateLeftAction->setShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_Left);
    connect(d->rotateLeftAction, SIGNAL(triggered()), m_canvas, SLOT(slotRotate270()));
    actionCollection()->addAction("editorwindow_rotate_left", d->rotateLeftAction);
    d->rotateLeftAction->setEnabled(false);

    d->rotateRightAction = new KAction(KIcon("object-rotate-right"), i18n("Rotate Right"), this);
    d->rotateRightAction->setShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_Right);
    connect(d->rotateRightAction, SIGNAL(triggered()), m_canvas, SLOT(slotRotate90()));
    actionCollection()->addAction("editorwindow_rotate_right", d->rotateRightAction);
    d->rotateRightAction->setEnabled(false);

    // -- Standard 'Configure' menu actions ----------------------------------------

    d->showMenuBarAction = KStandardAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());
    d->showMenuBarAction->setChecked(!menuBar()->isHidden());  // NOTE: workaround for B.K.O #171080

    KStandardAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStandardAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStandardAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // ---------------------------------------------------------------------------------

    m_themeMenuAction = new KSelectAction(i18n("&Themes"), this);
    m_themeMenuAction->setItems(ThemeEngine::instance()->themeNames());
    connect(m_themeMenuAction, SIGNAL(triggered(const QString&)),
            this, SLOT(slotChangeTheme(const QString&)));
    actionCollection()->addAction("theme_menu", m_themeMenuAction);

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // -- Standard 'Help' menu actions ---------------------------------------------

    d->donateMoneyAction = new KAction(i18n("Donate Money..."), this);
    connect(d->donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    actionCollection()->addAction("editorwindow_donatemoney", d->donateMoneyAction);

    d->contributeAction = new KAction(i18n("Contribute..."), this);
    connect(d->contributeAction, SIGNAL(triggered()), this, SLOT(slotContribute()));
    actionCollection()->addAction("editorwindow_contribute", d->contributeAction);

    d->rawCameraListAction = new KAction(KIcon("kdcraw"), i18n("Supported RAW Cameras"), this);
    connect(d->rawCameraListAction, SIGNAL(triggered()), this, SLOT(slotRawCameraList()));
    actionCollection()->addAction("editorwindow_rawcameralist", d->rawCameraListAction);

    d->libsInfoAction = new KAction(KIcon("help-about"), i18n("Components Information"), this);
    connect(d->libsInfoAction, SIGNAL(triggered()), this, SLOT(slotComponentsInfo()));
    actionCollection()->addAction("editorwindow_librariesinfo", d->libsInfoAction);

    // -- Keyboard-only actions added to <MainWindow> ------------------------------

//    KAction *exitFullscreenAction = new KAction(i18n("Exit Full Screen"), this);
//    actionCollection()->addAction("editorwindow_exitfullscreen", exitFullscreenAction);
//    exitFullscreenAction->setShortcut( QKeySequence(Qt::Key_Escape) );
//    connect(exitFullscreenAction, SIGNAL(triggered()), this, SLOT(slotEscapePressed()));

    KAction *closeToolAction = new KAction(i18n("Close Tool"), this);
    actionCollection()->addAction("editorwindow_closetool", closeToolAction);
    closeToolAction->setShortcut( QKeySequence(Qt::Key_Escape) );
    connect(closeToolAction, SIGNAL(triggered()), this, SLOT(slotCloseTool()));

    KAction *altBackwardAction = new KAction(i18n("Previous Image"), this);
    actionCollection()->addAction("editorwindow_backward_shift_space", altBackwardAction);
    altBackwardAction->setShortcut( KShortcut(Qt::SHIFT+Qt::Key_Space) );
    connect(altBackwardAction, SIGNAL(triggered()), this, SLOT(slotBackward()));

    m_animLogo = new DLogoAction(this);
    actionCollection()->addAction("logo_action", m_animLogo);
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
    d->selectLabel->setToolTip( i18n("Information about current selection area"));

    m_resLabel  = new QLabel(statusBar());
    m_resLabel->setAlignment(Qt::AlignCenter);
    m_resLabel->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(m_resLabel, 100);
    m_resLabel->setToolTip( i18n("Information about image size"));

    QSize iconSize(fontMetrics().height()+2, fontMetrics().height()+2);
    d->underExposureIndicator = new QToolButton(statusBar());
    d->underExposureIndicator->setIcon( SmallIcon("underexposure"));
    d->underExposureIndicator->setCheckable(true);
    d->underExposureIndicator->setMaximumSize(iconSize);
    statusBar()->addPermanentWidget(d->underExposureIndicator);

    d->overExposureIndicator = new QToolButton(statusBar());
    d->overExposureIndicator->setIcon(SmallIcon("overexposure"));
    d->overExposureIndicator->setCheckable(true);
    d->overExposureIndicator->setMaximumSize(iconSize);
    statusBar()->addPermanentWidget(d->overExposureIndicator);

    d->cmViewIndicator = new QToolButton(statusBar());
    d->cmViewIndicator->setIcon(SmallIcon("video-display"));
    d->cmViewIndicator->setCheckable(true);
    d->cmViewIndicator->setMaximumSize(iconSize);
    statusBar()->addPermanentWidget(d->cmViewIndicator);

    connect(d->underExposureIndicator, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleUnderExposureIndicator()));

    connect(d->overExposureIndicator, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleOverExposureIndicator()));

    connect(d->cmViewIndicator, SIGNAL(toggled(bool)),
            this, SLOT(slotToggleColorManagedView()));
}

void EditorWindow::printImage(const KUrl& /*url*/)
{
    uchar* ptr      = m_canvas->interface()->getImage();
    int w           = m_canvas->interface()->origWidth();
    int h           = m_canvas->interface()->origHeight();
    bool hasAlpha   = m_canvas->interface()->hasAlpha();
    bool sixteenBit = m_canvas->interface()->sixteenBit();

    if (!ptr || !w || !h)
        return;

    DImg image(w, h, sixteenBit, hasAlpha, ptr);

    PrintHelper printHelp(this);
    printHelp.print(image);
}

void EditorWindow::slotEditKeys()
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection(actionCollection(), i18nc("general editor shortcuts", "General"));
    dialog.addCollection(d->imagepluginsActionCollection, i18nc("imageplugins shortcuts", "Image Plugins"));
    dialog.configure();
}

void EditorWindow::slotAboutToShowUndoMenu()
{
    m_undoAction->menu()->clear();
    QStringList titles;
    m_canvas->getUndoHistory(titles);

    for (int i=0; i<titles.size(); ++i)
    {
        QAction *action =
            m_undoAction->menu()->addAction(titles[i], d->undoSignalMapper, SLOT(map()));
        d->undoSignalMapper->setMapping(action, i + 1);
    }
}

void EditorWindow::slotAboutToShowRedoMenu()
{
    m_redoAction->menu()->clear();
    QStringList titles;
    m_canvas->getRedoHistory(titles);

    for (int i=0; i<titles.size(); ++i)
    {
        QAction *action = m_redoAction->menu()->addAction(titles[i], d->redoSignalMapper, SLOT(map()));
        d->redoSignalMapper->setMapping(action, i + 1);
    }
}

void EditorWindow::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group("ImageViewer Settings"));
    KEditToolBar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void EditorWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group("ImageViewer Settings"));
}

void EditorWindow::slotIncreaseZoom()
{
    m_stackView->increaseZoom();
}

void EditorWindow::slotDecreaseZoom()
{
    m_stackView->decreaseZoom();
}

void EditorWindow::slotToggleFitToWindow()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomComboAction->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_stackView->toggleFitToWindow();
}

void EditorWindow::slotFitToSelect()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomComboAction->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_stackView->fitToSelect();
}

void EditorWindow::slotZoomTo100Percents()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomComboAction->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_stackView->zoomTo100Percents();
}

void EditorWindow::slotZoomSelected()
{
    QString txt = d->zoomCombo->currentText();
    txt         = txt.left(txt.indexOf('%'));
    slotZoomTextChanged(txt);
}

void EditorWindow::slotZoomTextChanged(const QString& txt)
{
    bool r      = false;
    double zoom = KGlobal::locale()->readNumber(txt, &r) / 100.0;
    if (r && zoom > 0.0)
        m_stackView->setZoomFactor(zoom);
}

void EditorWindow::slotZoomChanged(bool isMax, bool isMin, double zoom)
{
    d->zoomPlusAction->setEnabled(!isMax);
    d->zoomMinusAction->setEnabled(!isMin);

    d->zoomCombo->blockSignals(true);
    d->zoomCombo->setCurrentIndex(-1);
    d->zoomCombo->setEditText(QString::number(lround(zoom*100.0)) + QString("%"));
    d->zoomCombo->blockSignals(false);
}

void EditorWindow::slotToggleOffFitToWindow()
{
    d->zoomFitToWindowAction->blockSignals(true);
    d->zoomFitToWindowAction->setChecked(false);
    d->zoomFitToWindowAction->blockSignals(false);
}

void EditorWindow::slotEscapePressed()
{
    if (m_fullScreen)
        m_fullScreenAction->activate(QAction::Trigger);
}

void EditorWindow::loadImagePlugins()
{
    if (d->imagepluginsActionCollection)
    {
        d->imagepluginsActionCollection->clear();
        delete d->imagepluginsActionCollection;
    }
    d->imagepluginsActionCollection = new KActionCollection(this, KGlobal::mainComponent());

    QList<ImagePlugin *> pluginList = m_imagePluginLoader->pluginList();

    foreach (ImagePlugin *plugin, pluginList)
    {
        if (plugin)
        {
            guiFactory()->addClient(plugin);
            plugin->setEnabledSelectionActions(false);

            // add actions to imagepluginsActionCollection
            foreach (QAction* action, plugin->actionCollection()->actions())
            {
                d->imagepluginsActionCollection->addAction(action->objectName(), action);
            }
        }
        else
        {
            kDebug(50003) << "Invalid plugin to add!";
        }
    }

    // load imagepluginsActionCollection settings
    d->imagepluginsActionCollection->readSettings();
}

void EditorWindow::unLoadImagePlugins()
{
    if (d->imagepluginsActionCollection)
    {
        d->imagepluginsActionCollection->clear();
        delete d->imagepluginsActionCollection;
    }

    QList<ImagePlugin *> pluginList = m_imagePluginLoader->pluginList();

    foreach (ImagePlugin *plugin, pluginList)
    {
        if (plugin)
        {
            guiFactory()->removeClient(plugin);
            plugin->setEnabledSelectionActions(false);
        }
    }
}

void EditorWindow::readStandardSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");

    // Restore Canvas layout
    if (group.hasKey("Vertical Splitter Sizes") && m_vSplitter)
    {
        QByteArray state;
        state = group.readEntry("Vertical Splitter State", state);
        m_vSplitter->restoreState(QByteArray::fromBase64(state));
    }

    // Restore full screen Mode
    if (group.readEntry("FullScreen", false))
    {
        m_fullScreenAction->activate(QAction::Trigger);
        m_fullScreen = true;
    }

    // Restore Auto zoom action
    bool autoZoom = group.readEntry("AutoZoom", true);
    if (autoZoom)
        d->zoomFitToWindowAction->activate(QAction::Trigger);

    m_showBarAction->setChecked(group.readEntry("Show Thumbnails", true));
    slotToggleShowBar();
}

void EditorWindow::applyStandardSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();

    // -- Settings for Color Management stuff ----------------------------------------------

    KConfigGroup group = config->group("Color Management");

    d->ICCSettings->readFromConfig(group);

    d->viewCMViewAction->blockSignals(true);
    d->cmViewIndicator->blockSignals(true);
    d->viewCMViewAction->setEnabled(d->ICCSettings->enableCM);
    d->viewCMViewAction->setChecked(d->ICCSettings->useManagedView);
    d->cmViewIndicator->setEnabled(d->ICCSettings->enableCM);
    d->cmViewIndicator->setChecked(d->ICCSettings->useManagedView);
    setColorManagedViewIndicatorToolTip(d->ICCSettings->enableCM, d->ICCSettings->useManagedView);
    m_canvas->setICCSettings(d->ICCSettings);
    d->viewCMViewAction->blockSignals(false);
    d->cmViewIndicator->blockSignals(false);

    // -- JPEG, PNG, TIFF JPEG2000 files format settings --------------------------------------

    group = config->group("ImageViewer Settings");

    // JPEG quality slider settings : 1 - 100 ==> libjpeg settings : 25 - 100.
    m_IOFileSettings->JPEGCompression     = (int)((75.0/100.0)*
                                                 (float)group.readEntry("JPEGCompression", 75)
                                                 + 26.0 - (75.0/100.0));

    m_IOFileSettings->JPEGSubSampling     = group.readEntry("JPEGSubSampling", 1);  // Medium subsampling

    // PNG compression slider settings : 1 - 9 ==> libpng settings : 100 - 1.
    m_IOFileSettings->PNGCompression      = (int)(((1.0-100.0)/8.0)*
                                                 (float)group.readEntry("PNGCompression", 1)
                                                 + 100.0 - ((1.0-100.0)/8.0));

    // TIFF compression setting.
    m_IOFileSettings->TIFFCompression     = group.readEntry("TIFFCompression", false);

    // JPEG2000 quality slider settings : 1 - 100
    m_IOFileSettings->JPEG2000Compression = group.readEntry("JPEG2000Compression", 100);

    // JPEG2000 LossLess setting.
    m_IOFileSettings->JPEG2000LossLess    = group.readEntry("JPEG2000LossLess", true);

    // PGF quality slider settings : 1 - 9
    m_IOFileSettings->PGFCompression      = group.readEntry("PGFCompression", 3);

    // PGF LossLess setting.
    m_IOFileSettings->PGFLossLess         = group.readEntry("PGFLossLess", true);

    // -- RAW images decoding settings ------------------------------------------------------

    // If digiKam Color Management is enable, no need to correct color of decoded RAW image,
    // else, sRGB color workspace will be used.

    if (d->ICCSettings->enableCM)
        m_IOFileSettings->rawDecodingSettings.outputColorSpace = DRawDecoding::RAWCOLOR;
    else
        m_IOFileSettings->rawDecodingSettings.outputColorSpace = DRawDecoding::SRGB;

    m_IOFileSettings->rawDecodingSettings.sixteenBitsImage        = group.readEntry("SixteenBitsImage", false);
    m_IOFileSettings->rawDecodingSettings.whiteBalance            = (DRawDecoding::WhiteBalance)group.readEntry("WhiteBalance",
                                                                    (int)DRawDecoding::CAMERA);
    m_IOFileSettings->rawDecodingSettings.customWhiteBalance      = group.readEntry("CustomWhiteBalance", 6500);
    m_IOFileSettings->rawDecodingSettings.customWhiteBalanceGreen = group.readEntry("CustomWhiteBalanceGreen", 1.0);
    m_IOFileSettings->rawDecodingSettings.RGBInterpolate4Colors   = group.readEntry("RGBInterpolate4Colors", false);
    m_IOFileSettings->rawDecodingSettings.DontStretchPixels       = group.readEntry("DontStretchPixels", false);
    m_IOFileSettings->rawDecodingSettings.enableNoiseReduction    = group.readEntry("EnableNoiseReduction", false);
    m_IOFileSettings->rawDecodingSettings.unclipColors            = group.readEntry("UnclipColors", 0);
    m_IOFileSettings->rawDecodingSettings.RAWQuality              = (DRawDecoding::DecodingQuality)
                                                                    group.readEntry("RAWQuality",
                                                                    (int)DRawDecoding::BILINEAR);
    m_IOFileSettings->rawDecodingSettings.NRThreshold             = group.readEntry("NRThreshold", 100);
    m_IOFileSettings->rawDecodingSettings.enableCACorrection      = group.readEntry("EnableCACorrection", false);
    m_IOFileSettings->rawDecodingSettings.caMultiplier[0]         = group.readEntry("caRedMultiplier", 1.0);
    m_IOFileSettings->rawDecodingSettings.caMultiplier[1]         = group.readEntry("caBlueMultiplier", 1.0);
    m_IOFileSettings->rawDecodingSettings.brightness              = group.readEntry("RAWBrightness", 1.0);
    m_IOFileSettings->rawDecodingSettings.medianFilterPasses      = group.readEntry("MedianFilterPasses", 0);
#if KDCRAW_VERSION >= 0x000500
    m_IOFileSettings->rawDecodingSettings.autoBrightness          = group.readEntry("AutoBrightness", true);
#endif
    m_IOFileSettings->useRAWImport                                = group.readEntry("UseRawImportTool", false);

    // -- GUI Settings -------------------------------------------------------

    m_splitter->restoreState(group);

    d->fullScreenHideToolBar = group.readEntry("FullScreen Hide ToolBar", false);
    m_fullScreenHideThumbBar = group.readEntry("FullScreenHideThumbBar", true);

    slotThemeChanged();

    // -- Exposure Indicators Settings ---------------------------------------

    QColor black(Qt::black);
    QColor white(Qt::white);
    d->exposureSettings->underExposureIndicator = group.readEntry("UnderExposureIndicator", false);
    d->exposureSettings->overExposureIndicator  = group.readEntry("OverExposureIndicator", false);
    d->exposureSettings->underExposureColor     = group.readEntry("UnderExposureColor", white);
    d->exposureSettings->overExposureColor      = group.readEntry("OverExposureColor", black);

    d->viewUnderExpoAction->setChecked(d->exposureSettings->underExposureIndicator);
    d->viewOverExpoAction->setChecked(d->exposureSettings->overExposureIndicator);
    d->underExposureIndicator->setChecked(d->exposureSettings->underExposureIndicator);
    d->overExposureIndicator->setChecked(d->exposureSettings->overExposureIndicator);
    setUnderExposureToolTip(d->exposureSettings->underExposureIndicator);
    setOverExposureToolTip(d->exposureSettings->overExposureIndicator);
    m_canvas->setExposureSettings(d->exposureSettings);
}

void EditorWindow::saveStandardSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");

    group.writeEntry("AutoZoom", d->zoomFitToWindowAction->isChecked());
    m_splitter->saveState(group);
    if (m_vSplitter)
        group.writeEntry("Vertical Splitter State", m_vSplitter->saveState().toBase64());

    group.writeEntry("Show Thumbnails", m_showBarAction->isChecked());
    group.writeEntry("FullScreen", m_fullScreenAction->isChecked());
    group.writeEntry("UnderExposureIndicator", d->exposureSettings->underExposureIndicator);
    group.writeEntry("OverExposureIndicator", d->exposureSettings->overExposureIndicator);

    config->sync();
}

/** Method used by Editor Tools. Only Zoom+ and Zoom- are currently supported.
    TODO: Fix this behavour when editor tool preview widgets will be factored.
 */
void EditorWindow::toggleZoomActions(bool val)
{
    d->zoomMinusAction->setEnabled(val);
    d->zoomPlusAction->setEnabled(val);
}

void EditorWindow::toggleStandardActions(bool val)
{
    d->zoomComboAction->setEnabled(val);
    d->zoomTo100percents->setEnabled(val);
    d->zoomFitToWindowAction->setEnabled(val);
    d->zoomFitToSelectAction->setEnabled(val);
    toggleZoomActions(val);

    d->rotateLeftAction->setEnabled(val);
    d->rotateRightAction->setEnabled(val);
    d->flipHorizAction->setEnabled(val);
    d->flipVertAction->setEnabled(val);
    d->filePrintAction->setEnabled(val);
    m_fileDeleteAction->setEnabled(val);
    m_saveAsAction->setEnabled(val);
    d->selectAllAction->setEnabled(val);
    d->selectNoneAction->setEnabled(val);
    d->slideShowAction->setEnabled(val);

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

    QList<ImagePlugin *> pluginList = m_imagePluginLoader->pluginList();

    foreach (ImagePlugin *plugin, pluginList)
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
        setWindowState( windowState() & ~Qt::WindowFullScreen ); // reset

        m_canvas->setBackgroundColor(m_bgColor);

        menuBar()->show();
        statusBar()->show();
        showToolBars();

        if (d->removeFullScreenButton)
        {
            QList<KToolBar *> toolbars = toolBars();
            foreach(KToolBar *toolbar, toolbars)
            {
                // name is set in ui.rc XML file
                if (toolbar->objectName() == "ToolBar")
                {
                    toolbar->removeAction(m_fullScreenAction);
                    break;
                }
            }
        }

        toggleGUI2FullScreen();
        m_fullScreen = false;
    }
    else  // go to fullscreen
    {
        m_canvas->setBackgroundColor(QColor(Qt::black));

        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();

        if (d->fullScreenHideToolBar)
        {
            hideToolBars();
        }
        else
        {
            showToolBars();

            QList<KToolBar *> toolbars = toolBars();
            KToolBar *mainToolbar = 0;
            foreach(KToolBar *toolbar, toolbars)
            {
                if (toolbar->objectName() == "ToolBar")
                {
                    mainToolbar = toolbar;
                    break;
                }
            }

            // add fullscreen action if necessary
            if ( mainToolbar && !mainToolbar->actions().contains(m_fullScreenAction) )
            {
                mainToolbar->addAction(m_fullScreenAction);
                d->removeFullScreenButton=true;
            }
            else
            {
                // If FullScreen button is enabled in toolbar settings,
                // we shall not remove it when leaving of fullscreen mode.
                d->removeFullScreenButton=false;
            }
        }

        toggleGUI2FullScreen();
        setWindowState( windowState() | Qt::WindowFullScreen ); // set
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

bool EditorWindow::promptForOverWrite()
{
    QFileInfo fi(m_canvas->currentImageFilePath());
    QString warnMsg(i18n("About to overwrite file \"%1\"\nAre you sure?", fi.fileName()));
    return (KMessageBox::warningContinueCancel(this,
                                               warnMsg,
                                               i18n("Warning"),
                                               KGuiItem(i18n("Overwrite")),
                                               KStandardGuiItem::cancel(),
                                               QString("editorWindowSaveOverwrite"))
            ==  KMessageBox::Continue);
}

bool EditorWindow::promptUserSave(const KUrl& url, SaveOrSaveAs saveOrSaveAs)
{
    if (m_saveAction->isEnabled())
    {
        // if window is minimized, show it
        if (isMinimized())
        {
            KWindowSystem::unminimizeWindow(winId());
        }

        int result = KMessageBox::warningYesNoCancel(this,
                                  i18n("The image '%1' has been modified.\n"
                                       "Do you want to save it?",
                                       url.fileName()),
                                  QString(),
                                  KStandardGuiItem::save(),
                                  KStandardGuiItem::discard());

        if (result == KMessageBox::Yes)
        {
            bool saving = false;

            switch (saveOrSaveAs)
            {
                case AskIfNeeded:
                    if (m_canvas->isReadOnly())
                        saving = saveAs();
                    else if (promptForOverWrite())
                        saving = save();
                    break;
                case OverwriteWithoutAsking:
                    if (m_canvas->isReadOnly())
                        saving = saveAs();
                    else
                        saving = save();
                    break;
                case AlwaysSaveAs:
                    saving = saveAs();
                    break;
            }

            // save and saveAs return false if they were canceled and did not enter saving at all
            // In this case, do not call enterWaitingLoop because quitWaitingloop will not be called.
            if (saving)
            {
                // Waiting for asynchronous image file saving operation running in separate thread.
                m_savingContext->synchronizingState = SavingContextContainer::SynchronousSaving;
                enterWaitingLoop();
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
        // Waiting for asynchronous image file saving operation running in separate thread.
        m_savingContext->synchronizingState = SavingContextContainer::SynchronousSaving;
        KMessageBox::queuedMessageBox(this,
                                      KMessageBox::Information,
                                      i18n("Please wait while the image is being saved..."));
        enterWaitingLoop();
        m_savingContext->synchronizingState = SavingContextContainer::NormalSaving;
    }
    return true;
}

void EditorWindow::enterWaitingLoop()
{
    d->waitingLoop->exec(QEventLoop::ExcludeUserInputEvents);
}

void EditorWindow::quitWaitingLoop()
{
    d->waitingLoop->quit();
}

void EditorWindow::slotSelected(bool val)
{
    // Update menu actions.
    d->cropAction->setEnabled(val);
    d->zoomFitToSelectAction->setEnabled(val);
    d->copyAction->setEnabled(val);

    QList<ImagePlugin*> pluginList = m_imagePluginLoader->pluginList();
    foreach (ImagePlugin *plugin, pluginList)
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
    QList<KToolBar *> toolbars = toolBars();
    foreach(KToolBar *toolbar, toolbars)
    {
        toolbar->hide();
    }
}

void EditorWindow::showToolBars()
{
    QList<KToolBar *> toolbars = toolBars();
    foreach(KToolBar *toolbar, toolbars)
    {
        toolbar->show();
    }
}

void EditorWindow::slotPrepareToLoad()
{
    // Disable actions as appropriate during loading
    emit signalNoCurrentItem();
    toggleActions(false);
    slotUpdateItemInfo();
}

void EditorWindow::slotLoadingStarted(const QString& /*filename*/)
{
    setCursor(Qt::WaitCursor);
    m_animLogo->start();
    m_nameLabel->progressBarMode(StatusProgressBar::ProgressBarMode, i18n("Loading: "));
}

void EditorWindow::slotLoadingFinished(const QString& /*filename*/, bool success)
{
    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);
    slotUpdateItemInfo();

    // Enable actions as appropriate after loading
    // No need to re-enable image properties sidebar here, it's will be done
    // automatically by a signal from canvas
    toggleActions(success);
    unsetCursor();
    m_animLogo->stop();
}

void EditorWindow::slotNameLabelCancelButtonPressed()
{
    // If we saving an image...
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
    else if (promptForOverWrite())
        save();
}

void EditorWindow::slotSavingStarted(const QString& /*filename*/)
{
    setCursor(Qt::WaitCursor);
    m_animLogo->start();

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
                KMessageBox::error(this, i18n("Failed to save file\n\"%1\"\nto\n\"%2\".",
                                              m_savingContext->destinationURL.fileName(),
                                              m_savingContext->destinationURL.path()));
            }
            finishSaving(false);
            return;
        }

        kDebug(50003) << "renaming to " << m_savingContext->destinationURL.path();

        if (!moveFile())
        {
            finishSaving(false);
            return;
        }

        m_canvas->setUndoHistoryOrigin();

        // remove image from cache since it has changed
        LoadingCacheInterface::fileChanged(m_savingContext->destinationURL.path());
        // this won't be in the cache, but does not hurt to do it anyway
        LoadingCacheInterface::fileChanged(filename);

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
                KMessageBox::error(this, i18n("Failed to save file\n\"%1\"\nto\n\"%2\".",
                                              m_savingContext->destinationURL.fileName(),
                                              m_savingContext->destinationURL.path()));
            }
            finishSaving(false);
            return;
        }

        // Only try to write Exif if both src and destination are JPEG files

        kDebug(50003) << "renaming to " << m_savingContext->destinationURL.path();

        if (!moveFile())
        {
            finishSaving(false);
            return;
        }

        m_canvas->setUndoHistoryOrigin();

        LoadingCacheInterface::fileChanged(m_savingContext->destinationURL.path());
        LoadingCacheInterface::fileChanged(filename);

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
        quitWaitingLoop();

    // Enable actions as appropriate after saving
    toggleActions(true);
    unsetCursor();
    m_animLogo->stop();

    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);

    // On error, continue using current image
    if (!success)
    {
        m_canvas->switchToLastSaved(m_savingContext->srcURL.path());
    }
}

void EditorWindow::startingSave(const KUrl& url)
{
    // avoid any reentrancy. Should be impossible anyway since actions will be disabled.
    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
        return;

    if (!checkPermissions(url))
        return;

    QString tempDir = url.directory(KUrl::AppendTrailingSlash);
    // use magic file extension which tells the digikamalbums ioslave to ignore the file
    m_savingContext->saveTempFile = new KTemporaryFile();
    m_savingContext->saveTempFile->setPrefix(tempDir);
    m_savingContext->saveTempFile->setSuffix(".digikamtempfile.tmp");
    m_savingContext->saveTempFile->setAutoRemove(false);
    m_savingContext->saveTempFile->open();

    if (!m_savingContext->saveTempFile->open())
    {
        KMessageBox::error(this, i18n("Could not open a temporary file in the folder \"%1\": %2 (%3)",
                                      tempDir, m_savingContext->saveTempFile->errorString(),
                                      m_savingContext->saveTempFile->error()));
        return;
    }
    m_savingContext->saveTempFileName = m_savingContext->saveTempFile->fileName();
    delete m_savingContext->saveTempFile;
    m_savingContext->saveTempFile = 0;

    m_savingContext->srcURL             = url;
    m_savingContext->destinationURL     = m_savingContext->srcURL;
    m_savingContext->destinationExisted = true;
    m_savingContext->originalFormat     = m_canvas->currentImageFileFormat();
    m_savingContext->format             = m_savingContext->originalFormat;
    m_savingContext->abortingSaving     = false;
    m_savingContext->savingState        = SavingContextContainer::SavingStateSave;

    m_canvas->saveAs(m_savingContext->saveTempFileName, m_IOFileSettings,
                     m_setExifOrientationTag && (m_rotatedOrFlipped || m_canvas->exifRotated()));
}

bool EditorWindow::startingSaveAs(const KUrl& url)
{
    if (m_savingContext->savingState != SavingContextContainer::SavingStateNone)
        return false;

    QString pattern             = KImageIO::pattern(KImageIO::Writing);
    QStringList writablePattern = pattern.split(QChar('\n'));
    kDebug(50003) << "KDE Offered pattern: " << writablePattern;
    writablePattern.append(QString("*.jp2|") + i18n("JPEG 2000 image"));
    writablePattern.append(QString("*.pgf|") + i18n("Progressive Graphics File"));

    m_savingContext->srcURL = url;

    FileSaveOptionsBox *options      = new FileSaveOptionsBox();
    KFileDialog *imageFileSaveDialog = new KFileDialog(m_savingContext->srcURL.isLocalFile() ?
                                                       m_savingContext->srcURL : KUrl(QDir::homePath()),
                                                       QString(),
                                                       this,
                                                       options);

    connect(imageFileSaveDialog, SIGNAL(filterChanged(const QString&)),
            options, SLOT(slotImageFileFormatChanged(const QString&)));

    connect(imageFileSaveDialog, SIGNAL(fileSelected(const QString &)),
            options, SLOT(slotImageFileSelected(const QString&)));

    ImageDialogPreview *preview = new ImageDialogPreview(imageFileSaveDialog);
    imageFileSaveDialog->setPreviewWidget(preview);
    imageFileSaveDialog->setModal(false);
    imageFileSaveDialog->setOperationMode(KFileDialog::Saving);
    imageFileSaveDialog->setMode(KFile::File);
    imageFileSaveDialog->setCaption(i18n("New Image File Name"));

    QFileInfo info(m_savingContext->srcURL.fileName());
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    QString ext               = group.readEntry("LastSavedImageTypeMime", "png");
    QString fileName          = info.baseName() + QString(".") + ext;

    // Determine the default filter from LastSavedImageTypeMime
    QString defaultFilter;
    foreach(QString filter, writablePattern)
    {
        if (filter.contains(QString("*.%1").arg(ext.toLower())))
        {
            defaultFilter = filter;
            break;
        }
    }
    imageFileSaveDialog->filterWidget()->setDefaultFilter(defaultFilter);
    imageFileSaveDialog->setFilter(writablePattern.join(QChar('\n')));
    imageFileSaveDialog->setSelection(fileName);

    options->slotImageFileFormatChanged(ext);

    // Start dialog and check if canceled.
    if ( imageFileSaveDialog->exec() != KFileDialog::Accepted )
       return false;

    // Update file save settings in editor instance.
    options->applySettings();
    applyStandardSettings();

    KUrl newURL = imageFileSaveDialog->selectedUrl();

    // Check if target image format have been selected from Combo List of SaveAs dialog.

    QStringList mimes = KImageIO::typeForMime(imageFileSaveDialog->currentMimeFilter());
    if (!mimes.isEmpty())
    {
        m_savingContext->format = mimes.first();
    }
    else
    {
        // Else, check if target image format have been add to target image file name using extension.

        QFileInfo fi(newURL.path());
        m_savingContext->format = fi.suffix();

        if ( m_savingContext->format.isEmpty() )
        {
            // If format is empty then file format is same as that of the original file.
            m_savingContext->format = QImageReader::imageFormat(m_savingContext->srcURL.path());
        }
        else
        {
            // Else, check if format from file name extension is include on file mime type list.

            QStringList types = KImageIO::types(KImageIO::Writing);
            kDebug(50003) << "KDE Offered types: " << types;

            types << "TIF";
            types << "TIFF";
            types << "JPG";
            types << "JPEG";
            types << "JPE";
            types << "J2K";
            types << "JP2";
            types << "PGF";
            QString imgExtList = types.join(" ");

            if ( !imgExtList.toUpper().contains( m_savingContext->format.toUpper() ) )
            {
                KMessageBox::error(this, i18n("Target image file format \"%1\" unsupported.", m_savingContext->format));
                kWarning(50003) << "target image file format " << m_savingContext->format << " unsupported!";
                return false;
            }
        }
    }

    if (!newURL.isValid())
    {
        KMessageBox::error(this, i18n("Failed to save file\n\"%1\"\nto\n\"%2\".",
                                      newURL.fileName(),
                                      newURL.path().section('/', -2, -2)));
        kWarning(50003) << "target URL is not valid !";
        return false;
    }

    group.writeEntry("LastSavedImageTypeMime", m_savingContext->format);
    config->sync();

    // if new and original URL are equal use slotSave() ------------------------------

    KUrl currURL(m_savingContext->srcURL);
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
                                                  "to overwrite it?",
                                                  newURL.fileName()),
                                       i18n("Overwrite File?"),
                                       KStandardGuiItem::overwrite(),
                                       KStandardGuiItem::cancel() );

        if (result != KMessageBox::Yes)
            return false;

        // There will be two message boxes if the file is not writable.
        // This may be controversial, and it may be changed, but it was a deliberate decision.
        if (!checkPermissions(newURL))
            return false;
    }

    // Now do the actual saving -----------------------------------------------------

    // use magic file extension which tells the digikamalbums ioslave to ignore the file

    QString tempDir = newURL.directory(KUrl::AppendTrailingSlash);

    m_savingContext->saveTempFile = new KTemporaryFile();
    m_savingContext->saveTempFile->setPrefix(tempDir);
    m_savingContext->saveTempFile->setSuffix(".digikamtempfile.tmp");
    m_savingContext->saveTempFile->setAutoRemove(false);

    if (!m_savingContext->saveTempFile->open())
    {
        KMessageBox::error(this, i18n("Could not open a temporary file in the folder \"%1\": %2 (%3)",
                                      tempDir, m_savingContext->saveTempFile->errorString(),
                                      m_savingContext->saveTempFile->error()));
        return false;
    }
    m_savingContext->saveTempFileName = m_savingContext->saveTempFile->fileName();
    delete m_savingContext->saveTempFile;
    m_savingContext->saveTempFile = 0;

    m_savingContext->destinationURL = newURL;
    m_savingContext->originalFormat = m_canvas->currentImageFileFormat();
    m_savingContext->savingState    = SavingContextContainer::SavingStateSaveAs;
    m_savingContext->abortingSaving = false;

    m_canvas->saveAs(m_savingContext->saveTempFileName, m_IOFileSettings,
                     m_setExifOrientationTag && (m_rotatedOrFlipped || m_canvas->exifRotated()),
                     m_savingContext->format.toLower());

    return true;
}

bool EditorWindow::checkPermissions(const KUrl& url)
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
                                                  "to overwrite it?",
                                                  url.fileName()),
                                       i18n("Overwrite File?"),
                                       KStandardGuiItem::overwrite(),
                                       KStandardGuiItem::cancel() );

        if (result != KMessageBox::Yes)
            return false;
    }

    return true;
}

bool EditorWindow::moveFile()
{
    QByteArray dstFileName = QFile::encodeName(m_savingContext->destinationURL.toLocalFile());
#ifndef _WIN32
    // Store old permissions:
    // Just get the current umask.
    mode_t curr_umask = umask(S_IREAD | S_IWRITE);
    // Restore the umask.
    umask(curr_umask);

    // For new files respect the umask setting.
    mode_t filePermissions = (S_IREAD | S_IWRITE | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP) & ~curr_umask;

    // For existing files, use the mode of the original file.
    if (m_savingContext->destinationExisted)
    {
        struct stat stbuf;
        if (::stat(dstFileName, &stbuf) == 0)
        {
            filePermissions = stbuf.st_mode;
        }
    }
#endif
    // rename tmp file to dest
    int ret;
#if KDE_IS_VERSION(4,2,85)
    // KDE 4.3.0
    ret = KDE::rename(QFile::encodeName(m_savingContext->saveTempFileName),
                       dstFileName);
#else
    // KDE 4.2.x or 4.1.x
    ret = KDE_rename(QFile::encodeName(m_savingContext->saveTempFileName),
                      dstFileName);
#endif
    if (ret != 0)
    {
        KMessageBox::error(this, i18n("Failed to overwrite original file"),
                           i18n("Error Saving File"));
        return false;
    }

#ifndef _WIN32
    // restore permissions
    if (::chmod(dstFileName, filePermissions) != 0)
    {
        kWarning(50003) << "Failed to restore file permissions for file " << dstFileName;
    }
#endif
    return true;
}

void EditorWindow::slotToggleColorManagedView()
{
    d->cmViewIndicator->blockSignals(true);
    d->viewCMViewAction->blockSignals(true);
    bool cmv = false;
    if (d->ICCSettings->enableCM)
    {
        cmv = !d->ICCSettings->useManagedView;
        d->ICCSettings->useManagedView = cmv;
        m_canvas->setICCSettings(d->ICCSettings);

        // Save Color Managed View setting in config file. For performance
        // reason, no need to flush file, it cached in memory and will be flushed
        // to disk at end of session.
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group = config->group("Color Management");
        d->ICCSettings->writeManagedViewToConfig(group);
    }

    d->cmViewIndicator->setChecked(cmv);
    d->viewCMViewAction->setChecked(cmv);
    setColorManagedViewIndicatorToolTip(d->ICCSettings->enableCM, cmv);
    d->cmViewIndicator->blockSignals(false);
    d->viewCMViewAction->blockSignals(false);
}

void EditorWindow::setColorManagedViewIndicatorToolTip(bool available, bool cmv)
{
    QString tooltip;
    if (available)
    {
        if (cmv)
            tooltip = i18n("Color-Managed View is enabled.");
        else
            tooltip = i18n("Color-Managed View is disabled.");
    }
    else
    {
        tooltip = i18n("Color Management is not configured, so the Color-Managed View is not available.");
    }
    d->cmViewIndicator->setToolTip(tooltip);
}

void EditorWindow::slotToggleUnderExposureIndicator()
{
    d->underExposureIndicator->blockSignals(true);
    d->viewUnderExpoAction->blockSignals(true);
    bool uei = !d->exposureSettings->underExposureIndicator;
    d->underExposureIndicator->setChecked(uei);
    d->viewUnderExpoAction->setChecked(uei);
    d->exposureSettings->underExposureIndicator = uei;
    m_canvas->setExposureSettings(d->exposureSettings);
    setUnderExposureToolTip(uei);
    d->underExposureIndicator->blockSignals(false);
    d->viewUnderExpoAction->blockSignals(false);
}

void EditorWindow::setUnderExposureToolTip(bool uei)
{
    d->underExposureIndicator->setToolTip(
                  uei ? i18n("Under-Exposure indicator is enabled")
                      : i18n("Under-Exposure indicator is disabled"));
}

void EditorWindow::slotToggleOverExposureIndicator()
{
    d->overExposureIndicator->blockSignals(true);
    d->viewOverExpoAction->blockSignals(true);
    bool oei = !d->exposureSettings->overExposureIndicator;
    d->overExposureIndicator->setChecked(oei);
    d->viewOverExpoAction->setChecked(oei);
    d->exposureSettings->overExposureIndicator = oei;
    m_canvas->setExposureSettings(d->exposureSettings);
    setOverExposureToolTip(oei);
    d->overExposureIndicator->blockSignals(false);
    d->viewOverExpoAction->blockSignals(false);
}

void EditorWindow::setOverExposureToolTip(bool oei)
{
    d->overExposureIndicator->setToolTip(
                  oei ? i18n("Over-Exposure indicator is enabled")
                      : i18n("Over-Exposure indicator is disabled"));
}

void EditorWindow::slotDonateMoney()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=donation");
}

void EditorWindow::slotContribute()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=contrib");
}

void EditorWindow::slotToggleSlideShow()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("ImageViewer Settings");
    bool startWithCurrent = group.readEntry("SlideShowStartCurrent", false);

    SlideShowSettings settings;
    settings.delay                = group.readEntry("SlideShowDelay", 5) * 1000;
    settings.printName            = group.readEntry("SlideShowPrintName", true);
    settings.printDate            = group.readEntry("SlideShowPrintDate", false);
    settings.printApertureFocal   = group.readEntry("SlideShowPrintApertureFocal", false);
    settings.printExpoSensitivity = group.readEntry("SlideShowPrintExpoSensitivity", false);
    settings.printMakeModel       = group.readEntry("SlideShowPrintMakeModel", false);
    settings.printComment         = group.readEntry("SlideShowPrintComment", false);
    settings.printRating          = group.readEntry("SlideShowPrintRating", false);
    settings.loop                 = group.readEntry("SlideShowLoop", false);
    slideShow(startWithCurrent, settings);
}

void EditorWindow::slotSelectionChanged(const QRect& sel)
{
    d->selectLabel->setText(QString("(%1, %2) (%3 x %4)").arg(sel.x()).arg(sel.y())
                            .arg(sel.width()).arg(sel.height()));
}

void EditorWindow::slotRawCameraList()
{
    RawCameraDlg *dlg = new RawCameraDlg(kapp->activeWindow());
    dlg->show();
}

void EditorWindow::slotThemeChanged()
{
    QStringList themes(ThemeEngine::instance()->themeNames());
    int index = themes.indexOf(ThemeEngine::instance()->getCurrentThemeName());
    if (index == -1)
        index = themes.indexOf(i18n("Default"));

    m_themeMenuAction->setCurrentItem(index);

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");

    if (!group.readEntry("UseThemeBackgroundColor", true))
        m_bgColor = group.readEntry("BackgroundColor", QColor(Qt::black));
    else
        m_bgColor = ThemeEngine::instance()->baseColor();

    m_canvas->setBackgroundColor(m_bgColor);
}

void EditorWindow::slotChangeTheme(const QString& theme)
{
    ThemeEngine::instance()->slotChangeTheme(theme);
}

void EditorWindow::slotToggleShowBar()
{
    if (m_showBarAction->isChecked())
        thumbBar()->show();
    else
        thumbBar()->hide();
}

void EditorWindow::toggleGUI2FullScreen()
{
    if (m_fullScreen)
    {
        rightSideBar()->restore(QList<QWidget*>() << thumbBar(), d->fullscreenSizeBackup);

        if (m_showBarAction->isChecked() && m_fullScreenHideThumbBar)
            thumbBar()->show();
    }
    else
    {
        // See bug #166472, a simple backup()/restore() will hide non-sidebar splitter child widgets
        // in horizontal mode thumbbar wont be member of the splitter, it is just ignored then
        rightSideBar()->backup(QList<QWidget*>() << thumbBar(), &d->fullscreenSizeBackup);

        if (m_showBarAction->isChecked() && m_fullScreenHideThumbBar)
                thumbBar()->hide();
    }
}

void EditorWindow::slotComponentsInfo()
{
    LibsInfoDlg *dlg = new LibsInfoDlg(this);
    dlg->show();
}

void EditorWindow::setToolStartProgress(const QString& toolName)
{
    m_animLogo->start();
    m_nameLabel->setProgressValue(0);
    m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode, QString("%1: ").arg(toolName));
}

void EditorWindow::setToolProgress(int progress)
{
    m_nameLabel->setProgressValue(progress);
}

void EditorWindow::setToolStopProgress()
{
    m_animLogo->stop();
    m_nameLabel->setProgressValue(0);
    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);
    slotUpdateItemInfo();
}

void EditorWindow::slotShowMenuBar()
{
    const bool visible = menuBar()->isVisible();
    menuBar()->setVisible(!visible);
}

void EditorWindow::slotCloseTool()
{
    if (d->toolIface)
        d->toolIface->slotCloseTool();
}

}  // namespace Digikam
