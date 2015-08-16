/* ============================================================
*
* This file is a part of digiKam project
* http://www.digikam.org
*
* Date        : 2004-09-16
* Description : Import tool interface
*
* Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
* Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "importui.moc"
#include "importui_p.h"

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
#include <QSignalMapper>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

// KDE includes

#include <kactioncollection.h>
#include <kapplication.h>
#include <kcalendarsystem.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <kinputdialog.h>
#include <kio/global.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <knotifyconfigwidget.h>
#include <kshortcutsdialog.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <ktogglefullscreenaction.h>
#include <ktoolbar.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>
#include <kvbox.h>
#include <KMessageWidget>

// Libkdcraw includes

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/rexpanderbox.h>
#include <libkdcraw/version.h>

// Local includes

#include "advancedrenamemanager.h"
#include "album.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "albumselectdialog.h"
#include "cameracontroller.h"
#include "camerafolderdialog.h"
#include "cameramessagebox.h"
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
#include "downloadhistory.h"
#include "dzoombar.h"
#include "fileactionmngr.h"
#include "freespacewidget.h"
#include "iccsettings.h"
#include "imagepropertiessidebarcamgui.h"
#include "importsettings.h"
#include "importview.h"
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
#include "uifilevalidator.h"
#include "importthumbnailmodel.h"

using namespace KDcrawIface;

namespace Digikam
{

ImportUI* ImportUI::m_instance = 0;

ImportUI::ImportUI(QWidget* const parent, const QString& cameraTitle,
                   const QString& model, const QString& port,
                   const QString& path, int startIndex)
    : DXmlGuiWindow(parent), d(new Private)
{
    setXMLFile("importui.rc");
    setFullScreenOptions(FS_IMPORTUI);

    // --------------------------------------------------------

    UiFileValidator validator(localXMLFile());

    if (!validator.isValid())
    {
        validator.fixConfigFile();
    }

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
    setAutoSaveSettings("Camera Settings", true);

    // -------------------------------------------------------------------

    d->historyUpdater = new CameraHistoryUpdater(this);

    //connect (d->historyUpdater, SIGNAL(signalHistoryMap(CHUpdateItemMap)),
    //this, SLOT(slotRefreshIconView(CHUpdateItemMap)));

    connect(d->historyUpdater, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

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
    KHBox* const widget = new KHBox(this);
    d->splitter         = new SidebarSplitter(widget);
    KVBox* const vbox   = new KVBox(d->splitter);
    d->view             = new ImportView(this, vbox);
    d->view->importFilterModel()->setCameraThumbsController(d->camThumbsCtrl);
    d->historyView      = new DHistoryView(vbox);
    d->rightSideBar     = new ImagePropertiesSideBarCamGui(widget, d->splitter, KMultiTabBar::Right, true);
    d->rightSideBar->setObjectName("CameraGui Sidebar Right");
    d->splitter->setFrameStyle(QFrame::NoFrame);
    d->splitter->setFrameShadow(QFrame::Plain);
    d->splitter->setFrameShape(QFrame::NoFrame);
    d->splitter->setOpaqueResize(false);
    d->splitter->setStretchFactor(0, 10);      // set iconview default size to max.

    vbox->setStretchFactor(d->view, 10);
    vbox->setStretchFactor(d->historyView,  2);
    vbox->setMargin(0);
    vbox->setSpacing(0);

    d->errorWidget = new KMessageWidget(vbox);
    d->errorWidget->setMessageType(KMessageWidget::Error);
    d->errorWidget->setCloseButtonVisible(false);
    d->errorWidget->hide();

    // -------------------------------------------------------------------------

    d->advBox = new RExpanderBox(d->rightSideBar);
    d->advBox->setObjectName("Camera Settings Expander");

    d->renameCustomizer = new RenameCustomizer(d->advBox, d->cameraTitle);
    d->renameCustomizer->setWhatsThis(i18n("Set how digiKam will rename files as they are downloaded."));
    //d->view->setRenameCustomizer(d->renameCustomizer);
    d->advBox->addItem(d->renameCustomizer, SmallIcon("insert-image"), i18n("File Renaming Options"),
                       QString("RenameCustomizer"), true);

    // -- Albums Auto-creation options -----------------------------------------

    d->albumCustomizer = new AlbumCustomizer(d->advBox);
    d->advBox->addItem(d->albumCustomizer, SmallIcon("folder-new"), i18n("Auto-creation of Albums"),
                       QString("AlbumBox"), false);

    // -- On the Fly options ---------------------------------------------------

    d->advancedSettings = new AdvancedSettings(d->advBox);
    d->advBox->addItem(d->advancedSettings, SmallIcon("system-run"), i18n("On the Fly Operations (JPEG only)"),
                       QString("OnFlyBox"), true);

    // -- Scripting options ---------------------------------------------------

    d->scriptingSettings = new ScriptingSettings(d->advBox);
    d->advBox->addItem(d->scriptingSettings, SmallIcon("utilities-terminal"), i18n("Scripting"),
                       QString("ScriptingBox"), true);
    d->advBox->addStretch();

    d->rightSideBar->appendTab(d->advBox, SmallIcon("configure"), i18n("Settings"));
    d->rightSideBar->loadState();

    // -------------------------------------------------------------------------

    setCentralWidget(widget);
}

void ImportUI::setupActions()
{
    d->cameraActions = new QActionGroup(this);

    // -- File menu ----------------------------------------------------

    d->cameraCancelAction = new KAction(KIcon("process-stop"), i18nc("@action Cancel process", "Cancel"), this);
    connect(d->cameraCancelAction, SIGNAL(triggered()), this, SLOT(slotCancelButton()));
    actionCollection()->addAction("importui_cancelprocess", d->cameraCancelAction);
    d->cameraCancelAction->setEnabled(false);

    // -----------------------------------------------------------------

    d->cameraInfoAction = new KAction(KIcon("camera-photo"), i18nc("@action Information about camera", "Information"), this);
    connect(d->cameraInfoAction, SIGNAL(triggered()), this, SLOT(slotInformation()));
    actionCollection()->addAction("importui_info", d->cameraInfoAction);
    d->cameraActions->addAction(d->cameraInfoAction);

    // -----------------------------------------------------------------

    d->cameraCaptureAction = new KAction(KIcon("webcamreceive"), i18nc("@action Capture photo from camera", "Capture"), this);
    connect(d->cameraCaptureAction, SIGNAL(triggered()), this, SLOT(slotCapture()));
    actionCollection()->addAction("importui_capture", d->cameraCaptureAction);
    d->cameraActions->addAction(d->cameraCaptureAction);

    // -----------------------------------------------------------------

    KAction* const closeAction = KStandardAction::close(this, SLOT(close()), this);
    actionCollection()->addAction("importui_close", closeAction);

    // -- Edit menu ----------------------------------------------------

    d->selectAllAction = new KAction(i18nc("@action:inmenu", "Select All"), this);
    d->selectAllAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_A));
    connect(d->selectAllAction, SIGNAL(triggered()), d->view, SLOT(slotSelectAll()));
    actionCollection()->addAction("importui_selectall", d->selectAllAction);
    d->cameraActions->addAction(d->selectAllAction);

    // -----------------------------------------------------------------

    d->selectNoneAction = new KAction(i18nc("@action:inmenu", "Select None"), this);
    d->selectNoneAction->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_A));
    connect(d->selectNoneAction, SIGNAL(triggered()), d->view, SLOT(slotSelectNone()));
    actionCollection()->addAction("importui_selectnone", d->selectNoneAction);
    d->cameraActions->addAction(d->selectNoneAction);

    // -----------------------------------------------------------------

    d->selectInvertAction = new KAction(i18nc("@action:inmenu", "Invert Selection"), this);
    d->selectInvertAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_Asterisk));
    connect(d->selectInvertAction, SIGNAL(triggered()), d->view, SLOT(slotSelectInvert()));
    actionCollection()->addAction("importui_selectinvert", d->selectInvertAction);
    d->cameraActions->addAction(d->selectInvertAction);

    // -----------------------------------------------------------

    d->selectNewItemsAction = new KAction(KIcon("favorites"), i18nc("@action:inmenu", "Select New Items"), this);
    connect(d->selectNewItemsAction, SIGNAL(triggered()), this, SLOT(slotSelectNew()));
    actionCollection()->addAction("importui_selectnewitems", d->selectNewItemsAction);
    d->cameraActions->addAction(d->selectNewItemsAction);

    // -----------------------------------------------------------

    d->selectLockedItemsAction = new KAction(KIcon("object-locked"), i18nc("@action:inmenu", "Select Locked Items"), this);
    d->selectLockedItemsAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_L));
    connect(d->selectLockedItemsAction, SIGNAL(triggered()), this, SLOT(slotSelectLocked()));
    actionCollection()->addAction("importui_selectlockeditems", d->selectLockedItemsAction);
    d->cameraActions->addAction(d->selectLockedItemsAction);

    // --- Download actions ----------------------------------------------------

    d->downloadAction = new KActionMenu(KIcon("document-save"), i18nc("@title:menu", "Download"), this);
    d->downloadAction->setDelayed(false);
    actionCollection()->addAction("importui_imagedownload", d->downloadAction);
    d->cameraActions->addAction(d->downloadAction);

    d->downloadNewAction = new KAction(KIcon("favorites"), i18nc("@action", "Download New"), this);
    d->downloadNewAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_N));
    connect(d->downloadNewAction, SIGNAL(triggered()), this, SLOT(slotDownloadNew()));
    actionCollection()->addAction("importui_imagedownloadnew", d->downloadNewAction);
    d->downloadAction->addAction(d->downloadNewAction);
    d->cameraActions->addAction(d->downloadNewAction);

    d->downloadSelectedAction = new KAction(KIcon("document-save"), i18nc("@action", "Download Selected"), this);
    connect(d->downloadSelectedAction, SIGNAL(triggered()), this, SLOT(slotDownloadSelected()));
    actionCollection()->addAction("importui_imagedownloadselected", d->downloadSelectedAction);
    d->downloadSelectedAction->setEnabled(false);
    d->downloadAction->addAction(d->downloadSelectedAction);
    d->cameraActions->addAction(d->downloadSelectedAction);

    d->downloadAllAction = new KAction(KIcon("document-save"), i18nc("@action", "Download All"), this);
    connect(d->downloadAllAction, SIGNAL(triggered()), this, SLOT(slotDownloadAll()));
    actionCollection()->addAction("importui_imagedownloadall", d->downloadAllAction);
    d->downloadAction->addAction(d->downloadAllAction);
    d->cameraActions->addAction(d->downloadAllAction);

    // -------------------------------------------------------------------------

    d->downloadDelNewAction = new KAction(i18nc("@action", "Download && Delete New"), this);
    d->downloadDelNewAction->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    connect(d->downloadDelNewAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteNew()));
    actionCollection()->addAction("importui_imagedownloaddeletenew", d->downloadDelNewAction);
    d->cameraActions->addAction(d->downloadDelNewAction);

    // -----------------------------------------------------------------

    d->downloadDelSelectedAction = new KAction(i18nc("@action", "Download && Delete Selected"), this);
    connect(d->downloadDelSelectedAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteSelected()));
    actionCollection()->addAction("importui_imagedownloaddeleteselected", d->downloadDelSelectedAction);
    d->downloadDelSelectedAction->setEnabled(false);
    d->cameraActions->addAction(d->downloadDelSelectedAction);

    // -------------------------------------------------------------------------

    d->downloadDelAllAction = new KAction(i18nc("@action", "Download && Delete All"), this);
    connect(d->downloadDelAllAction, SIGNAL(triggered()), this, SLOT(slotDownloadAndDeleteAll()));
    actionCollection()->addAction("importui_imagedownloaddeleteall", d->downloadDelAllAction);
    d->cameraActions->addAction(d->downloadDelAllAction);

    // -------------------------------------------------------------------------

    d->uploadAction = new KAction(KIcon("media-flash-smart-media"), i18nc("@action", "Upload..."), this);
    d->uploadAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_U));
    connect(d->uploadAction, SIGNAL(triggered()), this, SLOT(slotUpload()));
    actionCollection()->addAction("importui_imageupload", d->uploadAction);
    d->cameraActions->addAction(d->uploadAction);

    // -------------------------------------------------------------------------

    d->lockAction = new KAction(KIcon("object-locked"), i18nc("@action", "Toggle Lock"), this);
    d->lockAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_L));
    connect(d->lockAction, SIGNAL(triggered()), this, SLOT(slotToggleLock()));
    actionCollection()->addAction("importui_imagelock", d->lockAction);
    d->cameraActions->addAction(d->lockAction);

    // -------------------------------------------------------------------------

    d->markAsDownloadedAction = new KAction(KIcon("dialog-ok"), i18nc("@action", "Mark as downloaded"), this);
    connect(d->markAsDownloadedAction, SIGNAL(triggered()), this, SLOT(slotMarkAsDownloaded()));
    actionCollection()->addAction("importui_imagemarkasdownloaded", d->markAsDownloadedAction);
    d->cameraActions->addAction(d->markAsDownloadedAction);

    // --- Delete actions ------------------------------------------------------

    d->deleteAction = new KActionMenu(KIcon("user-trash"), i18nc("@title:menu", "Delete"), this);
    d->deleteAction->setDelayed(false);
    actionCollection()->addAction("importui_delete", d->deleteAction);
    d->cameraActions->addAction(d->deleteAction);

    d->deleteSelectedAction = new KAction(KIcon("edit-delete"), i18nc("@action", "Delete Selected"), this);
    connect(d->deleteSelectedAction, SIGNAL(triggered()), this, SLOT(slotDeleteSelected()));
    actionCollection()->addAction("importui_imagedeleteselected", d->deleteSelectedAction);
    d->deleteSelectedAction->setShortcut(KShortcut(Qt::Key_Delete));
    d->deleteSelectedAction->setEnabled(false);
    d->deleteAction->addAction(d->deleteSelectedAction);
    d->cameraActions->addAction(d->deleteSelectedAction);

    d->deleteAllAction = new KAction(KIcon("edit-delete"), i18nc("@action", "Delete All"), this);
    connect(d->deleteAllAction, SIGNAL(triggered()), this, SLOT(slotDeleteAll()));
    actionCollection()->addAction("importui_imagedeleteall", d->deleteAllAction);
    d->deleteAction->addAction(d->deleteAllAction);
    d->cameraActions->addAction(d->deleteAllAction);

    d->deleteNewAction = new KAction(KIcon("edit-delete"), i18nc("@action", "Delete New"), this);
    connect(d->deleteNewAction, SIGNAL(triggered()), this, SLOT(slotDeleteNew()));
    actionCollection()->addAction("importui_imagedeletenew", d->deleteNewAction);
    d->deleteAction->addAction(d->deleteNewAction);
    d->cameraActions->addAction(d->deleteNewAction);

    // --- Icon view, items preview, and map actions ------------------------------------------------------

    d->imageViewSelectionAction = new KSelectAction(KIcon("viewimage"), i18nc("@title:group", "Views"), this);
    actionCollection()->addAction("importui_view_selection", d->imageViewSelectionAction);

    d->iconViewAction = new KToggleAction(KIcon("view-list-icons"),
                                          i18nc("@action Go to thumbnails (icon) view", "Thumbnails"), this);
    actionCollection()->addAction("importui_icon_view", d->iconViewAction);
    connect(d->iconViewAction, SIGNAL(triggered()), d->view, SLOT(slotIconView()));
    d->imageViewSelectionAction->addAction(d->iconViewAction);

    d->camItemPreviewAction = new KToggleAction(KIcon("viewimage"), i18nc("@action View the selected image", "Preview Item"), this);
    d->camItemPreviewAction->setShortcut(KShortcut(Qt::Key_F3));
    actionCollection()->addAction("importui_item_view", d->camItemPreviewAction);
    connect(d->camItemPreviewAction, SIGNAL(triggered()), d->view, SLOT(slotImagePreview()));
    d->imageViewSelectionAction->addAction(d->camItemPreviewAction);

#ifdef HAVE_KGEOMAP
    d->mapViewAction = new KToggleAction(KIcon("applications-internet"),
                                         i18nc("@action Switch to map view", "Map"), this);
    actionCollection()->addAction("importui_map_view", d->mapViewAction);
    connect(d->mapViewAction, SIGNAL(triggered()), d->view, SLOT(slotMapWidgetView()));
    d->imageViewSelectionAction->addAction(d->mapViewAction);
#endif // HAVE_KGEOMAP

    /// @todo Add table view stuff here

    // -- Item Sorting ------------------------------------------------------------

    d->itemSortAction                    = new KSelectAction(i18nc("@title:menu", "&Sort Items"), this);
    d->itemSortAction->setWhatsThis(i18nc("@info:whatsthis", "The value by which the items are sorted in the thumbnail view"));
    QSignalMapper* const imageSortMapper = new QSignalMapper(this);
    connect(imageSortMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSortImagesBy(int)));
    actionCollection()->addAction("item_sort", d->itemSortAction);

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

    imageSortMapper->setMapping(sortByNameAction, (int)CamItemSortSettings::SortByFileName);
    imageSortMapper->setMapping(sortByPathAction, (int)CamItemSortSettings::SortByFilePath);
    imageSortMapper->setMapping(sortByDateAction, (int)CamItemSortSettings::SortByCreationDate); //TODO: Implement sort by creation date.
    imageSortMapper->setMapping(sortByFileSizeAction, (int)CamItemSortSettings::SortByFileSize);
    imageSortMapper->setMapping(sortByRatingAction, (int)CamItemSortSettings::SortByRating);
    imageSortMapper->setMapping(sortByDownloadAction, (int)CamItemSortSettings::SortByDownloadState);

    d->itemSortAction->setCurrentItem(ImportSettings::instance()->getImageSortBy());

    // -- Item Sort Order ------------------------------------------------------------

    d->itemSortOrderAction                    = new KSelectAction(i18nc("@title:inmenu", "Item Sorting &Order"), this);
    d->itemSortOrderAction->setWhatsThis(i18nc("@info:whatsthis", "Defines whether items are sorted in ascending or descending manner."));
    QSignalMapper* const imageSortOrderMapper = new QSignalMapper(this);
    connect(imageSortOrderMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSortImagesOrder(int)));
    actionCollection()->addAction("item_sort_order", d->itemSortOrderAction);

    QAction* const sortAscendingAction = d->itemSortOrderAction->addAction(KIcon("view-sort-ascending"), i18nc("@item:inmenu Sorting Order", "Ascending"));
    QAction* const sortDescendingAction = d->itemSortOrderAction->addAction(KIcon("view-sort-descending"), i18nc("@item:inmenu Sorting Order", "Descending"));

    connect(sortAscendingAction,  SIGNAL(triggered()), imageSortOrderMapper, SLOT(map()));
    connect(sortDescendingAction, SIGNAL(triggered()), imageSortOrderMapper, SLOT(map()));

    imageSortOrderMapper->setMapping(sortAscendingAction, (int)CamItemSortSettings::AscendingOrder);
    imageSortOrderMapper->setMapping(sortDescendingAction, (int)CamItemSortSettings::DescendingOrder);

    d->itemSortOrderAction->setCurrentItem(ImportSettings::instance()->getImageSortOrder());

    // -- Item Grouping ------------------------------------------------------------

    d->itemsGroupAction                  = new KSelectAction(i18nc("@title:menu", "&Group Items"), this);
    d->itemsGroupAction->setWhatsThis(i18nc("@info:whatsthis", "The categories in which the items in the thumbnail view are displayed"));
    QSignalMapper* const itemGroupMapper = new QSignalMapper(this);
    connect(itemGroupMapper, SIGNAL(mapped(int)), d->view, SLOT(slotGroupImages(int)));
    actionCollection()->addAction("item_group", d->itemsGroupAction);

    // map to CamItemSortSettings enum
    QAction* const noCategoriesAction  = d->itemsGroupAction->addAction(i18nc("@item:inmenu Group Items", "Flat List"));
    QAction* const CategoryByFolderAction = d->itemsGroupAction->addAction(i18nc("@item:inmenu Group Items", "By Folder"));
    QAction* const groupByFormatAction = d->itemsGroupAction->addAction(i18nc("@item:inmenu Group Items", "By Format"));

    connect(noCategoriesAction,     SIGNAL(triggered()), itemGroupMapper, SLOT(map()));
    connect(CategoryByFolderAction, SIGNAL(triggered()), itemGroupMapper, SLOT(map()));
    connect(groupByFormatAction,    SIGNAL(triggered()), itemGroupMapper, SLOT(map()));

    itemGroupMapper->setMapping(noCategoriesAction,     (int)CamItemSortSettings::NoCategories);
    itemGroupMapper->setMapping(CategoryByFolderAction, (int)CamItemSortSettings::CategoryByFolder);
    itemGroupMapper->setMapping(groupByFormatAction,    (int)CamItemSortSettings::CategoryByFormat);

    d->itemsGroupAction->setCurrentItem(ImportSettings::instance()->getImageGroupMode());

    // -- Standard 'View' menu actions ---------------------------------------------

    d->increaseThumbsAction = KStandardAction::zoomIn(d->view, SLOT(slotZoomIn()), this);
    d->increaseThumbsAction->setEnabled(false);
    KShortcut keysPlus      = d->increaseThumbsAction->shortcut();
    keysPlus.setAlternate(Qt::Key_Plus);
    d->increaseThumbsAction->setShortcut(keysPlus);
    actionCollection()->addAction("importui_zoomplus", d->increaseThumbsAction);

    d->decreaseThumbsAction = KStandardAction::zoomOut(d->view, SLOT(slotZoomOut()), this);
    d->decreaseThumbsAction->setEnabled(false);
    KShortcut keysMinus     = d->decreaseThumbsAction->shortcut();
    keysMinus.setAlternate(Qt::Key_Minus);
    d->decreaseThumbsAction->setShortcut(keysMinus);
    actionCollection()->addAction("importui_zoomminus", d->decreaseThumbsAction);

    d->zoomFitToWindowAction = new KAction(KIcon("zoom-fit-best"), i18nc("@action:inmenu", "Fit to &Window"), this);
    d->zoomFitToWindowAction->setShortcut(KShortcut(Qt::ALT + Qt::CTRL + Qt::Key_E));
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), d->view, SLOT(slotFitToWindow()));
    actionCollection()->addAction("import_zoomfit2window", d->zoomFitToWindowAction);

    d->zoomTo100percents = new KAction(KIcon("zoom-original"), i18nc("@action:inmenu", "Zoom to 100%"), this);
    d->zoomTo100percents->setShortcut(KShortcut(Qt::CTRL + Qt::Key_Comma));
    connect(d->zoomTo100percents, SIGNAL(triggered()), d->view, SLOT(slotZoomTo100Percents()));
    actionCollection()->addAction("import_zoomto100percents", d->zoomTo100percents);

    // ------------------------------------------------------------------------------------------------

    d->viewCMViewAction = new KToggleAction(KIcon("video-display"), i18n("Color-Managed View"), this);
    d->viewCMViewAction->setShortcut(KShortcut(Qt::Key_F12));
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    actionCollection()->addAction("color_managed_view", d->viewCMViewAction);

    // ------------------------------------------------------------------------------------------------

    createFullScreenAction("importui_fullscreen");
    createSidebarActions();

    d->showLogAction = new KToggleAction(KIcon("view-history"), i18nc("@option:check", "Show History"), this);
    d->showLogAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_H));
    connect(d->showLogAction, SIGNAL(triggered()), this, SLOT(slotShowLog()));
    actionCollection()->addAction("importui_showlog", d->showLogAction);

    d->showBarAction = new KToggleAction(KIcon("view-choose"), i18nc("@option:check", "Show Thumbbar"), this);
    d->showBarAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_T));
    connect(d->showBarAction, SIGNAL(triggered()), this, SLOT(slotToggleShowBar()));
    actionCollection()->addAction("showthumbs", d->showBarAction);
    d->showBarAction->setEnabled(false);

    // -- Standard 'Configure' menu actions ----------------------------------------

    d->showMenuBarAction = KStandardAction::showMenubar(this,     SLOT(slotShowMenuBar()),       actionCollection());
    KStandardAction::keyBindings(this,                            SLOT(slotEditKeys()),          actionCollection());
    KStandardAction::configureToolbars(this,                      SLOT(slotConfToolbars()),      actionCollection());
    KStandardAction::configureNotifications(this,                 SLOT(slotConfNotifications()), actionCollection());
    d->showPreferencesAction = KStandardAction::preferences(this, SLOT(slotSetup()),             actionCollection());

    // ---------------------------------------------------------------------------------

    ThemeManager::instance()->registerThemeActions(this);

    // -- Standard 'Help' menu actions ---------------------------------------------

    createHelpActions();

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // -- Keyboard-only actions added to <MainWindow> ------------------------------

    KAction* const altBackwardAction = new KAction(i18nc("@action", "Previous Image"), this);
    actionCollection()->addAction("importui_backward_shift_space", altBackwardAction);
    altBackwardAction->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_Space));
    connect(altBackwardAction, SIGNAL(triggered()), d->view, SLOT(slotPrevItem()));

    // ---------------------------------------------------------------------------------

    d->connectAction = new KAction(KIcon("view-refresh"), i18nc("@action Connection failed, try again?", "Retry"), this);
    connect(d->connectAction, SIGNAL(triggered()), d->controller, SLOT(slotConnect()));

    createGUI(xmlFile());

    d->showMenuBarAction->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080
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

        foreach(CamItemInfo info, list)
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
    d->statusProgressBar->setNotificationTitle(d->cameraTitle, KIcon("camera-photo").pixmap(22));
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

    connect(d->controller, SIGNAL(signalFileList(CamItemInfoList)),
            this, SLOT(slotFileList(CamItemInfoList)));

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
    KAction* const escapeAction = new KAction(i18nc("@action", "Exit Preview Mode"), this);
    actionCollection()->addAction("exit_preview_mode", escapeAction);
    escapeAction->setShortcut( KShortcut(Qt::Key_Escape) );
    connect(escapeAction, SIGNAL(triggered()), this, SIGNAL(signalEscapePressed()));

    KAction* const nextImageAction = new KAction(i18nc("@action","Next Image"), this);
    nextImageAction->setIcon(SmallIcon("go-next"));
    actionCollection()->addAction("next_image", nextImageAction);
    nextImageAction->setShortcut(KShortcut(Qt::Key_Space));
    connect(nextImageAction, SIGNAL(triggered()), d->view, SLOT(slotNextItem()));

    KAction* const previousImageAction = new KAction(i18nc("@action", "Previous Image"), this);
    previousImageAction->setIcon(SmallIcon("go-previous"));
    actionCollection()->addAction("previous_image", previousImageAction);
    previousImageAction->setShortcut(KShortcut(Qt::Key_Backspace));
    connect(previousImageAction, SIGNAL(triggered()), d->view, SLOT(slotPrevItem()));

    KAction* const altpreviousImageAction = new KAction(i18nc("@action", "Previous Image"), this);
    actionCollection()->addAction("alt_previous_image_shift_space", altpreviousImageAction);
    altpreviousImageAction->setShortcut( KShortcut(Qt::SHIFT+Qt::Key_Space) );
    connect(altpreviousImageAction, SIGNAL(triggered()), d->view, SLOT(slotPrevItem()));

    KAction* const firstImageAction = new KAction(i18nc("@action Go to first image", "First Image"), this);
    actionCollection()->addAction("first_image", firstImageAction);
    firstImageAction->setShortcut(KShortcut(Qt::Key_Home) );
    connect(firstImageAction, SIGNAL(triggered()), d->view, SLOT(slotFirstItem()));

    KAction* const lastImageAction = new KAction(i18nc("@action Go to last image", "Last Image"), this);
    actionCollection()->addAction("last_image", lastImageAction);
    lastImageAction->setShortcut(KShortcut(Qt::Key_End) );
    connect(lastImageAction, SIGNAL(triggered()), d->view, SLOT(slotLastItem()));
}

void ImportUI::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    readFullScreenSettings(group);

    d->showBarAction->setChecked(ImportSettings::instance()->getShowThumbbar());
    d->showLogAction->setChecked(group.readEntry("ShowLog",               false));
    d->albumCustomizer->readSettings(group);
    d->advancedSettings->readSettings(group);
    d->scriptingSettings->readSettings(group);

#if KDCRAW_VERSION >= 0x020000
    d->advBox->readSettings(group);
#else
    d->advBox->readSettings();
#endif

    d->splitter->restoreState(group);

    slotShowLog();
}

void ImportUI::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    ImportSettings::instance()->setShowThumbbar(d->showBarAction->isChecked());
    ImportSettings::instance()->saveSettings();
    group.writeEntry("ShowLog",        d->showLogAction->isChecked());
    d->albumCustomizer->saveSettings(group);
    d->advancedSettings->saveSettings(group);
    d->scriptingSettings->saveSettings(group);

#if KDCRAW_VERSION >= 0x020000
    d->advBox->writeSettings(group);
#else
    d->advBox->writeSettings();
#endif

    d->rightSideBar->saveState();
    d->splitter->saveState(group);
    d->filterComboBox->saveSettings();

    config->sync();
}

void ImportUI::slotProcessUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
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
    d->scriptingSettings->settings(&settings);
    return settings;
}

void ImportUI::setInitialSorting()
{
    d->view->slotGroupImages(ImportSettings::instance()->getImageGroupMode());
    d->view->slotSortImagesBy(ImportSettings::instance()->getImageSortBy());
    d->view->slotSortImagesOrder(ImportSettings::instance()->getImageSortOrder());
}

void ImportUI::slotCancelButton()
{
    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                                          i18nc("@info:status", "Canceling current operation, please wait..."));
    d->controller->slotCancel();
    d->historyUpdater->slotCancel();
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
    /*FIXME
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
        if (KMessageBox::questionYesNo(this,
                                       i18nc("@info", "Do you want to close the dialog "
                                            "and cancel the current operation?"))
                == KMessageBox::No)
        {
            return false;
        }
    }

    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
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

        d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
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
        d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, i18nc("@info:status", "Ready"));

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
        KSharedConfig::Ptr config = KGlobal::config();
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

    KSharedConfig::Ptr config = KGlobal::config();
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

// FIXME d->filesToBeAdded seems to be unused...
void ImportUI::slotFileList(const CamItemInfoList& fileList)
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

    QString fileformats;

    QStringList patternList = KImageIO::pattern(KImageIO::Reading).split('\n');

    // All Images from list must been always the first entry given by KDE API
    QString allPictures = patternList.at(0);

    // Add RAW file format to All Images" type mime and replace current.
    allPictures.insert(allPictures.indexOf("|"), QString(KDcrawIface::KDcraw::rawFiles()));
    patternList.removeAll(patternList.at(0));
    patternList.prepend(allPictures);

    // Added RAW file formats supported by dcraw program like a type mime.
    // Note: we cannot use here "image/x-raw" type mime from KDE because it incomplete
    // or unavailable(dcraw_0)(see file #121242 in bug).
    patternList.append(QString("\n%1|Camera RAW files").arg(QString(KDcrawIface::KDcraw::rawFiles())));
    fileformats = patternList.join("\n");

    //kDebug() << "fileformats=" << fileformats;

    KUrl::List urls = KFileDialog::getOpenUrls(CollectionManager::instance()->oneAlbumRootPath(),
                                               fileformats, this, i18nc("@title:window", "Select Image to Upload"));

    if (!urls.isEmpty())
    {
        slotUploadItems(urls);
    }
}

void ImportUI::slotUploadItems(const KUrl::List& urls)
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
            totalKbSize += fi.size() / 1024;
        }

        if (totalKbSize >= d->cameraFreeSpace->kBAvail())
        {
            KMessageBox::error(this, i18nc("@info", "There is not enough free space on the Camera Medium "
                                          "to upload pictures.\n\n"
                                          "Space require: %1\n"
                                          "Available free space: %2",
                                          KIO::convertSizeFromKiB(totalKbSize),
                                          KIO::convertSizeFromKiB(d->cameraFreeSpace->kBAvail())));
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

        CamItemInfo uploadInfo;
        uploadInfo.folder = cameraFolder;
        uploadInfo.name   = name + ext;

        while (d->view->hasImage(uploadInfo))
        {
            QString msg(i18nc("@info", "Camera Folder <resource>%1</resource> already contains the item <resource>%2</resource>.<nl>"
                             "Please enter a new filename (without extension):",
                             cameraFolder, fi.fileName()));
            uploadInfo.name = KInputDialog::getText(i18nc("@title:window", "File already exists"), msg, name, &ok, this) + ext;

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
    slotNewSelection(d->view->selectedUrls().count() > 0);
    QTimer::singleShot(0, this, SLOT(slotDownloadSelected()));
}

void ImportUI::slotDownloadAndDeleteNew()
{
    slotSelectNew();
    slotNewSelection(d->view->selectedUrls().count() > 0);
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
        KMessageBox::information(this, i18nc("@info", "Your custom target album date format is not valid. Please check your settings..."));
        return;
    }

    // See bug #143934: force to select all items to prevent problem
    // when !renameCustomizer->useDefault() ==> iconItem->getDownloadName()
    // can return an empty string in this case because it depends on selection.
    if (!onlySelected)
    {
        d->view->slotSelectAll();
        slotNewSelection(d->view->selectedUrls().count() > 0);
    }

    // -- Get the destination album from digiKam library ---------------

    PAlbum* pAlbum = 0;

    if (!album)
    {
        AlbumManager* const man   = AlbumManager::instance();

        // Check if default target album option is enabled.

        KSharedConfig::Ptr config = KGlobal::config();
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
                    KMessageBox::information(this, i18nc("@info", "Collection which host your default target album set to process "
                                                        "download from camera device is not available. Please select another one from "
                                                        "camera configuration dialog."));
                    return;
                }
            }
            else
            {
                KMessageBox::information(this, i18nc("@info", "Your default target album set to process download "
                                                    "from camera device is not available. Please select another one from "
                                                    "camera configuration dialog."));
                return;
            }

            pAlbum = pa;
        }
        else
        {
            AlbumList list = man->currentAlbums();

            int albumId = 0;
            if (!list.isEmpty())
            {
                albumId = group.readEntry(d->configLastTargetAlbum, list.first()->globalID());
            }
            album          = man->findAlbum(albumId);

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
        kDebug() << "Destination Album is null";
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

            DownloadHistory::setDownloaded(d->controller->cameraMD5ID(),
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
                DNotificationWrapper("cameradownloaded", i18nc("@info Popup notification", "Images download finished, you can now detach your camera while the images will be auto-rotated"), this, windowTitle());
            }
            else
            {
                DNotificationWrapper("cameradownloaded", i18nc("@info Popup notification", "Images download finished"), this, windowTitle());
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

    foreach(CamItemInfo info, list)
    {
        setDownloaded(d->view->camItemInfoRef(info.folder, info.name), CamItemInfo::DownloadedYes);

        DownloadHistory::setDownloaded(d->controller->cameraMD5ID(),
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
        d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);
    }

    foreach(CamItemInfo info, list)
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
    KUrl::List selected       = d->view->selectedUrls();
    bool hasNoSelection       = selected.count() == 0;
    CamItemInfoList list      = d->view->allItems();
    DownloadSettings settings = downloadSettings();

    foreach (CamItemInfo info, list)
    {
        CamItemInfo& refInfo = d->view->camItemInfoRef(info.folder, info.name);
        kDebug() << "slotDownloadNameChanged, old: " << refInfo.downloadName;

        QString newName = info.name;

        if (hasNoSelection || selected.contains(info.url()))
        {
            if (d->renameCustomizer->useDefault())
            {
                newName = d->renameCustomizer->newName(info.name, info.ctime);
            }
            else if (d->renameCustomizer->isEnabled())
            {
                newName = d->renameCustomizer->newName(info.url().toLocalFile(), info.ctime);
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
        }

        refInfo.downloadName = newName;

        kDebug() << "slotDownloadNameChanged, new: " << refInfo.downloadName;
    }

    d->view->updateIconView();
}

//FIXME: the new pictures are marked by CameraHistoryUpdater which is not working yet.
void ImportUI::slotSelectNew()
{
    CamItemInfoList infos = d->view->allItems();
    CamItemInfoList toBeSelected;

    foreach (CamItemInfo info, infos)
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

    foreach(CamItemInfo info, allItems)
    {
        if(info.writePermissions == 0)
        {
            toBeSelected << info;
        }
    }

    d->view->setSelectedCamItemInfos(toBeSelected);
}

void ImportUI::toggleLock(CamItemInfo& info)
{
    if(!info.isNull())
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

    foreach(CamItemInfo info, infos)
    {
        path = info.folder;

        if (!path.isEmpty() && path.endsWith('/'))
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

    if(itemInfo.downloaded == CamItemInfo::DownloadStarted)
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

    KUrl::List selected       = d->view->selectedUrls();
    CamItemInfoList list      = d->view->allItems();
    DownloadSettings settings = downloadSettings();

    foreach (CamItemInfo info, list)
    {
        if (selected.contains(info.url()))
        {
            qint64 size = info.size;

            if (size < 0) // -1 if size is not provided by camera
            {
                continue;
            }

            fSize += size;

            if (info.mime == QString("image/jpeg"))
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

    foreach(CamItemInfo info, list)
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
        CameraMessageBox::informationList(d->camThumbsCtrl, this, infoMsg, lockedList, i18n("Information"));
    }

    if (folders.isEmpty())
    {
        return;
    }

    QString warnMsg(i18ncp("@info", "About to delete this image. "
                          "<warning>Deleted file is unrecoverable.</warning> "
                          "Are you sure?",
                          "About to delete these %1 images. "
                          "<warning>Deleted files are unrecoverable.</warning> "
                          "Are you sure?",
                          deleteList.count()));

    if (CameraMessageBox::warningContinueCancelList(d->camThumbsCtrl, 
                                                    this,
                                                    warnMsg,
                                                    deleteList,
                                                    i18n("Warning"),
                                                    KGuiItem(i18n("Delete")),
                                                    KStandardGuiItem::cancel(),
                                                    QString("DontAskAgainToDeleteItemsFromCamera"))
        ==  KMessageBox::Continue)
    {
        QStringList::const_iterator itFolder = folders.constBegin();
        QStringList::const_iterator itFile   = files.constBegin();

        d->statusProgressBar->setProgressValue(0);
        d->statusProgressBar->setProgressTotalSteps(deleteList.count());
        d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);

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
        KGuiItem cont   = KStandardGuiItem::cont();
        cont.setText(i18nc("@action:button Not enough disk-space warning", "Try Anyway"));
        KGuiItem cancel = KStandardGuiItem::cancel();
        cancel.setText(i18nc("@action:button Not enough disk-space warning", "Cancel Download"));
        int result     = KMessageBox::warningYesNo(this,
                                                   i18nc("@info", "There is not enough free space on the disk of the album you selected "
                                                         "to download and process the selected pictures from the camera.\n\n"
                                                         "Estimated space required: %1\n"
                                                         "Available free space: %2",
                                                         KIO::convertSizeFromKiB(dSize),
                                                         KIO::convertSizeFromKiB(kBAvail)),
                                                  i18nc("@title:window", "Insufficient Disk Space"),
                                                  cont, cancel);

        if (result == KMessageBox::No)
        {
            return false;
        }
    }
    return true;
}

bool ImportUI::downloadCameraItems(PAlbum* pAlbum, bool onlySelected, bool deleteAfter)
{
    d->controller->downloadPrep();

    QString              downloadName;
    QDateTime            dateTime;
    DownloadSettingsList allItems;
    DownloadSettings     settings = downloadSettings();
    KUrl url                      = pAlbum->fileUrl();
    int downloadedItems           = 0;

    // -- Download camera items -------------------------------

    KUrl::List selected  = d->view->selectedUrls();
    CamItemInfoList list = d->view->allItems();
    QSet<QString> usedDownloadPaths;

    foreach(CamItemInfo info, list)
    {
        if (onlySelected && !(selected.contains(info.url())))
        {
            continue;
        }

        settings.folder     = info.folder;
        settings.file       = info.name;
        settings.pickLabel  = info.pickLabel;
        settings.colorLabel = info.colorLabel;
        settings.rating     = info.rating;
        dateTime            = info.ctime;

        downloadName = info.downloadName; // downloadName should already be set by now

        KUrl downloadUrl(url);

        if (!createSubAlbums(downloadUrl, info))
        {
            return false;
        }

        d->foldersToScan << downloadUrl.toLocalFile();

        if (downloadName.isEmpty())
        {
            downloadUrl.addPath(settings.file);
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

        settings.dest = downloadUrl.toLocalFile();
        allItems.append(settings);

        if (settings.autoRotate)
        {
            d->autoRotateItemsList << downloadUrl.toLocalFile();
            kDebug() << "autorotating for " << downloadUrl;
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
    d->statusProgressBar->progressBarMode(StatusProgressBar::ProgressBarMode);

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

bool ImportUI::createSubAlbums(KUrl& downloadUrl, const CamItemInfo& info)
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

bool ImportUI::createSubAlbum(KUrl& downloadUrl, const QString& subalbum, const QDate& date)
{
    QString errMsg;

    if (!createAutoAlbum(downloadUrl, subalbum, date, errMsg))
    {
        KMessageBox::error(this, errMsg);
        return false;
    }

    downloadUrl.addPath(subalbum);
    return true;
}

bool ImportUI::createDateBasedSubAlbum(KUrl& downloadUrl, const CamItemInfo& info)
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

    // See bug #136927 : we need to support file system which do not
    // handle upper case properly.
    dirName = dirName.toLower();

    return createSubAlbum(downloadUrl, dirName, dateTime.date());
}

bool ImportUI::createExtBasedSubAlbum(KUrl& downloadUrl, const CamItemInfo& info)
{
    // We use the target file name to compute sub-albums name to take a care about
    // conversion on the fly option.
    QFileInfo fi(info.downloadName.isEmpty()
                 ? info.name
                 : info.downloadName);

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

    // See bug #136927 : we need to support file system which do not
    // handle upper case properly.
    subAlbum = subAlbum.toLower();

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

    foreach(CamItemInfo info, list)
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
            d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, i18ncp("@info:status", "No item selected (%1 item)",
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

                d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                                                      i18nc("@info:status Filename of first selected item of number of items",
                                                            "<filename>%1</filename> (%2 of %3)",
                                                            selection.first().url().fileName(), index, num_images));
            }
            else
            {
                d->rightSideBar->slotNoCurrentItem();
                d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                                                      i18ncp("@info:status", "No item selected (%1 item)",
                                                                             "No item selected (%1 items)",
                                                                             num_images));
            }

            break;
        }
        default:
        {
            d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, i18ncp("@info:status", "%2/%1 item selected",
                                                                                     "%2/%1 items selected",
                                                                                     num_images, selection.count()));
            break;
        }
    }
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
    return mime.split('/').at(0);
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

    FileActionMngr::instance()->transform(list, KExiv2Iface::RotationMatrix::NoTransformation);

    ScanController::instance()->resumeCollectionScan();

    d->autoRotateItemsList.clear();
}

bool ImportUI::createAutoAlbum(const KUrl& parentURL, const QString& sub,
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
            errMsg = i18nc("@info", "A file with the same name (<filename>%1</filename>) already exists in folder <resource>%2</resource>.",
                          sub, parentURL.toLocalFile());
            return false;
        }
    }

    // looks like the directory does not exist, try to create it.
    // First we make sure that the parent exists.
    PAlbum* parent = AlbumManager::instance()->findPAlbum(parentURL);

    if (!parent)
    {
        errMsg = i18nc("@info", "Failed to find Album for path <filename>%1</filename>.", parentURL.toLocalFile());
        return false;
    }

    // Create the album, with any parent albums required for the structure
    KUrl albumUrl(parentURL);
    foreach (const QString& folder, sub.split(QChar('/'), QString::SkipEmptyParts))
    {
        albumUrl.cd(folder);
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

void ImportUI::slotEditKeys()
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection(actionCollection(), i18nc("@title", "General"));
    dialog.configure();
}

void ImportUI::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group("Camera Settings"));
    KEditToolBar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void ImportUI::slotConfNotifications()
{
    KNotifyConfigWidget::configure(this);
}

void ImportUI::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group("Camera Settings"));
}

void ImportUI::slotSetup()
{
    Setup::exec(this);
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

void ImportUI::slotShowMenuBar()
{
    menuBar()->setVisible(d->showMenuBarAction->isChecked());
}

void ImportUI::slotLogMsg(const QString& msg, DHistoryView::EntryType type,
                          const QString& folder, const QString& file)
{
    d->statusProgressBar->setProgressText(msg);
    QStringList meta;
    meta << folder << file;
    d->historyView->addedEntry(msg, type, QVariant(meta));
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
#ifdef HAVE_KGEOMAP
    d->imageViewSelectionAction->setCurrentAction(d->mapViewAction);
#endif // HAVE_KGEOMAP
    toogleShowBar();
}

void ImportUI::customizedFullScreenMode(bool set)
{
    statusBarMenuAction()->setEnabled(!set);
    toolBarMenuAction()->setEnabled(!set);
    d->showMenuBarAction->setEnabled(!set);
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
    // Load full-screen options
    KConfigGroup group = ApplicationSettings::instance()->generalConfigGroup();
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
}

void ImportUI::slotColorManagementOptionsChanged()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    d->viewCMViewAction->blockSignals(true);
    d->viewCMViewAction->setEnabled(settings.enableCM);
    d->viewCMViewAction->setChecked(settings.useManagedPreviews);
    d->viewCMViewAction->blockSignals(false);
}

}  // namespace Digikam
