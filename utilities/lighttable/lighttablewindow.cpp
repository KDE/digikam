/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <Q3DockArea>
#include <Q3PtrList>
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

// LibKDcraw includes.

#include <libkdcraw/dcrawbinary.h>

// Local includes.

#include "ddebug.h"
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
#include "lighttablepreview.h"
#include "lighttablewindowprivate.h"
#include "lighttablewindow.h"
#include "lighttablewindow.moc"

namespace Digikam
{

LightTableWindow* LightTableWindow::m_componentData = 0;

LightTableWindow* LightTableWindow::lightTableWindow()
{
    if (!m_componentData)
        new LightTableWindow();

    return m_componentData;
}

bool LightTableWindow::lightTableWindowCreated()
{
    return m_componentData;
}

LightTableWindow::LightTableWindow()
                : KXmlGuiWindow(0)
{
    d = new LightTableWindowPriv;
    m_componentData = this;

    setWindowFlags(Qt::Window);
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
    m_componentData = 0;

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
    if(config->hasKey("Vertical Splitter Sizes"))
        d->vSplitter->setSizes(group.readEntry("Vertical Splitter Sizes", list));

    if(config->hasKey("Horizontal Splitter Sizes"))
        d->hSplitter->setSizes(group.readEntry("Horizontal Splitter Sizes", list));

    d->navigateByPairAction->setChecked(group.readEntry("Navigate By Pair", false));
    slotToggleNavigateByPair();
}

void LightTableWindow::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("LightTable Settings");
    group.writeEntry("Vertical Splitter Sizes", d->vSplitter->sizes());
    group.writeEntry("Horizontal Splitter Sizes", d->hSplitter->sizes());
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
    d->leftSidebar    = new ImagePropertiesSideBarDB(mainW, 
                            "LightTable Left Sidebar", d->hSplitter,
                            Sidebar::DockLeft, true);

    QWidget* centralW = new QWidget(d->hSplitter);
    QVBoxLayout *vlay = new QVBoxLayout(centralW);
    d->vSplitter      = new QSplitter(Qt::Vertical, centralW);
    d->barView        = new LightTableBar(d->vSplitter, Qt::Horizontal, 
                                          AlbumSettings::componentData()->getExifRotate());
    d->previewView    = new LightTableView(d->vSplitter);

    d->rightSidebar   = new ImagePropertiesSideBarDB(mainW, 
                            "LightTable Right Sidebar", d->hSplitter,
                            Sidebar::DockRight, true);

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
    statusBar()->addWidget(d->leftZoomBar, 1);
    d->leftZoomBar->setEnabled(false);

    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignCenter);
    d->statusProgressBar->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(d->statusProgressBar, 100);

    d->rightZoomBar = new StatusZoomBar(statusBar());
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

    d->backwardAction = actionCollection()->addAction(KStandardAction::Back, "lighttable_backward", 
                                                      this, SLOT(slotBackward()));
    d->backwardAction->setEnabled(false);

    d->forwardAction = actionCollection()->addAction(KStandardAction::Forward, "lighttable_forward", 
                                                    this, SLOT(slotForward()));
    d->forwardAction->setEnabled(false);

    d->firstAction = new KAction(KIcon("go-first"), i18n("&First"), this);
    d->firstAction->setShortcut(KStandardShortcut::Home);
    d->firstAction->setEnabled(false);
    connect(d->firstAction, SIGNAL(triggered()), this, SLOT(slotFirst()));
    actionCollection()->addAction("lighttable_first", d->firstAction);

    d->lastAction = new KAction(KIcon("go-last"), i18n("&Last"), this);
    d->lastAction->setShortcut(KStandardShortcut::End);
    d->lastAction->setEnabled(false);
    connect(d->lastAction, SIGNAL(triggered()), this, SLOT(slotLast()));
    actionCollection()->addAction("lighttable_last", d->lastAction);

    d->setItemLeftAction = new KAction(KIcon("arrow-left"), i18n("Show item on left panel"), this);
    d->setItemLeftAction->setShortcut(Qt::CTRL+Qt::Key_L);
    d->setItemLeftAction->setEnabled(false);
    connect(d->setItemLeftAction, SIGNAL(triggered()), this, SLOT(slotSetItemLeft()));
    actionCollection()->addAction("lighttable_setitemleft", d->setItemLeftAction);

    d->setItemRightAction = new KAction(KIcon("arrow-right"), i18n("Show item on right panel"), this);
    d->setItemRightAction->setShortcut(Qt::CTRL+Qt::Key_R);
    d->setItemRightAction->setEnabled(false);
    connect(d->setItemRightAction, SIGNAL(triggered()), this, SLOT(slotSetItemRight()));
    actionCollection()->addAction("lighttable_setitemright", d->setItemRightAction);

    d->editItemAction = new KAction(KIcon("editimage"), i18n("Edit"), this);
    d->editItemAction->setShortcut(Qt::Key_F4);
    d->editItemAction->setEnabled(false);
    connect(d->editItemAction, SIGNAL(triggered()), this, SLOT(slotEditItem()));
    actionCollection()->addAction("lighttable_edititem", d->editItemAction);

    d->removeItemAction = new KAction(KIcon("dialog-close"), i18n("Remove item"), this);
    d->removeItemAction->setShortcut(Qt::CTRL+Qt::Key_K);
    d->removeItemAction->setEnabled(false);
    connect(d->removeItemAction, SIGNAL(triggered()), this, SLOT(slotRemoveItem()));
    actionCollection()->addAction("lighttable_removeitem", d->removeItemAction);

    d->clearListAction = new KAction(KIcon("list-remove"), i18n("Clear all items"), this);
    d->clearListAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_K);
    d->clearListAction->setEnabled(false);
    connect(d->clearListAction, SIGNAL(triggered()), this, SLOT(slotClearItemsList()));
    actionCollection()->addAction("lighttable_clearlist", d->clearListAction);

    d->fileDeleteAction = new KAction(KIcon("edit-trash"), i18n("Move to Trash"), this);
    d->fileDeleteAction->setShortcut(Qt::Key_Delete);
    d->fileDeleteAction->setEnabled(false);
    connect(d->fileDeleteAction, SIGNAL(triggered()), this, SLOT(slotDeleteItem()));
    actionCollection()->addAction("lighttable_filedelete", d->fileDeleteAction);

    actionCollection()->addAction(KStandardAction::Close, "lighttable_close", 
                                  this, SLOT(close()));

    // -- Standard 'View' menu actions ---------------------------------------------

    d->syncPreviewAction = new KToggleAction(KIcon("footprint"), i18n("Synchronize Preview"), this);
    d->syncPreviewAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_Y);
    d->syncPreviewAction->setEnabled(false);
    connect(d->syncPreviewAction, SIGNAL(triggered()), this, SLOT(slotToggleSyncPreview()));
    actionCollection()->addAction("lighttable_syncpreview", d->syncPreviewAction);

    d->navigateByPairAction = new KToggleAction(KIcon("system-run"), i18n("Navigate by Pair"), this);
    d->navigateByPairAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_P);
    d->navigateByPairAction->setEnabled(false);
    connect(d->navigateByPairAction, SIGNAL(triggered()), this, SLOT(slotToggleNavigateByPair()));
    actionCollection()->addAction("lighttable_navigatebypair", d->navigateByPairAction);

    d->zoomPlusAction = actionCollection()->addAction(KStandardAction::ZoomIn, "lighttable_zoomplus", 
                                                      this, SLOT(slotIncreaseZoom()));
    d->zoomPlusAction->setEnabled(false);

    d->zoomMinusAction = actionCollection()->addAction(KStandardAction::ZoomOut, "lighttable_zoomminus", 
                                                       this, SLOT(slotDecreaseZoom()));
    d->zoomMinusAction->setEnabled(false);

    d->zoomTo100percents = new KAction(KIcon("viewmag1"), i18n("Zoom to 1:1"), this);
    d->zoomTo100percents->setShortcut(Qt::ALT+Qt::CTRL+Qt::Key_0);       // NOTE: Photoshop 7 use ALT+CTRL+0
    connect(d->zoomTo100percents, SIGNAL(triggered()), this, SLOT(slotZoomTo100Percents()));
    actionCollection()->addAction("lighttable_zoomto100percents", d->zoomTo100percents);

    d->zoomFitToWindowAction = new KToggleAction(KIcon("zoom-best-fit"), i18n("Fit to &Window"), this);
    d->zoomFitToWindowAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_E); // NOTE: Gimp 2 use CTRL+SHIFT+E.
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), this, SLOT(slotFitToWindow()));
    actionCollection()->addAction("lighttable_zoomfit2window", d->zoomFitToWindowAction);

    d->fullScreenAction = actionCollection()->addAction(KStandardAction::FullScreen,
                          "lighttable_fullscreen", this, SLOT(slotToggleFullScreen()));

    d->slideShowAction = new KAction(KIcon("datashow"), i18n("Slide Show"), this);
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

    // ---------------------------------------------------------------------------------

    createGUI("lighttablewindowui.rc");
}

void LightTableWindow::setupAccelerators()
{
#warning "TODO: kde4 port it";
/*  // TODO: KDE4PORT: use KAction/QAction framework instead KAccel

    d->accelerators = new KAccel(this);

    d->accelerators->insert("Exit fullscreen", i18n("Exit Fullscreen mode"),
                    i18n("Exit from fullscreen viewing mode"),
                    Qt::Key_Escape, this, SLOT(slotEscapePressed()),
                    false, true);

    d->accelerators->insert("Next Image Qt::Key_Space", i18n("Next Image"),
                    i18n("Load Next Image"),
                    Qt::Key_Space, this, SLOT(slotForward()),
                    false, true);

    d->accelerators->insert("Previous Image Qt::Key_Backspace", i18n("Previous Image"),
                    i18n("Load Previous Image"),
                    Qt::Key_Backspace, this, SLOT(slotBackward()),
                    false, true);

    d->accelerators->insert("Next Image Qt::Key_Next", i18n("Next Image"),
                    i18n("Load Next Image"),
                    Qt::Key_Next, this, SLOT(slotForward()),
                    false, true);

    d->accelerators->insert("Previous Image Qt::Key_Prior", i18n("Previous Image"),
                    i18n("Load Previous Image"),
                    Qt::Key_Prior, this, SLOT(slotBackward()),
                    false, true);

    d->accelerators->insert("Zoom Plus Qt::Key_Plus", i18n("Zoom in"),
                    i18n("Zoom in on image"),
                    Qt::Key_Plus, d->previewView, SLOT(slotIncreaseZoom()),
                    false, true);

    d->accelerators->insert("Zoom Plus Qt::Key_Minus", i18n("Zoom out"),
                    i18n("Zoom out of image"),
                    Qt::Key_Minus, d->previewView, SLOT(slotDecreaseZoom()),
                    false, true);
*/
}

void LightTableWindow::slotThumbbarDroppedItems(const ImageInfoList& list)
{
    loadImageInfos(list, ImageInfo());
}

void LightTableWindow::loadImageInfos(const ImageInfoList &list, const ImageInfo &givenImageInfoCurrent)
{
    ImageInfoList l = list;
    ImageInfo imageInfoCurrent = givenImageInfoCurrent;

    if (imageInfoCurrent.isNull())
        imageInfoCurrent = l.first();

    AlbumSettings *settings = AlbumSettings::componentData();

    if (!settings) return;

    QString imagefilter = settings->getImageFileFilter().toLower() +
                          settings->getImageFileFilter().toUpper();

    if (KDcrawIface::DcrawBinary::componentData()->versionIsRight())
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

    LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(imageInfoCurrent));
    if (ltItem) 
        d->barView->setSelectedItem(ltItem);

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
    // With navigate by pair option, only the Feft panel can be selected.
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

void LightTableWindow::slotLeftDroppedItems(const ImageInfoList& list)
{
    ImageInfo info = list.first();
    loadImageInfos(list, info);

    // We will check if first item from list is already stored in thumbbar
    // Note than thumbbar store all ImageInfo reference in memory for preview object.
    LightTableBarItem *item = d->barView->findItemByInfo(info);
    if (item)
        slotSetItemOnLeftPanel(item->info());
}

void LightTableWindow::slotRightDroppedItems(const ImageInfoList& list)
{
    ImageInfo info = list.first();
    loadImageInfos(list, info);

    // We will check if first item from list is already stored in thumbbar
    // Note than thumbbar store all ImageInfo reference in memory for preview object.
    LightTableBarItem *item = d->barView->findItemByInfo(info);
    if (item)
        slotSetItemOnRightPanel(item->info());
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
    PAlbum *palbum = AlbumManager::componentData()->findPAlbum(u.directory());
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
    if (!d->previewView->leftImageInfo().isNull())
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
        settings.exifRotate = AlbumSettings::componentData()->getExifRotate();

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
        setWindowState( windowState() & ~Qt::WindowFullScreen );
        menuBar()->show();
        statusBar()->show();

#warning "TODO: kde4 port it";
/* TODO: KDE4PORT: Check these methods
        leftDock()->show();
        rightDock()->show();
        topDock()->show();
        bottomDock()->show();*/

#warning "TODO: kde4 port it";
/*
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
*/

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

#warning "TODO: kde4 port it";
/* TODO: KDE4PORT: Check these methods
        topDock()->hide();
        leftDock()->hide();
        rightDock()->hide();
        bottomDock()->hide();*/

#warning "TODO: kde4 port it";
/*
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
*/
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
        d->fullScreenAction->activate(QAction::Trigger);
}

void LightTableWindow::showToolBars()
{
#warning "TODO: kde4 port it";
/*
    Q3PtrListIterator<KToolBar> it = toolBarIterator();
    KToolBar* bar;

    for( ; it.current()!=0L ; ++it)
    {
        bar=it.current();

        if (bar->area())
            bar->area()->show();
        else
            bar->show();
    }
*/
}

void LightTableWindow::hideToolBars()
{
#warning "TODO: kde4 port it";
/*
    Q3PtrListIterator<KToolBar> it = toolBarIterator();
    KToolBar* bar;

    for( ; it.current()!=0L ; ++it)
    {
        bar=it.current();

        if (bar->area()) 
            bar->area()->hide();
        else 
            bar->hide();
    }
*/
}

void LightTableWindow::plugActionAccel(KAction* action)
{
    if (!action)
        return;

#warning "TODO: kde4 port it";
/*  // TODO: KDE4PORT: use KAction/QAction framework instead KAccel
    d->accelerators->insert(action->text(),
                    action->text(),
                    action->whatsThis(),
                    action->shortcut(),
                    action,
                    SLOT(activate()));*/
}

void LightTableWindow::unplugActionAccel(KAction* /*action*/)
{
#warning "TODO: kde4 port it";
/*  // TODO: KDE4PORT: use KAction/QAction framework instead KAccel

    d->accelerators->remove(action->text());*/
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

}  // namespace Digikam
