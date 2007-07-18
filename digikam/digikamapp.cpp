/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation
 * 
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu> 
 * Copyright (C)      2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2002-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <Q3DockArea>
#include <QDataStream>
#include <QLabel>
#include <QStringList>
#include <QSignalMapper>
#include <QtDBus>


// KDE includes.

#include <khbox.h>
#include <kactioncollection.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kshortcutsdialog.h>
#include <ktoggleaction.h>
#include <kedittoolbar.h>
#include <kiconloader.h>
#include <ktip.h>
#include <kdeversion.h>
#include <kapplication.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kwindowsystem.h>
#include <kactionmenu.h>
#include <kglobal.h>
#include <ktoolinvocation.h>
#include <ktoolbarpopupaction.h>
#include <digikamadaptor.h>

// libKipi includes.

#include <libkipi/plugin.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawbinary.h>

// Local includes.

#include "ddebug.h"
#include "album.h"
#include "albumlister.h"
#include "albumthumbnailloader.h"
#include "cameratype.h"
#include "cameraui.h"
#include "setup.h"
#include "setupplugins.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "lighttablewindow.h"
#include "imagewindow.h"
#include "imageinfo.h"
#include "thumbnailsize.h"
#include "themeengine.h"
#include "scanlib.h"
#include "loadingcache.h"
#include "loadingcacheinterface.h"
#include "imageattributeswatch.h"
#include "batchthumbsgenerator.h"
#include "batchalbumssyncmetadata.h"
#include "digikamappprivate.h"
#include "digikamapp.h"
#include "digikamapp.moc"

using KIO::Job;
using KIO::UDSEntryList;
using KIO::UDSEntry;

namespace Digikam
{

DigikamApp* DigikamApp::m_instance = 0;

DigikamApp::DigikamApp()
          : KXmlGuiWindow(0)
{
    d = new DigikamAppPriv;
    m_instance = this;
    d->config  = KGlobal::config();

    new DigikamAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/Digikam", this);

    setObjectName("Digikam");

    if(d->config->group("General Settings").readEntry("Show Splash", true) &&
       !kapp->isSessionRestored())
    {
        d->splashScreen = new SplashScreen("digikam-splash.png");
        d->splashScreen->show();
    }

    d->albumSettings = new AlbumSettings();
    d->albumSettings->readSettings();

    d->albumManager = AlbumManager::componentData();
    AlbumLister::componentData();

    LoadingCache::cache();

    d->cameraMediaList = new KMenu;

    connect(d->cameraMediaList, SIGNAL( aboutToShow() ),
            this, SLOT(slotCameraMediaMenu()));

    d->cameraList = new CameraList(this, KStandardDirs::locateLocal("appdata", "cameras.xml"));

    connect(d->cameraList, SIGNAL(signalCameraAdded(CameraType *)),
            this, SLOT(slotCameraAdded(CameraType *)));

    connect(d->cameraList, SIGNAL(signalCameraRemoved(CameraType *)),
            this, SLOT(slotCameraRemoved(CameraType *)));

    setupView();
    setupStatusBar();
    setupAccelerators();
    setupActions();

    applyMainWindowSettings(d->config->group("General Settings"));

    // Check ICC profiles repository availability

    if(d->splashScreen)
        d->splashScreen->message(i18n("Checking ICC repository"), Qt::AlignLeft, Qt::white);

    d->validIccPath = SetupICC::iccRepositoryIsValid();

    // Check witch dcraw version available

    if(d->splashScreen)
        d->splashScreen->message(i18n("Checking dcraw version"), Qt::AlignLeft, Qt::white);

    KDcrawIface::DcrawBinary::componentData()->checkSystem();

    // Actual file scanning is done in main() - is this necessary here?
    //d->albumManager->setLibraryPath(d->albumSettings->getAlbumLibraryPath());

    // Read albums from database
    if(d->splashScreen)
        d->splashScreen->message(i18n("Reading database"), Qt::AlignLeft, Qt::white);

    d->albumManager->startScan();

    // Load KIPI Plugins.
    loadPlugins();

    // Load Themes
    populateThemes();

    setAutoSaveSettings();
}

DigikamApp::~DigikamApp()
{
    ImageAttributesWatch::shutDown();

    // Close and delete image editor instance.

    if (ImageWindow::imagewindowCreated())
        ImageWindow::imagewindow()->close();

    // Close and delete light table instance.
        
    if (LightTableWindow::lightTableWindowCreated())
        LightTableWindow::lightTableWindow()->close();

    if (d->view)
        delete d->view;

    d->albumSettings->saveSettings();
    delete d->albumSettings;

    delete d->albumManager;
    delete AlbumLister::componentData();

    ImageAttributesWatch::cleanUp();
    LoadingCacheInterface::cleanUp();
    KDcrawIface::DcrawBinary::cleanUp();
    AlbumThumbnailLoader::cleanUp();

    m_instance = 0;

    delete d;
}

DigikamApp* DigikamApp::getinstance()
{
    return m_instance;
}

void DigikamApp::show()
{
    // Remove Splashscreen.

    if(d->splashScreen)
    {
        d->splashScreen->finish(this);
        delete d->splashScreen;
        d->splashScreen = 0;
    }

    // Display application window.

    KMainWindow::show();

    // Report errors from ICC repository path.

    if(!d->validIccPath)
    {
        QString message = i18n("<qt><p>ICC profiles path seems to be invalid.</p>"
                               "<p>If you want to set it now, select \"Yes\", otherwise "
                               "select \"No\". In this case, \"Color Management\" feature "
                               "will be disabled until you solve this issue</p></qt>");

        if (KMessageBox::warningYesNo(this, message) == KMessageBox::Yes)
        {
            if (!setup(true))
            {
                d->config->group("Color Management").writeEntry("EnableCM", false);
                d->config->sync();
            }
        }
        else
        {
            d->config->group("Color Management").writeEntry("EnableCM", false);
            d->config->sync();
        }
    }

    // Report errors from dcraw detection.

    KDcrawIface::DcrawBinary::componentData()->checkReport();

    // Init album icon view zoom factor. 
    slotThumbSizeChanged(d->albumSettings->getDefaultIconSize());
    slotZoomSliderChanged(d->albumSettings->getDefaultIconSize());
}

const QList<QAction*>& DigikamApp::menuImageActions()
{
    return d->kipiImageActions;
}

const QList<QAction*>& DigikamApp::menuBatchActions()
{
    return d->kipiBatchActions;
}

const QList<QAction*>& DigikamApp::menuAlbumActions()
{
    return d->kipiAlbumActions;
}

QList<QAction*> DigikamApp::menuImportActions()
{
    QList<QAction*> importMenu;
    importMenu = d->kipiFileActionsImport;
    importMenu.append( d->albumImportAction );
    importMenu.append( d->addImagesAction );
    return importMenu;
}

const QList<QAction*>& DigikamApp::menuExportActions()
{
    return d->kipiFileActionsExport;
}

void DigikamApp::autoDetect()
{
    // Called from main if command line option is set

    if(d->splashScreen)
        d->splashScreen->message(i18n("Auto-detect camera"), Qt::AlignLeft, Qt::white);

    QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
}

void DigikamApp::downloadFrom(const QString &cameraGuiPath)
{
    // Called from main if command line option is set

    if (!cameraGuiPath.isNull())
    {
        d->cameraGuiPath = cameraGuiPath;

        if(d->splashScreen)
            d->splashScreen->message(i18n("Opening Download Dialog"), Qt::AlignLeft, Qt::white);

        QTimer::singleShot(0, this, SLOT(slotDownloadImages()));
    }
}

bool DigikamApp::queryClose()
{
    if (ImageWindow::imagewindowCreated())
    {
        return ImageWindow::imagewindow()->queryClose();
    }
    else
        return true;
}

void DigikamApp::setupView()
{
    if(d->splashScreen)
        d->splashScreen->message(i18n("Initializing Main View"), Qt::AlignLeft, Qt::white);

    d->view = new DigikamView(this);
    setCentralWidget(d->view);
    d->view->applySettings(d->albumSettings);

    connect(d->view, SIGNAL(signalAlbumSelected(bool)),
            this, SLOT(slotAlbumSelected(bool)));
            
    connect(d->view, SIGNAL(signalTagSelected(bool)),
            this, SLOT(slotTagSelected(bool)));

    connect(d->view, SIGNAL(signalImageSelected(const Q3PtrList<ImageInfo>&, bool, bool)),
            this, SLOT(slotImageSelected(const Q3PtrList<ImageInfo>&, bool, bool)));
}

void DigikamApp::setupStatusBar()
{
    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    d->statusProgressBar->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(d->statusProgressBar, 100);

    //------------------------------------------------------------------------------

    d->statusZoomBar = new StatusZoomBar(statusBar());
    statusBar()->addWidget(d->statusZoomBar, 1);

    //------------------------------------------------------------------------------

    d->statusNavigateBar = new StatusNavigateBar(statusBar());
    d->statusNavigateBar->setMaximumHeight(fontMetrics().height()+2);
    statusBar()->addWidget(d->statusNavigateBar, 1);

    //------------------------------------------------------------------------------

    connect(d->statusZoomBar, SIGNAL(signalZoomMinusClicked()),
            d->view, SLOT(slotZoomOut()));

    connect(d->statusZoomBar, SIGNAL(signalZoomPlusClicked()),
            d->view, SLOT(slotZoomIn()));

    connect(d->statusZoomBar, SIGNAL(signalZoomSliderChanged(int)),
            this, SLOT(slotZoomSliderChanged(int)));

    connect(d->view, SIGNAL(signalThumbSizeChanged(int)),
            this, SLOT(slotThumbSizeChanged(int)));

    connect(d->view, SIGNAL(signalZoomChanged(double, int)),
            this, SLOT(slotZoomChanged(double, int)));
    
    connect(d->view, SIGNAL(signalTogglePreview(bool)),
            this, SLOT(slotTooglePreview(bool)));

    connect(d->statusNavigateBar, SIGNAL(signalFirstItem()),
            d->view, SLOT(slotFirstItem()));

    connect(d->statusNavigateBar, SIGNAL(signalNextItem()),
            d->view, SLOT(slotNextItem()));

    connect(d->statusNavigateBar, SIGNAL(signalPrevItem()),
            d->view, SLOT(slotPrevItem()));

    connect(d->statusNavigateBar, SIGNAL(signalLastItem()),
            d->view, SLOT(slotLastItem()));

    connect(d->statusProgressBar, SIGNAL(signalCancelButtonPressed()),
            this, SIGNAL(signalCancelButtonPressed()));
}

void DigikamApp::setupAccelerators()
{
#warning "TODO: kde4 port it";
/*  // TODO: KDE4PORT: use KAction/QAction framework instead KAccel

    d->accelerators = new KAccel(this);

    d->accelerators->insert("Exit Preview Mode", i18n("Exit Preview"),
                           i18n("Exit preview mode"),
                           Qt::Key_Escape, this, SIGNAL(signalEscapePressed()),
                           false, true);
    
    d->accelerators->insert("Next Image Qt::Key_Space", i18n("Next Image"),
                           i18n("Next Image"),
                           Qt::Key_Space, this, SIGNAL(signalNextItem()),
                           false, true);

    d->accelerators->insert("Next Image SHIFT+Qt::Key_Space", i18n("Next Image"),
                           i18n("Next Image"),
                           SHIFT+Qt::Key_Space, this, SIGNAL(signalNextItem()),
                           false, true);

    d->accelerators->insert("Previous Image Qt::Key_Backspace", i18n("Previous Image"),
                           i18n("Previous Image"),
                           Qt::Key_Backspace, this, SIGNAL(signalPrevItem()),
                           false, true);

    d->accelerators->insert("Next Image Qt::Key_Next", i18n("Next Image"),
                           i18n("Next Image"),
                           Qt::Key_Next, this, SIGNAL(signalNextItem()),
                           false, true);

    d->accelerators->insert("Previous Image Prior", i18n("Previous Image"),
                           i18n("Previous Image"),
                           Qt::Key_Prior, this, SIGNAL(signalPrevItem()),
                           false, true);

    d->accelerators->insert("First Image Home", i18n("First Image"),
                           i18n("First Image"),
                           Qt::Key_Home, this, SIGNAL(signalFirstItem()),
                           false, true);

    d->accelerators->insert("Last Image End", i18n("Last Image"),
                           i18n("Last Image"),
                           Qt::Key_End, this, SIGNAL(signalLastItem()),
                           false, true);

    d->accelerators->insert("Copy Album Items Selection CTRL+Key_C", i18n("Copy Album Items Selection"),
                           i18n("Copy Album Items Selection"),
                           Qt::CTRL+Qt::Key_C, this, SIGNAL(signalCopyAlbumItemsSelection()),
                           false, true);

    d->accelerators->insert("Paste Album Items Selection CTRL+Key_V", i18n("Paste Album Items Selection"),
                           i18n("Paste Album Items Selection"),
                           Qt::CTRL+Qt::Key_V, this, SIGNAL(signalPasteAlbumItemsSelection()),
                           false, true);
*/
}

void DigikamApp::setupActions()
{
    // -----------------------------------------------------------------

    d->cameraMenuAction = new KActionMenu(i18n("&Camera"), this);
    d->cameraMenuAction->setDelayed(false);
    actionCollection()->addAction("camera_menu", d->cameraMenuAction);

    // -----------------------------------------------------------------

    d->themeMenuAction = new KSelectAction(i18n("&Themes"), this);
    connect(d->themeMenuAction, SIGNAL(triggered(const QString&)), this, SLOT(slotChangeTheme(const QString&)));
    actionCollection()->addAction("theme_menu", d->themeMenuAction);

    // -----------------------------------------------------------------

    d->backwardActionMenu = new KToolBarPopupAction(KIcon("go-previous"), i18n("&Back"), this);
    d->backwardActionMenu->setEnabled(false);
    d->backwardActionMenu->setShortcut(Qt::ALT+Qt::Key_Left);
    connect(d->backwardActionMenu, SIGNAL(triggered()), d->view, SLOT(slotAlbumHistoryBack()));
    actionCollection()->addAction("album_back", d->backwardActionMenu);

    connect(d->backwardActionMenu->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowBackwardMenu()));
    
    // TODO: KDE4PORT: this activated(int) have been replaced by triggered(QAction *)
    connect(d->backwardActionMenu->menu(), SIGNAL(activated(int)),
            d->view, SLOT(slotAlbumHistoryBack(int)));

    // -----------------------------------------------------------------

    d->forwardActionMenu = new KToolBarPopupAction(KIcon("go-next"), i18n("Forward"), this);
    d->forwardActionMenu->setEnabled(false);
    d->forwardActionMenu->setShortcut(Qt::ALT+Qt::Key_Right);
    connect(d->forwardActionMenu, SIGNAL(triggered()), d->view, SLOT(slotAlbumHistoryForward()));
    actionCollection()->addAction("album_forward", d->forwardActionMenu);

    connect(d->forwardActionMenu->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowForwardMenu()));
    
    // TODO: KDE4PORT: this activated(int) have been replaced by triggered(QAction *)
    connect(d->forwardActionMenu->menu(), SIGNAL(activated(int)),
            d->view, SLOT(slotAlbumHistoryForward(int)));

    // -----------------------------------------------------------------

    d->newAction = new KAction(KIcon("albumfolder-new"), i18n("&New Album..."), this);
    d->newAction->setShortcut(KStandardShortcut::New);
    d->newAction->setWhatsThis(i18n("Creates a new empty Album in the database."));
    connect(d->newAction, SIGNAL(triggered()), d->view, SLOT(slotNewAlbum()));
    actionCollection()->addAction("album_new", d->newAction);

    // -----------------------------------------------------------------

    d->albumSortAction = new KSelectAction(i18n("&Sort Albums"), this);
    connect(d->albumSortAction, SIGNAL(triggered(int)), d->view, SLOT(slotSortAlbums(int)));
    actionCollection()->addAction("album_sort", d->albumSortAction);

    // Use same list order as in albumsettings enum
    QStringList sortActionList;
    sortActionList.append(i18n("By Folder"));
    sortActionList.append(i18n("By Collection"));
    sortActionList.append(i18n("By Date"));
    d->albumSortAction->setItems(sortActionList);

    // -----------------------------------------------------------------

    d->deleteAction = new KAction(KIcon("edit-delete"), i18n("Delete Album"), this);
    connect(d->deleteAction, SIGNAL(triggered()), d->view, SLOT(slotDeleteAlbum()));
    actionCollection()->addAction("album_delete", d->deleteAction);

    // -----------------------------------------------------------------

    d->addImagesAction = new KAction(KIcon("albumfolder-importimages"), i18n("Add Images..."), this);
    d->addImagesAction->setShortcut(Qt::CTRL+Qt::Key_I);
    d->addImagesAction->setWhatsThis(i18n("Adds new items to the current Album."));
    connect(d->addImagesAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumAddImages()));
    actionCollection()->addAction("album_addImages", d->addImagesAction);

    // -----------------------------------------------------------------

    d->albumImportAction = new KAction(KIcon("albumfolder-importdir"), i18n("Import Folders..."), this);
    connect(d->albumImportAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumImportFolder()));
    actionCollection()->addAction("album_importFolder", d->albumImportAction);

    // -----------------------------------------------------------------

    d->propsEditAction = new KAction(KIcon("albumfolder-properties"), i18n("Edit Album Properties..."), this);
    d->propsEditAction->setWhatsThis(i18n("Edit Album Properties and Collection information."));
    connect(d->propsEditAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumPropsEdit()));
    actionCollection()->addAction("album_propsEdit", d->propsEditAction);

    // -----------------------------------------------------------------

    d->refreshAlbumAction = new KAction(KIcon("view-refresh"), i18n("Refresh"), this);
    d->refreshAlbumAction->setShortcut(Qt::Key_F5);
    d->refreshAlbumAction->setWhatsThis(i18n("Refresh all album contents."));
    connect(d->refreshAlbumAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumRefresh()));
    actionCollection()->addAction("album_refresh", d->refreshAlbumAction);

    // -----------------------------------------------------------------

    d->syncAlbumMetadataAction = new KAction(KIcon("rebuild"), i18n("Synchronize images with database"), this);
    d->syncAlbumMetadataAction->setWhatsThis(i18n("Updates all image metadata of the current "
                                                  "album with digiKam database contents "
                                                  "(image metadata will be over-written with data from "
                                                  "the database)."));
    connect(d->syncAlbumMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumSyncPicturesMetadata()));
    actionCollection()->addAction("album_syncmetadata", d->syncAlbumMetadataAction);

    // -----------------------------------------------------------------

    d->openInKonquiAction = new KAction(KIcon("konqueror"), i18n("Open in Konqueror"), this);
    connect(d->openInKonquiAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumOpenInKonqui()));
    actionCollection()->addAction("album_openinkonqui", d->openInKonquiAction);

    // -----------------------------------------------------------

    d->newTagAction = new KAction(KIcon("tag-new"), i18n("New &Tag..."), this);
    connect(d->newTagAction, SIGNAL(triggered()), d->view, SLOT(slotNewTag()));
    actionCollection()->addAction("tag_new", d->newTagAction);

    // -----------------------------------------------------------

    d->editTagAction = new KAction(KIcon("tag-properties"), i18n("Edit Tag Properties..."), this);
    connect(d->editTagAction, SIGNAL(triggered()), d->view, SLOT(slotEditTag()));
    actionCollection()->addAction("tag_edit", d->editTagAction);

    // -----------------------------------------------------------

    d->deleteTagAction = new KAction(KIcon("tag-delete"), i18n("Delete Tag"), this);
    connect(d->deleteTagAction, SIGNAL(triggered()), d->view, SLOT(slotDeleteTag()));
    actionCollection()->addAction("tag_delete", d->deleteTagAction);

    // -----------------------------------------------------------

    d->imagePreviewAction = new KToggleAction(KIcon("fileview-preview"), i18n("View..."), this);
    d->imagePreviewAction->setShortcut(Qt::Key_F3);
    connect(d->imagePreviewAction, SIGNAL(triggered()), d->view, SLOT(slotImagePreview()));
    actionCollection()->addAction("image_view", d->imagePreviewAction);

    // -----------------------------------------------------------

    d->imageViewAction = new KAction(KIcon("editimage"), i18n("Edit..."), this);
    d->imageViewAction->setShortcut(Qt::Key_F4);
    d->imageViewAction->setWhatsThis(i18n("Open the selected item in the image editor."));
    connect(d->imageViewAction, SIGNAL(triggered()), d->view, SLOT(slotImageEdit()));
    actionCollection()->addAction("image_edit", d->imageViewAction);

    // -----------------------------------------------------------

    d->imageLightTableAction = new KAction(KIcon("lighttable"), i18n("Place onto Light Table"), this);
    d->imageLightTableAction->setShortcut(Qt::Key_F6);
    d->imageLightTableAction->setWhatsThis(i18n("Insert the selected items into the light table thumbbar."));
    connect(d->imageLightTableAction, SIGNAL(triggered()), d->view, SLOT(slotImageLightTable()));
    actionCollection()->addAction("image_lighttable", d->imageLightTableAction);

    // -----------------------------------------------------------

    d->imageRenameAction = new KAction(KIcon("pencil"), i18n("Rename..."), this);
    d->imageRenameAction->setShortcut(Qt::Key_F2);
    d->imageRenameAction->setWhatsThis(i18n("Rename the filename of the currently selected item."));
    connect(d->imageRenameAction, SIGNAL(triggered()), d->view, SLOT(slotImageRename()));
    actionCollection()->addAction("image_rename", d->imageRenameAction);

    // -----------------------------------------------------------

    // Pop up dialog to ask user whether to move to trash
    d->imageDeleteAction = new KAction(KIcon("edit-trash"), i18n("Delete"), this);
    d->imageDeleteAction->setShortcut(Qt::Key_Delete);
    connect(d->imageDeleteAction, SIGNAL(triggered()), d->view, SLOT(slotImageDelete()));
    actionCollection()->addAction("image_delete", d->imageDeleteAction);

    // -----------------------------------------------------------

    // Pop up dialog to ask user whether to permanently delete
    d->imageDeletePermanentlyAction = new KAction(KIcon("edit-delete"), i18n("Delete permanently"), this);
    d->imageDeletePermanentlyAction->setShortcut(Qt::SHIFT+Qt::Key_Delete);
    connect(d->imageDeletePermanentlyAction, SIGNAL(triggered()), d->view, SLOT(slotImageDeletePermanently()));
    actionCollection()->addAction("image_delete_permanently", d->imageDeletePermanentlyAction);

    // -----------------------------------------------------------

    // These two actions are hidden, no menu entry, no toolbar entry, no shortcut.
    // Power users may add them.
    d->imageDeletePermanentlyDirectlyAction = new KAction(KIcon("edit-delete"),
                                              i18n("Delete permanently without confirmation"), this);
    connect(d->imageDeletePermanentlyDirectlyAction, SIGNAL(triggered()), 
            d->view, SLOT(slotImageDeletePermanentlyDirectly()));
    actionCollection()->addAction("image_delete_permanently_directly", d->imageDeletePermanentlyDirectlyAction);

    // -----------------------------------------------------------

    d->imageTrashDirectlyAction = new KAction(KIcon("edit-trash"),
                                  i18n("Move to trash without confirmation"), this);
    connect(d->imageTrashDirectlyAction, SIGNAL(triggered()), 
            d->view, SLOT(slotImageTrashDirectly()));
    actionCollection()->addAction("image_trash_directly", d->imageTrashDirectlyAction);

    // -----------------------------------------------------------

    d->imageSortAction = new KSelectAction(i18n("&Sort Images"), this);
    connect(d->imageSortAction, SIGNAL(triggered(int)), d->view, SLOT(slotSortImages(int)));
    actionCollection()->addAction("album_sort", d->imageSortAction);

    // Use same list order as in albumsettings enum
    QStringList sortImagesActionList;
    sortImagesActionList.append(i18n("By Name"));
    sortImagesActionList.append(i18n("By Path"));
    sortImagesActionList.append(i18n("By Date"));
    sortImagesActionList.append(i18n("By File Size"));
    sortImagesActionList.append(i18n("By Rating"));
    d->imageSortAction->setItems(sortImagesActionList);

    // -----------------------------------------------------------------

    QSignalMapper *exifOrientationMapper = new QSignalMapper(d->view);
    
    connect(exifOrientationMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotImageExifOrientation(int)));

    d->imageExifOrientationActionMenu = new KActionMenu(i18n("Adjust Exif orientation tag"), this);
    d->imageExifOrientationActionMenu->setDelayed(false);
    actionCollection()->addAction("image_set_exif_orientation", d->imageExifOrientationActionMenu);

    d->imageSetExifOrientation1Action =
        new KAction(i18n("Normal"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation2Action =
        new KAction(i18n("Flipped Horizontally"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation3Action =
        new KAction(i18n("Rotated upside down"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation4Action =
        new KAction(i18n("Flipped Vertically"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation5Action =
        new KAction(i18n("Rotated right / Horiz. Flipped"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation6Action =
        new KAction(i18n("Rotated right"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation7Action =
        new KAction(i18n("Rotated right / Vert. Flipped"), d->imageExifOrientationActionMenu);
    d->imageSetExifOrientation8Action =
        new KAction(i18n("Rotated left"), d->imageExifOrientationActionMenu);

    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation1Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation2Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation3Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation4Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation5Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation6Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation7Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation8Action);

    connect(d->imageSetExifOrientation1Action, SIGNAL(triggered()), exifOrientationMapper, SLOT(map()));
    connect(d->imageSetExifOrientation2Action, SIGNAL(triggered()), exifOrientationMapper, SLOT(map()));
    connect(d->imageSetExifOrientation3Action, SIGNAL(triggered()), exifOrientationMapper, SLOT(map()));
    connect(d->imageSetExifOrientation4Action, SIGNAL(triggered()), exifOrientationMapper, SLOT(map()));
    connect(d->imageSetExifOrientation5Action, SIGNAL(triggered()), exifOrientationMapper, SLOT(map()));
    connect(d->imageSetExifOrientation6Action, SIGNAL(triggered()), exifOrientationMapper, SLOT(map()));
    connect(d->imageSetExifOrientation7Action, SIGNAL(triggered()), exifOrientationMapper, SLOT(map()));
    connect(d->imageSetExifOrientation8Action, SIGNAL(triggered()), exifOrientationMapper, SLOT(map()));

    exifOrientationMapper->setMapping(d->imageSetExifOrientation1Action, 1);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation2Action, 2);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation3Action, 3);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation4Action, 4);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation5Action, 5);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation6Action, 6);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation7Action, 7);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation8Action, 8);

    // -----------------------------------------------------------------

    d->selectAllAction = new KAction(i18n("Select All"), this);
    d->selectAllAction->setShortcut(Qt::CTRL+Qt::Key_A);
    connect(d->selectAllAction, SIGNAL(triggered()), d->view, SLOT(slotSelectAll()));
    actionCollection()->addAction("selectAll", d->selectAllAction);

    // -----------------------------------------------------------------

    d->selectNoneAction = new KAction(i18n("Select None"), this);
    d->selectNoneAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_A);
    connect(d->selectNoneAction, SIGNAL(triggered()), d->view, SLOT(slotSelectNone()));
    actionCollection()->addAction("selectNone", d->selectNoneAction);

    // -----------------------------------------------------------------

    d->selectInvertAction = new KAction(i18n("Invert Selection"), this);
    d->selectInvertAction->setShortcut(Qt::CTRL+Qt::Key_Asterisk);
    connect(d->selectInvertAction, SIGNAL(triggered()), d->view, SLOT(slotSelectInvert()));
    actionCollection()->addAction("selectInvert", d->selectInvertAction);

    // -----------------------------------------------------------

    KStandardAction::keyBindings(this,       SLOT(slotEditKeys()),     actionCollection());
    KStandardAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStandardAction::preferences(this,       SLOT(slotSetup()),        actionCollection());

    // -----------------------------------------------------------

    d->zoomPlusAction = actionCollection()->addAction(KStandardAction::ZoomIn, "album_zoomin", 
                                                      d->view, SLOT(slotZoomIn()));

    // -----------------------------------------------------------
 
    d->zoomMinusAction = actionCollection()->addAction(KStandardAction::ZoomOut, "album_zoomout", 
                                                       d->view, SLOT(slotZoomOut()));

    // -----------------------------------------------------------

    d->zoomTo100percents = new KAction(KIcon("viewmag1"), i18n("Zoom to 1:1"), this);
    d->zoomTo100percents->setShortcut(Qt::ALT+Qt::CTRL+Qt::Key_0);       // NOTE: Photoshop 7 use ALT+CTRL+0
    connect(d->zoomTo100percents, SIGNAL(triggered()), d->view, SLOT(slotZoomTo100Percents()));
    actionCollection()->addAction("album_zoomto100percents", d->zoomTo100percents);

    // -----------------------------------------------------------

    d->zoomFitToWindowAction = new KAction(KIcon("zoom-best-fit"), i18n("Fit to &Window"), this);
    d->zoomFitToWindowAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_E); // NOTE: Gimp 2 use CTRL+SHIFT+E.
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), d->view, SLOT(slotFitToWindow()));
    actionCollection()->addAction("album_zoomfit2window", d->zoomFitToWindowAction);

    // -----------------------------------------------------------

    d->fullScreenAction = actionCollection()->addAction(KStandardAction::FullScreen,
                          "full_screen", this, SLOT(slotToggleFullScreen()));

    // -----------------------------------------------------------

    d->slideShowAction = new KActionMenu(KIcon("datashow"), i18n("Slide Show"), this);
    d->slideShowAction->setDelayed(false);
    actionCollection()->addAction("slideshow", d->slideShowAction);

    d->slideShowAllAction = new KAction(i18n("All"), this);
    d->slideShowAllAction->setShortcut(Qt::Key_F9); 
    connect(d->slideShowAllAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowAll()));
    actionCollection()->addAction("slideshow_all", d->slideShowAllAction);
    d->slideShowAction->addAction(d->slideShowAllAction);

    d->slideShowSelectionAction = new KAction(i18n("Selection"), this);
    d->slideShowSelectionAction->setShortcut(Qt::ALT+Qt::Key_F9); 
    connect(d->slideShowSelectionAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowSelection()));
    actionCollection()->addAction("slideshow_selected", d->slideShowSelectionAction);
    d->slideShowAction->addAction(d->slideShowSelectionAction);

    d->slideShowRecursiveAction = new KAction(i18n("With all sub-albums"), this);
    d->slideShowRecursiveAction->setShortcut(Qt::SHIFT+Qt::Key_F9); 
    connect(d->slideShowRecursiveAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowRecursive()));
    actionCollection()->addAction("slideshow_recursive", d->slideShowRecursiveAction);
    d->slideShowAction->addAction(d->slideShowRecursiveAction);

    // -----------------------------------------------------------

    d->quitAction = actionCollection()->addAction(KStandardAction::Quit, "app_exit", 
                                                  this, SLOT(slotExit()));

    // -----------------------------------------------------------

    d->kipiHelpAction = new KAction(KIcon("kipi"), i18n("Kipi Plugins Handbook"), this);
    connect(d->kipiHelpAction, SIGNAL(triggered()), this, SLOT(slotShowKipiHelp()));
    actionCollection()->addAction("help_kipi", d->kipiHelpAction);

    // -----------------------------------------------------------

    d->tipAction = actionCollection()->addAction(KStandardAction::TipofDay, "help_tipofday", 
                                                 this, SLOT(slotShowTip()));

    // -----------------------------------------------------------

    d->donateMoneyAction = new KAction(i18n("Make a donation..."), this);
    connect(d->donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    actionCollection()->addAction("help_donatemoney", d->donateMoneyAction);

    // -- Rating actions ---------------------------------------------------------------

    d->rating0Star = new KAction(i18n("Assign Rating \"No Star\""), this);
    d->rating0Star->setShortcut(Qt::CTRL+Qt::Key_0);
    connect(d->rating0Star, SIGNAL(triggered()), d->view, SLOT(slotAssignRatingNoStar()));
    actionCollection()->addAction("ratenostar", d->rating0Star);

    d->rating1Star = new KAction(i18n("Assign Rating \"One Star\""), this);
    d->rating1Star->setShortcut(Qt::CTRL+Qt::Key_1);
    connect(d->rating1Star, SIGNAL(triggered()), d->view, SLOT(slotAssignRatingOneStar()));
    actionCollection()->addAction("rateonestar", d->rating1Star);

    d->rating2Star = new KAction(i18n("Assign Rating \"Two Stars\""), this);
    d->rating2Star->setShortcut(Qt::CTRL+Qt::Key_2);
    connect(d->rating2Star, SIGNAL(triggered()), d->view, SLOT(slotAssignRatingTwoStar()));
    actionCollection()->addAction("ratetwostar", d->rating2Star);

    d->rating3Star = new KAction(i18n("Assign Rating \"Three Stars\""), this);
    d->rating3Star->setShortcut(Qt::CTRL+Qt::Key_3);
    connect(d->rating3Star, SIGNAL(triggered()), d->view, SLOT(slotAssignRatingThreeStar()));
    actionCollection()->addAction("ratethreestar", d->rating3Star);

    d->rating4Star = new KAction(i18n("Assign Rating \"Four Stars\""), this);
    d->rating4Star->setShortcut(Qt::CTRL+Qt::Key_4);
    connect(d->rating4Star, SIGNAL(triggered()), d->view, SLOT(slotAssignRatingFourStar()));
    actionCollection()->addAction("ratefourstar", d->rating4Star);

    d->rating5Star = new KAction(i18n("Assign Rating \"Five Stars\""), this);
    d->rating5Star->setShortcut(Qt::CTRL+Qt::Key_5);
    connect(d->rating5Star, SIGNAL(triggered()), d->view, SLOT(slotAssignRatingFiveStar()));
    actionCollection()->addAction("ratefivestar", d->rating5Star);

    // -----------------------------------------------------------

    QAction* findAction = actionCollection()->addAction(KStandardAction::Find, "search_quick", 
                                                        d->view, SLOT(slotNewQuickSearch()));
    findAction->setText(i18n("Quick Search..."));
    findAction->setIcon(BarIcon("file-find"));

    // -----------------------------------------------------------

    QAction* advFindAction = actionCollection()->addAction(KStandardAction::Find, "search_advanced", 
                                                           d->view, SLOT(slotNewAdvancedSearch()));
    advFindAction->setText(i18n("Advanced Search..."));
    advFindAction->setShortcut(Qt::CTRL+Qt::ALT+Qt::Key_F);

    // -----------------------------------------------------------

    KAction *ltAction = new KAction(KIcon("lighttable"), i18n("Light Table"), this);
    ltAction->setShortcut(Qt::CTRL+Qt::Key_F6); 
    connect(ltAction, SIGNAL(triggered()), d->view, SLOT(slotLightTable()));
    actionCollection()->addAction("light_table", ltAction);

    // -----------------------------------------------------------

    KAction *scanNewAction = new KAction(KIcon("rebuild"), i18n("Scan for New Images"), this);
    connect(scanNewAction, SIGNAL(triggered()), this, SLOT(slotDatabaseRescan()));
    actionCollection()->addAction("database_rescan", scanNewAction);

    // -----------------------------------------------------------

    KAction *rebuildThumbsAction = new KAction(KIcon("recycled"), i18n("Rebuild all Thumbnails..."), this);
    connect(rebuildThumbsAction, SIGNAL(triggered()), this, SLOT(slotRebuildAllThumbs()));
    actionCollection()->addAction("thumbs_rebuild", rebuildThumbsAction);

    // -----------------------------------------------------------

    KAction *syncMetadataAction = new KAction(KIcon("compfile"), 
                                              i18n("Update Metadata Database..."), this);
    connect(syncMetadataAction, SIGNAL(triggered()), this, SLOT(slotSyncAllPicturesMetadata()));
    actionCollection()->addAction("sync_metadata", syncMetadataAction);

    // -----------------------------------------------------------

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // Load Cameras -- do this before the createGUI so that the cameras
    // are plugged into the toolbar at startup
    if (d->splashScreen)
        d->splashScreen->message(i18n("Loading cameras"), Qt::AlignLeft, Qt::white);
    
    loadCameras();

    createGUI("digikamui.rc");

    // Initialize Actions ---------------------------------------

    d->deleteAction->setEnabled(false);
    d->addImagesAction->setEnabled(false);
    d->propsEditAction->setEnabled(false);
    d->openInKonquiAction->setEnabled(false);

    d->imageViewAction->setEnabled(false);
    d->imagePreviewAction->setEnabled(false);
    d->imageLightTableAction->setEnabled(false);
    d->imageRenameAction->setEnabled(false);
    d->imageDeleteAction->setEnabled(false);
    d->imageExifOrientationActionMenu->setEnabled(false);
    d->slideShowSelectionAction->setEnabled(false);

    d->albumSortAction->setCurrentItem((int)d->albumSettings->getAlbumSortOrder());
    d->imageSortAction->setCurrentItem((int)d->albumSettings->getImageSortOrder());
}

void DigikamApp::enableZoomPlusAction(bool val)
{
    d->zoomPlusAction->setEnabled(val);
    d->statusZoomBar->setEnableZoomPlus(val);
}

void DigikamApp::enableZoomMinusAction(bool val)
{
    d->zoomMinusAction->setEnabled(val);
    d->statusZoomBar->setEnableZoomMinus(val);
}

void DigikamApp::enableAlbumBackwardHistory(bool enable)
{
    d->backwardActionMenu->setEnabled(enable);
}

void DigikamApp::enableAlbumForwardHistory(bool enable)
{
    d->forwardActionMenu->setEnabled(enable);
}

void DigikamApp::slotAboutToShowBackwardMenu()
{
    d->backwardActionMenu->menu()->clear();
    QStringList titles;
    d->view->getBackwardHistory(titles);
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();
        for(; iter != titles.end(); ++iter,++id)
        {
            d->backwardActionMenu->menu()->insertItem(*iter, id);
        }
    }
}

void DigikamApp::slotAboutToShowForwardMenu()
{
    d->forwardActionMenu->menu()->clear();
    QStringList titles;
    d->view->getForwardHistory(titles);
    
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();
        for(; iter != titles.end(); ++iter,++id)
        {
            d->forwardActionMenu->menu()->insertItem(*iter, id);
        }
    }
}

void DigikamApp::slotAlbumSelected(bool val)
{
    Album *album = d->albumManager->currentAlbum();
    
    if(album && !val)
    {
        // No PAlbum is selected
        d->deleteAction->setEnabled(false);
        d->addImagesAction->setEnabled(false);
        d->propsEditAction->setEnabled(false);
        d->openInKonquiAction->setEnabled(false);
        d->newAction->setEnabled(false);
        d->albumImportAction->setEnabled(false);

    }
    else if(!album && !val)
    {
        // Groupitem selected (Collection/date)
        d->deleteAction->setEnabled(false);
        d->addImagesAction->setEnabled(false);
        d->propsEditAction->setEnabled(false);
        d->openInKonquiAction->setEnabled(false);
        d->newAction->setEnabled(false);
        d->albumImportAction->setEnabled(false);
        
        foreach(QAction *action, d->kipiFileActionsImport)
        {
            action->setEnabled(false);
        }

        foreach(QAction *action, d->kipiFileActionsExport)
        {
            action->setEnabled(false);
        }
    }
    else if(album && !album->isRoot() && album->type() == Album::PHYSICAL)
    {
        // Normal Album selected
        d->deleteAction->setEnabled(true);
        d->addImagesAction->setEnabled(true);
        d->propsEditAction->setEnabled(true);
        d->openInKonquiAction->setEnabled(true);
        d->newAction->setEnabled(true);
        d->albumImportAction->setEnabled(true);
        
        foreach(QAction *action, d->kipiFileActionsImport)
        {
            action->setEnabled(true);
        }

        foreach(QAction *action, d->kipiFileActionsExport)
        {
            action->setEnabled(true);    
        }        
    }
    else if(album && album->isRoot() && album->type() == Album::PHYSICAL)
    {
        // Root Album selected
        d->deleteAction->setEnabled(false);
        d->addImagesAction->setEnabled(false);
        d->propsEditAction->setEnabled(false);
       

        if(album->type() == Album::PHYSICAL)
        {
            d->newAction->setEnabled(true);
            d->openInKonquiAction->setEnabled(true);
            d->albumImportAction->setEnabled(true);
        }
        else
        {
            d->newAction->setEnabled(false);
            d->openInKonquiAction->setEnabled(false);
            d->albumImportAction->setEnabled(false);            
        }
        
        foreach(QAction *action, d->kipiFileActionsImport)
        {
            action->setEnabled(false);
        }

        foreach(QAction *action, d->kipiFileActionsExport)
        {
            action->setEnabled(true);
        }
    }
}

void DigikamApp::slotTagSelected(bool val)
{
    Album *album = d->albumManager->currentAlbum();
    
    if(!val)
    {
        d->deleteTagAction->setEnabled(false);
        d->editTagAction->setEnabled(false);
    }
    else if(!album->isRoot())
    {
        d->deleteTagAction->setEnabled(true);
        d->editTagAction->setEnabled(true);
        
        foreach(QAction *action, d->kipiFileActionsImport)
        {
            action->setEnabled(false);
        }

        foreach(QAction *action, d->kipiFileActionsExport)
        {
            action->setEnabled(true);
        }
    }
    else
    {
        d->deleteTagAction->setEnabled(false);
        d->editTagAction->setEnabled(false);
        
        foreach(QAction *action, d->kipiFileActionsImport)
        {
            action->setEnabled(false);
        }

        foreach(QAction *action, d->kipiFileActionsExport)
        {
            action->setEnabled(true);
        }
    }
}

void DigikamApp::slotImageSelected(const Q3PtrList<ImageInfo>& list, bool hasPrev, bool hasNext)
{
    Q3PtrList<ImageInfo> selection = list;
    bool val = selection.isEmpty() ? false : true;
    d->imageViewAction->setEnabled(val);
    d->imagePreviewAction->setEnabled(val);
    d->imageLightTableAction->setEnabled(val);
    d->imageRenameAction->setEnabled(val);
    d->imageDeleteAction->setEnabled(val);
    d->imageExifOrientationActionMenu->setEnabled(val);
    d->slideShowSelectionAction->setEnabled(selection.count() != 0);

    switch (selection.count())
    {
        case 0:
            d->statusProgressBar->setText(i18n("No item selected"));
        break;
        case 1:
            d->statusProgressBar->setText(selection.first()->fileUrl().fileName());
        break;
        default:
            d->statusProgressBar->setText(i18n("%1 items selected").arg(selection.count()));
        break;
    }

    d->statusNavigateBar->setNavigateBarState(hasPrev, hasNext);
}

void DigikamApp::slotProgressBarMode(int mode, const QString& text)
{
    d->statusProgressBar->progressBarMode(mode, text);
}

void DigikamApp::slotProgressValue(int count)
{
    d->statusProgressBar->setProgressValue(count);
}

void DigikamApp::slotExit()
{
    close();
}

QString DigikamApp::convertToLocalUrl( const QString& folder )
{
    // This function is copied from k3b.

    KUrl url( folder );
    if( !url.isLocalFile() )
    {
        // Support for system:/ and media:/ (c) Stephan Kulow
        KUrl mlu = KIO::NetAccess::mostLocalUrl( url, 0 );
        if (mlu.isLocalFile())
            return mlu.path();

        DWarning() << folder << " mlu " << mlu << endl;

        QString path = mlu.path();

        if ( mlu.protocol() == "system" && path.startsWith("/media") )
            path = path.mid(7);
        else if (mlu.protocol() == "media")
            path = path.mid(1);
        else
            return folder; // nothing to see - go on
#if 0
        DDebug() << "parsed import path is: " << path << endl;
        DCOPRef ref("kded", "mediamanager");
        DCOPReply reply = ref.call("properties", path);
        if (reply.isValid()) {
            QStringList slreply;
            reply.get(slreply);
            if ((slreply.count()>=9) && !slreply[9].isEmpty())
                return slreply[9];
            else
                return slreply[6];
        } else {
            DWarning() << "dcop call failed\n";
        }
#endif
        return path;
    }

    return url.path();
}

void DigikamApp::downloadImages( const QString& folder )
{
    if (!folder.isNull())
    {
        // activate window when called by media menu and DCOP
        if (isMinimized())
            KWindowSystem::unminimizeWindow(winId());
        KWindowSystem::activateWindow(winId());

        slotDownloadImages(folder);
    }
}

void DigikamApp::cameraAutoDetect()
{
    // activate window when called by media menu and DCOP
    if (isMinimized())
        KWindowSystem::unminimizeWindow(winId());
    KWindowSystem::activateWindow(winId());

    slotCameraAutoDetect();
}

void DigikamApp::slotDownloadImages( const QString& folder)
{
    if (!folder.isNull())
    {
        d->cameraGuiPath = folder;

        QTimer::singleShot(0, this, SLOT(slotDownloadImages()));
    }
}

void DigikamApp::slotDownloadImages()
{
    if (d->cameraGuiPath.isNull())
        return;

    // Fetch the contents of the device. This is needed to make sure that the
    // media:/device gets mounted.
    KIO::ListJob *job = KIO::listDir(KUrl(d->cameraGuiPath), false, false);
    KIO::NetAccess::synchronousRun(job,0);

    QString localUrl = convertToLocalUrl(d->cameraGuiPath);
    DDebug() << "slotDownloadImages: convertToLocalUrl " << d->cameraGuiPath << " to " << localUrl << endl;

    if (localUrl.isNull())
        return;

    bool alreadyThere = false;

    for (int i = 0 ; i != actionCollection()->count() ; i++)
    {
        if (actionCollection()->action(i)->objectName() == d->cameraGuiPath)
            alreadyThere = true;
    }

    if (!alreadyThere)
    {
        KAction *cAction = new KAction(KIcon("camera-photo"), 
                           i18n("Browse %1").arg(KUrl(d->cameraGuiPath).prettyUrl()), this);
        connect(cAction, SIGNAL(triggered()), this, SLOT(slotDownloadImages()));
        actionCollection()->addAction(d->cameraGuiPath.toLatin1(), cAction);

        d->cameraMenuAction->insertAction(cAction, 0);
    }

    // the CameraUI will delete itself when it has finished
    CameraUI* cgui = new CameraUI(this,
                                  i18n("Images found in %1").arg(d->cameraGuiPath),
                                  "directory browse", "Fixed", localUrl, 
                                  QDateTime::currentDateTime());
    cgui->show();

    connect(cgui, SIGNAL(signalLastDestination(const KUrl&)),
            d->view, SLOT(slotSelectAlbum(const KUrl&)));

    connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
            this, SLOT(slotSetupChanged()));
}

void DigikamApp::slotCameraConnect()
{
    CameraType* ctype = d->cameraList->find(sender()->objectName());

    if (ctype)
    {
        // check not to open two dialogs for the same camera
        if (ctype->currentCameraUI() && !ctype->currentCameraUI()->isClosed())
        {
            // show and raise dialog
            if (ctype->currentCameraUI()->isMinimized())
                KWindowSystem::unminimizeWindow(ctype->currentCameraUI()->winId());
            KWindowSystem::activateWindow(ctype->currentCameraUI()->winId());
        }
        else
        {
            // the CameraUI will delete itself when it has finished
            CameraUI* cgui = new CameraUI(this, ctype->title(), ctype->model(),
                                          ctype->port(), ctype->path(), ctype->lastAccess());

            ctype->setCurrentCameraUI(cgui);

            cgui->show();

            connect(cgui, SIGNAL(signalLastDestination(const KUrl&)),
                    d->view, SLOT(slotSelectAlbum(const KUrl&)));

            connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
                    this, SLOT(slotSetupChanged()));
        }
    }
}

void DigikamApp::slotCameraAdded(CameraType *ctype)
{
    if (!ctype) return;

    KAction *cAction = new KAction(KIcon("camera-photo"), ctype->title(), this);
    connect(cAction, SIGNAL(triggered()), this, SLOT(slotCameraConnect()));
    cAction->setObjectName(ctype->title());
    actionCollection()->addAction(ctype->title().toUtf8(), cAction);

    d->cameraMenuAction->insertAction(0, cAction);
    ctype->setAction(cAction);
}

void DigikamApp::slotCameraMediaMenu()
{
    d->mediaItems.clear();
    
    d->cameraMediaList->clear();
    d->cameraMediaList->insertItem(i18n("No media devices found"), 0);
    d->cameraMediaList->setItemEnabled(0, false);
        
    KUrl kurl("media:/");
    KIO::ListJob *job = KIO::listDir(kurl, false, false);
    
    connect( job, SIGNAL(entries(KIO::Job*,const KIO::UDSEntryList&)),
             this, SLOT(slotCameraMediaMenuEntries(KIO::Job*,const KIO::UDSEntryList&)) );
}

void DigikamApp::slotCameraMediaMenuEntries( Job *, const UDSEntryList & list )
{
    int i = 0;

    for(KIO::UDSEntryList::ConstIterator it = list.begin() ; it != list.end() ; ++it)
    {
        // TODO: KDE4PORT: check if this port is right.
        QString name = (*it).stringValue(KIO::UDS_NAME);
        QString path = (*it).stringValue(KIO::UDS_URL);

        /*for ( UDSEntry::const_iterator et = (*it).begin() ; et != (*it).end() ; ++et ) 
        {
            if ( (*et).m_uds == KIO::UDS_NAME)
                name = ( *et ).m_str;
            if ( (*et).m_uds == KIO::UDS_URL)
                path = ( *et ).m_str;

        }*/

        //DDebug() << name << " : " << path << endl;

        if (!name.isEmpty() && !path.isEmpty())
        {
            //DDebug() << "slotCameraMediaMenuEntries: Adding " << name << ", path " << path << endl;
            if (i == 0)
                d->cameraMediaList->clear();

            d->mediaItems[i] = path;

            d->cameraMediaList->insertItem(name, this, SLOT(slotDownloadImagesFromMedia(int)), 0, i);
            d->cameraMediaList->setItemParameter(i, i);
            i++;
        }
    }
}

void DigikamApp::slotDownloadImagesFromMedia( int id )
{
    slotDownloadImages( d->mediaItems[id] );
}

void DigikamApp::slotCameraRemoved(CameraType *ctype)
{
    if (!ctype) return;

    KAction *cAction = ctype->action();

    if (cAction)
        d->cameraMenuAction->removeAction(cAction);
}

void DigikamApp::slotCameraAutoDetect()
{
    bool retry = false;

    CameraType* ctype = d->cameraList->autoDetect(retry);
    
    if (!ctype && retry)
    {
        QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
        return;
    }

    if (ctype && ctype->action())
    {
        ctype->action()->activate(QAction::Trigger);
    }
}

void DigikamApp::slotSetup()
{
    setup();
}

bool DigikamApp::setup(bool iccSetupPage)
{
    Setup setup(this, 0, iccSetupPage ? Setup::ICCPage : Setup::LastPageUsed);

    // To show the number of KIPI plugins in the setup dialog.

    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();
    setup.kipiPluginsPage()->initPlugins((int)list.count());

    if (setup.exec() != QDialog::Accepted)
        return false;

    setup.kipiPluginsPage()->applyPlugins();

    slotSetupChanged();

    return true;
}

void DigikamApp::slotSetupCamera()
{
    Setup setup(this, 0, Setup::CameraPage);

    // For to show the number of KIPI plugins in the setup dialog.

    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();
    setup.kipiPluginsPage()->initPlugins((int)list.count());

    if (setup.exec() != QDialog::Accepted)
        return;

    setup.kipiPluginsPage()->applyPlugins();

    slotSetupChanged();
}

void DigikamApp::slotSetupChanged()
{
    // raw loading options might have changed
    LoadingCacheInterface::cleanCache();

    // TODO: clear history when location changed
    //if(d->albumSettings->getAlbumLibraryPath() != d->albumManager->getLibraryPath())
      //  d->view->clearHistory();

    d->albumManager->setAlbumRoot(d->albumSettings->getAlbumLibraryPath(), false);// TEMPORARY SOLUTION
    d->albumManager->startScan();

    d->view->applySettings(d->albumSettings);

    AlbumThumbnailLoader::componentData()->setThumbnailSize(d->albumSettings->getDefaultTreeIconSize());

    if (ImageWindow::imagewindowCreated())
        ImageWindow::imagewindow()->applySettings();

    d->config->sync();
}

void DigikamApp::slotEditKeys()
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection( actionCollection(), i18n( "General" ) );


    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();

    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin() ; it != list.end() ; ++it )
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if ( plugin )
           dialog.addCollection(plugin->actionCollection(), (*it)->comment());
    }

    dialog.configure();
}

void DigikamApp::slotConfToolbars()
{
    saveMainWindowSettings(d->config->group("General Settings"));
    KEditToolBar *dlg = new KEditToolBar(actionCollection(), this);
    dlg->setResourceFile("digikamui.rc");

    if(dlg->exec())
    {
        createGUI("digikamui.rc");
        applyMainWindowSettings(d->config->group("General Settings"));
        plugActionList( QString::fromLatin1("file_actions_import"), d->kipiFileActionsImport );
        plugActionList( QString::fromLatin1("image_actions"), d->kipiImageActions );
        plugActionList( QString::fromLatin1("tool_actions"), d->kipiToolsActions );
        plugActionList( QString::fromLatin1("batch_actions"), d->kipiBatchActions );
        plugActionList( QString::fromLatin1("album_actions"), d->kipiAlbumActions );
        plugActionList( QString::fromLatin1("file_actions_export"), d->kipiFileActionsExport );
    }
    
    delete dlg;
}

void DigikamApp::slotToggleFullScreen()
{
    if (d->fullScreen)
    {
#if QT_VERSION >= 0x030300
        setWindowState( windowState() & ~Qt::WindowFullScreen );
#else
        showNormal();
#endif
        menuBar()->show();
        statusBar()->show();

#warning "TODO: kde4 port it";
/* TODO: KDE4PORT: Check these methods
        topDock()->show();
        bottomDock()->show();
        leftDock()->show();
        rightDock()->show();
*/
        d->view->showSideBars();
        d->fullScreen = false;
    }
    else
    {
        KConfigGroup group         = d->config->group("ImageViewer Settings");
        bool fullScreenHideToolBar = group.readEntry("FullScreen Hide ToolBar", false);

        menuBar()->hide();
        statusBar()->hide();


/* TODO: KDE4PORT: Check these methods

        if (fullScreenHideToolBar)
            topDock()->hide();
        bottomDock()->hide();
        leftDock()->hide();
        rightDock()->hide();
*/
        d->view->hideSideBars();
        showFullScreen();
        d->fullScreen = true;
    }
}

void DigikamApp::slotShowTip()
{
    QStringList tipsFiles;
    tipsFiles.append("digikam/tips");
    tipsFiles.append("kipi/tips");
    KTipDialog::showMultiTip(this, tipsFiles, true);
}

void DigikamApp::slotShowKipiHelp()
{
    KToolInvocation::invokeHelp( QString(), "kipi-plugins" );
}

void DigikamApp::loadPlugins()
{
    if(d->splashScreen)
        d->splashScreen->message(i18n("Loading Kipi Plugins"), Qt::AlignLeft, Qt::white);

    QStringList ignores;
    d->kipiInterface = new DigikamKipiInterface( this, "Digikam_KIPI_interface" );

    ignores.append( "HelloWorld" );
    ignores.append( "KameraKlient" );

    d->kipiPluginLoader = new KIPI::PluginLoader( ignores, d->kipiInterface );

    connect( d->kipiPluginLoader, SIGNAL( replug() ),
             this, SLOT( slotKipiPluginPlug() ) );

    d->kipiPluginLoader->loadPlugins();

    d->kipiInterface->slotCurrentAlbumChanged(d->albumManager->currentAlbum());

    // Setting the initial menu options after all plugins have been loaded
    d->view->slotAlbumSelected(d->albumManager->currentAlbum());

    d->imagePluginsLoader = new ImagePluginLoader(this, d->splashScreen);
}

void DigikamApp::slotKipiPluginPlug()
{
    unplugActionList( QString::fromLatin1("file_actions_export") );
    unplugActionList( QString::fromLatin1("file_actions_import") );
    unplugActionList( QString::fromLatin1("image_actions") );
    unplugActionList( QString::fromLatin1("tool_actions") );
    unplugActionList( QString::fromLatin1("batch_actions") );
    unplugActionList( QString::fromLatin1("album_actions") );

    d->kipiImageActions.clear();
    d->kipiFileActionsExport.clear();
    d->kipiFileActionsImport.clear();
    d->kipiToolsActions.clear();
    d->kipiBatchActions.clear();
    d->kipiAlbumActions.clear();

    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();

    int cpt = 0;

    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin() ; it != list.end() ; ++it )
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if ( !plugin || !(*it)->shouldLoad() )
            continue;

        ++cpt;

        //if(d->splashScreen)
          //  d->splashScreen->message(i18n("Loading: %1").arg((*it)->name()));

        plugin->setup( this );

        // Plugin category identification using KAction method based.

        QList<KAction*> actions = plugin->actions();

        for( QList<KAction*>::Iterator it2 = actions.begin(); it2 != actions.end(); ++it2 )
        {
            if ( plugin->category(*it2) == KIPI::IMAGESPLUGIN )
            {
                d->kipiImageActions.append(*it2);
            }
            else if ( plugin->category(*it2) == KIPI::EXPORTPLUGIN )
            {
                d->kipiFileActionsExport.append(*it2);
            }
            else if ( plugin->category(*it2) == KIPI::IMPORTPLUGIN )
            {
                d->kipiFileActionsImport.append(*it2);
            }
            else if ( plugin->category(*it2) == KIPI::TOOLSPLUGIN )
            {
                d->kipiToolsActions.append(*it2);
            }
            else if ( plugin->category(*it2) == KIPI::BATCHPLUGIN )
            {
                d->kipiBatchActions.append(*it2);
            }
            else if ( plugin->category(*it2) == KIPI::COLLECTIONSPLUGIN )
            {
                d->kipiAlbumActions.append(*it2);
            }
            else
                DDebug() << "No menu found for a plugin!!!" << endl;
        }

#warning "TODO: kde4 port it";
/* TODO: KDE4PORT: how we can do it with KDE4 ?
        plugin->actionCollection()->readShortcutSettings();
*/
    }

    // Create GUI menu in according with plugins.

    plugActionList( QString::fromLatin1("file_actions_export"), d->kipiFileActionsExport );
    plugActionList( QString::fromLatin1("file_actions_import"), d->kipiFileActionsImport );
    plugActionList( QString::fromLatin1("image_actions"),       d->kipiImageActions );
    plugActionList( QString::fromLatin1("tool_actions"),        d->kipiToolsActions );
    plugActionList( QString::fromLatin1("batch_actions"),       d->kipiBatchActions );
    plugActionList( QString::fromLatin1("album_actions"),       d->kipiAlbumActions );
}

void DigikamApp::loadCameras()
{
    d->cameraList->load();
   
    d->cameraMenuAction->menu()->insertSeparator();
    d->cameraMenuAction->menu()->insertItem(i18n("Browse Media"), d->cameraMediaList);
    d->cameraMenuAction->menu()->insertSeparator();

    KAction *cameraAction = new KAction(i18n("Add Camera..."), this);
    connect(cameraAction, SIGNAL(triggered()), this, SLOT(slotSetupCamera()));
    actionCollection()->addAction("camera_add", cameraAction);
    d->cameraMenuAction->addAction(cameraAction);
}

void DigikamApp::populateThemes()
{
    if(d->splashScreen)
        d->splashScreen->message(i18n("Loading themes"), Qt::AlignLeft, Qt::white);

    ThemeEngine::componentData()->scanThemes();
    QStringList themes(ThemeEngine::componentData()->themeNames());

    d->themeMenuAction->setItems(themes);
    int index = themes.indexOf(d->albumSettings->getCurrentTheme());
    
    if (index == -1)
        index = themes.indexOf(i18n("Default"));
        
    d->themeMenuAction->setCurrentItem(index);
    ThemeEngine::componentData()->slotChangeTheme(d->themeMenuAction->currentText());
}

void DigikamApp::slotChangeTheme(const QString& theme)
{
    d->albumSettings->setCurrentTheme(theme);
    ThemeEngine::componentData()->slotChangeTheme(theme);
}

void DigikamApp::slotDatabaseRescan()
{
    ScanLib sLib;
    sLib.startScan();
}

void DigikamApp::slotRebuildAllThumbs()
{
    QString msg = i18n("Rebuilding all image thumbnails can take some time.\n"
                       "Do you want to continue?");
    int result = KMessageBox::warningContinueCancel(this, msg);
    if (result != KMessageBox::Continue)
        return;

    BatchThumbsGenerator *thumbsGenerator = new BatchThumbsGenerator(this);
    
    connect(thumbsGenerator, SIGNAL(signalRebuildAllThumbsDone()),
            this, SLOT(slotRebuildAllThumbsDone()));

    thumbsGenerator->exec();
}

void DigikamApp::slotRebuildAllThumbsDone()
{
    d->view->applySettings(d->albumSettings);
}

void DigikamApp::slotSyncAllPicturesMetadata()
{
    QString msg = i18n("Updating the metadata database can take some time. \nDo you want to continue?");
    int result = KMessageBox::warningContinueCancel(this, msg);
    if (result != KMessageBox::Continue)
        return;

    BatchAlbumsSyncMetadata *syncMetadata = new BatchAlbumsSyncMetadata(this);
    
    connect(syncMetadata, SIGNAL(signalComplete()),
            this, SLOT(slotSyncAllPicturesMetadataDone()));

    syncMetadata->exec();
}

void DigikamApp::slotSyncAllPicturesMetadataDone()
{
    d->view->applySettings(d->albumSettings);
}

void DigikamApp::slotDonateMoney()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=donation");
}

void DigikamApp::slotZoomSliderChanged(int size)
{
    d->view->setThumbSize(size);
}

void DigikamApp::slotThumbSizeChanged(int size)
{
    d->statusZoomBar->setZoomSliderValue(size);
    d->statusZoomBar->setZoomTrackerText(i18n("Size: %1").arg(size));
}

void DigikamApp::slotZoomChanged(double zoom, int size)
{
    d->statusZoomBar->setZoomSliderValue(size);
    d->statusZoomBar->setZoomTrackerText(i18n("zoom: %1%").arg((int)(zoom*100.0)));
}

void DigikamApp::slotTooglePreview(bool t)
{
    // NOTE: if 't' is true, we are in Preview Mode, else we are in AlbumView Mode

    // This is require if ESC is pressed to go out of Preview Mode. 
    // imagePreviewAction is handled by F3 key only. 
    d->imagePreviewAction->setChecked(t);

    // Here, we will toggle some menu actions depending of current Mode.
    
    // Select menu.
    d->selectAllAction->setEnabled(!t);
    d->selectNoneAction->setEnabled(!t);
    d->selectInvertAction->setEnabled(!t);

    // View menu     
    d->albumSortAction->setEnabled(!t);
    d->imageSortAction->setEnabled(!t);
    d->zoomTo100percents->setEnabled(t);
    d->zoomFitToWindowAction->setEnabled(t);
}

}  // namespace Digikam
