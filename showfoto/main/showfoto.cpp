/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 *
 * Copyright (C) 2004-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2008      by Arnd Baecker <arnd dot baecker at web dot de>
 * Copyright (C) 2013-2014 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "showfoto.moc"

// C ANSI includes

extern "C"
{
#include <unistd.h>
}

// C++ includes

#include <cstdio>

// Qt includes

#include <QCursor>
#include <QDesktopServices>
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

// KDE includes

#include <kaction.h>
#include <kactionmenu.h>
#include <kselectaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <kio/copyjob.h>
#include <kio/deletejob.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kmultitabbar.h>
#include <kprotocolinfo.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <ktoolbarpopupaction.h>
#include <kdebug.h>
#include <ksqueezedtextlabel.h>
#include <KVBox>

// LibKDcraw includes

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/version.h>

// Local includes

#include "canvas.h"
#include "editorcore.h"
#include "dmetadata.h"
#include "editorstackview.h"
#include "fileoperation.h"
#include "iccsettingscontainer.h"
#include "imagedialog.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imagepropertiessidebar.h"
#include "iofilesettings.h"
#include "loadingcache.h"
#include "loadingcacheinterface.h"
#include "metadatasettings.h"
#include "savingcontext.h"
#include "showfotosetup.h"
#include "showfotosetupmisc.h"
#include "setupicc.h"
#include "slideshow.h"
#include "splashscreen.h"
#include "statusprogressbar.h"
#include "thememanager.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "uifilevalidator.h"
#include "dnotificationwrapper.h"
#include "showfotodelegate.h"
#include "showfotocategorizedview.h"
#include "showfotosettings.h"
#include "showfoto_p.h"

namespace ShowFoto
{

ShowFoto::ShowFoto(const KUrl::List& urlList)
    : Digikam::EditorWindow("Showfoto"), d(new Private)
{
    setXMLFile("showfotoui.rc");

    m_nonDestructive = false;

    // --------------------------------------------------------

    Digikam::UiFileValidator validator(localXMLFile());

    if (!validator.isValid())
    {
        validator.fixConfigFile();
    }

    // --------------------------------------------------------

    // Show splash-screen at start up.

    KGlobal::dirs()->addResourceDir("data", KStandardDirs::installPath("data") + QString("digikam"));
    KIconLoader::global()->addAppDir("digikam");

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(EditorWindow::CONFIG_GROUP_NAME);

    if (group.readEntry("ShowSplash", true) && !kapp->isSessionRestored())
    {
        d->splash = new Digikam::SplashScreen();
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
        d->splash->message(i18n("Checking ICC repository..."));
    }

    d->validIccPath = Digikam::SetupICC::iccRepositoryIsValid();

    // Populate Themes

    if (d->splash)
    {
        d->splash->message(i18n("Loading themes..."));
    }

    Digikam::ThemeManager::instance();

    // -- Build the GUI -----------------------------------

    setupUserArea();
    setupActions();
    setupStatusBar();
    createGUI(xmlFile());

    // Load image plugins to GUI

    m_imagePluginLoader = new Digikam::ImagePluginLoader(this, d->splash);

    // Create context menu.

    setupContextMenu();

    // Make signals/slots connections

    setupConnections();

    // -- Read settings --------------------------------

    readSettings();
    applySettings();
    setAutoSaveSettings(EditorWindow::CONFIG_GROUP_NAME, true);

    d->rightSideBar->loadState();

    //--------------------------------------------------

    d->thumbBarDock->reInitialize();

    // -- Load current items ---------------------------
    slotDroppedUrls(urlList);

    if(!d->infoList.isEmpty())
    {
        slotOpenUrl(d->infoList.at(0));
    }
}

ShowFoto::~ShowFoto()
{
    unLoadImagePlugins();

    delete m_canvas;
    m_canvas = 0;

    Digikam::ThumbnailLoadThread::cleanUp();
    Digikam::LoadingCacheInterface::cleanUp();



    delete m_imagePluginLoader;
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

    KXmlGuiWindow::show();

    // Report errors from ICC repository path.

    KSharedConfig::Ptr config = KGlobal::config();

    if (!d->validIccPath)
    {
        QString message = i18n("<p>The ICC profile path seems to be invalid.</p>"
                               "<p>If you want to set it now, select \"Yes\", otherwise "
                               "select \"No\". In this case, \"Color Management\" feature "
                               "will be disabled until you solve this issue</p>");

        if (KMessageBox::warningYesNo(this, message) == KMessageBox::Yes)
        {
            if (!setup(true))
            {
                KConfigGroup group = config->group("Color Management");
                group.writeEntry("EnableCM", false);
                config->sync();
            }
        }
        else
        {
            KConfigGroup group = config->group("Color Management");
            group.writeEntry("EnableCM", false);
            config->sync();
        }
    }
}

void ShowFoto::setupConnections()
{
    setupStandardConnections();

    connect(d->thumbBarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            d->thumbBar, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    connect(d->thumbBar, SIGNAL(showfotoItemInfoActivated(ShowfotoItemInfo)),
            this, SLOT(slotOpenUrl(ShowfotoItemInfo)));

    connect(this, SIGNAL(signalSelectionChanged(QRect)),
            d->rightSideBar, SLOT(slotImageSelectionChanged(QRect)));

    connect(this, SIGNAL(signalOpenFolder(KUrl)),
            this, SLOT(slotOpenFolder(KUrl)));

    connect(this, SIGNAL(signalOpenFile(KUrl::List)),
            this, SLOT(slotOpenFile()));

    connect(this,SIGNAL(signalInfoList(ShowfotoItemInfoList&)),
            d->model,SLOT(reAddShowfotoItemInfos(ShowfotoItemInfoList&)));

    connect(d->thumbLoadThread,SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            d->model,SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSideBar, SLOT(slotNoCurrentItem()));

    connect(d->rightSideBar, SIGNAL(signalSetupMetadataFilters(int)),
            this, SLOT(slotSetupMetadataFilters(int)));

    connect(d->dDHandler, SIGNAL(signalDroppedUrls(KUrl::List)),
            this, SLOT(slotDroppedUrls(KUrl::List)));
}

void ShowFoto::setupUserArea()
{
    KSharedConfig::Ptr config  = KGlobal::config();
    KConfigGroup group         = config->group(EditorWindow::CONFIG_GROUP_NAME);

    QWidget* const widget      = new QWidget(this);
    QHBoxLayout* const hlay    = new QHBoxLayout(widget);
    m_splitter                 = new Digikam::SidebarSplitter(widget);

    KMainWindow* const viewContainer = new KMainWindow(widget, Qt::Widget);
    m_splitter->addWidget(viewContainer);
    m_stackView                      = new Digikam::EditorStackView(viewContainer);
    m_canvas                         = new Digikam::Canvas(m_stackView);
    viewContainer->setCentralWidget(m_stackView);

    m_splitter->setStretchFactor(1, 10);      // set Canvas default size to max.

    d->rightSideBar = new Digikam::ImagePropertiesSideBar(widget, m_splitter, KMultiTabBar::Right);
    d->rightSideBar->setObjectName("ShowFoto Sidebar Right");

    hlay->addWidget(m_splitter);
    hlay->addWidget(d->rightSideBar);
    hlay->setSpacing(0);
    hlay->setMargin(0);

    m_canvas->makeDefaultEditingCanvas();
    m_stackView->setCanvas(m_canvas);
    m_stackView->setViewMode(Digikam::EditorStackView::CanvasMode);

    m_splitter->setFrameStyle(QFrame::NoFrame);
    m_splitter->setFrameShadow(QFrame::Plain);
    m_splitter->setFrameShape(QFrame::NoFrame);
    m_splitter->setOpaqueResize(false);

    // Code to check for the now depreciated HorizontalThumbar directive. It
    // is found, it is honored and deleted. The state will from than on be saved
    // by viewContainers built-in mechanism.
    Qt::DockWidgetArea dockArea = Qt::LeftDockWidgetArea;

    d->thumbBarDock = new Digikam::ThumbBarDock(viewContainer, Qt::Tool);
    d->thumbBarDock->setObjectName("editor_thumbbar");
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

    viewContainer->setAutoSaveSettings("ImageViewer Thumbbar", true);

    d->thumbBar->installOverlays();

    setCentralWidget(widget);
}

void ShowFoto::setupActions()
{
    Digikam::ThemeManager::instance()->setThemeMenuAction(new KActionMenu(i18n("&Themes"), this));
    setupStandardActions();

    // Extra 'File' menu actions ---------------------------------------------

    d->fileOpenAction = actionCollection()->addAction(KStandardAction::Open, "showfoto_open_file",
                        this, SLOT(slotOpenFile()));

    d->openFilesInFolderAction = new KAction(KIcon("folder-image"), i18n("Open folder"), this);
    d->openFilesInFolderAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_O));

    connect(d->openFilesInFolderAction, SIGNAL(triggered()),
            this, SLOT(slotOpenFilesInFolder()));

    actionCollection()->addAction("showfoto_open_folder", d->openFilesInFolderAction);

    actionCollection()->addAction(KStandardAction::Quit, "showfoto_quit", this, SLOT(close()));

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
#if KDE_IS_VERSION(4,1,61)
        defaultDir = KGlobalSettings::picturesPath();
#else
        defaultDir = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
#endif
    }

    d->lastOpenedDirectory.setPath(defaultDir);

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
                              KMultiTabBar::VSNET : KMultiTabBar::KDEV3ICON);

    QString currentStyle = kapp->style()->objectName();
    QString newStyle     = d->settings->getApplicationStyle();

    if (newStyle != currentStyle)
    {
        kapp->setStyle(newStyle);
    }

    // Current image deleted go to trash ?
    d->deleteItem2Trash = d->settings->getDeleteItem2Trash();

    if (d->deleteItem2Trash)
    {
        m_fileDeleteAction->setIcon(KIcon("user-trash"));
        m_fileDeleteAction->setText(i18nc("Non-pluralized", "Move to Trash"));
    }
    else
    {
        m_fileDeleteAction->setIcon(KIcon("edit-delete"));
        m_fileDeleteAction->setText(i18n("Delete File"));
    }

    d->thumbBar->setToolTipEnabled(d->settings->getShowToolTip());

    d->rightSideBar->slotLoadMetadataFilters();
}

void ShowFoto::slotOpenFile()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    KUrl::List urls = Digikam::ImageDialog::getImageURLs(this, d->lastOpenedDirectory);    
    openUrls(urls);
}

void ShowFoto::openUrls(const KUrl::List &urls)
{
    if (!urls.isEmpty())
    {
        ShowfotoItemInfoList infos;
        ShowfotoItemInfo     iteminfo;
        DMetadata            meta;
        int i = 0;

        for (KUrl::List::const_iterator it = urls.constBegin();
             it != urls.constEnd(); ++it)
        {
            QFileInfo fi((*it).toLocalFile());
            iteminfo.name      = fi.fileName();
            iteminfo.mime      = fi.suffix();
            iteminfo.size      = fi.size();
            iteminfo.url       = fi.filePath();
            iteminfo.folder    = fi.path();
            iteminfo.dtime     = fi.created();
            meta.load(fi.filePath());
            iteminfo.ctime     = meta.getImageDateTime();
            iteminfo.width     = meta.getImageDimensions().width();
            iteminfo.height    = meta.getImageDimensions().height();
            iteminfo.photoInfo = meta.getPhotographInformation();
            infos.append(iteminfo);
            i++;
        }

        if(d->droppedUrls)
        {
            //replace the equal sign with "<<" to keep the previous pics in the list
            d->infoList << infos;
        }
        else
        {
            d->infoList = infos;
            d->model->clearShowfotoItemInfos();
            emit signalInfoList(d->infoList);
            slotOpenUrl(d->infoList.first());
        }

    }
}

void ShowFoto::slotOpenUrl(const ShowfotoItemInfo& info)
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        d->thumbBar->blockSignals(true);
        d->thumbBar->setCurrentUrl(info.url);
        d->thumbBar->blockSignals(false);
        return;
    }

    if (d->thumbBar->currentInfo().isNull())
    {
        return;
    }

    QString localFile;
    KIO::NetAccess::download(info.url, localFile, this);

    m_canvas->load(localFile, m_IOFileSettings);

    //TODO : add preload here like in ImageWindow::slotLoadCurrent() ???

    // By this condition we make sure that no crashes will happen
    // if no images were loaded to the canvas before
    if (!d->imagePluginsLoaded)
    {
        loadImagePlugins();
        d->imagePluginsLoaded = true;
    }
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
    m_resLabel->setText(str);

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

bool ShowFoto::setup()
{
    return setup(false);
}

bool ShowFoto::setupICC()
{
    return setup(true);
}

bool ShowFoto::setup(bool iccSetupPage)
{
    QPointer<Setup> setup = new Setup(this, iccSetupPage ? Setup::ICCPage : Setup::LastPageUsed);

    if (setup->exec() != QDialog::Accepted)
    {
        delete setup;
        return false;
    }

    KGlobal::config()->sync();

    applySettings();

    if ( d->itemsNb == 0 )
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
            if (d->thumbBar->showfotoItemInfos().at(i).url.equals(d->thumbBar->currentUrl()))
            {
                break;
            }

            ++index;
        }

        text = i18nc("<Image file name> (<Image number> of <Images in album>)",
                     "%1 (%2 of %3)", d->thumbBar->currentInfo().name,
                     index, d->itemsNb);

        setCaption(QDir::toNativeSeparators(d->thumbBar->currentUrl().directory()));
    }
    else
    {
        text = "";
        setCaption("");
    }

    m_nameLabel->setText(text);
    toggleNavigation( index );
}

void ShowFoto::slotOpenFolder(const KUrl& url)
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

    KUrl url(KFileDialog::getExistingDirectory(d->lastOpenedDirectory.directory(),
             this, i18n("Open Images From Folder")));

    if (!url.isEmpty())
    {
        d->lastOpenedDirectory = url;
        slotOpenFolder(url);
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
    if ( d->itemsNb == 0 || d->itemsNb == 1 )
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
    Digikam::DNotificationWrapper("editorsavefilecompleted", i18n("Image saved successfully"),
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
    kDebug() << wantedUrls;

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

KUrl ShowFoto::saveDestinationUrl()
{

    if (d->thumbBar->currentInfo().isNull())
    {
        kWarning() << "Cannot return the url of the image to save "
                   << "because no image is selected.";
        return KUrl();
    }

    return d->thumbBar->currentUrl();
}

bool ShowFoto::save()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        kWarning() << "This should not happen";
        return true;
    }

    startingSave(d->thumbBar->currentUrl());
    return true;
}

bool ShowFoto::saveAs()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        kWarning() << "This should not happen";
        return false;
    }

    return ( startingSaveAs(d->thumbBar->currentUrl()) );
}

void ShowFoto::slotDeleteCurrentItem()
{
    KUrl urlCurrent(d->thumbBar->currentUrl());

    if (!d->deleteItem2Trash)
    {
        QString warnMsg(i18n("About to delete file \"%1\"\nAre you sure?",
                             urlCurrent.fileName()));

        if (KMessageBox::warningContinueCancel(this,
                                               warnMsg,
                                               i18n("Warning"),
                                               KStandardGuiItem::del())
            !=  KMessageBox::Continue)
        {
            return;
        }
        else
        {
            KIO::Job* const job = KIO::del(urlCurrent);

            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDeleteCurrentItemResult(KJob*)) );
        }
    }
    else
    {
        KIO::Job* const job = KIO::trash(urlCurrent);

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(slotDeleteCurrentItemResult(KJob*)) );
    }
}

void ShowFoto::slotDeleteCurrentItemResult(KJob* job)
{
    if (job->error() != 0)
    {
        QString errMsg(job->errorString());
        KMessageBox::error(this, errMsg);
        return;
    }

    // No error, remove item in thumbbar.
    d->model->removeIndex(d->thumbBar->currentIndex());

    // Disable menu actions and SideBar if no current image.

    d->itemsNb = d->thumbBar->showfotoItemInfos().size();

    if ( d->itemsNb == 0 )
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

    m_nameLabel->progressBarMode(Digikam::StatusProgressBar::CancelProgressBarMode,
                                 i18n("Preparing slideshow. Please wait..."));

    for (KUrl::List::ConstIterator it = settings.fileList.constBegin() ;
         !m_cancelSlideShow && (it != settings.fileList.constEnd()) ; ++it)
    {
        Digikam::SlidePictureInfo pictInfo;
        meta.load((*it).toLocalFile());
        pictInfo.comment   = meta.getImageComments()[QString("x-default")].caption;
        pictInfo.photoInfo = meta.getPhotographInformation();
        settings.pictInfoMap.insert(*it, pictInfo);

        m_nameLabel->setProgressValue((int)((i++/cnt)*100.0f));
        kapp->processEvents();
    }

    m_nameLabel->progressBarMode(Digikam::StatusProgressBar::TextMode, QString());

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

void ShowFoto::openFolder(const KUrl& url)
{
    if (!url.isValid() || !url.isLocalFile())
    {
        return;
    }

    // Parse KDE image IO mime types registration to get files filter pattern.

    QStringList mimeTypes = KImageIO::mimeTypes(KImageIO::Reading);
    QString filter;

    for (QStringList::ConstIterator it = mimeTypes.constBegin() ; it != mimeTypes.constEnd() ; ++it)
    {
        QString format = KImageIO::typeForMime(*it).at(0).toUpper();
        filter.append ("*.");
        filter.append (format);
        filter.append (" ");
    }

    // Because KImageIO return only *.JPEG and *.TIFF mime types.
    if ( filter.contains("*.TIFF") )
    {
        filter.append (" *.TIF");
    }

    if ( filter.contains("*.JPEG") )
    {
        filter.append (" *.JPG");
        filter.append (" *.JPE");
    }

    // Added RAW files extensions supported by dcraw program and
    // defines to digikam/libs/dcraw/rawfiles.h
    filter.append (" ");
    filter.append ( QString(KDcrawIface::KDcraw::rawFiles()) );
    filter.append (" ");

    QString patterns = filter.toLower();
    patterns.append (" ");
    patterns.append (filter.toUpper());

    kDebug() << "patterns=" << patterns;


    // Get all image files from directory.

    QDir dir(url.toLocalFile(), patterns);
    dir.setFilter ( QDir::Files );
    d->dir = dir;

    if (!dir.exists())
    {
        return;
    }

    // Determine sort ordering for the entries from configuration setting:

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(EditorWindow::CONFIG_GROUP_NAME);

    QDir::SortFlags flag;
    bool            reverse   = group.readEntry("ReverseSort", false);

    switch (group.readEntry("SortOrder", (int)SetupMisc::SortByDate))
    {
        case SetupMisc::SortByName:
        {
            flag = QDir::Name;  // Ordering by file name.

            if (reverse)
            {
                flag = flag | QDir::Reversed;
            }

            break;
        }
        case SetupMisc::SortByFileSize:
        {
            flag = QDir::Size;  // Ordering by file size.

            // Disabled reverse in the settings leads e.g. to increasing file sizes
            // Note, that this is just the opposite to the sort order for QDir.
            if (!reverse)
            {
                flag = flag | QDir::Reversed;
            }

            break;
        }
        default:
        {
            flag = QDir::Time;  // Ordering by file date.

            // Disabled reverse in the settings leads e.g. to increasing dates
            // Note, that this is just the opposite to the sort order for QDir.
            if (!reverse)
            {
                flag = flag | QDir::Reversed;
            }

            break;
        }
    }

    dir.setSorting(flag);

    QFileInfoList fileinfolist = dir.entryInfoList();

    if (fileinfolist.isEmpty())
    {
        //emit signalSorry();
        return;
    }

    QFileInfoList::const_iterator fi;
    ShowfotoItemInfo iteminfo;
    ShowfotoItemInfoList infos;
    DMetadata meta;

    // And open all items in image editor.

    for (fi = fileinfolist.constBegin(); fi != fileinfolist.constEnd(); ++fi)
    {
        iteminfo.name      = (*fi).fileName();
        iteminfo.mime      = (*fi).suffix();
        iteminfo.size      = (*fi).size();
        iteminfo.folder    = (*fi).path();
        iteminfo.url       = (*fi).filePath();
        iteminfo.dtime     = (*fi).created();
        meta.load((*fi).filePath());
        iteminfo.ctime     = meta.getImageDateTime();
        iteminfo.width     = meta.getImageDimensions().width();
        iteminfo.height    = meta.getImageDimensions().height();
        iteminfo.photoInfo = meta.getPhotographInformation();
        infos.append(iteminfo);
    }

    if(d->droppedUrls)
    {
        d->infoList << infos;
    }
    else
    {
        d->infoList = infos;
        d->model->clearShowfotoItemInfos();
        emit signalInfoList(d->infoList);
        slotOpenUrl(d->infoList.at(0));
    }

    d->lastOpenedDirectory = d->infoList.at(0).url;
}

void ShowFoto::slotDroppedUrls(const KUrl::List& droppedUrls)
{
    if (!droppedUrls.isEmpty())
    {
        KUrl::List validUrls;

        foreach (const KUrl& url, droppedUrls)
        {
            if (url.isValid())
            {
                validUrls << url;
            }
        }

        d->droppedUrls = true;

        KUrl::List imagesUrls;
        KUrl::List foldersUrls;

        foreach (const KUrl& url, validUrls)
        {
            if (KMimeType::findByUrl(url)->name().startsWith("image", Qt::CaseInsensitive))
            {
                imagesUrls << url;
            }

            if (KMimeType::findByUrl(url)->name() == "inode/directory")
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
            foreach (const KUrl& fUrl, foldersUrls)
            {
                openFolder(fUrl);
            }
        }

        d->model->clearShowfotoItemInfos();
        emit signalInfoList(d->infoList);

        slotOpenUrl(d->infoList.at(0));

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
    KUrl::List urls;

    foreach(const QUrl& url, list)
    {
        QFileInfo fi(url.path());

        if (fi.exists())
        {
            urls.append(KUrl(url));
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
    Digikam::FileOperation::openFilesWithDefaultApplication(KUrl::List() << d->thumbBar->currentUrl(), this);
}

void ShowFoto::addServicesMenu()
{
    addServicesMenuForUrl(d->thumbBar->currentUrl());
}

void ShowFoto::slotOpenWith(QAction* action)
{
    openWith(d->thumbBar->currentUrl(), action);
}

}   // namespace ShowFoto
