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
 * Copyright (C) 2002-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qdatastream.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qsignalmapper.h>
#include <qdockarea.h>
#include <qhbox.h>

// KDE includes.

#include <kaboutdata.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kstdaction.h>
#include <kstdaccel.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kiconloader.h>
#include <ktip.h>
#include <kdeversion.h>
#include <kapplication.h>
#include <kmenubar.h>
#include <kglobalsettings.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kwin.h>
#include <kimageio.h>
#include <dcopref.h>

// libKipi includes.

#include <libkipi/plugin.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000106
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes.

#include "ddebug.h"
#include "dlogoaction.h"
#include "album.h"
#include "albumlister.h"
#include "albumthumbnailloader.h"
#include "albumiconviewfilter.h"
#include "cameratype.h"
#include "cameraui.h"
#include "setup.h"
#include "setupplugins.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "rawcameradlg.h"
#include "lighttablewindow.h"
#include "imagewindow.h"
#include "imageinfo.h"
#include "thumbnailsize.h"
#include "themeengine.h"
#include "scanlib.h"
#include "loadingcacheinterface.h"
#include "imageattributeswatch.h"
#include "batchthumbsgenerator.h"
#include "batchalbumssyncmetadata.h"
#include "dcopiface.h"
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
          : KMainWindow( 0, "Digikam" )
{
    d = new DigikamAppPriv;
    m_instance = this;
    d->config  = kapp->config();
    d->config->setGroup("General Settings");

    if(d->config->readBoolEntry("Show Splash", true) &&
       !kapp->isRestored())
    {
        d->splashScreen = new SplashScreen("digikam-splash.png");
        d->splashScreen->show();
    }

    if(d->splashScreen)
        d->splashScreen->message(i18n("Initializing..."));

    // Register image formats (especially for TIFF )
    KImageIO::registerFormats();

    d->albumSettings = new AlbumSettings();
    d->albumSettings->readSettings();

    d->albumManager = new Digikam::AlbumManager();

    AlbumLister::instance();

    d->cameraMediaList = new KPopupMenu;

    connect(d->cameraMediaList, SIGNAL( aboutToShow() ),
            this, SLOT(slotCameraMediaMenu()));

    d->cameraList = new CameraList(this, locateLocal("appdata", "cameras.xml"));

    connect(d->cameraList, SIGNAL(signalCameraAdded(CameraType *)),
            this, SLOT(slotCameraAdded(CameraType *)));

    connect(d->cameraList, SIGNAL(signalCameraRemoved(CameraType *)),
            this, SLOT(slotCameraRemoved(CameraType *)));

    setupView();
    setupStatusBar();
    setupAccelerators();
    setupActions();

    applyMainWindowSettings(d->config);

    // Check ICC profiles repository availability

    if(d->splashScreen)
        d->splashScreen->message(i18n("Checking ICC repository"));

    d->validIccPath = SetupICC::iccRepositoryIsValid();

#if KDCRAW_VERSION < 0x000106
    // Check witch dcraw version available

    if(d->splashScreen)
        d->splashScreen->message(i18n("Checking dcraw version"));

    KDcrawIface::DcrawBinary::instance()->checkSystem();
#endif

    if(d->splashScreen)
        d->splashScreen->message(i18n("Scan Albums"));

    d->albumManager->setLibraryPath(d->albumSettings->getAlbumLibraryPath(), d->splashScreen);

    // Read albums from database
    if(d->splashScreen)
        d->splashScreen->message(i18n("Reading database"));

    d->albumManager->startScan();

    // Load KIPI Plugins.
    loadPlugins();

    // Load Themes
    populateThemes();

    setAutoSaveSettings();

    d->dcopIface = new DCOPIface(this, "camera");

    connect(d->dcopIface, SIGNAL(signalCameraAutoDetect()),
            this, SLOT(slotDcopCameraAutoDetect()));

    connect(d->dcopIface, SIGNAL(signalDownloadImages( const QString & )),
            this, SLOT(slotDcopDownloadImages(const QString &)));
}

DigikamApp::~DigikamApp()
{
    ImageAttributesWatch::shutDown();

    // Close and delete image editor instance.

    if (ImageWindow::imagewindowCreated())
        ImageWindow::imagewindow()->close(true);

    // Close and delete light table instance.

    if (LightTableWindow::lightTableWindowCreated())
        LightTableWindow::lightTableWindow()->close(true);

    if (d->view)
        delete d->view;

    d->albumIconViewFilter->saveSettings();
    d->albumSettings->setRecurseAlbums(d->recurseAlbumsAction->isChecked());
    d->albumSettings->setRecurseTags(d->recurseTagsAction->isChecked());
    d->albumSettings->saveSettings();
    delete d->albumSettings;

    delete d->albumManager;
    delete AlbumLister::instance();

    ImageAttributesWatch::cleanUp();
    LoadingCacheInterface::cleanUp();
#if KDCRAW_VERSION < 0x000106
    KDcrawIface::DcrawBinary::cleanUp();
#endif
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
                d->config->setGroup("Color Management");
                d->config->writeEntry("EnableCM", false);
                d->config->sync();
            }
        }
        else
        {
            d->config->setGroup("Color Management");
            d->config->writeEntry("EnableCM", false);
            d->config->sync();
        }
    }

#if KDCRAW_VERSION < 0x000106
    // Report errors from dcraw detection.
    KDcrawIface::DcrawBinary::instance()->checkReport();
#endif

    // Init album icon view zoom factor.
    slotThumbSizeChanged(d->albumSettings->getDefaultIconSize());
}

const QPtrList<KAction>& DigikamApp::menuImageActions()
{
    return d->kipiImageActions;
}

const QPtrList<KAction>& DigikamApp::menuBatchActions()
{
    return d->kipiBatchActions;
}

const QPtrList<KAction>& DigikamApp::menuAlbumActions()
{
    return d->kipiAlbumActions;
}

const QPtrList<KAction> DigikamApp::menuImportActions()
{
    QPtrList<KAction> importMenu;
    importMenu = d->kipiFileActionsImport;
    importMenu.append( d->albumImportAction );
    importMenu.append( d->addImagesAction );
    return importMenu;
}

const QPtrList<KAction> DigikamApp::menuExportActions()
{
    return d->kipiFileActionsExport;
}

void DigikamApp::autoDetect()
{
    // Called from main if command line option is set

    if(d->splashScreen)
        d->splashScreen->message(i18n("Auto-detect camera"));

    QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
}

void DigikamApp::downloadFrom(const QString &cameraGuiPath)
{
    // Called from main if command line option is set

    if (!cameraGuiPath.isNull())
    {
        d->cameraGuiPath = cameraGuiPath;

        if(d->splashScreen)
            d->splashScreen->message(i18n("Opening Download Dialog"));

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
        d->splashScreen->message(i18n("Initializing Main View"));

    d->view = new DigikamView(this);
    setCentralWidget(d->view);
    d->view->applySettings();

    connect(d->view, SIGNAL(signalAlbumSelected(bool)),
            this, SLOT(slotAlbumSelected(bool)));

    connect(d->view, SIGNAL(signalTagSelected(bool)),
            this, SLOT(slotTagSelected(bool)));

    connect(d->view, SIGNAL(signalImageSelected(const QPtrList<ImageInfo>&, bool, bool, const KURL::List&)),
            this, SLOT(slotImageSelected(const QPtrList<ImageInfo>&, bool, bool, const KURL::List&)));
}

void DigikamApp::setupStatusBar()
{
    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    d->statusProgressBar->setMaximumHeight(fontMetrics().height()+4);
    statusBar()->addWidget(d->statusProgressBar, 100, true);

    //------------------------------------------------------------------------------

    d->albumIconViewFilter = new AlbumIconViewFilter(statusBar());
    d->albumIconViewFilter->setMaximumHeight(fontMetrics().height()+4);
    statusBar()->addWidget(d->albumIconViewFilter, 100, true);

    //------------------------------------------------------------------------------

    d->statusZoomBar = new StatusZoomBar(statusBar());
    d->statusZoomBar->setMaximumHeight(fontMetrics().height()+4);
    statusBar()->addWidget(d->statusZoomBar, 1, true);

    //------------------------------------------------------------------------------

    d->statusNavigateBar = new StatusNavigateBar(statusBar());
    d->statusNavigateBar->setMaximumHeight(fontMetrics().height()+4);
    statusBar()->addWidget(d->statusNavigateBar, 1, true);

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
            this, SLOT(slotTogglePreview(bool)));

    connect(d->albumIconViewFilter, SIGNAL(signalResetTagFilters()),
            this, SIGNAL(signalResetTagFilters()));

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
    d->accelerators = new KAccel(this);

    d->accelerators->insert("Exit Preview Mode", i18n("Exit Preview"),
                           i18n("Exit preview mode"),
                           Key_Escape, this, SIGNAL(signalEscapePressed()),
                           false, true);

    d->accelerators->insert("Next Image Key_Space", i18n("Next Image"),
                           i18n("Next Image"),
                           Key_Space, this, SIGNAL(signalNextItem()),
                           false, true);

    d->accelerators->insert("Previous Image SHIFT+Key_Space", i18n("Previous Image"),
                           i18n("Previous Image"),
                           SHIFT+Key_Space, this, SIGNAL(signalPrevItem()),
                           false, true);

    d->accelerators->insert("Previous Image Key_Backspace", i18n("Previous Image"),
                           i18n("Previous Image"),
                           Key_Backspace, this, SIGNAL(signalPrevItem()),
                           false, true);

    d->accelerators->insert("Next Image Key_Next", i18n("Next Image"),
                           i18n("Next Image"),
                           Key_Next, this, SIGNAL(signalNextItem()),
                           false, true);

    d->accelerators->insert("Previous Image Key_Prior", i18n("Previous Image"),
                           i18n("Previous Image"),
                           Key_Prior, this, SIGNAL(signalPrevItem()),
                           false, true);

    d->accelerators->insert("First Image Key_Home", i18n("First Image"),
                           i18n("First Image"),
                           Key_Home, this, SIGNAL(signalFirstItem()),
                           false, true);

    d->accelerators->insert("Last Image Key_End", i18n("Last Image"),
                           i18n("Last Image"),
                           Key_End, this, SIGNAL(signalLastItem()),
                           false, true);

    d->accelerators->insert("Copy Album Items Selection CTRL+Key_C", i18n("Copy Album Items Selection"),
                           i18n("Copy Album Items Selection"),
                           CTRL+Key_C, this, SIGNAL(signalCopyAlbumItemsSelection()),
                           false, true);

    d->accelerators->insert("Paste Album Items Selection CTRL+Key_V", i18n("Paste Album Items Selection"),
                           i18n("Paste Album Items Selection"),
                           CTRL+Key_V, this, SIGNAL(signalPasteAlbumItemsSelection()),
                           false, true);
}

void DigikamApp::setupActions()
{
    // -----------------------------------------------------------------

    d->cameraMenuAction = new KActionMenu(i18n("&Camera"),
                                    "digitalcam",
                                    actionCollection(),
                                    "camera_menu");
    d->cameraMenuAction->setDelayed(false);

    // -----------------------------------------------------------------

    d->themeMenuAction = new KSelectAction(i18n("&Themes"), 0, actionCollection(), "theme_menu");
    connect(d->themeMenuAction, SIGNAL(activated(const QString&)),
            this, SLOT(slotChangeTheme(const QString&)));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // -----------------------------------------------------------------

    d->backwardActionMenu = new KToolBarPopupAction(i18n("&Back"),
                                    "back",
                                    ALT+Key_Left,
                                    d->view,
                                    SLOT(slotAlbumHistoryBack()),
                                    actionCollection(),
                                    "album_back");
    d->backwardActionMenu->setEnabled(false);

    connect(d->backwardActionMenu->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowBackwardMenu()));

    connect(d->backwardActionMenu->popupMenu(), SIGNAL(activated(int)),
            d->view, SLOT(slotAlbumHistoryBack(int)));

    d->forwardActionMenu = new  KToolBarPopupAction(i18n("Forward"),
                                    "forward",
                                    ALT+Key_Right,
                                    d->view,
                                    SLOT(slotAlbumHistoryForward()),
                                    actionCollection(),
                                    "album_forward");
    d->forwardActionMenu->setEnabled(false);

    connect(d->forwardActionMenu->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowForwardMenu()));

    connect(d->forwardActionMenu->popupMenu(), SIGNAL(activated(int)),
            d->view, SLOT(slotAlbumHistoryForward(int)));

    d->newAction = new KAction(i18n("&New..."),
                                   "albumfolder-new",
                                   KStdAccel::shortcut(KStdAccel::New),
                                   d->view,
                                   SLOT(slotNewAlbum()),
                                   actionCollection(),
                                   "album_new");
    d->newAction->setWhatsThis(i18n("Creates a new empty Album in the database."));

    d->albumSortAction = new KSelectAction(i18n("&Sort Albums"),
                                    0,
                                    0,
                                    actionCollection(),
                                    "album_sort");

    connect(d->albumSortAction, SIGNAL(activated(int)),
            d->view, SLOT(slotSortAlbums(int)));

    // Use same list order as in albumsettings enum
    QStringList sortActionList;
    sortActionList.append(i18n("By Folder"));
    sortActionList.append(i18n("By Collection"));
    sortActionList.append(i18n("By Date"));
    d->albumSortAction->setItems(sortActionList);

    d->recurseAlbumsAction = new KToggleAction(i18n("Include Album Sub-Tree"),
                                               0,
                                               this,
                                               0,
                                               actionCollection(),
                                               "albums_recursive");
    d->recurseAlbumsAction->setWhatsThis(i18n("Activate this option to recursively show all sub-albums below "
                                              "the current album."));

    connect(d->recurseAlbumsAction, SIGNAL(toggled(bool)),
            this, SLOT(slotRecurseAlbums(bool)));

    d->recurseTagsAction = new KToggleAction(i18n("Include Tag Sub-Tree"),
                                             0,
                                             this,
                                             0,
                                             actionCollection(),
                                             "tags_recursive");
    d->recurseTagsAction->setWhatsThis(i18n("Activate this option to show all images marked by the given tag "
                                            "and its all its sub-tags."));

    connect(d->recurseTagsAction, SIGNAL(toggled(bool)),
            this, SLOT(slotRecurseTags(bool)));

    d->deleteAction = new KAction(i18n("Delete"),
                                    "editdelete",
                                    0,
                                    d->view,
                                    SLOT(slotDeleteAlbum()),
                                    actionCollection(),
                                    "album_delete");

    d->addImagesAction = new KAction( i18n("Add Images..."),
                                    "albumfolder-importimages",
                                    CTRL+Key_I,
                                    this,
                                    SLOT(slotAlbumAddImages()),
                                    actionCollection(),
                                    "album_addImages");
    d->addImagesAction->setWhatsThis(i18n("Adds new items to the current Album."));

    d->albumImportAction = new KAction( i18n("Add Folders..."),
                                    "albumfolder-importdir",
                                    0,
                                    d->view,
                                    SLOT(slotAlbumImportFolder()),
                                    actionCollection(),
                                    "album_importFolder");

    d->propsEditAction = new KAction( i18n("Properties..."),
                                    "albumfolder-properties",
                                    0,
                                    d->view,
                                    SLOT(slotAlbumPropsEdit()),
                                    actionCollection(),
                                    "album_propsEdit");
    d->propsEditAction->setWhatsThis(i18n("Edit Album Properties and Collection information."));

    d->refreshAlbumAction = new KAction( i18n("Refresh"),
                                    "rebuild",
                                    Key_F5,
                                    d->view,
                                    SLOT(slotAlbumRefresh()),
                                    actionCollection(),
                                    "album_refresh");
    d->refreshAlbumAction->setWhatsThis(i18n("Refresh all album contents"));

    d->syncAlbumMetadataAction = new KAction( i18n("Synchronize Images with Database"),
                                    "rebuild",
                                    0,
                                    d->view,
                                    SLOT(slotAlbumSyncPicturesMetadata()),
                                    actionCollection(),
                                    "album_syncmetadata");
    d->syncAlbumMetadataAction->setWhatsThis(i18n("Updates all image metadata of the current "
                                                  "album with the contents of the digiKam database "
                                                  "(image metadata will be over-written with data from the database)."));

    d->openInKonquiAction = new KAction( i18n("Open in File Manager"),
                                    "konqueror",
                                    0,
                                    d->view,
                                    SLOT(slotAlbumOpenInKonqui()),
                                    actionCollection(),
                                    "album_openinkonqui");

    // -----------------------------------------------------------

    d->newTagAction = new KAction(i18n("New &Tag..."), "tag-new",
                                0, d->view, SLOT(slotNewTag()),
                                actionCollection(), "tag_new");

    d->editTagAction = new KAction(i18n("Edit Tag Properties..."), "tag-properties",
                                 0, d->view, SLOT(slotEditTag()),
                                 actionCollection(), "tag_edit");

    d->deleteTagAction = new KAction(i18n("Delete Tag"), "tag-delete",
                                   0, d->view, SLOT(slotDeleteTag()),
                                   actionCollection(), "tag_delete");

    // -----------------------------------------------------------

    d->imagePreviewAction = new KToggleAction(i18n("View..."),
                                    "viewimage",
                                    Key_F3,
                                    d->view,
                                    SLOT(slotImagePreview()),
                                    actionCollection(),
                                    "image_view");

    d->imageViewAction = new KAction(i18n("Edit..."),
                                    "editimage",
                                    Key_F4,
                                    d->view,
                                    SLOT(slotImageEdit()),
                                    actionCollection(),
                                    "image_edit");
    d->imageViewAction->setWhatsThis(i18n("Open the selected item in the image editor."));

    d->imageLightTableAction = new KAction(i18n("Place onto Light Table"),
                                    "lighttable",
                                    CTRL+Key_L,
                                    d->view,
                                    SLOT(slotImageLightTable()),
                                    actionCollection(),
                                    "image_lighttable");
    d->imageLightTableAction->setWhatsThis(i18n("Place the selected items on the light table thumbbar."));

    d->imageAddLightTableAction = new KAction(i18n("Add to Light Table"),
                                    "lighttableadd",
                                    SHIFT+CTRL+Key_L,
                                    d->view,
                                    SLOT(slotImageAddToLightTable()),
                                    actionCollection(),
                                    "image_add_to_lighttable");
    d->imageAddLightTableAction->setWhatsThis(i18n("Add selected items to the light table thumbbar."));

    d->imageRenameAction = new KAction(i18n("Rename..."),
                                    "pencil",
                                    Key_F2,
                                    d->view,
                                    SLOT(slotImageRename()),
                                    actionCollection(),
                                    "image_rename");
    d->imageRenameAction->setWhatsThis(i18n("Change the filename of the currently selected item."));

    // Pop up dialog to ask user whether to move to trash
    d->imageDeleteAction            = new KAction(i18n("Delete"),
                                                "edittrash",
                                                Key_Delete,
                                                d->view,
                                                SLOT(slotImageDelete()),
                                                actionCollection(),
                                                "image_delete");

    // Pop up dialog to ask user whether to permanently delete
    d->imageDeletePermanentlyAction = new KAction(i18n("Delete permanently"),
                                                "editdelete",
                                                SHIFT+Key_Delete,
                                                d->view,
                                                SLOT(slotImageDeletePermanently()),
                                                actionCollection(),
                                                "image_delete_permanently");

    // These two actions are hidden, no menu entry, no toolbar entry, no shortcut.
    // Power users may add them.
    d->imageDeletePermanentlyDirectlyAction = new KAction(i18n("Delete permanently without confirmation"),
                                                        "editdelete",
                                                        0,
                                                        d->view,
                                                        SLOT(slotImageDeletePermanentlyDirectly()),
                                                        actionCollection(),
                                                        "image_delete_permanently_directly");

    d->imageTrashDirectlyAction = new KAction(i18n("Move to trash without confirmation"),
                                              "edittrash",
                                               0,
                                               d->view,
                                               SLOT(slotImageTrashDirectly()),
                                               actionCollection(),
                                               "image_trash_directly");

    d->imageSortAction = new KSelectAction(i18n("&Sort Images"),
                                           0,
                                           0,
                                           actionCollection(),
                                           "image_sort");

    connect(d->imageSortAction, SIGNAL(activated(int)),
            d->view, SLOT(slotSortImages(int)));

    // Use same list order as in albumsettings enum
    QStringList sortImagesActionList;
    sortImagesActionList.append(i18n("By Name"));
    sortImagesActionList.append(i18n("By Path"));
    sortImagesActionList.append(i18n("By Date"));
    sortImagesActionList.append(i18n("By File Size"));
    sortImagesActionList.append(i18n("By Rating"));
    d->imageSortAction->setItems(sortImagesActionList);

    // -----------------------------------------------------------------

    QSignalMapper *exifOrientationMapper = new QSignalMapper( d->view );

    connect(exifOrientationMapper, SIGNAL(mapped(int) ),
            d->view, SLOT(slotImageExifOrientation(int)));

    d->imageExifOrientationActionMenu = new KActionMenu(i18n("Adjust Exif orientation tag"),
                                                        actionCollection(),
                                                        "image_set_exif_orientation");
    d->imageExifOrientationActionMenu->setDelayed(false);

    d->imageSetExifOrientation1Action = new KAction(i18n("Normal"),
             0,
             d->imageExifOrientationActionMenu,
             0,
             actionCollection(),
             "image_set_exif_orientation_normal");
    d->imageSetExifOrientation2Action = new KAction(i18n("Flipped Horizontally"),
             0,
             d->imageExifOrientationActionMenu,
             0,
             actionCollection(),
             "image_set_exif_orientation_flipped_horizontal");
    d->imageSetExifOrientation3Action = new KAction(i18n("Rotated Upside Down"),
             0,
             d->imageExifOrientationActionMenu,
             0,
             actionCollection(),
             "image_set_exif_orientation_rotated_upside_down");
    d->imageSetExifOrientation4Action = new KAction(i18n("Flipped Vertically"),
             0,
             d->imageExifOrientationActionMenu,
             0,
             actionCollection(),
             "image_set_exif_orientation_flipped_vertically");
    d->imageSetExifOrientation5Action = new KAction(i18n("Rotated Right / Horiz. Flipped"),
             0,
             d->imageExifOrientationActionMenu,
             0,
             actionCollection(),
             "image_set_exif_orientation_rotated_right_hor_flipped");
    d->imageSetExifOrientation6Action = new KAction(i18n("Rotated Right"),
             0,
             d->imageExifOrientationActionMenu,
             0,
             actionCollection(),
             "image_set_exif_orientation_rotated_right");
    d->imageSetExifOrientation7Action = new KAction(i18n("Rotated Right / Vert. Flipped"),
             0,
             d->imageExifOrientationActionMenu,
             0,
             actionCollection(),
             "image_set_exif_orientation_rotated_right_ver_flipped");
    d->imageSetExifOrientation8Action = new KAction(i18n("Rotated Left"),
             0,
             d->imageExifOrientationActionMenu,
             0,
             actionCollection(),
             "image_set_exif_orientation_rotated_left");

    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation1Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation2Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation3Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation4Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation5Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation6Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation7Action);
    d->imageExifOrientationActionMenu->insert(d->imageSetExifOrientation8Action);

    connect(d->imageSetExifOrientation1Action, SIGNAL(activated()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation2Action, SIGNAL(activated()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation3Action, SIGNAL(activated()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation4Action, SIGNAL(activated()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation5Action, SIGNAL(activated()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation6Action, SIGNAL(activated()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation7Action, SIGNAL(activated()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation8Action, SIGNAL(activated()),
            exifOrientationMapper, SLOT(map()));

    exifOrientationMapper->setMapping(d->imageSetExifOrientation1Action, 1);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation2Action, 2);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation3Action, 3);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation4Action, 4);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation5Action, 5);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation6Action, 6);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation7Action, 7);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation8Action, 8);

    // -----------------------------------------------------------------

    d->selectAllAction = new KAction(i18n("Select All"),
                                     0,
                                     CTRL+Key_A,
                                     d->view,
                                     SLOT(slotSelectAll()),
                                     actionCollection(),
                                     "selectAll");

    d->selectNoneAction = new KAction(i18n("Select None"),
                                     0,
                                     CTRL+SHIFT+Key_A,
                                     d->view,
                                     SLOT(slotSelectNone()),
                                     actionCollection(),
                                     "selectNone");

    d->selectInvertAction = new KAction(i18n("Invert Selection"),
                                        0,
                                        CTRL+Key_Asterisk,
                                        d->view,
                                        SLOT(slotSelectInvert()),
                                        actionCollection(),
                                        "selectInvert");

    // -----------------------------------------------------------

    d->showMenuBarAction = KStdAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());

    KStdAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStdAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStdAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // -----------------------------------------------------------

    d->zoomPlusAction = new KAction(i18n("Zoom In"),
                                   "viewmag+",
                                   CTRL+Key_Plus,
                                   d->view,
                                   SLOT(slotZoomIn()),
                                   actionCollection(),
                                   "album_zoomin");

    d->zoomMinusAction = new KAction(i18n("Zoom Out"),
                                   "viewmag-",
                                   CTRL+Key_Minus,
                                   d->view,
                                   SLOT(slotZoomOut()),
                                   actionCollection(),
                                   "album_zoomout");

    d->zoomTo100percents = new KAction(i18n("Zoom to 100%"),
                                   "viewmag1",
                                   ALT+CTRL+Key_0,      // NOTE: Photoshop 7 use ALT+CTRL+0.
                                   d->view,
                                   SLOT(slotZoomTo100Percents()),
                                   actionCollection(),
                                   "album_zoomto100percents");

    d->zoomFitToWindowAction = new KAction(i18n("Fit to &Window"),
                                   "view_fit_window",
                                   CTRL+SHIFT+Key_E,
                                   d->view,
                                   SLOT(slotFitToWindow()),
                                   actionCollection(),
                                   "album_zoomfit2window");

    // Do not use std KDE action for full screen because action text is too large for app. toolbar.
    d->fullScreenAction = new KToggleAction(i18n("Full Screen"),
                                   "window_fullscreen",
                                   CTRL+SHIFT+Key_F,
                                   this,
                                   SLOT(slotToggleFullScreen()),
                                   actionCollection(),
                                   "full_screen");
    d->fullScreenAction->setWhatsThis(i18n("Switch the window to full screen mode"));

    d->slideShowAction = new KActionMenu(i18n("Slideshow"), "slideshow",
                                         actionCollection(), "slideshow");

    d->slideShowAction->setDelayed(false);

    d->slideShowAllAction = new KAction(i18n("All"), 0, Key_F9,
                                d->view, SLOT(slotSlideShowAll()),
                                actionCollection(), "slideshow_all");
    d->slideShowAction->insert(d->slideShowAllAction);

    d->slideShowSelectionAction = new KAction(i18n("Selection"), 0, ALT+Key_F9,
                                              d->view,
                                              SLOT(slotSlideShowSelection()),
                                              actionCollection(),
                                              "slideshow_selected");
    d->slideShowAction->insert(d->slideShowSelectionAction);

    d->slideShowRecursiveAction = new KAction(i18n("With All Sub-Albums"), 0, SHIFT+Key_F9,
                                              d->view,
                                              SLOT(slotSlideShowRecursive()),
                                              actionCollection(),
                                              "slideshow_recursive");
    d->slideShowAction->insert(d->slideShowRecursiveAction);

    d->quitAction = KStdAction::quit(this,
                                   SLOT(slotExit()),
                                   actionCollection(),
                                   "app_exit");

    d->rawCameraListAction = new KAction(i18n("Supported RAW Cameras"),
                                   "kdcraw",
                                   0,
                                   this,
                                   SLOT(slotRawCameraList()),
                                   actionCollection(),
                                   "help_rawcameralist");

    d->kipiHelpAction = new KAction(i18n("Kipi Plugins Handbook"),
                                   "kipi",
                                   0,
                                   this,
                                   SLOT(slotShowKipiHelp()),
                                   actionCollection(),
                                   "help_kipi");

    d->tipAction = KStdAction::tipOfDay(this,
                                   SLOT(slotShowTip()),
                                   actionCollection(),
                                   "help_tipofday");

    d->donateMoneyAction = new KAction(i18n("Donate..."),
                                   0,
                                   0,
                                   this,
                                   SLOT(slotDonateMoney()),
                                   actionCollection(),
                                   "help_donatemoney");

    d->contributeAction = new KAction(i18n("Contribute..."),
                                      0, 0,
                                      this, SLOT(slotContribute()),
                                      actionCollection(),
                                      "help_contribute");

    new DLogoAction(actionCollection(), "logo_action" );

    // -- Rating actions ---------------------------------------------------------------

    d->rating0Star = new KAction(i18n("Assign Rating \"No Stars\""), CTRL+Key_0,
                          d->view, SLOT(slotAssignRatingNoStar()),
                          actionCollection(), "ratenostar");
    d->rating1Star = new KAction(i18n("Assign Rating \"One Star\""), CTRL+Key_1,
                          d->view, SLOT(slotAssignRatingOneStar()),
                          actionCollection(), "rateonestar");
    d->rating2Star = new KAction(i18n("Assign Rating \"Two Stars\""), CTRL+Key_2,
                          d->view, SLOT(slotAssignRatingTwoStar()),
                          actionCollection(), "ratetwostar");
    d->rating3Star = new KAction(i18n("Assign Rating \"Three Stars\""), CTRL+Key_3,
                          d->view, SLOT(slotAssignRatingThreeStar()),
                          actionCollection(), "ratethreestar");
    d->rating4Star = new KAction(i18n("Assign Rating \"Four Stars\""), CTRL+Key_4,
                          d->view, SLOT(slotAssignRatingFourStar()),
                          actionCollection(), "ratefourstar");
    d->rating5Star = new KAction(i18n("Assign Rating \"Five Stars\""), CTRL+Key_5,
                          d->view, SLOT(slotAssignRatingFiveStar()),
                          actionCollection(), "ratefivestar");

    // -----------------------------------------------------------

    KAction* findAction = KStdAction::find(d->view, SLOT(slotNewQuickSearch()),
                                           actionCollection(), "search_quick");
    findAction->setText(i18n("Search..."));
    findAction->setIconSet(BarIcon("filefind"));

    KAction* advFindAction = KStdAction::find(d->view, SLOT(slotNewAdvancedSearch()),
                                              actionCollection(), "search_advanced");
    advFindAction->setText(i18n("Advanced Search..."));
    advFindAction->setShortcut("Ctrl+Alt+F");

    new KAction(i18n("Light Table"), "idea", Key_L,
                d->view, SLOT(slotLightTable()), actionCollection(),
                "light_table");

    new KAction(i18n("Scan for New Images"), "reload_page", 0,
                this, SLOT(slotDatabaseRescan()), actionCollection(),
                "database_rescan");

    new KAction(i18n("Rebuild All Thumbnails..."), "reload_page", 0,
                this, SLOT(slotRebuildAllThumbs()), actionCollection(),
                "thumbs_rebuild");

    new KAction(i18n("Update Metadata Database..."), "reload_page", 0,
                this, SLOT(slotSyncAllPicturesMetadata()), actionCollection(),
                "sync_metadata");

    // -----------------------------------------------------------

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // Load Cameras -- do this before the createGUI so that the cameras
    // are plugged into the toolbar at startup
    if (d->splashScreen)
        d->splashScreen->message(i18n("Loading cameras"));

    loadCameras();

    createGUI(QString::fromLatin1( "digikamui.rc" ), false);

    // Initialize Actions ---------------------------------------

    d->deleteAction->setEnabled(false);
    d->addImagesAction->setEnabled(false);
    d->propsEditAction->setEnabled(false);
    d->openInKonquiAction->setEnabled(false);

    d->imageViewAction->setEnabled(false);
    d->imagePreviewAction->setEnabled(false);
    d->imageLightTableAction->setEnabled(false);
    d->imageAddLightTableAction->setEnabled(false);
    d->imageRenameAction->setEnabled(false);
    d->imageDeleteAction->setEnabled(false);
    d->imageExifOrientationActionMenu->setEnabled(false);
    d->slideShowSelectionAction->setEnabled(false);

    d->albumSortAction->setCurrentItem((int)d->albumSettings->getAlbumSortOrder());
    d->imageSortAction->setCurrentItem((int)d->albumSettings->getImageSortOrder());

    d->recurseAlbumsAction->setChecked(d->albumSettings->getRecurseAlbums());
    d->recurseTagsAction->setChecked(d->albumSettings->getRecurseTags());
    slotRecurseAlbums(d->recurseAlbumsAction->isChecked());
    slotRecurseTags(d->recurseTagsAction->isChecked());

    // Setting the filter condition also updates the tooltip.
    // (So `setRating` is called first, as otherwise the filter value is not respected).
    d->albumIconViewFilter->readSettings();
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
    d->backwardActionMenu->popupMenu()->clear();
    QStringList titles;
    d->view->getBackwardHistory(titles);
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();
        for(; iter != titles.end(); ++iter,++id)
        {
            d->backwardActionMenu->popupMenu()->insertItem(*iter, id);
        }
    }
}

void DigikamApp::slotAboutToShowForwardMenu()
{
    d->forwardActionMenu->popupMenu()->clear();
    QStringList titles;
    d->view->getForwardHistory(titles);

    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();
        for(; iter != titles.end(); ++iter,++id)
        {
            d->forwardActionMenu->popupMenu()->insertItem(*iter, id);
        }
    }
}

void DigikamApp::slotAlbumSelected(bool val)
{
    Album *album = d->albumManager->currentAlbum();

    if(album && !val)
    {
        // Not a PAlbum is selected
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

        KAction *action;
        for (action = d->kipiFileActionsImport.first(); action;
             action = d->kipiFileActionsImport.next())
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

        KAction *action;
        for (action = d->kipiFileActionsImport.first(); action;
             action = d->kipiFileActionsImport.next())
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

        KAction *action;
        for (action = d->kipiFileActionsImport.first(); action;
             action = d->kipiFileActionsImport.next())
        {
            action->setEnabled(false);
        }
    }
}

void DigikamApp::slotTagSelected(bool val)
{
    Album *album = d->albumManager->currentAlbum();
    if (!album) return;

    if(!val)
    {
        d->deleteTagAction->setEnabled(false);
        d->editTagAction->setEnabled(false);
    }
    else if(!album->isRoot())
    {
        d->deleteTagAction->setEnabled(true);
        d->editTagAction->setEnabled(true);

        KAction *action;
        for (action = d->kipiFileActionsImport.first(); action;
             action = d->kipiFileActionsImport.next())
        {
            action->setEnabled(false);
        }
    }
    else
    {
        d->deleteTagAction->setEnabled(false);
        d->editTagAction->setEnabled(false);

        KAction *action;
        for (action = d->kipiFileActionsImport.first(); action;
             action = d->kipiFileActionsImport.next())
        {
            action->setEnabled(false);
        }
    }
}

void DigikamApp::slotImageSelected(const QPtrList<ImageInfo>& list, bool hasPrev, bool hasNext,
                                   const KURL::List& listAll)
{
    QPtrList<ImageInfo> selection = list;
    KURL::List all                = listAll;
    int num_images                = listAll.count();
    bool val                      = selection.isEmpty() ? false : true;
    QString text;
    int index = 1;

    d->imageViewAction->setEnabled(val);
    d->imagePreviewAction->setEnabled(val);
    d->imageLightTableAction->setEnabled(val);
    d->imageAddLightTableAction->setEnabled(val);
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
        {
            KURL first = selection.first()->kurl();

            for (KURL::List::iterator it = all.begin();
                it != all.end(); ++it)
            {
                if ((*it) == first)
                    break;

                index++;
            }

            text = selection.first()->kurl().fileName()
                                   + i18n(" (%1 of %2)")
                                   .arg(QString::number(index))
                                   .arg(QString::number(num_images));
            d->statusProgressBar->setText(text);
            break;
        }
        default:
          d->statusProgressBar->setText(i18n("%1/%2 items selected")
                                .arg(selection.count()).arg(QString::number(num_images)));

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
    if (d->fullScreen)
    {
        slotToggleFullScreen();
        QTimer::singleShot(0, this, SLOT(close()));
    }
    else
        close();
}

QString DigikamApp::convertToLocalUrl( const QString& folder )
{
    // This function is copied from k3b.

    KURL url( folder );
    if( !url.isLocalFile() )
    {
#if KDE_IS_VERSION(3,4,91)
        // Support for system:/ and media:/ (c) Stephan Kulow
        KURL mlu = KIO::NetAccess::mostLocalURL( url, 0 );
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
        }
        else
        {
            DWarning() << "dcop call failed\n";
        }

        return path;
#else
#ifndef UDS_LOCAL_PATH
#define UDS_LOCAL_PATH (72 | KIO::UDS_STRING)
#else
        using namespace KIO;
#endif
        KIO::UDSEntry e;
        if( KIO::NetAccess::stat( url, e, 0 ) )
        {
            const KIO::UDSEntry::ConstIterator end = e.end();
            for( KIO::UDSEntry::ConstIterator it = e.begin(); it != end; ++it )
            {
                if( (*it).m_uds == UDS_LOCAL_PATH && !(*it).m_str.isEmpty() )
                    return KURL::fromPathOrURL( (*it).m_str ).path();
            }
        }
#endif
    }

    return url.path();
}

void DigikamApp::slotDcopDownloadImages( const QString& folder )
{
    if (!folder.isNull())
    {
        // activate window when called by media menu and DCOP
        if (isMinimized())
            KWin::deIconifyWindow(winId());
        KWin::activateWindow(winId());

        slotDownloadImages(folder);
    }
}

void DigikamApp::slotDcopCameraAutoDetect()
{
    // activate window when called by media menu and DCOP
    if (isMinimized())
        KWin::deIconifyWindow(winId());
    KWin::activateWindow(winId());

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
    KIO::ListJob *job = KIO::listDir(KURL(d->cameraGuiPath), false, false);
    KIO::NetAccess::synchronousRun(job,0);

    QString localUrl = convertToLocalUrl(d->cameraGuiPath);
    DDebug() << "slotDownloadImages: convertToLocalUrl " << d->cameraGuiPath << " to " << localUrl << endl;

    if (localUrl.isNull())
        return;

    bool alreadyThere = false;

    for (uint i = 0 ; i != actionCollection()->count() ; i++)
    {
        if (actionCollection()->action(i)->name() == d->cameraGuiPath)
            alreadyThere = true;
    }

    if (!alreadyThere)
    {
        KAction *cAction  = new KAction(
                 i18n("Browse %1").arg(KURL(d->cameraGuiPath).prettyURL()),
                 "camera",
                 0,
                 this,
                 SLOT(slotDownloadImages()),
                 actionCollection(),
                 d->cameraGuiPath.latin1() );

        d->cameraMenuAction->insert(cAction, 0);
    }

    // the CameraUI will delete itself when it has finished
    CameraUI* cgui = new CameraUI(this,
                                  i18n("Images found in %1").arg(d->cameraGuiPath),
                                  "directory browse","Fixed", localUrl, QDateTime::currentDateTime());
    cgui->show();

    connect(cgui, SIGNAL(signalLastDestination(const KURL&)),
            d->view, SLOT(slotSelectAlbum(const KURL&)));

    connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
            this, SLOT(slotSetupChanged()));
}

void DigikamApp::slotCameraConnect()
{
    CameraType* ctype = d->cameraList->find(QString::fromUtf8(sender()->name()));

    if (ctype)
    {
        // check not to open two dialogs for the same camera
        if (ctype->currentCameraUI() && !ctype->currentCameraUI()->isClosed())
        {
            // show and raise dialog
            if (ctype->currentCameraUI()->isMinimized())
                KWin::deIconifyWindow(ctype->currentCameraUI()->winId());
            KWin::activateWindow(ctype->currentCameraUI()->winId());
        }
        else
        {
            // the CameraUI will delete itself when it has finished
            CameraUI* cgui = new CameraUI(this, ctype->title(), ctype->model(),
                                          ctype->port(), ctype->path(), ctype->lastAccess());

            ctype->setCurrentCameraUI(cgui);

            cgui->show();

            connect(cgui, SIGNAL(signalLastDestination(const KURL&)),
                    d->view, SLOT(slotSelectAlbum(const KURL&)));

            connect(cgui, SIGNAL(signalAlbumSettingsChanged()),
                    this, SLOT(slotSetupChanged()));
        }
    }
}

void DigikamApp::slotCameraAdded(CameraType *ctype)
{
    if (!ctype) return;

    KAction *cAction = new KAction(ctype->title(), "camera", 0,
                                   this, SLOT(slotCameraConnect()),
                                   actionCollection(),
                                   ctype->title().utf8());
    d->cameraMenuAction->insert(cAction, 0);
    ctype->setAction(cAction);
}

void DigikamApp::slotCameraMediaMenu()
{
    d->mediaItems.clear();

    d->cameraMediaList->clear();
    d->cameraMediaList->insertItem(i18n("No media devices found"), 0);
    d->cameraMediaList->setItemEnabled(0, false);

    KURL kurl("media:/");
    KIO::ListJob *job = KIO::listDir(kurl, false, false);

    connect( job, SIGNAL(entries(KIO::Job*,const KIO::UDSEntryList&)),
             this, SLOT(slotCameraMediaMenuEntries(KIO::Job*,const KIO::UDSEntryList&)) );
}

void DigikamApp::slotCameraMediaMenuEntries( Job *, const UDSEntryList & list )
{
    int i = 0;

    for(KIO::UDSEntryList::ConstIterator it = list.begin() ; it != list.end() ; ++it)
    {
        QString name;
        QString path;

        for ( UDSEntry::const_iterator et = (*it).begin() ; et != (*it).end() ; ++et )
        {
            if ( (*et).m_uds == KIO::UDS_NAME)
                name = ( *et ).m_str;
            if ( (*et).m_uds == KIO::UDS_URL)
                path = ( *et ).m_str;

            //DDebug() << ( *et ).m_str << endl;
        }

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
        d->cameraMenuAction->remove(cAction);
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
        ctype->action()->activate();
    }
}

void DigikamApp::slotSetup()
{
    setup();
}

bool DigikamApp::setup(bool iccSetupPage)
{
    Setup setup(this, 0, iccSetupPage ? Setup::IccProfiles : Setup::LastPageUsed);

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
    Setup setup(this, 0, Setup::Camera);

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

    if(d->albumSettings->getAlbumLibraryPath() != d->albumManager->getLibraryPath())
        d->view->clearHistory();

    d->albumManager->setLibraryPath(d->albumSettings->getAlbumLibraryPath());
    d->albumManager->startScan();

    if(d->albumSettings->getShowFolderTreeViewItemsCount())
        d->albumManager->refresh();

    d->view->applySettings();
    d->albumIconViewFilter->readSettings();

    AlbumThumbnailLoader::instance()->setThumbnailSize(d->albumSettings->getDefaultTreeIconSize());

    if (ImageWindow::imagewindowCreated())
        ImageWindow::imagewindow()->applySettings();

    if (LightTableWindow::lightTableWindowCreated())
        LightTableWindow::lightTableWindow()->applySettings();

    d->config->sync();
}

void DigikamApp::slotEditKeys()
{
    KKeyDialog* dialog = new KKeyDialog();
    dialog->insert( actionCollection(), i18n( "General" ) );

    KIPI::PluginLoader::PluginList list = d->kipiPluginLoader->pluginList();

    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin() ; it != list.end() ; ++it )
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if ( plugin )
           dialog->insert( plugin->actionCollection(), (*it)->comment() );
    }

    dialog->configure();
    delete dialog;
}

void DigikamApp::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config());
    KEditToolbar *dlg = new KEditToolbar(actionCollection(), "digikamui.rc");

    if(dlg->exec())
    {
        createGUI(QString::fromLatin1( "digikamui.rc" ), false);
        applyMainWindowSettings(KGlobal::config());
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
        setWindowState( windowState() & ~WindowFullScreen );
#else
        showNormal();
#endif
        menuBar()->show();
        statusBar()->show();
        topDock()->show();
        bottomDock()->show();
        leftDock()->show();
        rightDock()->show();
        d->view->showSideBars();

        d->fullScreen = false;
    }
    else
    {
        KConfig* config = kapp->config();
        config->setGroup("ImageViewer Settings");
        bool fullScreenHideToolBar = config->readBoolEntry("FullScreen Hide ToolBar", false);

        menuBar()->hide();
        statusBar()->hide();
        if (fullScreenHideToolBar)
            topDock()->hide();
        bottomDock()->hide();
        leftDock()->hide();
        rightDock()->hide();
        d->view->hideSideBars();

        showFullScreen();
        d->fullScreen = true;
    }
}

void DigikamApp::slotShowTip()
{
#if KDE_IS_VERSION(3,2,0)
    QStringList tipsFiles;
    tipsFiles.append("digikam/tips");

    tipsFiles.append("kipi/tips");

    KTipDialog::showMultiTip(this, tipsFiles, true);
#else
    KTipDialog::showTip(this, "digikam/tips", true);
#endif
}

void DigikamApp::slotShowKipiHelp()
{
    KApplication::kApplication()->invokeHelp( QString(), "kipi-plugins" );
}

void DigikamApp::slotRawCameraList()
{
    RawCameraDlg dlg(this);
    dlg.exec();
}

void DigikamApp::loadPlugins()
{
    if(d->splashScreen)
        d->splashScreen->message(i18n("Loading Kipi Plugins"));

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

        plugin->setup( this );
        QPtrList<KAction>* popup = 0;

        // Plugin category identification using KAction method based.

        KActionPtrList actions = plugin->actions();

        // List of obsolete kipi-plugins to not load.
        QStringList pluginActionsDisabled;
        pluginActionsDisabled << QString("raw_converter_single");  // Obsolete Since 0.9.5 and new Raw Import tool.

        for( KActionPtrList::Iterator it2 = actions.begin(); it2 != actions.end(); ++it2 )
        {
            if ( plugin->category(*it2) == KIPI::IMAGESPLUGIN )
               popup = &d->kipiImageActions;

            else if ( plugin->category(*it2) == KIPI::EXPORTPLUGIN )
               popup = &d->kipiFileActionsExport;

            else if ( plugin->category(*it2) == KIPI::IMPORTPLUGIN )
               popup = &d->kipiFileActionsImport;

            else if ( plugin->category(*it2) == KIPI::TOOLSPLUGIN )
               popup = &d->kipiToolsActions;

            else if ( plugin->category(*it2) == KIPI::BATCHPLUGIN )
               popup = &d->kipiBatchActions;

            else if ( plugin->category(*it2) == KIPI::COLLECTIONSPLUGIN )
               popup = &d->kipiAlbumActions;

            QString actionName((*it2)->name());

            // Plug the KIPI plugins actions in according with the KAction method.

            if (popup)
            {
                if (!pluginActionsDisabled.contains(actionName))
                    popup->append( *it2 );
                else
                    DDebug() << "Plugin '" << actionName << "' disabled." << endl;
            }
            else
            {
                DDebug() << "No menu found for plugin '" << actionName << "' !!!" << endl;
            }
        }

        plugin->actionCollection()->readShortcutSettings();
    }

    // Create GUI menu in according with plugins.

    plugActionList( QString::fromLatin1("file_actions_export"), d->kipiFileActionsExport );
    plugActionList( QString::fromLatin1("file_actions_import"), d->kipiFileActionsImport );
    plugActionList( QString::fromLatin1("image_actions"), d->kipiImageActions );
    plugActionList( QString::fromLatin1("tool_actions"), d->kipiToolsActions );
    plugActionList( QString::fromLatin1("batch_actions"), d->kipiBatchActions );
    plugActionList( QString::fromLatin1("album_actions"), d->kipiAlbumActions );
}

void DigikamApp::loadCameras()
{
    d->cameraList->load();

    d->cameraMenuAction->popupMenu()->insertSeparator();

    d->cameraMenuAction->popupMenu()->insertItem(i18n("Browse Media"), d->cameraMediaList);

    d->cameraMenuAction->popupMenu()->insertSeparator();

    d->cameraMenuAction->insert(new KAction(i18n("Add Camera..."), 0,
                                          this, SLOT(slotSetupCamera()),
                                          actionCollection(),
                                          "camera_add"));
}

void DigikamApp::populateThemes()
{
    if(d->splashScreen)
        d->splashScreen->message(i18n("Loading themes"));

    ThemeEngine::instance()->scanThemes();
    d->themeMenuAction->setItems(ThemeEngine::instance()->themeNames());
    slotThemeChanged();
    ThemeEngine::instance()->slotChangeTheme(d->themeMenuAction->currentText());
}

void DigikamApp::slotChangeTheme(const QString& theme)
{
    d->albumSettings->setCurrentTheme(theme);
    ThemeEngine::instance()->slotChangeTheme(theme);
}

void DigikamApp::slotThemeChanged()
{
    QStringList themes(ThemeEngine::instance()->themeNames());
    int index = themes.findIndex(d->albumSettings->getCurrentTheme());
    if (index == -1)
        index = themes.findIndex(i18n("Default"));

    d->themeMenuAction->setCurrentItem(index);
}

void DigikamApp::slotDatabaseRescan()
{
    ScanLib sLib;
    sLib.startScan();

    d->view->refreshView();

    if (ImageWindow::imagewindowCreated())
        ImageWindow::imagewindow()->refreshView();

    if (LightTableWindow::lightTableWindowCreated())
        LightTableWindow::lightTableWindow()->refreshView();
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
    d->view->applySettings();
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
    d->view->applySettings();
}

void DigikamApp::slotDonateMoney()
{
    KApplication::kApplication()->invokeBrowser("http://www.digikam.org/?q=donation");
}

void DigikamApp::slotContribute()
{
    KApplication::kApplication()->invokeBrowser("http://www.digikam.org/?q=contrib");
}

void DigikamApp::slotRecurseAlbums(bool checked)
{
    AlbumLister::instance()->setRecurseAlbums(checked);
}

void DigikamApp::slotRecurseTags(bool checked)
{
    AlbumLister::instance()->setRecurseTags(checked);
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

void DigikamApp::slotTogglePreview(bool t)
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

void DigikamApp::slotAlbumAddImages()
{
    QString path = KFileDialog::getExistingDirectory(KGlobalSettings::documentPath(), this,
                                i18n("Select folder to parse"));

    if(path.isEmpty())
        return;

    // The folder contents will be parsed by Camera interface in "Directory Browse" mode.
    downloadFrom(path);
}

void DigikamApp::slotShowMenuBar()
{
    if (menuBar()->isVisible())
        menuBar()->hide();
    else
        menuBar()->show();
}

}  // namespace Digikam
