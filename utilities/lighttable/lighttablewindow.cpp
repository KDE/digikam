/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lighttablewindow.h"
#include "lighttablewindow.moc"

// Qt includes

#include <QHBoxLayout>
#include <QFrame>
#include <QVBoxLayout>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kselectaction.h>
#include <kshortcutsdialog.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <ktoolinvocation.h>
#include <kwindowsystem.h>
#include <kxmlguifactory.h>

// Libkdcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000400
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes

#include "componentsinfo.h"
#include "digikamapp.h"
#include "themeengine.h"
#include "dimg.h"
#include "dlogoaction.h"
#include "dmetadata.h"
#include "albumsettings.h"
#include "albummanager.h"
#include "loadingcacheinterface.h"
#include "deletedialog.h"
#include "imagewindow.h"
#include "slideshow.h"
#include "setup.h"
#include "syncjob.h"
#include "thumbnailsize.h"
#include "lighttablepreview.h"
#include "lighttablewindow_p.h"

namespace Digikam
{

LightTableWindow* LightTableWindow::m_instance = 0;

LightTableWindow* LightTableWindow::lightTableWindow()
{
    if (!m_instance)
        new LightTableWindow();

    return m_instance;
}

bool LightTableWindow::lightTableWindowCreated()
{
    return m_instance;
}

LightTableWindow::LightTableWindow()
                : KXmlGuiWindow(0), d(new LightTableWindowPriv)
{
    m_instance = this;

    setWindowFlags(Qt::Window);
    setCaption(i18n("Light Table"));
    // We don't want to be deleted on close
    setAttribute(Qt::WA_DeleteOnClose, false);

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupStatusBar();
    setupActions();

    // Make signals/slots connections

    setupConnections();

    //-------------------------------------------------------------

    d->leftSideBar->loadViewState();
    d->rightSideBar->loadViewState();
    d->leftSideBar->populateTags();
    d->rightSideBar->populateTags();
    slotSidebarTabTitleStyleChanged();

    readSettings();
    applySettings();
    setAutoSaveSettings("LightTable Settings", true);
}

LightTableWindow::~LightTableWindow()
{
    m_instance = 0;

    delete d->barView;
    delete d->rightSideBar;
    delete d->leftSideBar;
    delete d;
}

void LightTableWindow::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("LightTable Settings");
    if (group.hasKey("Vertical Splitter State"))
    {
        QByteArray state;
        state = group.readEntry("Vertical Splitter State", state);
        d->vSplitter->restoreState(QByteArray::fromBase64(state));
    }

    d->hSplitter->restoreState(group, "Horizontal Splitter State");

    d->navigateByPairAction->setChecked(group.readEntry("Navigate By Pair", false));
    slotToggleNavigateByPair();
}

void LightTableWindow::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("LightTable Settings");
    group.writeEntry("Vertical Splitter State", d->vSplitter->saveState().toBase64());
    d->hSplitter->saveState(group, "Horizontal Splitter State");
    group.writeEntry("Navigate By Pair", d->navigateByPairAction->isChecked());
    group.writeEntry("Clear On Close", d->clearOnCloseAction->isChecked());
    config->sync();
}

void LightTableWindow::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("LightTable Settings");
    d->autoLoadOnRightPanel   = group.readEntry("Auto Load Right Panel",   true);
    d->autoSyncPreview        = group.readEntry("Auto Sync Preview",       true);
    d->fullScreenHideToolBar  = group.readEntry("FullScreen Hide ToolBar", false);
    d->clearOnCloseAction->setChecked(group.readEntry("Clear On Close", false));
    d->previewView->setLoadFullImageSize(group.readEntry("Load Full Image size", false));
    d->barView->applySettings();
    refreshView();
 }

void LightTableWindow::refreshView()
{
    d->leftSideBar->refreshTagsView();
    d->rightSideBar->refreshTagsView();
}

void LightTableWindow::closeEvent(QCloseEvent* e)
{
    if (!e) return;

    if (d->clearOnCloseAction->isChecked()) slotClearItemsList();
    writeSettings();

    e->accept();
}

void LightTableWindow::setupUserArea()
{
    QWidget* mainW    = new QWidget(this);
    d->hSplitter      = new SidebarSplitter(Qt::Horizontal, mainW);
    QHBoxLayout *hlay = new QHBoxLayout(mainW);
    d->leftSideBar    = new ImagePropertiesSideBarDB(mainW, d->hSplitter, KMultiTabBar::Left, true);

    QWidget* centralW = new QWidget(d->hSplitter);
    QVBoxLayout *vlay = new QVBoxLayout(centralW);
    d->vSplitter      = new QSplitter(Qt::Vertical, centralW);
    d->barView        = new LightTableBar(d->vSplitter, Qt::Horizontal,
                                          AlbumSettings::instance()->getExifRotate());
    d->previewView    = new LightTableView(d->vSplitter);

    d->rightSideBar   = new ImagePropertiesSideBarDB(mainW, d->hSplitter, KMultiTabBar::Right, true);

    vlay->addWidget(d->vSplitter);
    vlay->setSpacing(0);
    vlay->setMargin(0);

    hlay->addWidget(d->leftSideBar);
    hlay->addWidget(d->hSplitter);
    hlay->addWidget(d->rightSideBar);
    hlay->setSpacing(0);
    hlay->setMargin(0);
    hlay->setStretchFactor(d->hSplitter, 10);

    d->hSplitter->setFrameStyle( QFrame::NoFrame );
    d->hSplitter->setFrameShadow( QFrame::Plain );
    d->hSplitter->setFrameShape( QFrame::NoFrame );
    d->hSplitter->setOpaqueResize(false);
    d->hSplitter->setStretchFactor(1, 10);      // set previewview+thumbbar container default size to max.

    d->vSplitter->setFrameStyle( QFrame::NoFrame );
    d->vSplitter->setFrameShadow( QFrame::Plain );
    d->vSplitter->setFrameShape( QFrame::NoFrame );
    d->vSplitter->setOpaqueResize(false);
    d->vSplitter->setStretchFactor(1, 10);      // set previewview default size to max.

    setCentralWidget(mainW);
}

void LightTableWindow::setupStatusBar()
{
    d->leftZoomBar = new StatusZoomBar(statusBar());
    d->leftZoomBar->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(d->leftZoomBar, 1);
    d->leftZoomBar->setEnabled(false);

    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignCenter);
    d->statusProgressBar->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(d->statusProgressBar, 100);

    d->rightZoomBar = new StatusZoomBar(statusBar());
    d->rightZoomBar->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(d->rightZoomBar, 1);
    d->rightZoomBar->setEnabled(false);
}

void LightTableWindow::setupConnections()
{
    connect(d->statusProgressBar, SIGNAL(signalCancelButtonPressed()),
           this, SLOT(slotProgressBarCancelButtonPressed()));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSidebarTabTitleStyleChanged()));

    // Thumbs bar connections ---------------------------------------

    connect(d->barView, SIGNAL(signalSetItemOnLeftPanel(const ImageInfo &)),
           this, SLOT(slotSetItemOnLeftPanel(const ImageInfo &)));

    connect(d->barView, SIGNAL(signalSetItemOnRightPanel(const ImageInfo &)),
           this, SLOT(slotSetItemOnRightPanel(const ImageInfo &)));

    connect(d->barView, SIGNAL(signalRemoveItem(const ImageInfo &)),
           this, SLOT(slotRemoveItem(const ImageInfo &)));

    connect(d->barView, SIGNAL(signalEditItem(const ImageInfo &)),
           this, SLOT(slotEditItem(const ImageInfo &)));

    connect(d->barView, SIGNAL(signalClearAll()),
           this, SLOT(slotClearItemsList()));

    connect(d->barView, SIGNAL(signalLightTableBarItemSelected(const ImageInfo &)),
           this, SLOT(slotItemSelected(const ImageInfo &)));

    connect(d->barView, SIGNAL(signalDroppedItems(const ImageInfoList&)),
           this, SLOT(slotThumbbarDroppedItems(const ImageInfoList&)));

    // Zoom bars connections -----------------------------------------

    connect(d->leftZoomBar, SIGNAL(signalZoomMinusClicked()),
           d->previewView, SLOT(slotDecreaseLeftZoom()));

    connect(d->leftZoomBar, SIGNAL(signalZoomPlusClicked()),
           d->previewView, SLOT(slotIncreaseLeftZoom()));

    connect(d->leftZoomBar, SIGNAL(signalZoomSliderChanged(int)),
           d->previewView, SLOT(slotLeftZoomSliderChanged(int)));

    connect(d->rightZoomBar, SIGNAL(signalZoomMinusClicked()),
           d->previewView, SLOT(slotDecreaseRightZoom()));

    connect(d->rightZoomBar, SIGNAL(signalZoomPlusClicked()),
           d->previewView, SLOT(slotIncreaseRightZoom()));

    connect(d->rightZoomBar, SIGNAL(signalZoomSliderChanged(int)),
           d->previewView, SLOT(slotRightZoomSliderChanged(int)));

    // View connections ---------------------------------------------

    connect(d->previewView, SIGNAL(signalLeftZoomFactorChanged(double)),
           this, SLOT(slotLeftZoomFactorChanged(double)));

    connect(d->previewView, SIGNAL(signalRightZoomFactorChanged(double)),
           this, SLOT(slotRightZoomFactorChanged(double)));

    connect(d->previewView, SIGNAL(signalEditItem(const ImageInfo &)),
           this, SLOT(slotEditItem(const ImageInfo &)));

    connect(d->previewView, SIGNAL(signalDeleteItem(const ImageInfo &)),
           this, SLOT(slotDeleteItem(const ImageInfo &)));

    connect(d->previewView, SIGNAL(signalSlideShow()),
           this, SLOT(slotToggleSlideShow()));

    connect(d->previewView, SIGNAL(signalLeftDroppedItems(const ImageInfoList&)),
           this, SLOT(slotLeftDroppedItems(const ImageInfoList&)));

    connect(d->previewView, SIGNAL(signalRightDroppedItems(const ImageInfoList&)),
           this, SLOT(slotRightDroppedItems(const ImageInfoList&)));

    connect(d->previewView, SIGNAL(signalToggleOnSyncPreview(bool)),
           this, SLOT(slotToggleOnSyncPreview(bool)));

    connect(d->previewView, SIGNAL(signalLeftPreviewLoaded(bool)),
            this, SLOT(slotLeftPreviewLoaded(bool)));

    connect(d->previewView, SIGNAL(signalRightPreviewLoaded(bool)),
            this, SLOT(slotRightPreviewLoaded(bool)));

    connect(d->previewView, SIGNAL(signalLeftPanelLeftButtonClicked()),
            this, SLOT(slotLeftPanelLeftButtonClicked()));

    connect(d->previewView, SIGNAL(signalRightPanelLeftButtonClicked()),
            this, SLOT(slotRightPanelLeftButtonClicked()));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->leftZoomBar, SLOT(slotUpdateTrackerPos()));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->rightZoomBar, SLOT(slotUpdateTrackerPos()));

    // -- FileWatch connections ------------------------------

    LoadingCacheInterface::connectToSignalFileChanged(this, SLOT(slotFileChanged(const QString&)));
}

void LightTableWindow::setupActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    d->backwardAction = KStandardAction::back(this, SLOT(slotBackward()), this);
    actionCollection()->addAction("lighttable_backward", d->backwardAction);
    d->backwardAction->setShortcut( KShortcut(Qt::Key_PageUp, Qt::Key_Backspace) );

    d->forwardAction = KStandardAction::forward(this, SLOT(slotForward()), this);
    actionCollection()->addAction("lighttable_forward", d->forwardAction);
    d->forwardAction->setEnabled(false);
    d->forwardAction->setShortcut( KShortcut(Qt::Key_PageDown, Qt::Key_Space) );

    d->firstAction = new KAction(KIcon("go-first"), i18n("&First"), this);
    d->firstAction->setShortcut(KStandardShortcut::begin());
    d->firstAction->setEnabled(false);
    connect(d->firstAction, SIGNAL(triggered()), this, SLOT(slotFirst()));
    actionCollection()->addAction("lighttable_first", d->firstAction);

    d->lastAction = new KAction(KIcon("go-last"), i18n("&Last"), this);
    d->lastAction->setShortcut(KStandardShortcut::end());
    d->lastAction->setEnabled(false);
    connect(d->lastAction, SIGNAL(triggered()), this, SLOT(slotLast()));
    actionCollection()->addAction("lighttable_last", d->lastAction);

    d->setItemLeftAction = new KAction(KIcon("arrow-left"), i18n("On left"), this);
    d->setItemLeftAction->setShortcut(Qt::CTRL+Qt::Key_L);
    d->setItemLeftAction->setEnabled(false);
    d->setItemLeftAction->setWhatsThis(i18n("Show item on left panel"));
    connect(d->setItemLeftAction, SIGNAL(triggered()), this, SLOT(slotSetItemLeft()));
    actionCollection()->addAction("lighttable_setitemleft", d->setItemLeftAction);

    d->setItemRightAction = new KAction(KIcon("arrow-right"), i18n("On right"), this);
    d->setItemRightAction->setShortcut(Qt::CTRL+Qt::Key_R);
    d->setItemRightAction->setEnabled(false);
    d->setItemRightAction->setWhatsThis(i18n("Show item on right panel"));
    connect(d->setItemRightAction, SIGNAL(triggered()), this, SLOT(slotSetItemRight()));
    actionCollection()->addAction("lighttable_setitemright", d->setItemRightAction);

    d->editItemAction = new KAction(KIcon("editimage"), i18n("Edit"), this);
    d->editItemAction->setShortcut(Qt::Key_F4);
    d->editItemAction->setEnabled(false);
    connect(d->editItemAction, SIGNAL(triggered()), this, SLOT(slotEditItem()));
    actionCollection()->addAction("lighttable_edititem", d->editItemAction);

    d->removeItemAction = new KAction(KIcon("list-remove"), i18n("Remove item from LightTable"), this);
    d->removeItemAction->setShortcut(Qt::CTRL+Qt::Key_K);
    d->removeItemAction->setEnabled(false);
    connect(d->removeItemAction, SIGNAL(triggered()), this, SLOT(slotRemoveItem()));
    actionCollection()->addAction("lighttable_removeitem", d->removeItemAction);

    d->clearListAction = new KAction(KIcon("edit-clear"), i18n("Remove all items from LightTable"), this);
    d->clearListAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_K);
    d->clearListAction->setEnabled(false);
    connect(d->clearListAction, SIGNAL(triggered()), this, SLOT(slotClearItemsList()));
    actionCollection()->addAction("lighttable_clearlist", d->clearListAction);

    d->fileDeleteAction = new KAction(KIcon("user-trash"), i18n("Move to Trash"), this);
    d->fileDeleteAction->setShortcut(Qt::Key_Delete);
    d->fileDeleteAction->setEnabled(false);
    connect(d->fileDeleteAction, SIGNAL(triggered()), this, SLOT(slotDeleteItem()));
    actionCollection()->addAction("lighttable_filedelete", d->fileDeleteAction);

    KAction* closeAction = KStandardAction::close(this, SLOT(close()), this);
    actionCollection()->addAction("lighttable_close", closeAction);

    // -- Standard 'View' menu actions ---------------------------------------------

    d->syncPreviewAction = new KToggleAction(KIcon("view-split-left-right"), i18n("Synchronize"), this);
    d->syncPreviewAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_Y);
    d->syncPreviewAction->setEnabled(false);
    d->syncPreviewAction->setWhatsThis(i18n("Synchronize preview from left and right panels"));
    connect(d->syncPreviewAction, SIGNAL(triggered()), this, SLOT(slotToggleSyncPreview()));
    actionCollection()->addAction("lighttable_syncpreview", d->syncPreviewAction);

    d->navigateByPairAction = new KToggleAction(KIcon("system-run"), i18n("By Pair"), this);
    d->navigateByPairAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_P);
    d->navigateByPairAction->setEnabled(false);
    d->navigateByPairAction->setWhatsThis(i18n("Navigate by pairs with all items"));
    connect(d->navigateByPairAction, SIGNAL(triggered()), this, SLOT(slotToggleNavigateByPair()));
    actionCollection()->addAction("lighttable_navigatebypair", d->navigateByPairAction);

    d->clearOnCloseAction = new KToggleAction(KIcon("edit-clear"), i18n("Clear On Close"), this);
    d->clearOnCloseAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_C);
    d->clearOnCloseAction->setEnabled(true);
    d->clearOnCloseAction->setToolTip(i18n("Clear light table when it is closed"));
    d->clearOnCloseAction->setWhatsThis(i18n("Remove all images from the light table when it is closed"));
    actionCollection()->addAction("lighttable_clearonclose", d->clearOnCloseAction);

    d->zoomPlusAction = KStandardAction::zoomIn(d->previewView, SLOT(slotIncreaseZoom()), this);
    d->zoomPlusAction->setEnabled(false);
    d->zoomPlusAction->setShortcut(QKeySequence(Qt::Key_Plus));
    actionCollection()->addAction("lighttable_zoomplus", d->zoomPlusAction);

    d->zoomMinusAction = KStandardAction::zoomOut(d->previewView, SLOT(slotDecreaseZoom()), this);
    d->zoomMinusAction->setEnabled(false);
    d->zoomMinusAction->setShortcut(QKeySequence(Qt::Key_Minus));
    actionCollection()->addAction("lighttable_zoomminus", d->zoomMinusAction);

    d->zoomTo100percents = new KAction(KIcon("zoom-original"), i18n("Zoom to 100%"), this);
    d->zoomTo100percents->setShortcut(Qt::ALT+Qt::CTRL+Qt::Key_0);       // NOTE: Photoshop 7 use ALT+CTRL+0
    connect(d->zoomTo100percents, SIGNAL(triggered()), this, SLOT(slotZoomTo100Percents()));
    actionCollection()->addAction("lighttable_zoomto100percents", d->zoomTo100percents);

    d->zoomFitToWindowAction = new KToggleAction(KIcon("zoom-fit-best"), i18n("Fit to &Window"), this);
    d->zoomFitToWindowAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_E); // NOTE: Gimp 2 use CTRL+SHIFT+E.
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), this, SLOT(slotFitToWindow()));
    actionCollection()->addAction("lighttable_zoomfit2window", d->zoomFitToWindowAction);

    d->fullScreenAction = actionCollection()->addAction(KStandardAction::FullScreen,
                          "lighttable_fullscreen", this, SLOT(slotToggleFullScreen()));

    d->slideShowAction = new KAction(KIcon("view-presentation"), i18n("Slideshow"), this);
    d->slideShowAction->setShortcut(Qt::Key_F9);
    connect(d->slideShowAction, SIGNAL(triggered()), this, SLOT(slotToggleSlideShow()));
    actionCollection()->addAction("lighttable_slideshow", d->slideShowAction);

    // -- Standard 'Configure' menu actions ----------------------------------------

    d->showMenuBarAction = KStandardAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());

    KStandardAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStandardAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStandardAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // ---------------------------------------------------------------------------------

    d->themeMenuAction = new KSelectAction(i18n("&Themes"), this);
    connect(d->themeMenuAction, SIGNAL(triggered(const QString&)),
            this, SLOT(slotChangeTheme(const QString&)));
    actionCollection()->addAction("theme_menu", d->themeMenuAction);

    d->themeMenuAction->setItems(ThemeEngine::instance()->themeNames());
    slotThemeChanged();

    // -- Standard 'Help' menu actions ---------------------------------------------

    d->donateMoneyAction = new KAction(i18n("Donate Money..."), this);
    connect(d->donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    actionCollection()->addAction("lighttable_donatemoney", d->donateMoneyAction);

    d->contributeAction = new KAction(i18n("Contribute..."), this);
    connect(d->contributeAction, SIGNAL(triggered()), this, SLOT(slotContribute()));
    actionCollection()->addAction("lighttable_contribute", d->contributeAction);

    d->rawCameraListAction = new KAction(KIcon("kdcraw"), i18n("Supported RAW Cameras"), this);
    connect(d->rawCameraListAction, SIGNAL(triggered()), this, SLOT(slotRawCameraList()));
    actionCollection()->addAction("lighttable_rawcameralist", d->rawCameraListAction);

    d->libsInfoAction = new KAction(KIcon("help-about"), i18n("Components info"), this);
    connect(d->libsInfoAction, SIGNAL(triggered()), this, SLOT(slotComponentsInfo()));
    actionCollection()->addAction("lighttable_librariesinfo", d->libsInfoAction);

    d->dbStatAction = new KAction(KIcon("network-server-database"), i18n("Database Statistics"), this);
    connect(d->dbStatAction, SIGNAL(triggered()), this, SLOT(slotDBStat()));
    actionCollection()->addAction("lighttable_dbstat", d->dbStatAction);

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // -- Rating actions ---------------------------------------------------------------

    d->star0 = new KAction(i18n("Assign Rating \"No Stars\""), this);
    d->star0->setShortcut(Qt::CTRL+Qt::Key_0);
    connect(d->star0, SIGNAL(triggered()), d->barView, SLOT(slotAssignRatingNoStar()));
    actionCollection()->addAction("lighttable_ratenostar", d->star0);

    d->star1 = new KAction(i18n("Assign Rating \"One Star\""), this);
    d->star1->setShortcut(Qt::CTRL+Qt::Key_1);
    connect(d->star1, SIGNAL(triggered()), d->barView, SLOT(slotAssignRatingOneStar()));
    actionCollection()->addAction("lighttable_rateonestar", d->star1);

    d->star2 = new KAction(i18n("Assign Rating \"Two Stars\""), this);
    d->star2->setShortcut(Qt::CTRL+Qt::Key_2);
    connect(d->star2, SIGNAL(triggered()), d->barView, SLOT(slotAssignRatingTwoStar()));
    actionCollection()->addAction("lighttable_ratetwostar", d->star2);

    d->star3 = new KAction(i18n("Assign Rating \"Three Stars\""), this);
    d->star3->setShortcut(Qt::CTRL+Qt::Key_3);
    connect(d->star3, SIGNAL(triggered()), d->barView, SLOT(slotAssignRatingThreeStar()));
    actionCollection()->addAction("lighttable_ratethreestar", d->star3);

    d->star4 = new KAction(i18n("Assign Rating \"Four Stars\""), this);
    d->star4->setShortcut(Qt::CTRL+Qt::Key_4);
    connect(d->star4, SIGNAL(triggered()), d->barView, SLOT(slotAssignRatingFourStar()));
    actionCollection()->addAction("lighttable_ratefourstar", d->star4);

    d->star5 = new KAction(i18n("Assign Rating \"Five Stars\""), this);
    d->star5->setShortcut(Qt::CTRL+Qt::Key_5);
    connect(d->star5, SIGNAL(triggered()), d->barView, SLOT(slotAssignRatingFiveStar()));
    actionCollection()->addAction("lighttable_ratefivestar", d->star5);

    // -- Keyboard-only actions added to <MainWindow> ------------------------------

//    KAction *exitFullscreenAction = new KAction(i18n("Exit Fullscreen mode"), this);
//    actionCollection()->addAction("editorwindow_exitfullscreen", exitFullscreenAction);
//    exitFullscreenAction->setShortcut( QKeySequence(Qt::Key_Escape) );
//    connect(exitFullscreenAction, SIGNAL(triggered()), this, SLOT(slotEscapePressed()));

    KAction *altBackwardAction = new KAction(i18n("Previous Image"), this);
    actionCollection()->addAction("lighttable_backward_shift_space", altBackwardAction);
    altBackwardAction->setShortcut( KShortcut(Qt::SHIFT+Qt::Key_Space) );
    connect(altBackwardAction, SIGNAL(triggered()), this, SLOT(slotBackward()));

    // ---------------------------------------------------------------------------------

    actionCollection()->addAction("logo_action", new DLogoAction(this));

    createGUI("lighttablewindowui.rc");

    d->showMenuBarAction->setChecked(!menuBar()->isHidden());  // NOTE: workaround for B.K.O #171080
}

// Deal with items dropped onto the thumbbar (e.g. from the Album view)
void LightTableWindow::slotThumbbarDroppedItems(const ImageInfoList& list)
{
    // Setting the third parameter of loadImageInfos to true
    // means that the images are added to the presently available images.
    loadImageInfos(list, ImageInfo(), true);
}

// We get here either
// - via CTRL+L (from the albumview)
//     a) digikamapp.cpp:  CTRL+key_L leads to slotImageLightTable())
//     b) digikamview.cpp: void DigikamView::slotImageLightTable()
//          calls d->iconView->insertToLightTable(list, info);
//     c) albumiconview.cpp: AlbumIconView::insertToLightTable
//          calls ltview->loadImageInfos(list, current);
// - via drag&drop, i.e. calls issued by the ...Dropped... routines
void LightTableWindow::loadImageInfos(const ImageInfoList& list,
                                      const ImageInfo &givenImageInfoCurrent,
                                      bool addTo)
{
    // Clear all items before adding new images to the light table.
    if (!addTo)
        slotClearItemsList();

    ImageInfoList l = list;
    ImageInfo imageInfoCurrent = givenImageInfoCurrent;

    if (imageInfoCurrent.isNull() && !l.isEmpty())
        imageInfoCurrent = l.first();

    d->barView->blockSignals(true);
    for (ImageInfoList::const_iterator it = l.constBegin(); it != l.constEnd(); ++it)
    {
        if (!d->barView->findItemByInfo(*it))
        {
            new LightTableBarItem(d->barView, *it);
        }
    }
    d->barView->blockSignals(false);

    refreshStatusBar();
}

bool LightTableWindow::isEmpty() const
{
    return d->barView->countItems() == 0;
}

void LightTableWindow::refreshStatusBar()
{
    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                   i18np("%1 item on Light Table", "%1 items on Light Table",
                   d->barView->countItems()));
}

void LightTableWindow::slotFileChanged(const QString& path)
{
    KUrl url = KUrl::fromPath(path);
    d->barView->reloadThumbs(url);

    if (!d->previewView->leftImageInfo().isNull())
    {
        if (d->previewView->leftImageInfo().fileUrl() == url)
        {
            d->previewView->leftReload();
            d->leftSideBar->itemChanged(d->previewView->leftImageInfo());
        }
    }

    if (!d->previewView->rightImageInfo().isNull())
    {
        if (d->previewView->rightImageInfo().fileUrl() == url)
        {
            d->previewView->rightReload();
            d->rightSideBar->itemChanged(d->previewView->rightImageInfo());
        }
    }
}

void LightTableWindow::slotLeftPanelLeftButtonClicked()
{
    if (d->navigateByPairAction->isChecked()) return;

    d->barView->setSelectedItem(d->barView->findItemByInfo(d->previewView->leftImageInfo()));
}

void LightTableWindow::slotRightPanelLeftButtonClicked()
{
    // With navigate by pair option, only the left panel can be selected.
    if (d->navigateByPairAction->isChecked()) return;

    d->barView->setSelectedItem(d->barView->findItemByInfo(d->previewView->rightImageInfo()));
}

void LightTableWindow::slotLeftPreviewLoaded(bool b)
{
    d->leftZoomBar->setEnabled(b);

    if (b)
    {
        d->previewView->checkForSelection(d->barView->currentItemImageInfo());
        d->barView->setOnLeftPanel(d->previewView->leftImageInfo());

        LightTableBarItem *item = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(d->previewView->leftImageInfo()));
        if (item) item->setOnLeftPanel(true);

        if (d->navigateByPairAction->isChecked() && item)
        {
            LightTableBarItem* next = dynamic_cast<LightTableBarItem*>(item->next());
            if (next)
            {
                d->barView->setOnRightPanel(next->info());
                slotSetItemOnRightPanel(next->info());
            }
            else
            {
                LightTableBarItem* first = dynamic_cast<LightTableBarItem*>(d->barView->firstItem());
                slotSetItemOnRightPanel(first ? first->info() : ImageInfo());
            }
        }
    }
}

void LightTableWindow::slotRightPreviewLoaded(bool b)
{
    d->rightZoomBar->setEnabled(b);
    if (b)
    {
        d->previewView->checkForSelection(d->barView->currentItemImageInfo());
        d->barView->setOnRightPanel(d->previewView->rightImageInfo());

        LightTableBarItem *item = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(d->previewView->rightImageInfo()));
        if (item) item->setOnRightPanel(true);
    }
}

void LightTableWindow::slotItemSelected(const ImageInfo& info)
{
    if (!info.isNull())
    {
        d->setItemLeftAction->setEnabled(true);
        d->setItemRightAction->setEnabled(true);
        d->editItemAction->setEnabled(true);
        d->removeItemAction->setEnabled(true);
        d->clearListAction->setEnabled(true);
        d->fileDeleteAction->setEnabled(true);
        d->backwardAction->setEnabled(true);
        d->forwardAction->setEnabled(true);
        d->firstAction->setEnabled(true);
        d->lastAction->setEnabled(true);
        d->syncPreviewAction->setEnabled(true);
        d->zoomPlusAction->setEnabled(true);
        d->zoomMinusAction->setEnabled(true);
        d->navigateByPairAction->setEnabled(true);
        d->slideShowAction->setEnabled(true);

        LightTableBarItem* curr = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(info));
        if (curr)
        {
            if (!curr->prev())
            {
                d->firstAction->setEnabled(false);
            }

            if (!curr->next())
            {
                d->lastAction->setEnabled(false);
            }

            if (d->navigateByPairAction->isChecked())
            {
                d->setItemLeftAction->setEnabled(false);
                d->setItemRightAction->setEnabled(false);

                d->barView->setOnLeftPanel(info);
                slotSetItemOnLeftPanel(info);
            }
            else if (d->autoLoadOnRightPanel && !curr->isOnLeftPanel())
            {
                d->barView->setOnRightPanel(info);
                slotSetItemOnRightPanel(info);
            }
        }
    }
    else
    {
        d->setItemLeftAction->setEnabled(false);
        d->setItemRightAction->setEnabled(false);
        d->editItemAction->setEnabled(false);
        d->removeItemAction->setEnabled(false);
        d->clearListAction->setEnabled(false);
        d->fileDeleteAction->setEnabled(false);
        d->backwardAction->setEnabled(false);
        d->forwardAction->setEnabled(false);
        d->firstAction->setEnabled(false);
        d->lastAction->setEnabled(false);
        d->zoomPlusAction->setEnabled(false);
        d->zoomMinusAction->setEnabled(false);
        d->syncPreviewAction->setEnabled(false);
        d->navigateByPairAction->setEnabled(false);
        d->slideShowAction->setEnabled(false);
    }

    d->previewView->checkForSelection(info);
}

// Deal with one (or more) items dropped onto the left panel
void LightTableWindow::slotLeftDroppedItems(const ImageInfoList& list)
{
    ImageInfo info = list.first();
    // add the image to the existing images
    loadImageInfos(list, info, true);

    // We will check if first item from list is already stored in thumbbar
    // Note that the thumbbar stores all ImageInfo reference
    // in memory for preview object.
    LightTableBarItem *item = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(info));
    if (item)
    {
        slotSetItemOnLeftPanel(item->info());
    }
}

// Deal with one (or more) items dropped onto the right panel
void LightTableWindow::slotRightDroppedItems(const ImageInfoList& list)
{
    ImageInfo info = list.first();
    // add the image to the existing images
    loadImageInfos(list, info, true);

    // We will check if first item from list is already stored in thumbbar
    // Note that the thumbbar stores all ImageInfo reference
    // in memory for preview object.
    LightTableBarItem *item = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(info));
    if (item)
    {
        slotSetItemOnRightPanel(item->info());
        // Make this item the current one.
        d->barView->setSelectedItem(item);
    }
}

// Set the images for the left and right panel.
void LightTableWindow::setLeftRightItems(const ImageInfoList& list, bool addTo)
{
    ImageInfoList l = list;

    if (l.count() == 0)
        return;

    ImageInfo info            = l.first();
    LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(info));

    if (l.count() == 1 && !addTo)
    {
        // Just one item; this is used for the left panel.
        d->barView->setOnLeftPanel(info);
        slotSetItemOnLeftPanel(info);
        d->barView->setSelectedItem(ltItem);
        d->barView->ensureItemVisible(ltItem);
        return;
    }

    if (ltItem)
    {
        // The first item is used for the left panel.
        if (!addTo)
        {
            d->barView->setOnLeftPanel(info);
                slotSetItemOnLeftPanel(info);
        }

        // The subsequent item is used for the right panel.
        LightTableBarItem* next = dynamic_cast<LightTableBarItem*>(ltItem->next());
        if (next && !addTo)
        {
            d->barView->setOnRightPanel(next->info());
            slotSetItemOnRightPanel(next->info());

            if (!d->navigateByPairAction->isChecked())
            {
                d->barView->setSelectedItem(next);
                // ensure that the selected item is visible
                // FIXME: this does not work:
                d->barView->ensureItemVisible(next);
            }
        }

        // If navigate by pairs is active, the left panel item is selected.
        // (Fixes parts of bug #150296)
        if (d->navigateByPairAction->isChecked())
        {
            d->barView->setSelectedItem(ltItem);
            d->barView->ensureItemVisible(ltItem);
        }
    }
}

void LightTableWindow::slotSetItemLeft()
{
    if (!d->barView->currentItemImageInfo().isNull())
    {
        slotSetItemOnLeftPanel(d->barView->currentItemImageInfo());
    }
}

void LightTableWindow::slotSetItemRight()
{
    if (d->barView->currentItemImageInfo().isNull())
    {
        slotSetItemOnRightPanel(d->barView->currentItemImageInfo());
    }
}

void LightTableWindow::slotSetItemOnLeftPanel(const ImageInfo& info)
{
    d->previewView->setLeftImageInfo(info);
    if (!info.isNull())
        d->leftSideBar->itemChanged(info);
    else
        d->leftSideBar->slotNoCurrentItem();
}

void LightTableWindow::slotSetItemOnRightPanel(const ImageInfo& info)
{
    d->previewView->setRightImageInfo(info);
    if (!info.isNull())
        d->rightSideBar->itemChanged(info);
    else
        d->rightSideBar->slotNoCurrentItem();
}

void LightTableWindow::slotClearItemsList()
{
    if (!d->previewView->leftImageInfo().isNull())
    {
        d->previewView->setLeftImageInfo();
        d->leftSideBar->slotNoCurrentItem();
    }

    if (!d->previewView->rightImageInfo().isNull())
    {
        d->previewView->setRightImageInfo();
        d->rightSideBar->slotNoCurrentItem();
    }

    d->barView->clear();
    refreshStatusBar();
}

void LightTableWindow::slotDeleteItem()
{
    if (!d->barView->currentItemImageInfo().isNull())
        slotDeleteItem(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotDeleteItem(const ImageInfo& info)
{
    bool ask         = true;
    bool permanently = false;

    KUrl u = info.fileUrl();
    PAlbum *palbum = AlbumManager::instance()->findPAlbum(u.directory());
    if (!palbum)
        return;

    // Provide a digikamalbums:// URL to KIO
    KUrl kioURL  = info.databaseUrl();
    KUrl fileURL = u;

    bool useTrash;

    if (ask)
    {
        bool preselectDeletePermanently = permanently;

        DeleteDialog dialog(this);

        KUrl::List urlList;
        urlList.append(u);
        if (!dialog.confirmDeleteList(urlList,
             DeleteDialogMode::Files,
             preselectDeletePermanently ?
                     DeleteDialogMode::NoChoiceDeletePermanently : DeleteDialogMode::NoChoiceTrash))
            return;

        useTrash = !dialog.shouldDelete();
    }
    else
    {
        useTrash = !permanently;
    }

    // trash does not like non-local URLs, put is not implemented
    if (useTrash)
        kioURL = fileURL;

    SyncJobResult deleteResult = SyncJob::del(kioURL, useTrash);
    if (!deleteResult)
    {
        KMessageBox::error(this, deleteResult.errorString);
        return;
    }

    emit signalFileDeleted(u);

    slotRemoveItem(info);
}

void LightTableWindow::slotRemoveItem()
{
    if (!d->barView->currentItemImageInfo().isNull())
        slotRemoveItem(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotRemoveItem(const ImageInfo& info)
{
/*    if (!d->previewView->leftImageInfo().isNull())
    {
        if (d->previewView->leftImageInfo() == info)
        {
            d->previewView->setLeftImageInfo();
            d->leftSideBar->slotNoCurrentItem();
        }
    }

    if (!d->previewView->rightImageInfo().isNull())
    {
        if (d->previewView->rightImageInfo() == info)
        {
            d->previewView->setRightImageInfo();
            d->rightSideBar->slotNoCurrentItem();
        }
    }

    d->barView->removeItemByInfo(info);
    d->barView->setSelected(d->barView->currentItem());
*/

    // When either the image from the left or right panel is removed,
    // there are various situations to account for.
    // To describe them, 4 images A B C D are used
    // and the subscript _L and _ R  mark the currently
    // active item on the left and right panel

    bool leftPanelActive = false;
    ImageInfo curr_linfo = d->previewView->leftImageInfo();
    ImageInfo curr_rinfo = d->previewView->rightImageInfo();
    ImageInfo new_linfo;
    ImageInfo new_rinfo;

    qint64 infoId = info.id();

    // First determine the next images to the current left and right image:
    ImageInfo next_linfo;
    ImageInfo next_rinfo;

    if (!curr_linfo.isNull())
    {
        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(curr_linfo));
        if (ltItem)
        {
            LightTableBarItem* next = dynamic_cast<LightTableBarItem*>(ltItem->next());
            if (next)
            {
                next_linfo = next->info();
            }
        }
    }

    if (!curr_rinfo.isNull())
    {
        LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(curr_rinfo));
        if (ltItem)
        {
            LightTableBarItem* next = dynamic_cast<LightTableBarItem*>(ltItem->next());
            if (next)
            {
                next_rinfo = next->info();
            }
        }
    }

    d->barView->removeItemByInfo(info);

    // Make sure that next_linfo and next_rinfo are still available:
    if (!d->barView->findItemByInfo(next_linfo))
    {
         next_linfo = ImageInfo();
    }
    if (!d->barView->findItemByInfo(next_rinfo))
    {
         next_rinfo = ImageInfo();
    }

    // removal of the left panel item?
    if (!curr_linfo.isNull())
    {
        if ( curr_linfo.id() == infoId )
        {
            leftPanelActive = true;
            // Delete the item A_L of the left panel:
            // 1)  A_L  B_R  C    D   ->   B_L  C_R  D
            // 2)  A_L  B    C_R  D   ->   B    C_L  D_R
            // 3)  A_L  B    C    D_R ->   B_R  C    D_L
            // 4)  A_L  B_R           ->   A_L
            // some more corner cases:
            // 5)  A    B_L  C_R  D   ->   A    C_L  D_R
            // 6)  A    B_L  C_R      ->   A_R  C_L
            // 7)  A_LR B    C    D   ->   B_L    C_R  D  (does not yet work)
            // I.e. in 3) we wrap around circularly.

            // When removing the left panel image,
            // put the right panel image into the left panel.
            // Check if this one is not the same (i.e. also removed).
            if (!curr_rinfo.isNull())
            {
                if (curr_rinfo.id() != infoId)
                {
                    new_linfo = curr_rinfo;
                    // Set the right panel to the next image:
                    new_rinfo = next_rinfo;
                    // set the right panel active
                    leftPanelActive = false;
                }
            }
        }
    }

    // removal of the right panel item?
    if (!curr_rinfo.isNull())
    {
        if (curr_rinfo.id() == infoId)
        {
            // Leave the left panel as the current one
            new_linfo = curr_linfo;
            // Set the right panel to the next image
            new_rinfo = next_rinfo;
        }
    }

    // Now we deal with the corner cases, where no left or right item exists.
    // If the right panel would be set, but not the left-one, then swap
    if (new_linfo.isNull() && !new_rinfo.isNull())
    {
        new_linfo       = new_rinfo;
        new_rinfo       = ImageInfo();
        leftPanelActive = true;
    }

    if (new_linfo.isNull())
    {
        if (d->barView->countItems() > 0)
        {
            LightTableBarItem* first = dynamic_cast<LightTableBarItem*>(d->barView->firstItem());
            new_linfo = first->info();
        }
    }

    // Make sure that new_linfo and new_rinfo exist.
    // This addresses a crash occurring if the last image is removed
    // in the navigate by pairs mode.
    if (!d->barView->findItemByInfo(new_linfo))
    {
         new_linfo = ImageInfo();
    }
    if (!d->barView->findItemByInfo(new_rinfo))
    {
         new_rinfo = ImageInfo();
    }

    // no right item defined?
    if (new_rinfo.isNull())
    {
        // If there are at least two items, we can find reasonable right image.
        if (d->barView->countItems() > 1)
        {
            // See if there is an item next to the left one:
            LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(new_linfo));
            LightTableBarItem* next   = 0;
            if (ltItem)
            {
                next = dynamic_cast<LightTableBarItem*>(ltItem->next());
            }
            if (next)
            {
                new_rinfo = next->info();
            }
            else
            {
                // If there is no item to the right of new_linfo
                // then we can choose the first item for new_rinfo
                // (as we made sure that there are at least two items)
                LightTableBarItem* first = dynamic_cast<LightTableBarItem*>(d->barView->firstItem());
                new_rinfo               = first->info();
            }
        }
    }

    // Check if left and right are set to the same
    if (!new_linfo.isNull() && !new_rinfo.isNull())
    {
        if (new_linfo.id() == new_rinfo.id())
        {
            // Only keep the left one
            new_rinfo = ImageInfo();
        }
    }

    // If the right panel would be set, but not the left-one, then swap
    // (note that this has to be done here again!)
    if (new_linfo.isNull() && !new_rinfo.isNull())
    {
        new_linfo       = new_rinfo;
        new_rinfo       = ImageInfo();
        leftPanelActive = true;
    }

    // set the image for the left panel
    if (!new_linfo.isNull())
    {
        d->barView->setOnLeftPanel(new_linfo);
        slotSetItemOnLeftPanel(new_linfo);

        //  make this the selected item if the left was active before
        if ( leftPanelActive)
        {
            LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(new_linfo));
            d->barView->setSelectedItem(ltItem);
        }
    }
    else
    {
        d->previewView->setLeftImageInfo();
        d->leftSideBar->slotNoCurrentItem();
    }

    // set the image for the right panel
    if (!new_rinfo.isNull())
    {
        d->barView->setOnRightPanel(new_rinfo);
        slotSetItemOnRightPanel(new_rinfo);
        //  make this the selected item if the left was active before
        if (!leftPanelActive)
        {
            LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(new_rinfo));
            d->barView->setSelectedItem(ltItem);
        }
    }
    else
    {
        d->previewView->setRightImageInfo();
        d->rightSideBar->slotNoCurrentItem();
    }

    refreshStatusBar();
}

void LightTableWindow::slotEditItem()
{
    if (!d->barView->currentItemImageInfo().isNull())
        slotEditItem(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotEditItem(const ImageInfo& info)
{
    ImageWindow *im    = ImageWindow::imagewindow();
    ImageInfoList list = d->barView->itemsImageInfoList();

    im->loadImageInfos(list, info, i18n("Light Table"), true);

    if (im->isHidden())
        im->show();
    else
        im->raise();

    im->setFocus();
}

void LightTableWindow::slotZoomTo100Percents()
{
    d->previewView->toggleFitToWindowOr100();
}

void LightTableWindow::slotFitToWindow()
{
    d->previewView->fitToWindow();
}

void LightTableWindow::slotToggleSlideShow()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    bool startWithCurrent     = group.readEntry("SlideShowStartCurrent", false);

    SlideShowSettings settings;
    settings.exifRotate           = AlbumSettings::instance()->getExifRotate();
    settings.ratingColor          = ThemeEngine::instance()->textSpecialRegColor();
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

void LightTableWindow::slideShow(bool startWithCurrent, SlideShowSettings& settings)
{
    if (!d->barView->countItems()) return;

    int              i = 0;
    d->cancelSlideShow = false;

    d->statusProgressBar->progressBarMode(StatusProgressBar::CancelProgressBarMode,
                                  i18n("Preparing slideshow. Please wait..."));

    ImageInfoList list = d->barView->itemsImageInfoList();

    for (ImageInfoList::const_iterator it = list.constBegin();
         !d->cancelSlideShow && it != list.constEnd() ; ++it)
    {
        SlidePictureInfo pictInfo;
        pictInfo.comment   = (*it).comment();
        pictInfo.rating    = (*it).rating();
        pictInfo.photoInfo = (*it).photoInfoContainer();
        settings.pictInfoMap.insert((*it).fileUrl(), pictInfo);
        settings.fileList.append((*it).fileUrl());

        d->statusProgressBar->setProgressValue((int)((i++/(float)list.count())*100.0));
        kapp->processEvents();
    }

    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, QString());
    refreshStatusBar();

    if (!d->cancelSlideShow)
    {
        settings.exifRotate = AlbumSettings::instance()->getExifRotate();

        SlideShow *slide = new SlideShow(settings);
        if (startWithCurrent)
            slide->setCurrent(d->barView->currentItemImageInfo().fileUrl());

        slide->show();
    }
}

void LightTableWindow::slotProgressBarCancelButtonPressed()
{
    d->cancelSlideShow = true;
}

void LightTableWindow::slotToggleFullScreen()
{
    if (d->fullScreen) // out of fullscreen
    {
        setWindowState( windowState() & ~Qt::WindowFullScreen ); // reset

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
                    toolbar->removeAction(d->fullScreenAction);
                    break;
                }
            }
        }

        d->leftSideBar->restore();
        d->rightSideBar->restore();

        d->fullScreen = false;
    }
    else  // go to fullscreen
    {
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
            if ( mainToolbar && !mainToolbar->actions().contains(d->fullScreenAction) )
            {
                mainToolbar->addAction(d->fullScreenAction);
                d->removeFullScreenButton=true;
            }
            else
            {
                // If FullScreen button is enabled in toolbar settings,
                // we shall not remove it when leaving of fullscreen mode.
                d->removeFullScreenButton=false;
            }
        }

        d->leftSideBar->backup();
        d->rightSideBar->backup();

        setWindowState( windowState() | Qt::WindowFullScreen ); // set
        d->fullScreen = true;
    }
}

void LightTableWindow::slotEscapePressed()
{
    if (d->fullScreen)
        d->fullScreenAction->activate(QAction::Trigger);
}

void LightTableWindow::showToolBars()
{
    QList<KToolBar *> toolbars = toolBars();
    foreach(KToolBar *toolbar, toolbars)
    {
        toolbar->show();
    }
}

void LightTableWindow::hideToolBars()
{
    QList<KToolBar *> toolbars = toolBars();
    foreach(KToolBar *toolbar, toolbars)
    {
        toolbar->hide();
    }
}

void LightTableWindow::slotDonateMoney()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=donation");
}

void LightTableWindow::slotContribute()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=contrib");
}

void LightTableWindow::slotEditKeys()
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection( actionCollection(), i18n( "General" ) );
    dialog.configure();
}

void LightTableWindow::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group("LightTable Settings"));
    KEditToolBar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void LightTableWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group("LightTable Settings"));
}

void LightTableWindow::slotSetup()
{
    Setup::exec(this);
}

void LightTableWindow::slotLeftZoomFactorChanged(double zoom)
{
    double h    = (double)ThumbnailSize::Huge;
    double s    = (double)ThumbnailSize::Small;
    double zmin = d->previewView->leftZoomMin();
    double zmax = d->previewView->leftZoomMax();
    double b    = (zmin-(zmax*s/h))/(1-s/h);
    double a    = (zmax-b)/h;
    int size    = (int)((zoom - b) /a);

    d->leftZoomBar->setZoomSliderValue(size);
    d->leftZoomBar->setZoomTrackerText(i18n("zoom: %1%",(int)(zoom*100.0)));
    d->leftZoomBar->triggerZoomTrackerToolTip();

    d->leftZoomBar->setEnableZoomPlus(true);
    d->leftZoomBar->setEnableZoomMinus(true);

    if (d->previewView->leftMaxZoom())
        d->leftZoomBar->setEnableZoomPlus(false);

    if (d->previewView->leftMinZoom())
        d->leftZoomBar->setEnableZoomMinus(false);
}

void LightTableWindow::slotRightZoomFactorChanged(double zoom)
{
    double h    = (double)ThumbnailSize::Huge;
    double s    = (double)ThumbnailSize::Small;
    double zmin = d->previewView->rightZoomMin();
    double zmax = d->previewView->rightZoomMax();
    double b    = (zmin-(zmax*s/h))/(1-s/h);
    double a    = (zmax-b)/h;
    int size    = (int)((zoom - b) /a);

    d->rightZoomBar->setZoomSliderValue(size);
    d->rightZoomBar->setZoomTrackerText(i18n("zoom: %1%",(int)(zoom*100.0)));
    d->rightZoomBar->triggerZoomTrackerToolTip();

    d->rightZoomBar->setEnableZoomPlus(true);
    d->rightZoomBar->setEnableZoomMinus(true);

    if (d->previewView->rightMaxZoom())
        d->rightZoomBar->setEnableZoomPlus(false);

    if (d->previewView->rightMinZoom())
        d->rightZoomBar->setEnableZoomMinus(false);
}

void LightTableWindow::slotToggleSyncPreview()
{
    d->previewView->setSyncPreview(d->syncPreviewAction->isChecked());
}

void LightTableWindow::slotToggleOnSyncPreview(bool t)
{
    d->syncPreviewAction->setEnabled(t);

    if (!t)
    {
        d->syncPreviewAction->setChecked(false);
    }
    else
    {
        if (d->autoSyncPreview)
            d->syncPreviewAction->setChecked(true);
    }
}

void LightTableWindow::slotBackward()
{
    ThumbBarItem* curr = d->barView->currentItem();
    ThumbBarItem* last = d->barView->lastItem();
    if (curr)
    {
        if (curr->prev())
            d->barView->setSelected(curr->prev());
        else
            d->barView->setSelected(last);
    }
}

void LightTableWindow::slotForward()
{
    ThumbBarItem* curr = d->barView->currentItem();
    ThumbBarItem* first = d->barView->firstItem();
    if (curr)
    {
        if (curr->next())
            d->barView->setSelected(curr->next());
        else
            d->barView->setSelected(first);
    }
}

void LightTableWindow::slotFirst()
{
    d->barView->setSelected( d->barView->firstItem() );
}

void LightTableWindow::slotLast()
{
    d->barView->setSelected( d->barView->lastItem() );
}

void LightTableWindow::slotToggleNavigateByPair()
{
    d->barView->setNavigateByPair(d->navigateByPairAction->isChecked());
    d->previewView->setNavigateByPair(d->navigateByPairAction->isChecked());
    slotItemSelected(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotRawCameraList()
{
    showRawCameraList();
}

void LightTableWindow::slotThemeChanged()
{
    QStringList themes(ThemeEngine::instance()->themeNames());
    int index = themes.indexOf(AlbumSettings::instance()->getCurrentTheme());
    if (index == -1)
        index = themes.indexOf(i18n("Default"));

    d->themeMenuAction->setCurrentItem(index);
}

void LightTableWindow::slotChangeTheme(const QString& theme)
{
    // Theme menu entry is returned with keyboard accelerator. We remove it.
    QString name = theme;
    name.remove(QChar('&'));
    AlbumSettings::instance()->setCurrentTheme(theme);
    ThemeEngine::instance()->slotChangeTheme(theme);
}

void LightTableWindow::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void LightTableWindow::slotDBStat()
{
    showDigikamDatabaseStat();
}

void LightTableWindow::slotShowMenuBar()
{
    const bool visible = menuBar()->isVisible();
    menuBar()->setVisible(!visible);
}

void LightTableWindow::slotSidebarTabTitleStyleChanged()
{
    d->leftSideBar->setStyle(AlbumSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->setStyle(AlbumSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->applySettings();
}

void LightTableWindow::moveEvent(QMoveEvent *e)
{
    Q_UNUSED(e)
    emit signalWindowHasMoved();
}

}  // namespace Digikam
