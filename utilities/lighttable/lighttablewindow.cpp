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

#include <QHBoxLayout>
#include <QFrame>
#include <QVBoxLayout>

// KDE includes.

#include <kshortcutsdialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kxmlguifactory.h>
#include <kedittoolbar.h>
#include <ktoolinvocation.h>
#include <ktoggleaction.h>
#include <klocale.h>
#include <kwindowsystem.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kstatusbar.h>
#include <kmenubar.h>
#include <kglobal.h>
#include <ktoolbar.h>

// LibKDcraw includes.

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/dcrawbinary.h>

// Local includes.

#include "ddebug.h"
#include "themeengine.h"
#include "dimg.h"
#include "dlogoaction.h"
#include "dmetadata.h"
#include "albumsettings.h"
#include "albummanager.h"
#include "deletedialog.h"
#include "imagewindow.h"
#include "slideshow.h"
#include "setup.h"
#include "syncjob.h"
#include "thumbnailsize.h"
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
                : KXmlGuiWindow(0)
{
    d = new LightTableWindowPriv;
    m_instance = this;

    setWindowFlags(Qt::Window);
    setCaption(i18n("Light Table"));

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupStatusBar();
    setupActions();

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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("LightTable Settings");
    QList<int> list;
    if (group.hasKey("Vertical Splitter State")) 
    {
        QByteArray state;
        state = group.readEntry("Vertical Splitter State", state);
        d->vSplitter->restoreState(QByteArray::fromBase64(state));
    }

    if (group.hasKey("Horizontal Splitter State")) 
    {
        QByteArray state;
        state = group.readEntry("Horizontal Splitter State", state);
        d->hSplitter->restoreState(QByteArray::fromBase64(state));
    }

    d->navigateByPairAction->setChecked(group.readEntry("Navigate By Pair", false));
    slotToggleNavigateByPair();
}

void LightTableWindow::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("LightTable Settings");
    group.writeEntry("Vertical Splitter State", d->vSplitter->saveState().toBase64());
    group.writeEntry("Horizontal Splitter State", d->hSplitter->saveState().toBase64());
    group.writeEntry("Navigate By Pair", d->navigateByPairAction->isChecked());
    config->sync();
}

void LightTableWindow::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("LightTable Settings");

    d->autoLoadOnRightPanel  = group.readEntry("Auto Load Right Panel",   true);
    d->autoSyncPreview       = group.readEntry("Auto Sync Preview",       true);
    d->fullScreenHideToolBar = group.readEntry("FullScreen Hide ToolBar", false);
    d->previewView->setLoadFullImageSize(group.readEntry("Load Full Image size", false));
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
    d->leftSidebar    = new ImagePropertiesSideBarDB(mainW, d->hSplitter, Sidebar::DockLeft, true);

    QWidget* centralW = new QWidget(d->hSplitter);
    QVBoxLayout *vlay = new QVBoxLayout(centralW);
    d->vSplitter      = new QSplitter(Qt::Vertical, centralW);
    d->barView        = new LightTableBar(d->vSplitter, Qt::Horizontal, 
                                          AlbumSettings::instance()->getExifRotate());
    d->previewView    = new LightTableView(d->vSplitter);

    d->rightSidebar   = new ImagePropertiesSideBarDB(mainW, d->hSplitter, Sidebar::DockRight, true);

    vlay->addWidget(d->vSplitter);
    vlay->setSpacing(0);
    vlay->setMargin(0);

    hlay->addWidget(d->leftSidebar);
    hlay->addWidget(d->hSplitter);
    hlay->addWidget(d->rightSidebar);
    hlay->setSpacing(0);
    hlay->setMargin(0);

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
}

void LightTableWindow::setupActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    d->backwardAction = KStandardAction::back(this, SLOT(slotBackward()), this);
    actionCollection()->addAction("lighttable_backward", d->backwardAction);
    d->backwardAction->setShortcut( KShortcut(Qt::Key_Prior, Qt::Key_Backspace) );

    d->forwardAction = KStandardAction::forward(this, SLOT(slotForward()), this);
    actionCollection()->addAction("lighttable_forward", d->forwardAction);
    d->forwardAction->setEnabled(false);
    d->forwardAction->setShortcut( KShortcut(Qt::Key_Next, Qt::Key_Space) );

    d->firstAction = new KAction(KIcon("go-first"), i18n("&First"), this);
    d->firstAction->setShortcut(KStandardShortcut::Begin);
    d->firstAction->setEnabled(false);
    connect(d->firstAction, SIGNAL(triggered()), this, SLOT(slotFirst()));
    actionCollection()->addAction("lighttable_first", d->firstAction);

    d->lastAction = new KAction(KIcon("go-last"), i18n("&Last"), this);
    d->lastAction->setShortcut(KStandardShortcut::End);
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

    actionCollection()->addAction(KStandardAction::Close, "lighttable_close", 
                                  this, SLOT(close()));

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
    d->navigateByPairAction->setWhatsThis(i18n("Navigate by pair with all items"));
    connect(d->navigateByPairAction, SIGNAL(triggered()), this, SLOT(slotToggleNavigateByPair()));
    actionCollection()->addAction("lighttable_navigatebypair", d->navigateByPairAction);

    d->zoomPlusAction = actionCollection()->addAction(KStandardAction::ZoomIn, "lighttable_zoomplus", 
                                                      d->previewView, SLOT(slotIncreaseZoom()));
    d->zoomPlusAction->setEnabled(false);
    d->zoomPlusAction->setShortcut(QKeySequence(Qt::Key_Plus));

    d->zoomMinusAction = actionCollection()->addAction(KStandardAction::ZoomOut, "lighttable_zoomminus", 
                                                       d->previewView, SLOT(slotDecreaseZoom()));
    d->zoomMinusAction->setEnabled(false);
    d->zoomMinusAction->setShortcut(QKeySequence(Qt::Key_Minus));

    d->zoomTo100percents = new KAction(KIcon("zoom-original"), i18n("Zoom to 1:1"), this);
    d->zoomTo100percents->setShortcut(Qt::ALT+Qt::CTRL+Qt::Key_0);       // NOTE: Photoshop 7 use ALT+CTRL+0
    connect(d->zoomTo100percents, SIGNAL(triggered()), this, SLOT(slotZoomTo100Percents()));
    actionCollection()->addAction("lighttable_zoomto100percents", d->zoomTo100percents);

    d->zoomFitToWindowAction = new KToggleAction(KIcon("zoom-fit-best"), i18n("Fit to &Window"), this);
    d->zoomFitToWindowAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_E); // NOTE: Gimp 2 use CTRL+SHIFT+E.
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), this, SLOT(slotFitToWindow()));
    actionCollection()->addAction("lighttable_zoomfit2window", d->zoomFitToWindowAction);

    d->fullScreenAction = actionCollection()->addAction(KStandardAction::FullScreen,
                          "lighttable_fullscreen", this, SLOT(slotToggleFullScreen()));

    d->slideShowAction = new KAction(KIcon("view-presentation"), i18n("Slide Show"), this);
    d->slideShowAction->setShortcut(Qt::Key_F9);
    connect(d->slideShowAction, SIGNAL(triggered()), this, SLOT(slotToggleSlideShow()));
    actionCollection()->addAction("lighttable_slideshow", d->slideShowAction);

    // -- Standard 'Configure' menu actions ----------------------------------------

    KStandardAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStandardAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStandardAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // -- Standard 'Help' menu actions ---------------------------------------------

    d->donateMoneyAction = new KAction(i18n("Donate Money..."), this);
    connect(d->donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    actionCollection()->addAction("lighttable_donatemoney", d->donateMoneyAction);

    d->rawCameraListAction = new KAction(KIcon("kdcraw"), i18n("RAW camera supported"), this);
    connect(d->rawCameraListAction, SIGNAL(triggered()), this, SLOT(slotRawCameraList()));
    actionCollection()->addAction("lighttable_rawcameralist", d->rawCameraListAction);

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // -- Rating actions ---------------------------------------------------------------

    d->star0 = new KAction(i18n("Assign Rating \"No Star\""), this);
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

    KAction *exitFullscreenAction = new KAction(i18n("Exit Fullscreen mode"), this);
    actionCollection()->addAction("editorwindow_exitfullscreen", exitFullscreenAction);
    exitFullscreenAction->setShortcut( QKeySequence(Qt::Key_Escape) );
    connect(exitFullscreenAction, SIGNAL(triggered()), this, SLOT(slotEscapePressed()));

    // ---------------------------------------------------------------------------------

    actionCollection()->addAction("logo_action", new DLogoAction(this));

    createGUI("lighttablewindowui.rc");
}

// Deal with items dropped onto the thumbbar (e.g. from the Album view)
void LightTableWindow::slotThumbbarDroppedItems(const ImageInfoList& list)
{
    // Setting the third parameter of loadImageInfos to true
    // means that the images are added to the presently available images.
    loadImageInfos(list, ImageInfo(), true);
    setLeftRightItems(list);
}

// We get here either
// - via F6 (from the albumview)
//     a) digikamapp.cpp:  key_F6 leads to slotImageLightTable())
//     b) digikamview.cpp: void DigikamView::slotImageLightTable()
//          calls d->iconView->insertToLightTable(list, info);
//     c) albumiconview.cpp: AlbumIconView::insertToLightTable
//          calls ltview->loadImageInfos(list, current);
// - via drag&drop, i.e. calls issued by the ...Dropped... routines
void LightTableWindow::loadImageInfos(const ImageInfoList &list, 
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

    AlbumSettings *settings = AlbumSettings::instance();
    if (!settings) return;

    QString imagefilter = settings->getImageFileFilter().toLower() +
                          settings->getImageFileFilter().toUpper();

    if (KDcrawIface::DcrawBinary::instance()->versionIsRight())
    {
        // add raw files only if dcraw is available
        imagefilter += settings->getRawFileFilter().toLower() +
                       settings->getRawFileFilter().toUpper();
    }

    d->barView->blockSignals(true);
    for (ImageInfoList::const_iterator it = l.begin(); it != l.end(); ++it)
    {
        QString fileExtension = (*it).fileUrl().fileName().section( '.', -1 );

        if ( imagefilter.indexOf(fileExtension) != -1 &&
             !d->barView->findItemByInfo(*it) )
        {
            new LightTableBarItem(d->barView, *it);
        }
    }
    d->barView->blockSignals(false);

    // if window is iconified, show it
    if (isMinimized())
    {
        KWindowSystem::unminimizeWindow(winId());
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
                                                  i18n("%1 items on Light Table",
                                                  d->barView->countItems()));   
            break;
    }
}

void LightTableWindow::slotItemsUpdated(const KUrl::List& urls)
{
    d->barView->refreshThumbs(urls);

    for (KUrl::List::const_iterator it = urls.begin() ; it != urls.end() ; ++it)
    {
        if (!d->previewView->leftImageInfo().isNull())
        {
            if (d->previewView->leftImageInfo().fileUrl() == *it)
            {
                d->previewView->leftReload();
                d->leftSidebar->itemChanged(d->previewView->leftImageInfo());
            }
        }

        if (!d->previewView->rightImageInfo().isNull())
        {
            if (d->previewView->rightImageInfo().fileUrl() == *it)
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

        LightTableBarItem *item = d->barView->findItemByInfo(d->previewView->rightImageInfo());
        if (item) item->setOnRightPanel(true);
    }
}

void LightTableWindow::slotItemSelected(const ImageInfo &info)
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

        LightTableBarItem* curr = d->barView->findItemByInfo(info);
        if (curr)
        {
            if (!curr->prev())
            {
                d->backwardAction->setEnabled(false);
                d->firstAction->setEnabled(false);
            }

            if (!curr->next())
            {
                d->forwardAction->setEnabled(false);
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
    LightTableBarItem *item = d->barView->findItemByInfo(info);
    if (item) 
    {
        slotSetItemOnLeftPanel(item->info());
        // One approach is to make this item the current one, via
        // d->barView->setSelectedItem(item);
        // However, this is not good, because this also sets 
        // the right thumb to the same image.
        // Therefore we use setLeftRightItems if there is more than 
        // one item in the list of dropped images. 
    }
    setLeftRightItems(list);
}

// Deal with one (or more) items dropped onto the right panel
void LightTableWindow::slotRightDroppedItems(const ImageInfoList& list)
{
    ImageInfo info = list.first();
    // add the image to the existing images
    loadImageInfos(list, info, true);
    setLeftRightItems(list);

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
    setLeftRightItems(list);
}

// Set the images for the left and right panel.
void LightTableWindow::setLeftRightItems(const ImageInfoList &list)
{
    ImageInfoList l = list;

    if (l.count() == 0)
        return;

    ImageInfo info            = l.first();
    LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(info));

    if (l.count()==1)
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
        d->barView->setOnLeftPanel(info);
        slotSetItemOnLeftPanel(info);

        // The subsequent item is used for the right panel.
        LightTableBarItem* next = dynamic_cast<LightTableBarItem*>(ltItem->next());
        if (next)
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

void LightTableWindow::slotSetItemOnLeftPanel(const ImageInfo &info)
{
    d->previewView->setLeftImageInfo(info);
    if (!info.isNull())
        d->leftSidebar->itemChanged(info);
    else
        d->leftSidebar->slotNoCurrentItem();
}

void LightTableWindow::slotSetItemOnRightPanel(const ImageInfo &info)
{
    d->previewView->setRightImageInfo(info);
    if (!info.isNull())
        d->rightSidebar->itemChanged(info);
    else
        d->rightSidebar->slotNoCurrentItem();
}

void LightTableWindow::slotClearItemsList()
{
    if (!d->previewView->leftImageInfo().isNull())
    {
        d->previewView->setLeftImageInfo();
        d->leftSidebar->slotNoCurrentItem();
    }

    if (!d->previewView->rightImageInfo().isNull())
    {
        d->previewView->setRightImageInfo();
        d->rightSidebar->slotNoCurrentItem();
    }

    d->barView->clear();
    refreshStatusBar();
}

void LightTableWindow::slotDeleteItem()
{
    if (!d->barView->currentItemImageInfo().isNull())
        slotDeleteItem(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotDeleteItem(const ImageInfo &info)
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
    if (deleteResult)
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

void LightTableWindow::slotRemoveItem(const ImageInfo &info)
{
/*    if (!d->previewView->leftImageInfo().isNull())
    {
        if (d->previewView->leftImageInfo() == info)
        {
            d->previewView->setLeftImageInfo();
            d->leftSidebar->slotNoCurrentItem();
        }
    }

    if (!d->previewView->rightImageInfo().isNull())
    {
        if (d->previewView->rightImageInfo() == info)
        {
            d->previewView->setRightImageInfo();
            d->rightSidebar->slotNoCurrentItem();
        }
    }

    d->barView->removeItem(info);
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

    Q_LLONG infoId = info.id();

    // First determine the next images to the current left and right image:
    ImageInfo next_linfo;
    ImageInfo next_rinfo;

    if (!curr_linfo.isNull()) 
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

    if (!curr_rinfo.isNull()) 
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
    // This addresses a crash occuring if the last image is removed
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
            LightTableBarItem *ltItem = d->barView->findItemByInfo(new_linfo);
            LightTableBarItem* next   = dynamic_cast<LightTableBarItem*>(ltItem->next());
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
                new_rinfo = first->info();
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
    if (!new_rinfo.isNull())
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
    if (!d->barView->currentItemImageInfo().isNull())
        slotEditItem(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotEditItem(const ImageInfo &info)
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
    d->previewView->setLeftZoomFactor(1.0);
    d->previewView->setRightZoomFactor(1.0);
}

void LightTableWindow::slotFitToWindow()
{
    d->previewView->fitToWindow();
}

void LightTableWindow::slotToggleSlideShow()
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
    settings.loop                 = group.readEntry("SlideShowLoop", false);
    slideShow(startWithCurrent, settings);
}

void LightTableWindow::slideShow(bool startWithCurrent, SlideShowSettings& settings)
{
    int       i = 0;
    DMetadata meta;
    d->cancelSlideShow = false;

    d->statusProgressBar->progressBarMode(StatusProgressBar::CancelProgressBarMode, 
                                  i18n("Preparing slideshow. Please wait..."));

    ImageInfoList list = d->barView->itemsImageInfoList();

    for (ImageInfoList::const_iterator it = list.constBegin();
         !d->cancelSlideShow && it != list.constEnd() ; ++it)
    {
        SlidePictureInfo pictInfo;
        pictInfo.comment = (*it).comment();

        // Perform optimizations: only read pictures metadata if necessary.
        if (settings.printApertureFocal || settings.printExpoSensitivity || settings.printMakeModel)
        {
            meta.load((*it).fileUrl().path());
            pictInfo.photoInfo = meta.getPhotographInformations();
        }

        // In case of dateTime extraction from metadata failed 
        pictInfo.photoInfo.dateTime = (*it).dateTime();
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
        showNormal();

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

        d->leftSidebar->restore();
        d->rightSidebar->restore();

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

        d->leftSidebar->backup();
        d->rightSidebar->backup();

        showFullScreen();
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
    Setup setup(this, 0);

    if (setup.exec() != QDialog::Accepted)
        return;

    KGlobal::config()->sync();

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
    d->leftZoomBar->setZoomTrackerText(i18n("zoom: %1%",(int)(zoom*100.0)));

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
    if (curr && curr->prev())
        d->barView->setSelected(curr->prev());
}

void LightTableWindow::slotForward()
{
    ThumbBarItem* curr = d->barView->currentItem();
    if (curr && curr->next())
        d->barView->setSelected(curr->next());
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
    QStringList list      = KDcrawIface::DcrawBinary::instance()->supportedCamera();
    QString     dcrawVer  = KDcrawIface::DcrawBinary::instance()->internalVersion();
    QString     KDcrawVer = KDcrawIface::KDcraw::version();
    KMessageBox::informationList(this, 
                                 i18n("<p>Using KDcraw library version %1"
                                      "<p>Using Dcraw program version %2"
                                      "<p>%3 models in the list", 
                                      KDcrawVer, dcrawVer, list.count()),
                                 list, i18n("List of supported RAW camera"));
}

}  // namespace Digikam
