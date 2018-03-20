/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes

#include <cstdio>
#include <vector>
#include <algorithm>

// Qt includes

#include <QKeySequence>
#include <QCloseEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPersistentModelIndex>
#include <QPixmap>
#include <QProgressBar>
#include <QSplitter>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QApplication>

// KDE includes

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kwindowsystem.h>

// Local includes

#include "dlayoutbox.h"
#include "album.h"
#include "coredb.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "applicationsettings.h"
#include "canvas.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "collectionscanner.h"
#include "componentsinfo.h"
#include "coredbaccess.h"
#include "coredbwatch.h"
#include "coredbchangesets.h"
#include "ddragobjects.h"
#include "deletedialog.h"
#include "dimg.h"
#include "editorcore.h"
#include "dimagehistory.h"
#include "digikamapp.h"
#include "dio.h"
#include "dmetadata.h"
#include "editorstackview.h"
#include "fileactionmngr.h"
#include "dfileoperations.h"
#include "digikam_globals.h"
#include "iccsettingscontainer.h"
#include "imageattributeswatch.h"
#include "imagefiltermodel.h"
#include "imagedragdrop.h"
#include "imagedescedittab.h"
#include "imageinfo.h"
#include "imagegps.h"
#include "imagelistmodel.h"
#include "imagepropertiessidebardb.h"
#include "imagepropertiesversionstab.h"
#include "imagescanner.h"
#include "imagethumbnailbar.h"
#include "iofilesettings.h"
#include "dnotificationwrapper.h"
#include "loadingcacheinterface.h"
#include "metadatahub.h"
#include "metadatasettings.h"
#include "metadataedit.h"
#include "colorlabelwidget.h"
#include "picklabelwidget.h"
#include "presentationmngr.h"
#include "ratingwidget.h"
#include "savingcontext.h"
#include "scancontroller.h"
#include "setup.h"
#include "slideshow.h"
#include "statusprogressbar.h"
#include "syncjob.h"
#include "tagsactionmngr.h"
#include "tagscache.h"
#include "tagspopupmenu.h"
#include "tagregion.h"
#include "thememanager.h"
#include "thumbbardock.h"
#include "thumbnailloadthread.h"
#include "undostate.h"
#include "imagewindow_p.h"
#include "digikam_debug.h"
#include "dexpanderbox.h"
#include "dbinfoiface.h"
#include "calwizard.h"
#include "expoblendingmanager.h"
#include "mailwizard.h"
#include "advprintwizard.h"
#include "dmediaserverdlg.h"
#include "facetagseditor.h"
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

ImageWindow* ImageWindow::m_instance = 0;

ImageWindow* ImageWindow::imageWindow()
{
    if (!m_instance)
    {
        new ImageWindow();
    }

    return m_instance;
}

bool ImageWindow::imageWindowCreated()
{
    return m_instance;
}

ImageWindow::ImageWindow()
    : EditorWindow(QLatin1String("Image Editor")),
      d(new Private)
{
    setXMLFile(QLatin1String("imageeditorui5.rc"));

    m_instance = this;
    // We don't want to be deleted on close
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAcceptDrops(true);

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupActions();
    setupStatusBar();
    createGUI(xmlFile());
    cleanupActions();

    showMenuBarAction()->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080

    // Create tool selection view

    setupSelectToolsAction();

    // Create context menu.

    setupContextMenu();

    // Make signals/slots connections

    setupConnections();

    // -- Read settings --------------------------------

    readSettings();
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());
    applyMainWindowSettings(group);
    d->thumbBarDock->setShouldBeVisible(group.readEntry(d->configShowThumbbarEntry, false));
    setAutoSaveSettings(configGroupName(), true);
    d->viewContainer->setAutoSaveSettings(QLatin1String("ImageViewer Thumbbar"), true);

    //-------------------------------------------------------------

    d->rightSideBar->setConfigGroup(KConfigGroup(&group, QLatin1String("Right Sidebar")));
    d->rightSideBar->loadState();
    d->rightSideBar->populateTags();

    slotSetupChanged();
}

ImageWindow::~ImageWindow()
{
    m_instance = 0;

    delete d->rightSideBar;
    delete d->thumbBar;
    delete d;
}

void ImageWindow::closeEvent(QCloseEvent* e)
{
    if (!queryClose())
    {
        e->ignore();
        return;
    }

    // put right side bar in a defined state
    emit signalNoCurrentItem();

    m_canvas->resetImage();

    // There is one nasty habit with the thumbnail bar if it is floating: it
    // doesn't close when the parent window does, so it needs to be manually
    // closed. If the light table is opened again, its original state needs to
    // be restored.
    // This only needs to be done when closing a visible window and not when
    // destroying a closed window, since the latter case will always report that
    // the thumbnail bar isn't visible.
    if (isVisible())
    {
        thumbBar()->hide();
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());
    saveMainWindowSettings(group);
    saveSettings();

    d->rightSideBar->setConfigGroup(KConfigGroup(&group, "Right Sidebar"));
    d->rightSideBar->saveState();

    DXmlGuiWindow::closeEvent(e);
    e->accept();
}

void ImageWindow::showEvent(QShowEvent*)
{
    // Restore the visibility of the thumbbar and start autosaving again.
    thumbBar()->restoreVisibility();
}

bool ImageWindow::queryClose()
{
    // Note: we re-implement closeEvent above for this window.
    // Additionally, queryClose is called from DigikamApp.

    // wait if a save operation is currently running
    if (!waitForSavingToComplete())
    {
        return false;
    }

    return promptUserSave(d->currentUrl());
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

    d->rightSideBar = new ImagePropertiesSideBarDB(widget, m_splitter, Qt::RightEdge, true);
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

    d->imageInfoModel   = new ImageListModel(this);

    d->imageFilterModel = new ImageFilterModel(this);
    d->imageFilterModel->setSourceImageModel(d->imageInfoModel);

    d->imageInfoModel->setWatchFlags(d->imageFilterModel->suggestedWatchFlags());
    d->imageInfoModel->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());

    d->imageFilterModel->setCategorizationMode(ImageSortSettings::NoCategories);
    d->imageFilterModel->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
    d->imageFilterModel->setSortRole((ImageSortSettings::SortRole)ApplicationSettings::instance()->getImageSortOrder());
    d->imageFilterModel->setSortOrder((ImageSortSettings::SortOrder)ApplicationSettings::instance()->getImageSorting());
    d->imageFilterModel->setAllGroupsOpen(true); // disable filtering out by group, see bug #283847
    d->imageFilterModel->sort(0); // an initial sorting is necessary

    d->dragDropHandler  = new ImageDragDropHandler(d->imageInfoModel);
    d->dragDropHandler->setReadOnlyDrop(true);
    d->imageInfoModel->setDragDropHandler(d->dragDropHandler);

    // The thumb bar is placed in a detachable/dockable widget.
    d->thumbBarDock     = new ThumbBarDock(d->viewContainer, Qt::Tool);
    d->thumbBarDock->setObjectName(QLatin1String("editor_thumbbar"));

    d->thumbBar         = new ImageThumbnailBar(d->thumbBarDock);
    d->thumbBar->setModels(d->imageInfoModel, d->imageFilterModel);

    d->thumbBarDock->setWidget(d->thumbBar);
    d->viewContainer->addDockWidget(dockArea, d->thumbBarDock);
    d->thumbBarDock->setFloating(false);
    //d->thumbBar->slotDockLocationChanged(dockArea);

    setCentralWidget(widget);
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

    connect(d->rightSideBar->getFiltersHistoryTab(), SIGNAL(actionTriggered(ImageInfo)),
            this, SLOT(openImage(ImageInfo)));

    connect(this, SIGNAL(signalSelectionChanged(QRect)),
            d->rightSideBar, SLOT(slotImageSelectionChanged(QRect)));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSideBar, SLOT(slotNoCurrentItem()));

    ImageAttributesWatch* watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalFileMetadataChanged(QUrl)),
            this, SLOT(slotFileMetadataChanged(QUrl)));

/*
    connect(CoreDbAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)),
            Qt::QueuedConnection);

    connect(d->imageFilterModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(slotRowsAboutToBeRemoved(QModelIndex,int,int)));
*/

    connect(d->thumbBar, SIGNAL(currentChanged(ImageInfo)),
            this, SLOT(slotThumbBarImageSelected(ImageInfo)));

    connect(d->dragDropHandler, SIGNAL(imageInfosDropped(QList<ImageInfo>)),
            this, SLOT(slotDroppedOnThumbbar(QList<ImageInfo>)));

    connect(d->thumbBarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            d->thumbBar, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    connect(d->imageInfoModel, SIGNAL(allRefreshingFinished()),
            this, SLOT(slotThumbBarModelReady()));

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));
}

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

    d->fileTrashDirectlyAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")),
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
    ac->setDefaultShortcut(editTitles, Qt::META + Qt::Key_T);
    connect(editTitles, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateTitles()));

    QAction* const editComments = new QAction(i18n("Edit Comments"), this);
    ac->addAction(QLatin1String("edit_comments"), editComments);
    ac->setDefaultShortcut(editComments, Qt::META + Qt::Key_C);
    connect(editComments, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateComments()));

    QAction* const assignedTags = new QAction(i18n("Show Assigned Tags"), this);
    ac->addAction(QLatin1String("assigned _tags"), assignedTags);
    ac->setDefaultShortcut(assignedTags, Qt::META + Qt::Key_A);
    connect(assignedTags, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateAssignedTags()));
}

void ImageWindow::slotSetupChanged()
{
    applyStandardSettings();

    VersionManagerSettings versionSettings = ApplicationSettings::instance()->getVersionManagerSettings();
    d->versionManager.setSettings(versionSettings);
    m_nonDestructive                       = versionSettings.enabled;
    toggleNonDestructiveActions();

    d->imageFilterModel->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
    d->imageFilterModel->setSortRole((ImageSortSettings::SortRole)ApplicationSettings::instance()->getImageSortOrder());
    d->imageFilterModel->setSortOrder((ImageSortSettings::SortOrder)ApplicationSettings::instance()->getImageSorting());
    d->rightSideBar->setStyle(ApplicationSettings::instance()->getSidebarTitleStyle());
}

void ImageWindow::loadImageInfos(const ImageInfoList& imageInfoList, const ImageInfo& imageInfoCurrent,
                                 const QString& caption)
{
    // Very first thing is to check for changes, user may choose to cancel operation
    if (!promptUserSave(d->currentUrl(), AskIfNeeded))
    {
        return;
    }

    d->currentImageInfo = ImageInfo();
    d->currentImageInfo = imageInfoCurrent;

    // Note: Addition is asynchronous, indexes not yet available
    // We enable thumbbar as soon as indexes are available
    // If not, we load imageInfoCurrent, then the index 0, then again imageInfoCurrent
    d->thumbBar->setEnabled(false);
    d->imageInfoModel->setImageInfos(imageInfoList);
    d->setThumbBarToCurrent();

    if (!caption.isEmpty())
    {
        setCaption(i18n("Image Editor - %1", caption));
    }
    else
    {
        setCaption(i18n("Image Editor"));
    }

    // it can slightly improve the responsiveness when inserting an event loop run here
    QTimer::singleShot(0, this, SLOT(slotLoadImageInfosStage2()));
}

void ImageWindow::slotLoadImageInfosStage2()
{
    // if window is minimized, show it
    if (isMinimized())
    {
        KWindowSystem::unminimizeWindow(winId());
    }

    slotLoadCurrent();
}

void ImageWindow::slotThumbBarModelReady()
{
    d->thumbBar->setEnabled(true);
}

void ImageWindow::openImage(const ImageInfo& info)
{
    if (d->currentImageInfo == info)
    {
        return;
    }

    d->currentImageInfo = info;
    d->ensureModelContains(d->currentImageInfo);

    slotLoadCurrent();
}

void ImageWindow::slotLoadCurrent()
{
    if (!d->currentIsValid())
    {
        return;
    }

    m_canvas->load(d->currentImageInfo.filePath(), m_IOFileSettings);

    QModelIndex next = d->nextIndex();

    if (next.isValid())
    {
        m_canvas->preload(d->imageInfo(next).filePath());
    }

    d->setThumbBarToCurrent();

    // Do this _after_ the canvas->load(), so that the main view histogram does not load
    // a smaller version if a raw image, and after that the EditorCore loads the full version.
    // So first let EditorCore create its loading task, only then any external objects.
    setViewToURL(d->currentUrl());
}

void ImageWindow::setViewToURL(const QUrl& url)
{
    emit signalURLChanged(url);
}

void ImageWindow::slotThumbBarImageSelected(const ImageInfo& info)
{
    if (d->currentImageInfo == info || !d->thumbBar->isEnabled())
    {
        return;
    }

    if (!promptUserSave(d->currentUrl(), AskIfNeeded, false))
    {
        return;
    }

    d->currentImageInfo = info;
    slotLoadCurrent();
}

void ImageWindow::slotDroppedOnThumbbar(const QList<ImageInfo>& infos)
{
    // Check whether dropped image list is empty

    if (infos.isEmpty())
    {
        return;
    }

    // Create new list and images that are not present currently in the thumbbar

    QList<ImageInfo> toAdd;

    foreach(const ImageInfo& it, infos)
    {
        QModelIndex index(d->imageFilterModel->indexForImageInfo(it));

        if( !index.isValid() )
        {
            toAdd.append(it);
        }
    }

    // Loading images if new images are dropped

    if (!toAdd.isEmpty())
    {
        loadImageInfos(toAdd, toAdd.first(), QString());
    }
}

void ImageWindow::slotFileOriginChanged(const QString& filePath)
{
    // By redo or undo, we have virtually switched to a new image.
    // So we do _not_ load anything!
    ImageInfo newCurrent = ImageInfo::fromLocalFile(filePath);

    if (newCurrent.isNull() || !d->imageInfoModel->hasImage(newCurrent))
    {
        return;
    }

    d->currentImageInfo = newCurrent;
    d->setThumbBarToCurrent();
    setViewToURL(d->currentUrl());
}

void ImageWindow::loadIndex(const QModelIndex& index)
{
    if (!promptUserSave(d->currentUrl(), AskIfNeeded))
    {
        return;
    }

    if (!index.isValid())
    {
        return;
    }

    d->currentImageInfo = d->imageFilterModel->imageInfo(index);
    slotLoadCurrent();
}

void ImageWindow::slotForward()
{
    loadIndex(d->nextIndex());
}

void ImageWindow::slotBackward()
{
    loadIndex(d->previousIndex());
}

void ImageWindow::slotFirst()
{
    loadIndex(d->firstIndex());
}

void ImageWindow::slotLast()
{
    loadIndex(d->lastIndex());
}

void ImageWindow::slotContextMenu()
{
    if (m_contextMenu)
    {
        m_contextMenu->addSeparator();
        addServicesMenu();
        m_contextMenu->addSeparator();

        TagsPopupMenu* assignTagsMenu = 0;
        TagsPopupMenu* removeTagsMenu = 0;

        // Bulk assignment/removal of tags --------------------------

        QList<qlonglong> idList;
        idList << d->currentImageInfo.id();

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

void ImageWindow::slotChanged()
{
    QString mpixels;
    QSize dims(m_canvas->imageWidth(), m_canvas->imageHeight());
    mpixels.setNum(dims.width()*dims.height() / 1000000.0, 'f', 2);
    QString str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)",
                                                             dims.width(), dims.height(), mpixels);

    m_resLabel->setAdjustedText(str);

    if (!d->currentIsValid())
    {
        return;
    }

    DImg* const img           = m_canvas->interface()->getImg();
    DImageHistory history     = m_canvas->interface()->getImageHistory();
    DImageHistory redoHistory = m_canvas->interface()->getImageHistoryOfFullRedo();

    d->rightSideBar->itemChanged(d->currentImageInfo, m_canvas->getSelectedArea(), img, redoHistory);

    // Filters for redo will be greyed out
    d->rightSideBar->getFiltersHistoryTab()->setEnabledHistorySteps(history.actionCount());

/*
    if (!d->currentImageInfo.isNull())
    {
    }
    else
    {
        d->rightSideBar->itemChanged(d->currentUrl(), m_canvas->getSelectedArea(), img);
    }
*/
}

void ImageWindow::slotToggleTag(const QUrl& url, int tagID)
{
    toggleTag(ImageInfo::fromUrl(url), tagID);
}

void ImageWindow::toggleTag(int tagID)
{
    toggleTag(d->currentImageInfo, tagID);
}

void ImageWindow::toggleTag(const ImageInfo& info, int tagID)
{
    if (!info.isNull())
    {
        if (info.tagIds().contains(tagID))
        {
            FileActionMngr::instance()->removeTag(info, tagID);
        }
        else
        {
            FileActionMngr::instance()->assignTag(info, tagID);
        }
    }
}

void ImageWindow::slotAssignTag(int tagID)
{
    if (!d->currentImageInfo.isNull())
    {
        FileActionMngr::instance()->assignTag(d->currentImageInfo, tagID);
    }
}

void ImageWindow::slotRemoveTag(int tagID)
{
    if (!d->currentImageInfo.isNull())
    {
        FileActionMngr::instance()->removeTag(d->currentImageInfo, tagID);
    }
}

void ImageWindow::slotAssignPickLabel(int pickId)
{
    assignPickLabel(d->currentImageInfo, pickId);
}

void ImageWindow::slotAssignColorLabel(int colorId)
{
    assignColorLabel(d->currentImageInfo, colorId);
}

void ImageWindow::assignPickLabel(const ImageInfo& info, int pickId)
{
    if (!info.isNull())
    {
        FileActionMngr::instance()->assignPickLabel(info, pickId);
    }
}

void ImageWindow::assignColorLabel(const ImageInfo& info, int colorId)
{
    if (!info.isNull())
    {
        FileActionMngr::instance()->assignColorLabel(info, colorId);
    }
}

void ImageWindow::slotAssignRating(int rating)
{
    assignRating(d->currentImageInfo, rating);
}

void ImageWindow::assignRating(const ImageInfo& info, int rating)
{
    rating = qMin(RatingMax, qMax(RatingMin, rating));

    if (!info.isNull())
    {
        FileActionMngr::instance()->assignRating(info, rating);
    }
}

void ImageWindow::slotRatingChanged(const QUrl& url, int rating)
{
    assignRating(ImageInfo::fromUrl(url), rating);
}

void ImageWindow::slotColorLabelChanged(const QUrl& url, int color)
{
    assignColorLabel(ImageInfo::fromUrl(url), color);
}

void ImageWindow::slotPickLabelChanged(const QUrl& url, int pick)
{
    assignPickLabel(ImageInfo::fromUrl(url), pick);
}

void ImageWindow::slotUpdateItemInfo()
{
    QString text = i18nc("<Image file name> (<Image number> of <Images in album>)",
                         "%1 (%2 of %3)", d->currentImageInfo.name(),
                         QString::number(d->currentIndex().row() + 1),
                         QString::number(d->imageFilterModel->rowCount()));
    m_nameLabel->setText(text);

    if (!m_actionEnabledState)
    {
        return;
    }

    if (d->imageInfoModel->rowCount() == 1)
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

    if (d->currentIndex() == d->firstIndex())
    {
        m_backwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
    }

    if (d->currentIndex() == d->lastIndex())
    {
        m_forwardAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }

/*
    // Disable some menu actions if the current root image URL
    // is not include in the digiKam Albums library database.
    // This is necessary when ImageEditor is opened from cameraclient.

    QUrl u(d->currentUrl().directory());
    PAlbum* palbum = AlbumManager::instance()->findPAlbum(u);

    if (!palbum)
    {
        m_fileDeleteAction->setEnabled(false);
    }
    else
    {
        m_fileDeleteAction->setEnabled(true);
    }
*/
}

void ImageWindow::slotSetup()
{
    Setup::execDialog(this);
}

void ImageWindow::slotSetupICC()
{
    Setup::execSinglePage(this, Setup::ICCPage);
}

void ImageWindow::slotToMainWindow()
{
    close();
}

void ImageWindow::saveIsComplete()
{
    // With save(), we do not reload the image but just continue using the data.
    // This means that a saving operation does not lead to quality loss for
    // subsequent editing operations.

    // put image in cache, the LoadingCacheInterface cares for the details
    LoadingCacheInterface::putImage(m_savingContext.destinationURL.toLocalFile(), m_canvas->currentImage());
    ImageInfo info = ScanController::instance()->scannedInfo(m_savingContext.destinationURL.toLocalFile());

    // Save new face tags to the image
    saveFaceTagsToImage(info);

    // reset the orientation flag in the database
    DMetadata meta(m_canvas->currentImage().getMetadata());
    d->currentImageInfo.setOrientation(meta.getImageOrientation());

    // Pop-up a message to bring user when save is done.
    DNotificationWrapper(QLatin1String("editorsavefilecompleted"), i18n("Image saved successfully"),
                         this, windowTitle());

    resetOrigin();

    QModelIndex next = d->nextIndex();

    if (next.isValid())
    {
        m_canvas->preload(d->imageInfo(next).filePath());
    }

    slotUpdateItemInfo();
    setViewToURL(d->currentImageInfo.fileUrl());
}

void ImageWindow::saveVersionIsComplete()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "save version done";
    saveAsIsComplete();
}

void ImageWindow::saveAsIsComplete()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Saved" << m_savingContext.srcURL << "to" << m_savingContext.destinationURL;

    // Nothing to be done if operating without database
    if (d->currentImageInfo.isNull())
    {
        return;
    }

    if (CollectionManager::instance()->albumRootPath(m_savingContext.srcURL).isNull() ||
        CollectionManager::instance()->albumRootPath(m_savingContext.destinationURL).isNull())
    {
        // not in-collection operation - nothing to do
        return;
    }

    // copy the metadata of the original file to the new file
    qCDebug(DIGIKAM_GENERAL_LOG) << "was versioned"
             << (m_savingContext.executedOperation == SavingContext::SavingStateVersion)
             << "current" << d->currentImageInfo.id() << d->currentImageInfo.name()
             << "destinations" << m_savingContext.versionFileOperation.allFilePaths();

    ImageInfo sourceInfo = d->currentImageInfo;
    // Set new current index. Employ synchronous scanning for this main file.
    d->currentImageInfo = ScanController::instance()->scannedInfo(m_savingContext.destinationURL.toLocalFile());

    if (m_savingContext.destinationExisted)
    {
        // reset the orientation flag in the database
        DMetadata meta(m_canvas->currentImage().getMetadata());
        d->currentImageInfo.setOrientation(meta.getImageOrientation());
    }

    QStringList derivedFilePaths;

    if (m_savingContext.executedOperation == SavingContext::SavingStateVersion)
    {
        derivedFilePaths = m_savingContext.versionFileOperation.allFilePaths();
    }
    else
    {
        derivedFilePaths << m_savingContext.destinationURL.toLocalFile();
    }

    // Will ensure files are scanned, and then copy attributes in a thread
    FileActionMngr::instance()->copyAttributes(sourceInfo, derivedFilePaths);

    // The model updates asynchronously, so we need to force addition of the main entry
    d->ensureModelContains(d->currentImageInfo);

    // Save new face tags to the image
    saveFaceTagsToImage(d->currentImageInfo);

    // set origin of EditorCore: "As if" the last saved image was loaded directly
    resetOriginSwitchFile();

    // If the DImg is put in the cache under the new name, this means the new file will not be reloaded.
    // This may irritate users who want to check for quality loss in lossy formats.
    // In any case, only do that if the format did not change - too many assumptions otherwise (see bug #138949).
    if (m_savingContext.originalFormat == m_savingContext.format)
    {
        LoadingCacheInterface::putImage(m_savingContext.destinationURL.toLocalFile(), m_canvas->currentImage());
    }

    // all that is done in slotLoadCurrent, except for loading

    d->thumbBar->setCurrentIndex(d->currentIndex());

    QModelIndex next = d->nextIndex();

    if (next.isValid())
    {
        m_canvas->preload(d->imageInfo(next).filePath());
    }

    setViewToURL(d->currentImageInfo.fileUrl());

    slotUpdateItemInfo();

    // Pop-up a message to bring user when save is done.
    DNotificationWrapper(QLatin1String("editorsavefilecompleted"), i18n("Image saved successfully"),
                         this, windowTitle());
}

void ImageWindow::prepareImageToSave()
{
    if (!d->currentImageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->currentImageInfo);

        // Get face tags
        d->newFaceTags.clear();
        QMultiMap<QString, QVariant> faceTags;
        faceTags = hub.loadIntegerFaceTags(d->currentImageInfo);

        if (!faceTags.isEmpty())
        {
            QSize tempS = d->currentImageInfo.dimensions();
            QMap<QString, QVariant>::const_iterator it;

            for (it = faceTags.constBegin() ; it != faceTags.constEnd() ; it++)
            {
                // Start transform each face rect
                QRect faceRect = it.value().toRect();
                int   tempH    = tempS.height();
                int   tempW    = tempS.width();

                qCDebug(DIGIKAM_GENERAL_LOG) << ">>>>>>>>>face rect before:"
                                             << faceRect.x()     << faceRect.y()
                                             << faceRect.width() << faceRect.height();

                for (int i = 0 ; i < m_transformQue.size() ; i++)
                {
                    EditorWindow::TransformType type = m_transformQue[i];

                    switch (type)
                    {
                        case EditorWindow::TransformType::RotateLeft:
                            faceRect = TagRegion::ajustToRotatedImg(faceRect, QSize(tempW, tempH), 1);
                            std::swap(tempH, tempW);
                            break;
                        case EditorWindow::TransformType::RotateRight:
                            faceRect = TagRegion::ajustToRotatedImg(faceRect, QSize(tempW, tempH), 0);
                            std::swap(tempH, tempW);
                            break;
                        case EditorWindow::TransformType::FlipHorizontal:
                            faceRect = TagRegion::ajustToFlippedImg(faceRect, QSize(tempW, tempH), 0);
                            break;
                        case EditorWindow::TransformType::FlipVertical:
                            faceRect = TagRegion::ajustToFlippedImg(faceRect, QSize(tempW, tempH), 1);
                            break;
                        default:
                            break;
                    }

                    qCDebug(DIGIKAM_GENERAL_LOG) << ">>>>>>>>>face rect transform:"
                                                 << faceRect.x()     << faceRect.y()
                                                 << faceRect.width() << faceRect.height();
                }

                d->newFaceTags.insertMulti(it.key(), QVariant(faceRect));
            }
        }

        // Ensure there is a UUID for the source image in the database,
        // even if not in the source image's metadata
        if (d->currentImageInfo.uuid().isNull())
        {
            QString uuid = m_canvas->interface()->ensureHasCurrentUuid();
            d->currentImageInfo.setUuid(uuid);
        }
        else
        {
            m_canvas->interface()->provideCurrentUuid(d->currentImageInfo.uuid());
        }
    }
}

void ImageWindow::saveFaceTagsToImage(const ImageInfo& info)
{
    if (!info.isNull() && !d->newFaceTags.isEmpty())
    {
        // Delete all old faces
        FaceTagsEditor().removeAllFaces(info.id());

        QMap<QString, QVariant>::const_iterator it;

        for (it = d->newFaceTags.constBegin() ; it != d->newFaceTags.constEnd() ; it++)
        {
            int tagId = FaceTags::getOrCreateTagForPerson(it.key());

            if (tagId)
            {
                TagRegion region(it.value().toRect());
                FaceTagsEditor().add(info.id(), tagId, region, false);
            }
            else
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to create a person tag for name" << it.key();
            }
        }

        MetadataHub hub;
        hub.load(info);
        QSize tempS = info.dimensions();
        hub.setFaceTags(d->newFaceTags, tempS);
        hub.write(info.filePath(), MetadataHub::WRITE_ALL);
    }

    m_transformQue.clear();
    d->newFaceTags.clear();
}

VersionManager* ImageWindow::versionManager() const
{
    return &d->versionManager;
}

bool ImageWindow::save()
{
    prepareImageToSave();
    startingSave(d->currentUrl());
    return true;
}

bool ImageWindow::saveAs()
{
    prepareImageToSave();
    return startingSaveAs(d->currentUrl());
}

bool ImageWindow::saveNewVersion()
{
    prepareImageToSave();
    return startingSaveNewVersion(d->currentUrl());
}

bool ImageWindow::saveCurrentVersion()
{
    prepareImageToSave();
    return startingSaveCurrentVersion(d->currentUrl());
}

bool ImageWindow::saveNewVersionAs()
{
    prepareImageToSave();
    return startingSaveNewVersionAs(d->currentUrl());
}

bool ImageWindow::saveNewVersionInFormat(const QString& format)
{
    prepareImageToSave();
    return startingSaveNewVersionInFormat(d->currentUrl(), format);
}

QUrl ImageWindow::saveDestinationUrl()
{
    return d->currentUrl();
}

void ImageWindow::slotDeleteCurrentItem()
{
    deleteCurrentItem(true, false);
}

void ImageWindow::slotDeleteCurrentItemPermanently()
{
    deleteCurrentItem(true, true);
}

void ImageWindow::slotDeleteCurrentItemPermanentlyDirectly()
{
    deleteCurrentItem(false, true);
}

void ImageWindow::slotTrashCurrentItemDirectly()
{
    deleteCurrentItem(false, false);
}

void ImageWindow::deleteCurrentItem(bool ask, bool permanently)
{
    // This function implements all four of the above slots.
    // The meaning of permanently differs depending on the value of ask

    if (d->currentImageInfo.isNull())
    {
        return;
    }

    if (!promptUserDelete(d->currentUrl()))
    {
        return;
    }

    bool useTrash;

    if (ask)
    {
        bool preselectDeletePermanently = permanently;

        DeleteDialog dialog(this);

        QList<QUrl> urlList;
        urlList << d->currentUrl();

        if (!dialog.confirmDeleteList(urlList,
                                      DeleteDialogMode::Files,
                                      preselectDeletePermanently ?
                                      DeleteDialogMode::NoChoiceDeletePermanently : DeleteDialogMode::NoChoiceTrash))
        {
            return;
        }

        useTrash = !dialog.shouldDelete();
    }
    else
    {
        useTrash = !permanently;
    }

    DIO::del(d->currentImageInfo, useTrash);

    // bring all (sidebar) to a defined state without letting them sit on the deleted file
    emit signalNoCurrentItem();

    // We have database information, which means information will get through
    // everywhere. Just do it asynchronously.

    removeCurrent();
}

void ImageWindow::removeCurrent()
{
    if (!d->currentIsValid())
    {
        return;
    }

    if (m_canvas->interface()->undoState().hasChanges)
    {
        m_canvas->slotRestore();
    }

    d->imageInfoModel->removeImageInfo(d->currentImageInfo);

    if (d->imageInfoModel->isEmpty())
    {
        // No image in the current Album -> Quit ImageEditor...

        QMessageBox::information(this, i18n("No Image in Current Album"),
                                 i18n("There is no image to show in the current album.\n"
                                      "The image editor will be closed."));

        close();
    }
}

void ImageWindow::slotFileMetadataChanged(const QUrl& url)
{
    if (url == d->currentUrl())
    {
        m_canvas->interface()->readMetadataFromFile(url.toLocalFile());
    }
}

/*
 * Should all be done by ItemViewCategorized
 *
void ImageWindow::slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{

// ignore when closed
if (!isVisible() || !d->currentIsValid())
{
    return;
}

QModelIndex currentIndex = d->currentIndex();
if (currentIndex.row() >= start && currentIndex.row() <= end)
{
    promptUserSave(d->currentUrl(), AlwaysNewVersion, false);

    // ensure selection
    int totalToRemove = end - start + 1;
    if (d->imageFilterModel->rowCount(parent) > totalToRemove)
    {
        if (end == d->imageFilterModel->rowCount(parent) - 1)
        {
            loadIndex(d->imageFilterModel->index(start - 1, 0));    // last remaining, no next one left
        }
        else
        {
            loadIndex(d->imageFilterModel->index(end + 1, 0));    // next remaining
        }
    }
}
}*/

/*
void ImageWindow::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{

    bool needLoadCurrent = false;

    switch (changeset.operation())
    {
        case CollectionImageChangeset::Removed:

            for (int i=0; i<d->imageInfoList.size(); ++i)
            {
                if (changeset.containsImage(d->imageInfoList[i].id()))
                {
                    if (d->currentImageInfo == d->imageInfoList[i])
                    {
                        promptUserSave(d->currentUrl(), AlwaysNewVersion, false);

                        if (removeItem(i))
                        {
                            needLoadCurrent = true;
                        }
                    }
                    else
                    {
                        removeItem(i);
                    }

                    --i;
                }
            }

            break;
        case CollectionImageChangeset::RemovedAll:

            for (int i=0; i<d->imageInfoList.size(); ++i)
            {
                if (changeset.containsAlbum(d->imageInfoList[i].albumId()))
                {
                    if (d->currentImageInfo == d->imageInfoList[i])
                    {
                        promptUserSave(d->currentUrl(), AlwaysNewVersion, false);

                        if (removeItem(i))
                        {
                            needLoadCurrent = true;
                        }
                    }
                    else
                    {
                        removeItem(i);
                    }

                    --i;
                }
            }

            break;
        default:
            break;
    }

    if (needLoadCurrent)
    {
        QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
    }
}
*/

void ImageWindow::slotFilePrint()
{
    printImage(d->currentUrl());
}

void ImageWindow::slotPresentation()
{
    PresentationMngr* const mngr = new PresentationMngr(this);

    foreach(const ImageInfo& info, d->imageInfoModel->imageInfos())
    {
        mngr->addFile(info.fileUrl(), info.comment());
        qApp->processEvents();
    }

    mngr->showConfigDialog();
}

void ImageWindow::slideShow(SlideShowSettings& settings)
{
    m_cancelSlideShow   = false;
    settings.exifRotate = MetadataSettings::instance()->settings().exifRotate;

    if (!d->imageInfoModel->isEmpty())
    {
        // We have started image editor from Album GUI. we get picture comments from database.

        m_nameLabel->setProgressBarMode(StatusProgressBar::CancelProgressBarMode,
                                     i18n("Preparing slideshow. Please wait..."));

        float cnt = (float)d->imageInfoModel->rowCount();
        int     i = 0;

        foreach(const ImageInfo& info, d->imageInfoModel->imageInfos())
        {
            SlidePictureInfo pictInfo;
            pictInfo.comment    = info.comment();
            pictInfo.rating     = info.rating();
            pictInfo.colorLabel = info.colorLabel();
            pictInfo.pickLabel  = info.pickLabel();
            pictInfo.photoInfo  = info.photoInfoContainer();
            settings.pictInfoMap.insert(info.fileUrl(), pictInfo);
            settings.fileList << info.fileUrl();

            m_nameLabel->setProgressValue((int)((i++ / cnt) * 100.0));
            qApp->processEvents();
        }
    }

/*
    else
    {
        // We have started image editor from Camera GUI. we get picture comments from metadata.

        m_nameLabel->setProgressBarMode(StatusProgressBar::CancelProgressBarMode,
                                     i18n("Preparing slideshow. Please wait..."));

        cnt = (float)d->urlList.count();
        DMetadata meta;
        settings.fileList   = d->urlList;

        for (QList<QUrl>::Iterator it = d->urlList.begin() ;
             !m_cancelSlideShow && (it != d->urlList.end()) ; ++it)
        {
            SlidePictureInfo pictInfo;
            meta.load((*it).toLocalFile());
            pictInfo.comment   = meta.getImageComments()[QString("x-default")].caption;
            pictInfo.photoInfo = meta.getPhotographInformation();
            settings.pictInfoMap.insert(*it, pictInfo);

            m_nameLabel->setProgressValue((int)((i++/cnt)*100.0));
            qApp->processEvents();
        }
    }
*/

    m_nameLabel->setProgressBarMode(StatusProgressBar::TextMode, QString());

    if (!m_cancelSlideShow)
    {
        SlideShow* const slide = new SlideShow(settings);
        TagsActionMngr::defaultManager()->registerActionsToWidget(slide);

        if (settings.startWithCurrent)
        {
            slide->setCurrentItem(d->currentUrl());
        }

        connect(slide, SIGNAL(signalRatingChanged(QUrl,int)),
                this, SLOT(slotRatingChanged(QUrl,int)));

        connect(slide, SIGNAL(signalColorLabelChanged(QUrl,int)),
                this, SLOT(slotColorLabelChanged(QUrl,int)));

        connect(slide, SIGNAL(signalPickLabelChanged(QUrl,int)),
                this, SLOT(slotPickLabelChanged(QUrl,int)));

        connect(slide, SIGNAL(signalToggleTag(QUrl,int)),
                this, SLOT(slotToggleTag(QUrl,int)));

        slide->show();
    }
}

void ImageWindow::dragMoveEvent(QDragMoveEvent* e)
{
    int              albumID;
    QList<int>       albumIDs;
    QList<qlonglong> imageIDs;
    QList<QUrl>      urls;
    QList<QUrl>      kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs) ||
        DAlbumDrag::decode(e->mimeData(), urls, albumID)                    ||
        DTagListDrag::canDecode(e->mimeData()))
    {
        e->accept();
        return;
    }

    e->ignore();
}

void ImageWindow::dropEvent(QDropEvent* e)
{
    int              albumID;
    QList<int>       albumIDs;
    QList<qlonglong> imageIDs;
    QList<QUrl>      urls;
    QList<QUrl>      kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
    {
        ImageInfoList imageInfoList(imageIDs);

        if (imageInfoList.isEmpty())
        {
            e->ignore();
            return;
        }

        QString ATitle;
        AlbumManager* const man = AlbumManager::instance();
        PAlbum* const palbum    = man->findPAlbum(albumIDs.first());

        if (palbum)
        {
            ATitle = palbum->title();
        }

        loadImageInfos(imageInfoList, imageInfoList.first(),
                       i18n("Album \"%1\"", ATitle));
        e->accept();
    }
    else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
    {
        AlbumManager* const man  = AlbumManager::instance();
        QList<qlonglong> itemIDs = CoreDbAccess().db()->getItemIDsInAlbum(albumID);
        ImageInfoList imageInfoList(itemIDs);

        if (imageInfoList.isEmpty())
        {
            e->ignore();
            return;
        }

        QString ATitle;
        PAlbum* const palbum = man->findPAlbum(albumIDs.first());

        if (palbum)
        {
            ATitle = palbum->title();
        }

        loadImageInfos(imageInfoList, imageInfoList.first(),
                       i18n("Album \"%1\"", ATitle));
        e->accept();
    }
    else if (DTagListDrag::canDecode(e->mimeData()))
    {
        QList<int> tagIDs;

        if (!DTagListDrag::decode(e->mimeData(), tagIDs))
        {
            return;
        }

        AlbumManager* const man  = AlbumManager::instance();
        QList<qlonglong> itemIDs = CoreDbAccess().db()->getItemIDsInTag(tagIDs.first(), true);
        ImageInfoList imageInfoList(itemIDs);

        if (imageInfoList.isEmpty())
        {
            e->ignore();
            return;
        }

        QString ATitle;
        TAlbum* const talbum = man->findTAlbum(tagIDs.first());

        if (talbum)
        {
            ATitle = talbum->title();
        }

        loadImageInfos(imageInfoList, imageInfoList.first(),
                       i18n("Album \"%1\"", ATitle));
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void ImageWindow::slotRevert()
{
    if (!promptUserSave(d->currentUrl(), AskIfNeeded))
    {
        return;
    }

    if (m_canvas->interface()->undoState().hasChanges)
    {
        m_canvas->slotRestore();
    }
}

void ImageWindow::slotOpenOriginal()
{
    if (!hasOriginalToRestore())
    {
        return;
    }

    if (!promptUserSave(d->currentUrl(), AskIfNeeded))
    {
        return;
    }

    // this time, with mustBeAvailable = true
    DImageHistory availableResolved = ImageScanner::resolvedImageHistory(m_canvas->interface()->getImageHistory(), true);

    QList<HistoryImageId> originals = availableResolved.referredImagesOfType(HistoryImageId::Original);
    HistoryImageId originalId       = m_canvas->interface()->getImageHistory().originalReferredImage();

    if (originals.isEmpty())
    {
        //TODO: point to remote collection
        QMessageBox::warning(this, i18nc("@title", "File Not Available"),
                             i18nc("@info", "<qt>The original file (<b>%1</b>) is currently not available</qt>",
                                   originalId.m_fileName));
        return;
    }

    QList<ImageInfo> imageInfos;

    foreach(const HistoryImageId& id, originals)
    {
        QUrl url = QUrl::fromLocalFile(id.m_filePath);
        url      = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1Char('/') + (id.m_fileName));
        imageInfos << ImageInfo::fromUrl(url);
    }

    ImageScanner::sortByProximity(imageInfos, d->currentImageInfo);

    if (!imageInfos.isEmpty() && !imageInfos.first().isNull())
    {
        openImage(imageInfos.first());
    }
}

bool ImageWindow::hasOriginalToRestore()
{
    // not implemented for db-less situation, so check for ImageInfo
    return !d->currentImageInfo.isNull() && EditorWindow::hasOriginalToRestore();
}

DImageHistory ImageWindow::resolvedImageHistory(const DImageHistory& history)
{
    return ImageScanner::resolvedImageHistory(history);
}

ThumbBarDock* ImageWindow::thumbBar() const
{
    return d->thumbBarDock;
}

Sidebar* ImageWindow::rightSideBar() const
{
    return (dynamic_cast<Sidebar*>(d->rightSideBar));
}

void ImageWindow::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void ImageWindow::slotDBStat()
{
    showDigikamDatabaseStat();
}

void ImageWindow::slotAddedDropedItems(QDropEvent* e)
{
    int              albumID;
    QList<int>       albumIDs;
    QList<qlonglong> imageIDs;
    QList<QUrl>      urls, kioURLs;
    ImageInfoList    imgList;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
    {
        imgList = ImageInfoList(imageIDs);
    }
    else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
    {
        QList<qlonglong> itemIDs = CoreDbAccess().db()->getItemIDsInAlbum(albumID);

        imgList = ImageInfoList(itemIDs);
    }
    else if (DTagListDrag::canDecode(e->mimeData()))
    {
        QList<int> tagIDs;

        if (!DTagListDrag::decode(e->mimeData(), tagIDs))
        {
            return;
        }

        QList<qlonglong> itemIDs = CoreDbAccess().db()->getItemIDsInTag(tagIDs.first(), true);
        imgList = ImageInfoList(itemIDs);
    }

    e->accept();

    if (!imgList.isEmpty())
    {
        loadImageInfos(imgList, imgList.first(), QString());
    }
}

void ImageWindow::slotFileWithDefaultApplication()
{
    DFileOperations::openFilesWithDefaultApplication(QList<QUrl>() << d->currentUrl());
}

void ImageWindow::addServicesMenu()
{
    addServicesMenuForUrl(d->currentUrl());
}

void ImageWindow::slotOpenWith(QAction* action)
{
    openWith(d->currentUrl(), action);
}

void ImageWindow::slotRightSideBarActivateTitles()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->setFocusToTitlesEdit();
}

void ImageWindow::slotRightSideBarActivateComments()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->setFocusToCommentsEdit();
}

void ImageWindow::slotRightSideBarActivateAssignedTags()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->activateAssignedTagsButton();
}

void ImageWindow::slotImportFromScanner()
{
#ifdef HAVE_KSANE
    m_ksaneAction->activate(DigikamApp::instance()->scannerTargetPlace(), configGroupName());

    connect(m_ksaneAction, SIGNAL(signalImportedImage(QUrl)),
            this, SLOT(slotImportedImagefromScanner(QUrl)));
#endif
}

void ImageWindow::slotImportedImagefromScanner(const QUrl& url)
{
    ImageInfo info = ScanController::instance()->scannedInfo(url.toLocalFile());
    openImage(info);
}

void ImageWindow::slotEditGeolocation()
{
#ifdef HAVE_MARBLE
    ImageInfoList infos = d->thumbBar->allImageInfos();

    if (infos.isEmpty())
    {
        return;
    }

    TagModel* const tagModel                    = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, this);
    TagPropertiesFilterModel* const filterModel = new TagPropertiesFilterModel(this);
    filterModel->setSourceAlbumModel(tagModel);
    filterModel->sort(0);

    QPointer<GeolocationEdit> dialog = new GeolocationEdit(filterModel,
                                                           new DBInfoIface(this, d->thumbBar->allUrls()),
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

void ImageWindow::slotEditMetadata()
{
    if (d->currentImageInfo.isNull())
        return;

    QUrl url = d->currentImageInfo.fileUrl();

    QPointer<MetadataEditDialog> dialog = new MetadataEditDialog(QApplication::activeWindow(),
                                                                 QList<QUrl>() << url);
    dialog->exec();

    delete dialog;

    // Refresh Database with new metadata from file.
    CollectionScanner scanner;
    scanner.scanFile(url.toLocalFile(), CollectionScanner::Rescan);
}

void ImageWindow::slotHtmlGallery()
{
#ifdef HAVE_HTMLGALLERY
    HTMLWizard w(this, new DBInfoIface(this, d->thumbBar->allUrls()));
    w.exec();
#endif
}

void ImageWindow::slotCalendar()
{
    CalWizard w(d->thumbBar->allUrls(), this);
    w.exec();
}

void ImageWindow::slotPanorama()
{
#ifdef HAVE_PANORAMA
    PanoManager::instance()->checkBinaries();
    PanoManager::instance()->setItemsList(d->thumbBar->allUrls());
    PanoManager::instance()->run();
#endif
}

void ImageWindow::slotVideoSlideshow()
{
#ifdef HAVE_MEDIAPLAYER
    VidSlideWizard w(this, new DBInfoIface(this, d->thumbBar->allUrls()));
    w.exec();
#endif
}

void ImageWindow::slotExpoBlending()
{
    ExpoBlendingManager::instance()->checkBinaries();
    ExpoBlendingManager::instance()->setItemsList(d->thumbBar->allUrls());
    ExpoBlendingManager::instance()->run();
}

void ImageWindow::slotSendByMail()
{
    MailWizard w(this, new DBInfoIface(this, d->thumbBar->allUrls()));
    w.exec();
}

void ImageWindow::slotPrintCreator()
{
    AdvPrintWizard w(this, new DBInfoIface(this, d->thumbBar->allUrls()));
    w.exec();
}

void ImageWindow::slotMediaServer()
{
    DBInfoIface* const iface = new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools);
    // NOTE: We overwrite the default albums chooser object name for load save check items state between sessions.
    // The goal is not mix these settings with other export tools.
    iface->setObjectName(QLatin1String("SetupMediaServerIface"));

    DMediaServerDlg w(this, iface);
    w.exec();
}

void ImageWindow::slotExportTool()
{
    QAction* const tool = dynamic_cast<QAction*>(sender());

    if (tool == m_exportDropboxAction)
    {
        DBWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                   ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportFacebookAction)
    {
        FbWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                   ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportFlickrAction)
    {
        FlickrWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                       ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportGdriveAction)
    {
        GSWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                   ApplicationSettings::ImportExport),
                   this, QLatin1String("googledriveexport"));
        w.exec();
    }
    else if (tool == m_exportGphotoAction)
    {
        GSWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                   ApplicationSettings::ImportExport),
                   this, QLatin1String("googlephotoexport"));
        w.exec();
    }
    else if (tool == m_exportImageshackAction)
    {
        ImageShackWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                           ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportImgurAction)
    {
        ImgurWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                      ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportPiwigoAction)
    {
        PiwigoWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                       ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportRajceAction)
    {
        RajceWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                      ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportSmugmugAction)
    {
        SmugWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                     ApplicationSettings::ImportExport), this);
        w.exec();
    }
    else if (tool == m_exportYandexfotkiAction)
    {
        YFWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                   ApplicationSettings::ImportExport), this);
        w.exec();
    }

#ifdef HAVE_MEDIAWIKI
    else if (tool == m_exportMediawikiAction)
    {
        MediaWikiWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                          ApplicationSettings::ImportExport), this);
        w.exec();
    }
#endif

#ifdef HAVE_VKONTAKTE
    else if (tool == m_exportVkontakteAction)
    {
        VKWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                   ApplicationSettings::ImportExport), this);
        w.exec();
    }
#endif

#ifdef HAVE_KIO
    else if (tool == m_exportFileTransferAction)
    {
        FTExportWindow w(new DBInfoIface(this, d->thumbBar->allUrls(),
                                         ApplicationSettings::ImportExport), this);
        w.exec();
    }
#endif
}

void ImageWindow::slotImportTool()
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
        FTImportWindow w(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport),
                         this);
        w.exec();
    }
#endif
}

} // namespace Digikam
