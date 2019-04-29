/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam image editor - Internal setup
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

#include "imagewindow.h"
#include "imagewindow_p.h"

namespace Digikam
{

void ImageWindow::setupActions()
{
    setupStandardActions();

    KActionCollection* const ac = actionCollection();

    d->toMainWindowAction = new QAction(QIcon::fromTheme(QLatin1String("view-list-icons")),
                                        i18nc("@action Finish editing, close editor, back to main window", "Close Editor"), this);
    connect(d->toMainWindowAction, SIGNAL(triggered()), this, SLOT(slotToMainWindow()));
    ac->addAction(QLatin1String("imageview_tomainwindow"), d->toMainWindowAction);


    // -- Special Delete actions ---------------------------------------------------------------

    // Pop up dialog to ask user whether to permanently delete

    d->fileDeletePermanentlyAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete File Permanently"), this);
    connect(d->fileDeletePermanentlyAction, SIGNAL(triggered()),
            this, SLOT(slotDeleteCurrentItemPermanently()));
    ac->addAction(QLatin1String("image_delete_permanently"), d->fileDeletePermanentlyAction);
    ac->setDefaultShortcut(d->fileDeletePermanentlyAction, Qt::SHIFT + Qt::Key_Delete);

    // These two actions are hidden, no menu entry, no toolbar entry, no shortcut.
    // Power users may add them.

    d->fileDeletePermanentlyDirectlyAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),
                                                         i18n("Delete Permanently without Confirmation"), this);
    connect(d->fileDeletePermanentlyDirectlyAction, SIGNAL(triggered()),
            this, SLOT(slotDeleteCurrentItemPermanentlyDirectly()));
    ac->addAction(QLatin1String("image_delete_permanently_directly"),
                                  d->fileDeletePermanentlyDirectlyAction);

    d->fileTrashDirectlyAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash-full")),
                                             i18n("Move to Trash without Confirmation"), this);
    connect(d->fileTrashDirectlyAction, SIGNAL(triggered()),
            this, SLOT(slotTrashCurrentItemDirectly()));
    ac->addAction(QLatin1String("image_trash_directly"), d->fileTrashDirectlyAction);

    // ---------------------------------------------------------------------------------

    createHelpActions();

    // Labels shortcuts must be registered here to be saved in XML GUI files if user customize it.
    TagsActionMngr::defaultManager()->registerLabelsActions(ac);

    QAction* const editTitles = new QAction(i18n("Edit Titles"), this);
    ac->addAction(QLatin1String("edit_titles"), editTitles);
    ac->setDefaultShortcut(editTitles, Qt::ALT + Qt::SHIFT + Qt::Key_T);
    connect(editTitles, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateTitles()));

    QAction* const editComments = new QAction(i18n("Edit Comments"), this);
    ac->addAction(QLatin1String("edit_comments"), editComments);
    ac->setDefaultShortcut(editComments, Qt::ALT + Qt::SHIFT + Qt::Key_C);
    connect(editComments, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateComments()));

    QAction* const assignedTags = new QAction(i18n("Show Assigned Tags"), this);
    ac->addAction(QLatin1String("assigned_tags"), assignedTags);
    ac->setDefaultShortcut(assignedTags, Qt::ALT + Qt::SHIFT + Qt::Key_A);
    connect(assignedTags, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateAssignedTags()));
}

void ImageWindow::setupConnections()
{
    setupStandardConnections();

    connect(this, SIGNAL(loadCurrentLater()),
            this, SLOT(slotLoadCurrent()), Qt::QueuedConnection);

    // To toggle properly keyboards shortcuts from comments & tags side bar tab.

    connect(d->rightSideBar, SIGNAL(signalNextItem()),
            this, SLOT(slotForward()));

    connect(d->rightSideBar, SIGNAL(signalPrevItem()),
            this, SLOT(slotBackward()));

    connect(d->rightSideBar->getFiltersHistoryTab(), SIGNAL(actionTriggered(ItemInfo)),
            this, SLOT(openImage(ItemInfo)));

    connect(this, SIGNAL(signalSelectionChanged(QRect)),
            d->rightSideBar, SLOT(slotImageSelectionChanged(QRect)));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSideBar, SLOT(slotNoCurrentItem()));

    ItemAttributesWatch* watch = ItemAttributesWatch::instance();

    connect(watch, SIGNAL(signalFileMetadataChanged(QUrl)),
            this, SLOT(slotFileMetadataChanged(QUrl)));

/*
    connect(CoreDbAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)),
            Qt::QueuedConnection);

    connect(d->imageFilterModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));
*/

    connect(d->thumbBar, SIGNAL(currentChanged(ItemInfo)),
            this, SLOT(slotThumbBarImageSelected(ItemInfo)));

    connect(d->dragDropHandler, SIGNAL(itemInfosDropped(QList<ItemInfo>)),
            this, SLOT(slotDroppedOnThumbbar(QList<ItemInfo>)));

    connect(d->thumbBarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            d->thumbBar, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    connect(d->imageInfoModel, SIGNAL(allRefreshingFinished()),
            this, SLOT(slotThumbBarModelReady()));

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));
}

void ImageWindow::setupUserArea()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    QWidget* const widget   = new QWidget(this);
    QHBoxLayout* const hlay = new QHBoxLayout(widget);
    m_splitter              = new SidebarSplitter(widget);

    d->viewContainer        = new KMainWindow(widget, Qt::Widget);
    m_splitter->addWidget(d->viewContainer);
    m_stackView             = new EditorStackView(d->viewContainer);
    m_canvas                = new Canvas(m_stackView);
    d->viewContainer->setCentralWidget(m_stackView);

    m_splitter->setFrameStyle(QFrame::NoFrame);
    m_splitter->setFrameShape(QFrame::NoFrame);
    m_splitter->setFrameShadow(QFrame::Plain);
    m_splitter->setStretchFactor(0, 10);      // set Canvas default size to max.
    m_splitter->setOpaqueResize(false);

    m_canvas->makeDefaultEditingCanvas();
    m_stackView->setCanvas(m_canvas);
    m_stackView->setViewMode(EditorStackView::CanvasMode);

    d->rightSideBar = new ItemPropertiesSideBarDB(widget, m_splitter, Qt::RightEdge, true);
    d->rightSideBar->setObjectName(QLatin1String("ImageEditor Right Sidebar"));
    d->rightSideBar->getFiltersHistoryTab()->addOpenImageAction();

    hlay->addWidget(m_splitter);
    hlay->addWidget(d->rightSideBar);
    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(0);

    // Code to check for the now depreciated HorizontalThumbar directive. It
    // is found, it is honored and deleted. The state will from than on be saved
    // by d->viewContainers built-in mechanism.
    Qt::DockWidgetArea dockArea = Qt::LeftDockWidgetArea;

    if (group.hasKey(d->configHorizontalThumbbarEntry))
    {
        if (group.readEntry(d->configHorizontalThumbbarEntry, true))
        {
            // Horizontal thumbbar layout
            dockArea    = Qt::TopDockWidgetArea;
        }

        group.deleteEntry(d->configHorizontalThumbbarEntry);
    }

    d->imageInfoModel   = new ItemListModel(this);

    d->imageFilterModel = new ItemFilterModel(this);
    d->imageFilterModel->setSourceItemModel(d->imageInfoModel);

    d->imageInfoModel->setWatchFlags(d->imageFilterModel->suggestedWatchFlags());
    d->imageInfoModel->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());

    d->imageFilterModel->setCategorizationMode(ItemSortSettings::NoCategories);
    d->imageFilterModel->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
    d->imageFilterModel->setSortRole((ItemSortSettings::SortRole)ApplicationSettings::instance()->getImageSortOrder());
    d->imageFilterModel->setSortOrder((ItemSortSettings::SortOrder)ApplicationSettings::instance()->getImageSorting());
    d->imageFilterModel->setAllGroupsOpen(true); // disable filtering out by group, see bug #283847
    d->imageFilterModel->sort(0); // an initial sorting is necessary

    d->dragDropHandler  = new ItemDragDropHandler(d->imageInfoModel);
    d->dragDropHandler->setReadOnlyDrop(true);
    d->imageInfoModel->setDragDropHandler(d->dragDropHandler);

    // The thumb bar is placed in a detachable/dockable widget.
    d->thumbBarDock     = new ThumbBarDock(d->viewContainer, Qt::Tool);
    d->thumbBarDock->setObjectName(QLatin1String("editor_thumbbar"));
    d->thumbBarDock->setWindowTitle(i18n("Image Editor Thumbnail Dock"));

    d->thumbBar         = new ItemThumbnailBar(d->thumbBarDock);
    d->thumbBar->setModels(d->imageInfoModel, d->imageFilterModel);

    d->thumbBarDock->setWidget(d->thumbBar);
    d->viewContainer->addDockWidget(dockArea, d->thumbBarDock);
    d->thumbBarDock->setFloating(false);
    //d->thumbBar->slotDockLocationChanged(dockArea);

    setCentralWidget(widget);
}

void ImageWindow::addServicesMenu()
{
    addServicesMenuForUrl(d->currentUrl());
}

void ImageWindow::slotContextMenu()
{
    if (m_contextMenu)
    {
        m_contextMenu->addSeparator();
        addServicesMenu();
        m_contextMenu->addSeparator();

        TagsPopupMenu* assignTagsMenu = nullptr;
        TagsPopupMenu* removeTagsMenu = nullptr;

        // Bulk assignment/removal of tags --------------------------

        QList<qlonglong> idList;
        idList << d->currentItemInfo.id();

        assignTagsMenu = new TagsPopupMenu(idList, TagsPopupMenu::RECENTLYASSIGNED, this);
        removeTagsMenu = new TagsPopupMenu(idList, TagsPopupMenu::REMOVE, this);
        assignTagsMenu->menuAction()->setText(i18n("Assign Tag"));
        removeTagsMenu->menuAction()->setText(i18n("Remove Tag"));

        m_contextMenu->addSeparator();

        m_contextMenu->addMenu(assignTagsMenu);
        m_contextMenu->addMenu(removeTagsMenu);

        connect(assignTagsMenu, SIGNAL(signalTagActivated(int)),
                this, SLOT(slotAssignTag(int)));

        connect(removeTagsMenu, SIGNAL(signalTagActivated(int)),
                this, SLOT(slotRemoveTag(int)));

        connect(assignTagsMenu, SIGNAL(signalPopupTagsView()),
                d->rightSideBar, SLOT(slotPopupTagsView()));

        if (!CoreDbAccess().db()->hasTags(idList))
        {
            m_contextMenu->menuAction()->setEnabled(false);
        }

        m_contextMenu->addSeparator();

        // Assign Labels -------------------------------------------

        QMenu* const menuLabels           = new QMenu(i18n("Assign Labels"), m_contextMenu);
        PickLabelMenuAction* const pmenu  = new PickLabelMenuAction(m_contextMenu);
        ColorLabelMenuAction* const cmenu = new ColorLabelMenuAction(m_contextMenu);
        RatingMenuAction* const rmenu     = new RatingMenuAction(m_contextMenu);
        menuLabels->addAction(pmenu->menuAction());
        menuLabels->addAction(cmenu->menuAction());
        menuLabels->addAction(rmenu->menuAction());
        m_contextMenu->addMenu(menuLabels);

        connect(pmenu, SIGNAL(signalPickLabelChanged(int)),
                this, SLOT(slotAssignPickLabel(int)));

        connect(cmenu, SIGNAL(signalColorLabelChanged(int)),
                this, SLOT(slotAssignColorLabel(int)));

        connect(rmenu, SIGNAL(signalRatingChanged(int)),
                this, SLOT(slotAssignRating(int)));

        // --------------------------------------------------------------

        m_contextMenu->exec(QCursor::pos());

        delete assignTagsMenu;
        delete removeTagsMenu;
        delete cmenu;
        delete pmenu;
        delete rmenu;
        delete menuLabels;
    }
}

} // namespace Digikam
