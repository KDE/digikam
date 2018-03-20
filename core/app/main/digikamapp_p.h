/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-31-01
 * Description : main digiKam interface implementation
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Mohamed Anwer <m dot anwer at gmx dot com>
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

// KDE includes

#include <kselectaction.h>

// Local includes

#include "digikam_config.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "cameralist.h"
#include "dsplashscreen.h"
#include "dzoombar.h"
#include "digikamview.h"
#include "metadatastatusbar.h"

class KToolBarPopupAction;

namespace Digikam
{

class ImportUI;
class SearchTextBar;
class FilterStatusBar;
class TagsActionMngr;
class DAdjustableLabel;

class ProgressEntry
{
public:

    ProgressEntry()
        : progress(0),
          canCancel(false)
    {
    }

    QString message;
    float   progress;
    bool    canCancel;
};

// ------------------------------------------------------------------------------

class DigikamApp::Private
{
public:

    Private() :
        autoShowZoomToolTip(false),
        validIccPath(true),
        cameraMenu(0),
        usbMediaMenu(0),
        cardReaderMenu(0),
        quickImportMenu(0),
        config(0),
        newAction(0),
        moveSelectionToAlbumAction(0),
        deleteAction(0),
        renameAction(0),
        imageDeletePermanentlyAction(0),
        imageDeletePermanentlyDirectlyAction(0),
        imageTrashDirectlyAction(0),
        backwardActionMenu(0),
        forwardActionMenu(0),
        addImagesAction(0),
        propsEditAction(0),
        addFoldersAction(0),
        openInFileManagerAction(0),
        refreshAction(0),
        writeAlbumMetadataAction(0),
        readAlbumMetadataAction(0),
        browseTagsAction(0),
        openTagMngrAction(0),
        newTagAction(0),
        deleteTagAction(0),
        editTagAction(0),
        assignTagAction(0),
        imageViewSelectionAction(0),
        imagePreviewAction(0),
#ifdef HAVE_MARBLE
        imageMapViewAction(0),
#endif // HAVE_MARBLE
        imageTableViewAction(0),
        imageIconViewAction(0),
        imageLightTableAction(0),
        imageAddLightTableAction(0),
        imageAddCurrentQueueAction(0),
        imageAddNewQueueAction(0),
        imageViewAction(0),
        imageWriteMetadataAction(0),
        imageReadMetadataAction(0),
        imageFindSimilarAction(0),
        imageSetExifOrientation1Action(0),
        imageSetExifOrientation2Action(0),
        imageSetExifOrientation3Action(0),
        imageSetExifOrientation4Action(0),
        imageSetExifOrientation5Action(0),
        imageSetExifOrientation6Action(0),
        imageSetExifOrientation7Action(0),
        imageSetExifOrientation8Action(0),
        imageRenameAction(0),
        imageRotateActionMenu(0),
        imageFlipActionMenu(0),
        imageAutoExifActionMenu(0),
        imageDeleteAction(0),
        imageExifOrientationActionMenu(0),
        openWithAction(0),
        ieAction(0),
        ltAction(0),
        cutItemsAction(0),
        copyItemsAction(0),
        pasteItemsAction(0),
        selectAllAction(0),
        selectNoneAction(0),
        selectInvertAction(0),
        zoomPlusAction(0),
        zoomMinusAction(0),
        zoomFitToWindowAction(0),
        zoomTo100percents(0),
        imageSortAction(0),
        imageSortOrderAction(0),
        imageSeparationAction(0),
        imageSeparationSortOrderAction(0),
        albumSortAction(0),
        recurseAlbumsAction(0),
        recurseTagsAction(0),
        showBarAction(0),
        viewCMViewAction(0),
        slideShowAction(0),
        slideShowAllAction(0),
        slideShowSelectionAction(0),
        slideShowRecursiveAction(0),
        bqmAction(0),
        maintenanceAction(0),
        qualityAction(0),
        advSearchAction(0),
        addCameraSeparatorAction(0),
        quitAction(0),
        tipAction(0),
        backwardSignalMapper(0),
        forwardSignalMapper(0),
        manualCameraActionGroup(0),
        solidCameraActionGroup(0),
        solidUsmActionGroup(0),
        exifOrientationActionGroup(0),
        eventLoop(0),
        metadataStatusBar(0),
        filterStatusBar(0),
        splashScreen(0),
        view(0),
        cameraList(0),
        tagsActionManager(0),
        zoomBar(0),
        statusLabel(0),
        modelCollection(0)
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
    DigikamView*                        view;
    CameraList*                         cameraList;
    TagsActionMngr*                     tagsActionManager;
    DZoomBar*                           zoomBar;
    DAdjustableLabel*                   statusLabel;

    DigikamModelCollection*             modelCollection;
};

}  // namespace Digikam

#endif // DIGIKAM_APP_PRIVATE_H
