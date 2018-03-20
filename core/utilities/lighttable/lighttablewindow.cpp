/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "lighttablewindow_p.h"

// Qt includes

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>

// Local includes

#include "drawdecoder.h"
#include "digikam_debug.h"
#include "componentsinfo.h"
#include "digikamapp.h"
#include "thememanager.h"
#include "dimg.h"
#include "dio.h"
#include "dmetadata.h"
#include "dfileoperations.h"
#include "metadatasettings.h"
#include "metadataedit.h"
#include "applicationsettings.h"
#include "albummanager.h"
#include "loadingcacheinterface.h"
#include "deletedialog.h"
#include "iccsettings.h"
#include "imagewindow.h"
#include "imagegps.h"
#include "imagedescedittab.h"
#include "presentationmngr.h"
#include "slideshowbuilder.h"
#include "slideshow.h"
#include "setup.h"
#include "syncjob.h"
#include "lighttablepreview.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "coredbchangesets.h"
#include "collectionscanner.h"
#include "scancontroller.h"
#include "tagsactionmngr.h"
#include "thumbbardock.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "dexpanderbox.h"
#include "dbinfoiface.h"
#include "calwizard.h"
#include "expoblendingmanager.h"
#include "mailwizard.h"
#include "advprintwizard.h"
#include "dmediaserverdlg.h"
#include "dbwindow.h"
#include "fbwindow.h"
#include "flickrwindow.h"
#include "gswindow.h"
#include "imageshackwindow.h"
#include "imgurwindow.h"
#include "piwigowindow.h"
#include "rajcewindow.h"
#include "smugwindow.h"
#include "yfwindow.h"

#ifdef HAVE_MEDIAWIKI
#   include "mediawikiwindow.h"
#endif

#ifdef HAVE_VKONTAKTE
#   include "vkwindow.h"
#endif

#ifdef HAVE_KIO
#   include "ftexportwindow.h"
#   include "ftimportwindow.h"
#endif

#ifdef HAVE_MARBLE
#   include "geolocationedit.h"
#endif

#ifdef HAVE_HTMLGALLERY
#   include "htmlwizard.h"
#endif

#ifdef HAVE_PANORAMA
#   include "panomanager.h"
#endif

#ifdef HAVE_MEDIAPLAYER
#   include "vidslidewizard.h"
#endif

namespace Digikam
{

LightTableWindow* LightTableWindow::m_instance = 0;

LightTableWindow* LightTableWindow::lightTableWindow()
{
    if (!m_instance)
    {
        new LightTableWindow();
    }

    return m_instance;
}

bool LightTableWindow::lightTableWindowCreated()
{
    return m_instance;
}

LightTableWindow::LightTableWindow()
    : DXmlGuiWindow(0),
      d(new Private)
{
    setConfigGroupName(QLatin1String("LightTable Settings"));
    setXMLFile(QLatin1String("lighttablewindowui5.rc"));

    m_instance = this;

    setWindowFlags(Qt::Window);
    setCaption(i18n("Light Table"));
    // We don't want to be deleted on close
    setAttribute(Qt::WA_DeleteOnClose, false);
    setFullScreenOptions(FS_LIGHTTABLE);

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupActions();
    setupStatusBar();

    // ------------------------------------------------

    setupConnections();
    slotColorManagementOptionsChanged();

    readSettings();

    d->leftSideBar->populateTags();
    d->rightSideBar->populateTags();

    applySettings();
    setAutoSaveSettings(configGroupName(), true);
}

LightTableWindow::~LightTableWindow()
{
    m_instance = 0;

    delete d->thumbView;
    delete d->rightSideBar;
    delete d->leftSideBar;
    delete d;
}

void LightTableWindow::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    d->hSplitter->restoreState(group, QLatin1String("Horizontal Splitter State"));
    d->barViewDock->setShouldBeVisible(group.readEntry(QLatin1String("Show Thumbbar"), true));
    d->navigateByPairAction->setChecked(group.readEntry(QLatin1String("Navigate By Pair"), false));
    slotToggleNavigateByPair();

    d->leftSideBar->setConfigGroup(KConfigGroup(&group, QLatin1String("Left Sidebar")));
    d->leftSideBar->loadState();
    d->rightSideBar->setConfigGroup(KConfigGroup(&group, QLatin1String("Right Sidebar")));
    d->rightSideBar->loadState();

    readFullScreenSettings(group);
}

void LightTableWindow::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());
    d->hSplitter->saveState(group, QLatin1String("Horizontal Splitter State"));
    group.writeEntry(QLatin1String("Show Thumbbar"),    d->barViewDock->shouldBeVisible());
    group.writeEntry(QLatin1String("Navigate By Pair"), d->navigateByPairAction->isChecked());
    group.writeEntry(QLatin1String("Clear On Close"),   d->clearOnCloseAction->isChecked());

    d->leftSideBar->setConfigGroup(KConfigGroup(&group, QLatin1String("Left Sidebar")));
    d->leftSideBar->saveState();
    d->rightSideBar->setConfigGroup(KConfigGroup(&group, QLatin1String("Right Sidebar")));
    d->rightSideBar->saveState();

    config->sync();
}

void LightTableWindow::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());
    d->autoLoadOnRightPanel   = group.readEntry(QLatin1String("Auto Load Right Panel"), true);
    d->autoSyncPreview        = group.readEntry(QLatin1String("Auto Sync Preview"),     true);
    d->clearOnCloseAction->setChecked(group.readEntry(QLatin1String("Clear On Close"), false));
    slotApplicationSettingsChanged();

    // Restore full screen Mode
    readFullScreenSettings(group);

    // NOTE: Image orientation settings in thumbbar is managed by image model.
    refreshView();
}

void LightTableWindow::refreshView()
{
    d->leftSideBar->refreshTagsView();
    d->rightSideBar->refreshTagsView();
}

void LightTableWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    if (d->clearOnCloseAction->isChecked())
    {
        slotClearItemsList();
    }

    // There is one nasty habit with the thumbnail bar if it is floating: it
    // doesn't close when the parent window does, so it needs to be manually
    // closed. If the light table is opened again, its original state needs to
    // be restored.
    // This only needs to be done when closing a visible window and not when
    // destroying a closed window, since the latter case will always report that
    // the thumbnail bar isn't visible.
    if (isVisible())
    {
        d->barViewDock->hide();
    }

    writeSettings();

    DXmlGuiWindow::closeEvent(e);
    e->accept();
}

void LightTableWindow::showEvent(QShowEvent*)
{
    // Restore the visibility of the thumbbar and start autosaving again.
    d->barViewDock->restoreVisibility();
}

void LightTableWindow::setupUserArea()
{
    QWidget* const mainW    = new QWidget(this);
    d->hSplitter            = new SidebarSplitter(Qt::Horizontal, mainW);
    QHBoxLayout* const hlay = new QHBoxLayout(mainW);

    // The left sidebar
    d->leftSideBar          = new ImagePropertiesSideBarDB(mainW, d->hSplitter, Qt::LeftEdge, true);

    // The central preview is wrapped in a KMainWindow so that the thumbnail
    // bar can float around it.
    KMainWindow* const viewContainer = new KMainWindow(mainW, Qt::Widget);
    d->hSplitter->addWidget(viewContainer);
    d->previewView                   = new LightTableView(viewContainer);
    viewContainer->setCentralWidget(d->previewView);

    // The right sidebar.
    d->rightSideBar = new ImagePropertiesSideBarDB(mainW, d->hSplitter, Qt::RightEdge, true);

    hlay->addWidget(d->leftSideBar);
    hlay->addWidget(d->hSplitter);
    hlay->addWidget(d->rightSideBar);
    hlay->setSpacing(0);
    hlay->setContentsMargins(QMargins());
    hlay->setStretchFactor(d->hSplitter, 10);

    d->hSplitter->setFrameStyle(QFrame::NoFrame);
    d->hSplitter->setFrameShadow(QFrame::Plain);
    d->hSplitter->setFrameShape(QFrame::NoFrame);
    d->hSplitter->setOpaqueResize(false);
    d->hSplitter->setStretchFactor(1, 10);      // set previewview+thumbbar container default size to max.

    // The thumb bar is placed in a detachable/dockable widget.
    d->barViewDock = new ThumbBarDock(viewContainer, Qt::Tool);
    d->barViewDock->setObjectName(QLatin1String("lighttable_thumbbar"));

    d->thumbView   = new LightTableThumbBar(d->barViewDock);

    d->barViewDock->setWidget(d->thumbView);
    viewContainer->addDockWidget(Qt::TopDockWidgetArea, d->barViewDock);
    d->barViewDock->setFloating(false);

    // Restore the previous state. This doesn't emit the proper signals to the
    // dock widget, so it has to be manually reinitialized.
    viewContainer->setAutoSaveSettings(QLatin1String("LightTable Thumbbar"), true);

    connect(d->barViewDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            d->thumbView, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    d->barViewDock->reInitialize();

    setCentralWidget(mainW);
}

void LightTableWindow::setupStatusBar()
{
    d->leftZoomBar = new DZoomBar(statusBar());
    d->leftZoomBar->setZoomToFitAction(d->leftZoomFitToWindowAction);
    d->leftZoomBar->setZoomTo100Action(d->leftZoomTo100percents);
    d->leftZoomBar->setZoomPlusAction(d->leftZoomPlusAction);
    d->leftZoomBar->setZoomMinusAction(d->leftZoomMinusAction);
    d->leftZoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    d->leftZoomBar->setEnabled(false);
    statusBar()->addWidget(d->leftZoomBar, 1);

    d->leftFileName = new DAdjustableLabel(statusBar());
    d->leftFileName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusBar()->addWidget(d->leftFileName, 10);

    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(d->statusProgressBar, 10);

    d->rightFileName = new DAdjustableLabel(statusBar());
    d->rightFileName->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    statusBar()->addWidget(d->rightFileName, 10);

    d->rightZoomBar = new DZoomBar(statusBar());
    d->rightZoomBar->setZoomToFitAction(d->rightZoomFitToWindowAction);
    d->rightZoomBar->setZoomTo100Action(d->rightZoomTo100percents);
    d->rightZoomBar->setZoomPlusAction(d->rightZoomPlusAction);
    d->rightZoomBar->setZoomMinusAction(d->rightZoomMinusAction);
    d->rightZoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    d->rightZoomBar->setEnabled(false);
    statusBar()->addWidget(d->rightZoomBar, 1);
}

void LightTableWindow::setupConnections()
{
    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotApplicationSettingsChanged()));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(IccSettings::instance(), SIGNAL(settingsChanged()),
            this, SLOT(slotColorManagementOptionsChanged()));

    // Thumbs bar connections ---------------------------------------

    connect(d->thumbView, SIGNAL(signalSetItemOnLeftPanel(ImageInfo)),
            this, SLOT(slotSetItemOnLeftPanel(ImageInfo)));

    connect(d->thumbView, SIGNAL(signalSetItemOnRightPanel(ImageInfo)),
            this, SLOT(slotSetItemOnRightPanel(ImageInfo)));

    connect(d->thumbView, SIGNAL(signalRemoveItem(ImageInfo)),
            this, SLOT(slotRemoveItem(ImageInfo)));

    connect(d->thumbView, SIGNAL(signalEditItem(ImageInfo)),
            this, SLOT(slotEditItem(ImageInfo)));

    connect(d->thumbView, SIGNAL(signalClearAll()),
            this, SLOT(slotClearItemsList()));

    connect(d->thumbView, SIGNAL(signalDroppedItems(QList<ImageInfo>)),
            this, SLOT(slotThumbbarDroppedItems(QList<ImageInfo>)));

    connect(d->thumbView, SIGNAL(currentChanged(ImageInfo)),
            this, SLOT(slotItemSelected(ImageInfo)));

    connect(d->thumbView, SIGNAL(signalContentChanged()),
            this, SLOT(slotRefreshStatusBar()));

    // Zoom bars connections -----------------------------------------

    connect(d->leftZoomBar, SIGNAL(signalZoomSliderChanged(int)),
            d->previewView, SLOT(slotLeftZoomSliderChanged(int)));

    connect(d->leftZoomBar, SIGNAL(signalZoomValueEdited(double)),
            d->previewView, SLOT(setLeftZoomFactor(double)));

    connect(d->rightZoomBar, SIGNAL(signalZoomSliderChanged(int)),
            d->previewView, SLOT(slotRightZoomSliderChanged(int)));

    connect(d->rightZoomBar, SIGNAL(signalZoomValueEdited(double)),
            d->previewView, SLOT(setRightZoomFactor(double)));

    // View connections ---------------------------------------------

    connect(d->previewView, SIGNAL(signalLeftPopupTagsView()),
            d->leftSideBar, SLOT(slotPopupTagsView()));

    connect(d->previewView, SIGNAL(signalRightPopupTagsView()),
            d->rightSideBar, SLOT(slotPopupTagsView()));

    connect(d->previewView, SIGNAL(signalLeftZoomFactorChanged(double)),
            this, SLOT(slotLeftZoomFactorChanged(double)));

    connect(d->previewView, SIGNAL(signalRightZoomFactorChanged(double)),
            this, SLOT(slotRightZoomFactorChanged(double)));

    connect(d->previewView, SIGNAL(signalEditItem(ImageInfo)),
            this, SLOT(slotEditItem(ImageInfo)));

    connect(d->previewView, SIGNAL(signalDeleteItem(ImageInfo)),
            this, SLOT(slotDeleteItem(ImageInfo)));

    connect(d->previewView, SIGNAL(signalSlideShow()),
            this, SLOT(slotSlideShowAll()));

    connect(d->previewView, SIGNAL(signalLeftSlideShowCurrent()),
            this, SLOT(slotLeftSlideShowManualFromCurrent()));

    connect(d->previewView, SIGNAL(signalRightSlideShowCurrent()),
            this, SLOT(slotRightSlideShowManualFromCurrent()));

    connect(d->previewView, SIGNAL(signalLeftDroppedItems(ImageInfoList)),
            this, SLOT(slotLeftDroppedItems(ImageInfoList)));

    connect(d->previewView, SIGNAL(signalRightDroppedItems(ImageInfoList)),
            this, SLOT(slotRightDroppedItems(ImageInfoList)));

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

    LoadingCacheInterface::connectToSignalFileChanged(this, SLOT(slotFileChanged(QString)));
}

void LightTableWindow::setupActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    KActionCollection* const ac =   actionCollection();

    d->backwardAction = buildStdAction(StdBackAction, this, SLOT(slotBackward()), this);
    ac->addAction(QLatin1String("lighttable_backward"), d->backwardAction);
    ac->setDefaultShortcuts(d->backwardAction, QList<QKeySequence>() << Qt::Key_PageUp << Qt::Key_Backspace);

    d->forwardAction = buildStdAction(StdForwardAction, this, SLOT(slotForward()), this);
    ac->addAction(QLatin1String("lighttable_forward"), d->forwardAction);
    ac->setDefaultShortcuts(d->forwardAction, QList<QKeySequence>() << Qt::Key_PageDown << Qt::Key_Space);
    d->forwardAction->setEnabled(false);

    d->firstAction = new QAction(QIcon::fromTheme(QLatin1String("go-first")), i18n("&First"), this);
    d->firstAction->setEnabled(false);
    connect(d->firstAction, SIGNAL(triggered()), this, SLOT(slotFirst()));
    ac->addAction(QLatin1String("lighttable_first"), d->firstAction);
    ac->setDefaultShortcuts(d->firstAction, QList<QKeySequence>() << Qt::CTRL + Qt::Key_Home);

    d->lastAction = new QAction(QIcon::fromTheme(QLatin1String("go-last")), i18n("&Last"), this);
    d->lastAction->setEnabled(false);
    connect(d->lastAction, SIGNAL(triggered()), this, SLOT(slotLast()));
    ac->addAction(QLatin1String("lighttable_last"), d->lastAction);
    ac->setDefaultShortcuts(d->lastAction, QList<QKeySequence>() << Qt::CTRL + Qt::Key_End);

    d->setItemLeftAction = new QAction(QIcon::fromTheme(QLatin1String("go-previous")), i18n("On left"), this);
    d->setItemLeftAction->setEnabled(false);
    d->setItemLeftAction->setWhatsThis(i18n("Show item on left panel"));
    connect(d->setItemLeftAction, SIGNAL(triggered()), this, SLOT(slotSetItemLeft()));
    ac->addAction(QLatin1String("lighttable_setitemleft"), d->setItemLeftAction);
    ac->setDefaultShortcut(d->setItemLeftAction, Qt::CTRL + Qt::Key_L);

    d->setItemRightAction = new QAction(QIcon::fromTheme(QLatin1String("go-next")), i18n("On right"), this);
    d->setItemRightAction->setEnabled(false);
    d->setItemRightAction->setWhatsThis(i18n("Show item on right panel"));
    connect(d->setItemRightAction, SIGNAL(triggered()), this, SLOT(slotSetItemRight()));
    ac->addAction(QLatin1String("lighttable_setitemright"), d->setItemRightAction);
    ac->setDefaultShortcut(d->setItemRightAction, Qt::CTRL + Qt::Key_R);

    d->editItemAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Edit"), this);
    d->editItemAction->setEnabled(false);
    connect(d->editItemAction, SIGNAL(triggered()), this, SLOT(slotEditItem()));
    ac->addAction(QLatin1String("lighttable_edititem"), d->editItemAction);
    ac->setDefaultShortcut(d->editItemAction, Qt::Key_F4);

    QAction* const openWithAction = new QAction(QIcon::fromTheme(QLatin1String("preferences-desktop-filetype-association")), i18n("Open With Default Application"), this);
    openWithAction->setWhatsThis(i18n("Open the item with default assigned application."));
    connect(openWithAction, SIGNAL(triggered()), this, SLOT(slotFileWithDefaultApplication()));
    ac->addAction(QLatin1String("open_with_default_application"), openWithAction);
    ac->setDefaultShortcut(openWithAction, Qt::META + Qt::Key_F4);

    d->removeItemAction = new QAction(QIcon::fromTheme(QLatin1String("list-remove")), i18n("Remove item from LightTable"), this);
    d->removeItemAction->setEnabled(false);
    connect(d->removeItemAction, SIGNAL(triggered()), this, SLOT(slotRemoveItem()));
    ac->addAction(QLatin1String("lighttable_removeitem"), d->removeItemAction);
    ac->setDefaultShortcut(d->removeItemAction, Qt::CTRL + Qt::Key_K);

    d->clearListAction = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")), i18n("Remove all items from LightTable"), this);
    d->clearListAction->setEnabled(false);
    connect(d->clearListAction, SIGNAL(triggered()), this, SLOT(slotClearItemsList()));
    ac->addAction(QLatin1String("lighttable_clearlist"), d->clearListAction);
    ac->setDefaultShortcut(d->clearListAction, Qt::CTRL + Qt::SHIFT + Qt::Key_K);

    d->fileDeleteAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")), i18nc("Non-pluralized", "Move to Trash"), this);
    d->fileDeleteAction->setEnabled(false);
    connect(d->fileDeleteAction, SIGNAL(triggered()), this, SLOT(slotDeleteItem()));
    ac->addAction(QLatin1String("lighttable_filedelete"), d->fileDeleteAction);
    ac->setDefaultShortcut(d->fileDeleteAction, Qt::Key_Delete);

    d->fileDeleteFinalAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete immediately"), this);
    d->fileDeleteFinalAction->setEnabled(false);
    connect(d->fileDeleteFinalAction, SIGNAL(triggered()), this, SLOT(slotDeleteFinalItem()));
    ac->addAction(QLatin1String("lighttable_filefinaldelete"), d->fileDeleteFinalAction);
    ac->setDefaultShortcut(d->fileDeleteFinalAction, Qt::SHIFT + Qt::Key_Delete);

    QAction* const closeAction = buildStdAction(StdCloseAction, this, SLOT(close()), this);
    ac->addAction(QLatin1String("lighttable_close"), closeAction);

    // -- Standard 'View' menu actions ---------------------------------------------

    d->syncPreviewAction = new QAction(QIcon::fromTheme(QLatin1String("view-split-left-right")), i18n("Synchronize"), this);
    d->syncPreviewAction->setEnabled(false);
    d->syncPreviewAction->setCheckable(true);
    d->syncPreviewAction->setWhatsThis(i18n("Synchronize preview from left and right panels"));
    connect(d->syncPreviewAction, SIGNAL(triggered()), this, SLOT(slotToggleSyncPreview()));
    ac->addAction(QLatin1String("lighttable_syncpreview"), d->syncPreviewAction);
    ac->setDefaultShortcut(d->syncPreviewAction, Qt::CTRL + Qt::SHIFT + Qt::Key_Y);

    d->navigateByPairAction = new QAction(QIcon::fromTheme(QLatin1String("system-run")), i18n("By Pair"), this);
    d->navigateByPairAction->setEnabled(false);
    d->navigateByPairAction->setCheckable(true);
    d->navigateByPairAction->setWhatsThis(i18n("Navigate by pairs with all items"));
    connect(d->navigateByPairAction, SIGNAL(triggered()), this, SLOT(slotToggleNavigateByPair()));
    ac->addAction(QLatin1String("lighttable_navigatebypair"), d->navigateByPairAction);
    ac->setDefaultShortcut(d->navigateByPairAction, Qt::CTRL + Qt::SHIFT + Qt::Key_P);

    d->clearOnCloseAction = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")), i18n("Clear On Close"), this);
    d->clearOnCloseAction->setEnabled(true);
    d->clearOnCloseAction->setCheckable(true);
    d->clearOnCloseAction->setToolTip(i18n("Clear light table when it is closed"));
    d->clearOnCloseAction->setWhatsThis(i18n("Remove all images from the light table when it is closed"));
    ac->addAction(QLatin1String("lighttable_clearonclose"), d->clearOnCloseAction);
    ac->setDefaultShortcut(d->clearOnCloseAction, Qt::CTRL + Qt::SHIFT + Qt::Key_C);

    d->showBarAction = d->barViewDock->getToggleAction(this);
    ac->addAction(QLatin1String("lighttable_showthumbbar"), d->showBarAction);
    ac->setDefaultShortcut(d->showBarAction, Qt::CTRL + Qt::Key_T);

    createFullScreenAction(QLatin1String("lighttable_fullscreen"));
    createSidebarActions();

    d->slideShowAction = new QAction(QIcon::fromTheme(QLatin1String("view-presentation")), i18n("Slideshow"), this);
    connect(d->slideShowAction, SIGNAL(triggered()), this, SLOT(slotSlideShowAll()));
    ac->addAction(QLatin1String("lighttable_slideshow"), d->slideShowAction);
    ac->setDefaultShortcut(d->slideShowAction, Qt::Key_F9);

    createPresentationAction();

    // -- Standard 'Tools' menu actions ------------------------

    createMetadataEditAction();
    createGeolocationEditAction();
    createHtmlGalleryAction();
    createPanoramaAction();
    createExpoBlendingAction();
    createCalendarAction();
    createVideoSlideshowAction();
    createSendByMailAction();
    createPrintCreatorAction();
    createMediaServerAction();
    createExportActions();
    createImportActions();

    // Left Panel Zoom Actions

    d->leftZoomPlusAction  = buildStdAction(StdZoomInAction, d->previewView, SLOT(slotIncreaseLeftZoom()), this);
    d->leftZoomPlusAction->setEnabled(false);
    QKeySequence leftKeysPlus(d->leftZoomPlusAction->shortcut()[0], Qt::Key_Plus);
    ac->addAction(QLatin1String("lighttable_zoomplus_left"), d->leftZoomPlusAction);
    ac->setDefaultShortcut(d->leftZoomPlusAction, leftKeysPlus);

    d->leftZoomMinusAction  = buildStdAction(StdZoomOutAction, d->previewView, SLOT(slotDecreaseLeftZoom()), this);
    d->leftZoomMinusAction->setEnabled(false);
    QKeySequence leftKeysMinus(d->leftZoomMinusAction->shortcut()[0], Qt::Key_Minus);
    ac->addAction(QLatin1String("lighttable_zoomminus_left"), d->leftZoomMinusAction);
    ac->setDefaultShortcut(d->leftZoomMinusAction, leftKeysMinus);

    d->leftZoomTo100percents = new QAction(QIcon::fromTheme(QLatin1String("zoom-original")), i18n("Zoom to 100%"), this);
    connect(d->leftZoomTo100percents, SIGNAL(triggered()), d->previewView, SLOT(slotLeftZoomTo100()));
    ac->addAction(QLatin1String("lighttable_zoomto100percents_left"), d->leftZoomTo100percents);
    ac->setDefaultShortcut(d->leftZoomTo100percents, Qt::CTRL + Qt::Key_Period);

    d->leftZoomFitToWindowAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")), i18n("Fit to &Window"), this);
    connect(d->leftZoomFitToWindowAction, SIGNAL(triggered()), d->previewView, SLOT(slotLeftFitToWindow()));
    ac->addAction(QLatin1String("lighttable_zoomfit2window_left"), d->leftZoomFitToWindowAction);
    ac->setDefaultShortcut(d->leftZoomFitToWindowAction, Qt::ALT + Qt::CTRL + Qt::Key_E);

    // Right Panel Zoom Actions

    d->rightZoomPlusAction  = buildStdAction(StdZoomInAction, d->previewView, SLOT(slotIncreaseRightZoom()), this);
    d->rightZoomPlusAction->setEnabled(false);
    QKeySequence rightKeysPlus(d->rightZoomPlusAction->shortcut()[0], Qt::SHIFT + Qt::CTRL + Qt::Key_Plus, Qt::SHIFT + Qt::Key_Plus);
    ac->addAction(QLatin1String("lighttable_zoomplus_right"), d->rightZoomPlusAction);
    ac->setDefaultShortcut(d->rightZoomPlusAction, rightKeysPlus);

    d->rightZoomMinusAction  = buildStdAction(StdZoomOutAction, d->previewView, SLOT(slotDecreaseRightZoom()), this);
    d->rightZoomMinusAction->setEnabled(false);
    QKeySequence rightKeysMinus(d->rightZoomMinusAction->shortcut()[0], Qt::SHIFT + Qt::CTRL + Qt::Key_Minus, Qt::SHIFT + Qt::Key_Minus);
    ac->addAction(QLatin1String("lighttable_zoomminus_right"), d->rightZoomMinusAction);
    ac->setDefaultShortcut(d->rightZoomMinusAction, rightKeysMinus);

    d->rightZoomTo100percents = new QAction(QIcon::fromTheme(QLatin1String("zoom-original")), i18n("Zoom to 100%"), this);
    connect(d->rightZoomTo100percents, SIGNAL(triggered()), d->previewView, SLOT(slotRightZoomTo100()));
    ac->addAction(QLatin1String("lighttable_zoomto100percents_right"), d->rightZoomTo100percents);
    ac->setDefaultShortcut(d->rightZoomTo100percents, Qt::SHIFT + Qt::CTRL + Qt::Key_Period);

    d->rightZoomFitToWindowAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")), i18n("Fit to &Window"), this);
    connect(d->rightZoomFitToWindowAction, SIGNAL(triggered()), d->previewView, SLOT(slotRightFitToWindow()));
    ac->addAction(QLatin1String("lighttable_zoomfit2window_right"), d->rightZoomFitToWindowAction);
    ac->setDefaultShortcut(d->rightZoomFitToWindowAction, Qt::SHIFT + Qt::CTRL + Qt::Key_E);

    // -----------------------------------------------------------

    d->viewCMViewAction = new QAction(QIcon::fromTheme(QLatin1String("video-display")), i18n("Color-Managed View"), this);
    d->viewCMViewAction->setCheckable(true);
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    ac->addAction(QLatin1String("color_managed_view"), d->viewCMViewAction);
    ac->setDefaultShortcut(d->viewCMViewAction, Qt::Key_F12);

    // -----------------------------------------------------------------------------

    ThemeManager::instance()->registerThemeActions(this);

    // Standard 'Help' menu actions
    createHelpActions();

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // Standard 'Configure' menu actions
    createSettingsActions();

    // -- Keyboard-only actions ----------------------------------------------------

    d->addPageUpDownActions(this, this);

    QAction* const altBackwardAction = new QAction(i18n("Previous Image"), this);
    ac->addAction(QLatin1String("lighttable_backward_shift_space"), altBackwardAction);
    ac->setDefaultShortcut(altBackwardAction, Qt::SHIFT + Qt::Key_Space);
    connect(altBackwardAction, SIGNAL(triggered()), this, SLOT(slotBackward()));

    // Labels shortcuts must be registered here to be saved in XML GUI files if user customize it.
    TagsActionMngr::defaultManager()->registerLabelsActions(ac);

    QAction* const editTitlesRight = new QAction(i18n("Edit Titles on the Right"), this);
    ac->addAction(QLatin1String("edit_titles_right"), editTitlesRight);
    ac->setDefaultShortcut(editTitlesRight, Qt::META + Qt::Key_T);
    connect(editTitlesRight, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateTitles()));

    QAction* const editCommentsRight = new QAction(i18n("Edit Comments on the Right"), this);
    ac->addAction(QLatin1String("edit_comments_right"), editCommentsRight);
    ac->setDefaultShortcut(editCommentsRight, Qt::META + Qt::Key_C);
    connect(editCommentsRight, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateComments()));

    QAction* const editTitlesLeft = new QAction(i18n("Edit Titles on the Left"), this);
    ac->addAction(QLatin1String("edit_titles_left"), editTitlesLeft);
    ac->setDefaultShortcut(editTitlesLeft, Qt::SHIFT + Qt::META + Qt::Key_T);
    connect(editTitlesLeft, SIGNAL(triggered()), this, SLOT(slotLeftSideBarActivateTitles()));

    QAction* const editCommentsLeft = new QAction(i18n("Edit Comments on the Left"), this);
    ac->addAction(QLatin1String("edit_comments_left"), editCommentsLeft);
    ac->setDefaultShortcut(editCommentsLeft, Qt::SHIFT + Qt::META + Qt::Key_C);
    connect(editCommentsLeft, SIGNAL(triggered()), this, SLOT(slotLeftSideBarActivateComments()));

    QAction* const assignedTagsRight = new QAction(i18n("Show Assigned Tags on the Right"), this);
    ac->addAction(QLatin1String("assigned _tags_right"), assignedTagsRight);
    ac->setDefaultShortcut(assignedTagsRight, Qt::META + Qt::Key_A);
    connect(assignedTagsRight, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateAssignedTags()));

    QAction* const assignedTagsLeft = new QAction(i18n("Show Assigned Tags on the Left"), this);
    ac->addAction(QLatin1String("assigned _tags_left"), assignedTagsLeft);
    ac->setDefaultShortcut(assignedTagsLeft, Qt::SHIFT + Qt::META + Qt::Key_A);
    connect(assignedTagsLeft, SIGNAL(triggered()), this, SLOT(slotLeftSideBarActivateAssignedTags()));

    // ---------------------------------------------------------------------------------

    createGUI(xmlFile());
    cleanupActions();

    showMenuBarAction()->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080
}

// Deal with items dropped onto the thumbbar (e.g. from the Album view)
void LightTableWindow::slotThumbbarDroppedItems(const QList<ImageInfo>& list)
{
    // Setting the third parameter of loadImageInfos to true
    // means that the images are added to the presently available images.
    loadImageInfos(ImageInfoList() << list, ImageInfo(), true);
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
                                      const ImageInfo& givenImageInfoCurrent,
                                      bool  addTo)
{
    // Clear all items before adding new images to the light table.
    qCDebug(DIGIKAM_GENERAL_LOG) << "Clearing LT" << (!addTo);

    if (!addTo)
    {
        slotClearItemsList();
    }

    ImageInfoList l            = list;
    ImageInfo imageInfoCurrent = givenImageInfoCurrent;

    if (imageInfoCurrent.isNull() && !l.isEmpty())
    {
        imageInfoCurrent = l.first();
    }

    d->thumbView->setItems(l);

    QModelIndex index = d->thumbView->findItemByInfo(imageInfoCurrent);

    if (index.isValid())
    {
        d->thumbView->setCurrentIndex(index);
    }
    else
    {
        d->thumbView->setCurrentWhenAvailable(imageInfoCurrent.id());
    }
}

bool LightTableWindow::isEmpty() const
{
    return (d->thumbView->countItems() == 0);
}

void LightTableWindow::slotRefreshStatusBar()
{
    d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                             i18np("%1 item on Light Table",
                                                   "%1 items on Light Table",
                                             d->thumbView->countItems()));
}

void LightTableWindow::slotFileChanged(const QString& path)
{
    QUrl url = QUrl::fromLocalFile(path);
    // NOTE: Thumbbar handle change through ImageCategorizedView

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
    if (d->navigateByPairAction->isChecked())
    {
        return;
    }

    d->thumbView->setCurrentInfo(d->previewView->leftImageInfo());
}

void LightTableWindow::slotRightPanelLeftButtonClicked()
{
    // With navigate by pair option, only the left panel can be selected.
    if (d->navigateByPairAction->isChecked())
    {
        return;
    }

    d->thumbView->setCurrentInfo(d->previewView->rightImageInfo());
}

void LightTableWindow::slotLeftPreviewLoaded(bool b)
{
    d->leftZoomBar->setEnabled(b);
    d->leftFileName->setAdjustedText();

    if (b)
    {
        d->leftFileName->setAdjustedText(d->previewView->leftImageInfo().name());
        d->previewView->checkForSelection(d->thumbView->currentInfo());
        d->thumbView->setOnLeftPanel(d->previewView->leftImageInfo());

        QModelIndex index = d->thumbView->findItemByInfo(d->previewView->leftImageInfo());

        if (d->navigateByPairAction->isChecked() && index.isValid())
        {
            QModelIndex next = d->thumbView->nextIndex(index);

            if (next.isValid())
            {
                d->thumbView->setOnRightPanel(d->thumbView->findItemByIndex(next));
                slotSetItemOnRightPanel(d->thumbView->findItemByIndex(next));
            }
            else
            {
                QModelIndex first = d->thumbView->firstIndex();
                slotSetItemOnRightPanel(first.isValid() ? d->thumbView->findItemByIndex(first) : ImageInfo());
            }
        }
    }
}

void LightTableWindow::slotRightPreviewLoaded(bool b)
{
    d->rightZoomBar->setEnabled(b);
    d->rightFileName->setAdjustedText();

    if (b)
    {
        d->rightFileName->setAdjustedText(d->previewView->rightImageInfo().name());
        d->previewView->checkForSelection(d->thumbView->currentInfo());
        d->thumbView->setOnRightPanel(d->previewView->rightImageInfo());

        QModelIndex index = d->thumbView->findItemByInfo(d->previewView->rightImageInfo());

        if (index.isValid())
        {
            d->thumbView->setOnRightPanel(d->thumbView->findItemByIndex(index));
        }
    }
}

void LightTableWindow::slotItemSelected(const ImageInfo& info)
{
    bool hasInfo = !info.isNull();

    d->setItemLeftAction->setEnabled(hasInfo);
    d->setItemRightAction->setEnabled(hasInfo);
    d->editItemAction->setEnabled(hasInfo);
    d->removeItemAction->setEnabled(hasInfo);
    d->clearListAction->setEnabled(hasInfo);
    d->fileDeleteAction->setEnabled(hasInfo);
    d->fileDeleteFinalAction->setEnabled(hasInfo);
    d->backwardAction->setEnabled(hasInfo);
    d->forwardAction->setEnabled(hasInfo);
    d->firstAction->setEnabled(hasInfo);
    d->lastAction->setEnabled(hasInfo);
    d->syncPreviewAction->setEnabled(hasInfo);
    d->navigateByPairAction->setEnabled(hasInfo);
    d->slideShowAction->setEnabled(hasInfo);

    if (hasInfo)
    {
        QModelIndex curr = d->thumbView->findItemByInfo(info);

        if (curr.isValid())
        {
            if (!d->thumbView->previousIndex(curr).isValid())
            {
                d->firstAction->setEnabled(false);
            }

            if (!d->thumbView->nextIndex(curr).isValid())
            {
                d->lastAction->setEnabled(false);
            }

            if (d->navigateByPairAction->isChecked())
            {
                d->setItemLeftAction->setEnabled(false);
                d->setItemRightAction->setEnabled(false);

                d->thumbView->setOnLeftPanel(info);
                slotSetItemOnLeftPanel(info);
            }
            else if (d->autoLoadOnRightPanel && !d->thumbView->isOnLeftPanel(info))
            {
                d->thumbView->setOnRightPanel(info);
                slotSetItemOnRightPanel(info);
            }
        }
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
    QModelIndex index = d->thumbView->findItemByInfo(info);

    if (index.isValid())
    {
        slotSetItemOnLeftPanel(info);
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
    QModelIndex index = d->thumbView->findItemByInfo(info);

    if (index.isValid())
    {
        slotSetItemOnRightPanel(info);
        // Make this item the current one.
        d->thumbView->setCurrentInfo(info);
    }
}

// Set the images for the left and right panel.
void LightTableWindow::setLeftRightItems(const ImageInfoList& list, bool addTo)
{
    ImageInfoList l = list;

    if (l.count() == 0)
    {
        return;
    }

    ImageInfo info    = l.first();
    QModelIndex index = d->thumbView->findItemByInfo(info);

    if (l.count() == 1 && !addTo)
    {
        // Just one item; this is used for the left panel.
        d->thumbView->setOnLeftPanel(info);
        slotSetItemOnLeftPanel(info);
        d->thumbView->setCurrentInfo(info);
        return;
    }

    if (index.isValid())
    {
        // The first item is used for the left panel.
        if (!addTo)
        {
            d->thumbView->setOnLeftPanel(info);
            slotSetItemOnLeftPanel(info);
        }

        // The subsequent item is used for the right panel.
        QModelIndex next = d->thumbView->nextIndex(index);

        if (next.isValid() && !addTo)
        {
            ImageInfo nextInf = d->thumbView->findItemByIndex(next);
            d->thumbView->setOnRightPanel(nextInf);
            slotSetItemOnRightPanel(nextInf);

            if (!d->navigateByPairAction->isChecked())
            {
                d->thumbView->setCurrentInfo(nextInf);
            }
        }

        // If navigate by pairs is active, the left panel item is selected.
        // (Fixes parts of bug #150296)
        if (d->navigateByPairAction->isChecked())
        {
            d->thumbView->setCurrentInfo(info);
        }
    }
}

void LightTableWindow::slotSetItemLeft()
{
    if (!d->thumbView->currentInfo().isNull())
    {
        slotSetItemOnLeftPanel(d->thumbView->currentInfo());
    }
}

void LightTableWindow::slotSetItemRight()
{
    if (!d->thumbView->currentInfo().isNull())
    {
        slotSetItemOnRightPanel(d->thumbView->currentInfo());
    }
}

void LightTableWindow::slotSetItemOnLeftPanel(const ImageInfo& info)
{
    d->previewView->setLeftImageInfo(info);

    if (!info.isNull())
    {
        d->leftSideBar->itemChanged(info);
    }
    else
    {
        d->leftSideBar->slotNoCurrentItem();
    }
}

void LightTableWindow::slotSetItemOnRightPanel(const ImageInfo& info)
{
    d->previewView->setRightImageInfo(info);

    if (!info.isNull())
    {
        d->rightSideBar->itemChanged(info);
    }
    else
    {
        d->rightSideBar->slotNoCurrentItem();
    }
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

    d->thumbView->clear();
}

void LightTableWindow::slotDeleteItem()
{
    deleteItem(false);
}

void LightTableWindow::slotDeleteItem(const ImageInfo& info)
{
    deleteItem(info, false);
}

void LightTableWindow::slotDeleteFinalItem()
{
    deleteItem(true);
}

void LightTableWindow::slotDeleteFinalItem(const ImageInfo& info)
{
    deleteItem(info, true);
}

void LightTableWindow::deleteItem(bool permanently)
{
    if (!d->thumbView->currentInfo().isNull())
    {
        deleteItem(d->thumbView->currentInfo(), permanently);
    }
}

void LightTableWindow::deleteItem(const ImageInfo& info, bool permanently)
{
    QUrl u               = info.fileUrl();
    PAlbum* const palbum = AlbumManager::instance()->findPAlbum(u.adjusted(QUrl::RemoveFilename));

    if (!palbum)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Item to delete: " << u;

    bool useTrash;
    bool preselectDeletePermanently = permanently;

    DeleteDialog dialog(this);

    QList<QUrl> urlList;
    urlList.append(u);

    if (!dialog.confirmDeleteList(urlList, DeleteDialogMode::Files, preselectDeletePermanently ?
                                  DeleteDialogMode::NoChoiceDeletePermanently : DeleteDialogMode::NoChoiceTrash))
    {
        return;
    }

    useTrash = !dialog.shouldDelete();

    DIO::del(info, useTrash);
}

void LightTableWindow::slotRemoveItem()
{
    if (!d->thumbView->currentInfo().isNull())
    {
        slotRemoveItem(d->thumbView->currentInfo());
    }
}

void LightTableWindow::slotRemoveItem(const ImageInfo& info)
{
/*
    if (!d->previewView->leftImageInfo().isNull())
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

    d->thumbView->removeItemByInfo(info);
    d->thumbView->setSelected(d->thumbView->currentItem());
*/

    // When either the image from the left or right panel is removed,
    // there are various situations to account for.
    // To describe them, 4 images A B C D are used
    // and the subscript _L and _ R  mark the currently
    // active item on the left and right panel

    ImageInfo new_linfo;
    ImageInfo new_rinfo;
    bool leftPanelActive = false;
    ImageInfo curr_linfo = d->previewView->leftImageInfo();
    ImageInfo curr_rinfo = d->previewView->rightImageInfo();
    qint64 infoId        = info.id();

    // First determine the next images to the current left and right image:
    ImageInfo next_linfo;
    ImageInfo next_rinfo;

    if (!curr_linfo.isNull())
    {
        QModelIndex index = d->thumbView->findItemByInfo(curr_linfo);

        if (index.isValid())
        {
            QModelIndex next = d->thumbView->nextIndex(index);

            if (next.isValid())
            {
                next_linfo = d->thumbView->findItemByIndex(next);
            }
        }
    }

    if (!curr_rinfo.isNull())
    {
        QModelIndex index = d->thumbView->findItemByInfo(curr_rinfo);

        if (index.isValid())
        {
            QModelIndex next = d->thumbView->nextIndex(index);

            if (next.isValid())
            {
                next_rinfo = d->thumbView->findItemByIndex(next);
            }
        }
    }

    d->thumbView->removeItemByInfo(info);

    // Make sure that next_linfo and next_rinfo are still available:
    if (!d->thumbView->findItemByInfo(next_linfo).isValid())
    {
        next_linfo = ImageInfo();
    }

    if (!d->thumbView->findItemByInfo(next_rinfo).isValid())
    {
        next_rinfo = ImageInfo();
    }

    // removal of the left panel item?
    if (!curr_linfo.isNull())
    {
        if (curr_linfo.id() == infoId)
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

                    // set the right panel active, but not in pair mode
                    if (!d->navigateByPairAction->isChecked())
                    {
                        leftPanelActive = false;
                    }
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
        if (d->thumbView->countItems() > 0)
        {
            QModelIndex first = d->thumbView->firstIndex();
            new_linfo = d->thumbView->findItemByIndex(first);
        }
    }

    // Make sure that new_linfo and new_rinfo exist.
    // This addresses a crash occurring if the last image is removed
    // in the navigate by pairs mode.
    if (!d->thumbView->findItemByInfo(new_linfo).isValid())
    {
        new_linfo = ImageInfo();
    }

    if (!d->thumbView->findItemByInfo(new_rinfo).isValid())
    {
        new_rinfo = ImageInfo();
    }

    // no right item defined?
    if (new_rinfo.isNull())
    {
        // If there are at least two items, we can find reasonable right image.
        if (d->thumbView->countItems() > 1)
        {
            // See if there is an item next to the left one:
            QModelIndex index = d->thumbView->findItemByInfo(new_linfo);
            QModelIndex next;

            if (index.isValid())
            {
                next = d->thumbView->nextIndex(index);
            }

            if (next.isValid())
            {
                new_rinfo = d->thumbView->findItemByIndex(next);
            }
            else
            {
                // If there is no item to the right of new_linfo
                // then we can choose the first item for new_rinfo
                // (as we made sure that there are at least two items)
                QModelIndex first = d->thumbView->firstIndex();
                new_rinfo         = d->thumbView->findItemByIndex(first);
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
        d->thumbView->setOnLeftPanel(new_linfo);
        slotSetItemOnLeftPanel(new_linfo);

        //  make this the selected item if the left was active before
        if (leftPanelActive)
        {
            d->thumbView->setCurrentInfo(new_linfo);
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
        d->thumbView->setOnRightPanel(new_rinfo);
        slotSetItemOnRightPanel(new_rinfo);

        //  make this the selected item if the left was active before
        if (!leftPanelActive)
        {
            d->thumbView->setCurrentInfo(new_rinfo);
        }
    }
    else
    {
        d->previewView->setRightImageInfo();
        d->rightSideBar->slotNoCurrentItem();
    }
}

void LightTableWindow::slotEditItem()
{
    if (!d->thumbView->currentInfo().isNull())
    {
        slotEditItem(d->thumbView->currentInfo());
    }
}

void LightTableWindow::slotEditItem(const ImageInfo& info)
{
    ImageWindow* const im = ImageWindow::imageWindow();
    ImageInfoList list    = d->thumbView->allImageInfos();

    im->loadImageInfos(list, info, i18n("Light Table"));

    if (im->isHidden())
    {
        im->show();
    }
    else
    {
        im->raise();
    }

    im->setFocus();
}

void LightTableWindow::slotPresentation()
{
    PresentationMngr* const mngr = new PresentationMngr(this);

    foreach(const ImageInfo& info, d->thumbView->allImageInfos())
    {
        mngr->addFile(info.fileUrl(), info.comment());
        qApp->processEvents();
    }

    mngr->showConfigDialog();
}

void LightTableWindow::slotSlideShowAll()
{
   SlideShowBuilder* const builder = new SlideShowBuilder(d->thumbView->allImageInfos());

   d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                            i18n("Preparing slideshow. Please wait..."));

   connect(builder, SIGNAL(signalComplete(SlideShowSettings)),
           this, SLOT(slotSlideShowBuilderComplete(SlideShowSettings)));

   builder->run();
}

void LightTableWindow::slotLeftSlideShowManualFromCurrent()
{
    slotSlideShowManualFrom(d->previewView->leftImageInfo());
    d->fromLeftPreview = true;
}

void LightTableWindow::slotRightSlideShowManualFromCurrent()
{
    slotSlideShowManualFrom(d->previewView->rightImageInfo());
    d->fromLeftPreview = false;
}

void LightTableWindow::slotSlideShowManualFrom(const ImageInfo& info)
{
   SlideShowBuilder* const builder = new SlideShowBuilder(d->thumbView->allImageInfos());
   builder->setOverrideStartFrom(info);
   builder->setAutoPlayEnabled(false);

   d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                            i18n("Preparing slideshow. Please wait..."));

   connect(builder, SIGNAL(signalComplete(SlideShowSettings)),
           this, SLOT(slotSlideShowBuilderComplete(SlideShowSettings)));

   builder->run();
}

void LightTableWindow::slotSlideShowBuilderComplete(const SlideShowSettings& settings)
{
    SlideShow* const slide = new SlideShow(settings);
    TagsActionMngr::defaultManager()->registerActionsToWidget(slide);

    d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode, QString());
    slotRefreshStatusBar();

    if (settings.imageUrl.isValid())
    {
        slide->setCurrentItem(settings.imageUrl);
    }
    else if (settings.startWithCurrent)
    {
        slide->setCurrentItem(d->thumbView->currentInfo().fileUrl());
    }

    connect(slide, SIGNAL(signalRatingChanged(QUrl,int)),
            d->thumbView, SLOT(slotRatingChanged(QUrl,int)));

    connect(slide, SIGNAL(signalColorLabelChanged(QUrl,int)),
            d->thumbView, SLOT(slotColorLabelChanged(QUrl,int)));

    connect(slide, SIGNAL(signalPickLabelChanged(QUrl,int)),
            d->thumbView, SLOT(slotPickLabelChanged(QUrl,int)));

    connect(slide, SIGNAL(signalToggleTag(QUrl,int)),
            d->thumbView, SLOT(slotToggleTag(QUrl,int)));

    connect(slide, SIGNAL(signalLastItemUrl(QUrl)),
            this, SLOT(slotSlideShowLastItemUrl(QUrl)));

    slide->show();
}

void LightTableWindow::slotSlideShowLastItemUrl(const QUrl& url)
{
    if (d->fromLeftPreview && !d->navigateByPairAction->isChecked())
    {
        d->thumbView->blockSignals(true);
        d->thumbView->setCurrentUrl(url);
        d->thumbView->blockSignals(false);
        slotSetItemLeft();
    }
    else
    {
        d->thumbView->setCurrentUrl(url);
    }
}

void LightTableWindow::slotSetup()
{
    Setup::execDialog(this);
}

void LightTableWindow::slotLeftZoomFactorChanged(double zoom)
{
    double zmin = d->previewView->leftZoomMin();
    double zmax = d->previewView->leftZoomMax();
    d->leftZoomBar->setZoom(zoom, zmin, zmax);

    d->leftZoomPlusAction->setEnabled(!d->previewView->leftMaxZoom());
    d->leftZoomMinusAction->setEnabled(!d->previewView->leftMinZoom());
}

void LightTableWindow::slotRightZoomFactorChanged(double zoom)
{
    double zmin = d->previewView->rightZoomMin();
    double zmax = d->previewView->rightZoomMax();
    d->rightZoomBar->setZoom(zoom, zmin, zmax);

    d->rightZoomPlusAction->setEnabled(!d->previewView->rightMaxZoom());
    d->rightZoomMinusAction->setEnabled(!d->previewView->rightMinZoom());
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
        {
            d->syncPreviewAction->setChecked(true);
        }
    }
}

void LightTableWindow::slotBackward()
{
    d->thumbView->toPreviousIndex();
}

void LightTableWindow::slotForward()
{
    d->thumbView->toNextIndex();
}

void LightTableWindow::slotFirst()
{
    d->thumbView->toFirstIndex();
}

void LightTableWindow::slotLast()
{
    d->thumbView->toLastIndex();
}

void LightTableWindow::slotToggleNavigateByPair()
{
    d->thumbView->setNavigateByPair(d->navigateByPairAction->isChecked());
    d->previewView->setNavigateByPair(d->navigateByPairAction->isChecked());
    slotItemSelected(d->thumbView->currentInfo());
}

void LightTableWindow::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void LightTableWindow::slotDBStat()
{
    showDigikamDatabaseStat();
}

void LightTableWindow::slotApplicationSettingsChanged()
{
    d->leftSideBar->setStyle(ApplicationSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->setStyle(ApplicationSettings::instance()->getSidebarTitleStyle());

    /// @todo Which part of the settings has to be reloaded?
    //     d->rightSideBar->applySettings();

    d->previewView->setPreviewSettings(ApplicationSettings::instance()->getPreviewSettings());
}

void LightTableWindow::moveEvent(QMoveEvent* e)
{
    Q_UNUSED(e)
    emit signalWindowHasMoved();
}

void LightTableWindow::toggleTag(int tagID)
{
    d->thumbView->toggleTag(tagID);
}

void LightTableWindow::slotAssignPickLabel(int pickId)
{
    d->thumbView->slotAssignPickLabel(pickId);
}

void LightTableWindow::slotAssignColorLabel(int colorId)
{
    d->thumbView->slotAssignColorLabel(colorId);
}

void LightTableWindow::slotAssignRating(int rating)
{
    d->thumbView->slotAssignRating(rating);
}

void LightTableWindow::slotThemeChanged()
{
    d->previewView->checkForSelection(d->previewView->leftImageInfo());
    d->previewView->checkForSelection(d->previewView->rightImageInfo());
}

void LightTableWindow::showSideBars(bool visible)
{
    if (visible)
    {
        d->leftSideBar->restore();
        d->rightSideBar->restore();
    }
    else
    {
        d->leftSideBar->backup();
        d->rightSideBar->backup();
    }
}

void LightTableWindow::slotToggleLeftSideBar()
{
    d->leftSideBar->isExpanded() ? d->leftSideBar->shrink()
                                 : d->leftSideBar->expand();
}

void LightTableWindow::slotToggleRightSideBar()
{
    d->rightSideBar->isExpanded() ? d->rightSideBar->shrink()
                                  : d->rightSideBar->expand();
}

void LightTableWindow::slotPreviousLeftSideBarTab()
{
    d->leftSideBar->activePreviousTab();
}

void LightTableWindow::slotNextLeftSideBarTab()
{
    d->leftSideBar->activeNextTab();
}

void LightTableWindow::slotPreviousRightSideBarTab()
{
    d->rightSideBar->activePreviousTab();
}

void LightTableWindow::slotNextRightSideBarTab()
{
    d->rightSideBar->activeNextTab();
}

void LightTableWindow::customizedFullScreenMode(bool set)
{
    showStatusBarAction()->setEnabled(!set);
    toolBarMenuAction()->setEnabled(!set);
    showMenuBarAction()->setEnabled(!set);
    d->showBarAction->setEnabled(!set);

    d->previewView->toggleFullScreen(set);
}

void LightTableWindow::slotFileWithDefaultApplication()
{
    if (!d->thumbView->currentInfo().isNull())
    {
        DFileOperations::openFilesWithDefaultApplication(QList<QUrl>() << d->thumbView->currentInfo().fileUrl());
    }
}

void LightTableWindow::slotRightSideBarActivateTitles()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->setFocusToTitlesEdit();
}

void LightTableWindow::slotRightSideBarActivateComments()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->setFocusToCommentsEdit();
}

void LightTableWindow::slotRightSideBarActivateAssignedTags()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->activateAssignedTagsButton();
}

void LightTableWindow::slotLeftSideBarActivateTitles()
{
    d->leftSideBar->setActiveTab(d->leftSideBar->imageDescEditTab());
    d->leftSideBar->imageDescEditTab()->setFocusToTitlesEdit();
}

void LightTableWindow::slotLeftSideBarActivateComments()
{
    d->leftSideBar->setActiveTab(d->leftSideBar->imageDescEditTab());
    d->leftSideBar->imageDescEditTab()->setFocusToCommentsEdit();
}

void LightTableWindow::slotLeftSideBarActivateAssignedTags()
{
    d->leftSideBar->setActiveTab(d->leftSideBar->imageDescEditTab());
    d->leftSideBar->imageDescEditTab()->activateAssignedTagsButton();
}

void LightTableWindow::slotToggleColorManagedView()
{
    if (!IccSettings::instance()->isEnabled())
    {
        return;
    }

    bool cmv = !IccSettings::instance()->settings().useManagedPreviews;
    IccSettings::instance()->setUseManagedPreviews(cmv);
}

void LightTableWindow::slotColorManagementOptionsChanged()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    d->viewCMViewAction->blockSignals(true);
    d->viewCMViewAction->setEnabled(settings.enableCM);
    d->viewCMViewAction->setChecked(settings.useManagedPreviews);
    d->viewCMViewAction->blockSignals(false);
}

void LightTableWindow::slotEditGeolocation()
{
#ifdef HAVE_MARBLE
    ImageInfoList infos = d->thumbView->allImageInfos();

    if (infos.isEmpty())
    {
        return;
    }

    TagModel* const tagModel                    = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, this);
    TagPropertiesFilterModel* const filterModel = new TagPropertiesFilterModel(this);
    filterModel->setSourceAlbumModel(tagModel);
    filterModel->sort(0);

    QPointer<GeolocationEdit> dialog = new GeolocationEdit(filterModel,
                                                           new DBInfoIface(this, d->thumbView->allUrls()),
                                                           QApplication::activeWindow());
    dialog->setItems(ImageGPS::infosToItems(infos));
    dialog->exec();

    delete dialog;

    // Refresh Database with new metadata from files.
    foreach(const ImageInfo& inf, infos)
    {
        ScanController::instance()->scannedInfo(inf.fileUrl().toLocalFile());
    }
#endif
}

void LightTableWindow::slotEditMetadata()
{
    if (d->thumbView->currentInfo().isNull())
    {
        return;
    }

    QUrl url = d->thumbView->currentInfo().fileUrl();

    QPointer<MetadataEditDialog> dialog = new MetadataEditDialog(QApplication::activeWindow(),
                                                                 QList<QUrl>() << url);
    dialog->exec();

    delete dialog;

    // Refresh Database with new metadata from file.
    CollectionScanner scanner;
    scanner.scanFile(url.toLocalFile(), CollectionScanner::Rescan);
}

void LightTableWindow::slotImportFromScanner()
{
#ifdef HAVE_KSANE
    m_ksaneAction->activate(DigikamApp::instance()->scannerTargetPlace(), configGroupName());

    connect(m_ksaneAction, SIGNAL(signalImportedImage(QUrl)),
            this, SLOT(slotImportedImagefromScanner(QUrl)));
#endif
}

void LightTableWindow::slotImportedImagefromScanner(const QUrl& url)
{
    ImageInfo info = ScanController::instance()->scannedInfo(url.toLocalFile());
    loadImageInfos(ImageInfoList() << info, info, true);
}

void LightTableWindow::slotHtmlGallery()
{
#ifdef HAVE_HTMLGALLERY
    HTMLWizard w(this, new DBInfoIface(this, d->thumbView->allUrls()));
    w.exec();
#endif
}

void LightTableWindow::slotCalendar()
{
    CalWizard w(d->thumbView->allUrls(), this);
    w.exec();
}

void LightTableWindow::slotPanorama()
{
#ifdef HAVE_PANORAMA
    PanoManager::instance()->checkBinaries();
    PanoManager::instance()->setItemsList(d->thumbView->allUrls());
    PanoManager::instance()->run();
#endif
}

void LightTableWindow::slotExpoBlending()
{
    ExpoBlendingManager::instance()->checkBinaries();
    ExpoBlendingManager::instance()->setItemsList(d->thumbView->allUrls());
    ExpoBlendingManager::instance()->run();
}

void LightTableWindow::slotVideoSlideshow()
{
#ifdef HAVE_MEDIAPLAYER
    VidSlideWizard w(this, new DBInfoIface(this, d->thumbView->allUrls()));
    w.exec();
#endif
}

void LightTableWindow::slotSendByMail()
{
    MailWizard w(this, new DBInfoIface(this, d->thumbView->allUrls()));
    w.exec();
}

void LightTableWindow::slotPrintCreator()
{
    AdvPrintWizard w(this, new DBInfoIface(this, d->thumbView->allUrls()));
    w.exec();
}

void LightTableWindow::slotMediaServer()
{
    DBInfoIface* const iface = new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools);
    // NOTE: We overwrite the default albums chooser object name for load save check items state between sessions.
    // The goal is not mix these settings with other export tools.
    iface->setObjectName(QLatin1String("SetupMediaServerIface"));

    DMediaServerDlg w(this, iface);
    w.exec();
}

void LightTableWindow::slotExportTool()
{
    QAction* const tool = dynamic_cast<QAction*>(sender());

    if (tool == m_exportDropboxAction)
    {
        DBWindow w(new DBInfoIface(this, d->thumbView->allUrls(),
                                   ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportFacebookAction)
    {
        FbWindow w(new DBInfoIface(this, d->thumbView->allUrls(),
                                   ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportFlickrAction)
    {
        FlickrWindow w(new DBInfoIface(this, d->thumbView->allUrls(),
                                       ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportGdriveAction)
    {
        GSWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport),
                   this, QLatin1String("googledriveexport"));
        w.exec();
    }
    else if (tool == m_exportGphotoAction)
    {
        GSWindow w(new DBInfoIface(this, d->thumbView->allUrls(),
                                   ApplicationSettings::ImportExport),
                   this, QLatin1String("googlephotoexport"));
        w.exec();
    }
    else if (tool == m_exportImageshackAction)
    {
        ImageShackWindow w(new DBInfoIface(this, d->thumbView->allUrls(),
                                           ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportImgurAction)
    {
        ImgurWindow w(new DBInfoIface(this, d->thumbView->allUrls(),
                                      ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportPiwigoAction)
    {
        PiwigoWindow w(new DBInfoIface(this, d->thumbView->allUrls(),
                                       ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportRajceAction)
    {
        RajceWindow w(new DBInfoIface(this, d->thumbView->allUrls(),
                                      ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportSmugmugAction)
    {
        SmugWindow w(new DBInfoIface(this, d->thumbView->allUrls(),
                                     ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportYandexfotkiAction)
    {
        YFWindow w(new DBInfoIface(this, d->thumbView->allUrls(),
                                   ApplicationSettings::ImportExport), this);
        w.exec();
    }

#ifdef HAVE_MEDIAWIKI
    else if (tool == m_exportMediawikiAction)
    {
        MediaWikiWindow w(new DBInfoIface(this, d->thumbView->allUrls(), 
                                          ApplicationSettings::ImportExport), this);
        w.exec();
    }
#endif

#ifdef HAVE_VKONTAKTE
    else if (tool == m_exportVkontakteAction)
    {
        VKWindow w(new DBInfoIface(this, d->thumbView->allUrls(),
                                   ApplicationSettings::ImportExport), this);
        w.exec();
    }
#endif

#ifdef HAVE_KIO
    else if (tool == m_exportFileTransferAction)
    {
        FTExportWindow w(new DBInfoIface(this, d->thumbView->allUrls(),
                                         ApplicationSettings::ImportExport), this);
        w.exec();
    }
#endif
}

void LightTableWindow::slotImportTool()
{
    QAction* const tool = dynamic_cast<QAction*>(sender());

    if (tool == m_importGphotoAction)
    {
        GSWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport),
                   this, QLatin1String("googlephotoimport"));
        w.exec();
    }
    else if (tool == m_importSmugmugAction)
    {
        SmugWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport),
                     this, true);
        w.exec();
    }

#ifdef HAVE_KIO
    else if (tool == m_importFileTransferAction)
    {
        FTImportWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w.exec();
    }
#endif
}

} // namespace Digikam
