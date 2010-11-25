/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QPixmap>
#include <QProgressBar>
#include <QSplitter>
#include <QTimer>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdeversion.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kimageio.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
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
#include "albumsettings.h"
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
#include "dimginterface.h"
#include "dimagehistory.h"
#include "dio.h"
#include "dmetadata.h"
#include "dpopupmenu.h"
#include "editorstackview.h"
#include "globals.h"
#include "iccsettingscontainer.h"
#include "imageattributeswatch.h"
#include "imageinfo.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imagepreviewbar.h"
#include "imagepropertiessidebardb.h"
#include "imagepropertiesversionstab.h"
#include "imagescanner.h"
#include "iofilesettingscontainer.h"
#include "knotificationwrapper.h"
#include "loadingcacheinterface.h"
#include "metadatahub.h"
#include "metadatasettings.h"
#include "ratingpopupmenu.h"
#include "savingcontextcontainer.h"
#include "scancontroller.h"
#include "setup.h"
#include "slideshow.h"
#include "statusprogressbar.h"
#include "syncjob.h"
#include "tagscache.h"
#include "tagspopupmenu.h"
#include "themeengine.h"
#include "thumbbardock.h"
#include "uifilevalidator.h"
#include "versionmanager.h"

namespace Digikam
{

class DatabaseVersionManager : public VersionManager
{
public:
    virtual QString toplevelDirectory(const QString& path)
    {
        CollectionLocation loc = CollectionManager::instance()->locationForPath(path);
        if (!loc.isNull())
            return loc.albumRootPath();
        return "/";
    }
};

class ImageWindowPriv
{

public:

    ImageWindowPriv() :
        configShowThumbbarEntry("Show Thumbbar"),
        configHorizontalThumbbarEntry("HorizontalThumbbar"),
        allowSaving(true),
        toMainWindowAction(0),
        star0(0),
        star1(0),
        star2(0),
        star3(0),
        star4(0),
        star5(0),
        fileDeletePermanentlyAction(0),
        fileDeletePermanentlyDirectlyAction(0),
        fileTrashDirectlyAction(0),
        dbStatAction(0),
        thumbBar(0),
        thumbBarDock(0),
        rightSideBar(0)
    {
    }

    QString                   configShowThumbbarEntry;
    QString                   configHorizontalThumbbarEntry;

    // If image editor is launched by camera interface, current
    // image cannot be saved.
    bool                      allowSaving;

    KUrl::List                urlList;
    KUrl                      urlCurrent;

    KAction                  *toMainWindowAction;

    // Rating actions.
    KAction*                  star0;
    KAction*                  star1;
    KAction*                  star2;
    KAction*                  star3;
    KAction*                  star4;
    KAction*                  star5;

    // Delete actions
    KAction*                  fileDeletePermanentlyAction;
    KAction*                  fileDeletePermanentlyDirectlyAction;
    KAction*                  fileTrashDirectlyAction;

    KAction*                  dbStatAction;

    ImageInfoList             imageInfoList;
    ImageInfo                 imageInfoCurrent;

    ImagePreviewBar*          thumbBar;
    ThumbBarDock*             thumbBarDock;

    ImagePropertiesSideBarDB* rightSideBar;

    DatabaseVersionManager    versionManager;
};

ImageWindow* ImageWindow::m_instance = 0;

ImageWindow* ImageWindow::imagewindow()
{
    if (!m_instance)
        new ImageWindow();

    return m_instance;
}

bool ImageWindow::imagewindowCreated()
{
    return m_instance;
}

ImageWindow::ImageWindow()
           : EditorWindow("Image Editor"), d(new ImageWindowPriv)
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

    // Load image plugins to GUI

    m_imagePluginLoader = ImagePluginLoader::instance();
    loadImagePlugins();

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

    //-------------------------------------------------------------

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
        return false;

    return promptUserSave(d->urlCurrent);
}

void ImageWindow::setupConnections()
{
    setupStandardConnections();

    // To toggle properly keyboards shortcuts from comments & tags side bar tab.

    connect(d->rightSideBar, SIGNAL(signalNextItem()),
            this, SLOT(slotForward()));

    connect(d->rightSideBar, SIGNAL(signalPrevItem()),
            this, SLOT(slotBackward()));

    connect(this, SIGNAL(signalSelectionChanged( const QRect &)),
            d->rightSideBar, SLOT(slotImageSelectionChanged( const QRect &)));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSideBar, SLOT(slotNoCurrentItem()));

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalFileMetadataChanged(const KUrl &)),
            this, SLOT(slotFileMetadataChanged(const KUrl &)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(collectionImageChange(const CollectionImageChangeset &)),
            this, SLOT(slotCollectionImageChange(const CollectionImageChangeset &)),
            Qt::QueuedConnection);

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(d->thumbBar, SIGNAL(signalUrlSelected(const KUrl&)),
            this, SLOT(slotThumbBarItemSelected(const KUrl&)));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));
}

void ImageWindow::setupUserArea()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(EditorWindow::CONFIG_GROUP_NAME);

    QWidget* widget = new QWidget(this);
    QHBoxLayout *hlay = new QHBoxLayout(widget);
    m_splitter        = new SidebarSplitter(widget);

    KMainWindow* viewContainer = new KMainWindow(widget, Qt::Widget);
    m_splitter->addWidget(viewContainer);
    m_stackView = new Digikam::EditorStackView(viewContainer);
    m_canvas    = new Digikam::Canvas(m_stackView);
    viewContainer->setCentralWidget(m_stackView);

    m_splitter->setStretchFactor(0, 10);      // set Canvas default size to max.

    d->rightSideBar = new ImagePropertiesSideBarDB(widget, m_splitter, KMultiTabBar::Right, true);
    d->rightSideBar->setObjectName("ImageEditor Right Sidebar");

    hlay->addWidget(m_splitter);
    hlay->addWidget(d->rightSideBar);
    hlay->setSpacing(0);
    hlay->setMargin(0);

    m_canvas->makeDefaultEditingCanvas();
    m_stackView->setCanvas(m_canvas);
    m_stackView->setViewMode(EditorStackView::CanvasMode);

    m_splitter->setFrameStyle( QFrame::NoFrame );
    m_splitter->setFrameShadow( QFrame::Plain );
    m_splitter->setFrameShape( QFrame::NoFrame );
    m_splitter->setOpaqueResize(false);

    // Code to check for the now depreciated HorizontalThumbar directive. It
    // is found, it is honored and deleted. The state will from than on be saved
    // by viewContainers built-in mechanism.
    Qt::DockWidgetArea dockArea    = Qt::LeftDockWidgetArea;
    Qt::Orientation    orientation = Qt::Vertical;
    if (group.hasKey(d->configHorizontalThumbbarEntry))
    {
        if (group.readEntry(d->configHorizontalThumbbarEntry, true))
        {
            // Horizontal thumbbar layout
            dockArea    = Qt::TopDockWidgetArea;
            orientation = Qt::Horizontal;
        }
        group.deleteEntry(d->configHorizontalThumbbarEntry);
    }

    // The thumb bar is placed in a detachable/dockable widget.
    d->thumbBarDock = new ThumbBarDock(viewContainer, Qt::Tool);
    d->thumbBarDock->setObjectName("editor_thumbbar");
    d->thumbBar = new ImagePreviewBar(d->thumbBarDock, orientation);
    d->thumbBarDock->setWidget(d->thumbBar);
    viewContainer->addDockWidget(dockArea, d->thumbBarDock);
    d->thumbBarDock->setFloating(false);

    viewContainer->setAutoSaveSettings("ImageViewer Thumbbar", true);
    d->thumbBarDock->reInitialize();

    setCentralWidget(widget);
}

void ImageWindow::setupActions()
{
    setupStandardActions();

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    d->toMainWindowAction = new KAction(KIcon("view-list-icons"),
                                        i18nc("@action Finish editing, close editor, back to main window", "Close Editor"), this);
    connect(d->toMainWindowAction, SIGNAL(triggered()), this, SLOT(slotToMainWindow()));
    actionCollection()->addAction("imageview_tomainwindow", d->toMainWindowAction);

    // -- Rating actions ---------------------------------------------------------------

    d->star0 = new KAction(i18n("Assign Rating \"No Stars\""), this);
    d->star0->setShortcut(KShortcut(Qt::CTRL+Qt::Key_0));
    connect(d->star0, SIGNAL(triggered()), this, SLOT(slotAssignRatingNoStar()));
    actionCollection()->addAction("imageview_ratenostar", d->star0);

    d->star1 = new KAction(i18n("Assign Rating \"One Star\""), this);
    d->star1->setShortcut(KShortcut(Qt::CTRL+Qt::Key_1));
    connect(d->star1, SIGNAL(triggered()), this, SLOT(slotAssignRatingOneStar()));
    actionCollection()->addAction("imageview_rateonestar", d->star1);

    d->star2 = new KAction(i18n("Assign Rating \"Two Stars\""), this);
    d->star2->setShortcut(KShortcut(Qt::CTRL+Qt::Key_2));
    connect(d->star2, SIGNAL(triggered()), this, SLOT(slotAssignRatingTwoStar()));
    actionCollection()->addAction("imageview_ratetwostar", d->star2);

    d->star3 = new KAction(i18n("Assign Rating \"Three Stars\""), this);
    d->star3->setShortcut(KShortcut(Qt::CTRL+Qt::Key_3));
    connect(d->star3, SIGNAL(triggered()), this, SLOT(slotAssignRatingThreeStar()));
    actionCollection()->addAction("imageview_ratethreestar", d->star3);

    d->star4 = new KAction(i18n("Assign Rating \"Four Stars\""), this);
    d->star4->setShortcut(KShortcut(Qt::CTRL+Qt::Key_4));
    connect(d->star4, SIGNAL(triggered()), this, SLOT(slotAssignRatingFourStar()));
    actionCollection()->addAction("imageview_ratefourstar", d->star4);

    d->star5 = new KAction(i18n("Assign Rating \"Five Stars\""), this);
    d->star5->setShortcut(KShortcut(Qt::CTRL+Qt::Key_5));
    connect(d->star5, SIGNAL(triggered()), this, SLOT(slotAssignRatingFiveStar()));
    actionCollection()->addAction("imageview_ratefivestar", d->star5);

    // -- Special Delete actions ---------------------------------------------------------------

    // Pop up dialog to ask user whether to permanently delete

    d->fileDeletePermanentlyAction = new KAction(KIcon("edit-delete"), i18n("Delete File Permanently"), this);
    d->fileDeletePermanentlyAction->setShortcut(KShortcut(Qt::SHIFT+Qt::Key_Delete));
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

    d->dbStatAction = new KAction(KIcon("network-server-database"), i18n("Database Statistics"), this);
    connect(d->dbStatAction, SIGNAL(triggered()), this, SLOT(slotDBStat()));
    actionCollection()->addAction("editorwindow_dbstat", d->dbStatAction);

    // ---------------------------------------------------------------------------------

    createGUI(xmlFile());
}

void ImageWindow::slotSetupChanged()
{
    applyStandardSettings();
    toggleNonDestructiveActions();

    MetadataSettingsContainer writeSettings = MetadataSettings::instance()->settings();
    m_setExifOrientationTag                 = writeSettings.exifSetOrientation;
    m_canvas->setExifOrient(writeSettings.exifRotate);
    d->versionManager.setSettings(AlbumSettings::instance()->getVersionManagerSettings());

    d->thumbBar->applySettings();
    d->rightSideBar->setStyle(AlbumSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->applySettings();
}

void ImageWindow::slotThumbBarItemSelected(const KUrl& url)
{
    if (d->urlCurrent == url)
        return;

    if (!promptUserSave(d->urlCurrent, AskIfNeeded, false))
        return;

    int index = d->urlList.indexOf(url);
    if (index == -1)
        return;

    d->urlCurrent = url;
    if (!d->imageInfoList.isEmpty())
        d->imageInfoCurrent = d->imageInfoList[index];

    m_saveAction->setEnabled(false);
    m_revertAction->setEnabled(false);
    m_undoAction->setEnabled(false);
    m_redoAction->setEnabled(false);

    QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
}

void ImageWindow::loadURL(const KUrl::List& urlList, const KUrl& urlCurrent,
                          const QString& caption, bool allowSaving)
{
    if (!promptUserSave(d->urlCurrent, AskIfNeeded))
        return;

    d->urlList          = urlList;
    d->urlCurrent       = urlCurrent;
    d->imageInfoList    = ImageInfoList();
    d->imageInfoCurrent = ImageInfo();

    loadCurrentList(caption, allowSaving);
}

void ImageWindow::loadImageInfos(const ImageInfoList& imageInfoList, const ImageInfo& imageInfoCurrent,
                                 const QString& caption, bool allowSaving)
{
    // imageInfoCurrent is contained in imageInfoList.

    // Very first thing is to check for changes, user may choose to cancel operation
    if (!promptUserSave(d->urlCurrent, AskIfNeeded))
        return;

    // take over ImageInfo list
    d->imageInfoList    = imageInfoList;
    d->imageInfoCurrent = imageInfoCurrent;

    // create URL list
    d->urlList = KUrl::List();
    d->thumbBar->clear();

    d->thumbBar->blockSignals(true);
    for (ImageInfoList::iterator it = d->imageInfoList.begin(); it != d->imageInfoList.end(); ++it)
    {
        d->urlList.append(it->fileUrl());
        ImagePreviewBarItem *item = new ImagePreviewBarItem(d->thumbBar, *it);
        if (imageInfoCurrent == *it)
            d->thumbBar->setSelectedItem(item);
    }
    d->thumbBar->blockSignals(false);

    d->urlCurrent = d->imageInfoCurrent.fileUrl();

    loadCurrentList(caption, allowSaving);
}

void ImageWindow::loadCurrentList(const QString& caption, bool allowSaving)
{
    // this method contains the code shared by loadURL and loadImageInfos

    // if window is minimized, show it
    if (isMinimized())
    {
        KWindowSystem::unminimizeWindow(winId());
    }

    if (!caption.isEmpty())
        setCaption(i18n("Image Editor - %1",caption));
    else
        setCaption(i18n("Image Editor"));

    d->allowSaving = allowSaving;

    m_saveAction->setEnabled(false);

    m_undoAction->setEnabled(false);
    m_redoAction->setEnabled(false);

    QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
}

void ImageWindow::slotLoadCurrent()
{
    int index = d->urlList.indexOf(d->urlCurrent);

    if (index != -1)
    {
        m_canvas->load(d->urlCurrent.toLocalFile(), m_IOFileSettings);

        if (++index != d->urlList.size())
            m_canvas->preload(d->urlList[index].toLocalFile());
    }

    d->thumbBar->blockSignals(true);
    d->thumbBar->setSelected(d->thumbBar->findItemByUrl(d->urlCurrent));
    d->thumbBar->blockSignals(false);

    // Do this _after_ the canvas->load(), so that the main view histogram does not load
    // a smaller version if a raw image, and after that the DImgInterface loads the full version.
    // So first let DImgInterface create its loading task, only then any external objects.
    setViewToURL(d->urlCurrent);
}

void ImageWindow::setViewToURL(const KUrl& url)
{
    emit signalURLChanged(url);
}

void ImageWindow::slotForward()
{
    if (!promptUserSave(d->urlCurrent, AskIfNeeded))
        return;

    int index = d->urlList.indexOf(d->urlCurrent);

    if (index != -1)
    {
        ++index;
        if (index != d->urlList.size())
        {
           if (!d->imageInfoList.isEmpty())
               d->imageInfoCurrent = d->imageInfoList[index];
           d->urlCurrent = d->urlList[index];
           slotLoadCurrent();
        }
    }
}

void ImageWindow::slotBackward()
{
    if (!promptUserSave(d->urlCurrent, AskIfNeeded))
        return;

    int index = d->urlList.indexOf(d->urlCurrent);

    if (index != -1)
    {
        --index;

        if (index >= 0)
        {
           if (!d->imageInfoList.isEmpty())
               d->imageInfoCurrent = d->imageInfoList[index];
           d->urlCurrent = d->urlList[index];
           slotLoadCurrent();
        }
    }
}

void ImageWindow::slotFirst()
{
    if (!promptUserSave(d->urlCurrent, AskIfNeeded))
        return;

    d->urlCurrent = d->urlList.first();
    if (!d->imageInfoList.isEmpty())
        d->imageInfoCurrent = d->imageInfoList.first();
    slotLoadCurrent();
}

void ImageWindow::slotLast()
{
    if (!promptUserSave(d->urlCurrent, AskIfNeeded))
        return;

    d->urlCurrent = d->urlList.last();
    if (!d->imageInfoList.isEmpty())
        d->imageInfoCurrent = d->imageInfoList.last();
    slotLoadCurrent();
}

void ImageWindow::slotContextMenu()
{
    if (m_contextMenu)
    {
        RatingPopupMenu *ratingMenu     = 0;
        TagsPopupMenu   *assignTagsMenu = 0;
        TagsPopupMenu   *removeTagsMenu = 0;

        if (!d->imageInfoCurrent.isNull())
        {
            // Bulk assignment/removal of tags --------------------------

            QList<qlonglong> idList;
            idList << d->imageInfoCurrent.id();

            assignTagsMenu = new TagsPopupMenu(idList, TagsPopupMenu::ASSIGN, this);
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

            if (!DatabaseAccess().db()->hasTags( idList ))
                m_contextMenu->menuAction()->setEnabled(false);

            m_contextMenu->addSeparator();

            // Assign Star Rating -------------------------------------------

            ratingMenu = new RatingPopupMenu();
            ratingMenu->menuAction()->setText(i18n("Assign Rating"));

            connect(ratingMenu, SIGNAL(signalRatingChanged(int)),
                    this, SLOT(slotAssignRating(int)));

            m_contextMenu->addMenu(ratingMenu);
        }

        m_contextMenu->exec(QCursor::pos());

        delete assignTagsMenu;
        delete removeTagsMenu;
        delete ratingMenu;
    }
}

void ImageWindow::slotChanged()
{
    QString mpixels;
    QSize dims(m_canvas->imageWidth(), m_canvas->imageHeight());
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
    QString str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)",
                  dims.width(), dims.height(), mpixels);

    m_resLabel->setText(str);

    if (d->urlCurrent.isValid())
    {
        KUrl u(d->urlCurrent.directory());

        DImg* img = m_canvas->interface()->getImg();

        if (!d->imageInfoCurrent.isNull())
        {
            DImageHistory history     = m_canvas->interface()->getImageHistory();
            DImageHistory redoHistory = m_canvas->interface()->getImageHistoryOfFullRedo();
            kDebug() << "Current history:" << history.actionCount() << "full redo:" << redoHistory.actionCount();

            d->rightSideBar->itemChanged(d->imageInfoCurrent, m_canvas->getSelectedArea(),
                                         img, redoHistory);

            // Filters for redo will be greyed out
            d->rightSideBar->getFiltersHistoryTab()->setEnabledEntries(history.actionCount());
        }
        else
        {
            d->rightSideBar->itemChanged(d->urlCurrent, m_canvas->getSelectedArea(), img);
        }
    }
}

bool ImageWindow::hasChangesToSave()
{
    return EditorWindow::hasChangesToSave() && d->allowSaving;
}

void ImageWindow::slotAssignTag(int tagID)
{
    if (!d->imageInfoCurrent.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfoCurrent);
        hub.setTag(tagID, true);
        hub.write(d->imageInfoCurrent, MetadataHub::PartialWrite);
        hub.write(d->imageInfoCurrent.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImageWindow::slotRemoveTag(int tagID)
{
    if (!d->imageInfoCurrent.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfoCurrent);
        hub.setTag(tagID, false);
        hub.write(d->imageInfoCurrent, MetadataHub::PartialWrite);
        hub.write(d->imageInfoCurrent.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImageWindow::slotAssignRatingNoStar()
{
    slotAssignRating(0);
}

void ImageWindow::slotAssignRatingOneStar()
{
    slotAssignRating(1);
}

void ImageWindow::slotAssignRatingTwoStar()
{
    slotAssignRating(2);
}

void ImageWindow::slotAssignRatingThreeStar()
{
    slotAssignRating(3);
}

void ImageWindow::slotAssignRatingFourStar()
{
    slotAssignRating(4);
}

void ImageWindow::slotAssignRatingFiveStar()
{
    slotAssignRating(5);
}

void ImageWindow::slotAssignRating(int rating)
{
    assignRating(d->imageInfoCurrent, rating);
}

void ImageWindow::assignRating(const ImageInfo& info, int rating)
{
    rating = qMin(RatingMax, qMax(RatingMin, rating));
    if (!info.isNull())
    {
        MetadataHub hub;
        hub.load(info);
        hub.setRating(rating);
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImageWindow::slotRatingChanged(const KUrl& url, int rating)
{
    assignRating(ImageInfo(url), rating);
}

void ImageWindow::slotUpdateItemInfo()
{
    int index = d->urlList.indexOf(d->urlCurrent);

    m_rotatedOrFlipped = false;

    QString text = i18nc("<Image file name> (<Image number> of <Images in album>)",
                         "%1 (%2 of %3)", d->urlCurrent.fileName(),
                                          QString::number(index+1),
                                          QString::number(d->urlList.count()));
    m_nameLabel->setText(text);

    if (d->urlList.count() == 1)
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

    if (index == 0)
    {
        m_backwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
    }

    if (index == d->urlList.count()-1)
    {
        m_forwardAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }

    // Disable some menu actions if the current root image URL
    // is not include in the digiKam Albums library database.
    // This is necessary when ImageEditor is opened from cameraclient.

    KUrl u(d->urlCurrent.directory());
    PAlbum *palbum = AlbumManager::instance()->findPAlbum(u);

    if (!palbum)
    {
        m_fileDeleteAction->setEnabled(false);
    }
    else
    {
        m_fileDeleteAction->setEnabled(true);
    }
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
    d->thumbBar->refreshThumbs(KUrl::List() << d->urlCurrent);
    ScanController::instance()->scanFileDirectly(m_savingContext.destinationURL.toLocalFile());

    // Pop-up a message to bring user when save is done.
    KNotificationWrapper("editorsavefilecompleted", i18n("save file is completed..."),
                         this, windowTitle());

    // all that is done in slotLoadCurrent, except for loading
    int index = d->urlList.indexOf(d->urlCurrent);

    if (index != -1)
    {
        if (++index != d->urlList.size())
            m_canvas->preload(d->urlList[index].toLocalFile());
    }
    setViewToURL(d->urlCurrent);
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
    if (d->imageInfoCurrent.isNull())
        return;

    if (CollectionManager::instance()->albumRootPath(m_savingContext.srcURL).isNull()
        || CollectionManager::instance()->albumRootPath(m_savingContext.destinationURL).isNull())
    {
        // not in-collection operation - nothing to do
        return;
    }

    // Now copy the metadata of the original file to the new file ------------

    ScanController::instance()->scanFileDirectlyCopyAttributes(m_savingContext.destinationURL.toLocalFile(),
                                                               d->imageInfoCurrent.id());
    ImageInfo newInfo(m_savingContext.destinationURL.toLocalFile());

    if (!d->urlList.contains(m_savingContext.destinationURL) )
    {
        // The image file did not exist in the list.
        int index = d->urlList.indexOf(m_savingContext.srcURL);
        d->urlList.insert(index, m_savingContext.destinationURL);
        d->imageInfoCurrent = newInfo;
        d->imageInfoList.insert(index, d->imageInfoCurrent);
    }
    else if (d->urlCurrent != m_savingContext.destinationURL)
    {
        for (int i=0; i<d->imageInfoList.count(); ++i)
        {
            ImageInfo info = d->imageInfoList[i];
            if (info.fileUrl() == m_savingContext.destinationURL)
            {
                d->imageInfoCurrent = newInfo;
                // setAutoDelete is true
                d->imageInfoList.replace(i, d->imageInfoCurrent);
                break;
            }
        }
    }

    d->urlCurrent = m_savingContext.destinationURL;
    m_canvas->switchToLastSaved(m_savingContext.destinationURL.toLocalFile());

    // If the DImg is put in the cache under the new name, this means the new file will not be reloaded.
    // This may irritate users who want to check for quality loss in lossy formats.
    // In any case, only do that if the format did not change - too many assumptions otherwise (see bug #138949).
    if (m_savingContext.originalFormat == m_savingContext.format)
        LoadingCacheInterface::putImage(m_savingContext.destinationURL.toLocalFile(), m_canvas->currentImage());

    // all that is done in slotLoadCurrent, except for loading
    int index = d->urlList.indexOf(d->urlCurrent);

    if (index != -1)
    {
        setViewToURL(d->urlCurrent);
        if (++index != d->urlList.count())
            m_canvas->preload(d->urlList[index].toLocalFile());
    }

    // Add the file to the list of thumbbar images if it's not there already
    ImagePreviewBarItem *foundItem = d->thumbBar->findItemByInfo(d->imageInfoCurrent);
    d->thumbBar->reloadThumb(foundItem);

    if (!foundItem)
        foundItem = new ImagePreviewBarItem(d->thumbBar, d->imageInfoCurrent);

    d->thumbBar->blockSignals(true);
    d->thumbBar->setSelected(foundItem);
    d->thumbBar->blockSignals(false);

    slotUpdateItemInfo();

    // Pop-up a message to bring user when save is done.
    KNotificationWrapper("editorsavefilecompleted", i18n("save file is completed..."),
                            this, windowTitle());
}

void ImageWindow::prepareImageToSave()
{
    if (!d->imageInfoCurrent.isNull())
    {
        // Write metadata from database to DImg
        MetadataHub hub;
        hub.load(d->imageInfoCurrent);
        DImg image(m_canvas->currentImage());
        hub.write(image, MetadataHub::FullWrite);

        // Ensure there is a UUID for the source image in the database,
        // even if not in the source image's metadata
        if (d->imageInfoCurrent.uuid().isNull())
        {
            QString uuid = m_canvas->ensureHasCurrentUuid();
            d->imageInfoCurrent.setUuid(uuid);
        }
    }
}

VersionManager *ImageWindow::versionManager()
{
    return &d->versionManager;
}

bool ImageWindow::save()
{
    prepareImageToSave();
    startingSave(d->urlCurrent);
    return true;
}

bool ImageWindow::saveAs()
{
    prepareImageToSave();
    return startingSaveAs(d->urlCurrent);
}

bool ImageWindow::saveNewVersion()
{
    prepareImageToSave();
    return startingSaveNewVersion(d->urlCurrent);
}

bool ImageWindow::saveCurrentVersion()
{
    prepareImageToSave();
    return startingSaveCurrentVersion(d->urlCurrent);
}

KUrl ImageWindow::saveDestinationUrl()
{
    return d->urlCurrent;
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

    KUrl u;
    u.setPath(d->urlCurrent.directory());
    PAlbum *palbum = AlbumManager::instance()->findPAlbum(u);

    // if available, provide a digikamalbums:// URL to KIO
    KUrl kioURL;
    if (!d->imageInfoCurrent.isNull())
        kioURL = d->imageInfoCurrent.databaseUrl();
    else
        kioURL = d->urlCurrent;
    KUrl fileURL = d->urlCurrent;

    if (!palbum)
        return;

    bool useTrash;

    if (ask)
    {
        bool preselectDeletePermanently = permanently;

        DeleteDialog dialog(this);

        KUrl::List urlList;
        urlList.append(d->urlCurrent);
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

    // bring all (sidebar) to a defined state without letting them sit on the deleted file
    emit signalNoCurrentItem();

    // trash does not like non-local URLs, put is not implemented
    if (useTrash)
        kioURL = fileURL;

    int index = d->urlList.indexOf(d->urlCurrent);

    if (d->imageInfoCurrent.isNull())
    {
        // no database information: Do it the old way

        SyncJobResult deleteResult = SyncJob::del(kioURL, useTrash);
        if (!deleteResult)
        {
            KMessageBox::error(this, deleteResult.errorString);
            return;
        }

        emit signalFileDeleted(d->urlCurrent);

    }
    else
    {
        // We have database information, which means information will get through
        // everywhere. Just do it asynchronously.

        KIO::Job *job = DIO::del(kioURL, useTrash);
        job->ui()->setWindow(this);
        job->ui()->setAutoErrorHandlingEnabled(true);
    }

    if (removeItem(index))
        QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
}

bool ImageWindow::removeItem(int index)
{
    if (index == -1 || index >= d->urlList.size())
        return false;

    KUrl url = d->urlList[index];

    // Remove item from Thumbbar.
    d->thumbBar->blockSignals(true);
    d->thumbBar->removeItem(d->thumbBar->findItemByUrl(url));
    d->thumbBar->blockSignals(false);

    // remove from internal lists
    d->urlList.removeAt(index);
    if (!d->imageInfoList.isEmpty())
        d->imageInfoList.removeAt(index);
    // Remember: index now points to the next item in the list

    if (url != d->urlCurrent)
        return true;

    if (index < d->urlList.size())
    {
        // Try to get the next image in the current Album...
        d->urlCurrent       = d->urlList[index];
        d->imageInfoCurrent = d->imageInfoList[index];
        return true;
    }
    else if (index - 1 >= 0)
    {
        // Try to get the previous image in the current Album.
        --index;
        d->urlCurrent       = d->urlList[index];
        d->imageInfoCurrent = d->imageInfoList[index];
        return true;
    }

    // No image in the current Album -> Quit ImageEditor...

    KMessageBox::information(this,
                            i18n("There is no image to show in the current album.\n"
                                    "The image editor will be closed."),
                            i18n("No Image in Current Album"));

    close();
    return false;
}

void ImageWindow::slotFileMetadataChanged(const KUrl& url)
{
    if (url == d->urlCurrent)
    {
        m_canvas->readMetadataFromFile(url.toLocalFile());
    }
}

void ImageWindow::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{
    // ignore when closed
    if (!isVisible())
        return;

    bool needLoadCurrent = false;
    switch(changeset.operation())
    {
        case CollectionImageChangeset::Removed:
            for (int i=0;i<d->imageInfoList.size();++i)
            {
                if (changeset.containsImage(d->imageInfoList[i].id()))
                {
                    if (d->imageInfoCurrent == d->imageInfoList[i])
                    {
                        promptUserSave(d->urlCurrent, AlwaysNewVersion, false);
                        if (removeItem(i))
                            needLoadCurrent = true;
                    }
                    else
                        removeItem(i);
                    --i;
                }
            }
            break;
        case CollectionImageChangeset::RemovedAll:
            for (int i=0;i<d->imageInfoList.size();++i)
            {
                if (changeset.containsAlbum(d->imageInfoList[i].albumId()))
                {
                    if (d->imageInfoCurrent == d->imageInfoList[i])
                    {
                        promptUserSave(d->urlCurrent, AlwaysNewVersion, false);
                        if (removeItem(i))
                            needLoadCurrent = true;
                    }
                    else
                        removeItem(i);
                    --i;
                }
            }
            break;
        default:
            break;
    }

    if (needLoadCurrent)
        QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
}

void ImageWindow::slotFilePrint()
{
    printImage(d->urlCurrent);
}

void ImageWindow::slideShow(bool startWithCurrent, SlideShowSettings& settings)
{
    float cnt;
    int i                = 0;
    m_cancelSlideShow    = false;
    settings.exifRotate  = MetadataSettings::instance()->settings().exifRotate;
    settings.ratingColor = ThemeEngine::instance()->textSpecialRegColor();

    if (!d->imageInfoList.isEmpty())
    {
        // We have started image editor from Album GUI. we get picture comments from database.

        m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode,
                                     i18n("Preparing slideshow. Please wait..."));

        cnt = (float)d->imageInfoList.count();

        for (ImageInfoList::iterator it = d->imageInfoList.begin();
             !m_cancelSlideShow && it != d->imageInfoList.end(); ++it)
        {
            SlidePictureInfo pictInfo;
            pictInfo.comment   = it->comment();
            pictInfo.rating    = it->rating();
            pictInfo.photoInfo = it->photoInfoContainer();
            settings.pictInfoMap.insert(it->fileUrl(), pictInfo);

            m_nameLabel->setProgressValue((int)((i++/cnt)*100.0));
            kapp->processEvents();
        }
    }
    else
    {
        // We have started image editor from Camera GUI. we get picture comments from metadata.

        m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode,
                                     i18n("Preparing slideshow. Please wait..."));

        cnt = (float)d->urlList.count();
        DMetadata meta;

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

    m_nameLabel->progressBarMode(StatusProgressBar::TextMode, QString());

    if (!m_cancelSlideShow)
    {
        settings.exifRotate = MetadataSettings::instance()->settings().exifRotate;
        settings.fileList   = d->urlList;

        SlideShow* slide = new SlideShow(settings);
        if (startWithCurrent)
            slide->setCurrent(d->urlCurrent);

        connect(slide, SIGNAL(signalRatingChanged(const KUrl&, int)),
                this, SLOT(slotRatingChanged(const KUrl&, int)));

        slide->show();
    }
}

void ImageWindow::dragMoveEvent(QDragMoveEvent* e)
{
    int        albumID;
    QList<int> albumIDs;
    QList<int> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs) ||
        DAlbumDrag::decode(e->mimeData(), urls, albumID) ||
        DTagDrag::canDecode(e->mimeData()))
    {
        e->accept();
        return;
    }

    e->ignore();
}

void ImageWindow::dropEvent(QDropEvent *e)
{
    int        albumID;
    QList<int> albumIDs;
    QList<int> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
    {
        ImageInfoList imageInfoList;

        for (QList<int>::const_iterator it = imageIDs.constBegin();
             it != imageIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);
            imageInfoList << info;
        }

        if (imageInfoList.isEmpty())
        {
            e->ignore();
            return;
        }

        QString ATitle;
        AlbumManager* man  = AlbumManager::instance();
        PAlbum* palbum     = man->findPAlbum(albumIDs.first());
        if (palbum) ATitle = palbum->title();

        TAlbum* talbum     = man->findTAlbum(albumIDs.first());
        if (talbum) ATitle = talbum->title();

        loadImageInfos(imageInfoList, imageInfoList.first(),
                       i18n("Album \"%1\"",ATitle), true);
        e->accept();
    }
    else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
    {
        AlbumManager* man        = AlbumManager::instance();
        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInAlbum(albumID);
        ImageInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = itemIDs.constBegin();
             it != itemIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);
            imageInfoList << info;
        }

        if (imageInfoList.isEmpty())
        {
            e->ignore();
            return;
        }

        QString ATitle;
        PAlbum* palbum     = man->findPAlbum(albumIDs.first());
        if (palbum) ATitle = palbum->title();

        loadImageInfos(imageInfoList, imageInfoList.first(),
                       i18n("Album \"%1\"",ATitle), true);
        e->accept();
    }
    else if (DTagDrag::canDecode(e->mimeData()))
    {
        int tagID;
        if (!DTagDrag::decode(e->mimeData(), tagID))
            return;

        AlbumManager* man        = AlbumManager::instance();
        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInTag(tagID, true);
        ImageInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = itemIDs.constBegin();
             it != itemIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);
            imageInfoList << info;
        }

        if (imageInfoList.isEmpty())
        {
            e->ignore();
            return;
        }

        QString ATitle;
        TAlbum* talbum     = man->findTAlbum(tagID);
        if (talbum) ATitle = talbum->title();

        loadImageInfos(imageInfoList, imageInfoList.first(),
                       i18n("Album \"%1\"",ATitle), true);
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void ImageWindow::slotRevert()
{
    if (!promptUserSave(d->urlCurrent, AskIfNeeded))
        return;

    if (hasChangesToSave())
    {
        m_canvas->slotRestore();
    }
}

void ImageWindow::slotOpenOriginal()
{
    if (!hasOriginalToRestore())
        return;

    if (!promptUserSave(d->urlCurrent, AskIfNeeded))
        return;

    // this time, with mustBeAvailable = true
    DImageHistory availableResolved = ImageScanner::resolvedImageHistory(m_canvas->interface()->getImageHistory(), true);

    QList<HistoryImageId> originals = availableResolved.referredImagesOfType(HistoryImageId::Original);
    HistoryImageId originalId = m_canvas->interface()->getImageHistory().originalReferredImage();

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
    foreach (const HistoryImageId& id, originals)
    {
        KUrl url;
        url.addPath(id.m_filePath);
        url.addPath(id.m_fileName);
        imageInfos << ImageInfo(url);
    }
    ImageScanner::sortByProximity(imageInfos, d->imageInfoCurrent);

    if (!imageInfos.isEmpty() && !imageInfos.first().isNull())
    {
        d->imageInfoCurrent = imageInfos.first();
        d->urlCurrent       = d->imageInfoCurrent.fileUrl();
        slotLoadCurrent();
    }
}

bool ImageWindow::hasOriginalToRestore()
{
    // not implemented for db-less situation, so check for ImageInfo
    return !d->imageInfoCurrent.isNull() && EditorWindow::hasOriginalToRestore();
}

DImageHistory ImageWindow::resolvedImageHistory(const DImageHistory& history)
{
    return ImageScanner::resolvedImageHistory(history);
}

void ImageWindow::slotChangeTheme(const QString& theme)
{
    // Theme menu entry is returned with keyboard accelerator. We remove it.
    QString name = theme;
    name.remove(QChar('&'));
    AlbumSettings::instance()->setCurrentTheme(theme);
    ThemeEngine::instance()->slotChangeTheme(theme);
}

ThumbBarDock *ImageWindow::thumbBar() const
{
    return d->thumbBarDock;
}

Sidebar *ImageWindow::rightSideBar() const
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

}  // namespace Digikam