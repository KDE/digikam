/* ============================================================
*
* This file is a part of digiKam project
* http://www.digikam.org
*
* Date        : 2004-09-16
* Description : Camera interface
*
* Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
* Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
* Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "cameraui.moc"
#include "cameraui_p.h"

// Qt includes

#include <QCheckBox>
#include <QCloseEvent>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

// KDE includes

#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcalendarsystem.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <kinputdialog.h>
#include <kio/global.h>
#include <kio/previewjob.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <knotifyconfigwidget.h>
#include <kshortcutsdialog.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>
#include <kvbox.h>
#include <kdebug.h>

// Libkdcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>
#include <libkdcraw/rexpanderbox.h>

// Local includes

#include "componentsinfo.h"
#include "statusprogressbar.h"
#include "statusnavigatebar.h"
#include "dlogoaction.h"
#include "thumbnailsize.h"
#include "sidebar.h"
#include "thememanager.h"
#include "templateselector.h"
#include "setup.h"
#include "downloadsettingscontainer.h"
#include "downloadhistory.h"
#include "dzoombar.h"
#include "imagepropertiessidebarcamgui.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "album.h"
#include "albumselectdialog.h"
#include "renamecustomizer.h"
#include "freespacewidget.h"
#include "collectionscanner.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "scancontroller.h"
#include "capturedlg.h"
#include "camerafolderdialog.h"
#include "camerainfodialog.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"
#include "cameracontroller.h"
#include "cameralist.h"
#include "cameratype.h"
#include "cameranamehelper.h"
#include "uifilevalidator.h"
#include "knotificationwrapper.h"

using namespace KDcrawIface;

namespace Digikam
{

CameraUI::CameraUI(QWidget* const parent, const QString& cameraTitle,
                   const QString& model, const QString& port,
                   const QString& path, int startIndex)
    : KXmlGuiWindow(parent), d(new CameraUIPriv)

{
    setXMLFile("cameraui.rc");

    // --------------------------------------------------------

    UiFileValidator validator(localXMLFile());

    if (!validator.isValid())
    {
        validator.fixConfigFile();
    }

    // --------------------------------------------------------

    QString title  = CameraNameHelper::formattedCameraName(cameraTitle);
    d->cameraTitle = (title.isEmpty()) ? cameraTitle : title;
    setCaption(d->cameraTitle);

    // -------------------------------------------------------------------

    d->refreshIconViewTimer = new QTimer(this);
    d->refreshIconViewTimer->setInterval(0);
    d->refreshIconViewTimer->setSingleShot(true);

    connect(d->refreshIconViewTimer, SIGNAL(timeout()),
            this, SLOT(slotRefreshIconViewTimer()));

    // -------------------------------------------------------------------

    setupUserArea();
    setupActions();
    setupStatusBar();
    setupAccelerators();

    // -- Make signals/slots connections ---------------------------------

    setupConnections();
    slotSidebarTabTitleStyleChanged();

    // -- Read settings --------------------------------------------------

    readSettings();
    setAutoSaveSettings("Camera Settings", true);

    // -- Init. camera controller ----------------------------------------

    setupCameraController(model, port, path);

    // --------------------------------------------------------

    d->historyUpdater = new CameraHistoryUpdater(this);

    connect (d->historyUpdater, SIGNAL(signalHistoryMap(CHUpdateItemMap)),
             this, SLOT(slotRefreshIconView(CHUpdateItemMap)));

    connect(d->historyUpdater, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    // --------------------------------------------------------

    d->renameCustomizer->setStartIndex(startIndex);
    d->view->setFocus();
    QTimer::singleShot(0, d->controller, SLOT(slotConnect()));
}

CameraUI::~CameraUI()
{
    disconnect(d->view, 0, this, 0);
    delete d->view;
    delete d->rightSideBar;
    delete d->controller;
    delete d;
}

void CameraUI::setupUserArea()
{
    KHBox* widget   = new KHBox(this);
    d->splitter     = new SidebarSplitter(widget);
    KVBox* vbox     = new KVBox(d->splitter);
    d->view         = new CameraIconView(this, vbox);
    d->historyView  = new DHistoryView(vbox);
    d->rightSideBar = new ImagePropertiesSideBarCamGui(widget, d->splitter, KMultiTabBar::Right, true);
    d->rightSideBar->setObjectName("CameraGui Sidebar Right");
    d->splitter->setFrameStyle( QFrame::NoFrame );
    d->splitter->setFrameShadow( QFrame::Plain );
    d->splitter->setFrameShape( QFrame::NoFrame );
    d->splitter->setOpaqueResize(false);
    d->splitter->setStretchFactor(0, 10);      // set iconview default size to max.

    vbox->setStretchFactor(d->view, 10);
    vbox->setStretchFactor(d->historyView,  2);
    vbox->setMargin(0);
    vbox->setSpacing(0);

    // -------------------------------------------------------------------------

    d->advBox = new RExpanderBox(d->rightSideBar);
    d->advBox->setObjectName("Camera Settings Expander");

    d->renameCustomizer = new RenameCustomizer(d->advBox, d->cameraTitle);
    d->renameCustomizer->setWhatsThis( i18n("Set how digiKam will rename files as they are downloaded."));
    d->view->setRenameCustomizer(d->renameCustomizer);
    d->advBox->addItem(d->renameCustomizer, SmallIcon("insert-image"), i18n("File Renaming Options"),
                       QString("RenameCustomizer"), true);

    // -- Albums Auto-creation options -----------------------------------------

    QWidget* albumBox      = new QWidget(d->advBox);
    QVBoxLayout* albumVlay = new QVBoxLayout(albumBox);
    d->autoAlbumExtCheck   = new QCheckBox(i18n("Extension-based sub-albums"), albumBox);
    d->autoAlbumDateCheck  = new QCheckBox(i18n("Date-based sub-albums"), albumBox);
    KHBox* hbox1           = new KHBox(albumBox);
    d->folderDateLabel     = new QLabel(i18n("Date format:"), hbox1);
    d->folderDateFormat    = new KComboBox(hbox1);
    d->folderDateFormat->insertItem(CameraUIPriv::IsoDateFormat,   i18n("ISO"));
    d->folderDateFormat->insertItem(CameraUIPriv::TextDateFormat,  i18n("Full Text"));
    d->folderDateFormat->insertItem(CameraUIPriv::LocalDateFormat, i18n("Local Settings"));

    albumVlay->addWidget(d->autoAlbumExtCheck);
    albumVlay->addWidget(d->autoAlbumDateCheck);
    albumVlay->addWidget(hbox1);
    albumVlay->addStretch();
    albumVlay->setMargin(KDialog::spacingHint());
    albumVlay->setSpacing(KDialog::spacingHint());

    albumBox->setWhatsThis( i18n("Set how digiKam creates albums automatically when downloading."));
    d->autoAlbumExtCheck->setWhatsThis( i18n("Enable this option if you want to download your "
                                             "pictures into automatically created file extension-based sub-albums of the destination "
                                             "album. This way, you can separate JPEG and RAW files as they are downloaded from your camera."));
    d->autoAlbumDateCheck->setWhatsThis( i18n("Enable this option if you want to "
                                              "download your pictures into automatically created file date-based sub-albums "
                                              "of the destination album."));
    d->folderDateFormat->setWhatsThis( i18n("<p>Select your preferred date format used to "
                                            "create new albums. The options available are:</p>"
                                            "<p><b>ISO</b>: the date format is in accordance with ISO 8601 "
                                            "(YYYY-MM-DD). E.g.: <i>2006-08-24</i></p>"
                                            "<p><b>Full Text</b>: the date format is in a user-readable string. "
                                            "E.g.: <i>Thu Aug 24 2006</i></p>"
                                            "<p><b>Local Settings</b>: the date format depending on KDE control panel settings.</p>"));

    d->advBox->addItem(albumBox, SmallIcon("folder-new"), i18n("Auto-creation of Albums"),
                       QString("AlbumBox"), false);

    // -- On the Fly options ---------------------------------------------------

    QWidget* onFlyBox      = new QWidget(d->advBox);
    QVBoxLayout* onFlyVlay = new QVBoxLayout(onFlyBox);
    d->templateSelector    = new TemplateSelector(onFlyBox);
    d->fixDateTimeCheck    = new QCheckBox(i18n("Fix internal date && time"), onFlyBox);
    d->dateTimeEdit        = new DDateTimeEdit(onFlyBox, "datepicker");
    d->autoRotateCheck     = new QCheckBox(i18n("Auto-rotate/flip image"), onFlyBox);
    d->convertJpegCheck    = new QCheckBox(i18n("Convert to lossless file format"), onFlyBox);
    KHBox* hbox2           = new KHBox(onFlyBox);
    d->formatLabel         = new QLabel(i18n("New image format:"), hbox2);
    d->losslessFormat      = new KComboBox(hbox2);
    d->losslessFormat->insertItem(0, "PNG");
    d->losslessFormat->insertItem(1, "TIF");
    d->losslessFormat->insertItem(2, "JP2");
    d->losslessFormat->insertItem(3, "PGF");

    onFlyVlay->addWidget(d->templateSelector);
    onFlyVlay->addWidget(d->fixDateTimeCheck);
    onFlyVlay->addWidget(d->dateTimeEdit);
    onFlyVlay->addWidget(d->autoRotateCheck);
    onFlyVlay->addWidget(d->convertJpegCheck);
    onFlyVlay->addWidget(hbox2);
    onFlyVlay->addStretch();
    onFlyVlay->setMargin(KDialog::spacingHint());
    onFlyVlay->setSpacing(KDialog::spacingHint());

    onFlyBox->setWhatsThis( i18n("Set here all options to fix/transform JPEG files automatically "
                                 "as they are downloaded."));
    d->autoRotateCheck->setWhatsThis( i18n("Enable this option if you want images automatically "
                                           "rotated or flipped using EXIF information provided by the camera."));
    d->templateSelector->setWhatsThis( i18n("Select here which metadata template you want to apply "
                                            "to images."));
    d->fixDateTimeCheck->setWhatsThis( i18n("Enable this option to set date and time metadata "
                                            "tags to the right values if your camera does not set "
                                            "these tags correctly when pictures are taken. The values will "
                                            "be saved in the DateTimeDigitized and DateTimeCreated EXIF, XMP, and IPTC tags."));
    d->convertJpegCheck->setWhatsThis( i18n("Enable this option to automatically convert "
                                            "all JPEG files to a lossless image format. <b>Note:</b> Image conversion can take a "
                                            "while on a slow computer."));
    d->losslessFormat->setWhatsThis( i18n("Select your preferred lossless image file format to "
                                          "convert to. <b>Note:</b> All metadata will be preserved during the conversion."));

    d->advBox->addItem(onFlyBox, SmallIcon("system-run"), i18n("On the Fly Operations (JPEG only)"),
                       QString("OnFlyBox"), true);
    d->advBox->addStretch();

    d->rightSideBar->appendTab(d->advBox, SmallIcon("configure"), i18n("Settings"));
    d->rightSideBar->loadState();

    // -------------------------------------------------------------------------

    setCentralWidget(widget);
}

void CameraUI::setupActions()
{
    // -- File menu ----------------------------------------------------

    d->cameraCancelAction = new KAction(KIcon("process-stop"), i18n("Cancel"), this);
    connect(d->cameraCancelAction, SIGNAL(triggered()), this, SLOT(slotCancelButton()));
    actionCollection()->addAction("cameraui_cancelprocess", d->cameraCancelAction);
    d->cameraCancelAction->setEnabled(false);

    // -----------------------------------------------------------------

    d->cameraInfoAction = new KAction(KIcon("camera-photo"), i18n("Information"), this);
    connect(d->cameraInfoAction, SIGNAL(triggered()), this, SLOT(slotInformation()));
    actionCollection()->addAction("cameraui_info", d->cameraInfoAction);

    // -----------------------------------------------------------------

    d->cameraCaptureAction = new KAction(KIcon("webcamreceive"), i18n("Capture"), this);
    connect(d->cameraCaptureAction, SIGNAL(triggered()), this, SLOT(slotCapture()));
    actionCollection()->addAction("cameraui_capture", d->cameraCaptureAction);

    // -----------------------------------------------------------------

    KAction* closeAction = KStandardAction::close(this, SLOT(close()), this);
    actionCollection()->addAction("cameraui_close", closeAction);

    // -- Edit menu ----------------------------------------------------

    d->selectAllAction = new KAction(i18n("Select All"), this);
    d->selectAllAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_A));
    connect(d->selectAllAction, SIGNAL(triggered()), d->view, SLOT(slotSelectAll()));
    actionCollection()->addAction("cameraui_selectall", d->selectAllAction);

    // -----------------------------------------------------------------

    d->selectNoneAction = new KAction(i18n("Select None"), this);
    d->selectNoneAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_A));
    connect(d->selectNoneAction, SIGNAL(triggered()), d->view, SLOT(slotSelectNone()));
    actionCollection()->addAction("cameraui_selectnone", d->selectNoneAction);

    // -----------------------------------------------------------------

    d->selectInvertAction = new KAction(i18n("Invert Selection"), this);
    d->selectInvertAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_Asterisk));
    connect(d->selectInvertAction, SIGNAL(triggered()), d->view, SLOT(slotSelectInvert()));
    actionCollection()->addAction("cameraui_selectinvert", d->selectInvertAction);

    // -----------------------------------------------------------

    d->selectNewItemsAction = new KAction(KIcon("document-new"), i18n("Select New Items"), this);
    connect(d->selectNewItemsAction, SIGNAL(triggered()), d->view, SLOT(slotSelectNew()));
    actionCollection()->addAction("cameraui_selectnewitems", d->selectNewItemsAction);

    // -----------------------------------------------------------

    d->selectLockedItemsAction = new KAction(KIcon("object-locked"), i18n("Select Locked Items"), this);
    connect(d->selectLockedItemsAction, SIGNAL(triggered()), d->view, SLOT(slotSelectLocked()));
    actionCollection()->addAction("cameraui_selectlockeditems", d->selectLockedItemsAction);

    // -- Image menu ---------------------------------------------

    d->imageViewAction = new KAction(KIcon("editimage"), i18nc("View the selected image", "View"), this);
    connect(d->imageViewAction, SIGNAL(triggered()), this, SLOT(slotFileView()));
    actionCollection()->addAction("cameraui_imageview", d->imageViewAction);
    d->imageViewAction->setEnabled(false);

    // -----------------------------------------------------------------

    d->downloadSelectedAction = new KAction(KIcon("computer"), i18n("Download Selected"), this);
    connect(d->downloadSelectedAction, SIGNAL(triggered()), this, SLOT(slotDownloadSelected()));
    actionCollection()->addAction("cameraui_imagedownloadselected", d->downloadSelectedAction);
    d->downloadSelectedAction->setEnabled(false);

    // -----------------------------------------------------------------

    d->downloadAllAction = new KAction(i18n("Download All"), this);
    connect(d->downloadAllAction, SIGNAL(triggered()), this, SLOT(slotDownloadAll()));
    actionCollection()->addAction("cameraui_imagedownloadall", d->downloadAllAction);

    // -----------------------------------------------------------------

    d->downloadDelSelectedAction = new KAction(i18n("Download/Delete Selected"), this);
    connect(d->downloadDelSelectedAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteSelected()));
    actionCollection()->addAction("cameraui_imagedownloaddeleteselected", d->downloadDelSelectedAction);
    d->downloadDelSelectedAction->setEnabled(false);

    // -------------------------------------------------------------------------

    d->downloadDelAllAction = new KAction(i18n("Download/Delete All"), this);
    connect(d->downloadDelAllAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteAll()));
    actionCollection()->addAction("cameraui_imagedownloaddeleteall", d->downloadDelAllAction);

    // -------------------------------------------------------------------------

    d->uploadAction = new KAction(KIcon("media-flash-smart-media"), i18n("Upload..."), this);
    connect(d->uploadAction, SIGNAL(triggered()), this, SLOT(slotUpload()));
    actionCollection()->addAction("cameraui_imageupload", d->uploadAction);

    // -------------------------------------------------------------------------

    d->lockAction = new KAction(KIcon("object-locked"), i18n("Toggle Lock"), this);
    connect(d->lockAction, SIGNAL(triggered()), this, SLOT(slotToggleLock()));
    actionCollection()->addAction("cameraui_imagelock", d->lockAction);

    // -------------------------------------------------------------------------

    d->markAsDownloadedAction = new KAction(KIcon("dialog-ok"), i18n("Mark as downloaded"), this);
    connect(d->markAsDownloadedAction, SIGNAL(triggered()), this, SLOT(slotMarkAsDownloaded()));
    actionCollection()->addAction("cameraui_imagemarkasdownloaded", d->markAsDownloadedAction);

    // -------------------------------------------------------------------------

    d->deleteSelectedAction = new KAction(KIcon("edit-delete"), i18n("Delete Selected"), this);
    connect(d->deleteSelectedAction, SIGNAL(triggered()), this, SLOT(slotDeleteSelected()));
    actionCollection()->addAction("cameraui_imagedeleteselected", d->deleteSelectedAction);
    d->deleteSelectedAction->setShortcut(KShortcut(Qt::Key_Delete));
    d->deleteSelectedAction->setEnabled(false);

    // -------------------------------------------------------------------------

    d->deleteAllAction = new KAction(i18n("Delete All"), this);
    connect(d->deleteAllAction, SIGNAL(triggered()), this, SLOT(slotDeleteAll()));
    actionCollection()->addAction("cameraui_imagedeleteall", d->deleteAllAction);

    // -- Last Photo First menu actions --------------------------------------------

    d->lastPhotoFirstAction = new KToggleAction(i18n("Show last photo first"), this);
    connect(d->lastPhotoFirstAction, SIGNAL(triggered()), this, SLOT(slotlastPhotoFirst()));
    actionCollection()->addAction("cameraui_lastphotofirst", d->lastPhotoFirstAction);

    // -- Standard 'View' menu actions ---------------------------------------------

    d->increaseThumbsAction = KStandardAction::zoomIn(this, SLOT(slotIncreaseThumbSize()), this);
    d->increaseThumbsAction->setEnabled(false);
    KShortcut keysPlus      = d->increaseThumbsAction->shortcut();
    keysPlus.setAlternate(Qt::Key_Plus);
    d->increaseThumbsAction->setShortcut(keysPlus);
    actionCollection()->addAction("cameraui_zoomplus", d->increaseThumbsAction);

    d->decreaseThumbsAction = KStandardAction::zoomOut(this, SLOT(slotDecreaseThumbSize()), this);
    d->decreaseThumbsAction->setEnabled(false);
    KShortcut keysMinus     = d->decreaseThumbsAction->shortcut();
    keysMinus.setAlternate(Qt::Key_Minus);
    d->decreaseThumbsAction->setShortcut(keysMinus);
    actionCollection()->addAction("cameraui_zoomminus", d->decreaseThumbsAction);

    d->fullScreenAction = actionCollection()->addAction(KStandardAction::FullScreen,
                                                        "cameraui_fullscreen", this, SLOT(slotToggleFullScreen()));

    d->showLogAction = new KToggleAction(KIcon("view-history"), i18n("Show History"), this);
    d->showLogAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_L));
    connect(d->showLogAction, SIGNAL(triggered()), this, SLOT(slotShowLog()));
    actionCollection()->addAction("cameraui_showlog", d->showLogAction);

    // -- Standard 'Configure' menu actions ----------------------------------------

    d->showMenuBarAction = KStandardAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());

    KStandardAction::keyBindings(this,            SLOT(slotEditKeys()),          actionCollection());
    KStandardAction::configureToolbars(this,      SLOT(slotConfToolbars()),      actionCollection());
    KStandardAction::configureNotifications(this, SLOT(slotConfNotifications()), actionCollection());
    KStandardAction::preferences(this,            SLOT(slotSetup()),             actionCollection());

    // ---------------------------------------------------------------------------------

    ThemeManager::instance()->registerThemeActions(this);

    // -- Standard 'Help' menu actions ---------------------------------------------

    d->about = new DAboutData(this);
    d->about->registerHelpActions();

    d->libsInfoAction = new KAction(KIcon("help-about"), i18n("Components Information"), this);
    connect(d->libsInfoAction, SIGNAL(triggered()), this, SLOT(slotComponentsInfo()));
    actionCollection()->addAction("cameraui_librariesinfo", d->libsInfoAction);

    d->dbStatAction = new KAction(KIcon("network-server-database"), i18n("Database Statistics"), this);
    connect(d->dbStatAction, SIGNAL(triggered()), this, SLOT(slotDBStat()));
    actionCollection()->addAction("cameraui_dbstat", d->dbStatAction);

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // -- Keyboard-only actions added to <MainWindow> ------------------------------

    KAction* altBackwardAction = new KAction(i18n("Previous Image"), this);
    actionCollection()->addAction("cameraui_backward_shift_space", altBackwardAction);
    altBackwardAction->setShortcut( KShortcut(Qt::SHIFT+Qt::Key_Space) );
    connect(altBackwardAction, SIGNAL(triggered()), this, SLOT(slotPrevItem()));

    // ---------------------------------------------------------------------------------

    d->anim = new DLogoAction(this);
    actionCollection()->addAction("logo_action", d->anim);

    createGUI(xmlFile());

    d->showMenuBarAction->setChecked(!menuBar()->isHidden());  // NOTE: workaround for B.K.O #171080
}

void CameraUI::setupConnections()
{
    connect(d->autoAlbumDateCheck, SIGNAL(toggled(bool)),
            d->folderDateFormat, SLOT(setEnabled(bool)));

    connect(d->autoAlbumDateCheck, SIGNAL(toggled(bool)),
            d->folderDateLabel, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->losslessFormat, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->formatLabel, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->view, SLOT(slotDownloadNameChanged()));

    connect(d->losslessFormat, SIGNAL(activated(int)),
            d->view, SLOT(slotDownloadNameChanged()));

    connect(d->fixDateTimeCheck, SIGNAL(toggled(bool)),
            d->dateTimeEdit, SLOT(setEnabled(bool)));

    connect(d->historyView, SIGNAL(signalEntryClicked(QVariant)),
            this, SLOT(slotHistoryEntryClicked(QVariant)));

    // -------------------------------------------------------------------------

    connect(d->view, SIGNAL(signalSelected(const CamItemInfo&,bool)),
            this, SLOT(slotItemsSelected(const CamItemInfo&,bool)));

    connect(d->view, SIGNAL(signalFileView(const CamItemInfo&)),
            this, SLOT(slotFileView(const CamItemInfo&)));

    connect(d->view, SIGNAL(signalUpload(KUrl::List)),
            this, SLOT(slotUploadItems(KUrl::List)));

    connect(d->view, SIGNAL(signalDownload()),
            this, SLOT(slotDownloadSelected()));

    connect(d->view, SIGNAL(signalDownloadAndDelete()),
            this, SLOT(slotDownloadAndDeleteSelected()));

    connect(d->view, SIGNAL(signalDelete()),
            this, SLOT(slotDeleteSelected()));

    connect(d->view, SIGNAL(signalToggleLock()),
            this, SLOT(slotToggleLock()));

    connect(d->view, SIGNAL(signalNewSelection(bool)),
            this, SLOT(slotNewSelection(bool)));

    connect(d->view, SIGNAL(signalZoomOut()),
            this, SLOT(slotDecreaseThumbSize()));

    connect(d->view, SIGNAL(signalZoomIn()),
            this, SLOT(slotIncreaseThumbSize()));

    connect(d->view, SIGNAL(signalThumbSizeChanged(int)),
            this, SLOT(slotThumbSizeChanged(int)));

    connect(d->view, SIGNAL(signalPrepareRepaint(const CamItemInfoList&)),
            this, SLOT(slotRequestThumbnails(const CamItemInfoList&)));

    // -------------------------------------------------------------------------

    connect(d->statusNavigateBar, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));

    connect(d->statusNavigateBar, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->statusNavigateBar, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->statusNavigateBar, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));

    // -------------------------------------------------------------------------

    connect(d->zoomBar, SIGNAL(signalZoomSliderChanged(int)),
            this, SLOT(slotZoomSliderChanged(int)));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->zoomBar, SLOT(slotUpdateTrackerPos()));

    // -------------------------------------------------------------------------

    connect(CollectionManager::instance(), SIGNAL(locationStatusChanged(CollectionLocation,int)),
            this, SLOT(slotCollectionLocationStatusChanged(CollectionLocation,int)));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSidebarTabTitleStyleChanged()));
}

void CameraUI::setupStatusBar()
{
    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statusBar()->addWidget(d->statusProgressBar, 100);

    //------------------------------------------------------------------------------

    d->cameraFreeSpace = new FreeSpaceWidget(statusBar(), 100);
    statusBar()->addWidget(d->cameraFreeSpace, 1);

    //------------------------------------------------------------------------------

    d->albumLibraryFreeSpace = new FreeSpaceWidget(statusBar(), 100);
    d->albumLibraryFreeSpace->setMode(FreeSpaceWidget::AlbumLibrary);
    statusBar()->addWidget(d->albumLibraryFreeSpace, 1);
    refreshCollectionFreeSpace();

    //------------------------------------------------------------------------------

    d->zoomBar = new DZoomBar(statusBar());
    d->zoomBar->setZoomPlusAction(d->increaseThumbsAction);
    d->zoomBar->setZoomMinusAction(d->decreaseThumbsAction);
    d->zoomBar->setBarMode(DZoomBar::NoPreviewZoomCtrl);
    statusBar()->addPermanentWidget(d->zoomBar, 1);

    //------------------------------------------------------------------------------

    d->statusNavigateBar = new StatusNavigateBar(statusBar());
    statusBar()->addPermanentWidget(d->statusNavigateBar, 1);
}

void CameraUI::setupCameraController(const QString& model, const QString& port, const QString& path)
{
    d->controller = new CameraController(this, d->cameraTitle, model, port, path);

    if (d->controller->cameraDriverType() == DKCamera::GPhotoDriver)
    {
        d->cameraFreeSpace->setMode(FreeSpaceWidget::GPhotoCamera);
        connect(d->controller, SIGNAL(signalFreeSpace(ulong,ulong)),
                this, SLOT(slotCameraFreeSpaceInfo(ulong,ulong)));
    }
    else
    {
        d->cameraFreeSpace->setMode(FreeSpaceWidget::UMSCamera);
        d->cameraFreeSpace->setPath(d->controller->cameraPath());
    }

    connect(d->controller, SIGNAL(signalConnected(bool)),
            this, SLOT(slotConnected(bool)));

    connect(d->controller, SIGNAL(signalLogMsg(QString,DHistoryView::EntryType,QString,QString)),
            this, SLOT(slotLogMsg(QString,DHistoryView::EntryType,QString,QString)));

    connect(d->controller, SIGNAL(signalCameraInformation(QString,QString,QString)),
            this, SLOT(slotCameraInformation(QString,QString,QString)));

    connect(d->controller, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->controller, SIGNAL(signalFolderList(QStringList)),
            this, SLOT(slotFolderList(QStringList)));

    connect(d->controller, SIGNAL(signalFileList(CamItemInfoList)),
            this, SLOT(slotFileList(CamItemInfoList)));

    connect(d->controller, SIGNAL(signalThumbInfo(QString,QString,CamItemInfo,QImage)),
            this, SLOT(slotThumbInfo(QString,QString,CamItemInfo,QImage)));

    connect(d->controller, SIGNAL(signalThumbInfoFailed(QString,QString,CamItemInfo)),
            this, SLOT(slotThumbInfoFailed(QString,QString,CamItemInfo)));

    connect(d->controller, SIGNAL(signalDownloaded(QString,QString,int)),
            this, SLOT(slotDownloaded(QString,QString,int)));

    connect(d->controller, SIGNAL(signalDownloadComplete(QString,QString,QString,QString)),
            this, SLOT(slotDownloadComplete(QString,QString,QString,QString)));

    connect(d->controller, SIGNAL(signalSkipped(QString,QString)),
            this, SLOT(slotSkipped(QString,QString)));

    connect(d->controller, SIGNAL(signalDeleted(QString,QString,bool)),
            this, SLOT(slotDeleted(QString,QString,bool)));

    connect(d->controller, SIGNAL(signalLocked(QString,QString,bool)),
            this, SLOT(slotLocked(QString,QString,bool)));

    connect(d->controller, SIGNAL(signalMetadata(QString,QString,DMetadata)),
            this, SLOT(slotMetadata(QString,QString,DMetadata)));

    connect(d->controller, SIGNAL(signalUploaded(CamItemInfo)),
            this, SLOT(slotUploaded(CamItemInfo)));
}

void CameraUI::setupAccelerators()
{
}

void CameraUI::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Camera Settings");

    d->autoRotateCheck->setChecked(group.readEntry("AutoRotate",             true));
    d->autoAlbumDateCheck->setChecked(group.readEntry("AutoAlbumDate",       false));
    d->autoAlbumExtCheck->setChecked(group.readEntry("AutoAlbumExt",         false));
    d->fixDateTimeCheck->setChecked(group.readEntry("FixDateTime",           false));
    d->templateSelector->setTemplateIndex(group.readEntry("Template",        0));
    d->convertJpegCheck->setChecked(group.readEntry("ConvertJpeg",           false));
    d->losslessFormat->setCurrentIndex(group.readEntry("LossLessFormat",     0));   // PNG by default
    d->folderDateFormat->setCurrentIndex(group.readEntry("FolderDateFormat", (int)CameraUIPriv::IsoDateFormat));
    d->view->setThumbnailSize(group.readEntry("ThumbnailSize",               (int)ThumbnailSize::Large));
    d->showLogAction->setChecked(group.readEntry("ShowLog",                  false));
    d->lastPhotoFirstAction->setChecked(group.readEntry("LastPhotoFirst",    true));

#if KDCRAW_VERSION >= 0x020000
    d->advBox->readSettings(group);
#else
    d->advBox->readSettings();
#endif

    d->splitter->restoreState(group);

    d->dateTimeEdit->setEnabled(d->fixDateTimeCheck->isChecked());
    d->losslessFormat->setEnabled(convertLosslessJpegFiles());
    d->formatLabel->setEnabled(convertLosslessJpegFiles());
    d->folderDateFormat->setEnabled(d->autoAlbumDateCheck->isChecked());
    d->folderDateLabel->setEnabled(d->autoAlbumDateCheck->isChecked());
    slotShowLog();
}

void CameraUI::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Camera Settings");

    group.writeEntry("AutoRotate",          d->autoRotateCheck->isChecked());
    group.writeEntry("AutoAlbumDate",       d->autoAlbumDateCheck->isChecked());
    group.writeEntry("AutoAlbumExt",        d->autoAlbumExtCheck->isChecked());
    group.writeEntry("FixDateTime",         d->fixDateTimeCheck->isChecked());
    group.writeEntry("Template",            d->templateSelector->getTemplateIndex());
    group.writeEntry("ConvertJpeg",         convertLosslessJpegFiles());
    group.writeEntry("LossLessFormat",      d->losslessFormat->currentIndex());
    group.writeEntry("ThumbnailSize",       d->view->thumbnailSize());
    group.writeEntry("FolderDateFormat",    d->folderDateFormat->currentIndex());
    group.writeEntry("ShowLog",             d->showLogAction->isChecked());
    group.writeEntry("LastPhotoFirst",      d->lastPhotoFirstAction->isChecked());

#if KDCRAW_VERSION >= 0x020000
    d->advBox->writeSettings(group);
#else
    d->advBox->writeSettings();
#endif

    d->rightSideBar->saveState();
    d->splitter->saveState(group);
    config->sync();
}

void CameraUI::slotProcessUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

bool CameraUI::isBusy() const
{
    return d->busy;
}

bool CameraUI::isClosed() const
{
    return d->closed;
}

bool CameraUI::autoRotateJpegFiles() const
{
    return d->autoRotateCheck->isChecked();
}

bool CameraUI::convertLosslessJpegFiles() const
{
    return d->convertJpegCheck->isChecked();
}

QString CameraUI::losslessFormat() const
{
    return d->losslessFormat->currentText();
}

QString CameraUI::cameraTitle() const
{
    return d->cameraTitle;
}

void CameraUI::slotCancelButton()
{
    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                                          i18n("Canceling current operation, please wait..."));
    d->controller->slotCancel();
    d->historyUpdater->slotCancel();
    d->currentlyDeleting.clear();
    refreshFreeSpace();
}

void CameraUI::refreshFreeSpace()
{
    if (d->controller->cameraDriverType() == DKCamera::GPhotoDriver)
    {
        d->controller->getFreeSpace();
    }
    else
    {
        d->cameraFreeSpace->refresh();
    }
}

void CameraUI::closeEvent(QCloseEvent* e)
{
    if (dialogClosed())
    {
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void CameraUI::moveEvent(QMoveEvent* e)
{
    Q_UNUSED(e)
    emit signalWindowHasMoved();
}

void CameraUI::slotClose()
{
    /*FIXME
        if (dialogClosed())
            reject();
    */
}

bool CameraUI::dialogClosed()
{
    if (d->closed)
    {
        return true;
    }

    if (isBusy())
    {
        if (KMessageBox::questionYesNo(this,
                                       i18n("Do you want to close the dialog "
                                            "and cancel the current operation?"))
            == KMessageBox::No)
        {
            return false;
        }
    }

    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                                          i18n("Disconnecting from camera, please wait..."));

    if (isBusy())
    {
        d->controller->slotCancel();
        // will be read in slotBusy later and finishDialog
        // will be called only when everything is finished
        d->closed = true;
    }
    else
    {
        d->closed = true;
        finishDialog();
    }

    return true;
}

void CameraUI::finishDialog()
{
    // Look if an item have been downloaded to computer during camera GUI session.
    // If yes, update the starting number value used to rename camera items from camera list.

    if (d->view->itemsDownloaded() > 0)
    {
        CameraList* clist = CameraList::defaultList();

        if (clist)
        {
            clist->changeCameraStartIndex(d->cameraTitle, d->renameCustomizer->startIndex());
        }
    }

    // When a directory is created, a watch is put on it to spot new files
    // but it can occur that the file is copied there before the watch is
    // completely setup. That is why as an extra safeguard run CollectionScanner
    // over the folders we used. Bug: 119201

    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                                          i18n("Scanning for new files, please wait..."));
    foreach (const QString& folder, d->foldersToScan)
    {
        ScanController::instance()->scheduleCollectionScan(folder);
    }
    d->foldersToScan.clear();

    deleteLater();

    if (!d->lastDestURL.isEmpty())
    {
        emit signalLastDestination(d->lastDestURL);
    }

    saveSettings();
}

void CameraUI::slotBusy(bool val)
{
    if (!val)   // Camera is available for actions.
    {
        if (!d->busy)
        {
            return;
        }

        d->busy = false;
        d->cameraCancelAction->setEnabled(false);
        d->view->viewport()->setEnabled(true);

        d->advBox->setEnabled(true);
        // B.K.O #127614: The Focus need to be restored in custom prefix widget.
        // commenting this out again: If we do not disable, no need to restore focus
        // d->renameCustomizer->restoreFocus();

        d->uploadAction->setEnabled(d->controller->cameraUploadSupport());
        d->downloadAllAction->setEnabled(true);
        d->downloadDelAllAction->setEnabled(d->controller->cameraDeleteSupport());
        d->deleteAllAction->setEnabled(d->controller->cameraDeleteSupport());
        d->selectAllAction->setEnabled(true);
        d->selectNoneAction->setEnabled(true);
        d->selectInvertAction->setEnabled(true);
        d->selectNewItemsAction->setEnabled(true);
        d->selectLockedItemsAction->setEnabled(true);
        d->cameraInfoAction->setEnabled(true);
        d->cameraCaptureAction->setEnabled(d->controller->cameraCaptureImageSupport());

        // selection-dependent update of lockAction, markAsDownloadedAction,
        // downloadSelectedAction, downloadDelSelectedAction, deleteSelectedAction
        slotNewSelection(d->view->countSelected()>0);

        d->anim->stop();
        d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, i18n("Ready"));

        // like WDestructiveClose, but after camera controller operation has safely finished
        if (d->closed)
        {
            finishDialog();
        }
    }
    else    // Camera is busy.
    {
        if (d->busy)
        {
            return;
        }

        if (!d->anim->running())
        {
            d->anim->start();
        }

        d->busy = true;

        // Has camera icon view item selection is used to control download post processing,
        // all selection actions are disable when camera interface is busy.
        d->view->viewport()->setEnabled(false);

        d->cameraCancelAction->setEnabled(true);
        d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);

        // Settings tab is disabled in slotDownload, selectively when downloading
        // Fast dis/enabling would create the impression of flicker, e.g. when retrieving EXIF from camera
        //d->advBox->setEnabled(false);

        d->uploadAction->setEnabled(false);
        d->downloadSelectedAction->setEnabled(false);
        d->downloadDelSelectedAction->setEnabled(false);
        d->downloadAllAction->setEnabled(false);
        d->downloadDelAllAction->setEnabled(false);
        d->deleteSelectedAction->setEnabled(false);
        d->deleteAllAction->setEnabled(false);
        d->selectAllAction->setEnabled(false);
        d->selectNoneAction->setEnabled(false);
        d->selectInvertAction->setEnabled(false);
        d->selectNewItemsAction->setEnabled(false);
        d->selectLockedItemsAction->setEnabled(false);
        d->lockAction->setEnabled(false);
        d->markAsDownloadedAction->setEnabled(false);
        d->cameraInfoAction->setEnabled(false);
        d->cameraCaptureAction->setEnabled(false);
        d->imageViewAction->setEnabled(false);
    }
}

void CameraUI::slotIncreaseThumbSize()
{
    int thumbSize = d->view->thumbnailSize() + ThumbnailSize::Step;
    d->view->setThumbnailSize(thumbSize);
}

void CameraUI::slotDecreaseThumbSize()
{
    int thumbSize = d->view->thumbnailSize() - ThumbnailSize::Step;
    d->view->setThumbnailSize(thumbSize);
}

void CameraUI::slotZoomSliderChanged(int size)
{
    d->view->setThumbnailSize(size);
}

void CameraUI::slotThumbSizeChanged(int size)
{
    d->zoomBar->setThumbsSize(size);

    d->increaseThumbsAction->setEnabled(true);
    d->decreaseThumbsAction->setEnabled(true);

    if (d->view->thumbnailSize() == ThumbnailSize::Small)
    {
        d->decreaseThumbsAction->setEnabled(false);
    }

    if (d->view->thumbnailSize() == ThumbnailSize::Huge)
    {
        d->increaseThumbsAction->setEnabled(false);
    }
}

void CameraUI::slotConnected(bool val)
{
    if (!val)
    {
        if (KMessageBox::warningYesNo(this,
                                      i18n("Failed to connect to the camera. "
                                           "Please make sure it is connected "
                                           "properly and turned on. "
                                           "Would you like to try again?"),
                                      i18n("Connection Failed"),
                                      KGuiItem(i18n("Retry")),
                                      KGuiItem(i18n("Abort")))
            == KMessageBox::Yes)
        {
            QTimer::singleShot(0, d->controller, SLOT(slotConnect()));
        }
        else
        {
            close();
        }
    }
    else
    {
        refreshFreeSpace();
        d->controller->listFolders();
    }
}

void CameraUI::slotFolderList(const QStringList& folderList)
{
    if (d->closed)
    {
        return;
    }

    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->setProgressTotalSteps(0);

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    bool useMetadata          = group.readEntry(d->configUseMetadataDateEntry, false);

    for (QStringList::const_iterator it = folderList.constBegin();
         it != folderList.constEnd(); ++it)
    {
        d->controller->listFiles(*it, useMetadata);
    }
}

void CameraUI::slotFileList(const CamItemInfoList& fileList)
{
    if (d->closed)
    {
        return;
    }

    if (fileList.empty())
    {
        return;
    }

    d->filesToBeAdded << fileList;
    d->refreshIconViewTimer->start();
}

void CameraUI::slotRefreshIconViewTimer()
{
    if (d->busy)
    {
        d->refreshIconViewTimer->start();
        return;
    }

    kDebug() << "filesToBeAdded count : " << d->filesToBeAdded.count();

    if (d->filesToBeAdded.isEmpty())
    {
        return;
    }

    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings)
    {
        return;
    }

    // We sort the map by time stamp
    // and we remove internal camera files which are not image/video/sounds.
    QStringList fileNames, fileExts;
    QFileInfo   info;

    // JVC camera (see B.K.O #133185).
    fileNames.append("mgr_data");
    fileNames.append("pgr_mgr");

    // HP Photosmart camera (see B.K.O #156338).
    fileExts.append("dsp");

    // Minolta camera in PTP mode
    fileExts.append("dps");

    // NOTE: see B.K.O #181726: list of accepted file extensions from Album Settings.
    QStringList list = settings->getAllFileFilter().toLower().split(' ');

    QMultiMap<QDateTime, CamItemInfo> map;
    CameraIconItem* citem = static_cast<CameraIconItem*>(d->view->firstItem());

    while (citem)
    {
        info.setFile(citem->itemInfo().name);
        map.insertMulti(citem->itemInfo().mtime, citem->itemInfo());
        citem = static_cast<CameraIconItem*>(citem->nextItem());
    }

    CamItemInfoList::iterator it = d->filesToBeAdded.begin();

    while (it != d->filesToBeAdded.end())
    {
        info.setFile((*it).name);

        if (!fileNames.contains(info.fileName().toLower()) &&
            !fileExts.contains(info.suffix().toLower())    &&
            list.contains(QString("*.%1").arg(info.suffix().toLower())))
        {
            map.insertMulti((*it).mtime, *it);
        }

        it = d->filesToBeAdded.erase(it);
    }

    citem = static_cast<CameraIconItem*>(d->view->firstItem());

    while (citem)
    {
        CameraIconItem* tempItem = citem;
        citem                    = static_cast<CameraIconItem*>(tempItem->nextItem());
        d->view->removeItem(tempItem->itemInfo());
    }

    d->historyUpdater->addItems(d->controller->cameraMD5ID(), map);
}

void CameraUI::slotRefreshIconView(const CHUpdateItemMap& map)
{
    if (map.empty())
    {
        return;
    }

    CHUpdateItemMap _map = map;

    QMultiMap<QDateTime, CamItemInfo>::iterator it;
    bool lastPhotoFirst = d->lastPhotoFirstAction->isChecked();
    CamItemInfo item;

    it = lastPhotoFirst ? _map.end() : _map.begin();

    do
    {
        if (lastPhotoFirst)
        {
            --it;
        }

        item = *it;
        d->view->addItem(item);

        if (!lastPhotoFirst)
        {
            ++it;
        }
    }
    while ((lastPhotoFirst ? it != _map.begin() : it != _map.end()));
}

void CameraUI::slotlastPhotoFirst()
{
    saveSettings();

    QMultiMap<QDateTime, CamItemInfo> map;
    CameraIconItem* item = dynamic_cast<CameraIconItem*>(d->view->firstItem());
    QFileInfo info;

    while (item)
    {
        info.setFile(item->itemInfo().name);
        map.insertMulti(item->itemInfo().mtime, item->itemInfo());
        item = dynamic_cast<CameraIconItem*>(item->nextItem());
    }

    item = dynamic_cast<CameraIconItem*>(d->view->firstItem());

    while (item)
    {
        d->view->removeItem(item->itemInfo());
        item = dynamic_cast<CameraIconItem*>(item->nextItem());
    }

    slotRefreshIconView(map);
}

void CameraUI::slotRequestThumbnails(const CamItemInfoList& list)
{
    if (list.isEmpty()) return;
    d->controller->getThumbsInfo(list);
}

void CameraUI::slotThumbInfo(const QString& folder, const QString& file, const CamItemInfo& info, const QImage& thumbnail)
{
    d->view->setItemInfo(folder, file, info);

    if (thumbnail.isNull())
    {
        // This call must be run outside Camera Controller thread.
        QImage thumb = d->controller->mimeTypeThumbnail(file).toImage();
        d->view->setThumbnail(folder, file, thumb);
    }
    else
    {
        d->view->setThumbnail(folder, file, thumbnail);
    }
}

void CameraUI::slotThumbInfoFailed(const QString& folder, const QString& file, const CamItemInfo& info)
{
    d->view->setItemInfo(folder, file, info);

    if (d->controller->cameraDriverType() == DKCamera::UMSDriver)
    {
        d->kdeTodo << info.url();
        startKdePreviewJob();
    }
    else
    {
        // This call must be run outside Camera Controller thread.
        QImage thumb = d->controller->mimeTypeThumbnail(file).toImage();
        d->view->setThumbnail(folder, file, thumb);
    }
}

void CameraUI::startKdePreviewJob()
{
    if (d->kdeJob || d->kdeTodo.isEmpty())
    {
        return;
    }

    KUrl::List list = d->kdeTodo;
    d->kdeTodo.clear();
    d->kdeJob = KIO::filePreview(list, 256);

    connect(d->kdeJob, SIGNAL(gotPreview(KFileItem,QPixmap)),
            this, SLOT(slotGotKDEPreview(KFileItem,QPixmap)));

    connect(d->kdeJob, SIGNAL(failed(KFileItem)),
            this, SLOT(slotFailedKDEPreview(KFileItem)));

    connect(d->kdeJob, SIGNAL(finished(KJob*)),
            this, SLOT(slotKdePreviewFinished(KJob*)));
}

void CameraUI::slotGotKDEPreview(const KFileItem& item, const QPixmap& pix)
{
    QString file   = item.url().fileName();
    QString folder = item.url().toLocalFile().remove(QString("/") + file);
    QImage thumb   = pix.toImage();

    if (thumb.isNull())
    {
        // This call must be run outside Camera Controller thread.
        thumb = d->controller->mimeTypeThumbnail(file).toImage();
    }

    d->view->setThumbnail(folder, file, thumb);
}

void CameraUI::slotFailedKDEPreview(const KFileItem& item)
{
    QString file   = item.url().fileName();
    QString folder = item.url().toLocalFile().remove(QString("/") + file);
    QImage thumb   = d->controller->mimeTypeThumbnail(file).toImage();
    d->view->setThumbnail(folder, file, thumb);
}

void CameraUI::slotKdePreviewFinished(KJob*)
{
    d->kdeJob = 0;
}

void CameraUI::slotCapture()
{
    if (d->busy)
    {
        return;
    }

    CaptureDlg* captureDlg = new CaptureDlg(this, d->controller, d->cameraTitle);
    captureDlg->show();
}

void CameraUI::slotInformation()
{
    if (d->busy)
    {
        return;
    }

    d->controller->getCameraInformation();
}

void CameraUI::slotCameraInformation(const QString& summary, const QString& manual, const QString& about)
{
    CameraInfoDialog* infoDlg = new CameraInfoDialog(this, summary, manual, about);
    infoDlg->show();
}

void CameraUI::slotUpload()
{
    if (d->busy)
    {
        return;
    }

    QString fileformats;

    QStringList patternList = KImageIO::pattern(KImageIO::Reading).split('\n');

    // All Images from list must been always the first entry given by KDE API
    QString allPictures = patternList[0];

    // Add RAW file format to All Images" type mime and replace current.
    allPictures.insert(allPictures.indexOf("|"), QString(KDcrawIface::KDcraw::rawFiles()));
    patternList.removeAll(patternList[0]);
    patternList.prepend(allPictures);

    // Added RAW file formats supported by dcraw program like a type mime.
    // Note: we cannot use here "image/x-raw" type mime from KDE because it incomplete
    // or unavailable(dcraw_0)(see file #121242 in B.K.O).
    patternList.append(QString("\n%1|Camera RAW files").arg(QString(KDcrawIface::KDcraw::rawFiles())));
    fileformats = patternList.join("\n");

    kDebug() << "fileformats=" << fileformats;

    KUrl::List urls = KFileDialog::getOpenUrls(CollectionManager::instance()->oneAlbumRootPath(),
                                               fileformats, this, i18n("Select Image to Upload"));

    if (!urls.isEmpty())
    {
        slotUploadItems(urls);
    }
}

void CameraUI::slotUploadItems(const KUrl::List& urls)
{
    if (d->busy)
    {
        return;
    }

    if (urls.isEmpty())
    {
        return;
    }

    if (d->cameraFreeSpace->isValid())
    {
        // Check if space require to upload new items in camera is enough.
        quint64 totalKbSize = 0;

        for (KUrl::List::const_iterator it = urls.constBegin() ; it != urls.constEnd() ; ++it)
        {
            QFileInfo fi((*it).toLocalFile());
            totalKbSize += fi.size()/1024;
        }

        if (totalKbSize >= d->cameraFreeSpace->kBAvail())
        {
            KMessageBox::error(this, i18n("There is not enough free space on the Camera Medium "
                                          "to upload pictures.\n\n"
                                          "Space require: %1\n"
                                          "Available free space: %2",
                                          KIO::convertSizeFromKiB(totalKbSize),
                                          KIO::convertSizeFromKiB(d->cameraFreeSpace->kBAvail())));
            return;
        }
    }

    QMap<QString, int> map           = d->view->countItemsByFolders();
    QPointer<CameraFolderDialog> dlg = new CameraFolderDialog(this, map, d->controller->cameraTitle(),
                                                              d->controller->cameraPath());

    if (dlg->exec() != QDialog::Accepted)
    {
        delete dlg;
        return;
    }

    // since we access members here, check if the pointer is still valid
    if (!dlg)
    {
        return;
    }

    QString cameraFolder = dlg->selectedFolderPath();

    for (KUrl::List::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        QFileInfo fi((*it).toLocalFile());

        if (!fi.exists())
        {
            continue;
        }

        if (fi.isDir())
        {
            continue;
        }

        QString ext  = QString(".") + fi.completeSuffix();
        QString name = fi.fileName();
        name.truncate(fi.fileName().length() - ext.length());

        bool ok;

        while (d->view->findItem(cameraFolder, name + ext))
        {
            QString msg(i18n("Camera Folder <b>%1</b> already contains the item <b>%2</b>.<br/>"
                             "Please enter a new filename (without extension):",
                             cameraFolder, fi.fileName()));
            name = KInputDialog::getText(i18n("File already exists"), msg, name, &ok, this);

            if (!ok)
            {
                return;
            }
        }

        d->controller->upload(fi, name + ext, cameraFolder);
    }

    delete dlg;
}

void CameraUI::slotUploaded(const CamItemInfo& itemInfo)
{
    if (d->closed)
    {
        return;
    }

    d->view->addItem(itemInfo);
    d->controller->getThumbsInfo(CamItemInfoList() << itemInfo);
    refreshFreeSpace();
}

void CameraUI::slotDownloadSelected()
{
    slotDownload(true, false);
}

void CameraUI::slotDownloadAndDeleteSelected()
{
    slotDownload(true, true);
}

void CameraUI::slotDownloadAll()
{
    slotDownload(false, false);
}

void CameraUI::slotDownloadAndDeleteAll()
{
    slotDownload(false, true);
}

void CameraUI::slotDownload(bool onlySelected, bool deleteAfter, Album* album)
{
    // See B.K.O #143934: force to select all items to prevent problem
    // when !renameCustomizer->useDefault() ==> iconItem->getDownloadName()
    // can return an empty string in this case because it depends on selection.
    if (!onlySelected)
    {
        d->view->slotSelectAll();
    }

    QString   newDirName;
    IconItem* firstItem = d->view->firstItem();

    if (firstItem)
    {
        CameraIconItem* iconItem = static_cast<CameraIconItem*>(firstItem);

        QDateTime dateTime = iconItem->itemInfo().mtime;

        switch (d->folderDateFormat->currentIndex())
        {
            case CameraUIPriv::TextDateFormat:
                newDirName = dateTime.date().toString(Qt::TextDate);
                break;
            case CameraUIPriv::LocalDateFormat:
                newDirName = dateTime.date().toString(Qt::LocalDate);
                break;
            default:        // IsoDateFormat
                newDirName = dateTime.date().toString(Qt::ISODate);
                break;
        }
    }

    // -- Get the destination album from digiKam library ---------------

    if (!album)
    {
        AlbumManager* man = AlbumManager::instance();

        album = man->currentAlbum();

        if (album && album->type() != Album::PHYSICAL)
        {
            album = 0;
        }

        QString header(i18n("<p>Please select the destination album from the digiKam library to "
                            "import the camera pictures into.</p>"));

        album = AlbumSelectDialog::selectAlbum(this, (PAlbum*)album, header, newDirName);

        if (!album)
        {
            return;
        }
    }

    PAlbum* pAlbum = dynamic_cast<PAlbum*>(album);

    if (!pAlbum)
    {
        return;
    }

    // -- Check disk space ------------------------

    // See B.K.O #139519: Always check free space available before to
    // download items selection from camera.
    unsigned long fSize = 0;
    unsigned long dSize = 0;
    d->view->itemsSelectionSizeInfo(fSize, dSize);
    QString albumRootPath = pAlbum->albumRootPath();
    unsigned long kBAvail = d->albumLibraryFreeSpace->kBAvail(albumRootPath);

    if (dSize >= kBAvail)
    {
        KGuiItem cont = KStandardGuiItem::cont();
        cont.setText(i18n("Try Anyway"));
        KGuiItem cancel = KStandardGuiItem::cancel();
        cancel.setText(i18n("Cancel Download"));
        int result =
            KMessageBox::warningYesNo(this,
                                      i18n("There is not enough free space on the disk of the album you selected "
                                           "to download and process the selected pictures from the camera.\n\n"
                                           "Estimated space required: %1\n"
                                           "Available free space: %2",
                                           KIO::convertSizeFromKiB(dSize),
                                           KIO::convertSizeFromKiB(kBAvail)),
                                      i18n("Insufficient Disk Space"),
                                      cont, cancel);

        if (result == KMessageBox::No)
        {
            return;
        }
    }

    // -- Prepare downloading of camera items ------------------------

    KUrl url = pAlbum->fileUrl();

    d->controller->downloadPrep();

    DownloadSettingsContainer downloadSettings;
    QString   downloadName;
    QDateTime dateTime;
    int       total = 0;

    downloadSettings.autoRotate     = d->autoRotateCheck->isChecked();
    downloadSettings.fixDateTime    = d->fixDateTimeCheck->isChecked();
    downloadSettings.newDateTime    = d->dateTimeEdit->dateTime();
    downloadSettings.templateTitle  = d->templateSelector->getTemplate().templateTitle();
    downloadSettings.convertJpeg    = convertLosslessJpegFiles();
    downloadSettings.losslessFormat = losslessFormat();

    // -- Download camera items -------------------------------
    // Since we show camera items in reverse order, downloading need to be done also in reverse order.

    QSet<QString> usedDownloadPaths;
    bool lastPhotoFirst = d->lastPhotoFirstAction->isChecked();

    for (IconItem* item = (lastPhotoFirst ? d->view->lastItem() : d->view->firstItem()); item;
         item = (lastPhotoFirst ? item->prevItem() : item->nextItem()))
    {
        if (onlySelected && !(item->isSelected()))
        {
            continue;
        }

        CameraIconItem* iconItem = static_cast<CameraIconItem*>(item);
        downloadSettings.folder  = iconItem->itemInfo().folder;
        downloadSettings.file    = iconItem->itemInfo().name;
        downloadName             = iconItem->itemInfo().downloadName;
        dateTime                 = iconItem->itemInfo().mtime;

        KUrl downloadUrl(url);
        QString errMsg;

        // Auto sub-albums creation based on file date.

        if (d->autoAlbumDateCheck->isChecked())
        {
            QString dirName;

            switch (d->folderDateFormat->currentIndex())
            {
                case CameraUIPriv::TextDateFormat:
                    dirName = dateTime.date().toString(Qt::TextDate);
                    break;
                case CameraUIPriv::LocalDateFormat:
                    dirName = dateTime.date().toString(Qt::LocalDate);
                    break;
                default:        // IsoDateFormat
                    dirName = dateTime.date().toString(Qt::ISODate);
                    break;
            }

            // See B.K.O #136927 : we need to support file system which do not
            // handle upper case properly.
            dirName = dirName.toLower();

            if (!createAutoAlbum(downloadUrl, dirName, dateTime.date(), errMsg))
            {
                KMessageBox::error(this, errMsg);
                return;
            }

            downloadUrl.addPath(dirName);
        }

        // Auto sub-albums creation based on file extensions.

        if (d->autoAlbumExtCheck->isChecked())
        {
            // We use the target file name to compute sub-albums name to take a care about
            // conversion on the fly option.
            QFileInfo fi(downloadName);

            QString subAlbum = fi.suffix().toUpper();

            if (fi.suffix().toUpper() == QString("JPEG") ||
                fi.suffix().toUpper() == QString("JPE"))
            {
                subAlbum = QString("JPG");
            }

            if (fi.suffix().toUpper() == QString("TIFF"))
            {
                subAlbum = QString("TIF");
            }

            if (fi.suffix().toUpper() == QString("MPEG") ||
                fi.suffix().toUpper() == QString("MPE") ||
                fi.suffix().toUpper() == QString("MPO"))
            {
                subAlbum = QString("MPG");
            }

            // See B.K.O #136927 : we need to support file system which do not
            // handle upper case properly.
            subAlbum = subAlbum.toLower();

            if (!createAutoAlbum(downloadUrl, subAlbum, dateTime.date(), errMsg))
            {
                KMessageBox::error(this, errMsg);
                return;
            }

            downloadUrl.addPath(subAlbum);
        }

        d->foldersToScan << downloadUrl.toLocalFile();

        if (downloadName.isEmpty())
        {
            downloadUrl.addPath(downloadSettings.file);
        }
        else
        {
            // when using custom renaming (e.g. by date, see bug 179902)
            // make sure that we create unique names
            downloadUrl.addPath(downloadName);
            QString suggestedPath = downloadUrl.toLocalFile();

            if (usedDownloadPaths.contains(suggestedPath))
            {
                QFileInfo fi(downloadName);
                QString suffix = '.' + fi.suffix();
                QString pathWithoutSuffix(suggestedPath);
                pathWithoutSuffix.chop(suffix.length());
                QString currentVariant;
                int counter = 1;

                do
                {
                    currentVariant = pathWithoutSuffix + '-' + QString::number(counter++) + suffix;
                }
                while (usedDownloadPaths.contains(currentVariant));

                usedDownloadPaths << currentVariant;
                downloadUrl = KUrl(currentVariant);
            }
            else
            {
                usedDownloadPaths << suggestedPath;
            }
        }

        downloadSettings.dest = downloadUrl.toLocalFile();

        d->controller->download(downloadSettings);
        ++total;
    }

    if (total <= 0)
    {
        return;
    }

    d->lastDestURL = url;
    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->setProgressTotalSteps(total);
    d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);

    // disable settings tab here instead of slotBusy:
    // Only needs to be disabled while downloading
    d->advBox->setEnabled(false);

    d->deleteAfter = deleteAfter;
}

void CameraUI::slotDownloaded(const QString& folder, const QString& file, int status)
{
    CameraIconItem* iconItem = d->view->findItem(folder, file);

    if (iconItem)
    {
        iconItem->setDownloaded(status);

        //if (iconItem->isSelected())
        //  slotItemsSelected(iconItem->itemInfo(), true);

        if (status == CamItemInfo::DownloadedYes)
        {
            int curr = d->statusProgressBar->progressValue();
            d->statusProgressBar->setProgressValue(curr+1);
            d->renameCustomizer->setStartIndex(d->renameCustomizer->startIndex() + 1);

            DownloadHistory::setDownloaded(d->controller->cameraMD5ID(),
                                           iconItem->itemInfo().name,
                                           iconItem->itemInfo().size,
                                           iconItem->itemInfo().mtime);
        }
    }

    // Download all items is complete ?
    if (d->statusProgressBar->progressValue() == d->statusProgressBar->progressTotalSteps())
    {
        if (d->deleteAfter)
        {
            // No need passive pop-up here, because wil ask to confirm items deletion with dialog.
            deleteItems(true, true);
        }
        else
        {
            // Pop-up a message to bring user when all is done.
            KNotificationWrapper("cameradownloaded", i18n("Download is completed..."), this, windowTitle());
        }
    }
}

void CameraUI::slotDownloadComplete(const QString&, const QString&,
                                    const QString& destFolder, const QString&)
{
    ScanController::instance()->scheduleCollectionScanRelaxed(destFolder);
}

void CameraUI::slotSkipped(const QString& folder, const QString& file)
{
    CameraIconItem* iconItem = d->view->findItem(folder, file);

    if (iconItem)
    {
        iconItem->setDownloaded(CamItemInfo::DownloadedNo);
    }

    int curr = d->statusProgressBar->progressValue();
    d->statusProgressBar->setProgressValue(curr+1);
}

void CameraUI::slotMarkAsDownloaded()
{
    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconItem* iconItem = static_cast<CameraIconItem*>(item);

        if (iconItem->isSelected())
        {
            iconItem->setDownloaded(CamItemInfo::DownloadedYes);

            DownloadHistory::setDownloaded(d->controller->cameraMD5ID(),
                                           iconItem->itemInfo().name,
                                           iconItem->itemInfo().size,
                                           iconItem->itemInfo().mtime);
        }
    }
}


void CameraUI::slotToggleLock()
{
    int count = 0;

    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconItem* iconItem = static_cast<CameraIconItem*>(item);

        if (iconItem->isSelected())
        {
            QString folder = iconItem->itemInfo().folder;
            QString file   = iconItem->itemInfo().name;
            int writePerm  = iconItem->itemInfo().writePermissions;
            bool lock      = true;

            // If item is currently locked, unlock it.
            if (writePerm == 0)
            {
                lock = false;
            }

            d->controller->lockFile(folder, file, lock);
            ++count;
        }
    }

    if (count > 0)
    {
        d->statusProgressBar->setProgressValue(0);
        d->statusProgressBar->setProgressTotalSteps(count);
        d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);
    }
}

void CameraUI::slotLocked(const QString& folder, const QString& file, bool status)
{
    if (status)
    {
        CameraIconItem* iconItem = d->view->findItem(folder, file);

        if (iconItem)
        {
            iconItem->toggleLock();
            //if (iconItem->isSelected())
            //  slotItemsSelected(iconItem->itemInfo(), true);
        }
    }

    int curr = d->statusProgressBar->progressValue();
    d->statusProgressBar->setProgressValue(curr+1);
}


void CameraUI::checkItem4Deletion(const CamItemInfo& info, QStringList& folders, QStringList& files,
                                  QStringList& deleteList, QStringList& lockedList)
{
    if (info.writePermissions != 0)  // Item not locked ?
    {
        QString folder = info.folder;
        QString file   = info.name;
        folders.append(folder);
        files.append(file);
        deleteList.append(folder + QString("/") + file);
    }
    else
    {
        lockedList.append(info.name);
    }
}

void CameraUI::deleteItems(bool onlySelected, bool onlyDownloaded)
{
    QStringList folders;
    QStringList files;
    QStringList deleteList;
    QStringList lockedList;

    for (IconItem* item = d->view->firstItem(); item; item = item->nextItem())
    {
        CameraIconItem* iconItem = dynamic_cast<CameraIconItem*>(item);

        if (iconItem)
        {
            CamItemInfo info = iconItem->itemInfo();

            if (onlySelected)
            {
                if (iconItem->isSelected())
                {
                    if (onlyDownloaded)
                    {
                        if (iconItem->isDownloaded())
                        {
                            checkItem4Deletion(info, folders, files, deleteList, lockedList);
                        }
                    }
                    else
                    {
                        checkItem4Deletion(info, folders, files, deleteList, lockedList);
                    }
                }
            }
            else    // All items
            {
                if (onlyDownloaded)
                {
                    if (iconItem->isDownloaded())
                    {
                        checkItem4Deletion(info, folders, files, deleteList, lockedList);
                    }
                }
                else
                {
                    checkItem4Deletion(info, folders, files, deleteList, lockedList);
                }
            }
        }
    }

    // If we want to delete some locked files, just give a feedback to user.
    if (!lockedList.isEmpty())
    {
        QString infoMsg(i18n("The items listed below are locked by camera (read-only). "
                             "These items will not be deleted. If you really want to delete these items, "
                             "please unlock them and try again."));
        KMessageBox::informationList(this, infoMsg, lockedList, i18n("Information"));
    }

    if (folders.isEmpty())
    {
        return;
    }

    QString warnMsg(i18np("About to delete this image. "
                          "Deleted file is unrecoverable. "
                          "Are you sure?",
                          "About to delete these %1 images. "
                          "Deleted files are unrecoverable. "
                          "Are you sure?",
                          deleteList.count()));

    if (KMessageBox::warningContinueCancelList(this, warnMsg,
                                               deleteList,
                                               i18n("Warning"),
                                               KGuiItem(i18n("Delete")))
        ==  KMessageBox::Continue)
    {
        QStringList::iterator itFolder = folders.begin();
        QStringList::iterator itFile   = files.begin();

        d->statusProgressBar->setProgressValue(0);
        d->statusProgressBar->setProgressTotalSteps(deleteList.count());
        d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);

        for ( ; itFolder != folders.end(); ++itFolder, ++itFile)
        {
            d->controller->deleteFile(*itFolder, *itFile);
            // the currentlyDeleting list is used to prevent loading items which
            // will immanently be deleted into the sidebar and wasting time
            d->currentlyDeleting.append(*itFolder + *itFile);
        }
    }
}

void CameraUI::slotDeleteSelected()
{
    deleteItems(true, false);
}

void CameraUI::slotDeleteAll()
{
    deleteItems(false, false);
}

void CameraUI::slotDeleted(const QString& folder, const QString& file, bool status)
{
    if (status)
    {
        d->view->removeItem(d->view->findItemInfo(folder, file));
        // do this after removeItem, which will signal to slotItemsSelected, which checks for the list
        d->currentlyDeleting.removeAll(folder + file);
    }

    int curr = d->statusProgressBar->progressValue();
    d->statusProgressBar->setProgressTotalSteps(curr+1);
    refreshFreeSpace();
}

void CameraUI::slotFileView()
{
    CamItemInfo info = d->view->firstItemSelected();
    if (!info.isNull())
    {
        slotFileView(info);
    }
}

void CameraUI::slotFileView(const CamItemInfo& info)
{
    d->controller->openFile(info.folder, info.name);
}

void CameraUI::slotMetadata(const QString& folder, const QString& file, const DMetadata& meta)
{
    CamItemInfo info = d->view->findItemInfo(folder, file);
    if (!info.isNull())
    {
        d->rightSideBar->itemChanged(info, meta);
    }
}

void CameraUI::slotNewSelection(bool hasSelection)
{
    if (!d->controller)
    {
        return;
    }

    d->downloadSelectedAction->setEnabled(hasSelection);
    d->downloadDelSelectedAction->setEnabled(hasSelection && d->controller->cameraDeleteSupport());
    d->deleteSelectedAction->setEnabled(hasSelection && d->controller->cameraDeleteSupport());
    d->imageViewAction->setEnabled(hasSelection);
    d->lockAction->setEnabled(hasSelection);

    if (hasSelection)
    {
        // only enable "Mark as downloaded" if at least one
        // selected image has not been downloaded
        bool haveNotDownloadedItem = false;

        for (IconItem* item = d->view->firstItem(); item;
             item = item->nextItem())
        {
            const CameraIconItem* const iconItem = static_cast<CameraIconItem*>(item);

            if (iconItem->isSelected())
            {
                haveNotDownloadedItem = !iconItem->isDownloaded();

                if (haveNotDownloadedItem)
                {
                    break;
                }
            }
        }

        d->markAsDownloadedAction->setEnabled(haveNotDownloadedItem);
    }
    else
    {
        d->markAsDownloadedAction->setEnabled(false);
    }

    unsigned long fSize = 0;
    unsigned long dSize = 0;
    d->view->itemsSelectionSizeInfo(fSize, dSize);
    d->albumLibraryFreeSpace->setEstimatedDSizeKb(dSize);
}

void CameraUI::slotItemsSelected(const CamItemInfo& info, bool selected)
{
    if (!d->controller)
    {
        return;
    }

    if (selected)
    {
        // if selected item is in the list of item which will be deleted, set no current item
        if (!d->currentlyDeleting.contains(info.folder + info.name))
        {
            d->rightSideBar->itemChanged(info, DMetadata());
            d->controller->getMetadata(info.folder, info.name);
        }
        else
        {
            d->rightSideBar->slotNoCurrentItem();
        }
    }
    else
    {
        d->rightSideBar->slotNoCurrentItem();
    }

    // update availability of actions
    slotNewSelection(d->view->countSelected() > 0);
}

bool CameraUI::createAutoAlbum(const KUrl& parentURL, const QString& sub,
                               const QDate& date, QString& errMsg) const
{
    KUrl u(parentURL);
    u.addPath(sub);

    // first stat to see if the album exists
    QFileInfo info(u.toLocalFile());

    if (info.exists())
    {
        // now check if its really a directory
        if (info.isDir())
        {
            return true;
        }
        else
        {
            errMsg = i18n("A file with the same name (%1) already exists in folder %2.",
                          sub, parentURL.toLocalFile());
            return false;
        }
    }

    // looks like the directory does not exist, try to create it

    PAlbum* parent = AlbumManager::instance()->findPAlbum(parentURL);

    if (!parent)
    {
        errMsg = i18n("Failed to find Album for path '%1'.", parentURL.toLocalFile());
        return false;
    }

    return AlbumManager::instance()->createPAlbum(parent, sub, QString(), date, QString(), errMsg);
}

void CameraUI::slotFirstItem()
{
    CameraIconItem* currItem = dynamic_cast<CameraIconItem*>(d->view->firstItem());
    d->view->clearSelection();
    d->view->updateContents();

    if (currItem)
    {
        d->view->setCurrentItem(currItem);
    }
}

void CameraUI::slotPrevItem()
{
    CameraIconItem* currItem = dynamic_cast<CameraIconItem*>(d->view->currentItem());
    d->view->clearSelection();
    d->view->updateContents();

    if (currItem)
    {
        d->view->setCurrentItem(currItem->prevItem());
    }
}

void CameraUI::slotNextItem()
{
    CameraIconItem* currItem = dynamic_cast<CameraIconItem*>(d->view->currentItem());
    d->view->clearSelection();
    d->view->updateContents();

    if (currItem)
    {
        d->view->setCurrentItem(currItem->nextItem());
    }
}

void CameraUI::slotLastItem()
{
    CameraIconItem* currItem = dynamic_cast<CameraIconItem*>(d->view->lastItem());
    d->view->clearSelection();
    d->view->updateContents();

    if (currItem)
    {
        d->view->setCurrentItem(currItem);
    }
}

void CameraUI::slotEditKeys()
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection( actionCollection(), i18n( "General" ) );
    dialog.configure();
}

void CameraUI::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group("Camera Settings"));
    KEditToolBar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void CameraUI::slotConfNotifications()
{
    KNotifyConfigWidget::configure(this);
}

void CameraUI::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group("Camera Settings"));
}

void CameraUI::slotSetup()
{
    Setup::exec(this);
}

void CameraUI::slotToggleFullScreen()
{
    if (d->fullScreen) // out of fullscreen
    {
        setWindowState( windowState() & ~Qt::WindowFullScreen ); // reset

        slotShowMenuBar();
        statusBar()->show();
        showToolBars();

        if (d->removeFullScreenButton)
        {
            QList<KToolBar*> toolbars = toolBars();
            foreach (KToolBar* toolbar, toolbars)
            {
                // name is set in ui.rc XML file
                if (toolbar->objectName() == "ToolBar")
                {
                    toolbar->removeAction(d->fullScreenAction);
                    break;
                }
            }
        }

        d->rightSideBar->restore();

        d->fullScreen = false;
    }
    else  // go to fullscreen
    {
        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();

        if (d->fullScreenHideToolBar)
        {
            hideToolBars();
        }
        else
        {
            showToolBars();

            QList<KToolBar*> toolbars = toolBars();
            KToolBar* mainToolbar = 0;
            foreach (KToolBar* toolbar, toolbars)
            {
                if (toolbar->objectName() == "ToolBar")
                {
                    mainToolbar = toolbar;
                    break;
                }
            }

            // add fullscreen action if necessary
            if ( mainToolbar && !mainToolbar->actions().contains(d->fullScreenAction) )
            {
                mainToolbar->addAction(d->fullScreenAction);
                d->removeFullScreenButton=true;
            }
            else
            {
                // If FullScreen button is enabled in toolbar settings,
                // we shall not remove it when leaving of fullscreen mode.
                d->removeFullScreenButton=false;
            }
        }

        d->rightSideBar->backup();

        setWindowState( windowState() | Qt::WindowFullScreen ); // set
        d->fullScreen = true;
    }
}

void CameraUI::slotEscapePressed()
{
    if (d->fullScreen)
    {
        d->fullScreenAction->activate(QAction::Trigger);
    }
}

void CameraUI::showToolBars()
{
    QList<KToolBar*> toolbars = toolBars();
    foreach (KToolBar* toolbar, toolbars)
    {
        toolbar->show();
    }
}

void CameraUI::hideToolBars()
{
    QList<KToolBar*> toolbars = toolBars();
    foreach (KToolBar* toolbar, toolbars)
    {
        toolbar->hide();
    }
}

void CameraUI::slotCameraFreeSpaceInfo(unsigned long kBSize, unsigned long kBAvail)
{
    d->cameraFreeSpace->addInformation(kBSize, kBSize-kBAvail, kBAvail, QString());
}

bool CameraUI::cameraDeleteSupport() const
{
    return d->controller->cameraDeleteSupport();
}

bool CameraUI::cameraUploadSupport() const
{
    return d->controller->cameraUploadSupport();
}

bool CameraUI::cameraMkDirSupport() const
{
    return d->controller->cameraMkDirSupport();
}

bool CameraUI::cameraDelDirSupport() const
{
    return d->controller->cameraDelDirSupport();
}

void CameraUI::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void CameraUI::slotDBStat()
{
    showDigikamDatabaseStat();
}

void CameraUI::refreshCollectionFreeSpace()
{
    d->albumLibraryFreeSpace->setPaths(CollectionManager::instance()->allAvailableAlbumRootPaths());
}

void CameraUI::slotCollectionLocationStatusChanged(const CollectionLocation&, int)
{
    refreshCollectionFreeSpace();
}

void CameraUI::slotShowMenuBar()
{
    menuBar()->setVisible(d->showMenuBarAction->isChecked());
}

void CameraUI::slotSidebarTabTitleStyleChanged()
{
    d->rightSideBar->setStyle(AlbumSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->applySettings();
}

void CameraUI::slotLogMsg(const QString& msg, DHistoryView::EntryType type,
                          const QString& folder, const QString& file)
{
    d->statusProgressBar->setProgressText(msg);
    QStringList meta;
    meta << folder << file;
    d->historyView->addedEntry(msg, type, QVariant(meta));
}

void CameraUI::slotShowLog()
{
    d->showLogAction->isChecked() ? d->historyView->show() : d->historyView->hide();
}

void CameraUI::slotHistoryEntryClicked(const QVariant& metadata)
{
    QStringList meta = metadata.toStringList();
    QString folder   = meta[0];
    QString file     = meta[1];
    d->view->ensureItemVisible(folder, file);
}

bool CameraUI::chronologicOrder() const
{
    return !d->lastPhotoFirstAction->isChecked();
}

}  // namespace Digikam
