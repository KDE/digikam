/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-31-01
 * Description : main digiKam interface implementation
 *
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#ifndef DIGIKAM_APP_PRIVATE_H
#define DIGIKAM_APP_PRIVATE_H

// Qt includes

#include <QEventLoop>
#include <QMap>
#include <QPointer>
#include <QSignalMapper>
#include <QString>
#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QStringList>
#include <QDomDocument>
#include <QStandardPaths>
#include <QKeySequence>
#include <QMenuBar>
#include <QTimer>
#include <QIcon>
#include <QMessageBox>
#include <QStatusBar>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <ktoolbar.h>
#include <ktoolbarpopupaction.h>
#include <kwindowsystem.h>
#include <kselectaction.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "cameralist.h"
#include "cameratype.h"
#include "cameranamehelper.h"
#include "dsplashscreen.h"
#include "dzoombar.h"
#include "itemiconview.h"
#include "metadatastatusbar.h"
#include "itempropertiestab.h"
#include "importui.h"
#include "setup.h"
#include "actioncategorizedview.h"
#include "drawdecoder.h"
#include "dlayoutbox.h"
#include "album.h"
#include "coredb.h"
#include "albummodel.h"
#include "albumselectdialog.h"
#include "albumthumbnailloader.h"
#include "dbinfoiface.h"
#include "itemgps.h"
#include "categorizeditemmodel.h"
#include "collectionscanner.h"
#include "collectionmanager.h"
#include "componentsinfodlg.h"
#include "coredbthumbinfoprovider.h"
#include "dio.h"
#include "dlogoaction.h"
#include "fileactionmngr.h"
#include "filterstatusbar.h"
#include "iccsettings.h"
#include "itemattributeswatch.h"
#include "iteminfo.h"
#include "imagewindow.h"
#include "lighttablewindow.h"
#include "queuemgrwindow.h"
#include "loadingcache.h"
#include "loadingcacheinterface.h"
#include "loadsavethread.h"
#include "metaengine_rotation.h"
#include "scancontroller.h"
#include "setupeditor.h"
#include "setupicc.h"
#include "thememanager.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "dmetadata.h"
#include "tagscache.h"
#include "tagsactionmngr.h"
#include "databaseserverstarter.h"
#include "metaenginesettings.h"
#include "statusbarprogresswidget.h"
#include "dbmigrationdlg.h"
#include "progressmanager.h"
#include "progressview.h"
#include "maintenancedlg.h"
#include "maintenancemngr.h"
#include "newitemsfinder.h"
#include "dbcleaner.h"
#include "tagsmanager.h"
#include "itemsortsettings.h"
#include "metadatahubmngr.h"
#include "metadataedit.h"
#include "dfiledialog.h"
#include "dpluginloader.h"

#ifdef HAVE_MARBLE
#   include "geolocationedit.h"
#endif

#ifdef HAVE_DBUS
#   include "digikamadaptor.h"
#endif

#ifdef HAVE_KFILEMETADATA
#   include "baloowrap.h"
#endif

class KToolBarPopupAction;

namespace Digikam
{

class SearchTextBar;
class FilterStatusBar;
class TagsActionMngr;
class DAdjustableLabel;

class Q_DECL_HIDDEN ProgressEntry
{
public:

    explicit ProgressEntry()
      : progress(0),
        canCancel(false)
    {
    }

    QString message;
    float   progress;
    bool    canCancel;
};

// ------------------------------------------------------------------------------

class Q_DECL_HIDDEN DigikamApp::Private
{
public:

    explicit Private()
      : autoShowZoomToolTip(false),
        validIccPath(true),
        cameraMenu(nullptr),
        usbMediaMenu(nullptr),
        cardReaderMenu(nullptr),
        quickImportMenu(nullptr),
        config(nullptr),
        newAction(nullptr),
        moveSelectionToAlbumAction(nullptr),
        deleteAction(nullptr),
        renameAction(nullptr),
        imageDeletePermanentlyAction(nullptr),
        imageDeletePermanentlyDirectlyAction(nullptr),
        imageTrashDirectlyAction(nullptr),
        backwardActionMenu(nullptr),
        forwardActionMenu(nullptr),
        addImagesAction(nullptr),
        propsEditAction(nullptr),
        addFoldersAction(nullptr),
        openInFileManagerAction(nullptr),
        refreshAction(nullptr),
        writeAlbumMetadataAction(nullptr),
        readAlbumMetadataAction(nullptr),
        browseTagsAction(nullptr),
        openTagMngrAction(nullptr),
        newTagAction(nullptr),
        deleteTagAction(nullptr),
        editTagAction(nullptr),
        assignTagAction(nullptr),
        imageViewSelectionAction(nullptr),
        imagePreviewAction(nullptr),
#ifdef HAVE_MARBLE
        imageMapViewAction(nullptr),
#endif // HAVE_MARBLE
        imageTableViewAction(nullptr),
        imageIconViewAction(nullptr),
        imageLightTableAction(nullptr),
        imageAddLightTableAction(nullptr),
        imageAddCurrentQueueAction(nullptr),
        imageAddNewQueueAction(nullptr),
        imageViewAction(nullptr),
        imageWriteMetadataAction(nullptr),
        imageReadMetadataAction(nullptr),
        imageScanForFacesAction(nullptr),
        imageFindSimilarAction(nullptr),
        imageSetExifOrientation1Action(nullptr),
        imageSetExifOrientation2Action(nullptr),
        imageSetExifOrientation3Action(nullptr),
        imageSetExifOrientation4Action(nullptr),
        imageSetExifOrientation5Action(nullptr),
        imageSetExifOrientation6Action(nullptr),
        imageSetExifOrientation7Action(nullptr),
        imageSetExifOrientation8Action(nullptr),
        imageRenameAction(nullptr),
        imageRotateActionMenu(nullptr),
        imageFlipActionMenu(nullptr),
        imageAutoExifActionMenu(nullptr),
        imageDeleteAction(nullptr),
        imageExifOrientationActionMenu(nullptr),
        openWithAction(nullptr),
        ieAction(nullptr),
        ltAction(nullptr),
        cutItemsAction(nullptr),
        copyItemsAction(nullptr),
        pasteItemsAction(nullptr),
        selectAllAction(nullptr),
        selectNoneAction(nullptr),
        selectInvertAction(nullptr),
        zoomPlusAction(nullptr),
        zoomMinusAction(nullptr),
        zoomFitToWindowAction(nullptr),
        zoomTo100percents(nullptr),
        imageSortAction(nullptr),
        imageSortOrderAction(nullptr),
        imageSeparationAction(nullptr),
        imageSeparationSortOrderAction(nullptr),
        albumSortAction(nullptr),
        recurseAlbumsAction(nullptr),
        recurseTagsAction(nullptr),
        showBarAction(nullptr),
        viewCMViewAction(nullptr),
        slideShowAction(nullptr),
        slideShowAllAction(nullptr),
        slideShowSelectionAction(nullptr),
        slideShowRecursiveAction(nullptr),
        bqmAction(nullptr),
        maintenanceAction(nullptr),
        qualityAction(nullptr),
        advSearchAction(nullptr),
        addCameraSeparatorAction(nullptr),
        quitAction(nullptr),
        tipAction(nullptr),
        backwardSignalMapper(nullptr),
        forwardSignalMapper(nullptr),
        manualCameraActionGroup(nullptr),
        solidCameraActionGroup(nullptr),
        solidUsmActionGroup(nullptr),
        exifOrientationActionGroup(nullptr),
        eventLoop(nullptr),
        metadataStatusBar(nullptr),
        filterStatusBar(nullptr),
        splashScreen(nullptr),
        view(nullptr),
        cameraList(nullptr),
        tagsActionManager(nullptr),
        zoomBar(nullptr),
        statusLabel(nullptr),
        modelCollection(nullptr)
    {
    }

    bool                                autoShowZoomToolTip;
    bool                                validIccPath;

    QMenu*                              cameraMenu;
    QMenu*                              usbMediaMenu;
    QMenu*                              cardReaderMenu;
    QMenu*                              quickImportMenu;
    QHash<QString, QDateTime>           cameraAppearanceTimes;

    KSharedConfig::Ptr                  config;

    // Album Actions
    QAction*                            newAction;
    QAction*                            moveSelectionToAlbumAction;
    QAction*                            deleteAction;
    QAction*                            renameAction;
    QAction*                            imageDeletePermanentlyAction;
    QAction*                            imageDeletePermanentlyDirectlyAction;
    QAction*                            imageTrashDirectlyAction;
    KToolBarPopupAction*                backwardActionMenu;
    KToolBarPopupAction*                forwardActionMenu;

    QAction*                            addImagesAction;
    QAction*                            propsEditAction;
    QAction*                            addFoldersAction;
    QAction*                            openInFileManagerAction;
    QAction*                            refreshAction;
    QAction*                            writeAlbumMetadataAction;
    QAction*                            readAlbumMetadataAction;

    // Tag Actions
    QAction*                            browseTagsAction;
    QAction*                            openTagMngrAction;
    QAction*                            newTagAction;
    QAction*                            deleteTagAction;
    QAction*                            editTagAction;
    QAction*                            assignTagAction;

    // Image Actions
    KSelectAction*                      imageViewSelectionAction;
    QAction*                            imagePreviewAction;

#ifdef HAVE_MARBLE
    QAction*                            imageMapViewAction;
#endif // HAVE_MARBLE

    QAction*                            imageTableViewAction;
    QAction*                            imageIconViewAction;
    QAction*                            imageLightTableAction;
    QAction*                            imageAddLightTableAction;
    QAction*                            imageAddCurrentQueueAction;
    QAction*                            imageAddNewQueueAction;
    QAction*                            imageViewAction;
    QAction*                            imageWriteMetadataAction;
    QAction*                            imageReadMetadataAction;
    QAction*                            imageScanForFacesAction;
    QAction*                            imageFindSimilarAction;
    QAction*                            imageSetExifOrientation1Action;
    QAction*                            imageSetExifOrientation2Action;
    QAction*                            imageSetExifOrientation3Action;
    QAction*                            imageSetExifOrientation4Action;
    QAction*                            imageSetExifOrientation5Action;
    QAction*                            imageSetExifOrientation6Action;
    QAction*                            imageSetExifOrientation7Action;
    QAction*                            imageSetExifOrientation8Action;
    QAction*                            imageRenameAction;
    QMenu*                              imageRotateActionMenu;
    QMenu*                              imageFlipActionMenu;
    QAction*                            imageAutoExifActionMenu;
    QAction*                            imageDeleteAction;
    QMenu*                              imageExifOrientationActionMenu;
    QAction*                            openWithAction;
    QAction*                            ieAction;
    QAction*                            ltAction;

    // Edit Actions
    QAction*                            cutItemsAction;
    QAction*                            copyItemsAction;
    QAction*                            pasteItemsAction;
    QAction*                            selectAllAction;
    QAction*                            selectNoneAction;
    QAction*                            selectInvertAction;

    // View Actions
    QAction*                            zoomPlusAction;
    QAction*                            zoomMinusAction;
    QAction*                            zoomFitToWindowAction;
    QAction*                            zoomTo100percents;
    KSelectAction*                      imageSortAction;
    KSelectAction*                      imageSortOrderAction;
    KSelectAction*                      imageSeparationAction;
    KSelectAction*                      imageSeparationSortOrderAction;
    KSelectAction*                      albumSortAction;
    QAction*                            recurseAlbumsAction;
    QAction*                            recurseTagsAction;
    QAction*                            showBarAction;
    QAction*                            viewCMViewAction;

    // Tools Actions
    QMenu*                              slideShowAction;
    QAction*                            slideShowAllAction;
    QAction*                            slideShowSelectionAction;
    QAction*                            slideShowRecursiveAction;
    QAction*                            bqmAction;
    QAction*                            maintenanceAction;
    QAction*                            qualityAction;
    QAction*                            advSearchAction;

    // Application Actions
    QAction*                            addCameraSeparatorAction;
    QAction*                            quitAction;
    QAction*                            tipAction;

    QSignalMapper*                      backwardSignalMapper;
    QSignalMapper*                      forwardSignalMapper;
    QActionGroup*                       manualCameraActionGroup;
    QActionGroup*                       solidCameraActionGroup;
    QActionGroup*                       solidUsmActionGroup;
    QActionGroup*                       exifOrientationActionGroup;

    QMap<QString, QPointer<ImportUI> >  cameraUIMap;

    QEventLoop*                         eventLoop;
    QString                             solidErrorMessage;

    MetadataStatusBar*                  metadataStatusBar;
    FilterStatusBar*                    filterStatusBar;
    DSplashScreen*                      splashScreen;
    ItemIconView*                        view;
    CameraList*                         cameraList;
    TagsActionMngr*                     tagsActionManager;
    DZoomBar*                           zoomBar;
    DAdjustableLabel*                   statusLabel;

    DModelFactory*                      modelCollection;
};

} // namespace Digikam

#endif // DIGIKAM_APP_PRIVATE_H
