/* ============================================================
*
* This file is a part of digiKam project
* http://www.digikam.org
*
* Date        : 2004-09-16
* Description : Import tool interface
*
* Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
* Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
* Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
* Copyright (C) 2012      by Andi Clemens <andi dot clemens at gmail dot com>
* Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
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

#include "importui.h"
#include "importui_p.h"

// Qt includes

#include <QCheckBox>
#include <QCloseEvent>
#include <QDir>
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
#include <QSignalMapper>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QKeySequence>
#include <QInputDialog>
#include <QMenuBar>
#include <QMenu>
#include <QIcon>
#include <QMessageBox>
#include <QStatusBar>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>

// Local includes

#include "drawdecoder.h"
#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "dfileselector.h"
#include "digikam_debug.h"
#include "digikam_globals.h"
#include "cameramessagebox.h"
#include "advancedrenamemanager.h"
#include "album.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "albumselectdialog.h"
#include "cameracontroller.h"
#include "camerafolderdialog.h"
#include "camerainfodialog.h"
#include "cameralist.h"
#include "cameranamehelper.h"
#include "cameratype.h"
#include "capturedlg.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "collectionscanner.h"
#include "componentsinfo.h"
#include "dlogoaction.h"
#include "coredbdownloadhistory.h"
#include "dzoombar.h"
#include "fileactionmngr.h"
#include "freespacewidget.h"
#include "iccsettings.h"
#include "imagepropertiessidebarcamgui.h"
#include "importsettings.h"
#include "importview.h"
#include "imagedialog.h"
#include "dnotificationwrapper.h"
#include "newitemsfinder.h"
#include "parsesettings.h"
#include "renamecustomizer.h"
#include "scancontroller.h"
#include "setup.h"
#include "sidebar.h"
#include "statusprogressbar.h"
#include "thememanager.h"
#include "thumbnailsize.h"
#include "importthumbnailmodel.h"
#include "imagepropertiestab.h"

namespace Digikam
{

ImportUI* ImportUI::m_instance = 0;

ImportUI::ImportUI(const QString& cameraTitle, const QString& model,
                   const QString& port, const QString& path, int startIndex)
    : DXmlGuiWindow(0),
      d(new Private)
{
    setConfigGroupName(QLatin1String("Camera Settings"));

    setXMLFile(QLatin1String("importui5.rc"));
    setFullScreenOptions(FS_IMPORTUI);
    setWindowFlags(Qt::Window);

    m_instance = this;

    // --------------------------------------------------------

    QString title  = CameraNameHelper::cameraName(cameraTitle);
    d->cameraTitle = (title.isEmpty()) ? cameraTitle : title;
    setCaption(d->cameraTitle);

    setupCameraController(model, port, path);
    setupUserArea();
    setInitialSorting();
    setupActions();
    setupStatusBar();
    setupAccelerators();

    // -- Make signals/slots connections ---------------------------------

    setupConnections();
    sidebarTabTitleStyleChanged();
    slotColorManagementOptionsChanged();

    // -- Read settings --------------------------------------------------

    readSettings();
    setAutoSaveSettings(configGroupName(), true);

    // -------------------------------------------------------------------

    //d->historyUpdater = new CameraHistoryUpdater(this);

    //connect (d->historyUpdater, SIGNAL(signalHistoryMap(CHUpdateItemMap)),
    //this, SLOT(slotRefreshIconView(CHUpdateItemMap)));

    //connect(d->historyUpdater, SIGNAL(signalBusy(bool)),
    //        this, SLOT(slotBusy(bool)));

    // --------------------------------------------------------

    d->progressTimer = new QTimer(this);

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));

    // --------------------------------------------------------

    d->renameCustomizer->setStartIndex(startIndex);
    d->view->setFocus();

    // -- Init icon view zoom factor --------------------------

    slotThumbSizeChanged(ImportSettings::instance()->getDefaultIconSize());
    slotZoomSliderChanged(ImportSettings::instance()->getDefaultIconSize());

    // try to connect in the end, this allows us not to block the UI to show up..
    QTimer::singleShot(0, d->controller, SLOT(slotConnect()));
}

ImportUI::~ImportUI()
{
    saveSettings();
    m_instance = 0;
    disconnect(d->view, 0, this, 0);
    delete d->view;
    delete d->rightSideBar;
    delete d->controller;
    delete d;
}

ImportUI* ImportUI::instance()
{
    return m_instance;
}

void ImportUI::setupUserArea()
{
    DHBox* const widget = new DHBox(this);
    d->splitter         = new SidebarSplitter(widget);
    DVBox* const vbox   = new DVBox(d->splitter);
    d->view             = new ImportView(this, vbox);
    d->view->importFilterModel()->setCameraThumbsController(d->camThumbsCtrl);
    d->view->importFilterModel()->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
    d->historyView      = new DHistoryView(vbox);
    d->rightSideBar     = new ImagePropertiesSideBarCamGui(widget, d->splitter, Qt::RightEdge, true);
    d->rightSideBar->setObjectName(QLatin1String("CameraGui Sidebar Right"));
    d->splitter->setFrameStyle(QFrame::NoFrame);
    d->splitter->setFrameShadow(QFrame::Plain);
    d->splitter->setFrameShape(QFrame::NoFrame);
    d->splitter->setOpaqueResize(false);
    d->splitter->setStretchFactor(0, 10);      // set iconview default size to max.

    vbox->setStretchFactor(d->view, 10);
    vbox->setStretchFactor(d->historyView, 2);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(0);

    d->errorWidget = new DNotificationWidget(vbox);
    d->errorWidget->setMessageType(DNotificationWidget::Error);
    d->errorWidget->setCloseButtonVisible(false);
    d->errorWidget->hide();

    // -------------------------------------------------------------------------

    d->advBox = new DExpanderBox(d->rightSideBar);
    d->advBox->setObjectName(QLatin1String("Camera Settings Expander"));

    d->renameCustomizer = new RenameCustomizer(d->advBox, d->cameraTitle);
    d->renameCustomizer->setWhatsThis(i18n("Set how digiKam will rename files as they are downloaded."));
    d->advBox->addItem(d->renameCustomizer, QIcon::fromTheme(QLatin1String("insert-image")), i18n("File Renaming Options"),
                       QLatin1String("RenameCustomizer"), true);

    // -- Albums Auto-creation options -----------------------------------------

    d->albumCustomizer = new AlbumCustomizer(d->advBox);
    d->advBox->addItem(d->albumCustomizer, QIcon::fromTheme(QLatin1String("folder-new")), i18n("Auto-creation of Albums"),
                       QLatin1String("AlbumBox"), false);

    // -- On the Fly options ---------------------------------------------------

    d->advancedSettings = new AdvancedSettings(d->advBox);
    d->advBox->addItem(d->advancedSettings, QIcon::fromTheme(QLatin1String("system-run")), i18n("On the Fly Operations (JPEG only)"),
                       QLatin1String("OnFlyBox"), true);

    // -- DNG convert options --------------------------------------------------

    d->dngConvertSettings = new DNGConvertSettings(d->advBox);
    d->advBox->addItem(d->dngConvertSettings, QIcon::fromTheme(QLatin1String("image-x-adobe-dng")), i18n("DNG Convert Options"),
                       QLatin1String("DNGSettings"), false);

    // -- Scripting options ----------------------------------------------------

    d->scriptingSettings = new ScriptingSettings(d->advBox);
    d->advBox->addItem(d->scriptingSettings, QIcon::fromTheme(QLatin1String("utilities-terminal")), i18n("Scripting"),
                       QLatin1String("ScriptingBox"), false);
    d->advBox->addStretch();

    d->rightSideBar->appendTab(d->advBox, QIcon::fromTheme(QLatin1String("configure")), i18n("Settings"));
    d->rightSideBar->loadState();

    // -------------------------------------------------------------------------

    setCentralWidget(widget);
}

void ImportUI::setupActions()
{
    d->cameraActions = new QActionGroup(this);

    KActionCollection *ac = actionCollection();
    // -- File menu ----------------------------------------------------

    d->cameraCancelAction = new QAction(QIcon::fromTheme(QLatin1String("process-stop")), i18nc("@action Cancel process", "Cancel"), this);
    connect(d->cameraCancelAction, SIGNAL(triggered()), this, SLOT(slotCancelButton()));
    ac->addAction(QLatin1String("importui_cancelprocess"), d->cameraCancelAction);
    d->cameraCancelAction->setEnabled(false);

    // -----------------------------------------------------------------

    d->cameraInfoAction = new QAction(QIcon::fromTheme(QLatin1String("camera-photo")), i18nc("@action Information about camera", "Information"), this);
    connect(d->cameraInfoAction, SIGNAL(triggered()), this, SLOT(slotInformation()));
    ac->addAction(QLatin1String("importui_info"), d->cameraInfoAction);
    d->cameraActions->addAction(d->cameraInfoAction);

    // -----------------------------------------------------------------

    d->cameraCaptureAction = new QAction(QIcon::fromTheme(QLatin1String("webcamreceive")), i18nc("@action Capture photo from camera", "Capture"), this);
    connect(d->cameraCaptureAction, SIGNAL(triggered()), this, SLOT(slotCapture()));
    ac->addAction(QLatin1String("importui_capture"), d->cameraCaptureAction);
    d->cameraActions->addAction(d->cameraCaptureAction);

    // -----------------------------------------------------------------

    QAction* const closeAction = buildStdAction(StdCloseAction, this, SLOT(close()), this);
    ac->addAction(QLatin1String("importui_close"), closeAction);

    // -- Edit menu ----------------------------------------------------

    d->selectAllAction = new QAction(i18nc("@action:inmenu", "Select All"), this);
    connect(d->selectAllAction, SIGNAL(triggered()), d->view, SLOT(slotSelectAll()));
    ac->addAction(QLatin1String("importui_selectall"), d->selectAllAction);
    ac->setDefaultShortcut(d->selectAllAction, Qt::CTRL + Qt::Key_A);
    d->cameraActions->addAction(d->selectAllAction);

    // -----------------------------------------------------------------

    d->selectNoneAction = new QAction(i18nc("@action:inmenu", "Select None"), this);
    connect(d->selectNoneAction, SIGNAL(triggered()), d->view, SLOT(slotSelectNone()));
    ac->addAction(QLatin1String("importui_selectnone"), d->selectNoneAction);
    ac->setDefaultShortcut(d->selectNoneAction, Qt::CTRL + Qt::SHIFT + Qt::Key_A);
    d->cameraActions->addAction(d->selectNoneAction);

    // -----------------------------------------------------------------

    d->selectInvertAction = new QAction(i18nc("@action:inmenu", "Invert Selection"), this);
    connect(d->selectInvertAction, SIGNAL(triggered()), d->view, SLOT(slotSelectInvert()));
    ac->addAction(QLatin1String("importui_selectinvert"), d->selectInvertAction);
    ac->setDefaultShortcut(d->selectInvertAction, Qt::CTRL + Qt::Key_Asterisk);
    d->cameraActions->addAction(d->selectInvertAction);

    // -----------------------------------------------------------

    d->selectNewItemsAction = new QAction(QIcon::fromTheme(QLatin1String("folder-favorites")), i18nc("@action:inmenu", "Select New Items"), this);
    connect(d->selectNewItemsAction, SIGNAL(triggered()), this, SLOT(slotSelectNew()));
    ac->addAction(QLatin1String("importui_selectnewitems"), d->selectNewItemsAction);
    d->cameraActions->addAction(d->selectNewItemsAction);

    // -----------------------------------------------------------

    d->selectLockedItemsAction = new QAction(QIcon::fromTheme(QLatin1String("object-locked")), i18nc("@action:inmenu", "Select Locked Items"), this);
    connect(d->selectLockedItemsAction, SIGNAL(triggered()), this, SLOT(slotSelectLocked()));
    ac->addAction(QLatin1String("importui_selectlockeditems"), d->selectLockedItemsAction);
    ac->setDefaultShortcut(d->selectLockedItemsAction, Qt::CTRL + Qt::Key_L);
    d->cameraActions->addAction(d->selectLockedItemsAction);

    // --- Download actions ----------------------------------------------------

    d->downloadAction = new QMenu(i18nc("@title:menu", "Download"), this);
    d->downloadAction->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    ac->addAction(QLatin1String("importui_imagedownload"), d->downloadAction->menuAction());
    d->cameraActions->addAction(d->downloadAction->menuAction());

    d->downloadNewAction = new QAction(QIcon::fromTheme(QLatin1String("folder-favorites")), i18nc("@action", "Download New"), this);
    connect(d->downloadNewAction, SIGNAL(triggered()), this, SLOT(slotDownloadNew()));
    ac->addAction(QLatin1String("importui_imagedownloadnew"), d->downloadNewAction);
    ac->setDefaultShortcut(d->downloadNewAction, Qt::CTRL + Qt::Key_N);
    d->downloadAction->addAction(d->downloadNewAction);
    d->cameraActions->addAction(d->downloadNewAction);

    d->downloadSelectedAction = new QAction(QIcon::fromTheme(QLatin1String("document-save")), i18nc("@action", "Download Selected"), this);
    connect(d->downloadSelectedAction, SIGNAL(triggered()), this, SLOT(slotDownloadSelected()));
    ac->addAction(QLatin1String("importui_imagedownloadselected"), d->downloadSelectedAction);
    d->downloadSelectedAction->setEnabled(false);
    d->downloadAction->addAction(d->downloadSelectedAction);
    d->cameraActions->addAction(d->downloadSelectedAction);

    d->downloadAllAction = new QAction(QIcon::fromTheme(QLatin1String("document-save")), i18nc("@action", "Download All"), this);
    connect(d->downloadAllAction, SIGNAL(triggered()), this, SLOT(slotDownloadAll()));
    ac->addAction(QLatin1String("importui_imagedownloadall"), d->downloadAllAction);
    d->downloadAction->addAction(d->downloadAllAction);
    d->cameraActions->addAction(d->downloadAllAction);

    // -------------------------------------------------------------------------

    d->downloadDelNewAction = new QAction(i18nc("@action", "Download && Delete New"), this);
    connect(d->downloadDelNewAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteNew()));
    ac->addAction(QLatin1String("importui_imagedownloaddeletenew"), d->downloadDelNewAction);
    ac->setDefaultShortcut(d->downloadDelNewAction, Qt::CTRL + Qt::SHIFT + Qt::Key_N);
    d->cameraActions->addAction(d->downloadDelNewAction);

    // -----------------------------------------------------------------

    d->downloadDelSelectedAction = new QAction(i18nc("@action", "Download && Delete Selected"), this);
    connect(d->downloadDelSelectedAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteSelected()));
    ac->addAction(QLatin1String("importui_imagedownloaddeleteselected"), d->downloadDelSelectedAction);
    d->downloadDelSelectedAction->setEnabled(false);
    d->cameraActions->addAction(d->downloadDelSelectedAction);

    // -------------------------------------------------------------------------

    d->downloadDelAllAction = new QAction(i18nc("@action", "Download && Delete All"), this);
    connect(d->downloadDelAllAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteAll()));
    ac->addAction(QLatin1String("importui_imagedownloaddeleteall"), d->downloadDelAllAction);
    d->cameraActions->addAction(d->downloadDelAllAction);

    // -------------------------------------------------------------------------

    d->uploadAction = new QAction(QIcon::fromTheme(QLatin1String("media-flash-sd-mmc")), i18nc("@action", "Upload..."), this);
    connect(d->uploadAction, SIGNAL(triggered()), this, SLOT(slotUpload()));
    ac->addAction(QLatin1String("importui_imageupload"), d->uploadAction);
    ac->setDefaultShortcut(d->uploadAction, Qt::CTRL + Qt::Key_U);
    d->cameraActions->addAction(d->uploadAction);

    // -------------------------------------------------------------------------

    d->lockAction = new QAction(QIcon::fromTheme(QLatin1String("object-locked")), i18nc("@action", "Toggle Lock"), this);
    connect(d->lockAction, SIGNAL(triggered()), this, SLOT(slotToggleLock()));
    ac->addAction(QLatin1String("importui_imagelock"), d->lockAction);
    ac->setDefaultShortcut(d->lockAction, Qt::CTRL + Qt::Key_G);
    d->cameraActions->addAction(d->lockAction);

    // -------------------------------------------------------------------------

    d->markAsDownloadedAction = new QAction(QIcon::fromTheme(QLatin1String("dialog-ok-apply")), i18nc("@action", "Mark as downloaded"), this);
    connect(d->markAsDownloadedAction, SIGNAL(triggered()), this, SLOT(slotMarkAsDownloaded()));
    ac->addAction(QLatin1String("importui_imagemarkasdownloaded"), d->markAsDownloadedAction);
    d->cameraActions->addAction(d->markAsDownloadedAction);

    // --- Delete actions ------------------------------------------------------

    d->deleteAction = new QMenu(i18nc("@title:menu", "Delete"), this);
    d->deleteAction->setIcon(QIcon::fromTheme(QLatin1String("user-trash")));
    ac->addAction(QLatin1String("importui_delete"), d->deleteAction->menuAction());
    d->cameraActions->addAction(d->deleteAction->menuAction());

    d->deleteSelectedAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18nc("@action", "Delete Selected"), this);
    connect(d->deleteSelectedAction, SIGNAL(triggered()), this, SLOT(slotDeleteSelected()));
    ac->addAction(QLatin1String("importui_imagedeleteselected"), d->deleteSelectedAction);
    ac->setDefaultShortcut(d->deleteSelectedAction, Qt::Key_Delete);
    d->deleteSelectedAction->setEnabled(false);
    d->deleteAction->addAction(d->deleteSelectedAction);
    d->cameraActions->addAction(d->deleteSelectedAction);

    d->deleteAllAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18nc("@action", "Delete All"), this);
    connect(d->deleteAllAction, SIGNAL(triggered()), this, SLOT(slotDeleteAll()));
    ac->addAction(QLatin1String("importui_imagedeleteall"), d->deleteAllAction);
    d->deleteAction->addAction(d->deleteAllAction);
    d->cameraActions->addAction(d->deleteAllAction);

    d->deleteNewAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18nc("@action", "Delete New"), this);
    connect(d->deleteNewAction, SIGNAL(triggered()), this, SLOT(slotDeleteNew()));
    ac->addAction(QLatin1String("importui_imagedeletenew"), d->deleteNewAction);
    d->deleteAction->addAction(d->deleteNewAction);
    d->cameraActions->addAction(d->deleteNewAction);

    // --- Icon view, items preview, and map actions ------------------------------------------------------

    d->imageViewSelectionAction = new KSelectAction(QIcon::fromTheme(QLatin1String("view-preview")), i18nc("@title:group", "Views"), this);
    ac->addAction(QLatin1String("importui_view_selection"), d->imageViewSelectionAction);

    d->iconViewAction = new QAction(QIcon::fromTheme(QLatin1String("view-list-icons")),
                                    i18nc("@action Go to thumbnails (icon) view", "Thumbnails"), this);
    d->iconViewAction->setCheckable(true);
    ac->addAction(QLatin1String("importui_icon_view"), d->iconViewAction);
    connect(d->iconViewAction, SIGNAL(triggered()), d->view, SLOT(slotIconView()));
    d->imageViewSelectionAction->addAction(d->iconViewAction);

    d->camItemPreviewAction = new QAction(QIcon::fromTheme(QLatin1String("view-preview")),
                                                i18nc("@action View the selected image", "Preview Item"), this);
    d->camItemPreviewAction->setCheckable(true);
    ac->addAction(QLatin1String("importui_item_view"), d->camItemPreviewAction);
    ac->setDefaultShortcut(d->camItemPreviewAction, Qt::Key_F3);
    connect(d->camItemPreviewAction, SIGNAL(triggered()), d->view, SLOT(slotImagePreview()));
    d->imageViewSelectionAction->addAction(d->camItemPreviewAction);

#ifdef HAVE_MARBLE
    d->mapViewAction = new QAction(QIcon::fromTheme(QLatin1String("globe")),
                                   i18nc("@action Switch to map view", "Map"), this);
    d->mapViewAction->setCheckable(true);
    ac->addAction(QLatin1String("importui_map_view"), d->mapViewAction);
    connect(d->mapViewAction, SIGNAL(triggered()), d->view, SLOT(slotMapWidgetView()));
    d->imageViewSelectionAction->addAction(d->mapViewAction);
#endif // HAVE_MARBLE

    /// @todo Add table view stuff here

    // -- Item Sorting ------------------------------------------------------------

    d->itemSortAction                    = new KSelectAction(i18nc("@title:menu", "&Sort Items"), this);
    d->itemSortAction->setWhatsThis(i18nc("@info:whatsthis", "The value by which the items are sorted in the thumbnail view"));
    QSignalMapper* const imageSortMapper = new QSignalMapper(this);
    connect(imageSortMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSortImagesBy(int)));
    ac->addAction(QLatin1String("item_sort"), d->itemSortAction);

    // map to CamItemSortSettings enum
    QAction* const sortByNameAction     = d->itemSortAction->addAction(i18nc("item:inmenu Sort by", "By Name"));
    QAction* const sortByPathAction     = d->itemSortAction->addAction(i18nc("item:inmenu Sort by", "By Path"));
    QAction* const sortByDateAction     = d->itemSortAction->addAction(i18nc("item:inmenu Sort by", "By Date")); //TODO: Implement sort by creation date.
    QAction* const sortByFileSizeAction = d->itemSortAction->addAction(i18nc("item:inmenu Sort by", "By Size"));
    QAction* const sortByRatingAction   = d->itemSortAction->addAction(i18nc("item:inmenu Sort by", "By Rating"));
    QAction* const sortByDownloadAction = d->itemSortAction->addAction(i18nc("item:inmenu Sort by", "By Download State"));

    connect(sortByNameAction,     SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByPathAction,     SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByDateAction,     SIGNAL(triggered()), imageSortMapper, SLOT(map())); //TODO: Implement sort by creation date.
    connect(sortByFileSizeAction, SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByRatingAction,   SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByDownloadAction, SIGNAL(triggered()), imageSortMapper, SLOT(map()));

    imageSortMapper->setMapping(sortByNameAction,     (int)CamItemSortSettings::SortByFileName);
    imageSortMapper->setMapping(sortByPathAction,     (int)CamItemSortSettings::SortByFilePath);
    imageSortMapper->setMapping(sortByDateAction,     (int)CamItemSortSettings::SortByCreationDate); //TODO: Implement sort by creation date.
    imageSortMapper->setMapping(sortByFileSizeAction, (int)CamItemSortSettings::SortByFileSize);
    imageSortMapper->setMapping(sortByRatingAction,   (int)CamItemSortSettings::SortByRating);
    imageSortMapper->setMapping(sortByDownloadAction, (int)CamItemSortSettings::SortByDownloadState);

    d->itemSortAction->setCurrentItem(ImportSettings::instance()->getImageSortBy());

    // -- Item Sort Order ------------------------------------------------------------

    d->itemSortOrderAction                    = new KSelectAction(i18nc("@title:inmenu", "Item Sorting &Order"), this);
    d->itemSortOrderAction->setWhatsThis(i18nc("@info:whatsthis", "Defines whether items are sorted in ascending or descending manner."));
    QSignalMapper* const imageSortOrderMapper = new QSignalMapper(this);
    connect(imageSortOrderMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSortImagesOrder(int)));
    ac->addAction(QLatin1String("item_sort_order"), d->itemSortOrderAction);

    QAction* const sortAscendingAction = d->itemSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-ascending")), i18nc("@item:inmenu Sorting Order", "Ascending"));
    QAction* const sortDescendingAction = d->itemSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-descending")), i18nc("@item:inmenu Sorting Order", "Descending"));

    connect(sortAscendingAction,  SIGNAL(triggered()), imageSortOrderMapper, SLOT(map()));
    connect(sortDescendingAction, SIGNAL(triggered()), imageSortOrderMapper, SLOT(map()));

    imageSortOrderMapper->setMapping(sortAscendingAction, (int)CamItemSortSettings::AscendingOrder);
    imageSortOrderMapper->setMapping(sortDescendingAction, (int)CamItemSortSettings::DescendingOrder);

    d->itemSortOrderAction->setCurrentItem(ImportSettings::instance()->getImageSortOrder());

    // -- Item Grouping ------------------------------------------------------------

    d->itemsGroupAction                  = new KSelectAction(i18nc("@title:menu", "&Group Items"), this);
    d->itemsGroupAction->setWhatsThis(i18nc("@info:whatsthis", "The categories in which the items in the thumbnail view are displayed"));
    QSignalMapper* const itemSeparationMapper = new QSignalMapper(this);
    connect(itemSeparationMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSeparateImages(int)));
    ac->addAction(QLatin1String("item_group"), d->itemsGroupAction);

    // map to CamItemSortSettings enum
    QAction* const noCategoriesAction  = d->itemsGroupAction->addAction(i18nc("@item:inmenu Group Items", "Flat List"));
    QAction* const groupByFolderAction = d->itemsGroupAction->addAction(i18nc("@item:inmenu Group Items", "By Folder"));
    QAction* const groupByFormatAction = d->itemsGroupAction->addAction(i18nc("@item:inmenu Group Items", "By Format"));
    QAction* const groupByDateAction =   d->itemsGroupAction->addAction(i18nc("@item:inmenu Group Items", "By Date"));

    connect(noCategoriesAction,  SIGNAL(triggered()), itemSeparationMapper, SLOT(map()));
    connect(groupByFolderAction, SIGNAL(triggered()), itemSeparationMapper, SLOT(map()));
    connect(groupByFormatAction, SIGNAL(triggered()), itemSeparationMapper, SLOT(map()));
    connect(groupByDateAction,   SIGNAL(triggered()), itemSeparationMapper, SLOT(map()));

    itemSeparationMapper->setMapping(noCategoriesAction,  (int)CamItemSortSettings::NoCategories);
    itemSeparationMapper->setMapping(groupByFolderAction, (int)CamItemSortSettings::CategoryByFolder);
    itemSeparationMapper->setMapping(groupByFormatAction, (int)CamItemSortSettings::CategoryByFormat);
    itemSeparationMapper->setMapping(groupByDateAction,   (int)CamItemSortSettings::CategoryByDate);

    d->itemsGroupAction->setCurrentItem(ImportSettings::instance()->getImageSeparationMode());

    // -- Standard 'View' menu actions ---------------------------------------------

    d->increaseThumbsAction = buildStdAction(StdZoomInAction, d->view, SLOT(slotZoomIn()), this);
    d->increaseThumbsAction->setEnabled(false);
    QKeySequence keysPlus(d->increaseThumbsAction->shortcut()[0], Qt::Key_Plus);
    ac->addAction(QLatin1String("importui_zoomplus"), d->increaseThumbsAction);
    ac->setDefaultShortcut(d->increaseThumbsAction, keysPlus);

    d->decreaseThumbsAction = buildStdAction(StdZoomOutAction, d->view, SLOT(slotZoomOut()), this);
    d->decreaseThumbsAction->setEnabled(false);
    QKeySequence keysMinus(d->decreaseThumbsAction->shortcut()[0], Qt::Key_Minus);
    ac->addAction(QLatin1String("importui_zoomminus"), d->decreaseThumbsAction);
    ac->setDefaultShortcut(d->decreaseThumbsAction, keysMinus);

    d->zoomFitToWindowAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")), i18nc("@action:inmenu", "Fit to &Window"), this);
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), d->view, SLOT(slotFitToWindow()));
    ac->addAction(QLatin1String("import_zoomfit2window"), d->zoomFitToWindowAction);
    ac->setDefaultShortcut(d->zoomFitToWindowAction, Qt::ALT + Qt::CTRL + Qt::Key_E);

    d->zoomTo100percents = new QAction(QIcon::fromTheme(QLatin1String("zoom-original")), i18nc("@action:inmenu", "Zoom to 100%"), this);
    connect(d->zoomTo100percents, SIGNAL(triggered()), d->view, SLOT(slotZoomTo100Percents()));
    ac->addAction(QLatin1String("import_zoomto100percents"), d->zoomTo100percents);
    ac->setDefaultShortcut(d->zoomTo100percents, Qt::CTRL + Qt::Key_Period);

    // ------------------------------------------------------------------------------------------------

    d->viewCMViewAction = new QAction(QIcon::fromTheme(QLatin1String("video-display")), i18n("Color-Managed View"), this);
    d->viewCMViewAction->setCheckable(true);
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    ac->addAction(QLatin1String("color_managed_view"), d->viewCMViewAction);
    ac->setDefaultShortcut(d->viewCMViewAction, Qt::Key_F12);

    // ------------------------------------------------------------------------------------------------

    createFullScreenAction(QLatin1String("importui_fullscreen"));
    createSidebarActions();

    d->showLogAction = new QAction(QIcon::fromTheme(QLatin1String("edit-find")), i18nc("@option:check", "Show History"), this);
    d->showLogAction->setCheckable(true);
    connect(d->showLogAction, SIGNAL(triggered()), this, SLOT(slotShowLog()));
    ac->addAction(QLatin1String("importui_showlog"), d->showLogAction);
    ac->setDefaultShortcut(d->showLogAction, Qt::CTRL + Qt::Key_H);

    d->showBarAction = new QAction(QIcon::fromTheme(QLatin1String("view-choose")), i18nc("@option:check", "Show Thumbbar"), this);
    d->showBarAction->setCheckable(true);
    connect(d->showBarAction, SIGNAL(triggered()), this, SLOT(slotToggleShowBar()));
    ac->addAction(QLatin1String("showthumbs"), d->showBarAction);
    ac->setDefaultShortcut(d->showBarAction, Qt::CTRL+Qt::Key_T);
    d->showBarAction->setEnabled(false);

    // ---------------------------------------------------------------------------------

    ThemeManager::instance()->registerThemeActions(this);

    // Standard 'Help' menu actions
    createHelpActions();

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // Standard 'Configure' menu actions
    createSettingsActions();

    // -- Keyboard-only actions added to <MainWindow> ----------------------------------

    QAction* const altBackwardAction = new QAction(i18nc("@action", "Previous Image"), this);
    ac->addAction(QLatin1String("importui_backward_shift_space"), altBackwardAction);
    ac->setDefaultShortcut(altBackwardAction, Qt::SHIFT + Qt::Key_Space);
    connect(altBackwardAction, SIGNAL(triggered()), d->view, SLOT(slotPrevItem()));

    // ---------------------------------------------------------------------------------

    d->connectAction = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")), i18nc("@action Connection failed, try again?", "Retry"), this);
    connect(d->connectAction, SIGNAL(triggered()), d->controller, SLOT(slotConnect()));

    createGUI(xmlFile());
    cleanupActions();

    showMenuBarAction()->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080
}

void ImportUI::updateActions()
{
    CamItemInfoList list = d->view->selectedCamItemInfos();
    bool hasSelection    = list.count() > 0;

    d->downloadDelSelectedAction->setEnabled(hasSelection && d->controller->cameraDeleteSupport());
    d->deleteSelectedAction->setEnabled(hasSelection && d->controller->cameraDeleteSupport());
    d->camItemPreviewAction->setEnabled(hasSelection && cameraUseUMSDriver());
    d->downloadSelectedAction->setEnabled(hasSelection);
    d->lockAction->setEnabled(hasSelection);

    if (hasSelection)
    {
        // only enable "Mark as downloaded" if at least one
        // selected image has not been downloaded
        bool haveNotDownloadedItem = false;

        foreach(const CamItemInfo& info, list)
        {
            haveNotDownloadedItem = !(info.downloaded == CamItemInfo::DownloadedYes);

            if (haveNotDownloadedItem)
            {
                break;
            }
        }

        d->markAsDownloadedAction->setEnabled(haveNotDownloadedItem);
    }
    else
    {
        d->markAsDownloadedAction->setEnabled(false);
    }
}

void ImportUI::setupConnections()
{
    //TODO: Needs testing.
    connect(d->advancedSettings, SIGNAL(signalDownloadNameChanged()),
            this, SLOT(slotUpdateDownloadName()));

    connect(d->dngConvertSettings, SIGNAL(signalDownloadNameChanged()),
            this, SLOT(slotUpdateDownloadName()));

    connect(d->historyView, SIGNAL(signalEntryClicked(QVariant)),
            this, SLOT(slotHistoryEntryClicked(QVariant)));

    connect(IccSettings::instance(), SIGNAL(settingsChanged()),
            this, SLOT(slotColorManagementOptionsChanged()));

    // -------------------------------------------------------------------------

    connect(d->view, SIGNAL(signalImageSelected(CamItemInfoList,CamItemInfoList)),
            this, SLOT(slotImageSelected(CamItemInfoList,CamItemInfoList)));

    connect(d->view, SIGNAL(signalSwitchedToPreview()),
            this, SLOT(slotSwitchedToPreview()));

    connect(d->view, SIGNAL(signalSwitchedToIconView()),
            this, SLOT(slotSwitchedToIconView()));

    connect(d->view, SIGNAL(signalSwitchedToMapView()),
            this, SLOT(slotSwitchedToMapView()));

    connect(d->view, SIGNAL(signalNewSelection(bool)),
            this, SLOT(slotNewSelection(bool)));

    // -------------------------------------------------------------------------

    connect(d->view, SIGNAL(signalThumbSizeChanged(int)),
            this, SLOT(slotThumbSizeChanged(int)));

    connect(d->view, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));

    connect(d->zoomBar, SIGNAL(signalZoomSliderChanged(int)),
            this, SLOT(slotZoomSliderChanged(int)));

    connect(d->zoomBar, SIGNAL(signalZoomValueEdited(double)),
            d->view, SLOT(setZoomFactor(double)));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->zoomBar, SLOT(slotUpdateTrackerPos()));

    // -------------------------------------------------------------------------

    connect(CollectionManager::instance(), SIGNAL(locationStatusChanged(CollectionLocation,int)),
            this, SLOT(slotCollectionLocationStatusChanged(CollectionLocation,int)));

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    connect(d->renameCustomizer, SIGNAL(signalChanged()),
            this, SLOT(slotUpdateDownloadName()));
}

void ImportUI::setupStatusBar()
{
    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->statusProgressBar->setNotificationTitle(d->cameraTitle, QIcon::fromTheme(QLatin1String("camera-photo")));
    statusBar()->addWidget(d->statusProgressBar, 100);

    //------------------------------------------------------------------------------

    d->cameraFreeSpace = new FreeSpaceWidget(statusBar(), 100);

    if (cameraUseGPhotoDriver())
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

    statusBar()->addWidget(d->cameraFreeSpace, 1);

    //------------------------------------------------------------------------------

    d->albumLibraryFreeSpace = new FreeSpaceWidget(statusBar(), 100);
    d->albumLibraryFreeSpace->setMode(FreeSpaceWidget::AlbumLibrary);
    statusBar()->addWidget(d->albumLibraryFreeSpace, 1);
    refreshCollectionFreeSpace();

    //------------------------------------------------------------------------------

    //TODO: Replace it with FilterStatusBar after advanced filtring is implemented.
    d->filterComboBox = new FilterComboBox(statusBar());
    setFilter(d->filterComboBox->currentFilter());
    statusBar()->addWidget(d->filterComboBox, 1);
    connect(d->filterComboBox, SIGNAL(filterChanged(Filter*)), this, SLOT(setFilter(Filter*)));

    //------------------------------------------------------------------------------

    d->zoomBar = new DZoomBar(statusBar());
    d->zoomBar->setZoomToFitAction(d->zoomFitToWindowAction);
    d->zoomBar->setZoomTo100Action(d->zoomTo100percents);
    d->zoomBar->setZoomPlusAction(d->increaseThumbsAction);
    d->zoomBar->setZoomMinusAction(d->decreaseThumbsAction);
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    statusBar()->addPermanentWidget(d->zoomBar, 1);
}

void ImportUI::setupCameraController(const QString& model, const QString& port, const QString& path)
{
    d->controller = new CameraController(this, d->cameraTitle, model, port, path);

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

    d->controller->start();

    // Setup Thumbnails controller -------------------------------------------------------

    d->camThumbsCtrl = new CameraThumbsCtrl(d->controller, this);
}

CameraThumbsCtrl* ImportUI::getCameraThumbsCtrl() const
{
    return d->camThumbsCtrl;
}

void ImportUI::setupAccelerators()
{
    KActionCollection *ac = actionCollection();

    QAction* const escapeAction = new QAction(i18nc("@action", "Exit Preview Mode"), this);
    ac->addAction(QLatin1String("exit_preview_mode"), escapeAction);
    ac->setDefaultShortcut(escapeAction, Qt::Key_Escape);
    connect(escapeAction, SIGNAL(triggered()), this, SIGNAL(signalEscapePressed()));

    QAction* const nextImageAction = new QAction(i18nc("@action","Next Image"), this);
    nextImageAction->setIcon(QIcon::fromTheme(QLatin1String("go-next")));
    ac->addAction(QLatin1String("next_image"), nextImageAction);
    ac->setDefaultShortcut(nextImageAction, Qt::Key_Space);
    connect(nextImageAction, SIGNAL(triggered()), d->view, SLOT(slotNextItem()));

    QAction* const previousImageAction = new QAction(i18nc("@action", "Previous Image"), this);
    previousImageAction->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));
    ac->addAction(QLatin1String("previous_image"), previousImageAction);
    ac->setDefaultShortcuts(previousImageAction, QList<QKeySequence>() << Qt::Key_Backspace << Qt::SHIFT+Qt::Key_Space);
    connect(previousImageAction, SIGNAL(triggered()), d->view, SLOT(slotPrevItem()));

    QAction* const firstImageAction = new QAction(i18nc("@action Go to first image", "First Image"), this);
    ac->addAction(QLatin1String("first_image"), firstImageAction);
    ac->setDefaultShortcut(firstImageAction, Qt::Key_Home);
    connect(firstImageAction, SIGNAL(triggered()), d->view, SLOT(slotFirstItem()));

    QAction* const lastImageAction = new QAction(i18nc("@action Go to last image", "Last Image"), this);
    ac->addAction(QLatin1String("last_image"), lastImageAction);
    ac->setDefaultShortcut(lastImageAction, Qt::Key_End);
    connect(lastImageAction, SIGNAL(triggered()), d->view, SLOT(slotLastItem()));
}

void ImportUI::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    readFullScreenSettings(group);

    d->showBarAction->setChecked(ImportSettings::instance()->getShowThumbbar());
    d->showLogAction->setChecked(group.readEntry(QLatin1String("ShowLog"), false));
    d->albumCustomizer->readSettings(group);
    d->advancedSettings->readSettings(group);
    d->dngConvertSettings->readSettings(group);
    d->scriptingSettings->readSettings(group);

    d->advBox->readSettings(group);

    d->splitter->restoreState(group);

    slotShowLog();
}

void ImportUI::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    ImportSettings::instance()->setShowThumbbar(d->showBarAction->isChecked());
    ImportSettings::instance()->saveSettings();
    group.writeEntry(QLatin1String("ShowLog"), d->showLogAction->isChecked());
    d->albumCustomizer->saveSettings(group);
    d->advancedSettings->saveSettings(group);
    d->dngConvertSettings->saveSettings(group);
    d->scriptingSettings->saveSettings(group);

    d->advBox->writeSettings(group);

    d->rightSideBar->saveState();
    d->splitter->saveState(group);
    d->filterComboBox->saveSettings();

    config->sync();
}

bool ImportUI::isBusy() const
{
    return d->busy;
}

bool ImportUI::isClosed() const
{
    return d->closed;
}

QString ImportUI::cameraTitle() const
{
    return d->cameraTitle;
}

DownloadSettings ImportUI::downloadSettings() const
{
    DownloadSettings settings = d->advancedSettings->settings();
    d->dngConvertSettings->settings(&settings);
    d->scriptingSettings->settings(&settings);
    return settings;
}

void ImportUI::setInitialSorting()
{
    d->view->slotSeparateImages(ImportSettings::instance()->getImageSeparationMode());
    d->view->slotSortImagesBy(ImportSettings::instance()->getImageSortBy());
    d->view->slotSortImagesOrder(ImportSettings::instance()->getImageSortOrder());
}

void ImportUI::slotCancelButton()
{
    d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                             i18nc("@info:status", "Canceling current operation, please wait..."));
    d->controller->slotCancel();
    //d->historyUpdater->slotCancel();
    d->currentlyDeleting.clear();
    refreshFreeSpace();
}

void ImportUI::refreshFreeSpace()
{
    if (cameraUseGPhotoDriver())
    {
        d->controller->getFreeSpace();
    }
    else
    {
        d->cameraFreeSpace->refresh();
    }
}

void ImportUI::closeEvent(QCloseEvent* e)
{
    DXmlGuiWindow::closeEvent(e);

    if (dialogClosed())
    {
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void ImportUI::moveEvent(QMoveEvent* e)
{
    Q_UNUSED(e)
    emit signalWindowHasMoved();
}

void ImportUI::slotClose()
{
/* FIXME
    if (dialogClosed())
        reject();
*/
}

bool ImportUI::dialogClosed()
{
    if (d->closed)
    {
        return true;
    }

    if (isBusy())
    {
        if (QMessageBox::question(this, qApp->applicationName(),
                                  i18nc("@info", "Do you want to close the dialog "
                                        "and cancel the current operation?"),
                                  QMessageBox::Yes | QMessageBox::No
                                 ) == QMessageBox::No)
        {
            return false;
        }
    }

    d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                             i18nc("@info:status", "Disconnecting from camera, please wait..."));

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

void ImportUI::finishDialog()
{
    // Look if an item have been downloaded to computer during camera GUI session.
    // If yes, update the starting number value used to rename camera items from camera list.

    if (d->view->downloadedCamItemInfos() > 0)
    {
        CameraList* const clist = CameraList::defaultList();

        if (clist)
        {
            clist->changeCameraStartIndex(d->cameraTitle, d->renameCustomizer->startIndex());
        }
    }

    if (!d->foldersToScan.isEmpty())
    {
        // TODO is this note valid anymore with new progress handling?
        // When a directory is created, a watch is put on it to spot new files
        // but it can occur that the file is copied there before the watch is
        // completely setup. That is why as an extra safeguard run CollectionScanner
        // over the folders we used. Bug: 119201

        d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                                 i18nc("@info:status", "Scanning for new files, please wait..."));

        NewItemsFinder* const tool = new NewItemsFinder(NewItemsFinder::ScheduleCollectionScan, d->foldersToScan.toList());
        tool->start();

        d->foldersToScan.clear();
    }

    deleteLater();

    if (!d->lastDestURL.isEmpty())
    {
        emit signalLastDestination(d->lastDestURL);
    }

    saveSettings();
}

void ImportUI::slotBusy(bool val)
{
    if (!val)   // Camera is available for actions.
    {
        if (!d->busy)
        {
            return;
        }

        d->busy = false;
        d->cameraCancelAction->setEnabled(false);
        d->cameraActions->setEnabled(true);
        d->advBox->setEnabled(true);
        d->view->setEnabled(true);

        // selection-dependent update of lockAction, markAsDownloadedAction,
        // downloadSelectedAction, downloadDelSelectedAction, deleteSelectedAction
        updateActions();

        m_animLogo->stop();
        d->statusProgressBar->setProgressValue(0);
        d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode, i18nc("@info:status", "Ready"));

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

        if (!m_animLogo->running())
        {
            m_animLogo->start();
        }

        d->busy = true;
        d->cameraActions->setEnabled(false);
    }
}

void ImportUI::slotZoomSliderChanged(int size)
{
    d->view->setThumbSize(size);
}

void ImportUI::slotZoomChanged(double zoom)
{
    double zmin = d->view->zoomMin();
    double zmax = d->view->zoomMax();
    d->zoomBar->setZoom(zoom, zmin, zmax);

    if (!fullScreenIsActive())
    {
        d->zoomBar->triggerZoomTrackerToolTip();
    }
}

void ImportUI::slotThumbSizeChanged(int size)
{
    d->zoomBar->setThumbsSize(size);

    if (!fullScreenIsActive())
    {
        d->zoomBar->triggerZoomTrackerToolTip();
    }
}

void ImportUI::slotConnected(bool val)
{
    if (!val)
    {
        d->errorWidget->setText(i18nc("@info", "Failed to connect to the camera. "
                                               "Please make sure it is connected "
                                               "properly and turned on."));

        d->errorWidget->actions().clear();
        d->errorWidget->addAction(d->connectAction);
        d->errorWidget->addAction(d->showPreferencesAction);
        d->errorWidget->animatedShow();
    }
    else
    {
        // disable unsupported actions
        d->uploadAction->setEnabled(d->controller->cameraUploadSupport());

        d->cameraCaptureAction->setEnabled(d->controller->cameraCaptureImageSupport());

        d->errorWidget->hide();
        refreshFreeSpace();
        // FIXME ugly c&p from slotFolderList
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(d->configGroupName);
        bool useMetadata          = group.readEntry(d->configUseFileMetadata, false);
        d->controller->listRootFolder(useMetadata);
    }
}

void ImportUI::slotFolderList(const QStringList& folderList)
{
    if (d->closed)
    {
        return;
    }

    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->setProgressTotalSteps(0);

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    bool useMetadata          = group.readEntry(d->configUseFileMetadata, false);

    // when getting a list of subfolders, request their contents and also their subfolders
    for (QStringList::const_iterator it = folderList.constBegin();
         it != folderList.constEnd(); ++it)
    {
        d->controller->listFiles(*it, useMetadata);
        d->controller->listFolders(*it);
    }
}

void ImportUI::setFilter(Filter *filter)
{
    d->view->importFilterModel()->setFilter(filter);
}

void ImportUI::slotCapture()
{
    if (d->busy)
    {
        return;
    }

    CaptureDlg* const captureDlg = new CaptureDlg(this, d->controller, d->cameraTitle);
    captureDlg->show();
}

void ImportUI::slotInformation()
{
    if (d->busy)
    {
        return;
    }

    d->controller->getCameraInformation();
}

void ImportUI::slotCameraInformation(const QString& summary, const QString& manual, const QString& about)
{
    CameraInfoDialog* const infoDlg = new CameraInfoDialog(this, summary, manual, about);
    infoDlg->show();
}

void ImportUI::slotUpload()
{
    if (d->busy)
    {
        return;
    }

    QList<QUrl> urls = ImageDialog::getImageURLs(this,
                                                 QUrl::fromLocalFile(CollectionManager::instance()->oneAlbumRootPath()),
                                                 i18nc("@title:window", "Select Image to Upload"));

    if (!urls.isEmpty())
    {
        slotUploadItems(urls);
    }
}

void ImportUI::slotUploadItems(const QList<QUrl>& urls)
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

        for (QList<QUrl>::const_iterator it = urls.constBegin() ; it != urls.constEnd() ; ++it)
        {
            QFileInfo fi((*it).toLocalFile());
            totalKbSize += fi.size() / 1024;
        }

        if (totalKbSize >= d->cameraFreeSpace->kBAvail())
        {
            QMessageBox::critical(this, qApp->applicationName(),
                                  i18nc("@info", "There is not enough free space on the Camera Medium "
                                        "to upload pictures.\n\n"
                                        "Space require: %1\n"
                                        "Available free space: %2",
                                        ImagePropertiesTab::humanReadableBytesCount(totalKbSize * 1024),
                                        ImagePropertiesTab::humanReadableBytesCount(d->cameraFreeSpace->kBAvail() * 1024)));
            return;
        }
    }

    QMap<QString, int> map           = countItemsByFolders();
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

    for (QList<QUrl>::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
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

        QString ext  = QLatin1String(".") + fi.completeSuffix();
        QString name = fi.fileName();
        name.truncate(fi.fileName().length() - ext.length());

        bool ok;

        CamItemInfo uploadInfo;
        uploadInfo.folder = cameraFolder;
        uploadInfo.name   = name + ext;

        while (d->view->hasImage(uploadInfo))
        {
            QString msg(i18nc("@info", "<qt>Camera Folder <resource>%1</resource> already contains the item <resource>%2</resource>.<br>"
                              "Please enter a new filename (without extension):</qt>",
                              QDir::toNativeSeparators(cameraFolder), fi.fileName()));
            uploadInfo.name = QInputDialog::getText(this,
                                                    i18nc("@title:window", "File already exists"),
                                                    msg,
                                                    QLineEdit::Normal,
                                                    name,
                                                    &ok) + ext;

            if (!ok)
            {
                return;
            }
        }

        d->controller->upload(fi, uploadInfo.name, cameraFolder);
    }

    delete dlg;
}

void ImportUI::slotUploaded(const CamItemInfo& /*itemInfo*/)
{
    if (d->closed)
    {
        return;
    }

    refreshFreeSpace();
}

void ImportUI::slotDownloadNew()
{
    slotSelectNew();
    QTimer::singleShot(0, this, SLOT(slotDownloadSelected()));
}

void ImportUI::slotDownloadAndDeleteNew()
{
    slotSelectNew();
    QTimer::singleShot(0, this, SLOT(slotDownloadAndDeleteSelected()));
}

void ImportUI::slotDownloadSelected()
{
    slotDownload(true, false);
}

void ImportUI::slotDownloadAndDeleteSelected()
{
    slotDownload(true, true);
}

void ImportUI::slotDownloadAll()
{
    slotDownload(false, false);
}

void ImportUI::slotDownloadAndDeleteAll()
{
    slotDownload(false, true);
}

void ImportUI::slotDownload(bool onlySelected, bool deleteAfter, Album* album)
{
    if (d->albumCustomizer->folderDateFormat() == AlbumCustomizer::CustomDateFormat &&
        !d->albumCustomizer->customDateFormatIsValid())
    {
        QMessageBox::information(this, qApp->applicationName(),
                                 i18nc("@info", "Your custom target album date format is not valid. Please check your settings..."));
        return;
    }

    // See bug #143934: force to select all items to prevent problem
    // when !renameCustomizer->useDefault() ==> iconItem->getDownloadName()
    // can return an empty string in this case because it depends on selection.
    if (!onlySelected)
    {
        d->view->slotSelectAll();
    }

    // Update the download names.
    slotNewSelection(d->view->selectedUrls().count() > 0);

    // -- Get the destination album from digiKam library ---------------

    PAlbum* pAlbum = 0;

    if (!album)
    {
        AlbumManager* const man   = AlbumManager::instance();

        // Check if default target album option is enabled.

        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(d->configGroupName);
        bool useDefaultTarget     = group.readEntry(d->configUseDefaultTargetAlbum, false);

        if (useDefaultTarget)
        {
            PAlbum* const pa = man->findPAlbum(group.readEntry(d->configDefaultTargetAlbumId, 0));

            if (pa)
            {
                CollectionLocation cl = CollectionManager::instance()->locationForAlbumRootId(pa->albumRootId());

                if (!cl.isAvailable() || cl.isNull())
                {
                    QMessageBox::information(this,qApp->applicationName(),
                                             i18nc("@info", "Collection which host your default target album set to process "
                                                   "download from camera device is not available. Please select another one from "
                                                   "camera configuration dialog."));
                    return;
                }
            }
            else
            {
                QMessageBox::information(this, qApp->applicationName(),
                                         i18nc("@info", "Your default target album set to process download "
                                               "from camera device is not available. Please select another one from "
                                               "camera configuration dialog."));
                return;
            }

            pAlbum = pa;
        }
        else
        {
            AlbumList list = man->currentAlbums();
            int albumId    = 0;

            if (!list.isEmpty())
            {
                albumId = group.readEntry(d->configLastTargetAlbum, list.first()->globalID());
            }

            album = man->findAlbum(albumId);

            if (album && album->type() != Album::PHYSICAL)
            {
                album = 0;
            }

            QString header(i18nc("@info", "<p>Please select the destination album from the digiKam library to "
                                 "import the camera pictures into.</p>"));

            album = AlbumSelectDialog::selectAlbum(this, dynamic_cast<PAlbum*>(album), header);

            if (!album)
            {
                return;
            }

            pAlbum = dynamic_cast<PAlbum*>(album);
            group.writeEntry(d->configLastTargetAlbum, album->globalID());
        }
    }
    else
    {
        pAlbum = dynamic_cast<PAlbum*>(album);
    }

    if (!pAlbum)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Destination Album is null";
        return;
    }

    // -- Check disk space ------------------------
    // See bug #139519: Always check free space available before to
    // download items selection from camera.

    if (!checkDiskSpace(pAlbum))
    {
        return;
    }

    // -- Prepare and download camera items ------------------------
    // Since we show camera items in reverse order, downloading need to be done also in reverse order.

    downloadCameraItems(pAlbum, onlySelected, deleteAfter);
}

void ImportUI::slotDownloaded(const QString& folder, const QString& file, int status)
{
    // Is auto-rotate option checked?
    bool autoRotate = downloadSettings().autoRotate;
    bool previewItems = ImportSettings::instance()->getPreviewItemsWhileDownload();

    CamItemInfo& info = d->view->camItemInfoRef(folder, file);

    if (!info.isNull())
    {
        setDownloaded(info, status);

        if (status == CamItemInfo::DownloadStarted && previewItems)
        {
            emit signalPreviewRequested(info, true);
        }

        if (d->rightSideBar->url() == info.url())
        {
            updateRightSideBar(info);
        }

        if (info.downloaded == CamItemInfo::DownloadedYes)
        {
            int curr = d->statusProgressBar->progressValue();
            d->statusProgressBar->setProgressValue(curr + 1);

            d->renameCustomizer->setStartIndex(d->renameCustomizer->startIndex() + 1);

            CoreDbDownloadHistory::setDownloaded(QString::fromUtf8(d->controller->cameraMD5ID()),
                                           info.name,
                                           info.size,
                                           info.ctime);
        }
    }

    // Download all items is complete ?
    if (d->statusProgressBar->progressValue() == d->statusProgressBar->progressTotalSteps())
    {
        if (d->deleteAfter)
        {
            // No need passive pop-up here, because we will ask to confirm items deletion with dialog.
            deleteItems(true, true);
        }
        else
        {
            // Pop-up a notification to inform user when all is done, and inform if auto-rotation will take place.
            if (autoRotate)
            {
                DNotificationWrapper(QLatin1String("cameradownloaded"),
                                     i18nc("@info Popup notification",
                                           "Images download finished, you can now detach your camera while the images will be auto-rotated"),
                                     this, windowTitle());
            }
            else
            {
                DNotificationWrapper(QLatin1String("cameradownloaded"),
                                     i18nc("@info Popup notification",
                                           "Images download finished"),
                                     this, windowTitle());
            }
        }
    }
}

void ImportUI::slotDownloadComplete(const QString&, const QString&,
                                    const QString& destFolder, const QString&)
{
    ScanController::instance()->scheduleCollectionScanRelaxed(destFolder);
    autoRotateItems();
}

void ImportUI::slotSkipped(const QString& folder, const QString& file)
{
    CamItemInfo info = d->view->camItemInfo(folder, file);

    if (!info.isNull())
    {
        setDownloaded(info, CamItemInfo::DownloadedNo);
    }

    int curr = d->statusProgressBar->progressValue();
    d->statusProgressBar->setProgressValue(curr + 1);
}

void ImportUI::slotMarkAsDownloaded()
{
    CamItemInfoList list = d->view->selectedCamItemInfos();

    foreach(const CamItemInfo& info, list)
    {
        setDownloaded(d->view->camItemInfoRef(info.folder, info.name), CamItemInfo::DownloadedYes);

        CoreDbDownloadHistory::setDownloaded(QString::fromUtf8(d->controller->cameraMD5ID()),
                                       info.name,
                                       info.size,
                                       info.ctime);
    }
}

void ImportUI::slotToggleLock()
{
    CamItemInfoList list = d->view->selectedCamItemInfos();
    int count            = list.count();

    if (count > 0)
    {
        d->statusProgressBar->setProgressValue(0);
        d->statusProgressBar->setProgressTotalSteps(count);
        d->statusProgressBar->setProgressBarMode(StatusProgressBar::ProgressBarMode);
    }

    foreach(const CamItemInfo& info, list)
    {
        QString folder = info.folder;
        QString file   = info.name;
        int writePerm  = info.writePermissions;
        bool lock      = true;

        // If item is currently locked, unlock it.
        if (writePerm == 0)
        {
            lock = false;
        }

        d->controller->lockFile(folder, file, lock);
    }
}

void ImportUI::slotLocked(const QString& folder, const QString& file, bool status)
{
    if (status)
    {
        CamItemInfo& info = d->view->camItemInfoRef(folder, file);

        if (!info.isNull())
        {
            toggleLock(info);

            if (d->rightSideBar->url() == info.url())
            {
                updateRightSideBar(info);
            }
        }
    }

    int curr = d->statusProgressBar->progressValue();
    d->statusProgressBar->setProgressValue(curr + 1);
}

void ImportUI::slotUpdateDownloadName()
{
    QList<QUrl> selected      = d->view->selectedUrls();
    bool hasNoSelection       = selected.count() == 0;
    CamItemInfoList list      = d->view->allItems();
    DownloadSettings settings = downloadSettings();
    QString newName;

    foreach(const CamItemInfo& info, list)
    {
        CamItemInfo& refInfo = d->view->camItemInfoRef(info.folder, info.name);
        // qCDebug(DIGIKAM_IMPORTUI_LOG) << "slotDownloadNameChanged, old: " << refInfo.downloadName;

        newName = info.name;

        if (hasNoSelection || selected.contains(info.url()))
        {
            if (d->renameCustomizer->useDefault())
            {
                newName = d->renameCustomizer->newName(info.name);
            }
            else if (d->renameCustomizer->isEnabled())
            {
                newName = d->renameCustomizer->newName(info.url().toLocalFile());
            }
            else if (!refInfo.downloadName.isEmpty())
            {
                newName = refInfo.downloadName;
            }

            // Renaming files for the converting jpg to a lossless format
            // is from cameracontroller.cpp moved here.

            if (settings.convertJpeg && info.mime == QLatin1String("image/jpeg"))
            {
                QFileInfo     fi(newName);
                QString ext = fi.suffix();

                if (!ext.isEmpty())
                {
                    if (ext[0].isUpper() && ext[ext.length()-1].isUpper())
                    {
                        ext = settings.losslessFormat.toUpper();
                    }
                    else if (ext[0].isUpper())
                    {
                        ext    = settings.losslessFormat.toLower();
                        ext[0] = ext[0].toUpper();
                    }
                    else
                    {
                        ext = settings.losslessFormat.toLower();
                    }

                    newName = fi.completeBaseName() + QLatin1Char('.') + ext;
                }
                else
                {
                    newName = newName + QLatin1Char('.') + settings.losslessFormat.toLower();
                }
            }
            else if (settings.convertDng && info.mime == QLatin1String("image/x-raw"))
            {
                QFileInfo     fi(newName);
                QString ext = fi.suffix();

                if (!ext.isEmpty())
                {
                    if (ext[0].isUpper() && (ext[ext.length()-1].isUpper() || ext[ext.length()-1].isDigit()))
                    {
                        ext = QLatin1String("DNG");
                    }
                    else if (ext[0].isUpper())
                    {
                        ext = QLatin1String("Dng");
                    }
                    else
                    {
                        ext = QLatin1String("dng");
                    }

                    newName = fi.completeBaseName() + QLatin1Char('.') + ext;
                }
                else
                {
                    newName = newName + QLatin1Char('.') + QLatin1String("dng");
                }
            }
        }

        refInfo.downloadName = newName;
        // qCDebug(DIGIKAM_IMPORTUI_LOG) << "slotDownloadNameChanged, new: " << refInfo.downloadName;
    }

    d->view->updateIconView();
}

//FIXME: the new pictures are marked by CameraHistoryUpdater which is not working yet.
void ImportUI::slotSelectNew()
{
    CamItemInfoList infos = d->view->allItems();
    CamItemInfoList toBeSelected;

    foreach(const CamItemInfo& info, infos)
    {
        if (info.downloaded == CamItemInfo::DownloadedNo)
        {
            toBeSelected << info;
        }
    }

    d->view->setSelectedCamItemInfos(toBeSelected);
}

void ImportUI::slotSelectLocked()
{
    CamItemInfoList allItems = d->view->allItems();
    CamItemInfoList toBeSelected;

    foreach(const CamItemInfo& info, allItems)
    {
        if (info.writePermissions == 0)
        {
            toBeSelected << info;
        }
    }

    d->view->setSelectedCamItemInfos(toBeSelected);
}

void ImportUI::toggleLock(CamItemInfo& info)
{
    if (!info.isNull())
    {
        if (info.writePermissions == 0)
        {
            info.writePermissions = 1;
        }
        else
        {
            info.writePermissions = 0;
        }
    }
}

// TODO is this really necessary? why not just use the folders from listfolders call?
QMap<QString, int> ImportUI::countItemsByFolders() const
{
    QString                      path;
    QMap<QString, int>           map;
    QMap<QString, int>::iterator it;

    CamItemInfoList infos = d->view->allItems();

    foreach(const CamItemInfo& info, infos)
    {
        path = info.folder;

        if (!path.isEmpty() && path.endsWith(QLatin1Char('/')))
        {
            path.truncate(path.length() - 1);
        }

        it = map.find(path);

        if (it == map.end())
        {
            map.insert(path, 1);
        }
        else
        {
            it.value() ++;
        }
    }

    return map;
}

void ImportUI::setDownloaded(CamItemInfo& itemInfo, int status)
{
    itemInfo.downloaded = status;
    d->progressValue = 0;

    if (itemInfo.downloaded == CamItemInfo::DownloadStarted)
    {
        d->progressTimer->start(500);
    }
    else
    {
        d->progressTimer->stop();
    }
}

void ImportUI::slotProgressTimerDone()
{
    d->progressTimer->start(300);
}

void ImportUI::itemsSelectionSizeInfo(unsigned long& fSizeKB, unsigned long& dSizeKB)
{
    qint64 fSize = 0;  // Files size
    qint64 dSize = 0;  // Estimated space requires to download and process files.

    QList<QUrl> selected      = d->view->selectedUrls();
    CamItemInfoList list      = d->view->allItems();
    DownloadSettings settings = downloadSettings();

    foreach(const CamItemInfo& info, list)
    {
        if (selected.contains(info.url()))
        {
            qint64 size = info.size;

            if (size < 0) // -1 if size is not provided by camera
            {
                continue;
            }

            fSize += size;

            if (info.mime == QLatin1String("image/jpeg"))
            {
                if (settings.convertJpeg)
                {
                    // Estimated size is around 5 x original size when JPEG=>PNG.
                    dSize += size * 5;
                }
                else if (settings.autoRotate)
                {
                    // We need a double size to perform rotation.
                    dSize += size * 2;
                }
                else
                {
                    // Real file size is added.
                    dSize += size;
                }
            }
            else if (settings.convertDng && info.mime == QLatin1String("image/x-raw"))
            {
                // Estimated size is around 2 x original size when RAW=>DNG.
                dSize += size * 2;
            }
            else
            {
                dSize += size;
            }

        }
    }

    fSizeKB = fSize / 1024;
    dSizeKB = dSize / 1024;
}

void ImportUI::checkItem4Deletion(const CamItemInfo& info, QStringList& folders, QStringList& files,
                                  CamItemInfoList& deleteList, CamItemInfoList& lockedList)
{
    if (info.writePermissions != 0)  // Item not locked ?
    {
        QString folder = info.folder;
        QString file   = info.name;
        folders.append(folder);
        files.append(file);
        deleteList.append(info);
    }
    else
    {
        lockedList.append(info);
    }
}

void ImportUI::deleteItems(bool onlySelected, bool onlyDownloaded)
{
    QStringList     folders;
    QStringList     files;
    CamItemInfoList deleteList;
    CamItemInfoList lockedList;
    CamItemInfoList list = onlySelected ? d->view->selectedCamItemInfos() : d->view->allItems();

    foreach(const CamItemInfo& info, list)
    {
        if (onlyDownloaded)
        {
            if (info.downloaded == CamItemInfo::DownloadedYes)
            {
                checkItem4Deletion(info, folders, files, deleteList, lockedList);
            }
        }
        else
        {
            checkItem4Deletion(info, folders, files, deleteList, lockedList);
        }
    }

    // If we want to delete some locked files, just give a feedback to user.
    if (!lockedList.isEmpty())
    {
        QString infoMsg(i18nc("@info", "The items listed below are locked by camera (read-only). "
                              "These items will not be deleted. If you really want to delete these items, "
                              "please unlock them and try again."));
        CameraMessageBox::informationList(d->camThumbsCtrl, this, i18n("Information"), infoMsg, lockedList);
    }

    if (folders.isEmpty())
    {
        return;
    }

    QString warnMsg(i18ncp("@info", "About to delete this image. "
                           "<b>Deleted file is unrecoverable.</b> "
                           "Are you sure?",
                           "About to delete these %1 images. "
                           "<b>Deleted files are unrecoverable.</b> "
                           "Are you sure?",
                           deleteList.count()));

    if (CameraMessageBox::warningContinueCancelList(d->camThumbsCtrl,
                                                    this,
                                                    i18n("Warning"),
                                                    warnMsg,
                                                    deleteList,
                                                    QLatin1String("DontAskAgainToDeleteItemsFromCamera"))
        ==  QMessageBox::Yes)
    {
        QStringList::const_iterator itFolder = folders.constBegin();
        QStringList::const_iterator itFile   = files.constBegin();

        d->statusProgressBar->setProgressValue(0);
        d->statusProgressBar->setProgressTotalSteps(deleteList.count());
        d->statusProgressBar->setProgressBarMode(StatusProgressBar::ProgressBarMode);

        // enable cancel action.
        d->cameraCancelAction->setEnabled(true);

        for (; itFolder != folders.constEnd(); ++itFolder, ++itFile)
        {
            d->controller->deleteFile(*itFolder, *itFile);
            // the currentlyDeleting list is used to prevent loading items which
            // will immanently be deleted into the sidebar and wasting time
            d->currentlyDeleting.append(*itFolder + *itFile);
        }
    }
}

bool ImportUI::checkDiskSpace(PAlbum *pAlbum)
{
    if (!pAlbum)
    {
        return false;
    }

    unsigned long fSize   = 0;
    unsigned long dSize   = 0;
    itemsSelectionSizeInfo(fSize, dSize);
    QString albumRootPath = pAlbum->albumRootPath();
    unsigned long kBAvail = d->albumLibraryFreeSpace->kBAvail(albumRootPath);

    if (dSize >= kBAvail)
    {
        int result = QMessageBox::warning(this, i18nc("@title:window", "Insufficient Disk Space"),
                                          i18nc("@info", "There is not enough free space on the disk of the album you selected "
                                                "to download and process the selected pictures from the camera.\n\n"
                                                "Estimated space required: %1\n"
                                                "Available free space: %2\n\n"
                                                "Try Anyway?",
                                                ImagePropertiesTab::humanReadableBytesCount(dSize * 1024),
                                                ImagePropertiesTab::humanReadableBytesCount(kBAvail * 1024)),
                                          QMessageBox::Yes | QMessageBox::No);

        if (result == QMessageBox::No)
        {
            return false;
        }
    }

    return true;
}

bool ImportUI::downloadCameraItems(PAlbum* pAlbum, bool onlySelected, bool deleteAfter)
{
    KSharedConfig::Ptr config      = KSharedConfig::openConfig();
    KConfigGroup group             = config->group(d->configGroupName);
    SetupCamera::ConflictRule rule = (SetupCamera::ConflictRule)group.readEntry(d->configFileSaveConflictRule,
                                                                                (int)SetupCamera::DIFFNAME);

    d->controller->downloadPrep(rule);

    QString              downloadName;
    DownloadSettingsList allItems;
    DownloadSettings     settings = downloadSettings();
    QUrl url = pAlbum->fileUrl();
    int downloadedItems           = 0;

    // -- Download camera items -------------------------------

    QList<QUrl> selected = d->view->selectedUrls();
    CamItemInfoList list = d->view->allItems();
    QSet<QString> usedDownloadPaths;

    foreach(const CamItemInfo& info, list)
    {
        if (onlySelected && !(selected.contains(info.url())))
        {
            continue;
        }

        settings.folder     = info.folder;
        settings.file       = info.name;
        settings.mime       = info.mime;
        settings.pickLabel  = info.pickLabel;
        settings.colorLabel = info.colorLabel;
        settings.rating     = info.rating;
        // downloadName should already be set by now
        downloadName = info.downloadName;

        QUrl downloadUrl(url);

        if (!createSubAlbums(downloadUrl, info))
        {
            return false;
        }

        d->foldersToScan << downloadUrl.toLocalFile();

        if (downloadName.isEmpty())
        {
            downloadUrl = downloadUrl.adjusted(QUrl::StripTrailingSlash);
            downloadUrl.setPath(downloadUrl.path() + QLatin1Char('/') + (settings.file));
        }
        else
        {
            // when using custom renaming (e.g. by date, see bug 179902)
            // make sure that we create unique names
            downloadUrl = downloadUrl.adjusted(QUrl::StripTrailingSlash);
            downloadUrl.setPath(downloadUrl.path() + QLatin1Char('/') + (downloadName));
            QString suggestedPath = downloadUrl.toLocalFile();

            if (usedDownloadPaths.contains(suggestedPath))
            {
                QFileInfo fi(downloadName);
                QString suffix = QLatin1Char('.') + fi.suffix();
                QString pathWithoutSuffix(suggestedPath);
                pathWithoutSuffix.chop(suffix.length());
                QString currentVariant;
                int counter = 1;

                do
                {
                    currentVariant = pathWithoutSuffix + QLatin1Char('-') + QString::number(counter++) + suffix;
                }
                while (usedDownloadPaths.contains(currentVariant));

                usedDownloadPaths << currentVariant;
                downloadUrl = QUrl::fromLocalFile(currentVariant);
            }
            else
            {
                usedDownloadPaths << suggestedPath;
            }
        }

        settings.dest = downloadUrl.toLocalFile();
        allItems.append(settings);

        if (settings.autoRotate && settings.mime == QLatin1String("image/jpeg"))
        {
            d->autoRotateItemsList << downloadUrl.toLocalFile();
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "autorotating for " << downloadUrl;
        }

        ++downloadedItems;
    }

    if (downloadedItems <= 0)
    {
        return false;
    }

    d->lastDestURL = url;
    d->statusProgressBar->setNotify(true);
    d->statusProgressBar->setProgressValue(0);
    d->statusProgressBar->setProgressTotalSteps(downloadedItems);
    d->statusProgressBar->setProgressBarMode(StatusProgressBar::ProgressBarMode);

    // enable cancel action.
    d->cameraCancelAction->setEnabled(true);

    // disable settings tab here instead of slotBusy:
    // Only needs to be disabled while downloading
    d->advBox->setEnabled(false);
    d->view->setEnabled(false);

    d->deleteAfter = deleteAfter;

    d->controller->download(allItems);

    return true;
}

bool ImportUI::createSubAlbums(QUrl& downloadUrl, const CamItemInfo& info)
{
    bool success = true;

    if (d->albumCustomizer->autoAlbumDateEnabled())
    {
        success &= createDateBasedSubAlbum(downloadUrl, info);
    }
    if (d->albumCustomizer->autoAlbumExtEnabled())
    {
        success &= createExtBasedSubAlbum(downloadUrl, info);
    }

    return success;
}

bool ImportUI::createSubAlbum(QUrl& downloadUrl, const QString& subalbum, const QDate& date)
{
    QString errMsg;

    if (!createAutoAlbum(downloadUrl, subalbum, date, errMsg))
    {
        QMessageBox::critical(this, qApp->applicationName(), errMsg);
        return false;
    }

    downloadUrl = downloadUrl.adjusted(QUrl::StripTrailingSlash);
    downloadUrl.setPath(downloadUrl.path() + QLatin1Char('/') + (subalbum));
    return true;
}

bool ImportUI::createDateBasedSubAlbum(QUrl& downloadUrl, const CamItemInfo& info)
{
    QString dirName;
    QDateTime dateTime = info.ctime;

    switch (d->albumCustomizer->folderDateFormat())
    {
        case AlbumCustomizer::TextDateFormat:
            dirName = dateTime.date().toString(Qt::TextDate);
            break;

        case AlbumCustomizer::LocalDateFormat:
            dirName = dateTime.date().toString(Qt::LocalDate);
            break;

        case AlbumCustomizer::IsoDateFormat:
            dirName = dateTime.date().toString(Qt::ISODate);
            break;

        default:        // Custom
            dirName = dateTime.date().toString(d->albumCustomizer->customDateFormat());
            break;
    }

    return createSubAlbum(downloadUrl, dirName, dateTime.date());
}

bool ImportUI::createExtBasedSubAlbum(QUrl& downloadUrl, const CamItemInfo& info)
{
    // We use the target file name to compute sub-albums name to take a care about
    // conversion on the fly option.
    QFileInfo fi(info.downloadName.isEmpty()
                 ? info.name
                 : info.downloadName);

    QString subAlbum = fi.suffix().toUpper();

    if (fi.suffix().toUpper() == QLatin1String("JPEG") ||
        fi.suffix().toUpper() == QLatin1String("JPE"))
    {
        subAlbum = QLatin1String("JPG");
    }

    if (fi.suffix().toUpper() == QLatin1String("TIFF"))
    {
        subAlbum = QLatin1String("TIF");
    }

    if (fi.suffix().toUpper() == QLatin1String("MPEG") ||
        fi.suffix().toUpper() == QLatin1String("MPE") ||
        fi.suffix().toUpper() == QLatin1String("MPO"))
    {
        subAlbum = QLatin1String("MPG");
    }

    return createSubAlbum(downloadUrl, subAlbum, info.ctime.date());
}

void ImportUI::slotDeleteNew()
{
    slotSelectNew();
    QTimer::singleShot(0, this, SLOT(slotDeleteSelected()));
}

void ImportUI::slotDeleteSelected()
{
    deleteItems(true, false);
}

void ImportUI::slotDeleteAll()
{
    deleteItems(false, false);
}

void ImportUI::slotDeleted(const QString& folder, const QString& file, bool status)
{
    if (status)
    {
        // do this after removeItem.
        d->currentlyDeleting.removeAll(folder + file);
    }

    int curr = d->statusProgressBar->progressValue();
    d->statusProgressBar->setProgressTotalSteps(curr + 1);
    refreshFreeSpace();
}

void ImportUI::slotMetadata(const QString& folder, const QString& file, const DMetadata& meta)
{
    CamItemInfo info = d->view->camItemInfo(folder, file);

    if (!info.isNull())
    {
        d->rightSideBar->itemChanged(info, meta);
    }
}

void ImportUI::slotNewSelection(bool hasSelection)
{
    updateActions();

    QList<ParseSettings> renameFiles;
    CamItemInfoList list = hasSelection ? d->view->selectedCamItemInfos() : d->view->allItems();

    foreach(const CamItemInfo& info, list)
    {
        ParseSettings parseSettings;

        parseSettings.fileUrl      = info.url();
        parseSettings.creationTime = info.ctime;
        renameFiles.append(parseSettings);
    }

    d->renameCustomizer->renameManager()->reset();
    d->renameCustomizer->renameManager()->addFiles(renameFiles);
    d->renameCustomizer->renameManager()->parseFiles();

    slotUpdateDownloadName();

    unsigned long fSize = 0;
    unsigned long dSize = 0;
    itemsSelectionSizeInfo(fSize, dSize);
    d->albumLibraryFreeSpace->setEstimatedDSizeKb(dSize);
}

void ImportUI::slotImageSelected(const CamItemInfoList& selection, const CamItemInfoList& listAll)
{
    if (d->cameraCancelAction->isEnabled())
    {
        return;
    }

    int num_images = listAll.count();

    switch (selection.count())
    {
        case 0:
        {
            d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                                     i18ncp("@info:status",
                                                            "No item selected (%1 item)",
                                                            "No item selected (%1 items)",
                                                            num_images));

            d->rightSideBar->slotNoCurrentItem();
            break;
        }
        case 1:
        {
            // if selected item is in the list of item which will be deleted, set no current item
            if (!d->currentlyDeleting.contains(selection.first().folder + selection.first().name))
            {
                updateRightSideBar(selection.first());

                int index = listAll.indexOf(selection.first()) + 1;

                d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                                         i18nc("@info:status Filename of first selected item of number of items",
                                                               "<b>%1</b> (%2 of %3)",
                                                               selection.first().url().fileName(), index, num_images));
            }
            else
            {
                d->rightSideBar->slotNoCurrentItem();
                d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                                         i18ncp("@info:status",
                                                                "No item selected (%1 item)",
                                                                "No item selected (%1 items)",
                                                                num_images));
            }

            break;
        }
        default:
        {
            d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                                     i18ncp("@info:status", "%2/%1 item selected",
                                                            "%2/%1 items selected",
                                                            num_images, selection.count()));
            break;
        }
    }

    slotNewSelection(d->view->selectedCamItemInfos().count() > 0);
}

void ImportUI::updateRightSideBar(const CamItemInfo& info)
{
    d->rightSideBar->itemChanged(info, DMetadata());

    if (!d->busy)
    {
        d->controller->getMetadata(info.folder, info.name);
    }
}

QString ImportUI::identifyCategoryforMime(const QString& mime)
{
    return mime.split(QLatin1Char('/')).at(0);
}

void ImportUI::autoRotateItems()
{
    if (d->statusProgressBar->progressValue() != d->statusProgressBar->progressTotalSteps())
    {
        return;
    }

    if (d->autoRotateItemsList.isEmpty())
    {
        return;
    }

    qlonglong         id;
    ImageInfoList     list;
    CollectionScanner scanner;

    ScanController::instance()->suspendCollectionScan();

    foreach (const QString& downloadUrl, d->autoRotateItemsList)
    {
        //TODO: Needs test for Gphoto items.
        // make ImageInfo up to date
        id = scanner.scanFile(downloadUrl, CollectionScanner::NormalScan);
        list << ImageInfo(id);
    }

    FileActionMngr::instance()->transform(list, MetaEngineRotation::NoTransformation);

    ScanController::instance()->resumeCollectionScan();

    d->autoRotateItemsList.clear();
}

bool ImportUI::createAutoAlbum(const QUrl& parentURL, const QString& sub,
                               const QDate& date, QString& errMsg) const
{
    QUrl url(parentURL);
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + (sub));

    // first stat to see if the album exists
    QFileInfo info(url.toLocalFile());

    if (info.exists())
    {
        // now check if its really a directory
        if (info.isDir())
        {
            return true;
        }
        else
        {
            errMsg = i18nc("@info", "A file with the same name (<b>%1</b>) already exists in folder <resource>%2</resource>.",
                           sub, QDir::toNativeSeparators(parentURL.toLocalFile()));
            return false;
        }
    }

    // looks like the directory does not exist, try to create it.
    // First we make sure that the parent exists.
    PAlbum* parent = AlbumManager::instance()->findPAlbum(parentURL);

    if (!parent)
    {
        errMsg = i18nc("@info", "Failed to find Album for path <b>%1</b>.", QDir::toNativeSeparators(parentURL.toLocalFile()));
        return false;
    }

    // Create the album, with any parent albums required for the structure
    QUrl albumUrl(parentURL);

    foreach (const QString& folder, sub.split(QLatin1Char('/'), QString::SkipEmptyParts))
    {
        albumUrl = albumUrl.adjusted(QUrl::StripTrailingSlash);
        albumUrl.setPath(albumUrl.path() + QLatin1Char('/') + (folder));

        PAlbum* album = AlbumManager::instance()->findPAlbum(albumUrl);

        if (!album)
        {
            album = AlbumManager::instance()->createPAlbum(parent, folder, QString(), date, QString(), errMsg);

            if (!album)
            {
                return false;
            }
        }

        parent = album;
    }

    return true;
}

void ImportUI::slotSetup()
{
    Setup::execDialog(this);
}

void ImportUI::slotCameraFreeSpaceInfo(unsigned long kBSize, unsigned long kBAvail)
{
    d->cameraFreeSpace->addInformation(kBSize, kBSize - kBAvail, kBAvail, QString());
}

bool ImportUI::cameraDeleteSupport() const
{
    return d->controller->cameraDeleteSupport();
}

bool ImportUI::cameraUploadSupport() const
{
    return d->controller->cameraUploadSupport();
}

bool ImportUI::cameraMkDirSupport() const
{
    return d->controller->cameraMkDirSupport();
}

bool ImportUI::cameraDelDirSupport() const
{
    return d->controller->cameraDelDirSupport();
}

bool ImportUI::cameraUseUMSDriver() const
{
    return d->controller->cameraDriverType() == DKCamera::UMSDriver;
}

bool ImportUI::cameraUseGPhotoDriver() const
{
    return d->controller->cameraDriverType() == DKCamera::GPhotoDriver;
}

void ImportUI::enableZoomPlusAction(bool val)
{
    d->increaseThumbsAction->setEnabled(val);
}

void ImportUI::enableZoomMinusAction(bool val)
{
    d->decreaseThumbsAction->setEnabled(val);
}

void ImportUI::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void ImportUI::slotDBStat()
{
    showDigikamDatabaseStat();
}

void ImportUI::refreshCollectionFreeSpace()
{
    d->albumLibraryFreeSpace->setPaths(CollectionManager::instance()->allAvailableAlbumRootPaths());
}

void ImportUI::slotCollectionLocationStatusChanged(const CollectionLocation&, int)
{
    refreshCollectionFreeSpace();
}

void ImportUI::slotToggleShowBar()
{
    showThumbBar(d->showBarAction->isChecked());
}

void ImportUI::slotLogMsg(const QString& msg, DHistoryView::EntryType type,
                          const QString& folder, const QString& file)
{
    d->statusProgressBar->setProgressText(msg);
    QStringList meta;
    meta << folder << file;
    d->historyView->addEntry(msg, type, QVariant(meta));
}

void ImportUI::slotShowLog()
{
    d->showLogAction->isChecked() ? d->historyView->show() : d->historyView->hide();
}

void ImportUI::slotHistoryEntryClicked(const QVariant& metadata)
{
    QStringList meta = metadata.toStringList();
    QString folder   = meta.at(0);
    QString file     = meta.at(1);
    d->view->scrollTo(folder, file);
}

void ImportUI::showSideBars(bool visible)
{
    visible ? d->rightSideBar->restore()
            : d->rightSideBar->backup();
}

void ImportUI::slotToggleRightSideBar()
{
    d->rightSideBar->isExpanded() ? d->rightSideBar->shrink()
                                  : d->rightSideBar->expand();
}

void ImportUI::slotPreviousRightSideBarTab()
{
    d->rightSideBar->activePreviousTab();
}

void ImportUI::slotNextRightSideBarTab()
{
    d->rightSideBar->activeNextTab();
}

void ImportUI::showThumbBar(bool visible)
{
    d->view->toggleShowBar(visible);
}

bool ImportUI::thumbbarVisibility() const
{
    return d->showBarAction->isChecked();
}

void ImportUI::slotSwitchedToPreview()
{
    d->zoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    d->imageViewSelectionAction->setCurrentAction(d->camItemPreviewAction);
    toogleShowBar();
}

void ImportUI::slotSwitchedToIconView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    d->imageViewSelectionAction->setCurrentAction(d->iconViewAction);
    toogleShowBar();
}

void ImportUI::slotSwitchedToMapView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
#ifdef HAVE_MARBLE
    d->imageViewSelectionAction->setCurrentAction(d->mapViewAction);
#endif // HAVE_MARBLE
    toogleShowBar();
}

void ImportUI::customizedFullScreenMode(bool set)
{
    toolBarMenuAction()->setEnabled(!set);
    showMenuBarAction()->setEnabled(!set);
    showStatusBarAction()->setEnabled(!set);
    set ? d->showBarAction->setEnabled(false)
        : toogleShowBar();

    d->view->toggleFullScreen(set);
}

void ImportUI::toogleShowBar()
{
    switch (d->view->viewMode())
    {
        case ImportStackedView::PreviewImageMode:
        case ImportStackedView::MediaPlayerMode:
            d->showBarAction->setEnabled(true);
            break;

        default:
            d->showBarAction->setEnabled(false);
            break;
    }
}

void ImportUI::slotSetupChanged()
{
    d->view->importFilterModel()->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
    // Load full-screen options
    KConfigGroup group = KSharedConfig::openConfig()->group(ApplicationSettings::instance()->generalConfigGroupName());
    readFullScreenSettings(group);

    d->view->applySettings();
    sidebarTabTitleStyleChanged();
}

void ImportUI::sidebarTabTitleStyleChanged()
{
    d->rightSideBar->setStyle(ApplicationSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->applySettings();
}

void ImportUI::slotToggleColorManagedView()
{
    if (!IccSettings::instance()->isEnabled())
    {
        return;
    }

    bool cmv = !IccSettings::instance()->settings().useManagedPreviews;
    IccSettings::instance()->setUseManagedPreviews(cmv);
    d->camThumbsCtrl->clearCache();
}

void ImportUI::slotColorManagementOptionsChanged()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    d->viewCMViewAction->blockSignals(true);
    d->viewCMViewAction->setEnabled(settings.enableCM);
    d->viewCMViewAction->setChecked(settings.useManagedPreviews);
    d->viewCMViewAction->blockSignals(false);
}

} // namespace Digikam
