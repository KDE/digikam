/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagewindow.moc"

// C++ includes

#include <cstdio>

// Qt includes

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

// KDE includes

#include <kaction.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcategorizedview.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdeversion.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kimageio.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kselectaction.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstdaccel.h>
#include <kstdguiitem.h>
#include <ktemporaryfile.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <ktoolbarpopupaction.h>
#include <kwindowsystem.h>
#include <kdebug.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albummodel.h"
#include "applicationsettings.h"
#include "canvas.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "componentsinfo.h"
#include "databaseaccess.h"
#include "databasewatch.h"
#include "databasechangesets.h"
#include "ddragobjects.h"
#include "deletedialog.h"
#include "dimg.h"
#include "editorcore.h"
#include "dimagehistory.h"
#include "dio.h"
#include "dmetadata.h"
#include "editorstackview.h"
#include "fileactionmngr.h"
#include "fileoperation.h"
#include "globals.h"
#include "iccsettingscontainer.h"
#include "imageattributeswatch.h"
#include "imagefiltermodel.h"
#include "imagedragdrop.h"
#include "imagedescedittab.h"
#include "imageinfo.h"
#include "imagelistmodel.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imagepropertiessidebardb.h"
#include "imagepropertiesversionstab.h"
#include "imagescanner.h"
#include "imagethumbnailbar.h"
#include "iofilesettings.h"
#include "dnotificationwrapper.h"
#include "loadingcacheinterface.h"
#include "metadatahub.h"
#include "metadatasettings.h"
#include "colorlabelwidget.h"
#include "picklabelwidget.h"
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
#include "thememanager.h"
#include "thumbbardock.h"
#include "thumbnailloadthread.h"
#include "uifilevalidator.h"
#include "undostate.h"
#include "imagewindow_p.h"

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
    : EditorWindow("Image Editor"), d(new Private)
{
    setXMLFile("digikamimagewindowui.rc");

    // --------------------------------------------------------

    UiFileValidator validator(localXMLFile());

    if (!validator.isValid())
    {
        validator.fixConfigFile();
    }

    // --------------------------------------------------------

    m_instance = this;
    // We don't want to be deleted on close
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAcceptDrops(true);

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupActions();
    setupStatusBar();
    createGUI(xmlFile());

    // Load image plugins to GUI

    m_imagePluginLoader = ImagePluginLoader::instance();
    loadImagePlugins();

    // Create tool selection view

    setupSelectToolsAction();

    // Create context menu.

    setupContextMenu();

    // Make signals/slots connections

    setupConnections();

    // -- Read settings --------------------------------

    readSettings();
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(EditorWindow::CONFIG_GROUP_NAME);
    applyMainWindowSettings(group);
    d->thumbBarDock->setShouldBeVisible(group.readEntry(d->configShowThumbbarEntry, false));
    setAutoSaveSettings(EditorWindow::CONFIG_GROUP_NAME, true);
    d->viewContainer->setAutoSaveSettings("ImageViewer Thumbbar", true);

    //-------------------------------------------------------------

    d->rightSideBar->setConfigGroup(KConfigGroup(&group, "Right Sidebar"));
    d->rightSideBar->loadState();
    d->rightSideBar->populateTags();

    slotSetupChanged();
}

ImageWindow::~ImageWindow()
{
    m_instance = 0;

    unLoadImagePlugins();

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

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(EditorWindow::CONFIG_GROUP_NAME);
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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(EditorWindow::CONFIG_GROUP_NAME);

    QWidget* const widget   = new QWidget(this);
    QHBoxLayout* const hlay = new QHBoxLayout(widget);
    m_splitter              = new SidebarSplitter(widget);

    d->viewContainer        = new KMainWindow(widget, Qt::Widget);
    m_splitter->addWidget(d->viewContainer);
    m_stackView             = new EditorStackView(d->viewContainer);
    m_canvas                = new Canvas(m_stackView);
    d->viewContainer->setCentralWidget(m_stackView);

    m_splitter->setStretchFactor(0, 10);      // set Canvas default size to max.

    d->rightSideBar = new ImagePropertiesSideBarDB(widget, m_splitter, KMultiTabBar::Right, true);
    d->rightSideBar->setObjectName("ImageEditor Right Sidebar");
    d->rightSideBar->getFiltersHistoryTab()->addOpenImageAction();

    hlay->addWidget(m_splitter);
    hlay->addWidget(d->rightSideBar);
    hlay->setSpacing(0);
    hlay->setMargin(0);

    m_canvas->makeDefaultEditingCanvas();
    m_stackView->setCanvas(m_canvas);
    m_stackView->setViewMode(EditorStackView::CanvasMode);

    m_splitter->setFrameStyle(QFrame::NoFrame);
    m_splitter->setFrameShadow(QFrame::Plain);
    m_splitter->setFrameShape(QFrame::NoFrame);
    m_splitter->setOpaqueResize(false);

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
    d->imageFilterModel->setSortRole((ImageSortSettings::SortRole)ApplicationSettings::instance()->getImageSortOrder());
    d->imageFilterModel->setSortOrder((ImageSortSettings::SortOrder)ApplicationSettings::instance()->getImageSorting());
    d->imageFilterModel->setAllGroupsOpen(true); // disable filtering out by group, see bug #283847
    d->imageFilterModel->sort(0); // an initial sorting is necessary

    d->dragDropHandler  = new ImageDragDropHandler(d->imageInfoModel);
    d->dragDropHandler->setReadOnlyDrop(true);
    d->imageInfoModel->setDragDropHandler(d->dragDropHandler);

    // The thumb bar is placed in a detachable/dockable widget.
    d->thumbBarDock     = new ThumbBarDock(d->viewContainer, Qt::Tool);
    d->thumbBarDock->setObjectName("editor_thumbbar");

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

    connect(watch, SIGNAL(signalFileMetadataChanged(KUrl)),
            this, SLOT(slotFileMetadataChanged(KUrl)));

/*
    connect(DatabaseAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
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

    d->toMainWindowAction = new KAction(KIcon("view-list-icons"),
                                        i18nc("@action Finish editing, close editor, back to main window", "Close Editor"), this);
    connect(d->toMainWindowAction, SIGNAL(triggered()), this, SLOT(slotToMainWindow()));
    actionCollection()->addAction("imageview_tomainwindow", d->toMainWindowAction);

    // -- Special Delete actions ---------------------------------------------------------------

    // Pop up dialog to ask user whether to permanently delete

    d->fileDeletePermanentlyAction = new KAction(KIcon("edit-delete"), i18n("Delete File Permanently"), this);
    d->fileDeletePermanentlyAction->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_Delete));
    connect(d->fileDeletePermanentlyAction, SIGNAL(triggered()),
            this, SLOT(slotDeleteCurrentItemPermanently()));
    actionCollection()->addAction("image_delete_permanently", d->fileDeletePermanentlyAction);

    // These two actions are hidden, no menu entry, no toolbar entry, no shortcut.
    // Power users may add them.

    d->fileDeletePermanentlyDirectlyAction = new KAction(KIcon("edit-delete"),
                                                         i18n("Delete Permanently without Confirmation"), this);
    connect(d->fileDeletePermanentlyDirectlyAction, SIGNAL(triggered()),
            this, SLOT(slotDeleteCurrentItemPermanentlyDirectly()));
    actionCollection()->addAction("image_delete_permanently_directly",
                                  d->fileDeletePermanentlyDirectlyAction);

    d->fileTrashDirectlyAction = new KAction(KIcon("user-trash"),
                                             i18n("Move to Trash without Confirmation"), this);
    connect(d->fileTrashDirectlyAction, SIGNAL(triggered()),
            this, SLOT(slotTrashCurrentItemDirectly()));
    actionCollection()->addAction("image_trash_directly", d->fileTrashDirectlyAction);

    // ---------------------------------------------------------------------------------

    createHelpActions();

    // Labels shortcuts must be registered here to be saved in XML GUI files if user customize it.
    TagsActionMngr::defaultManager()->registerLabelsActions(actionCollection());

    KAction* const editTitles = new KAction(i18n("Edit Titles"), this);
    editTitles->setShortcut( KShortcut(Qt::META + Qt::Key_T) );
    actionCollection()->addAction("edit_titles", editTitles);
    connect(editTitles, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateTitles()));

    KAction* const editComments = new KAction(i18n("Edit Comments"), this);
    editComments->setShortcut( KShortcut(Qt::META + Qt::Key_C) );
    actionCollection()->addAction("edit_comments", editComments);
    connect(editComments, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateComments()));

    KAction* const assignedTags = new KAction(i18n("Show Assigned Tags"), this);
    assignedTags->setShortcut( KShortcut(Qt::META + Qt::Key_A) );
    actionCollection()->addAction("assigned _tags", assignedTags);
    connect(assignedTags, SIGNAL(triggered()), this, SLOT(slotRightSideBarActivateAssignedTags()));
}

void ImageWindow::slotSetupChanged()
{
    applyStandardSettings();

    VersionManagerSettings versionSettings = ApplicationSettings::instance()->getVersionManagerSettings();
    d->versionManager.setSettings(versionSettings);
    m_nonDestructive                       = versionSettings.enabled;
    toggleNonDestructiveActions();

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
    // Update the sorting of images, see bug #342788
    d->imageFilterModel->setSortRole((ImageSortSettings::SortRole)ApplicationSettings::instance()->getImageSortOrder());
    d->imageFilterModel->setSortOrder((ImageSortSettings::SortOrder)ApplicationSettings::instance()->getImageSorting());
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

void ImageWindow::setViewToURL(const KUrl& url)
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

    foreach(ImageInfo it, infos)
    {
        QModelIndex index(d->imageFilterModel->indexForImageInfo(it));

        if( !index.isValid() )
        {
            toAdd.append(it);
        }
    }

    // Loading images if new images are dropped

    if(!toAdd.isEmpty())
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

        if (!DatabaseAccess().db()->hasTags(idList))
        {
            m_contextMenu->menuAction()->setEnabled(false);
        }

        m_contextMenu->addSeparator();

        // Assign Labels -------------------------------------------

        KMenu* const menuLabels           = new KMenu(i18n("Assign Labels"), m_contextMenu);
        PickLabelMenuAction* const pmenu  = new PickLabelMenuAction(m_contextMenu);
        ColorLabelMenuAction* const cmenu = new ColorLabelMenuAction(m_contextMenu);
        RatingMenuAction* const rmenu     = new RatingMenuAction(m_contextMenu);
        menuLabels->addAction(pmenu);
        menuLabels->addAction(cmenu);
        menuLabels->addAction(rmenu);
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

    m_resLabel->setText(str);

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

void ImageWindow::slotToggleTag(const KUrl& url, int tagID)
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

void ImageWindow::slotRatingChanged(const KUrl& url, int rating)
{
    assignRating(ImageInfo::fromUrl(url), rating);
}

void ImageWindow::slotColorLabelChanged(const KUrl& url, int color)
{
    assignColorLabel(ImageInfo::fromUrl(url), color);
}

void ImageWindow::slotPickLabelChanged(const KUrl& url, int pick)
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

    KUrl u(d->currentUrl().directory());
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

bool ImageWindow::setup()
{
    return Setup::exec(this);
}

bool ImageWindow::setupICC()
{
    return Setup::execSinglePage(this, Setup::ICCPage);
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
    ScanController::instance()->scannedInfo(m_savingContext.destinationURL.toLocalFile());
    // reset the orientation flag in the database
    DMetadata meta(m_canvas->currentImage().getMetadata());
    d->currentImageInfo.setOrientation(meta.getImageOrientation());

    // Pop-up a message to bring user when save is done.
    DNotificationWrapper("editorsavefilecompleted", i18n("Image saved successfully"),
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
    kDebug() << "save version done";
    saveAsIsComplete();
}

void ImageWindow::saveAsIsComplete()
{
    kDebug() << "Saved" << m_savingContext.srcURL << "to" << m_savingContext.destinationURL;

    // Nothing to be done if operating without database
    if (d->currentImageInfo.isNull())
    {
        return;
    }

    if (CollectionManager::instance()->albumRootPath(m_savingContext.srcURL).isNull()
        || CollectionManager::instance()->albumRootPath(m_savingContext.destinationURL).isNull())
    {
        // not in-collection operation - nothing to do
        return;
    }

    // copy the metadata of the original file to the new file
    kDebug() << "was versioned"
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
    DNotificationWrapper("editorsavefilecompleted", i18n("Image saved successfully"),
                         this, windowTitle());
}

void ImageWindow::prepareImageToSave()
{
    if (!d->currentImageInfo.isNull())
    {
        // Write metadata from database to DImg
        MetadataHub hub;
        hub.load(d->currentImageInfo);
        DImg image(m_canvas->currentImage());
        hub.write(image, MetadataHub::FullWrite);

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

KUrl ImageWindow::saveDestinationUrl()
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

    //PAlbum* palbum = AlbumManager::instance()->findPAlbum(d->currentImageInfo.albumId());

    bool useTrash;

    if (ask)
    {
        bool preselectDeletePermanently = permanently;

        DeleteDialog dialog(this);

        KUrl::List urlList;
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

    d->imageInfoModel->removeImageInfo(d->currentImageInfo);

    if (d->imageInfoModel->isEmpty())
    {
        // No image in the current Album -> Quit ImageEditor...

        KMessageBox::information(this,
                                 i18n("There is no image to show in the current album.\n"
                                      "The image editor will be closed."),
                                 i18n("No Image in Current Album"));

        close();
    }
}

void ImageWindow::slotFileMetadataChanged(const KUrl& url)
{
    if (url == d->currentUrl())
    {
        m_canvas->interface()->readMetadataFromFile(url.toLocalFile());
    }
}

/*
 * Should all be done by DCategorizedView
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

void ImageWindow::slideShow(SlideShowSettings& settings)
{
    m_cancelSlideShow   = false;
    settings.exifRotate = MetadataSettings::instance()->settings().exifRotate;

    if (!d->imageInfoModel->isEmpty())
    {
        // We have started image editor from Album GUI. we get picture comments from database.

        m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode,
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
            kapp->processEvents();
        }
    }

/*
    else
    {
        // We have started image editor from Camera GUI. we get picture comments from metadata.

        m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode,
                                     i18n("Preparing slideshow. Please wait..."));

        cnt = (float)d->urlList.count();
        DMetadata meta;
        settings.fileList   = d->urlList;

        for (KUrl::List::Iterator it = d->urlList.begin() ;
             !m_cancelSlideShow && (it != d->urlList.end()) ; ++it)
        {
            SlidePictureInfo pictInfo;
            meta.load((*it).toLocalFile());
            pictInfo.comment   = meta.getImageComments()[QString("x-default")].caption;
            pictInfo.photoInfo = meta.getPhotographInformation();
            settings.pictInfoMap.insert(*it, pictInfo);

            m_nameLabel->setProgressValue((int)((i++/cnt)*100.0));
            kapp->processEvents();
        }
    }
*/

    m_nameLabel->progressBarMode(StatusProgressBar::TextMode, QString());

    if (!m_cancelSlideShow)
    {
        SlideShow* const slide = new SlideShow(settings);
        TagsActionMngr::defaultManager()->registerActionsToWidget(slide);

        if (settings.startWithCurrent)
        {
            slide->setCurrentItem(d->currentUrl());
        }

        connect(slide, SIGNAL(signalRatingChanged(KUrl,int)),
                this, SLOT(slotRatingChanged(KUrl,int)));

        connect(slide, SIGNAL(signalColorLabelChanged(KUrl,int)),
                this, SLOT(slotColorLabelChanged(KUrl,int)));

        connect(slide, SIGNAL(signalPickLabelChanged(KUrl,int)),
                this, SLOT(slotPickLabelChanged(KUrl,int)));

        connect(slide, SIGNAL(signalToggleTag(KUrl,int)),
                this, SLOT(slotToggleTag(KUrl,int)));

        slide->show();
    }
}

void ImageWindow::dragMoveEvent(QDragMoveEvent* e)
{
    int        albumID;
    QList<int> albumIDs;
    QList<qlonglong> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

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
    int        albumID;
    QList<int> albumIDs;
    QList<qlonglong> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

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
        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInAlbum(albumID);
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
        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInTag(tagIDs.first(), true);
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
        KMessageBox::sorry(this,
                           i18nc("@info",
                                 "The original file (<filename>%1</filename>) is currently not available",
                                 originalId.m_fileName),
                           i18nc("@title",
                                 "File Not Available"));
        return;
    }

    QList<ImageInfo> imageInfos;

    foreach(const HistoryImageId& id, originals)
    {
        KUrl url;
        url.addPath(id.m_filePath);
        url.addPath(id.m_fileName);
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
    KUrl::List       urls, kioURLs;
    ImageInfoList    imgList;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
    {
        imgList = ImageInfoList(imageIDs);
    }
    else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
    {
        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInAlbum(albumID);

        imgList = ImageInfoList(itemIDs);
    }
    else if (DTagListDrag::canDecode(e->mimeData()))
    {
        QList<int> tagIDs;

        if (!DTagListDrag::decode(e->mimeData(), tagIDs))
        {
            return;
        }

        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInTag(tagIDs.first(), true);
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
    FileOperation::openFilesWithDefaultApplication(KUrl::List() << d->currentUrl(), this);
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

}  // namespace Digikam
