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
 * Copyright (C) 2008 by Arnd Baecker <arnd dot baecker at web dot de>
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

#include <qlabel.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qcursor.h>

// KDE includes.

#include <kcursor.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kimageio.h>
#include <kaccel.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kprotocolinfo.h>
#include <kglobalsettings.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kprogress.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000106
#include <libkdcraw/dcrawbinary.h>
#endif

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
#include "editorstackview.h"
#include "showfoto.h"
#include "showfoto.moc"

namespace ShowFoto
{

class ShowFotoPriv
{
public:

    ShowFotoPriv()
    {
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

    KURL                             lastOpenedDirectory;

    KToggleAction                   *showBarAction;

    KAction                         *openFilesInFolderAction;
    KAction                         *fileOpenAction;

    KActionMenu                     *BCGAction;

    Digikam::ThumbBarView           *thumbBar;
    Digikam::ThumbBarItem           *currentItem;
    Digikam::ImagePropertiesSideBar *rightSidebar;
    Digikam::SplashScreen           *splash;
};

ShowFoto::ShowFoto(const KURL::List& urlList)
        : Digikam::EditorWindow( "Showfoto" )
{
    d = new ShowFotoPriv();

    // -- Show splash at start ----------------------------

    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    KGlobal::dirs()->addResourceType("data", KGlobal::dirs()->kde_default("data") + "digikam");
    KGlobal::iconLoader()->addAppDir("digikam");

    if(config->readBoolEntry("ShowSplash", true) && !kapp->isRestored())
    {
        d->splash = new Digikam::SplashScreen("showfoto-splash.png");
        d->splash->show();
    }

    // Check ICC profiles repository availability

    if(d->splash)
        d->splash->message(i18n("Checking ICC repository"));

    d->validIccPath = Digikam::SetupICC::iccRepositoryIsValid();

#if KDCRAW_VERSION < 0x000106
    // Check witch dcraw version available

    if(d->splash)
        d->splash->message(i18n("Checking dcraw version"));

    KDcrawIface::DcrawBinary::instance()->checkSystem();
#endif

    // Populate Themes

    if(d->splash)
        d->splash->message(i18n("Loading themes"));

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
        d->BCGAction = new KActionMenu(i18n("Brightness/Contrast/Gamma"), 0, 0, "showfoto_bcg");
        d->BCGAction->setDelayed(false);

        KAction *incGammaAction = new KAction(i18n("Increase Gamma"), 0, ALT+Key_G,
                                            this, SLOT(slotChangeBCG()),
                                            actionCollection(), "gamma_plus");
        KAction *decGammaAction = new KAction(i18n("Decrease Gamma"), 0, ALT+SHIFT+Key_G,
                                            this, SLOT(slotChangeBCG()),
                                            actionCollection(), "gamma_minus");
        KAction *incBrightAction = new KAction(i18n("Increase Brightness"), 0, ALT+Key_B,
                                            this, SLOT(slotChangeBCG()),
                                            actionCollection(), "brightness_plus");
        KAction *decBrightAction = new KAction(i18n("Decrease Brightness"), 0, ALT+SHIFT+Key_B,
                                            this, SLOT(slotChangeBCG()),
                                            actionCollection(), "brightness_minus");
        KAction *incContrastAction = new KAction(i18n("Increase Contrast"), 0, ALT+Key_C,
                                            this, SLOT(slotChangeBCG()),
                                            actionCollection(), "contrast_plus");
        KAction *decContrastAction = new KAction(i18n("Decrease Contrast"), 0, ALT+SHIFT+Key_C,
                                            this, SLOT(slotChangeBCG()),
                                            actionCollection(), "contrast_minus");

        d->BCGAction->insert(incBrightAction);
        d->BCGAction->insert(decBrightAction);
        d->BCGAction->insert(incContrastAction);
        d->BCGAction->insert(decContrastAction);
        d->BCGAction->insert(incGammaAction);
        d->BCGAction->insert(decGammaAction);

        QPtrList<KAction> bcg_actions;
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

    for (KURL::List::const_iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        KURL url = *it;
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
    delete d;
}

Digikam::Sidebar* ShowFoto::rightSideBar() const
{
    return dynamic_cast<Digikam::Sidebar*>(d->rightSidebar);
}

bool ShowFoto::queryClose()
{
    // wait if a save operation is currently running
    if (!waitForSavingToComplete())
        return false;

    if (d->currentItem && !promptUserSave(d->currentItem->url()))
        return false;

    // put right side bar in a defined state
    emit signalNoCurrentItem();
    m_canvas->resetImage();

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

    KMainWindow::show();

    // Report errors from ICC repository path.

    KConfig* config = kapp->config();
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
                config->setGroup("Color Management");
                config->writeEntry("EnableCM", false);
                config->sync();
            }
        }
        else
        {
            config->setGroup("Color Management");
            config->writeEntry("EnableCM", false);
            config->sync();
        }
    }

#if KDCRAW_VERSION < 0x000106
    // Report errors from dcraw detection.

    KDcrawIface::DcrawBinary::instance()->checkReport();
#endif
}

void ShowFoto::setupConnections()
{
    setupStandardConnections();

    connect(d->thumbBar, SIGNAL(signalURLSelected(const KURL&)),
            this, SLOT(slotOpenURL(const KURL&)));

    connect(d->thumbBar, SIGNAL(signalItemAdded()),
            this, SLOT(slotUpdateItemInfo()));

    connect(this, SIGNAL(signalSelectionChanged(const QRect &)),
            d->rightSidebar, SLOT(slotImageSelectionChanged(const QRect &)));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSidebar, SLOT(slotNoCurrentItem()));
}

void ShowFoto::setupUserArea()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    QWidget* widget = new QWidget(this);
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);

    if(!config->readBoolEntry("HorizontalThumbbar", false)) // Vertical thumbbar layout
    {
        QHBoxLayout *hlay = new QHBoxLayout(widget);
        m_splitter        = new QSplitter(widget);
        d->thumbBar       = new Digikam::ThumbBarView(m_splitter, Digikam::ThumbBarView::Vertical);
        m_stackView       = new Digikam::EditorStackView(m_splitter);
        m_canvas          = new Digikam::Canvas(m_stackView);
        m_canvas->setSizePolicy(rightSzPolicy);

        d->rightSidebar   = new Digikam::ImagePropertiesSideBar(widget, "ShowFoto Sidebar Right", m_splitter,
                                                                Digikam::Sidebar::Right);

        hlay->addWidget(m_splitter);
        hlay->addWidget(d->rightSidebar);
    }
    else                                                     // Horizontal thumbbar layout
    {
        m_splitter        = new QSplitter(Qt::Horizontal, widget);
        QWidget* widget2  = new QWidget(m_splitter);
        QVBoxLayout *vlay = new QVBoxLayout(widget2);
        d->vSplitter      = new QSplitter(Qt::Vertical, widget2);
        m_stackView       = new Digikam::EditorStackView(d->vSplitter);
        m_canvas          = new Digikam::Canvas(m_stackView);
        d->thumbBar       = new Digikam::ThumbBarView(d->vSplitter, Digikam::ThumbBarView::Horizontal);

        m_canvas->setSizePolicy(rightSzPolicy);

        d->vSplitter->setFrameStyle( QFrame::NoFrame );
        d->vSplitter->setFrameShadow( QFrame::Plain );
        d->vSplitter->setFrameShape( QFrame::NoFrame );
        d->vSplitter->setOpaqueResize(false);

        vlay->addWidget(d->vSplitter);

        QHBoxLayout *hlay = new QHBoxLayout(widget);
        d->rightSidebar   = new Digikam::ImagePropertiesSideBar(widget, "ShowFoto Sidebar Right", m_splitter,
                                                                Digikam::Sidebar::Right);
        hlay->addWidget(m_splitter);
        hlay->addWidget(d->rightSidebar);
    }

    m_canvas->makeDefaultEditingCanvas();
    m_stackView->setCanvas(m_canvas);
    m_stackView->setViewMode(Digikam::EditorStackView::CanvasMode);

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

    d->fileOpenAction = KStdAction::open(this, SLOT(slotOpenFile()),
                        actionCollection(), "showfoto_open_file");

    d->openFilesInFolderAction = new KAction(i18n("Open folder"),
                                             "folder_image",
                                             CTRL+SHIFT+Key_O,
                                             this,
                                             SLOT(slotOpenFilesInFolder()),
                                             actionCollection(),
                                             "showfoto_open_folder");

    KStdAction::quit(this, SLOT(close()), actionCollection(), "showfoto_quit");

    // Extra 'View' menu actions ---------------------------------------------

    d->showBarAction = new KToggleAction(i18n("Show Thumbnails"), 0,
                                         CTRL+Key_T,
                                         this, SLOT(slotToggleShowBar()),
                                         actionCollection(), "shofoto_showthumbs");

    // --- Create the gui --------------------------------------------------------------

    createGUI("showfotoui.rc", false);

    setupStandardAccelerators();
}

void ShowFoto::readSettings()
{
    readStandardSettings();

    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    d->showBarAction->setChecked(config->readBoolEntry("Show Thumbnails", true));
    slotToggleShowBar();

    d->lastOpenedDirectory.setPath( config->readEntry("Last Opened Directory",
                                    KGlobalSettings::documentPath()) );

    QSizePolicy szPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    if(config->hasKey("Vertical Splitter Sizes") && d->vSplitter)
        d->vSplitter->setSizes(config->readIntListEntry("Vertical Splitter Sizes"));
    else
        m_canvas->setSizePolicy(szPolicy);

    Digikam::ThemeEngine::instance()->setCurrentTheme(config->readEntry("Theme", i18n("Default")));
}

void ShowFoto::saveSettings()
{
    saveStandardSettings();

    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    config->writeEntry("Last Opened Directory", d->lastOpenedDirectory.path() );
    config->writeEntry("Show Thumbnails", d->showBarAction->isChecked());

    if (d->vSplitter)
        config->writeEntry("Vertical Splitter Sizes", d->vSplitter->sizes());

    config->writeEntry("Theme", Digikam::ThemeEngine::instance()->getCurrentThemeName());

    config->sync();
}

void ShowFoto::applySettings()
{
    applyStandardSettings();

    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    // Current image deleted go to trash ?
    d->deleteItem2Trash = config->readBoolEntry("DeleteItem2Trash", true);
    if (d->deleteItem2Trash)
    {
        m_fileDeleteAction->setIcon("edittrash");
        m_fileDeleteAction->setText(i18n("Move to Trash"));
    }
    else
    {
        m_fileDeleteAction->setIcon("editdelete");
        m_fileDeleteAction->setText(i18n("Delete File"));
    }

    bool exifRotate = config->readBoolEntry("EXIF Rotate", true);
    m_canvas->setExifOrient(exifRotate);
    d->thumbBar->setExifRotate(exifRotate);

    m_setExifOrientationTag   = config->readBoolEntry("EXIF Set Orientation", true);

    d->fullScreenHideThumbBar = config->readBoolEntry("FullScreenHideThumbBar", true);

    Digikam::ThumbBarToolTipSettings settings;
    settings.showToolTips   = config->readBoolEntry("Show ToolTips", true);
    settings.showFileName   = config->readBoolEntry("ToolTips Show File Name", true);
    settings.showFileDate   = config->readBoolEntry("ToolTips Show File Date", false);
    settings.showFileSize   = config->readBoolEntry("ToolTips Show File Size", false);
    settings.showImageType  = config->readBoolEntry("ToolTips Show Image Type", false);
    settings.showImageDim   = config->readBoolEntry("ToolTips Show Image Dim", true);
    settings.showPhotoMake  = config->readBoolEntry("ToolTips Show Photo Make", true);
    settings.showPhotoDate  = config->readBoolEntry("ToolTips Show Photo Date", true);
    settings.showPhotoFocal = config->readBoolEntry("ToolTips Show Photo Focal", true);
    settings.showPhotoExpo  = config->readBoolEntry("ToolTips Show Photo Expo", true);
    settings.showPhotoMode  = config->readBoolEntry("ToolTips Show Photo Mode", true);
    settings.showPhotoFlash = config->readBoolEntry("ToolTips Show Photo Flash", false);
    settings.showPhotoWB    = config->readBoolEntry("ToolTips Show Photo WB", false);
    d->thumbBar->setToolTipSettings(settings);
}

void ShowFoto::slotOpenFile()
{
    if (d->currentItem && !promptUserSave(d->currentItem->url()))
        return;

    KURL::List urls = Digikam::ImageDialog::getImageURLs(this, d->lastOpenedDirectory);

    if (!urls.isEmpty())
    {
        d->currentItem = 0;
        d->thumbBar->clear();

        for (KURL::List::const_iterator it = urls.begin();
             it != urls.end(); ++it)
        {
            new Digikam::ThumbBarItem(d->thumbBar, *it);
            d->lastOpenedDirectory=(*it);
        }

        toggleActions(true);
    }
}

void ShowFoto::slotOpenURL(const KURL& url)
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
#if KDE_IS_VERSION(3,2,0)
    KIO::NetAccess::download(url, localFile, this);
#else
    KIO::NetAccess::download(url, localFile);
#endif

    m_canvas->load(localFile, m_IOFileSettings);

    // TODO : add preload here like in ImageWindow::slotLoadCurrent() ???
}

void ShowFoto::toggleGUI2FullScreen()
{
    if (m_fullScreen)
    {
        d->rightSidebar->restore();

        // If show Thumbbar option is checked, restore it.
        if (d->showBarAction->isChecked())
            d->thumbBar->show();
    }
    else
    {
        d->rightSidebar->backup();

        // If Hide Thumbbar option is checked, catch it if necessary.
        if (d->showBarAction->isChecked())
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
        name = sender()->name();

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
    QString str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)")
                  .arg(dims.width()).arg(dims.height()).arg(mpixels);
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

    kapp->config()->sync();

    applySettings();

    if ( d->itemsNb == 0 )
    {
        slotUpdateItemInfo();
        toggleActions(false);
    }

    return true;
}

void ShowFoto::slotUpdateItemInfo(void)
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

        text = d->currentItem->url().filename() +
                   i18n(" (%2 of %3)")
                   .arg(QString::number(index))
                   .arg(QString::number(d->itemsNb));

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

void ShowFoto::slotOpenFolder(const KURL& url)
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

void ShowFoto::openFolder(const KURL& url)
{
    if (!url.isValid() || !url.isLocalFile())
       return;

    // Parse KDE image IO mime types registration to get files filter pattern.

    QStringList mimeTypes = KImageIO::mimeTypes(KImageIO::Reading);
    QString filter;

    for (QStringList::ConstIterator it = mimeTypes.begin() ; it != mimeTypes.end() ; ++it)
    {
        QString format = KImageIO::typeForMime(*it);
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
#if KDCRAW_VERSION < 0x000106
    filter.append ( QString(KDcrawIface::DcrawBinary::instance()->rawFiles()) );
#else
    filter.append ( QString(KDcrawIface::KDcraw::rawFiles()) );
#endif
    filter.append (" ");

    QString patterns = filter.lower();
    patterns.append (" ");
    patterns.append (filter.upper());

    DDebug() << "patterns=" << patterns << endl;

    // Get all image files from directory.

    QDir dir(url.path(), patterns);
    dir.setFilter ( QDir::Files | QDir::NoSymLinks );

    if (!dir.exists())
       return;

    // Determine sort ordering for the entries from configuration setting:

    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    int flag;

    switch(config->readNumEntry("SortOrder", 0))
    {
        case 1:
            flag = QDir::Name;  // Ordering by file name.
            break;
        case 2:
            flag = QDir::Size;  // Ordering by file size.
            break;
        default:
            flag = QDir::Time;  // Ordering by file date.
            break;
    }

    // Disabled reverse in the settings leads e.g. to increasing dates
    // Note, that this is just the opposite to the sort order for QDir.

    if (!config->readBoolEntry("ReverseSort", false))
        flag = flag | QDir::Reversed;

    dir.setSorting(flag);

    const QFileInfoList* fileinfolist = dir.entryInfoList();
    if (!fileinfolist || fileinfolist->isEmpty())
    {
        KMessageBox::sorry(this, i18n("There are no images in this folder."));
        return;
    }

    QFileInfoListIterator it(*fileinfolist);
    QFileInfo* fi;

    // And open all items in image editor.

    while( (fi = it.current() ) )
    {
        new Digikam::ThumbBarItem( d->thumbBar, KURL(fi->filePath()) );
        ++it;
    }
}

void ShowFoto::slotOpenFilesInFolder()
{
    if (d->currentItem && !promptUserSave(d->currentItem->url()))
        return;

    KURL url(KFileDialog::getExistingDirectory(d->lastOpenedDirectory.directory(),
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
        DWarning() << k_funcinfo << "This should not happen" << endl;
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
        DWarning() << k_funcinfo << "This should not happen" << endl;
        return false;
    }

    return ( startingSaveAs(d->currentItem->url()) );
}

void ShowFoto::slotDeleteCurrentItem()
{
    KURL urlCurrent(d->currentItem->url());

    if (!d->deleteItem2Trash)
    {
        QString warnMsg(i18n("About to delete file \"%1\"\nAre you sure?")
                        .arg(urlCurrent.filename()));
        if (KMessageBox::warningContinueCancel(this,
                                               warnMsg,
                                               i18n("Warning"),
                                               i18n("Delete"))
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
        KURL dest("trash:/");

        if (!KProtocolInfo::isKnownProtocol(dest))
        {
            dest = KGlobalSettings::trashPath();
        }

        KIO::Job* job = KIO::move( urlCurrent, dest );
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
            d->currentItem = 0;
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
    if (!d->thumbBar->countItems()) return;

    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    settings.exifRotate = config->readBoolEntry("EXIF Rotate", true);
    settings.fileList   = d->thumbBar->itemsURLs();

    int       i   = 0;
    float     cnt = settings.fileList.count();
    Digikam::DMetadata meta;
    m_cancelSlideShow = false;

    m_nameLabel->progressBarMode(Digikam::StatusProgressBar::CancelProgressBarMode,
                                 i18n("Preparing slideshow. Please wait..."));

    for (KURL::List::Iterator it = settings.fileList.begin() ;
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
