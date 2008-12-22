/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qdockarea.h>

// KDE includes.

#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kwin.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kstatusbar.h>
#include <kmenubar.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000106
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes.

#include "ddebug.h"
#include "dlogoaction.h"
#include "themeengine.h"
#include "dimg.h"
#include "dmetadata.h"
#include "albumsettings.h"
#include "albummanager.h"
#include "deletedialog.h"
#include "imagewindow.h"
#include "slideshow.h"
#include "setup.h"
#include "syncjob.h"
#include "thumbnailsize.h"
#include "rawcameradlg.h"
#include "lighttablepreview.h"
#include "lighttablewindowprivate.h"
#include "lighttablewindow.h"
#include "lighttablewindow.moc"

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
                : KMainWindow(0, "lighttable", WType_TopLevel)
{
    d = new LightTableWindowPriv;
    m_instance = this;

    setCaption(i18n("Light Table"));

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupStatusBar();
    setupActions();
    setupAccelerators();

    // Make signals/slots connections

    setupConnections();

    //-------------------------------------------------------------

    d->leftSidebar->loadViewState();
    d->rightSidebar->loadViewState();
    d->leftSidebar->populateTags();
    d->rightSidebar->populateTags();

    readSettings();
    applySettings();
    setAutoSaveSettings("LightTable Settings");
}

LightTableWindow::~LightTableWindow()
{
    m_instance = 0;

    delete d->barView;
    delete d->rightSidebar;
    delete d->leftSidebar;
    delete d;
}

void LightTableWindow::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("LightTable Settings");

    if(config->hasKey("Vertical Splitter Sizes"))
        d->vSplitter->setSizes(config->readIntListEntry("Vertical Splitter Sizes"));

    if(config->hasKey("Horizontal Splitter Sizes"))
        d->hSplitter->setSizes(config->readIntListEntry("Horizontal Splitter Sizes"));

    d->navigateByPairAction->setChecked(config->readBoolEntry("Navigate By Pair", false));
    slotToggleNavigateByPair();
}

void LightTableWindow::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("LightTable Settings");
    config->writeEntry("Vertical Splitter Sizes", d->vSplitter->sizes());
    config->writeEntry("Horizontal Splitter Sizes", d->hSplitter->sizes());
    config->writeEntry("Navigate By Pair", d->navigateByPairAction->isChecked());
    config->sync();
}

void LightTableWindow::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("LightTable Settings");

    d->autoLoadOnRightPanel  = config->readBoolEntry("Auto Load Right Panel",   true);
    d->autoSyncPreview       = config->readBoolEntry("Auto Sync Preview",       true);
    d->fullScreenHideToolBar = config->readBoolEntry("FullScreen Hide ToolBar", false);
    d->previewView->setLoadFullImageSize(config->readBoolEntry("Load Full Image size", false));
    refreshView();
}

void LightTableWindow::refreshView()
{
    d->leftSidebar->refreshTagsView();
    d->rightSidebar->refreshTagsView();
}

void LightTableWindow::closeEvent(QCloseEvent* e)
{
    if (!e) return;

    writeSettings();

    e->accept();
}

void LightTableWindow::setupUserArea()
{
    QWidget* mainW    = new QWidget(this);
    d->hSplitter      = new QSplitter(Qt::Horizontal, mainW);
    QHBoxLayout *hlay = new QHBoxLayout(mainW);
    d->leftSidebar    = new ImagePropertiesSideBarDB(mainW,
                            "LightTable Left Sidebar", d->hSplitter,
                            Sidebar::Left, true);

    QWidget* centralW = new QWidget(d->hSplitter);
    QVBoxLayout *vlay = new QVBoxLayout(centralW);
    d->vSplitter      = new QSplitter(Qt::Vertical, centralW);
    d->barView        = new LightTableBar(d->vSplitter, ThumbBarView::Horizontal,
                                          AlbumSettings::instance()->getExifRotate());
    d->previewView    = new LightTableView(d->vSplitter);
    vlay->addWidget(d->vSplitter);

    d->rightSidebar   = new ImagePropertiesSideBarDB(mainW,
                            "LightTable Right Sidebar", d->hSplitter,
                            Sidebar::Right, true);

    hlay->addWidget(d->leftSidebar);
    hlay->addWidget(d->hSplitter);
    hlay->addWidget(d->rightSidebar);

    d->hSplitter->setFrameStyle( QFrame::NoFrame );
    d->hSplitter->setFrameShadow( QFrame::Plain );
    d->hSplitter->setFrameShape( QFrame::NoFrame );
    d->hSplitter->setOpaqueResize(false);
    d->vSplitter->setFrameStyle( QFrame::NoFrame );
    d->vSplitter->setFrameShadow( QFrame::Plain );
    d->vSplitter->setFrameShape( QFrame::NoFrame );
    d->vSplitter->setOpaqueResize(false);

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

    // Thumbs bar connections ---------------------------------------

    connect(d->barView, SIGNAL(signalSetItemOnLeftPanel(ImageInfo*)),
           this, SLOT(slotSetItemOnLeftPanel(ImageInfo*)));

    connect(d->barView, SIGNAL(signalSetItemOnRightPanel(ImageInfo*)),
           this, SLOT(slotSetItemOnRightPanel(ImageInfo*)));

    connect(d->barView, SIGNAL(signalRemoveItem(ImageInfo*)),
           this, SLOT(slotRemoveItem(ImageInfo*)));

    connect(d->barView, SIGNAL(signalEditItem(ImageInfo*)),
           this, SLOT(slotEditItem(ImageInfo*)));

    connect(d->barView, SIGNAL(signalClearAll()),
           this, SLOT(slotClearItemsList()));

    connect(d->barView, SIGNAL(signalLightTableBarItemSelected(ImageInfo*)),
           this, SLOT(slotItemSelected(ImageInfo*)));

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

    connect(d->previewView, SIGNAL(signalEditItem(ImageInfo*)),
           this, SLOT(slotEditItem(ImageInfo*)));

    connect(d->previewView, SIGNAL(signalDeleteItem(ImageInfo*)),
           this, SLOT(slotDeleteItem(ImageInfo*)));

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
}

void LightTableWindow::setupActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    d->backwardAction = KStdAction::back(this, SLOT(slotBackward()),
                                    actionCollection(), "lighttable_backward");
    d->backwardAction->setEnabled(false);

    d->forwardAction = KStdAction::forward(this, SLOT(slotForward()),
                                   actionCollection(), "lighttable_forward");
    d->forwardAction->setEnabled(false);

    d->firstAction = new KAction(i18n("&First"), "start",
                                 KStdAccel::shortcut( KStdAccel::Home),
                                 this, SLOT(slotFirst()),
                                 actionCollection(), "lighttable_first");
    d->firstAction->setEnabled(false);

    d->lastAction = new KAction(i18n("&Last"), "finish",
                                KStdAccel::shortcut( KStdAccel::End),
                                this, SLOT(slotLast()),
                                actionCollection(), "lighttable_last");
    d->lastAction->setEnabled(false);

    d->setItemLeftAction = new KAction(i18n("On Left"), "previous",
                                       CTRL+Key_L, this, SLOT(slotSetItemLeft()),
                                       actionCollection(), "lighttable_setitemleft");
    d->setItemLeftAction->setEnabled(false);
    d->setItemLeftAction->setWhatsThis(i18n("Show item on left panel"));

    d->setItemRightAction = new KAction(i18n("On Right"), "next",
                                       CTRL+Key_R, this, SLOT(slotSetItemRight()),
                                       actionCollection(), "lighttable_setitemright");
    d->setItemRightAction->setEnabled(false);
    d->setItemRightAction->setWhatsThis(i18n("Show item on right panel"));

    d->editItemAction = new KAction(i18n("Edit"), "editimage",
                                       Key_F4, this, SLOT(slotEditItem()),
                                       actionCollection(), "lighttable_edititem");
    d->editItemAction->setEnabled(false);

    d->removeItemAction = new KAction(i18n("Remove item from LightTable"), "fileclose",
                                       CTRL+Key_K, this, SLOT(slotRemoveItem()),
                                       actionCollection(), "lighttable_removeitem");
    d->removeItemAction->setEnabled(false);

    d->clearListAction = new KAction(i18n("Remove all items from LightTable"), "editshred",
                                     CTRL+SHIFT+Key_K, this, SLOT(slotClearItemsList()),
                                     actionCollection(), "lighttable_clearlist");
    d->clearListAction->setEnabled(false);

    d->fileDeleteAction = new KAction(i18n("Move to Trash"), "edittrash",
                                     Key_Delete,
                                     this, SLOT(slotDeleteItem()),
                                     actionCollection(), "lighttable_filedelete");
    d->fileDeleteAction->setEnabled(false);

    KStdAction::close(this, SLOT(close()), actionCollection(), "lighttable_close");

    // -- Standard 'View' menu actions ---------------------------------------------

    d->syncPreviewAction = new KToggleAction(i18n("Synchronize"), "goto",
                                            CTRL+SHIFT+Key_Y, this,
                                            SLOT(slotToggleSyncPreview()),
                                            actionCollection(), "lighttable_syncpreview");
    d->syncPreviewAction->setEnabled(false);
    d->syncPreviewAction->setWhatsThis(i18n("Synchronize preview from left and right panels"));

    d->navigateByPairAction = new KToggleAction(i18n("By Pair"), "kcmsystem",
                                            CTRL+SHIFT+Key_P, this,
                                            SLOT(slotToggleNavigateByPair()),
                                            actionCollection(), "lighttable_navigatebypair");
    d->navigateByPairAction->setEnabled(false);
    d->navigateByPairAction->setWhatsThis(i18n("Navigate by pair with all items"));

    d->zoomPlusAction = KStdAction::zoomIn(d->previewView, SLOT(slotIncreaseZoom()),
                                          actionCollection(), "lighttable_zoomplus");
    d->zoomPlusAction->setEnabled(false);

    d->zoomMinusAction = KStdAction::zoomOut(d->previewView, SLOT(slotDecreaseZoom()),
                                             actionCollection(), "lighttable_zoomminus");
    d->zoomMinusAction->setEnabled(false);

    d->zoomTo100percents = new KAction(i18n("Zoom to 100%"), "viewmag1",
                                       ALT+CTRL+Key_0,      // NOTE: Photoshop 7 use ALT+CTRL+0.
                                       this, SLOT(slotZoomTo100Percents()),
                                       actionCollection(), "lighttable_zoomto100percents");

    d->zoomFitToWindowAction = new KAction(i18n("Fit to &Window"), "view_fit_window",
                                           CTRL+SHIFT+Key_E, this, SLOT(slotFitToWindow()),
                                           actionCollection(), "lighttable_zoomfit2window");

    // Do not use std KDE action for full screen because action text is too large for app. toolbar.
    d->fullScreenAction = new KToggleAction(i18n("Full Screen"), "window_fullscreen",
                                            CTRL+SHIFT+Key_F, this,
                                            SLOT(slotToggleFullScreen()),
                                            actionCollection(), "lighttable_fullscreen");
    d->fullScreenAction->setWhatsThis(i18n("Toggle the window to full screen mode"));

    d->slideShowAction = new KAction(i18n("Slideshow"), "slideshow", Key_F9,
                                     this, SLOT(slotToggleSlideShow()),
                                     actionCollection(),"lighttable_slideshow");

    // -- Standard 'Configure' menu actions ----------------------------------------

    d->showMenuBarAction = KStdAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());

    KStdAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStdAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStdAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // -----------------------------------------------------------------------------------------

    d->themeMenuAction = new KSelectAction(i18n("&Themes"), 0, actionCollection(), "theme_menu");
    connect(d->themeMenuAction, SIGNAL(activated(const QString&)),
            this, SLOT(slotChangeTheme(const QString&)));

    d->themeMenuAction->setItems(ThemeEngine::instance()->themeNames());
    slotThemeChanged();

    // -- Standard 'Help' menu actions ---------------------------------------------


    d->donateMoneyAction = new KAction(i18n("Donate..."),
                                       0, 0,
                                       this, SLOT(slotDonateMoney()),
                                       actionCollection(),
                                       "lighttable_donatemoney");

    d->contributeAction = new KAction(i18n("Contribute..."),
                                      0, 0,
                                      this, SLOT(slotContribute()),
                                      actionCollection(),
                                      "lighttable_contribute");

    d->rawCameraListAction = new KAction(i18n("Supported RAW Cameras"),
                                         "kdcraw",
                                         0,
                                         this,
                                         SLOT(slotRawCameraList()),
                                         actionCollection(),
                                         "lighttable_rawcameralist");


    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // -- Rating actions ---------------------------------------------------------------

    d->star0 = new KAction(i18n("Assign Rating \"No Stars\""), CTRL+Key_0,
                          d->barView, SLOT(slotAssignRatingNoStar()),
                          actionCollection(), "lighttable_ratenostar");
    d->star1 = new KAction(i18n("Assign Rating \"One Star\""), CTRL+Key_1,
                          d->barView, SLOT(slotAssignRatingOneStar()),
                          actionCollection(), "lighttable_rateonestar");
    d->star2 = new KAction(i18n("Assign Rating \"Two Stars\""), CTRL+Key_2,
                          d->barView, SLOT(slotAssignRatingTwoStar()),
                          actionCollection(), "lighttable_ratetwostar");
    d->star3 = new KAction(i18n("Assign Rating \"Three Stars\""), CTRL+Key_3,
                          d->barView, SLOT(slotAssignRatingThreeStar()),
                          actionCollection(), "lighttable_ratethreestar");
    d->star4 = new KAction(i18n("Assign Rating \"Four Stars\""), CTRL+Key_4,
                          d->barView, SLOT(slotAssignRatingFourStar()),
                          actionCollection(), "lighttable_ratefourstar");
    d->star5 = new KAction(i18n("Assign Rating \"Five Stars\""), CTRL+Key_5,
                          d->barView, SLOT(slotAssignRatingFiveStar()),
                          actionCollection(), "lighttable_ratefivestar");

    // ---------------------------------------------------------------------------------

    new DLogoAction(actionCollection(), "logo_action");

    createGUI("lighttablewindowui.rc", false);
}

void LightTableWindow::setupAccelerators()
{
    d->accelerators = new KAccel(this);

    d->accelerators->insert("Exit fullscreen", i18n("Exit Fullscreen mode"),
                    i18n("Exit fullscreen viewing mode"),
                    Key_Escape, this, SLOT(slotEscapePressed()),
                    false, true);

    d->accelerators->insert("Next Image Key_Space", i18n("Next Image"),
                    i18n("Load Next Image"),
                    Key_Space, this, SLOT(slotForward()),
                    false, true);

    d->accelerators->insert("Previous Image SHIFT+Key_Space", i18n("Previous Image"),
                    i18n("Load Previous Image"),
                    SHIFT+Key_Space, this, SLOT(slotBackward()),
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

    d->accelerators->insert("Zoom Plus Key_Plus", i18n("Zoom in"),
                    i18n("Zoom in on image"),
                    Key_Plus, d->previewView, SLOT(slotIncreaseZoom()),
                    false, true);

    d->accelerators->insert("Zoom Plus Key_Minus", i18n("Zoom out"),
                    i18n("Zoom out from image"),
                    Key_Minus, d->previewView, SLOT(slotDecreaseZoom()),
                    false, true);
}

// Deal with items dropped onto the thumbbar (e.g. from the Album view)
void LightTableWindow::slotThumbbarDroppedItems(const ImageInfoList& list)
{
    // Setting the third parameter of loadImageInfos to true
    // means that the images are added to the presently available images.
    loadImageInfos(list, 0, true);
}

// We get here either
// - via CTRL+L (from the albumview)
//     a) digikamapp.cpp:  CTRL+key_L leads to slotImageLightTable())
//     b) digikamview.cpp: void DigikamView::slotImageLightTable()
//          calls d->iconView->insertToLightTable(list, info);
//     c) albumiconview.cpp: AlbumIconView::insertToLightTable
//          calls ltview->loadImageInfos(list, current);
// - via drag&drop, i.e. calls issued by the ...Dropped... routines
void LightTableWindow::loadImageInfos(const ImageInfoList &list,
                                      ImageInfo *imageInfoCurrent,
                                      bool addTo)
{
    // Clear all items before adding new images to the light table.
    if (!addTo)
        slotClearItemsList();

    ImageInfoList l = list;

    if (!imageInfoCurrent)
        imageInfoCurrent = l.first();

    AlbumSettings *settings = AlbumSettings::instance();
    if (!settings) return;

    QString imagefilter = settings->getImageFileFilter().lower() +
                          settings->getImageFileFilter().upper();

#if KDCRAW_VERSION < 0x000106
    if (KDcrawIface::DcrawBinary::instance()->versionIsRight())
    {
        // add raw files only if dcraw is available
        imagefilter += settings->getRawFileFilter().lower() +
                       settings->getRawFileFilter().upper();
    }
#else
    imagefilter += settings->getRawFileFilter().lower() +
                   settings->getRawFileFilter().upper();
#endif

    d->barView->blockSignals(true);
    for (QPtrList<ImageInfo>::const_iterator it = l.begin(); it != l.end(); ++it)
    {
        QString fileExtension = (*it)->kurl().fileName().section( '.', -1 );

        if ( imagefilter.find(fileExtension) != -1 &&
             !d->barView->findItemByInfo(*it) )
        {
            new LightTableBarItem(d->barView, *it);
        }
    }
    d->barView->blockSignals(false);

    // if window is iconified, show it
    if (isMinimized())
    {
        KWin::deIconifyWindow(winId());
    }

    refreshStatusBar();
}

void LightTableWindow::refreshStatusBar()
{
    switch (d->barView->countItems())
    {
        case 0:
            d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                                                  i18n("No item on Light Table"));
            break;
        case 1:
            d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                                                  i18n("1 item on Light Table"));
            break;
        default:
            d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                                                  i18n("%1 items on Light Table")
                                                  .arg(d->barView->countItems()));
            break;
    }
}

void LightTableWindow::slotItemsUpdated(const KURL::List& urls)
{
    d->barView->refreshThumbs(urls);

    for (KURL::List::const_iterator it = urls.begin() ; it != urls.end() ; ++it)
    {
        if (d->previewView->leftImageInfo())
        {
            if (d->previewView->leftImageInfo()->kurl() == *it)
            {
                d->previewView->leftReload();
                d->leftSidebar->itemChanged(d->previewView->leftImageInfo());
            }
        }

        if (d->previewView->rightImageInfo())
        {
            if (d->previewView->rightImageInfo()->kurl() == *it)
            {
                d->previewView->rightReload();
                d->rightSidebar->itemChanged(d->previewView->rightImageInfo());
            }
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

        LightTableBarItem *item = d->barView->findItemByInfo(d->previewView->leftImageInfo());
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
                slotSetItemOnRightPanel(first ? first->info() : 0);
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

        LightTableBarItem *item = d->barView->findItemByInfo(d->previewView->rightImageInfo());
        if (item) item->setOnRightPanel(true);
    }
}

void LightTableWindow::slotItemSelected(ImageInfo* info)
{
    if (info)
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

        LightTableBarItem* curr = d->barView->findItemByInfo(info);
        if (curr)
        {
            if (!curr->prev())
            {
//                d->backwardAction->setEnabled(false);
                d->firstAction->setEnabled(false);
            }

            if (!curr->next())
            {
//                d->forwardAction->setEnabled(false);
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
    ImageInfo *info = *(list.begin());
    // add the image to the existing images
    loadImageInfos(list, info, true);

    // We will check if first item from list is already stored in thumbbar
    // Note that the thumbbar stores all ImageInfo reference
    // in memory for preview object.
    LightTableBarItem *item = d->barView->findItemByInfo(info);
    if (item)
    {
        slotSetItemOnLeftPanel(item->info());
        // One approach is to make this item the current one, via
        //    d->barView->setSelectedItem(item);
        // However, this is not good, because this also sets
        // the right thumb to the same image.
        // Therefore we use setLeftRightItems if there is more than
        // one item in the list of dropped images.
    }
}

// Deal with one (or more) items dropped onto the right panel
void LightTableWindow::slotRightDroppedItems(const ImageInfoList& list)
{
    ImageInfo *info = *(list.begin());
    // add the image to the existing images
    loadImageInfos(list, info, true);

    // We will check if first item from list is already stored in thumbbar
    // Note that the thumbbar stores all ImageInfo reference
    // in memory for preview object.
    LightTableBarItem *item = d->barView->findItemByInfo(info);
    if (item)
    {
        slotSetItemOnRightPanel(item->info());
        // Make this item the current one.
        d->barView->setSelectedItem(item);
    }
}

// Set the images for the left and right panel.
void LightTableWindow::setLeftRightItems(const ImageInfoList &list, bool addTo)
{
    ImageInfoList l = list;

    if (l.count() == 0)
        return;

    ImageInfo *info           = l.first();
    LightTableBarItem *ltItem = d->barView->findItemByInfo(info);

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
    if (d->barView->currentItemImageInfo())
    {
        slotSetItemOnLeftPanel(d->barView->currentItemImageInfo());
    }
}

void LightTableWindow::slotSetItemRight()
{
    if (d->barView->currentItemImageInfo())
    {
        slotSetItemOnRightPanel(d->barView->currentItemImageInfo());
    }
}

void LightTableWindow::slotSetItemOnLeftPanel(ImageInfo* info)
{
    d->previewView->setLeftImageInfo(info);
    if (info)
        d->leftSidebar->itemChanged(info);
    else
        d->leftSidebar->slotNoCurrentItem();
}

void LightTableWindow::slotSetItemOnRightPanel(ImageInfo* info)
{
    d->previewView->setRightImageInfo(info);
    if (info)
        d->rightSidebar->itemChanged(info);
    else
        d->rightSidebar->slotNoCurrentItem();
}

void LightTableWindow::slotClearItemsList()
{
    if (d->previewView->leftImageInfo())
    {
        d->previewView->setLeftImageInfo();
        d->leftSidebar->slotNoCurrentItem();
    }

    if (d->previewView->rightImageInfo())
    {
        d->previewView->setRightImageInfo();
        d->rightSidebar->slotNoCurrentItem();
    }

    d->barView->clear();
    refreshStatusBar();
}

void LightTableWindow::slotDeleteItem()
{
    if (d->barView->currentItemImageInfo())
        slotDeleteItem(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotDeleteItem(ImageInfo* info)
{
    bool ask         = true;
    bool permanently = false;

    KURL u = info->kurl();
    PAlbum *palbum = AlbumManager::instance()->findPAlbum(u.directory());
    if (!palbum)
        return;

    // Provide a digikamalbums:// URL to KIO
    KURL kioURL  = info->kurlForKIO();
    KURL fileURL = u;

    bool useTrash;

    if (ask)
    {
        bool preselectDeletePermanently = permanently;

        DeleteDialog dialog(this);

        KURL::List urlList;
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

    if (!SyncJob::del(kioURL, useTrash))
    {
        QString errMsg(SyncJob::lastErrorMsg());
        KMessageBox::error(this, errMsg, errMsg);
        return;
    }

    emit signalFileDeleted(u);

    slotRemoveItem(info);
}

void LightTableWindow::slotRemoveItem()
{
    if (d->barView->currentItemImageInfo())
        slotRemoveItem(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotRemoveItem(ImageInfo* info)
{
    // When either the image from the left or right panel is removed,
    // there are various situations to account for.
    // To describe them, 4 images A B C D are used
    // and the subscript _L and _ R  mark the currently
    // active item on the left and right panel

    bool leftPanelActive = false;
    ImageInfo *curr_linfo = d->previewView->leftImageInfo();
    ImageInfo *curr_rinfo = d->previewView->rightImageInfo();
    ImageInfo *new_linfo = 0;
    ImageInfo *new_rinfo = 0;

    Q_LLONG infoId = info->id();

    // First determine the next images to the current left and right image:
    ImageInfo *next_linfo = 0;
    ImageInfo *next_rinfo = 0;

    if (curr_linfo)
    {
        LightTableBarItem *ltItem = d->barView->findItemByInfo(curr_linfo);
        if (ltItem)
        {
            LightTableBarItem* next = dynamic_cast<LightTableBarItem*>(ltItem->next());
            if (next)
            {
                next_linfo = next->info();
            }
        }
    }

    if (curr_rinfo)
    {
        LightTableBarItem *ltItem = d->barView->findItemByInfo(curr_rinfo);
        if (ltItem)
        {
            LightTableBarItem* next = dynamic_cast<LightTableBarItem*>(ltItem->next());
            if (next)
            {
                next_rinfo = next->info();
            }
        }
    }

    d->barView->removeItem(info);

    // Make sure that next_linfo and next_rinfo are still available:
    if (!d->barView->findItemByInfo(next_linfo))
    {
         next_linfo = 0;
    }
    if (!d->barView->findItemByInfo(next_rinfo))
    {
         next_rinfo = 0;
    }

    // removal of the left panel item?
    if (curr_linfo)
    {
        if ( curr_linfo->id() == infoId )
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
            if (curr_rinfo)
            {
                if (curr_rinfo->id() != infoId)
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
    if (curr_rinfo)
    {
        if (curr_rinfo->id() == infoId)
        {
            // Leave the left panel as the current one
            new_linfo = curr_linfo;
            // Set the right panel to the next image
            new_rinfo = next_rinfo;
        }
    }

    // Now we deal with the corner cases, where no left or right item exists.
    // If the right panel would be set, but not the left-one, then swap
    if (!new_linfo && new_rinfo)
    {
        new_linfo       = new_rinfo;
        new_rinfo       = 0;
        leftPanelActive = true;
    }

    if (!new_linfo)
    {
        if (d->barView->countItems() > 0)
        {
            LightTableBarItem* first = dynamic_cast<LightTableBarItem*>(d->barView->firstItem());
            new_linfo = first->info();
        }
    }


    // Make sure that new_linfo and new_rinfo exist.
    // This addresses a crash occuring if the last image is removed
    // in the navigate by pairs mode.
    if (!d->barView->findItemByInfo(new_linfo))
    {
         new_linfo = 0;
    }
    if (!d->barView->findItemByInfo(new_rinfo))
    {
         new_rinfo = 0;
    }

    // no right item defined?
    if (!new_rinfo)
    {
        // If there are at least two items, we can find reasonable right image.
        if (d->barView->countItems() > 1)
        {
            // See if there is an item next to the left one:
            LightTableBarItem *ltItem = d->barView->findItemByInfo(new_linfo);
            LightTableBarItem* next   = 0;
            // re-check if ltItem is really set
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
    if (new_linfo && new_rinfo)
    {
        if (new_linfo->id() == new_rinfo->id())
        {
            // Only keep the left one
            new_rinfo = 0;
        }
    }

    // If the right panel would be set, but not the left-one, then swap
    // (note that this has to be done here again!)
    if (!new_linfo && new_rinfo)
    {
        new_linfo       = new_rinfo;
        new_rinfo       = 0;
        leftPanelActive = true;
    }

    // set the image for the left panel
    if (new_linfo)
    {
        d->barView->setOnLeftPanel(new_linfo);
        slotSetItemOnLeftPanel(new_linfo);

        //  make this the selected item if the left was active before
        if ( leftPanelActive)
        {
            LightTableBarItem *ltItem = d->barView->findItemByInfo(new_linfo);
            d->barView->setSelectedItem(ltItem);
        }
    }
    else
    {
        d->previewView->setLeftImageInfo();
        d->leftSidebar->slotNoCurrentItem();
    }

    // set the image for the right panel
    if (new_rinfo)
    {
        d->barView->setOnRightPanel(new_rinfo);
        slotSetItemOnRightPanel(new_rinfo);
        //  make this the selected item if the left was active before
        if (!leftPanelActive)
        {
            LightTableBarItem *ltItem = d->barView->findItemByInfo(new_rinfo);
            d->barView->setSelectedItem(ltItem);
        }
    }
    else
    {
        d->previewView->setRightImageInfo();
        d->rightSidebar->slotNoCurrentItem();
    }

    refreshStatusBar();
}

void LightTableWindow::slotEditItem()
{
    if (d->barView->currentItemImageInfo())
        slotEditItem(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotEditItem(ImageInfo* info)
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
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    bool startWithCurrent = config->readBoolEntry("SlideShowStartCurrent", false);

    SlideShowSettings settings;
    settings.exifRotate           = AlbumSettings::instance()->getExifRotate();
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

void LightTableWindow::slideShow(bool startWithCurrent, SlideShowSettings& settings)
{
    if (!d->barView->countItems()) return;

    DMetadata meta;
    int              i = 0;
    d->cancelSlideShow = false;

    d->statusProgressBar->progressBarMode(StatusProgressBar::CancelProgressBarMode,
                                  i18n("Preparing slideshow. Please wait..."));

    ImageInfoList list = d->barView->itemsImageInfoList();

    for (ImageInfo *info = list.first() ; !d->cancelSlideShow && info ; info = list.next())
    {
        SlidePictureInfo pictInfo;
        pictInfo.comment = info->caption();

        // Perform optimizations: only read pictures metadata if necessary.
        if (settings.printApertureFocal || settings.printExpoSensitivity || settings.printMakeModel)
        {
            meta.load(info->kurl().path());
            pictInfo.photoInfo = meta.getPhotographInformations();
        }

        // In case of dateTime extraction from metadata failed
        pictInfo.photoInfo.dateTime = info->dateTime();
        settings.pictInfoMap.insert(info->kurl(), pictInfo);
        settings.fileList.append(info->kurl());

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
            slide->setCurrent(d->barView->currentItemImageInfo()->kurl());

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

            if (d->fullScreenAction->isPlugged(toolBar) && d->removeFullScreenButton)
                d->fullScreenAction->unplug(toolBar);

            if (toolBar->isHidden())
                showToolBars();
        }

        // -- remove the gui action accels ----

        unplugActionAccel(d->zoomFitToWindowAction);

        if (d->fullScreen)
        {
            d->leftSidebar->restore();
            d->rightSidebar->restore();
        }
        else
        {
            d->leftSidebar->backup();
            d->rightSidebar->backup();
        }

        d->fullScreen = false;
    }
    else  // go to fullscreen
    {
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

                if ( !d->fullScreenAction->isPlugged(toolBar) )
                {
                    d->fullScreenAction->plug(toolBar);
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

        plugActionAccel(d->zoomFitToWindowAction);

        if (d->fullScreen)
        {
            d->leftSidebar->restore();
            d->rightSidebar->restore();
        }
        else
        {
            d->leftSidebar->backup();
            d->rightSidebar->backup();
        }

        showFullScreen();
        d->fullScreen = true;
    }
}

void LightTableWindow::slotEscapePressed()
{
    if (d->fullScreen)
        d->fullScreenAction->activate();
}

void LightTableWindow::showToolBars()
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

void LightTableWindow::hideToolBars()
{
    QPtrListIterator<KToolBar> it = toolBarIterator();
    KToolBar* bar;

    for( ; it.current()!=0L ; ++it)
    {
        bar=it.current();

        if (bar->area())
            bar->area()->hide();
        else
            bar->hide();
    }
}

void LightTableWindow::plugActionAccel(KAction* action)
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

void LightTableWindow::unplugActionAccel(KAction* action)
{
    d->accelerators->remove(action->text());
}

void LightTableWindow::slotDonateMoney()
{
    KApplication::kApplication()->invokeBrowser("http://www.digikam.org/?q=donation");
}

void LightTableWindow::slotContribute()
{
    KApplication::kApplication()->invokeBrowser("http://www.digikam.org/?q=contrib");
}

void LightTableWindow::slotEditKeys()
{
    KKeyDialog dialog(true, this);
    dialog.insert( actionCollection(), i18n( "General" ) );
    dialog.configure();
}

void LightTableWindow::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config(), "LightTable Settings");
    KEditToolbar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void LightTableWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config(), "LightTable Settings");
}

void LightTableWindow::slotSetup()
{
    Setup setup(this, 0);

    if (setup.exec() != QDialog::Accepted)
        return;

    kapp->config()->sync();

    applySettings();
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
    d->leftZoomBar->setZoomTrackerText(i18n("zoom: %1%").arg((int)(zoom*100.0)));

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
    d->rightZoomBar->setZoomTrackerText(i18n("zoom: %1%").arg((int)(zoom*100.0)));

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
    RawCameraDlg dlg(this);
    dlg.exec();
}

void LightTableWindow::slotThemeChanged()
{
    QStringList themes(ThemeEngine::instance()->themeNames());
    int index = themes.findIndex(AlbumSettings::instance()->getCurrentTheme());
    if (index == -1)
        index = themes.findIndex(i18n("Default"));

    d->themeMenuAction->setCurrentItem(index);
}

void LightTableWindow::slotChangeTheme(const QString& theme)
{
    AlbumSettings::instance()->setCurrentTheme(theme);
    ThemeEngine::instance()->slotChangeTheme(theme);
}

void LightTableWindow::slotShowMenuBar()
{
    if (menuBar()->isVisible())
        menuBar()->hide();
    else
        menuBar()->show();
}

}  // namespace Digikam
