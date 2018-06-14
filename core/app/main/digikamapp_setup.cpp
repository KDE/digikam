/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Internal setup
 *
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikamapp.h"
#include "digikamapp_p.h"

namespace Digikam
{

void DigikamApp::rebuild()
{
    QString file = xmlFile();

    if (!file.isEmpty())
    {
        setXMLGUIBuildDocument(QDomDocument());
        loadStandardsXmlFile();
        setXMLFile(file, true);
    }
}

void DigikamApp::setupView()
{
    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Initializing Main View..."));
    }

    d->view = new DigikamView(this, d->modelCollection);
    setCentralWidget(d->view);
    d->view->applySettings();
}

void DigikamApp::setupViewConnections()
{
    connect(d->view, SIGNAL(signalAlbumSelected(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

    connect(d->view, SIGNAL(signalSelectionChanged(int)),
            this, SLOT(slotSelectionChanged(int)));

    connect(d->view, SIGNAL(signalImageSelected(ImageInfoList,ImageInfoList)),
            this, SLOT(slotImageSelected(ImageInfoList,ImageInfoList)));

    connect(d->view, SIGNAL(signalSwitchedToPreview()),
            this, SLOT(slotSwitchedToPreview()));

    connect(d->view, SIGNAL(signalSwitchedToIconView()),
            this, SLOT(slotSwitchedToIconView()));

    connect(d->view, SIGNAL(signalSwitchedToMapView()),
            this, SLOT(slotSwitchedToMapView()));

    connect(d->view, SIGNAL(signalSwitchedToTableView()),
            this, SLOT(slotSwitchedToTableView()));

    connect(d->view, SIGNAL(signalSwitchedToTrashView()),
            this, SLOT(slotSwitchedToTrashView()));
}

void DigikamApp::setupStatusBar()
{
    d->statusLabel = new DAdjustableLabel(statusBar());
    d->statusLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statusBar()->addWidget(d->statusLabel, 80);

    //------------------------------------------------------------------------------

    d->metadataStatusBar = new MetadataStatusBar(statusBar());
    statusBar()->addWidget(d->metadataStatusBar, 50);

    //------------------------------------------------------------------------------

    d->filterStatusBar = new FilterStatusBar(statusBar());
    statusBar()->addWidget(d->filterStatusBar, 50);
    d->view->connectIconViewFilter(d->filterStatusBar);

    //------------------------------------------------------------------------------

    ProgressView* const view = new ProgressView(statusBar(), this);
    view->hide();

    StatusbarProgressWidget* const littleProgress = new StatusbarProgressWidget(view, statusBar());
    littleProgress->show();
    statusBar()->addPermanentWidget(littleProgress);

    //------------------------------------------------------------------------------

    d->zoomBar = new DZoomBar(statusBar());
    d->zoomBar->setZoomToFitAction(d->zoomFitToWindowAction);
    d->zoomBar->setZoomTo100Action(d->zoomTo100percents);
    d->zoomBar->setZoomPlusAction(d->zoomPlusAction);
    d->zoomBar->setZoomMinusAction(d->zoomMinusAction);
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    statusBar()->addPermanentWidget(d->zoomBar);

    //------------------------------------------------------------------------------

    connect(d->zoomBar, SIGNAL(signalZoomSliderChanged(int)),
            this, SLOT(slotZoomSliderChanged(int)));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->zoomBar, SLOT(slotUpdateTrackerPos()));

    connect(d->zoomBar, SIGNAL(signalZoomValueEdited(double)),
            d->view, SLOT(setZoomFactor(double)));

    connect(d->view, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));

    connect(d->view, SIGNAL(signalThumbSizeChanged(int)),
            this, SLOT(slotThumbSizeChanged(int)));
}

void DigikamApp::setupActions()
{
    KActionCollection* const ac = actionCollection();

    d->solidCameraActionGroup = new QActionGroup(this);
    connect(d->solidCameraActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenSolidCamera(QAction*)));

    d->solidUsmActionGroup = new QActionGroup(this);
    connect(d->solidUsmActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenSolidUsmDevice(QAction*)));

    d->manualCameraActionGroup = new QActionGroup(this);
    connect(d->manualCameraActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenManualCamera(QAction*)));

    // -----------------------------------------------------------------

    d->backwardActionMenu = new KToolBarPopupAction(QIcon::fromTheme(QLatin1String("go-previous")), i18n("&Back"), this);
    d->backwardActionMenu->setEnabled(false);
    ac->addAction(QLatin1String("album_back"), d->backwardActionMenu);
    ac->setDefaultShortcut(d->backwardActionMenu, Qt::ALT+Qt::Key_Left);

    connect(d->backwardActionMenu->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowBackwardMenu()));

    // we are using a signal mapper to identify which of a bunch of actions was triggered
    d->backwardSignalMapper = new QSignalMapper(this);

    // connect mapper to view
    connect(d->backwardSignalMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotAlbumHistoryBack(int)));

    // connect action to mapper
    connect(d->backwardActionMenu, SIGNAL(triggered()),
            d->backwardSignalMapper, SLOT(map()));

    // inform mapper about number of steps
    d->backwardSignalMapper->setMapping(d->backwardActionMenu, 1);

    // -----------------------------------------------------------------

    d->forwardActionMenu = new KToolBarPopupAction(QIcon::fromTheme(QLatin1String("go-next")), i18n("Forward"), this);
    d->forwardActionMenu->setEnabled(false);
    ac->addAction(QLatin1String("album_forward"), d->forwardActionMenu);
    ac->setDefaultShortcut(d->forwardActionMenu, Qt::ALT+Qt::Key_Right);

    connect(d->forwardActionMenu->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowForwardMenu()));

    d->forwardSignalMapper = new QSignalMapper(this);

    connect(d->forwardSignalMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotAlbumHistoryForward(int)));

    connect(d->forwardActionMenu, SIGNAL(triggered()), d->forwardSignalMapper, SLOT(map()));
    d->forwardSignalMapper->setMapping(d->forwardActionMenu, 1);

    // -----------------------------------------------------------------

    d->refreshAction = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")), i18n("Refresh"), this);
    d->refreshAction->setWhatsThis(i18n("Refresh the current contents."));
    connect(d->refreshAction, SIGNAL(triggered()), d->view, SLOT(slotRefresh()));
    ac->addAction(QLatin1String("view_refresh"), d->refreshAction);
    ac->setDefaultShortcut(d->refreshAction, Qt::Key_F5);

    // -----------------------------------------------------------------

    QSignalMapper* const browseActionsMapper = new QSignalMapper(this);
    connect(browseActionsMapper, SIGNAL(mapped(QWidget*)),
            d->view, SLOT(slotLeftSideBarActivate(QWidget*)));

    foreach(SidebarWidget* const leftWidget, d->view->leftSidebarWidgets())
    {
        QString actionName = QLatin1String("browse_") + leftWidget->objectName()
                                                        .remove(QLatin1Char(' '))
                                                        .remove(QLatin1String("Sidebar"))
                                                        .remove(QLatin1String("FolderView"))
                                                        .remove(QLatin1String("View")).toLower();
        qCDebug(DIGIKAM_GENERAL_LOG) << actionName;

        QAction* const action = new QAction(leftWidget->getIcon(), leftWidget->getCaption(), this);
        ac->addAction(actionName, action);
        ac->setDefaultShortcut(action, QKeySequence(leftWidget->property("Shortcut").toInt()));
        connect(action, SIGNAL(triggered()), browseActionsMapper, SLOT(map()));
        browseActionsMapper->setMapping(action, leftWidget);
    }

    // -----------------------------------------------------------------

    d->newAction = new QAction(QIcon::fromTheme(QLatin1String("folder-new")), i18n("&New..."), this);
    d->newAction->setWhatsThis(i18n("Creates a new empty Album in the collection."));
    connect(d->newAction, SIGNAL(triggered()), d->view, SLOT(slotNewAlbum()));
    ac->addAction(QLatin1String("album_new"), d->newAction);
    ac->setDefaultShortcuts(d->newAction, QList<QKeySequence>() << Qt::CTRL + Qt::Key_N);

    // -----------------------------------------------------------------

    d->moveSelectionToAlbumAction = new QAction(QIcon::fromTheme(QLatin1String("folder-new")), i18n("&Move to Album..."), this);
    d->moveSelectionToAlbumAction->setWhatsThis(i18n("Move selected images into an album."));
    connect(d->moveSelectionToAlbumAction, SIGNAL(triggered()), d->view, SLOT(slotMoveSelectionToAlbum()));
    ac->addAction(QLatin1String("move_selection_to_album"), d->moveSelectionToAlbumAction);

    // -----------------------------------------------------------------

    d->deleteAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete Album"), this);
    connect(d->deleteAction, SIGNAL(triggered()), d->view, SLOT(slotDeleteAlbum()));
    ac->addAction(QLatin1String("album_delete"), d->deleteAction);

    // -----------------------------------------------------------------

    d->renameAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Rename..."), this);
    connect(d->renameAction, SIGNAL(triggered()), d->view, SLOT(slotRenameAlbum()));
    ac->addAction(QLatin1String("album_rename"), d->renameAction);
    ac->setDefaultShortcut(d->renameAction, Qt::SHIFT + Qt::Key_F2);

    // -----------------------------------------------------------------

    d->propsEditAction = new QAction(QIcon::fromTheme(QLatin1String("configure")), i18n("Properties"), this);
    d->propsEditAction->setWhatsThis(i18n("Edit album properties and collection information."));
    connect(d->propsEditAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumPropsEdit()));
    ac->addAction(QLatin1String("album_propsEdit"), d->propsEditAction);
    ac->setDefaultShortcut(d->propsEditAction, Qt::ALT + Qt::Key_Return);

    // -----------------------------------------------------------------

    d->writeAlbumMetadataAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Write Metadata to Files"), this);
    d->writeAlbumMetadataAction->setWhatsThis(i18n("Updates metadata of files in the current "
                                                   "album with the contents of digiKam database "
                                                   "(file metadata will be overwritten with data from "
                                                   "the database)."));
    connect(d->writeAlbumMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumWriteMetadata()));
    ac->addAction(QLatin1String("album_write_metadata"), d->writeAlbumMetadataAction);

    // -----------------------------------------------------------------

    d->readAlbumMetadataAction = new QAction(QIcon::fromTheme(QLatin1String("edit-redo")), i18n("Reread Metadata From Files"), this);
    d->readAlbumMetadataAction->setWhatsThis(i18n("Updates the digiKam database from the metadata "
                                                  "of the files in the current album "
                                                  "(information in the database will be overwritten with data from "
                                                  "the files' metadata)."));
    connect(d->readAlbumMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumReadMetadata()));
    ac->addAction(QLatin1String("album_read_metadata"), d->readAlbumMetadataAction);

    // -----------------------------------------------------------------

    d->openInFileManagerAction = new QAction(QIcon::fromTheme(QLatin1String("folder-open")), i18n("Open in File Manager"), this);
    connect(d->openInFileManagerAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumOpenInFileManager()));
    ac->addAction(QLatin1String("album_openinfilemanager"), d->openInFileManagerAction);

    // -----------------------------------------------------------

    d->openTagMngrAction = new QAction(QIcon::fromTheme(QLatin1String("tag")), i18n("Tag Manager"), this);
    connect(d->openTagMngrAction, SIGNAL(triggered()), d->view, SLOT(slotOpenTagsManager()));
    ac->addAction(QLatin1String("open_tag_mngr"), d->openTagMngrAction);

    // -----------------------------------------------------------

    d->newTagAction = new QAction(QIcon::fromTheme(QLatin1String("tag-new")), i18nc("new tag", "N&ew..."), this);
    connect(d->newTagAction, SIGNAL(triggered()), d->view, SLOT(slotNewTag()));
    ac->addAction(QLatin1String("tag_new"), d->newTagAction);

    // -----------------------------------------------------------

    d->editTagAction = new QAction(QIcon::fromTheme(QLatin1String("tag-properties")), i18n("Properties"), this);
    connect(d->editTagAction, SIGNAL(triggered()), d->view, SLOT(slotEditTag()));
    ac->addAction(QLatin1String("tag_edit"), d->editTagAction);

    // -----------------------------------------------------------

    d->deleteTagAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")), i18n("Delete"), this);
    connect(d->deleteTagAction, SIGNAL(triggered()), d->view, SLOT(slotDeleteTag()));
    ac->addAction(QLatin1String("tag_delete"), d->deleteTagAction);

    // -----------------------------------------------------------

    d->assignTagAction = new QAction(QIcon::fromTheme(QLatin1String("tag-new")), i18n("Assign Tag"), this);
    connect(d->assignTagAction, SIGNAL(triggered()), d->view, SLOT(slotAssignTag()));
    ac->addAction(QLatin1String("tag_assign"), d->assignTagAction);
    ac->setDefaultShortcut(d->assignTagAction, Qt::Key_T);

    // -----------------------------------------------------------

    d->imageViewSelectionAction = new KSelectAction(QIcon::fromTheme(QLatin1String("view-preview")), i18n("Views"), this);
    ac->addAction(QLatin1String("view_selection"), d->imageViewSelectionAction);

    d->imageIconViewAction = new QAction(QIcon::fromTheme(QLatin1String("view-list-icons")),
                                         i18nc("@action Go to thumbnails (icon) view", "Thumbnails"), this);
    d->imageIconViewAction->setCheckable(true);
    ac->addAction(QLatin1String("icon_view"), d->imageIconViewAction);
    connect(d->imageIconViewAction, SIGNAL(triggered()), d->view, SLOT(slotIconView()));
    d->imageViewSelectionAction->addAction(d->imageIconViewAction);

    d->imagePreviewAction = new QAction(QIcon::fromTheme(QLatin1String("view-preview")),
                                        i18nc("View the selected image", "Preview"), this);
    d->imagePreviewAction->setCheckable(true);
    ac->addAction(QLatin1String("image_view"), d->imagePreviewAction);
    ac->setDefaultShortcut(d->imagePreviewAction, Qt::Key_F3);
    connect(d->imagePreviewAction, SIGNAL(triggered()), d->view, SLOT(slotImagePreview()));
    d->imageViewSelectionAction->addAction(d->imagePreviewAction);

#ifdef HAVE_MARBLE
    d->imageMapViewAction = new QAction(QIcon::fromTheme(QLatin1String("globe")),
                                        i18nc("@action Switch to map view", "Map"), this);
    d->imageMapViewAction->setCheckable(true);
    ac->addAction(QLatin1String("map_view"), d->imageMapViewAction);
    connect(d->imageMapViewAction, SIGNAL(triggered()), d->view, SLOT(slotMapWidgetView()));
    d->imageViewSelectionAction->addAction(d->imageMapViewAction);
#endif // HAVE_MARBLE

    d->imageTableViewAction = new QAction(QIcon::fromTheme(QLatin1String("view-list-details")),
                                          i18nc("@action Switch to table view", "Table"), this);
    d->imageTableViewAction->setCheckable(true);
    ac->addAction(QLatin1String("table_view"), d->imageTableViewAction);
    connect(d->imageTableViewAction, SIGNAL(triggered()), d->view, SLOT(slotTableView()));
    d->imageViewSelectionAction->addAction(d->imageTableViewAction);

    // -----------------------------------------------------------

    d->imageViewAction = new QAction(QIcon::fromTheme(QLatin1String("quickopen-file")), i18n("Open..."), this);
    d->imageViewAction->setWhatsThis(i18n("Open the selected item."));
    connect(d->imageViewAction, SIGNAL(triggered()), d->view, SLOT(slotImageEdit()));
    ac->addAction(QLatin1String("image_edit"), d->imageViewAction);
    ac->setDefaultShortcut(d->imageViewAction, Qt::Key_F4);

    d->openWithAction = new QAction(QIcon::fromTheme(QLatin1String("preferences-desktop-filetype-association")), i18n("Open With Default Application"), this);
    d->openWithAction->setWhatsThis(i18n("Open the selected item with default assigned application."));
    connect(d->openWithAction, SIGNAL(triggered()), d->view, SLOT(slotFileWithDefaultApplication()));
    ac->addAction(QLatin1String("open_with_default_application"), d->openWithAction);
    ac->setDefaultShortcut(d->openWithAction, Qt::META + Qt::Key_F4);

    d->ieAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Image Editor"), this);
    d->ieAction->setWhatsThis(i18n("Open the image editor."));
    connect(d->ieAction, SIGNAL(triggered()), d->view, SLOT(slotEditor()));
    ac->addAction(QLatin1String("imageeditor"), d->ieAction);

    // -----------------------------------------------------------

    d->ltAction = new QAction(QIcon::fromTheme(QLatin1String("lighttable")), i18n("Light Table"), this);
    connect(d->ltAction, SIGNAL(triggered()), d->view, SLOT(slotLightTable()));
    ac->addAction(QLatin1String("light_table"), d->ltAction);
    ac->setDefaultShortcut(d->ltAction, Qt::Key_L);

    d->imageLightTableAction = new QAction(QIcon::fromTheme(QLatin1String("lighttable")), i18n("Place onto Light Table"), this);
    d->imageLightTableAction->setWhatsThis(i18n("Place the selected items on the light table thumbbar."));
    connect(d->imageLightTableAction, SIGNAL(triggered()), d->view, SLOT(slotImageLightTable()));
    ac->addAction(QLatin1String("image_lighttable"), d->imageLightTableAction);
    ac->setDefaultShortcut(d->imageLightTableAction, Qt::CTRL+Qt::Key_L);

    d->imageAddLightTableAction = new QAction(QIcon::fromTheme(QLatin1String("list-add")), i18n("Add to Light Table"), this);
    d->imageAddLightTableAction->setWhatsThis(i18n("Add selected items to the light table thumbbar."));
    connect(d->imageAddLightTableAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToLightTable()));
    ac->addAction(QLatin1String("image_add_to_lighttable"), d->imageAddLightTableAction);
    ac->setDefaultShortcut(d->imageAddLightTableAction, Qt::SHIFT+Qt::CTRL+Qt::Key_L);

    // -----------------------------------------------------------

    d->bqmAction = new QAction(QIcon::fromTheme(QLatin1String("run-build")), i18n("Batch Queue Manager"), this);
    connect(d->bqmAction, SIGNAL(triggered()), d->view, SLOT(slotQueueMgr()));
    ac->addAction(QLatin1String("queue_manager"), d->bqmAction);
    ac->setDefaultShortcut(d->bqmAction, Qt::Key_B);

    d->imageAddCurrentQueueAction = new QAction(QIcon::fromTheme(QLatin1String("go-up")), i18n("Add to Current Queue"), this);
    d->imageAddCurrentQueueAction->setWhatsThis(i18n("Add selected items to current queue from batch manager."));
    connect(d->imageAddCurrentQueueAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToCurrentQueue()));
    ac->addAction(QLatin1String("image_add_to_current_queue"), d->imageAddCurrentQueueAction);
    ac->setDefaultShortcut(d->imageAddCurrentQueueAction, Qt::CTRL+Qt::Key_B);

    d->imageAddNewQueueAction = new QAction(QIcon::fromTheme(QLatin1String("list-add")), i18n("Add to New Queue"), this);
    d->imageAddNewQueueAction->setWhatsThis(i18n("Add selected items to a new queue from batch manager."));
    connect(d->imageAddNewQueueAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToNewQueue()));
    ac->addAction(QLatin1String("image_add_to_new_queue"), d->imageAddNewQueueAction);
    ac->setDefaultShortcut(d->imageAddNewQueueAction, Qt::SHIFT+Qt::CTRL+Qt::Key_B);

    // -----------------------------------------------------------------

    d->quickImportMenu->setTitle(i18nc("@action Import photos from camera", "Import"));
    d->quickImportMenu->setIcon(QIcon::fromTheme(QLatin1String("camera-photo")));
    ac->addAction(QLatin1String("import_auto"), d->quickImportMenu->menuAction());

    // -----------------------------------------------------------------

    d->imageWriteMetadataAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")),
                                              i18n("Write Metadata to Selected Files"), this);
    d->imageWriteMetadataAction->setWhatsThis(i18n("Updates metadata of files in the current "
                                                   "album with the contents of digiKam database "
                                                   "(file metadata will be overwritten with data from "
                                                   "the database)."));
    connect(d->imageWriteMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotImageWriteMetadata()));
    ac->addAction(QLatin1String("image_write_metadata"), d->imageWriteMetadataAction);

    // -----------------------------------------------------------------

    d->imageReadMetadataAction = new QAction(QIcon::fromTheme(QLatin1String("edit-redo")),
                                             i18n("Reread Metadata From Selected Files"), this);
    d->imageReadMetadataAction->setWhatsThis(i18n("Updates the digiKam database from the metadata "
                                                  "of the files in the current album "
                                                  "(information in the database will be overwritten with data from "
                                                  "the files' metadata)."));
    connect(d->imageReadMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotImageReadMetadata()));
    ac->addAction(QLatin1String("image_read_metadata"), d->imageReadMetadataAction);

    // -----------------------------------------------------------

    d->imageScanForFacesAction = new QAction(QIcon::fromTheme(QLatin1String("list-add-user")), i18n("Scan for Faces"), this);
    connect(d->imageScanForFacesAction, SIGNAL(triggered()), d->view, SLOT(slotImageScanForFaces()));
    ac->addAction(QLatin1String("image_scan_for_faces"), d->imageScanForFacesAction);

    // -----------------------------------------------------------

    d->imageFindSimilarAction = new QAction(QIcon::fromTheme(QLatin1String("tools-wizard")), i18n("Find Similar..."), this);
    d->imageFindSimilarAction->setWhatsThis(i18n("Find similar images using selected item as reference."));
    connect(d->imageFindSimilarAction, SIGNAL(triggered()), d->view, SLOT(slotImageFindSimilar()));
    ac->addAction(QLatin1String("image_find_similar"), d->imageFindSimilarAction);

    // -----------------------------------------------------------

    d->imageRenameAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Rename..."), this);
    d->imageRenameAction->setWhatsThis(i18n("Change the filename of the currently selected item."));
    connect(d->imageRenameAction, SIGNAL(triggered()), d->view, SLOT(slotImageRename()));
    ac->addAction(QLatin1String("image_rename"), d->imageRenameAction);
    ac->setDefaultShortcut(d->imageRenameAction, Qt::Key_F2);

    // -----------------------------------------------------------

    // Pop up dialog to ask user whether to move to trash
    d->imageDeleteAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")), i18nc("Non-pluralized", "Move to Trash"), this);
    connect(d->imageDeleteAction, SIGNAL(triggered()), d->view, SLOT(slotImageDelete()));
    ac->addAction(QLatin1String("image_delete"), d->imageDeleteAction);
    ac->setDefaultShortcut(d->imageDeleteAction, Qt::Key_Delete);

    // -----------------------------------------------------------

    // Pop up dialog to ask user whether to permanently delete
    // FIXME: This action is never used?? How can someone delete a album directly, without moving it to the trash first?
    //        This is especially important when deleting from a different partition or from a net source.
    //        Also note that we use the wrong icon for the default album delete action, which should have a trashcan icon instead
    //        of a red cross, it confuses users.
    d->imageDeletePermanentlyAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Delete Permanently"), this);
    connect(d->imageDeletePermanentlyAction, SIGNAL(triggered()), d->view, SLOT(slotImageDeletePermanently()));
    ac->addAction(QLatin1String("image_delete_permanently"), d->imageDeletePermanentlyAction);
    ac->setDefaultShortcut(d->imageDeletePermanentlyAction, Qt::SHIFT+Qt::Key_Delete);

    // -----------------------------------------------------------

    // These two actions are hidden, no menu entry, no toolbar entry, no shortcut.
    // Power users may add them.
    d->imageDeletePermanentlyDirectlyAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),
                                                          i18n("Delete permanently without confirmation"), this);
    connect(d->imageDeletePermanentlyDirectlyAction, SIGNAL(triggered()),
            d->view, SLOT(slotImageDeletePermanentlyDirectly()));
    ac->addAction(QLatin1String("image_delete_permanently_directly"), d->imageDeletePermanentlyDirectlyAction);

    // -----------------------------------------------------------

    d->imageTrashDirectlyAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")),
                                              i18n("Move to trash without confirmation"), this);
    connect(d->imageTrashDirectlyAction, SIGNAL(triggered()),
            d->view, SLOT(slotImageTrashDirectly()));
    ac->addAction(QLatin1String("image_trash_directly"), d->imageTrashDirectlyAction);

    // -----------------------------------------------------------------

    d->albumSortAction = new KSelectAction(i18n("&Sort Albums"), this);
    d->albumSortAction->setWhatsThis(i18n("Sort Albums in tree-view."));
    connect(d->albumSortAction, SIGNAL(triggered(int)), d->view, SLOT(slotSortAlbums(int)));
    ac->addAction(QLatin1String("album_sort"), d->albumSortAction);

    // Use same list order as in applicationsettings enum
    QStringList sortActionList;
    sortActionList.append(i18n("By Folder"));
    sortActionList.append(i18n("By Category"));
    sortActionList.append(i18n("By Date"));
    d->albumSortAction->setItems(sortActionList);

    // -----------------------------------------------------------

    d->recurseAlbumsAction = new QAction(i18n("Include Album Sub-Tree"), this);
    d->recurseAlbumsAction->setCheckable(true);
    d->recurseAlbumsAction->setWhatsThis(i18n("Activate this option to show all sub-albums below "
                                              "the current album."));
    connect(d->recurseAlbumsAction, SIGNAL(toggled(bool)), this, SLOT(slotRecurseAlbums(bool)));
    ac->addAction(QLatin1String("albums_recursive"), d->recurseAlbumsAction);

    d->recurseTagsAction = new QAction(i18n("Include Tag Sub-Tree"), this);
    d->recurseTagsAction->setCheckable(true);
    d->recurseTagsAction->setWhatsThis(i18n("Activate this option to show all images marked by the given tag "
                                            "and all its sub-tags."));
    connect(d->recurseTagsAction, SIGNAL(toggled(bool)), this, SLOT(slotRecurseTags(bool)));
    ac->addAction(QLatin1String("tags_recursive"), d->recurseTagsAction);

    // -----------------------------------------------------------

    d->imageSortAction                   = new KSelectAction(i18n("&Sort Items"), this);
    d->imageSortAction->setWhatsThis(i18n("The value by which the images in one album are sorted in the thumbnail view"));
    QSignalMapper* const imageSortMapper = new QSignalMapper(this);
    connect(imageSortMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSortImages(int)));
    ac->addAction(QLatin1String("image_sort"), d->imageSortAction);

    // map to ImageSortSettings enum
    QAction* const sortByNameAction        = d->imageSortAction->addAction(i18n("By Name"));
    QAction* const sortByPathAction        = d->imageSortAction->addAction(i18n("By Path"));
    QAction* const sortByDateAction        = d->imageSortAction->addAction(i18n("By Date"));
    QAction* const sortByFileSizeAction    = d->imageSortAction->addAction(i18n("By File Size"));
    QAction* const sortByRatingAction      = d->imageSortAction->addAction(i18n("By Rating"));
    QAction* const sortByImageSizeAction   = d->imageSortAction->addAction(i18n("By Image Size"));
    QAction* const sortByAspectRatioAction = d->imageSortAction->addAction(i18n("By Aspect Ratio"));
    QAction* const sortBySimilarityAction  = d->imageSortAction->addAction(i18n("By Similarity"));

    // activate the sort by similarity if the fuzzy search sidebar is active. Deactivate at start.
    sortBySimilarityAction->setEnabled(false);
    connect(d->view, SIGNAL(signalFuzzySidebarActive(bool)), sortBySimilarityAction, SLOT(setEnabled(bool)));

    connect(sortByNameAction,        SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByPathAction,        SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByDateAction,        SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByFileSizeAction,    SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByRatingAction,      SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByImageSizeAction,   SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortByAspectRatioAction, SIGNAL(triggered()), imageSortMapper, SLOT(map()));
    connect(sortBySimilarityAction,  SIGNAL(triggered()), imageSortMapper, SLOT(map()));

    imageSortMapper->setMapping(sortByNameAction,        (int)ImageSortSettings::SortByFileName);
    imageSortMapper->setMapping(sortByPathAction,        (int)ImageSortSettings::SortByFilePath);
    imageSortMapper->setMapping(sortByDateAction,        (int)ImageSortSettings::SortByCreationDate);
    imageSortMapper->setMapping(sortByFileSizeAction,    (int)ImageSortSettings::SortByFileSize);
    imageSortMapper->setMapping(sortByRatingAction,      (int)ImageSortSettings::SortByRating);
    imageSortMapper->setMapping(sortByImageSizeAction,   (int)ImageSortSettings::SortByImageSize);
    imageSortMapper->setMapping(sortByAspectRatioAction, (int)ImageSortSettings::SortByAspectRatio);
    imageSortMapper->setMapping(sortBySimilarityAction,  (int)ImageSortSettings::SortBySimilarity);

    // -----------------------------------------------------------

    d->imageSortOrderAction                   = new KSelectAction(i18n("Item Sort &Order"), this);
    d->imageSortOrderAction->setWhatsThis(i18n("Defines whether images are sorted in ascending or descending manner."));
    QSignalMapper* const imageSortOrderMapper = new QSignalMapper(this);
    connect(imageSortOrderMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSortImagesOrder(int)));
    ac->addAction(QLatin1String("image_sort_order"), d->imageSortOrderAction);

    QAction* const sortAscendingAction  = d->imageSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-ascending")),  i18n("Ascending"));
    QAction* const sortDescendingAction = d->imageSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-descending")), i18n("Descending"));

    connect(sortAscendingAction,  SIGNAL(triggered()), imageSortOrderMapper, SLOT(map()));
    connect(sortDescendingAction, SIGNAL(triggered()), imageSortOrderMapper, SLOT(map()));

    imageSortOrderMapper->setMapping(sortAscendingAction,  (int)ImageSortSettings::AscendingOrder);
    imageSortOrderMapper->setMapping(sortDescendingAction, (int)ImageSortSettings::DescendingOrder);

    // -----------------------------------------------------------

    d->imageSeparationAction                   = new KSelectAction(i18n("Separate Items"), this);
    d->imageSeparationAction->setWhatsThis(i18n("The categories in which the images in the thumbnail view are displayed"));
    QSignalMapper* const imageSeparationMapper = new QSignalMapper(this);
    connect(imageSeparationMapper, SIGNAL(mapped(int)), d->view, SLOT(slotSeparateImages(int)));
    ac->addAction(QLatin1String("image_separation"), d->imageSeparationAction);

    // map to ImageSortSettings enum
    QAction* const noCategoriesAction     = d->imageSeparationAction->addAction(i18n("Flat List"));
    QAction* const separateByAlbumAction  = d->imageSeparationAction->addAction(i18n("By Album"));
    QAction* const separateByFormatAction = d->imageSeparationAction->addAction(i18n("By Format"));

    connect(noCategoriesAction,  SIGNAL(triggered()), imageSeparationMapper, SLOT(map()));
    connect(separateByAlbumAction,  SIGNAL(triggered()), imageSeparationMapper, SLOT(map()));
    connect(separateByFormatAction, SIGNAL(triggered()), imageSeparationMapper, SLOT(map()));

    imageSeparationMapper->setMapping(noCategoriesAction,  (int)ImageSortSettings::OneCategory);
    imageSeparationMapper->setMapping(separateByAlbumAction,  (int)ImageSortSettings::CategoryByAlbum);
    imageSeparationMapper->setMapping(separateByFormatAction, (int)ImageSortSettings::CategoryByFormat);

    // -----------------------------------------------------------------

    d->imageSeparationSortOrderAction                   = new KSelectAction(i18n("Item Separation Order"), this);
    d->imageSeparationSortOrderAction->setWhatsThis(i18n("The sort order of the groups of separated items"));
    QSignalMapper* const imageSeparationSortOrderMapper = new QSignalMapper(this);
    connect(imageSeparationSortOrderMapper, SIGNAL(mapped(int)), d->view, SLOT(slotImageSeparationSortOrder(int)));
    ac->addAction(QLatin1String("image_separation_sort_order"), d->imageSeparationSortOrderAction);

    QAction* const sortSeparationsAscending  = d->imageSeparationSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-ascending")),  i18n("Ascending"));
    QAction* const sortSeparationsDescending = d->imageSeparationSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-descending")), i18n("Descending"));

    connect(sortSeparationsAscending,  SIGNAL(triggered()), imageSeparationSortOrderMapper, SLOT(map()));
    connect(sortSeparationsDescending, SIGNAL(triggered()), imageSeparationSortOrderMapper, SLOT(map()));

    imageSeparationSortOrderMapper->setMapping(sortSeparationsAscending, (int)ImageSortSettings::AscendingOrder);
    imageSeparationSortOrderMapper->setMapping(sortSeparationsDescending, (int)ImageSortSettings::DescendingOrder);

    // -----------------------------------------------------------------

    setupImageTransformActions();
    setupExifOrientationActions();
    createMetadataEditAction();
    createGeolocationEditAction();
    createExportActions();
    createImportActions();

    // -----------------------------------------------------------------

    d->selectAllAction = new QAction(i18n("Select All"), this);
    connect(d->selectAllAction, SIGNAL(triggered()), d->view, SLOT(slotSelectAll()));
    ac->addAction(QLatin1String("selectAll"), d->selectAllAction);
    ac->setDefaultShortcut(d->selectAllAction, Qt::CTRL+Qt::Key_A);

    // -----------------------------------------------------------------

    d->selectNoneAction = new QAction(i18n("Select None"), this);
    connect(d->selectNoneAction, SIGNAL(triggered()), d->view, SLOT(slotSelectNone()));
    ac->addAction(QLatin1String("selectNone"), d->selectNoneAction);
    ac->setDefaultShortcut(d->selectNoneAction, Qt::CTRL+Qt::SHIFT+Qt::Key_A);

    // -----------------------------------------------------------------

    d->selectInvertAction = new QAction(i18n("Invert Selection"), this);
    connect(d->selectInvertAction, SIGNAL(triggered()), d->view, SLOT(slotSelectInvert()));
    ac->addAction(QLatin1String("selectInvert"), d->selectInvertAction);
    ac->setDefaultShortcut(d->selectInvertAction, Qt::CTRL+Qt::Key_I);

    // -----------------------------------------------------------

    d->showBarAction = new QAction(QIcon::fromTheme(QLatin1String("view-choose")), i18n("Show Thumbbar"), this);
    d->showBarAction->setCheckable(true);
    connect(d->showBarAction, SIGNAL(triggered()), this, SLOT(slotToggleShowBar()));
    ac->addAction(QLatin1String("showthumbs"), d->showBarAction);
    ac->setDefaultShortcut(d->showBarAction, Qt::CTRL+Qt::Key_T);

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // Standard 'Configure' menu actions
    createSettingsActions();

    // -----------------------------------------------------------

    d->zoomPlusAction  = buildStdAction(StdZoomInAction, d->view, SLOT(slotZoomIn()), this);
    QKeySequence keysPlus(d->zoomPlusAction->shortcut()[0], Qt::Key_Plus);
    ac->addAction(QLatin1String("album_zoomin"), d->zoomPlusAction);
    ac->setDefaultShortcut(d->zoomPlusAction, keysPlus);

    // -----------------------------------------------------------

    d->zoomMinusAction  = buildStdAction(StdZoomOutAction, d->view, SLOT(slotZoomOut()), this);
    QKeySequence keysMinus(d->zoomMinusAction->shortcut()[0], Qt::Key_Minus);
    ac->addAction(QLatin1String("album_zoomout"), d->zoomMinusAction);
    ac->setDefaultShortcut(d->zoomMinusAction, keysMinus);

    // -----------------------------------------------------------

    d->zoomTo100percents = new QAction(QIcon::fromTheme(QLatin1String("zoom-original")), i18n("Zoom to 100%"), this);
    connect(d->zoomTo100percents, SIGNAL(triggered()), d->view, SLOT(slotZoomTo100Percents()));
    ac->addAction(QLatin1String("album_zoomto100percents"), d->zoomTo100percents);
    ac->setDefaultShortcut(d->zoomTo100percents, Qt::CTRL + Qt::Key_Period);

    // -----------------------------------------------------------

    d->zoomFitToWindowAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")), i18n("Fit to &Window"), this);
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), d->view, SLOT(slotFitToWindow()));
    ac->addAction(QLatin1String("album_zoomfit2window"), d->zoomFitToWindowAction);
    ac->setDefaultShortcut(d->zoomFitToWindowAction, Qt::ALT + Qt::CTRL + Qt::Key_E);

    // -----------------------------------------------------------

    createFullScreenAction(QLatin1String("full_screen"));
    createSidebarActions();

    // -----------------------------------------------------------

    d->slideShowAction = new QMenu(i18n("Slideshow"), this);
    d->slideShowAction->setIcon(QIcon::fromTheme(QLatin1String("view-presentation")));
    ac->addAction(QLatin1String("slideshow"), d->slideShowAction->menuAction());

    d->slideShowAllAction = new QAction(i18n("All"), this);
    connect(d->slideShowAllAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowAll()));
    ac->addAction(QLatin1String("slideshow_all"), d->slideShowAllAction);
    ac->setDefaultShortcut(d->slideShowAllAction, Qt::Key_F9);
    d->slideShowAction->addAction(d->slideShowAllAction);

    d->slideShowSelectionAction = new QAction(i18n("Selection"), this);
    connect(d->slideShowSelectionAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowSelection()));
    ac->addAction(QLatin1String("slideshow_selected"), d->slideShowSelectionAction);
    ac->setDefaultShortcut(d->slideShowSelectionAction, Qt::ALT+Qt::Key_F9);
    d->slideShowAction->addAction(d->slideShowSelectionAction);

    d->slideShowRecursiveAction = new QAction(i18n("With All Sub-Albums"), this);
    connect(d->slideShowRecursiveAction, SIGNAL(triggered()), d->view, SLOT(slotSlideShowRecursive()));
    ac->addAction(QLatin1String("slideshow_recursive"), d->slideShowRecursiveAction);
    ac->setDefaultShortcut(d->slideShowRecursiveAction, Qt::SHIFT+Qt::Key_F9);
    d->slideShowAction->addAction(d->slideShowRecursiveAction);

    createPresentationAction();

    // -----------------------------------------------------------

    d->viewCMViewAction = new QAction(QIcon::fromTheme(QLatin1String("video-display")), i18n("Color-Managed View"), this);
    d->viewCMViewAction->setCheckable(true);
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    ac->addAction(QLatin1String("color_managed_view"), d->viewCMViewAction);
    ac->setDefaultShortcut(d->viewCMViewAction, Qt::Key_F12);

    // -----------------------------------------------------------

    d->quitAction = buildStdAction(StdQuitAction, this, SLOT(slotExit()), this);
    ac->addAction(QLatin1String("app_exit"), d->quitAction);

    // -----------------------------------------------------------

    createHelpActions();

    // -----------------------------------------------------------

    QAction* const findAction = new QAction(QIcon::fromTheme(QLatin1String("edit-find")), i18n("Search..."), this);
    connect(findAction, SIGNAL(triggered()), d->view, SLOT(slotNewKeywordSearch()));
    ac->addAction(QLatin1String("search_quick"), findAction);
    ac->setDefaultShortcut(findAction, Qt::CTRL+Qt::Key_F);

    // -----------------------------------------------------------

    d->advSearchAction = new QAction(QIcon::fromTheme(QLatin1String("edit-find")), i18n("Advanced Search..."), this);
    connect(d->advSearchAction, SIGNAL(triggered()), d->view, SLOT(slotNewAdvancedSearch()));
    ac->addAction(QLatin1String("search_advanced"), d->advSearchAction);
    ac->setDefaultShortcut(d->advSearchAction, Qt::CTRL+Qt::ALT+Qt::Key_F);

    // -----------------------------------------------------------

    QAction* const duplicatesAction = new QAction(QIcon::fromTheme(QLatin1String("tools-wizard")), i18n("Find Duplicates..."), this);
    connect(duplicatesAction, SIGNAL(triggered()), d->view, SLOT(slotNewDuplicatesSearch()));
    ac->addAction(QLatin1String("find_duplicates"), duplicatesAction);
    ac->setDefaultShortcut(duplicatesAction, Qt::CTRL+Qt::Key_D);

    // -----------------------------------------------------------

#ifdef HAVE_MYSQLSUPPORT
    QAction* const databaseMigrationAction = new QAction(QIcon::fromTheme(QLatin1String("network-server-database")), i18n("Database Migration..."), this);
    connect(databaseMigrationAction, SIGNAL(triggered()), this, SLOT(slotDatabaseMigration()));
    ac->addAction(QLatin1String("database_migration"), databaseMigrationAction);
#endif

    // -----------------------------------------------------------

    d->maintenanceAction = new QAction(QIcon::fromTheme(QLatin1String("run-build-prune")), i18n("Maintenance..."), this);
    connect(d->maintenanceAction, SIGNAL(triggered()), this, SLOT(slotMaintenance()));
    ac->addAction(QLatin1String("maintenance"), d->maintenanceAction);

    createExpoBlendingAction();
    createPanoramaAction();
    createHtmlGalleryAction();
    createCalendarAction();
    createVideoSlideshowAction();
    createSendByMailAction();
    createPrintCreatorAction();
    createMediaServerAction();

    // -----------------------------------------------------------

    QAction* const cameraAction = new QAction(i18n("Add Camera Manually..."), this);
    connect(cameraAction, SIGNAL(triggered()), this, SLOT(slotSetupCamera()));
    ac->addAction(QLatin1String("camera_add"), cameraAction);

    // -----------------------------------------------------------

    // Load Cameras -- do this before the createGUI so that the cameras
    // are plugged into the toolbar at startup
    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Loading cameras..."));
    }

    loadCameras();

    // Load Themes

    populateThemes();

    createGUI(xmlFile());

    cleanupActions();

    // NOTE: see bug #252130 and #283281 : we need to disable these actions when BQM is running.
    // These connections must be done after loading color theme else theme menu cannot be plugged to Settings menu,

    connect(QueueMgrWindow::queueManagerWindow(), SIGNAL(signalBqmIsBusy(bool)),
            d->bqmAction, SLOT(setDisabled(bool)));

    connect(QueueMgrWindow::queueManagerWindow(), SIGNAL(signalBqmIsBusy(bool)),
            d->imageAddCurrentQueueAction, SLOT(setDisabled(bool)));

    connect(QueueMgrWindow::queueManagerWindow(), SIGNAL(signalBqmIsBusy(bool)),
            d->imageAddNewQueueAction, SLOT(setDisabled(bool)));
}

void DigikamApp::setupAccelerators()
{
    KActionCollection* const ac = actionCollection();

    // Action are added by <MainWindow> tag in ui.rc XML file
    QAction* const escapeAction = new QAction(i18n("Exit Preview Mode"), this);
    ac->addAction(QLatin1String("exit_preview_mode"), escapeAction);
    ac->setDefaultShortcut(escapeAction, Qt::Key_Escape);
    connect(escapeAction, SIGNAL(triggered()), this, SIGNAL(signalEscapePressed()));

    QAction* const nextImageAction = new QAction(i18n("Next Image"), this);
    nextImageAction->setIcon(QIcon::fromTheme(QLatin1String("go-next")));
    ac->addAction(QLatin1String("next_image"), nextImageAction);
    ac->setDefaultShortcut(nextImageAction, Qt::Key_Space);
    connect(nextImageAction, SIGNAL(triggered()), this, SIGNAL(signalNextItem()));

    QAction* const previousImageAction = new QAction(i18n("Previous Image"), this);
    previousImageAction->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));
    ac->addAction(QLatin1String("previous_image"), previousImageAction);
    ac->setDefaultShortcuts(previousImageAction, QList<QKeySequence>() << Qt::Key_Backspace << Qt::SHIFT+Qt::Key_Space);
    connect(previousImageAction, SIGNAL(triggered()), this, SIGNAL(signalPrevItem()));

    QAction* const firstImageAction = new QAction(i18n("First Image"), this);
    ac->addAction(QLatin1String("first_image"), firstImageAction);
    ac->setDefaultShortcuts(firstImageAction, QList<QKeySequence>() << Qt::CTRL + Qt::Key_Home);
    connect(firstImageAction, SIGNAL(triggered()), this, SIGNAL(signalFirstItem()));

    QAction* const lastImageAction = new QAction(i18n("Last Image"), this);
    ac->addAction(QLatin1String("last_image"), lastImageAction);
    ac->setDefaultShortcuts(lastImageAction, QList<QKeySequence>() << Qt::CTRL + Qt::Key_End);
    connect(lastImageAction, SIGNAL(triggered()), this, SIGNAL(signalLastItem()));

    d->cutItemsAction = new QAction(i18n("Cu&t"), this);
    d->cutItemsAction->setIcon(QIcon::fromTheme(QLatin1String("edit-cut")));
    d->cutItemsAction->setWhatsThis(i18n("Cut selection to clipboard"));
    ac->addAction(QLatin1String("cut_album_selection"), d->cutItemsAction);
    // NOTE: shift+del keyboard shortcut must not be assigned to Cut action
    // else the shortcut for Delete permanently collides with secondary shortcut of Cut
    ac->setDefaultShortcut(d->cutItemsAction, Qt::CTRL + Qt::Key_X);
    connect(d->cutItemsAction, SIGNAL(triggered()), this, SIGNAL(signalCutAlbumItemsSelection()));

    d->copyItemsAction = buildStdAction(StdCopyAction, this, SIGNAL(signalCopyAlbumItemsSelection()), this);
    ac->addAction(QLatin1String("copy_album_selection"), d->copyItemsAction);

    d->pasteItemsAction = buildStdAction(StdPasteAction, this, SIGNAL(signalPasteAlbumItemsSelection()), this);
    ac->addAction(QLatin1String("paste_album_selection"), d->pasteItemsAction);

    // Labels shortcuts must be registered here to be saved in XML GUI files if user customize it.
    d->tagsActionManager->registerLabelsActions(ac);

    QAction* const editTitles = new QAction(i18n("Edit Titles"), this);
    ac->addAction(QLatin1String("edit_titles"), editTitles);
    ac->setDefaultShortcut(editTitles, Qt::META + Qt::Key_T);
    connect(editTitles, SIGNAL(triggered()), d->view, SLOT(slotRightSideBarActivateTitles()));

    QAction* const editComments = new QAction(i18n("Edit Comments"), this);
    ac->addAction(QLatin1String("edit_comments"), editComments);
    ac->setDefaultShortcut(editComments, Qt::META + Qt::Key_C);
    connect(editComments, SIGNAL(triggered()), d->view, SLOT(slotRightSideBarActivateComments()));

    QAction* const assignedTags = new QAction(i18n("Show Assigned Tags"), this);
    ac->addAction(QLatin1String("assigned _tags"), assignedTags);
    ac->setDefaultShortcut(assignedTags, Qt::META + Qt::Key_A);
    connect(assignedTags, SIGNAL(triggered()), d->view, SLOT(slotRightSideBarActivateAssignedTags()));
}

void DigikamApp::setupExifOrientationActions()
{
    KActionCollection* const ac                = actionCollection();
    QSignalMapper* const exifOrientationMapper = new QSignalMapper(d->view);

    connect(exifOrientationMapper, SIGNAL(mapped(int)),
            d->view, SLOT(slotImageExifOrientation(int)));

    d->imageExifOrientationActionMenu = new QMenu(i18n("Adjust Exif Orientation Tag"), this);
    ac->addAction(QLatin1String("image_set_exif_orientation"), d->imageExifOrientationActionMenu->menuAction());

    d->imageSetExifOrientation1Action = new QAction(i18nc("normal exif orientation", "Normal"), this);
    d->imageSetExifOrientation1Action->setCheckable(true);
    d->imageSetExifOrientation2Action = new QAction(i18n("Flipped Horizontally"),               this);
    d->imageSetExifOrientation2Action->setCheckable(true);
    d->imageSetExifOrientation3Action = new QAction(i18n("Rotated Upside Down"),                this);
    d->imageSetExifOrientation3Action->setCheckable(true);
    d->imageSetExifOrientation4Action = new QAction(i18n("Flipped Vertically"),                 this);
    d->imageSetExifOrientation4Action->setCheckable(true);
    d->imageSetExifOrientation5Action = new QAction(i18n("Rotated Right / Horiz. Flipped"),     this);
    d->imageSetExifOrientation5Action->setCheckable(true);
    d->imageSetExifOrientation6Action = new QAction(i18n("Rotated Right"),                      this);
    d->imageSetExifOrientation6Action->setCheckable(true);
    d->imageSetExifOrientation7Action = new QAction(i18n("Rotated Right / Vert. Flipped"),      this);
    d->imageSetExifOrientation7Action->setCheckable(true);
    d->imageSetExifOrientation8Action = new QAction(i18n("Rotated Left"),                       this);
    d->imageSetExifOrientation8Action->setCheckable(true);

    d->exifOrientationActionGroup = new QActionGroup(d->imageExifOrientationActionMenu);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation1Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation2Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation3Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation4Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation5Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation6Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation7Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation8Action);
    d->imageSetExifOrientation1Action->setChecked(true);

    ac->addAction(QLatin1String("image_set_exif_orientation_normal"),                    d->imageSetExifOrientation1Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_flipped_horizontal"),        d->imageSetExifOrientation2Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_upside_down"),       d->imageSetExifOrientation3Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_flipped_vertically"),        d->imageSetExifOrientation4Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_right_hor_flipped"), d->imageSetExifOrientation5Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_right"),             d->imageSetExifOrientation6Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_right_ver_flipped"), d->imageSetExifOrientation7Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_left"),              d->imageSetExifOrientation8Action);

    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation1Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation2Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation3Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation4Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation5Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation6Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation7Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation8Action);

    connect(d->imageSetExifOrientation1Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation2Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation3Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation4Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation5Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation6Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation7Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    connect(d->imageSetExifOrientation8Action, SIGNAL(triggered()),
            exifOrientationMapper, SLOT(map()));

    exifOrientationMapper->setMapping(d->imageSetExifOrientation1Action, 1);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation2Action, 2);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation3Action, 3);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation4Action, 4);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation5Action, 5);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation6Action, 6);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation7Action, 7);
    exifOrientationMapper->setMapping(d->imageSetExifOrientation8Action, 8);
}

void DigikamApp::setupImageTransformActions()
{
    KActionCollection* const ac = actionCollection();

    d->imageRotateActionMenu = new QMenu(i18n("Rotate"), this);
    d->imageRotateActionMenu->setIcon(QIcon::fromTheme(QLatin1String("object-rotate-right")));

    QAction* const left = ac->addAction(QLatin1String("rotate_ccw"));
    left->setText(i18nc("rotate image left", "Left"));
    ac->setDefaultShortcut(left, Qt::SHIFT+Qt::CTRL+Qt::Key_Left);
    connect(left, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageRotateActionMenu->addAction(left);

    QAction* const right = ac->addAction(QLatin1String("rotate_cw"));
    right->setText(i18nc("rotate image right", "Right"));
    ac->setDefaultShortcut(right, Qt::SHIFT+Qt::CTRL+Qt::Key_Right);
    connect(right, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageRotateActionMenu->addAction(right);

    ac->addAction(QLatin1String("image_rotate"), d->imageRotateActionMenu->menuAction());

    // -----------------------------------------------------------------------------------

    d->imageFlipActionMenu = new QMenu(i18n("Flip"), this);
    d->imageFlipActionMenu->setIcon(QIcon::fromTheme(QLatin1String("flip-horizontal")));

    QAction* const hori = ac->addAction(QLatin1String("flip_horizontal"));
    hori->setText(i18n("Horizontally"));
    ac->setDefaultShortcut(hori, Qt::CTRL+Qt::Key_Asterisk);
    connect(hori, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageFlipActionMenu->addAction(hori);

    QAction* const verti = ac->addAction(QLatin1String("flip_vertical"));
    verti->setText(i18n("Vertically"));
    ac->setDefaultShortcut(verti, Qt::CTRL+Qt::Key_Slash);
    connect(verti, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageFlipActionMenu->addAction(verti);

    ac->addAction(QLatin1String("image_flip"), d->imageFlipActionMenu->menuAction());

    // -----------------------------------------------------------------------------------

    d->imageAutoExifActionMenu = new QAction(i18n("Auto Rotate/Flip Using Exif Information"), this);
    connect(d->imageAutoExifActionMenu, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));

    ac->addAction(QLatin1String("image_transform_exif"), d->imageAutoExifActionMenu);
}

void DigikamApp::populateThemes()
{
    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Loading themes..."));
    }

    ThemeManager::instance()->setThemeMenuAction(new QMenu(i18n("&Themes"), this));
    ThemeManager::instance()->registerThemeActions(this);
    ThemeManager::instance()->setCurrentTheme(ApplicationSettings::instance()->getCurrentTheme());

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

void DigikamApp::preloadWindows()
{
    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Loading tools..."));
    }

    QueueMgrWindow::queueManagerWindow();
    ImageWindow::imageWindow();
    LightTableWindow::lightTableWindow();

    d->tagsActionManager->registerTagsActionCollections();
}

void DigikamApp::initGui()
{
    // Initialize Actions ---------------------------------------

    d->deleteAction->setEnabled(false);
    d->renameAction->setEnabled(false);
    d->addImagesAction->setEnabled(false);
    d->propsEditAction->setEnabled(false);
    d->openInFileManagerAction->setEnabled(false);

    d->imageViewAction->setEnabled(false);
    d->imagePreviewAction->setEnabled(false);
    d->imageLightTableAction->setEnabled(false);
    d->imageAddLightTableAction->setEnabled(false);
    d->imageScanForFacesAction->setEnabled(false);
    d->imageFindSimilarAction->setEnabled(false);
    d->imageRenameAction->setEnabled(false);
    d->imageDeleteAction->setEnabled(false);
    d->imageExifOrientationActionMenu->setEnabled(false);
    d->openWithAction->setEnabled(false);
    d->slideShowSelectionAction->setEnabled(false);
    m_metadataEditAction->setEnabled(false);
    d->imageAutoExifActionMenu->setEnabled(false);

#ifdef HAVE_MARBLE
    m_geolocationEditAction->setEnabled(false);
#endif

    d->albumSortAction->setCurrentItem((int)ApplicationSettings::instance()->getAlbumSortRole());
    d->imageSortAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSortOrder());
    d->imageSortOrderAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSorting());
    d->imageSeparationAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSeparationMode()-1); // no action for enum 0
    d->imageSeparationSortOrderAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSeparationSortOrder());
    d->recurseAlbumsAction->setChecked(ApplicationSettings::instance()->getRecurseAlbums());
    d->recurseTagsAction->setChecked(ApplicationSettings::instance()->getRecurseTags());
    d->showBarAction->setChecked(ApplicationSettings::instance()->getShowThumbbar());
    showMenuBarAction()->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080

    slotSwitchedToIconView();
}

} // namespace Digikam
