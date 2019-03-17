/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor - Internal setup
 *
 * Copyright (C) 2004-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "showfoto.h"
#include "showfoto_p.h"

namespace ShowFoto
{

void ShowFoto::setupActions()
{
    Digikam::ThemeManager::instance()->setThemeMenuAction(new QMenu(i18n("&Themes"), this));
    setupStandardActions();

    // Extra 'File' menu actions ---------------------------------------------

    d->fileOpenAction = buildStdAction(StdOpenAction, this, SLOT(slotOpenFile()), this);
    actionCollection()->addAction(QLatin1String("showfoto_open_file"), d->fileOpenAction);

    // ---------

    d->openFilesInFolderAction = new QAction(QIcon::fromTheme(QLatin1String("folder-pictures")), i18n("Open folder"), this);
    actionCollection()->setDefaultShortcut(d->openFilesInFolderAction, Qt::CTRL + Qt::SHIFT + Qt::Key_O);

    connect(d->openFilesInFolderAction, &QAction::triggered,
            this, &ShowFoto::slotOpenFolder);

    actionCollection()->addAction(QLatin1String("showfoto_open_folder"), d->openFilesInFolderAction);

    // ---------

    QAction* const quit = buildStdAction(StdQuitAction, this, SLOT(close()), this);
    actionCollection()->addAction(QLatin1String("showfoto_quit"), quit);

    // -- Standard 'Help' menu actions ---------------------------------------------

    createHelpActions(false);
}

void ShowFoto::setupConnections()
{
    setupStandardConnections();

    connect(d->thumbBarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            d->thumbBar, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    connect(d->thumbBar, SIGNAL(showfotoItemInfoActivated(ShowfotoItemInfo)),
            this, SLOT(slotShowfotoItemInfoActivated(ShowfotoItemInfo)));

    connect(this, SIGNAL(signalSelectionChanged(QRect)),
            d->rightSideBar, SLOT(slotImageSelectionChanged(QRect)));

    connect(this, &ShowFoto::signalOpenFolder,
            this, &ShowFoto::slotOpenFolder);

    connect(this, &ShowFoto::signalOpenFile,
            this, &ShowFoto::slotOpenFile);

    connect(this,SIGNAL(signalInfoList(ShowfotoItemInfoList&)),
            d->model,SLOT(reAddShowfotoItemInfos(ShowfotoItemInfoList&)));

    connect(d->thumbLoadThread,SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            d->model,SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSideBar, SLOT(slotNoCurrentItem()));

    connect(d->rightSideBar, SIGNAL(signalSetupMetadataFilters(int)),
            this, SLOT(slotSetupMetadataFilters(int)));

    connect(d->dDHandler, SIGNAL(signalDroppedUrls(QList<QUrl>,bool)),
            this, SLOT(slotDroppedUrls(QList<QUrl>,bool)));
}

void ShowFoto::setupUserArea()
{
    KSharedConfig::Ptr config  = KSharedConfig::openConfig();
    KConfigGroup group         = config->group(configGroupName());

    QWidget* const widget      = new QWidget(this);
    QHBoxLayout* const hlay    = new QHBoxLayout(widget);
    m_splitter                 = new Digikam::SidebarSplitter(widget);

    KMainWindow* const viewContainer = new KMainWindow(widget, Qt::Widget);
    m_splitter->addWidget(viewContainer);
    m_stackView                      = new Digikam::EditorStackView(viewContainer);
    m_canvas                         = new Digikam::Canvas(m_stackView);
    viewContainer->setCentralWidget(m_stackView);

    m_splitter->setFrameStyle(QFrame::NoFrame);
    m_splitter->setFrameShape(QFrame::NoFrame);
    m_splitter->setFrameShadow(QFrame::Plain);
    m_splitter->setStretchFactor(1, 10);      // set Canvas default size to max.
    m_splitter->setOpaqueResize(false);

    m_canvas->makeDefaultEditingCanvas();
    m_stackView->setCanvas(m_canvas);
    m_stackView->setViewMode(Digikam::EditorStackView::CanvasMode);

    d->rightSideBar = new Digikam::ItemPropertiesSideBar(widget, m_splitter, Qt::RightEdge);
    d->rightSideBar->setObjectName(QLatin1String("ShowFoto Sidebar Right"));

    hlay->addWidget(m_splitter);
    hlay->addWidget(d->rightSideBar);
    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(0);

    // Code to check for the now depreciated HorizontalThumbar directive. It
    // is found, it is honored and deleted. The state will from than on be saved
    // by viewContainers built-in mechanism.
    Qt::DockWidgetArea dockArea = Qt::LeftDockWidgetArea;

    d->thumbBarDock = new Digikam::ThumbBarDock(viewContainer, Qt::Tool);
    d->thumbBarDock->setObjectName(QLatin1String("editor_thumbbar"));
    d->thumbBarDock->setWindowTitle(i18n("ShowFoto Thumbnail Dock"));
    d->thumbBarDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea);
    d->thumbBar     = new ShowfotoThumbnailBar(d->thumbBarDock);

    d->thumbBarDock->setWidget(d->thumbBar);

    viewContainer->addDockWidget(dockArea, d->thumbBarDock);
    d->thumbBarDock->setFloating(false);

    d->model       = new ShowfotoThumbnailModel(d->thumbBar);
    d->model->setThumbnailLoadThread(d->thumbLoadThread);
    d->dDHandler   = new ShowfotoDragDropHandler(d->model);
    d->model->setDragDropHandler(d->dDHandler);

    d->filterModel = new ShowfotoFilterModel(d->thumbBar);
    d->filterModel->setSourceShowfotoModel(d->model);
    d->filterModel->setCategorizationMode(ShowfotoItemSortSettings::NoCategories);
    d->filterModel->sort(0);

    d->thumbBar->setModels(d->model, d->filterModel);
    d->thumbBar->setSelectionMode(QAbstractItemView::SingleSelection);

    viewContainer->setAutoSaveSettings(QLatin1String("ImageViewer Thumbbar"), true);

    d->thumbBar->installOverlays();

    setCentralWidget(widget);
}

void ShowFoto::slotContextMenu()
{
    if (m_contextMenu)
    {
        m_contextMenu->addSeparator();
        addServicesMenu();
        m_contextMenu->exec(QCursor::pos());
    }
}

void ShowFoto::addServicesMenu()
{
    addServicesMenuForUrl(d->thumbBar->currentUrl());
}

void ShowFoto::toggleNavigation(int index)
{
    if (!m_actionEnabledState)
    {
        return;
    }

    if (d->itemsNb == 0 || d->itemsNb == 1)
    {
        m_backwardAction->setEnabled(false);
        m_forwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }
    else
    {
        m_backwardAction->setEnabled(true);
        m_forwardAction->setEnabled(true);
        m_firstAction->setEnabled(true);
        m_lastAction->setEnabled(true);
    }

    if (index == 1)
    {
        m_backwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
    }

    if (index == d->itemsNb)
    {
        m_forwardAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }
}

void ShowFoto::toggleActions(bool val)
{
    toggleStandardActions(val);
}

} // namespace ShowFoto
