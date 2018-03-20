/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2008      by Arnd Baecker <arnd dot baecker at web dot de>
 * Copyright (C) 2013-2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

// Qt includes

#include <QCursor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QPointer>
#include <QProgressBar>
#include <QSplitter>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QStandardPaths>
#include <QStyle>
#include <QKeySequence>
#include <QApplication>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMimeDatabase>
#include <QMimeType>
#include <QMessageBox>
#include <QApplication>
#include <QMimeType>
#include <QMimeDatabase>
#include <QTemporaryFile>

// KDE includes

#include <kactioncollection.h>
#include <klocalizedstring.h>

// Local includes

#include "drawdecoder.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "canvas.h"
#include "editorcore.h"
#include "dmetadata.h"
#include "editorstackview.h"
#include "dfileoperations.h"
#include "iccsettingscontainer.h"
#include "imagedialog.h"
#include "imagepropertiessidebar.h"
#include "iofilesettings.h"
#include "loadingcache.h"
#include "loadingcacheinterface.h"
#include "metadatasettings.h"
#include "metadataedit.h"
#include "presentationmngr.h"
#include "savingcontext.h"
#include "showfotosetup.h"
#include "showfotosetupmisc.h"
#include "setupicc.h"
#include "slideshow.h"
#include "dsplashscreen.h"
#include "statusprogressbar.h"
#include "thememanager.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "dnotificationwrapper.h"
#include "showfotodelegate.h"
#include "showfotothumbnailmodel.h"
#include "showfotocategorizedview.h"
#include "showfotosettings.h"
#include "dmetainfoiface.h"
#include "showfoto_p.h"
#include "dexpanderbox.h"
#include "dfiledialog.h"
#include "calwizard.h"
#include "expoblendingmanager.h"
#include "mailwizard.h"
#include "advprintwizard.h"
#include "dmediaservermngr.h"
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

namespace ShowFoto
{

ShowFoto::ShowFoto(const QList<QUrl>& urlList)
    : Digikam::EditorWindow(QLatin1String("Showfoto")),
      d(new Private)
{
    setXMLFile(QLatin1String("showfotoui5.rc"));

    m_nonDestructive = false;

    // Show splash-screen at start up.

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    if (group.readEntry(QLatin1String("ShowSplash"), true) && !qApp->isSessionRestored())
    {
        d->splash = new Digikam::DSplashScreen();
        d->splash->show();
    }

    // Setup loading cache and thumbnails interface.

    Digikam::LoadingCacheInterface::initialize();
    Digikam::MetadataSettings::instance();

    d->thumbLoadThread = new Digikam::ThumbnailLoadThread();
    d->thumbLoadThread->setThumbnailSize(Digikam::ThumbnailSize::Huge);
    d->thumbLoadThread->setSendSurrogatePixmap(true);

    // Check ICC profiles repository availability

    if (d->splash)
    {
        d->splash->setMessage(i18n("Checking ICC repository..."));
    }

    d->validIccPath = Digikam::SetupICC::iccRepositoryIsValid();

    // Populate Themes

    if (d->splash)
    {
        d->splash->setMessage(i18n("Loading themes..."));
    }

    Digikam::ThemeManager::instance();

    // -- Build the GUI -----------------------------------

    setupUserArea();
    setupActions();
    setupStatusBar();
    createGUI(xmlFile());
    cleanupActions();

    // Create tool selection view

    setupSelectToolsAction();

    // Create context menu.

    setupContextMenu();

    // Make signals/slots connections

    setupConnections();

    // -- Read settings --------------------------------

    readSettings();
    applySettings();
    setAutoSaveSettings(configGroupName(), true);

    d->rightSideBar->loadState();

    //--------------------------------------------------

    d->thumbBarDock->reInitialize();

    // -- Load current items ---------------------------

    slotDroppedUrls(urlList);

    if (!d->infoList.isEmpty())
    {
        slotOpenUrl(d->thumbBar->currentInfo());
    }
}

ShowFoto::~ShowFoto()
{
    delete m_canvas;
    m_canvas = 0;

    Digikam::ThumbnailLoadThread::cleanUp();
    Digikam::LoadingCacheInterface::cleanUp();
    Digikam::DMediaServerMngr::instance()->saveAtShutdown();

    delete d->model;
    delete d->filterModel;
    delete d->thumbBar;
    delete d->rightSideBar;
    delete d->thumbLoadThread;
    delete d;
}

bool ShowFoto::queryClose()
{
    // wait if a save operation is currently running
    if (!waitForSavingToComplete())
    {
        return false;
    }

    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return false;
    }

    saveSettings();
    return true;
}

void ShowFoto::show()
{
    // Remove Splashscreen.

    if (d->splash)
    {
        d->splash->finish(this);
        delete d->splash;
        d->splash = 0;
    }

    // Display application window.

    QMainWindow::show();

    // Report errors from ICC repository path.

    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    if (!d->validIccPath)
    {
        QString message = i18n("<p>The ICC profile path seems to be invalid.</p>"
                               "<p>If you want to set it now, select \"Yes\", otherwise "
                               "select \"No\". In this case, \"Color Management\" feature "
                               "will be disabled until you solve this issue</p>");

        if (QMessageBox::warning(this, qApp->applicationName(), message, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            if (!setup(true))
            {
                KConfigGroup group = config->group(QLatin1String("Color Management"));
                group.writeEntry(QLatin1String("EnableCM"), false);
                config->sync();
            }
        }
        else
        {
            KConfigGroup group = config->group(QLatin1String("Color Management"));
            group.writeEntry(QLatin1String("EnableCM"), false);
            config->sync();
        }
    }
    
    // Start the Media Server if necessary
    
    Digikam::DMediaServerMngr::instance()->loadAtStartup();
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

    connect(d->dDHandler, SIGNAL(signalDroppedUrls(QList<QUrl>)),
            this, SLOT(slotDroppedUrls(QList<QUrl>)));
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

    d->rightSideBar = new Digikam::ImagePropertiesSideBar(widget, m_splitter, Qt::RightEdge);
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

void ShowFoto::setupActions()
{
    Digikam::ThemeManager::instance()->setThemeMenuAction(new QMenu(i18n("&Themes"), this));
    setupStandardActions();

    // Extra 'File' menu actions ---------------------------------------------

    d->fileOpenAction = buildStdAction(StdOpenAction, this, SLOT(slotOpenFile()), this);
    actionCollection()->addAction(QLatin1String("showfoto_open_file"), d->fileOpenAction);

    // ---------

    d->openFilesInFolderAction = new QAction(QIcon::fromTheme(QLatin1String("folder-pictures")), i18n("Open folder"), this);
    actionCollection()->setDefaultShortcut(d->openFilesInFolderAction, Qt::CTRL+Qt::SHIFT+Qt::Key_O);

    connect(d->openFilesInFolderAction, &QAction::triggered,
            this, &ShowFoto::slotOpenFilesInFolder);

    actionCollection()->addAction(QLatin1String("showfoto_open_folder"), d->openFilesInFolderAction);

    // ---------

    QAction* const quit = buildStdAction(StdQuitAction, this, SLOT(close()), this);
    actionCollection()->addAction(QLatin1String("showfoto_quit"), quit);

    // -- Standard 'Help' menu actions ---------------------------------------------

    createHelpActions(false);
}

void ShowFoto::readSettings()
{
    d->settings        = ShowfotoSettings::instance();

    readStandardSettings();

    QString defaultDir = d->settings->getLastOpenedDir();

    if (defaultDir.isNull())
    {
        defaultDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }

    d->lastOpenedDirectory = QUrl::fromLocalFile(defaultDir);

    d->rightSideBar->loadState();

    Digikam::ThemeManager::instance()->setCurrentTheme(d->settings->getCurrentTheme());

    d->thumbBar->setToolTipEnabled(d->settings->getShowToolTip());
}

void ShowFoto::saveSettings()
{
    saveStandardSettings();

    d->settings->setLastOpenedDir(d->lastOpenedDirectory.toLocalFile());
    d->settings->setCurrentTheme(Digikam::ThemeManager::instance()->currentThemeName());
    d->settings->syncConfig();

    d->rightSideBar->saveState();
}

void ShowFoto::applySettings()
{
    applyStandardSettings();

    d->settings->readSettings();

    d->rightSideBar->setStyle(d->settings->getRightSideBarStyle() == 0 ?
                              DMultiTabBar::ActiveIconText : DMultiTabBar::AllIconsText);

    QString currentStyle = qApp->style()->objectName();
    QString newStyle     = d->settings->getApplicationStyle();

    if (currentStyle.compare(newStyle, Qt::CaseInsensitive) != 0)
    {
        qApp->setStyle(newStyle);
        qApp->style()->polish(qApp);
        qCDebug(DIGIKAM_SHOWFOTO_LOG) << "Switch to widget style: " << newStyle;
    }

    m_fileDeleteAction->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    m_fileDeleteAction->setText(i18n("Delete File"));

    d->thumbBar->setToolTipEnabled(d->settings->getShowToolTip());

    d->rightSideBar->slotLoadMetadataFilters();

    // Determine sort ordering for the entries from the Showfoto settings:

    if (d->settings->getReverseSort())
    {
        d->filterModel->setSortOrder(ShowfotoItemSortSettings::DescendingOrder);
    }
    else
    {
        d->filterModel->setSortOrder(ShowfotoItemSortSettings::AscendingOrder);
    }

    switch (d->settings->getSortRole())
    {
        case SetupMisc::SortByName:
        {
            d->filterModel->setSortRole(ShowfotoItemSortSettings::SortByFileName);
            break;
        }
        case SetupMisc::SortByFileSize:
        {
            d->filterModel->setSortRole(ShowfotoItemSortSettings::SortByFileSize);
            break;
        }
        default:
        {
            d->filterModel->setSortRole(ShowfotoItemSortSettings::SortByCreationDate);
            break;
        }
    }
}

void ShowFoto::slotOpenFile()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    QList<QUrl> urls = Digikam::ImageDialog::getImageURLs(this, d->lastOpenedDirectory);
    openUrls(urls);
}

void ShowFoto::openUrls(const QList<QUrl> &urls)
{
    if (!urls.isEmpty())
    {
        ShowfotoItemInfoList infos;
        ShowfotoItemInfo iteminfo;
        DMetadata meta;

        for (QList<QUrl>::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
        {
            QFileInfo fi((*it).toLocalFile());
            iteminfo.name      = fi.fileName();
            iteminfo.mime      = fi.suffix();
            iteminfo.size      = fi.size();
            iteminfo.url       = QUrl::fromLocalFile(fi.filePath());
            iteminfo.folder    = fi.path();
            iteminfo.dtime     = fi.created();
            meta.load(fi.filePath());
            iteminfo.ctime     = meta.getImageDateTime();
            iteminfo.width     = meta.getImageDimensions().width();
            iteminfo.height    = meta.getImageDimensions().height();
            iteminfo.photoInfo = meta.getPhotographInformation();
            infos.append(iteminfo);
        }

        if (d->droppedUrls)
        {
            //replace the equal sign with "<<" to keep the previous pics in the list
            d->infoList << infos;
        }
        else
        {
            d->infoList = infos;
            d->model->clearShowfotoItemInfos();
            emit signalInfoList(d->infoList);
            slotOpenUrl(d->thumbBar->currentInfo());
        }
    }
}

void ShowFoto::slotOpenUrl(const ShowfotoItemInfo& info)
{
    if (d->thumbBar->currentInfo().isNull())
    {
        return;
    }

    QString localFile;

    if (info.url.isLocalFile())
    {
        // file protocol. We do not need the network
        localFile = info.url.toLocalFile();
    }
    else
    {
        QMessageBox::critical(this, i18n("Error Loading File"),
                              i18n("Failed to load file: %1\n"
                                   "Remote file handling is not supported",
                                   info.url.fileName()));
        return;
    }

    d->currentLoadedUrl = info.url;

    m_canvas->load(localFile, m_IOFileSettings);

    //TODO : add preload here like in ImageWindow::slotLoadCurrent() ???
}

void ShowFoto::slotShowfotoItemInfoActivated(const ShowfotoItemInfo& info)
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->currentLoadedUrl))
    {
        d->thumbBar->setCurrentUrl(d->currentLoadedUrl);
        return;
    }

    slotOpenUrl(info);
}

Digikam::ThumbBarDock* ShowFoto::thumbBar() const
{
    return d->thumbBarDock;
}

Digikam::Sidebar* ShowFoto::rightSideBar() const
{
    return (dynamic_cast<Digikam::Sidebar*>(d->rightSideBar));
}

void ShowFoto::slotChanged()
{
    QString mpixels;
    QSize dims(m_canvas->imageWidth(), m_canvas->imageHeight());
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
    QString str = (!dims.isValid()) ? i18nc("unknown image dimensions", "Unknown")
                                    : i18nc("%1 width, %2 height, %3 mpixels", "%1x%2 (%3Mpx)",
                                            dims.width(),dims.height(),mpixels);
    m_resLabel->setAdjustedText(str);

    if (!d->thumbBar->currentInfo().isNull())
    {
        if (d->thumbBar->currentUrl().isValid())
        {
            QRect sel                = m_canvas->getSelectedArea();
            Digikam::DImg* const img = m_canvas->interface()->getImg();
            d->rightSideBar->itemChanged(d->thumbBar->currentUrl(), sel, img);
        }
    }
}

void ShowFoto::toggleActions(bool val)
{
    toggleStandardActions(val);
}

void ShowFoto::slotFilePrint()
{
    printImage(d->thumbBar->currentUrl());
}

void ShowFoto::slotSetup()
{
    setup(false);
}

void ShowFoto::slotSetupICC()
{
    setup(true);
}

bool ShowFoto::setup(bool iccSetupPage)
{
    QPointer<Setup> setup = new Setup(this, iccSetupPage ? Setup::ICCPage : Setup::LastPageUsed);

    if (setup->exec() != QDialog::Accepted)
    {
        delete setup;
        return false;
    }

    KSharedConfig::openConfig()->sync();

    applySettings();

    if (d->itemsNb == 0)
    {
        slotUpdateItemInfo();
        toggleActions(false);
    }

    delete setup;
    return true;
}

void ShowFoto::slotUpdateItemInfo()
{
    d->itemsNb = d->thumbBar->showfotoItemInfos().size();
    int index  = 0;
    QString text;

    if (d->itemsNb > 0)
    {
        index = 1;

        for (int i = 0; i < d->itemsNb; i++)
        {
            QUrl url = d->thumbBar->showfotoItemInfos().at(i).url;

            if (url.matches(d->thumbBar->currentUrl(), QUrl::None))
            {
                break;
            }

            ++index;
        }

        text = i18nc("<Image file name> (<Image number> of <Images in album>)",
                     "%1 (%2 of %3)", d->thumbBar->currentInfo().name,
                     index, d->itemsNb);

        setCaption(QDir::toNativeSeparators(d->thumbBar->currentUrl().adjusted(QUrl::RemoveFilename).toLocalFile()));
    }
    else
    {
        text = QLatin1String("");
        setCaption(QLatin1String(""));
    }

    m_nameLabel->setText(text);
    toggleNavigation(index);
}

void ShowFoto::slotOpenFolder(const QUrl& url)
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    m_canvas->load(QString(), m_IOFileSettings);
    d->thumbBar->showfotoItemInfos().clear();
    emit signalNoCurrentItem();

    openFolder(url);
    toggleNavigation(1);
}

void ShowFoto::slotOpenFilesInFolder()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    QUrl url = QUrl::fromLocalFile(DFileDialog::getExistingDirectory(this, i18n("Open Images From Folder"),
                                                                     d->lastOpenedDirectory.toLocalFile()));
    if (!url.isEmpty())
    {
        m_canvas->load(QString(), m_IOFileSettings);
        d->thumbBar->showfotoItemInfos().clear();
        d->lastOpenedDirectory = url;
        emit signalNoCurrentItem();

        openFolder(url);
        toggleNavigation(1);
    }
}

void ShowFoto::slotFirst()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    d->thumbBar->toFirstIndex();
    d->thumbBar->setCurrentInfo(d->thumbBar->showfotoItemInfos().first());
    slotOpenUrl(d->thumbBar->showfotoItemInfos().first());
}

void ShowFoto::slotLast()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    d->thumbBar->toLastIndex();
    d->thumbBar->setCurrentInfo(d->thumbBar->showfotoItemInfos().last());
    slotOpenUrl(d->thumbBar->showfotoItemInfos().last());
}

void ShowFoto::slotForward()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    bool currentIsNull = d->thumbBar->currentInfo().isNull();

    if (!currentIsNull)
    {
         d->thumbBar->toNextIndex();
         slotOpenUrl(d->thumbBar->currentInfo());
    }
}

void ShowFoto::slotBackward()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    bool currentIsNull = d->thumbBar->currentInfo().isNull();

    if (!currentIsNull)
    {
         d->thumbBar->toPreviousIndex();
         slotOpenUrl(d->thumbBar->currentInfo());
    }
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

void ShowFoto::slotPrepareToLoad()
{
    Digikam::EditorWindow::slotPrepareToLoad();

    // Here we enable specific actions on showfoto.
    d->openFilesInFolderAction->setEnabled(true);
    d->fileOpenAction->setEnabled(true);
}

void ShowFoto::slotLoadingStarted(const QString& filename)
{
    Digikam::EditorWindow::slotLoadingStarted(filename);

    // Here we disable specific actions on showfoto.
    d->openFilesInFolderAction->setEnabled(false);
    d->fileOpenAction->setEnabled(false);
}

void ShowFoto::slotLoadingFinished(const QString& filename, bool success)
{
    Digikam::EditorWindow::slotLoadingFinished(filename, success);

    // Here we re-enable specific actions on showfoto.
    d->openFilesInFolderAction->setEnabled(true);
    d->fileOpenAction->setEnabled(true);
}

void ShowFoto::slotSavingStarted(const QString& filename)
{
    Digikam::EditorWindow::slotSavingStarted(filename);

    // Here we disable specific actions on showfoto.
    d->openFilesInFolderAction->setEnabled(false);
    d->fileOpenAction->setEnabled(false);
}

void ShowFoto::moveFile()
{
    /*
     * moveFile() -> moveLocalFile() ->  movingSaveFileFinished()
     *                                     |               |
     *                            finishSaving(true)  save...IsComplete()
     */

    qCDebug(DIGIKAM_SHOWFOTO_LOG) << m_savingContext.destinationURL << m_savingContext.destinationURL.isLocalFile();

    if (m_savingContext.destinationURL.isLocalFile())
    {
        qCDebug(DIGIKAM_SHOWFOTO_LOG) << "moving a local file";
        EditorWindow::moveFile();
    }
    else
    {
        QMessageBox::critical(this, i18n("Error Saving File"),
                              i18n("Failed to save file: %1",
                              i18n("Remote file handling is not supported")));
    }
}

void ShowFoto::finishSaving(bool success)
{
    Digikam::EditorWindow::finishSaving(success);

    // Here we re-enable specific actions on showfoto.
    d->openFilesInFolderAction->setEnabled(true);
    d->fileOpenAction->setEnabled(true);
}

void ShowFoto::saveIsComplete()
{
    Digikam::LoadingCacheInterface::putImage(m_savingContext.destinationURL.toLocalFile(), m_canvas->currentImage());
    //d->thumbBar->invalidateThumb(d->currentItem);

    // Pop-up a message to bring user when save is done.
    Digikam::DNotificationWrapper(QLatin1String("editorsavefilecompleted"), i18n("Image saved successfully"),
                                  this, windowTitle());

    resetOrigin();
}

void ShowFoto::saveAsIsComplete()
{
    resetOriginSwitchFile();
/*
    Digikam::LoadingCacheInterface::putImage(m_savingContext.destinationURL.toLocalFile(), m_canvas->currentImage());

    // Add the file to the list of thumbbar images if it's not there already
    Digikam::ThumbBarItem* foundItem = d->thumbBar->findItemByUrl(m_savingContext.destinationURL);
    d->thumbBar->invalidateThumb(foundItem);
    qCDebug(DIGIKAM_SHOWFOTO_LOG) << wantedUrls;

    if (!foundItem)
    {
        foundItem = new Digikam::ThumbBarItem(d->thumbBar, m_savingContext.destinationURL);
    }

    // shortcut slotOpenUrl
    d->thumbBar->blockSignals(true);
    d->thumbBar->setSelected(foundItem);
    d->thumbBar->blockSignals(false);
    d->currentItem = foundItem;
    slotUpdateItemInfo();

    // Pop-up a message to bring user when save is done.
    Digikam::DNotificationWrapper("editorsavefilecompleted", i18n("Image saved successfully"),
                                  this, windowTitle());
*/
}

void ShowFoto::saveVersionIsComplete()
{
}

QUrl ShowFoto::saveDestinationUrl()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot return the url of the image to save "
                                       << "because no image is selected.";
        return QUrl();
    }

    return d->thumbBar->currentUrl();
}

bool ShowFoto::save()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "This should not happen";
        return true;
    }

    startingSave(d->currentLoadedUrl);
    return true;
}

bool ShowFoto::saveAs()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "This should not happen";
        return false;
    }

    return (startingSaveAs(d->currentLoadedUrl));
}

void ShowFoto::slotDeleteCurrentItem()
{
    QUrl urlCurrent(d->thumbBar->currentUrl());

    QString warnMsg(i18n("About to delete file \"%1\"\nAre you sure?",
                         urlCurrent.fileName()));

    if (QMessageBox::warning(this, qApp->applicationName(), warnMsg, QMessageBox::Apply | QMessageBox::Abort)
        !=  QMessageBox::Apply)
    {
        return;
    }
    else
    {
        bool ret = QFile::remove(urlCurrent.toLocalFile());

        if (!ret)
        {
            QMessageBox::critical(this, qApp->applicationName(), i18n("Cannot delete \"%1\"", urlCurrent.fileName()));
            return;
        }

        // No error, remove item in thumbbar.
        d->model->removeIndex(d->thumbBar->currentIndex());

        // Disable menu actions and SideBar if no current image.

        d->itemsNb = d->thumbBar->showfotoItemInfos().size();

        if (d->itemsNb == 0)
        {
            slotUpdateItemInfo();
            toggleActions(false);
            m_canvas->load(QString(), m_IOFileSettings);
        }
        else
        {
            // If there is an image after the deleted one, make that selected.

            slotOpenUrl(d->thumbBar->currentInfo());
        }
    }
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

void ShowFoto::slideShow(Digikam::SlideShowSettings& settings)
{
    if (!d->thumbBar->showfotoItemInfos().size())
    {
        return;
    }

    settings.exifRotate = Digikam::MetadataSettings::instance()->settings().exifRotate;
    settings.fileList   = d->thumbBar->urls();
    int   i             = 0;
    float cnt           = settings.fileList.count();
    m_cancelSlideShow   = false;
    Digikam::DMetadata meta;

    m_nameLabel->setProgressBarMode(Digikam::StatusProgressBar::CancelProgressBarMode,
                                    i18n("Preparing slideshow. Please wait..."));

    for (QList<QUrl>::ConstIterator it = settings.fileList.constBegin() ;
         !m_cancelSlideShow && (it != settings.fileList.constEnd()) ; ++it)
    {
        Digikam::SlidePictureInfo pictInfo;
        meta.load((*it).toLocalFile());
        pictInfo.comment   = meta.getImageComments()[QLatin1String("x-default")].caption;
        pictInfo.photoInfo = meta.getPhotographInformation();
        settings.pictInfoMap.insert(*it, pictInfo);

        m_nameLabel->setProgressValue((int)((i++/cnt)*100.0f));
        qApp->processEvents();
    }

    m_nameLabel->setProgressBarMode(Digikam::StatusProgressBar::TextMode, QString());

    if (!m_cancelSlideShow)
    {
        Digikam::SlideShow* const slide = new Digikam::SlideShow(settings);

        if (settings.startWithCurrent)
        {
            slide->setCurrentItem(d->thumbBar->currentUrl());
        }

        slide->show();
    }
}

void ShowFoto::slotPresentation()
{
    Digikam::PresentationMngr* const mngr = new Digikam::PresentationMngr(this);
    Digikam::DMetadata meta;

    QList<QUrl> urlList = d->thumbBar->urls();
    int i               = 0;
    float cnt           = urlList.count();
    m_cancelSlideShow   = false;

    m_nameLabel->setProgressBarMode(Digikam::StatusProgressBar::CancelProgressBarMode,
                                 i18n("Preparing presentation. Please wait..."));

    for (QList<QUrl>::ConstIterator it = urlList.constBegin() ;
         !m_cancelSlideShow && (it != urlList.constEnd()) ; ++it)
    {
        meta.load((*it).toLocalFile());
        mngr->addFile(*it, meta.getImageComments()[QLatin1String("x-default")].caption);

        m_nameLabel->setProgressValue((int)((i++/cnt)*100.0f));
        qApp->processEvents();
    }

    m_nameLabel->setProgressBarMode(Digikam::StatusProgressBar::TextMode, QString());
    mngr->showConfigDialog();
}

void ShowFoto::slotRevert()
{
    if (!promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    m_canvas->slotRestore();
}

bool ShowFoto::saveNewVersion()
{
    return false;
}

bool ShowFoto::saveCurrentVersion()
{
    return false;
}

bool ShowFoto::saveNewVersionAs()
{
    return false;
}

bool ShowFoto::saveNewVersionInFormat(const QString&)
{
    return false;
}

void ShowFoto::openFolder(const QUrl& url)
{
    if (!url.isValid() || !url.isLocalFile())
    {
        return;
    }

    // Parse image IO mime types registration to get files filter pattern.

    QString filter;
    QStringList mimeTypes = supportedImageMimeTypes(QIODevice::ReadOnly, filter);

    QString patterns = filter.toLower();
    patterns.append (QLatin1String(" "));
    patterns.append (filter.toUpper());

    qCDebug(DIGIKAM_SHOWFOTO_LOG) << "patterns=" << patterns;

    // Get all image files from directory.

    QDir dir(url.toLocalFile(), patterns);
    dir.setFilter(QDir::Files);

    if (!dir.exists())
    {
        return;
    }

    QFileInfoList fileinfolist = dir.entryInfoList();

    if (fileinfolist.isEmpty())
    {
        //emit signalSorry();
        return;
    }

    QFileInfoList::const_iterator fi;
    ShowfotoItemInfoList infos;
    ShowfotoItemInfo iteminfo;
    DMetadata meta;

    // And open all items in image editor.

    for (fi = fileinfolist.constBegin(); fi != fileinfolist.constEnd(); ++fi)
    {
        iteminfo.name      = (*fi).fileName();
        iteminfo.mime      = (*fi).suffix();
        iteminfo.size      = (*fi).size();
        iteminfo.folder    = (*fi).path();
        iteminfo.url       = QUrl::fromLocalFile((*fi).filePath());
        iteminfo.dtime     = (*fi).created();
        meta.load((*fi).filePath());
        iteminfo.ctime     = meta.getImageDateTime();
        iteminfo.width     = meta.getImageDimensions().width();
        iteminfo.height    = meta.getImageDimensions().height();
        iteminfo.photoInfo = meta.getPhotographInformation();
        infos.append(iteminfo);
    }

    if (d->droppedUrls)
    {
        d->infoList << infos;
    }
    else
    {
        d->infoList = infos;
        d->model->clearShowfotoItemInfos();
        emit signalInfoList(d->infoList);
        slotOpenUrl(d->thumbBar->currentInfo());
    }

    d->lastOpenedDirectory = QUrl::fromLocalFile(dir.absolutePath());
}

void ShowFoto::slotDroppedUrls(const QList<QUrl>& droppedUrls)
{
    if (!droppedUrls.isEmpty())
    {
        QList<QUrl> validUrls;

        foreach (const QUrl& url, droppedUrls)
        {
            if (url.isValid())
            {
                validUrls << url;
            }
        }

        d->droppedUrls = true;

        QList<QUrl> imagesUrls;
        QList<QUrl> foldersUrls;

        foreach (const QUrl& url, validUrls)
        {
            if (QMimeDatabase().mimeTypeForUrl(url).name().startsWith(QLatin1String("image"), Qt::CaseInsensitive))
            {
                imagesUrls << url;
            }

            if (QMimeDatabase().mimeTypeForUrl(url).name() == QLatin1String("inode/directory"))
            {
                foldersUrls << url;
            }
        }

        if (!imagesUrls.isEmpty())
        {
            openUrls(imagesUrls);
        }

        if (!foldersUrls.isEmpty())
        {
            foreach (const QUrl& fUrl, foldersUrls)
            {
                openFolder(fUrl);
            }
        }

        d->model->clearShowfotoItemInfos();
        emit signalInfoList(d->infoList);

        if (!d->infoList.isEmpty())
        {
            slotOpenUrl(d->thumbBar->currentInfo());
        }
        else
        {
            QMessageBox::information(this, qApp->applicationName(), i18n("There is no dropped item to process."));
            qWarning(DIGIKAM_SHOWFOTO_LOG) << "infolist is empty..";
        }

        d->droppedUrls = false;
    }
}

void ShowFoto::slotSetupMetadataFilters(int tab)
{
    Setup::execMetadataFilters(this, tab+1);
}

void ShowFoto::slotAddedDropedItems(QDropEvent* e)
{
    QList<QUrl> list = e->mimeData()->urls();
    QList<QUrl> urls;

    foreach(const QUrl& url, list)
    {
        QFileInfo fi(url.toLocalFile());

        if (fi.exists())
        {
            urls.append(url);
        }
    }

    e->accept();

    if (!urls.isEmpty())
    {
        slotDroppedUrls(urls);
    }
}

void ShowFoto::slotFileWithDefaultApplication()
{
    Digikam::DFileOperations::openFilesWithDefaultApplication(QList<QUrl>() << d->thumbBar->currentUrl());
}

void ShowFoto::addServicesMenu()
{
    addServicesMenuForUrl(d->thumbBar->currentUrl());
}

void ShowFoto::slotOpenWith(QAction* action)
{
    openWith(d->thumbBar->currentUrl(), action);
}

void ShowFoto::slotImportFromScanner()
{
#ifdef HAVE_KSANE

    QString place    = QDir::homePath();
    QStringList pics = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    if (!pics.isEmpty())
        place = pics.first();

    QUrl trg = saveDestinationUrl();

    if (!trg.isEmpty())
    {
        QString path = trg.adjusted(QUrl::RemoveFilename).toLocalFile();

        if (!path.isEmpty())
            place = path;
    }

    m_ksaneAction->activate(place, configGroupName());

    connect(m_ksaneAction, SIGNAL(signalImportedImage(QUrl)),
            this, SLOT(slotImportedImagefromScanner(QUrl)));
#endif
}

void ShowFoto::slotImportedImagefromScanner(const QUrl& url)
{
    openUrls(QList<QUrl>() << url);
}

void ShowFoto::slotEditGeolocation()
{
#ifdef HAVE_MARBLE
    QList<QUrl> infos = d->thumbBar->urls();

    if (infos.isEmpty())
    {
        return;
    }

    QPointer<GeolocationEdit> dialog = new GeolocationEdit(0,
                                                           new DMetaInfoIface(this, d->thumbBar->urls()),
                                                           QApplication::activeWindow());
    dialog->setImages(infos);
    dialog->exec();

    delete dialog;

    // Update image information everywhere.
    slotChanged();
#endif
}

void ShowFoto::slotEditMetadata()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        return;
    }

    QPointer<MetadataEditDialog> dialog = new MetadataEditDialog(QApplication::activeWindow(),
                                                                 QList<QUrl>() << d->thumbBar->currentInfo().url);
    dialog->exec();

    delete dialog;

    // Update image information everywhere.
    slotChanged();
}

void ShowFoto::slotHtmlGallery()
{
#ifdef HAVE_HTMLGALLERY
    HTMLWizard w(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w.exec();
#endif
}

void ShowFoto::slotCalendar()
{
    CalWizard w(d->thumbBar->urls(), this);
    w.exec();
}

void ShowFoto::slotPanorama()
{
#ifdef HAVE_PANORAMA
    PanoManager::instance()->checkBinaries();
    PanoManager::instance()->setItemsList(d->thumbBar->urls());
    PanoManager::instance()->run();
#endif
}

void ShowFoto::slotExpoBlending()
{
    ExpoBlendingManager::instance()->checkBinaries();
    ExpoBlendingManager::instance()->setItemsList(d->thumbBar->urls());
    ExpoBlendingManager::instance()->run();
}

void ShowFoto::slotVideoSlideshow()
{
#ifdef HAVE_MEDIAPLAYER
    VidSlideWizard w(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w.exec();
#endif
}

void ShowFoto::slotSendByMail()
{
    MailWizard w(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w.exec();
}

void ShowFoto::slotPrintCreator()
{
    AdvPrintWizard w(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w.exec();
}

void ShowFoto::slotMediaServer()
{
    DMediaServerDlg w(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w.exec();
}

void ShowFoto::slotExportTool()
{
    QAction* const tool = dynamic_cast<QAction*>(sender());

    if (tool == m_exportDropboxAction)
    {
        DBWindow w(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w.exec();
    }
    else if (tool == m_exportFacebookAction)
    {
        FbWindow w(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w.exec();
    }
    else if (tool == m_exportFlickrAction)
    {
        FlickrWindow w(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w.exec();
    }
    else if (tool == m_exportGdriveAction)
    {
        GSWindow w(new DMetaInfoIface(this, d->thumbBar->urls()),
                   this, QLatin1String("googledriveexport"));
        w.exec();
    }
    else if (tool == m_exportGphotoAction)
    {
        GSWindow w(new DMetaInfoIface(this, d->thumbBar->urls()),
                   this, QLatin1String("googlephotoexport"));
        w.exec();
    }
    else if (tool == m_exportImageshackAction)
    {
        ImageShackWindow w(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w.exec();
    }
    else if (tool == m_exportImgurAction)
    {
        ImgurWindow w(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w.exec();
    }
    else if (tool == m_exportPiwigoAction)
    {
        PiwigoWindow w(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w.exec();
    }
    else if (tool == m_exportRajceAction)
    {
        RajceWindow w(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w.exec();
    }
    else if (tool == m_exportSmugmugAction)
    {
        SmugWindow w(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w.exec();
    }
    else if (tool == m_exportYandexfotkiAction)
    {
        YFWindow w(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w.exec();
    }

#ifdef HAVE_MEDIAWIKI
    else if (tool == m_exportMediawikiAction)
    {
        MediaWikiWindow w(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w.exec();
    }
#endif

#ifdef HAVE_VKONTAKTE
    else if (tool == m_exportVkontakteAction)
    {
        VKWindow w(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w.exec();
    }
#endif

#ifdef HAVE_KIO
    else if (tool == m_exportFileTransferAction)
    {
        FTExportWindow w(new DMetaInfoIface(this, d->thumbBar->urls()), this);
        w.exec();
    }
#endif
}

void ShowFoto::slotImportTool()
{
    QAction* const tool = dynamic_cast<QAction*>(sender());

    if (tool == m_importGphotoAction)
    {
        GSWindow w(new DMetaInfoIface(this, QList<QUrl>() << d->thumbBar->currentUrl()),
                   this, QLatin1String("googlephotoimport"));
        w.exec();
    }
    else if (tool == m_importSmugmugAction)
    {
        SmugWindow w(new DMetaInfoIface(this, QList<QUrl>() << d->thumbBar->currentUrl()),
                     this, true);
        w.exec();
    }

#ifdef HAVE_KIO
    else if (tool == m_importFileTransferAction)
    {
        FTImportWindow w(new DMetaInfoIface(this, QList<QUrl>() << d->thumbBar->currentUrl()),
                         this);
        w.exec();
    }
#endif
}

} // namespace ShowFoto

#include "moc_showfoto.cpp"
