/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

// C Ansi includes.

extern "C"
{
#include <unistd.h>
}

// C++ includes.

#include <cstdio>

// Qt includes.

#include <QList>
#include <QLabel>
#include <QLayout>
#include <QSplitter>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QCursor>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QFrame>
#include <QVBoxLayout>

// KDE includes.

#include <kcursor.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kactionmenu.h>
#include <ktoolbarpopupaction.h>
#include <kstandardaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kimageio.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kio/copyjob.h>
#include <kio/deletejob.h>
#include <kprotocolinfo.h>
#include <kglobalsettings.h>
#include <kfiledialog.h>
#include <ktoolbar.h>
#include <kstatusbar.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawbinary.h>

// Local includes.

#include "ddebug.h"
#include "dpopupmenu.h"
#include "dmetadata.h"
#include "canvas.h"
#include "thumbbar.h"
#include "imagepropertiessidebar.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imagedialog.h"
#include "dimginterface.h"
#include "loadingcache.h"
#include "splashscreen.h"
#include "slideshow.h"
#include "setup.h"
#include "setupicc.h"
#include "statusprogressbar.h"
#include "iccsettingscontainer.h"
#include "iofilesettingscontainer.h"
#include "loadingcacheinterface.h"
#include "savingcontextcontainer.h"
#include "themeengine.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "showfoto.h"
#include "showfoto.moc"

namespace ShowFoto
{

class ShowFotoPriv
{
public:

    ShowFotoPriv()
    {
        thumbLoadThread         = 0;
        currentItem             = 0;
        itemsNb                 = 0;
        splash                  = 0;
        BCGAction               = 0;
        showBarAction           = 0;
        openFilesInFolderAction = 0;
        fileOpenAction          = 0;
        thumbBar                = 0;
        rightSidebar            = 0;
        splash                  = 0;
        itemsNb                 = 0;
        vSplitter               = 0;
        deleteItem2Trash        = true;
        fullScreenHideThumbBar  = true;
        validIccPath            = true;
    }

    bool                             fullScreenHideThumbBar;
    bool                             deleteItem2Trash;
    bool                             validIccPath;

    int                              itemsNb;

    QSplitter                       *vSplitter;

    QAction                         *fileOpenAction;

    KUrl                             lastOpenedDirectory;

    KToggleAction                   *showBarAction;

    KAction                         *openFilesInFolderAction;

    KActionMenu                     *BCGAction;

    Digikam::ThumbnailLoadThread    *thumbLoadThread;
    Digikam::ThumbBarView           *thumbBar;
    Digikam::ThumbBarItem           *currentItem;
    Digikam::ImagePropertiesSideBar *rightSidebar;
    Digikam::SplashScreen           *splash;
};

ShowFoto::ShowFoto(const KUrl::List& urlList)
        : Digikam::EditorWindow( "Showfoto" )
{
    d = new ShowFotoPriv();

    // -- Show splash at start ----------------------------

    KGlobal::dirs()->addResourceDir("data", KStandardDirs::installPath("data") + QString("digikam")); 
    KIconLoader::global()->addAppDir("digikam");

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("ImageViewer Settings");

    if(group.readEntry("ShowSplash", true) && !kapp->isSessionRestored())
    {
        d->splash = new Digikam::SplashScreen("showfoto-splash.png");
        d->splash->show();
    }

    // Setup loading cache and thumbnails interface.

    Digikam::LoadingCacheInterface::initialize();

    d->thumbLoadThread = new Digikam::ThumbnailLoadThread();
    d->thumbLoadThread->setThumbnailSize(ThumbnailSize::Huge);
    d->thumbLoadThread->setSendSurrogatePixmap(true);

    // Check ICC profiles repository availability

    if(d->splash)
    {
        d->splash->message(i18n("Checking ICC repository"), Qt::AlignLeft, Qt::white);
    }

    d->validIccPath = Digikam::SetupICC::iccRepositoryIsValid();

    // Check witch dcraw version available

    if(d->splash)
        d->splash->message(i18n("Checking dcraw version"), Qt::AlignLeft, Qt::white);

    KDcrawIface::DcrawBinary::instance()->checkSystem();

    // Populate Themes

    if(d->splash)
        d->splash->message(i18n("Loading themes"), Qt::AlignLeft, Qt::white);

    Digikam::ThemeEngine::instance()->scanThemes();

    // -- Build the GUI -----------------------------------

    setupUserArea();
    setupStatusBar();
    setupActions();

    // Load image plugins to GUI

    m_imagePluginLoader = new Digikam::ImagePluginLoader(this, d->splash);
    loadImagePlugins();

    // If plugin core is not available, plug BCG actions to collection instead.

    if ( !m_imagePluginLoader->pluginLibraryIsLoaded("digikamimageplugin_core") )
    {
        d->BCGAction = new KActionMenu(i18n("Brightness/Contrast/Gamma"), 0);
        d->BCGAction->setObjectName("showfoto_bcg");
        d->BCGAction->setDelayed(false);

        KAction *incGammaAction = new KAction(i18n("Increase Gamma"), this);
        incGammaAction->setShortcut(Qt::ALT+Qt::Key_G);
        connect(incGammaAction, SIGNAL(triggered()), this, SLOT(slotChangeBCG()));
        actionCollection()->addAction("gamma_plus", incGammaAction);

        KAction *decGammaAction = new KAction(i18n("Decrease Gamma"), this);
        decGammaAction->setShortcut(Qt::ALT+Qt::SHIFT+Qt::Key_G);
        connect(decGammaAction, SIGNAL(triggered()), this, SLOT(slotChangeBCG()));
        actionCollection()->addAction("gamma_minus", decGammaAction);

        KAction *incBrightAction = new KAction(i18n("Increase Brightness"), this);
        incBrightAction->setShortcut(Qt::ALT+Qt::Key_B);
        connect(incBrightAction, SIGNAL(triggered()), this, SLOT(slotChangeBCG()));
        actionCollection()->addAction("brightness_plus", incBrightAction);

        KAction *decBrightAction = new KAction(i18n("Decrease Brightness"), this);
        decBrightAction->setShortcut(Qt::ALT+Qt::SHIFT+Qt::Key_B);
        connect(decBrightAction, SIGNAL(triggered()), this, SLOT(slotChangeBCG()));
        actionCollection()->addAction("brightness_minus", decBrightAction);

        KAction *incContrastAction = new KAction(i18n("Increase Contrast"), this);
        incContrastAction->setShortcut(Qt::ALT+Qt::Key_C);
        connect(incContrastAction, SIGNAL(triggered()), this, SLOT(slotChangeBCG()));
        actionCollection()->addAction("contrast_plus", incContrastAction);

        KAction *decContrastAction = new KAction(i18n("Decrease Contrast"), this);
        decContrastAction->setShortcut(Qt::ALT+Qt::SHIFT+Qt::Key_C);
        connect(decContrastAction, SIGNAL(triggered()), this, SLOT(slotChangeBCG()));
        actionCollection()->addAction("contrast_minus", decContrastAction);

        d->BCGAction->addAction(incBrightAction);
        d->BCGAction->addAction(decBrightAction);
        d->BCGAction->addAction(incContrastAction);
        d->BCGAction->addAction(decContrastAction);
        d->BCGAction->addAction(incGammaAction);
        d->BCGAction->addAction(decGammaAction);

        QList<QAction*> bcg_actions;
        bcg_actions.append( d->BCGAction );
        unplugActionList( "showfoto_bcg" );
        plugActionList( "showfoto_bcg", bcg_actions );
    }

    // Create context menu.

    setupContextMenu();

    // Make signals/slots connections

    setupConnections();

    // -- Read settings --------------------------------

    readSettings();
    applySettings();
    setAutoSaveSettings("ImageViewer Settings");

    // -- Load current items ---------------------------

    for (KUrl::List::const_iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        KUrl url = *it;
        if (url.isLocalFile())
        {
            QFileInfo fi(url.path());
            if (fi.isDir())
            {
                // Local Dir
                openFolder(url);
            }
            else
            {
                // Local file
                new Digikam::ThumbBarItem(d->thumbBar, url);
                d->lastOpenedDirectory=(*it);
            }
        }
        else
        {
            // Remote file.
            new Digikam::ThumbBarItem(d->thumbBar, url);
            d->lastOpenedDirectory=(*it);
        }
    }

    if ( urlList.isEmpty() )
    {
        emit signalNoCurrentItem();
        toggleActions(false);
        toggleNavigation(0);
    }
    else
    {
        toggleNavigation(1);
        toggleActions(true);
    }
}

ShowFoto::~ShowFoto()
{
    unLoadImagePlugins();

    delete m_imagePluginLoader;
    delete d->thumbBar;
    delete d->rightSidebar;
    delete d->thumbLoadThread;
    delete d;
}

bool ShowFoto::queryClose()
{
    // wait if a save operation is currently running
    if (!waitForSavingToComplete())
        return false;

    if (d->currentItem && !promptUserSave(d->currentItem->url()))
        return false;

    return true;
}

bool ShowFoto::queryExit()
{
    saveSettings();
    return true;
}

void ShowFoto::show()
{
    // Remove Splashscreen.

    if(d->splash)
    {
        d->splash->finish(this);
        delete d->splash;
        d->splash = 0;
    }

    // Display application window.

    KXmlGuiWindow::show();

    // Report errors from ICC repository path.

    KSharedConfig::Ptr config = KGlobal::config();
    if(!d->validIccPath)
    {
        QString message = i18n("<qt><p>The ICC profile path seems to be invalid.</p>"
                               "<p>If you want to set it now, select \"Yes\", otherwise "
                               "select \"No\". In this case, \"Color Management\" feature "
                               "will be disabled until you solve this issue</p></qt>");

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

    // Report errors from dcraw detection.

    KDcrawIface::DcrawBinary::instance()->checkReport();
}

void ShowFoto::setupConnections()
{
    setupStandardConnections();

    connect(d->thumbBar, SIGNAL(signalURLSelected(const KUrl&)),
            this, SLOT(slotOpenURL(const KUrl&)));

    connect(d->thumbBar, SIGNAL(signalItemAdded()),
            this, SLOT(slotUpdateItemInfo()));

    connect(this, SIGNAL(signalSelectionChanged(const QRect &)),
            d->rightSidebar, SLOT(slotImageSelectionChanged(const QRect &)));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSidebar, SLOT(slotNoCurrentItem()));
}

void ShowFoto::setupUserArea()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");

    QWidget* widget = new QWidget(this);
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    rightSzPolicy.setHorizontalStretch(2);
    rightSzPolicy.setVerticalStretch(1);

    if(!group.readEntry("HorizontalThumbbar", false)) // Vertical thumbbar layout
    {
        QHBoxLayout *hlay = new QHBoxLayout(widget);
        m_splitter        = new QSplitter(widget);
        d->thumbBar       = new Digikam::ThumbBarView(m_splitter, Qt::Vertical);
        m_canvas          = new Digikam::Canvas(m_splitter);

        m_canvas->setSizePolicy(rightSzPolicy);
        m_canvas->makeDefaultEditingCanvas();

        d->rightSidebar   = new Digikam::ImagePropertiesSideBar(widget, m_splitter, Digikam::Sidebar::DockRight);
        d->rightSidebar->setObjectName("ShowFoto Sidebar Right");

        hlay->addWidget(m_splitter);
        hlay->addWidget(d->rightSidebar);
        hlay->setSpacing(0);
        hlay->setMargin(0);
    }
    else                                                     // Horizontal thumbbar layout
    {
        m_splitter        = new QSplitter(Qt::Horizontal, widget);
        QWidget* widget2  = new QWidget(m_splitter);
        QVBoxLayout *vlay = new QVBoxLayout(widget2);
        d->vSplitter      = new QSplitter(Qt::Vertical, widget2);
        m_canvas          = new Digikam::Canvas(d->vSplitter);
        d->thumbBar       = new Digikam::ThumbBarView(d->vSplitter, Qt::Horizontal);

        m_canvas->setSizePolicy(rightSzPolicy);
        m_canvas->makeDefaultEditingCanvas();

        d->vSplitter->setFrameStyle( QFrame::NoFrame );
        d->vSplitter->setFrameShadow( QFrame::Plain );
        d->vSplitter->setFrameShape( QFrame::NoFrame );
        d->vSplitter->setOpaqueResize(false);

        vlay->addWidget(d->vSplitter);
        vlay->setSpacing(0);
        vlay->setMargin(0);

        QHBoxLayout *hlay = new QHBoxLayout(widget);
        d->rightSidebar   = new Digikam::ImagePropertiesSideBar(widget, m_splitter, Digikam::Sidebar::DockRight);
        d->rightSidebar->setObjectName("ShowFoto Sidebar Right");

        hlay->addWidget(m_splitter);
        hlay->addWidget(d->rightSidebar);
        hlay->setSpacing(0);
        hlay->setMargin(0);
    }

    m_splitter->setFrameStyle( QFrame::NoFrame );
    m_splitter->setFrameShadow( QFrame::Plain );
    m_splitter->setFrameShape( QFrame::NoFrame );
    m_splitter->setOpaqueResize(false);
    setCentralWidget(widget);
    d->rightSidebar->loadViewState();
}

void ShowFoto::setupActions()
{
    setupStandardActions();

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // Extra 'File' menu actions ---------------------------------------------

    d->fileOpenAction = actionCollection()->addAction(KStandardAction::Open, "showfoto_open_file", 
                                                      this, SLOT(slotOpenFile()));

    d->openFilesInFolderAction = new KAction(KIcon("folder-image"), i18n("Open folder"), this);
    d->openFilesInFolderAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_O);
    connect(d->openFilesInFolderAction, SIGNAL(triggered()), this, SLOT(slotOpenFilesInFolder()));
    actionCollection()->addAction("showfoto_open_folder", d->openFilesInFolderAction);

    actionCollection()->addAction(KStandardAction::Quit, "showfoto_quit", this, SLOT(close()));

    // Extra 'View' menu actions ---------------------------------------------

    d->showBarAction = new KToggleAction(KIcon("view-choose"), i18n("Show Thumbnails"), this);
    d->showBarAction->setShortcut(Qt::CTRL+Qt::Key_T);
    connect(d->showBarAction, SIGNAL(triggered()), this, SLOT(slotToggleShowBar()));
    actionCollection()->addAction("shofoto_showthumbs", d->showBarAction);

    // --- Create the gui ----------------------------------------------------

    createGUI("showfotoui.rc");
}

void ShowFoto::readSettings()
{
    readStandardSettings();

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("ImageViewer Settings");

    d->showBarAction->setChecked(group.readEntry("Show Thumbnails", true));
    slotToggleShowBar();

    d->lastOpenedDirectory.setPath( group.readEntry("Last Opened Directory",
                                    KGlobalSettings::documentPath()) );

    QSizePolicy szPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    szPolicy.setHorizontalStretch(2);
    szPolicy.setVerticalStretch(1);
    QList<int> list;
    if(group.hasKey("Vertical Splitter Sizes") && d->vSplitter)
    {
        QByteArray state;
        state = group.readEntry("Vertical Splitter State", state);
        d->vSplitter->restoreState(QByteArray::fromBase64(state));
    }
    else
        m_canvas->setSizePolicy(szPolicy);

    Digikam::ThemeEngine::instance()->setCurrentTheme(group.readEntry("Theme", i18n("Default")));
}

void ShowFoto::saveSettings()
{
    saveStandardSettings();

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("ImageViewer Settings");

    group.writeEntry("Last Opened Directory", d->lastOpenedDirectory.path() );
    group.writeEntry("Show Thumbnails", d->showBarAction->isChecked());

    if (d->vSplitter)
        group.writeEntry("Vertical Splitter State", d->vSplitter->saveState().toBase64());

    group.writeEntry("Theme", Digikam::ThemeEngine::instance()->getCurrentThemeName());

    group.sync();
}

void ShowFoto::applySettings()
{
    applyStandardSettings();

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("ImageViewer Settings");

    // Current image deleted go to trash ?
    d->deleteItem2Trash = group.readEntry("DeleteItem2Trash", true);
    if (d->deleteItem2Trash)
    {
        m_fileDeleteAction->setIcon(KIcon("user-trash"));
        m_fileDeleteAction->setText(i18n("Move to Trash"));
    }
    else
    {
        m_fileDeleteAction->setIcon(KIcon("edit-delete"));
        m_fileDeleteAction->setText(i18n("Delete File"));
    }

    bool exifRotate = group.readEntry("EXIF Rotate", true);
    m_canvas->setExifOrient(exifRotate);
    d->thumbBar->setExifRotate(exifRotate);

    m_setExifOrientationTag   = group.readEntry("EXIF Set Orientation", true);

    d->fullScreenHideThumbBar = group.readEntry("FullScreenHideThumbBar", true);

    Digikam::ThumbBarToolTipSettings settings;
    settings.showToolTips   = group.readEntry("Show ToolTips", true);
    settings.showFileName   = group.readEntry("ToolTips Show File Name", true);
    settings.showFileDate   = group.readEntry("ToolTips Show File Date", false);
    settings.showFileSize   = group.readEntry("ToolTips Show File Size", false);
    settings.showImageType  = group.readEntry("ToolTips Show Image Type", false);
    settings.showImageDim   = group.readEntry("ToolTips Show Image Dim", true);
    settings.showPhotoMake  = group.readEntry("ToolTips Show Photo Make", true);
    settings.showPhotoDate  = group.readEntry("ToolTips Show Photo Date", true);
    settings.showPhotoFocal = group.readEntry("ToolTips Show Photo Focal", true);
    settings.showPhotoExpo  = group.readEntry("ToolTips Show Photo Expo", true);
    settings.showPhotoMode  = group.readEntry("ToolTips Show Photo Mode", true);
    settings.showPhotoFlash = group.readEntry("ToolTips Show Photo Flash", false);
    settings.showPhotoWB    = group.readEntry("ToolTips Show Photo WB", false);
    d->thumbBar->setToolTipSettings(settings);
}

void ShowFoto::slotOpenFile()
{
    if (d->currentItem && !promptUserSave(d->currentItem->url()))
        return;

    KUrl::List urls = ImageDialog::getImageURLs(this, d->lastOpenedDirectory);

    if (!urls.isEmpty())
    {
        d->currentItem = 0;
        d->thumbBar->clear();

        for (KUrl::List::const_iterator it = urls.begin();
             it != urls.end(); ++it)
        {
            new Digikam::ThumbBarItem(d->thumbBar, *it);
            d->lastOpenedDirectory=(*it);
        }

        toggleActions(true);
    }
}

void ShowFoto::slotOpenURL(const KUrl& url)
{
    if(d->currentItem && !promptUserSave(d->currentItem->url()))
    {
        d->thumbBar->blockSignals(true);
        d->thumbBar->setSelected(d->currentItem);
        d->thumbBar->blockSignals(false);
        return;
    }

    d->currentItem = d->thumbBar->currentItem();
    if(!d->currentItem)
        return;

    QString localFile;
    KIO::NetAccess::download(url, localFile, this);
    
    m_canvas->load(localFile, m_IOFileSettings);

    // TODO : add preload here like in ImageWindow::slotLoadCurrent() ???
}

void ShowFoto::toggleGUI2FullScreen()
{
    if (m_fullScreen)
    {
        d->rightSidebar->restore();

        // If Hide Thumbbar option is checked, restore it.
        if (!d->showBarAction->isChecked())
            d->thumbBar->show();
    }
    else
    {
        d->rightSidebar->backup();

        // If Hide Thumbbar option is checked, catch it if necessary.
        if (!d->showBarAction->isChecked())
        {
            if (d->fullScreenHideThumbBar)
                d->thumbBar->hide();
        }
    }
}

void ShowFoto::slotToggleShowBar()
{
    if (d->showBarAction->isChecked())
        d->thumbBar->show();
    else
        d->thumbBar->hide();
}

void ShowFoto::slotChangeBCG()
{
    QString name;
    if (sender())
        name = sender()->objectName();

    if (name == "gamma_plus")
    {
        m_canvas->increaseGamma();
    }
    else if  (name == "gamma_minus")
    {
        m_canvas->decreaseGamma();
    }
    else if  (name == "brightness_plus")
    {
        m_canvas->increaseBrightness();
    }
    else if  (name == "brightness_minus")
    {
        m_canvas->decreaseBrightness();
    }
    else if  (name == "contrast_plus")
    {
        m_canvas->increaseContrast();
    }
    else if  (name == "contrast_minus")
    {
        m_canvas->decreaseContrast();
    }
}

void ShowFoto::slotChanged()
{
    QString mpixels;
    QSize dims(m_canvas->imageWidth(), m_canvas->imageHeight());
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
    QString str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)",dims.width(),dims.height(),mpixels);
    m_resLabel->setText(str);

    if (d->currentItem)
    {
        if (d->currentItem->url().isValid())
        {
            QRect sel          = m_canvas->getSelectedArea();
            Digikam::DImg* img = m_canvas->interface()->getImg();
            d->rightSidebar->itemChanged(d->currentItem->url(), sel, img);
        }
    }    
}

void ShowFoto::slotUndoStateChanged(bool moreUndo, bool moreRedo, bool canSave)
{
    m_revertAction->setEnabled(canSave);
    m_undoAction->setEnabled(moreUndo);
    m_redoAction->setEnabled(moreRedo);
    m_saveAction->setEnabled(canSave);
    
    if (!moreUndo)
        m_rotatedOrFlipped = false;        
}

void ShowFoto::toggleActions(bool val)
{
    toggleStandardActions(val);
        
    // if BCG actions exists then toggle it.
    if (d->BCGAction)
        d->BCGAction->setEnabled(val);
}

void ShowFoto::slotFilePrint()
{
    printImage(d->currentItem->url());
}

bool ShowFoto::setup(bool iccSetupPage)
{
    Setup setup(this, 0, iccSetupPage ? Setup::ICCPage : Setup::LastPageUsed);
    
    if (setup.exec() != QDialog::Accepted)
        return false;

    KGlobal::config()->sync();
    
    applySettings();

    if ( d->itemsNb == 0 )
    {
        slotUpdateItemInfo();
        toggleActions(false);
    }

    return true;
}

void ShowFoto::slotUpdateItemInfo()
{
    d->itemsNb = d->thumbBar->countItems();
    
    m_rotatedOrFlipped = false;
    int index = 0;
    QString text;
    
    if (d->itemsNb > 0)
    {
        index = 1;
        
        for (Digikam::ThumbBarItem *item = d->thumbBar->firstItem(); item; item = item->next())
        {
            if (item->url().equals(d->currentItem->url()))
            {
                break;
            }
            index++;
        }

        text = i18nc("<Image file name> (<Image number> of <Images in album>)", 
                     "%1 (%2 of %3)", d->currentItem->url().fileName(), 
                                      index, d->itemsNb);
    
        setCaption(d->currentItem->url().directory());
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
    if (d->currentItem && !promptUserSave(d->currentItem->url()))
        return;

    m_canvas->load(QString(), m_IOFileSettings);
    d->thumbBar->clear(true);
    emit signalNoCurrentItem();
    d->currentItem = 0;
    openFolder(url);
    toggleActions(true);
    toggleNavigation(1);
}

void ShowFoto::openFolder(const KUrl& url)
{
    if (!url.isValid() || !url.isLocalFile())
       return;

    // Parse KDE image IO mime types registration to get files filter pattern.

    QStringList mimeTypes = KImageIO::mimeTypes(KImageIO::Reading);
    QString filter;

    for (QStringList::ConstIterator it = mimeTypes.begin() ; it != mimeTypes.end() ; ++it)
    {    
        QString format = KImageIO::typeForMime(*it)[0].toUpper();
        filter.append ("*.");
        filter.append (format);
        filter.append (" ");
    }    

    // Because KImageIO return only *.JPEG and *.TIFF mime types.
    if ( filter.contains("*.TIFF") )
        filter.append (" *.TIF");

    if ( filter.contains("*.JPEG") )
    {
        filter.append (" *.JPG");
        filter.append (" *.JPE");
    }

    // Added RAW files estentions supported by dcraw program and 
    // defines to digikam/libs/dcraw/rawfiles.h
    filter.append (" ");
    filter.append ( QString(KDcrawIface::DcrawBinary::instance()->rawFiles()) );  
    filter.append (" ");

    QString patterns = filter.toLower();
    patterns.append (" ");
    patterns.append (filter.toUpper());

    DDebug() << "patterns=" << patterns << endl;    

    // Get all image files from directory.

    QDir dir(url.path(), patterns);
    
    if (!dir.exists())
       return;
    
    // Directory items sorting. Perhaps we need to add any settings in config dialog.
    dir.setFilter ( QDir::Files | QDir::NoSymLinks );
    dir.setSorting ( QDir::Time );

    QFileInfoList fileinfolist = dir.entryInfoList();

    if (fileinfolist.isEmpty())
    {
        KMessageBox::sorry(this, i18n("There are no images in this folder."));
        return;
    }
    
    QFileInfoList::const_iterator fi;

    // And open all items in image editor.

    for (fi = fileinfolist.constBegin(); fi != fileinfolist.constEnd(); ++fi)
    {
        new Digikam::ThumbBarItem( d->thumbBar, KUrl(fi->filePath()) );
    }
}
    
void ShowFoto::slotOpenFilesInFolder()
{
    if (d->currentItem && !promptUserSave(d->currentItem->url()))
        return;

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
    if (d->currentItem && !promptUserSave(d->currentItem->url()))
        return;

    d->thumbBar->setSelected( d->thumbBar->firstItem() );
    d->currentItem = d->thumbBar->firstItem();
}

void ShowFoto::slotLast()
{
    if (d->currentItem && !promptUserSave(d->currentItem->url()))
        return;

    d->thumbBar->setSelected( d->thumbBar->lastItem() );
    d->currentItem = d->thumbBar->lastItem();
}

void ShowFoto::slotForward()
{
    if (d->currentItem && !promptUserSave(d->currentItem->url()))
        return;

    Digikam::ThumbBarItem* curr = d->thumbBar->currentItem();
    if (curr && curr->next())
    {
        d->thumbBar->setSelected(curr->next());
        d->currentItem = d->thumbBar->currentItem();
    }
}

void ShowFoto::slotBackward()
{
    if (d->currentItem && !promptUserSave(d->currentItem->url()))
        return;

    Digikam::ThumbBarItem* curr = d->thumbBar->currentItem();
    if (curr && curr->prev())
    {
        d->thumbBar->setSelected(curr->prev());
        d->currentItem = d->thumbBar->currentItem();
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
    Digikam::LoadingCacheInterface::putImage(m_savingContext->destinationURL.path(), m_canvas->currentImage());
    d->thumbBar->invalidateThumb(d->currentItem);
}

void ShowFoto::saveAsIsComplete()
{
    m_canvas->switchToLastSaved(m_savingContext->destinationURL.path());
    Digikam::LoadingCacheInterface::putImage(m_savingContext->destinationURL.path(), m_canvas->currentImage());

    // Add the file to the list of thumbbar images if it's not there already
    Digikam::ThumbBarItem* foundItem = d->thumbBar->findItemByURL(m_savingContext->destinationURL);
    d->thumbBar->invalidateThumb(foundItem);

    if (!foundItem)
        foundItem = new Digikam::ThumbBarItem(d->thumbBar, m_savingContext->destinationURL);

    // shortcut slotOpenURL
    d->thumbBar->blockSignals(true);
    d->thumbBar->setSelected(foundItem);
    d->thumbBar->blockSignals(false);
    d->currentItem = foundItem;
}

bool ShowFoto::save()
{
    if (!d->currentItem)
    {
        DWarning() << "This should not happen" << endl;
        return true;
    }

    if (!d->currentItem->url().isLocalFile())
    {
        return false;
    }

    startingSave(d->currentItem->url());
    return true;
}

bool ShowFoto::saveAs()
{
    if (!d->currentItem)
    {
        DWarning() << "This should not happen" << endl;
        return false;
    }

    return ( startingSaveAs(d->currentItem->url()) );
}

void ShowFoto::slotDeleteCurrentItem()
{
    KUrl urlCurrent(d->currentItem->url());

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
            KIO::Job* job = KIO::del( urlCurrent );
            connect( job, SIGNAL(result( KIO::Job* )),
                     this, SLOT(slotDeleteCurrentItemResult( KIO::Job*)) );
        }
    }
    else
    {
        KIO::Job* job = KIO::trash( urlCurrent );
        connect( job, SIGNAL(result( KIO::Job* )),
                 this, SLOT(slotDeleteCurrentItemResult( KIO::Job*)) );
    }
}

void ShowFoto::slotDeleteCurrentItemResult( KIO::Job * job )
{
    if (job->error() != 0)
    {
        QString errMsg(job->errorString());
        KMessageBox::error(this, errMsg);
        return;
    }

    // No error, remove item in thumbbar.

    Digikam::ThumbBarItem *item2remove = d->currentItem;
    Digikam::ThumbBarItem *nextItem    = 0;

    for (Digikam::ThumbBarItem *item = d->thumbBar->firstItem(); item; item = item->next())
    {
        if (item->url().equals(item2remove->url()))
        {
            // Find item next to the current item
            nextItem = item->next();
            d->thumbBar->removeItem(item);
            break;
        }
    }
    
    d->itemsNb = d->thumbBar->countItems();

    // Disable menu actions and SideBar if no current image.

    if ( d->itemsNb == 0 )
    {
        emit signalNoCurrentItem();
        slotUpdateItemInfo();
        toggleActions(false);
        m_canvas->load(QString(), m_IOFileSettings);
        d->currentItem = 0;
    }
    else
    {
        // If there is an image after the deleted one, make that selected.
        if (nextItem)
            d->thumbBar->setSelected(nextItem);

        d->currentItem = d->thumbBar->currentItem();
        slotOpenURL(d->currentItem->url());
    }
}

void ShowFoto::slotContextMenu()
{
    m_contextMenu->exec(QCursor::pos());
}

void ShowFoto::slideShow(bool startWithCurrent, Digikam::SlideShowSettings& settings)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("ImageViewer Settings");

    settings.exifRotate = group.readEntry("EXIF Rotate", true);
    settings.fileList   = d->thumbBar->itemsURLs();

    int   i           = 0;
    float cnt         = settings.fileList.count();
    m_cancelSlideShow = false;
    Digikam::DMetadata meta;

    m_nameLabel->progressBarMode(Digikam::StatusProgressBar::CancelProgressBarMode, 
                                 i18n("Prepare slideshow. Please wait..."));

    for (KUrl::List::Iterator it = settings.fileList.begin() ; 
         !m_cancelSlideShow && (it != settings.fileList.end()) ; ++it)
    {
        Digikam::SlidePictureInfo pictInfo;
        meta.load((*it).path());
        pictInfo.comment   = meta.getImageComment();
        pictInfo.photoInfo = meta.getPhotographInformations();
        settings.pictInfoMap.insert(*it, pictInfo);

        m_nameLabel->setProgressValue((int)((i++/cnt)*100.0));
        kapp->processEvents();
    }

    m_nameLabel->progressBarMode(Digikam::StatusProgressBar::TextMode, QString());   

    if (!m_cancelSlideShow)
    {
        Digikam::SlideShow *slide = new Digikam::SlideShow(settings);
        if (startWithCurrent)
            slide->setCurrent(d->currentItem->url());
    
        slide->show();
    }
}

void ShowFoto::slotRevert()
{
    if(!promptUserSave(d->currentItem->url()))
        return;

    m_canvas->slotRestore();
}

}   // namespace ShowFoto
